/*
 * Lightweight Neural Net ++ - Trainer class
 * http://lwneuralnetplus.sourceforge.net/
 *
 * This C++ library provides the class trainer wich implements a 
 * the most used techniques for training a network
 *
 * By Lorenzo Masetti <lorenzo.masetti@libero.it> and Luca Cinti <lucacinti@supereva.it>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 */
#include "trainer.h"
#include "shuffle.h"
#include <errno.h>

#define NEPOCHS_DEFAULT 1000
#define MIN_ERROR_DEFAULT 0.001
#define MIN_NEPOCHS 2
#define BATCHES_NOT_SAVING_DEFAULT 100
#define HIGHER_VALID_DEFAULT 5
#define EPOCHS_REPORT_DEFAULT 50

trainer::trainer(network* n, const string& error, const string& accuracy) {
  net = n;
  /* sanity check */

  if (net==NULL) {
    throw runtime_error ("trainer: network is NULL");
  }

  ninput = net->get_no_of_inputs();
  noutput = net->get_no_of_outputs();
  error_filename = error;
  accuracy_filename = accuracy;
  set_defaults();
  check_files();
}

trainer::trainer(int inputs, int outputs, int hidden, const string& error, const string& accuracy) {
  net = new network(network::LOGISTIC, 3, inputs, hidden, outputs);
  ninput = inputs;
  noutput = outputs;

  error_filename = error;
  accuracy_filename = accuracy;
  set_defaults();
  check_files();
}

trainer::trainer(const string& training_file, int hidden, const string& error, const string& accuracy, iomanage* mymanager) {
  set_defaults((mymanager==NULL));
  if (mymanager != NULL) {
    iomanager = mymanager;
  }
  ninput = -1; noutput = -1; /* will be set by load_training */

  load_training(training_file);

  if (hidden==0) { hidden = ninput; }
  net = new network(network::LOGISTIC, 3, ninput, hidden, noutput);
  error_filename = error;
  accuracy_filename = accuracy;
  check_files();
}

void trainer::check_files() {
  if (error_filename != "") {
    ofstream err(error_filename.c_str());
    if (! err.good()) {
      cerr << "trainer: Could not open " << error_filename << " for writing. Unable to write logs!" << endl;
      error_filename = "";
    }
  }

  if (accuracy_filename != "") {
    ofstream acc(accuracy_filename.c_str());
    if (! acc.good() ) {
      cerr << "trainer: Could not open " << accuracy_filename << " for writing. Unable to write accuracy logs!" << endl;
      accuracy_filename = "";
    }  
  }
  
}

void trainer::set_defaults(bool construct_iomanager) {
  npattern_training = 0;
  npattern_validation = 0;
  npattern_certification = 0;
  max_epochs = NEPOCHS_DEFAULT;
  min_error = MIN_ERROR_DEFAULT;

  training_set_input=NULL;
  training_set_target=NULL;

  validation_set_input=NULL;
  validation_set_target=NULL;

  certification_set_input=NULL;
  certification_set_target=NULL;
  
  accuracy_mode = 0;
  

  error_on_training = -1.0;
  error_on_validation = -1.0;
  error_on_certification= -1.0;

  accuracy_on_training= -1.0;
  accuracy_on_validation= -1.0;
  accuracy_on_certification= -1.0;

  train_valid = false;
  stop_on_overfit = false;
  epochs_checking_error = NEPOCHS_DEFAULT / 10;
  epochs_report = EPOCHS_REPORT_DEFAULT;
  
  _batches_not_saving = BATCHES_NOT_SAVING_DEFAULT;
  _higher_valid = HIGHER_VALID_DEFAULT;
  best_net = NULL;
  
  wanted_accuracy=-1.0;
  stopping_cause = trainer::STOPPING_UNDEFINED_CAUSE;

  clear();
  if (construct_iomanager) {
    iomanager = new iomanagelwnnfann();
  }
}

void trainer::clear() {
  epoch = 0;
  best_error_on_validation = -1.0;
  _best_epoch = 0;

  if (best_net != NULL) {
    delete best_net;
    best_net= NULL;
  }

  if (error_filename!="") {
    if (remove(error_filename.c_str()) != 0) {
      if (errno != ENOENT) {
	cerr <<  "trainer: Unable to remove log file " <<  error_filename << "(error code " << errno << ")" << endl;
      }
    };
  }

  if (accuracy_filename!="") {
    if (remove(accuracy_filename.c_str()) != 0) {
      if (errno != ENOENT) {
	cerr << "trainer: Unable to remove log file " << accuracy_filename << "(error code " << errno << ")" << endl;
      }
    };
  }
}

void trainer::destroy (int n, float **input, float **output) {
  iomanage::destroy(n, input, output);
}

void trainer::set_max_epochs(int nepochs) {
  if (nepochs > MIN_NEPOCHS)
    max_epochs = nepochs;
  else
    max_epochs = MIN_NEPOCHS;
}

bool trainer::set_network (network* newnet) {
  /* sanity check */
  if (newnet == NULL) 
    return false;

  if ((newnet->get_no_of_inputs() != ninput) || (newnet->get_no_of_outputs() != noutput)) {
    /* the net does not fit the trainer */
    return false;
  }

  net = newnet;
  if (error_on_training != -1.0) {
    // if some training was made we need to calculate
    // new values for errors and accuracy
    test();
    validate();
    certificate();
  }
  return true;
} 

bool trainer::set_network_best () {
  network* oldnet = net;
  bool success = set_network(best_net);
  if (success) {
    delete oldnet;
    epoch =_best_epoch;
    // error_on_validation = best_error_on_validation;
    // should be automatically set by set network ^^^^^^
    // with the calling of test()
    best_net = NULL;
    best_error_on_validation = -1.0;
    _best_epoch = 0;
  }
  return success;
}


void trainer::set_iomanager(iomanage* manager) {
  if (manager==NULL) {
    throw runtime_error("Could not set iomanager to a NULL value");
  }
  iomanager = manager;
}

bool trainer::set_wanted_accuracy (float accuracy) {
  if (npattern_validation == 0) {
    wanted_accuracy=-1.0;
    return false;
  }
  
  if (accuracy_mode == 0) {
    wanted_accuracy=-1.0;
    return false;
  }

  if (accuracy <= 0.0) {
    wanted_accuracy=-1.0;
    return false;
  }

  if (accuracy > 1.0) {
    wanted_accuracy=-1.0;
    return false;
  }
  
  train_valid = true;
  wanted_accuracy = accuracy;
  return true;
}


void trainer::allocate_data (int npattern, float **&input, float **&target) {
  iomanage::allocate_data(npattern, ninput, noutput, input, target);
}

void trainer::load_training(const string &filename) {
  destroy(npattern_training, training_set_input, training_set_target);
  
  npattern_training = 0;
  
  int ninput_training, noutput_training;
  iomanager->info_from_file(filename, &npattern_training, &ninput_training, &noutput_training);
  if (ninput==-1) { /* if called from constructor */
    ninput = ninput_training;
    noutput = noutput_training;
  } else {
    if ((ninput != ninput_training) || (noutput != noutput_training)) {
      npattern_training = 0;
      throw runtime_error("trainer: Number of inputs or outputs in training patterns is wrong");
    }
  }
  allocate_data (npattern_training, training_set_input, training_set_target);
  
  iomanager->load_patterns(filename, training_set_input, training_set_target, ninput, noutput,npattern_training);

  /* when the training set is changed we set at 0 epoch  */
  clear();

#ifdef DEBUG
  print_input(training_set_input, 0);
  print_target(training_set_target, 0);
#endif
}

void trainer::load_validation(const string &filename) {
  destroy(npattern_validation, validation_set_input, validation_set_target);

  int ninval, noutval;
  iomanager->info_from_file(filename, &npattern_validation, &ninval, &noutval);
  if ((ninput != ninval) || (noutput != noutval)) {
    npattern_validation = 0;
    throw runtime_error ("trainer: Number of inputs or outputs in validation patterns is wrong");
  }
  allocate_data (npattern_validation, validation_set_input, validation_set_target);
  
  iomanager->load_patterns(filename, validation_set_input, validation_set_target, ninput, noutput,npattern_validation);

#ifdef DEBUG
  print_input(validation_set_input, 0);
  print_target(validation_set_target, 0);
#endif
}

void trainer::load_certification(const string &filename) {
  destroy(npattern_certification, certification_set_input, certification_set_target);

  int nincer, noutcer;
  iomanager->info_from_file(filename, &npattern_certification, &nincer, &noutcer);
  if ((ninput != nincer) || (noutput != noutcer)) {
    npattern_certification = 0;
    throw runtime_error ("trainer: Number of inputs or outputs on  certification patterns is wrong");
  }

  allocate_data (npattern_certification, certification_set_input, certification_set_target);

  iomanager->load_patterns(filename, certification_set_input, certification_set_target, ninput, noutput,npattern_certification);

#ifdef DEBUG
  print_input(certification_set_input, 0);
  print_target(certification_set_target, 0);
#endif
}

#ifdef DEBUG
/* Methods used for debugging but useless.... */
void trainer::print_input(float** patt, int np)  const {
  for (int i=0; i < ninput; i++) {
    cout << patt[np][i] << " ";
  }
  cout << endl;
}

void trainer::print_target(float** patt, int np)  const {
  for (int i=0; i < noutput; i++) {
    cout << patt[np][i] << " ";
  }
  cout << endl;
}
#endif

void trainer::set_training_validation(bool training_valid) {
  if (npattern_validation==0) {
    train_valid = false;
  } else {
    train_valid = training_valid;
  }
}



void trainer::set_stop_on_overfit(bool stop, int batches_not_saving, float valid_higher) {
  if (npattern_validation==0) {
    train_valid = false;
    stop_on_overfit = false;
  } else {
    stop_on_overfit = stop;
    if (stop_on_overfit) { 
      train_valid = true; 
      if (batches_not_saving > 0) { _batches_not_saving = batches_not_saving; }
      if (valid_higher>0.0) { _higher_valid = valid_higher; }
#ifdef DEBUG_OVERFIT
      cout << "Setting stop on overfit for " << batches_not_saving << " batches with higher valid " << _higher_valid << endl;
#endif
    }
  }
}


void trainer::set_accuracy_mode (short int mode) {
  if ((mode>2) || (mode <0)) {
    return;
  }
  accuracy_mode = mode;
}



bool trainer::continue_training (int start) {
  if (epoch - start < MIN_NEPOCHS) {
    return true;
  }

  if (epoch-start>max_epochs) {
    stopping_cause = trainer::STOPPING_MAX_EPOCHS;
    return false;
  }

  if (! train_valid) {
    /* validation set is not used for stopping condition */
    stopping_cause = trainer::STOPPING_MIN_ERROR_REACHED_ON_TRAINING;
    return  (error_on_training >= min_error);
  }


  /* now train_valid is true */
  if ( epoch % epochs_checking_error != 0) {
    return true;
  }

  validate();

  if (wanted_accuracy != -1.0) {
    if (accuracy_on_validation >= wanted_accuracy) {
      stopping_cause = trainer::STOPPING_WANTED_ACCURACY_REACHED;
      return false;
    }
  }

  if (stop_on_overfit) {
    /* 
     * A good method to avoid overfitting.
     * See Tom Mitchell, Machine Learning, 1997, McGraw-Hill
     */
#ifdef DEBUG_OVERFIT
  cout << "Epoch: " << epoch << "\tbest_err " << best_error_on_validation << "\tErr valid " << error_on_validation << "\t\tErr training " << error_on_training; 
#endif

    /* first time... */
    if (best_error_on_validation==-1.0) {
      best_error_on_validation = error_on_validation;
    }

    if ((error_on_validation <= best_error_on_validation)) {
      delete best_net;
      best_net = new network(*net);
      best_error_on_validation = error_on_validation;
      _best_epoch = epoch;
#ifdef DEBUG_OVERFIT
      cout << " SAVING !! " ;
#endif 
    } 
#ifdef DEBUG_OVERFIT
    cout <<  endl;
#endif
    if (best_error_on_validation < min_error) {
#ifdef DEBUG_OVERFIT
      cout << "Min error reached on validation set" << endl;
#endif
      stopping_cause = trainer::STOPPING_MIN_ERROR_REACHED_ON_BEST_ERROR;
      return false;
    }

#ifdef DEBUG_OVERFIT
    cout << " error on valid is " << (error_on_validation/error_on_training) << " times greater than error on training (we want " << _higher_valid << ") " << endl;
    cout << " Epoch " << epoch << " Best Epoch " << _best_epoch << " Batches requires " << _batches_not_saving <<  " and now it's " << t-_best_epoch << endl;
    if (((epoch - _best_epoch) > _batches_not_saving) && ((error_on_validation/error_on_training) > _higher_valid)) {
      cout << " STOPPING " << endl;
    }
#endif

    if (epoch - _best_epoch < _batches_not_saving) {
#ifdef DEBUG_OVERFIT
      cout << "Continuing because not enough epochs..." << endl;
#endif
      return true;
    }
    stopping_cause = trainer::STOPPING_OVERFIT;
    return ((error_on_validation/error_on_training) <= _higher_valid);
  } else {
    /* ! (stop_on_overfit) */
    stopping_cause = trainer::STOPPING_MIN_ERROR_REACHED_ON_VALIDATION;
    return (error_on_validation >= min_error);
  }
}

void trainer::verbose_report() {
  int w = cout.width(11);
  if (! train_valid ) {
      cout << "Epoch: " << epoch << "\tError: " << error_on_training;
      cout << endl;
  } else { /** train_valid **/
    cout << "Epoch: " << epoch << "\t Error: " << error_on_training;
    cout << "\tErr Valid: " << error_on_validation ;
    if (using_best_net()) {
      if (best_error_on_validation != error_on_validation) {
	cout << " (best " << best_error_on_validation << ")";
      }
    }
    if (accuracy_mode > 0) {
      cout << "\t" << ((accuracy_mode==1)?"Binary Acc.: ":"Max Guess: ") << accuracy_on_validation*100 << "%";
    }
    cout << endl;
  }
  cout.width(w);
}


void trainer::reports (bool verbose) {
    if ((verbose) && (epoch % epochs_report == 0)) {
      verbose_report();
    }

    if (error_filename == "") 
      return;
  
    FILE* err_file = fopen(error_filename.c_str(),"a");

    if (! train_valid) {
      fprintf(err_file,"%f\n", error_on_training);
      if (accuracy_mode > 0) {
	compute_accuracy_on_training();
	print_accuracy();
      }
    } else { /** train_valid **/
      if (epoch <=MIN_NEPOCHS) return; 
      if ((epoch-1) % epochs_checking_error == 0) {
	fprintf(err_file, "%i\t%f\t%f\n", (epoch-1),error_on_training, error_on_validation);
	if (accuracy_mode > 0) {
	  compute_accuracy_on_training();
	  print_accuracy();
	}
      }
    }
    fclose(err_file);
}

int  trainer::train_online(bool verbose) {    
  if (net==NULL) {
    throw runtime_error("trainer: network is NULL");
  }
  float error;
  int start = epoch;
  while (continue_training(start)) {
    error = 0.0;
    for (int i = 0; i < npattern_training; i++) {
      /* compute the outputs for inputs[i] */
      net->compute (training_set_input[i], NULL);      
      /* find the error with respect to targets[i] */
      error += net->compute_output_error ( training_set_target[i] );	
      /* train the network one step */
      net->train ();
    }
    error_on_training = error / npattern_training;
    epoch++;
    reports(verbose);
  }
  return epoch - start;
}

int trainer::train_batch(bool verbose) {
  /* sanity check */
  if (net==NULL) {
    throw runtime_error("trainer: network is NULL");
  }
  float error;
  int start = epoch;
  while (continue_training(start)) {
    error = 0.0;
    net->begin_batch();
    for (int i = 0; i < npattern_training; i++) {
      /* compute the outputs for inputs[i] */
      net->compute (training_set_input[i], NULL);      
      /* find the error with respect to targets[i] */
      error += net->compute_output_error ( training_set_target[i] );	
      /* train the network one step */
      net->train_batch ();
    }
    net->end_batch();
    error_on_training = error / npattern_training;
    epoch++;
    reports(verbose);
  }
  return epoch - start;
}

int trainer::train_ssab(bool verbose, bool reset_ssab) {
 /* sanity check */
  if (net==NULL) {
    throw runtime_error("trainer: network is NULL");
  }

  if (! net->is_ssab_active() ) {
    net->begin_ssab();
  } else if (reset_ssab) {
    net->reset_ssab();
  }
  float error;
  int start = epoch;
  while (continue_training(start)) {
    error = 0.0;
    net->begin_batch();
    for (int i = 0; i < npattern_training; i++) {
      /* compute the outputs for inputs(i) */
      net->compute (training_set_input[i], NULL);      
      /* find the error with respect to targets(i) */
      error += net->compute_output_error ( training_set_target[i] );	
      /* train the network one step */
      net->train_batch ();
    }
    net->end_batch_ssab();
    error_on_training = error / npattern_training;
    epoch++;
    reports(verbose);
  }
  return epoch - start;
}

int  trainer::train_shuffle(bool verbose) {    
  /* sanity check */
  if (net==NULL) {
    throw runtime_error("trainer: network is NULL");
  }
  float error;
  int start = epoch;
  // a permutation of integers 0...n-1
  shuffle shuf(npattern_training);

  while (continue_training(start)) {
    error = 0.0;
    for (int i = 0; i < npattern_training; i++) {
      /* compute the outputs for inputs[shuf[i]] */
      net->compute (training_set_input[shuf[i]], NULL);      
      /* find the error with respect to targets[shuf[i]] */
      error += net->compute_output_error ( training_set_target[shuf[i]] );	
      /* train the network one step */
      net->train ();
    }
    error_on_training = error / npattern_training;
    epoch++;
    reports(verbose);
    shuf.redo();
  }
  return epoch - start;
}


int trainer::train (int mode, bool verbose) {
#ifdef DEBUG
  cout << "Training in mode " << mode << endl;
#endif
  if (mode==1) {
    return train_batch(verbose);
  } else if (mode==2) {
    return train_ssab(verbose,false);
  } else if (mode==3) {
    return train_shuffle(verbose);
  } else {
    return train_online(verbose);
  }
}

bool trainer::check(float* output, float* ctrl)  const {
  if (accuracy_mode==2) {
    float maxout = output[0];
    float maxctrl = ctrl[0];
    int indout = 0;
    int indctrl = 0;
    for (int i=1; i < noutput; i++) {
      if (ctrl[i] > maxctrl) {
	maxctrl = ctrl[i];
	indctrl = i;
      }
      if (output[i] > maxout) {
	maxout = output[i];
	indout = i;
      }    
    }
    return (indout == indctrl);
  } else {
    for (int i = 0; i < noutput; i++) {
      if ((ctrl[i]<.5)?(output[i]>.5):(output[i]<.5)) {
	return false;
      }
    }
    return true;
  } 
}

void trainer::print_accuracy()  const {
  if (!accuracy_filename.empty()) {
    FILE* acc_file;
    acc_file = fopen(accuracy_filename.c_str(), "a");      
    if (train_valid) {
      fprintf(acc_file, "%i\t%f\t%f\n", epoch, accuracy_on_training,accuracy_on_validation);
    } else {
      fprintf(acc_file, "%f\n",  accuracy_on_training);
    }
    fclose(acc_file);
  }
}

void trainer::compute_error_and_accuracy(float& error, float &accuracy, int npatterns, float** input, float** target) {
 /* sanity check */
  if (net==NULL) {
    throw runtime_error("trainer: network is NULL");
  }

 float* output = (float*) malloc (noutput * sizeof (float));
 error = 0.0;
 int nok = 0;
 for (int i=0; i <npatterns; i++) {
   net->compute (input[i], output);
   if (accuracy_mode > 0) {
     if (check(output, target[i])) {
       nok++;
     }
   }
   error += net->compute_output_error(target[i]);
 }
 error = error / npatterns;
 if (accuracy_mode > 0) {
   accuracy = ((float) nok)/((float) npatterns);
 } 
 free(output);
}


float trainer::compute_accuracy(int npatterns, float** input, float** target) {
 /* sanity check */
  if (net==NULL) {
    throw runtime_error("trainer: network is NULL");
  }
  if (accuracy_mode==0) {
    return -1.0;
  }

 float* output = (float*) malloc (noutput * sizeof (float));
  int nok = 0;
  for (int i=0; i <npatterns; i++) {
    net->compute (input[i], output);
      if (check(output, target[i])) {
	nok++;
      }
  }
  free(output);
  return ((float) nok)/((float) npatterns);
}

void trainer::compute_accuracy_on_training() {
  accuracy_on_training = compute_accuracy(npattern_training, training_set_input, training_set_target);
}

bool trainer::test() {
  if (npattern_training==0) {
    return false;
  }
  
  compute_error_and_accuracy(error_on_training, accuracy_on_training, npattern_training,training_set_input, training_set_target);
  return true;
}

bool trainer::validate() {
  if (npattern_validation==0) {
    return false;
  }
  
  compute_error_and_accuracy(error_on_validation, accuracy_on_validation, npattern_validation,validation_set_input, validation_set_target);
  return true;
}

bool trainer::certificate() {
  if (npattern_certification==0) {
    return false;
  }
  
  compute_error_and_accuracy(error_on_certification, accuracy_on_certification, npattern_certification, certification_set_input, certification_set_target);
  return true;
}
  
float trainer::show_results (bool verbose,int set)  const {
  if (set == 0) {
    return show_on_set(verbose,npattern_training, training_set_input, training_set_target);
  }
  
  if (set == 2) {
    return show_on_set(verbose,npattern_certification, certification_set_input, certification_set_target);
  } 

  return show_on_set(verbose,npattern_validation, validation_set_input, validation_set_target);
}

float trainer::show_on_set(bool verbose,int npatterns, float** input, float** target)  const {
 /* sanity check */
  if (net==NULL) {
    throw runtime_error("trainer: network is NULL");
  }

  if (npatterns<=0) {
    throw runtime_error("trainer: the set was not loaded");
  }

  int wrong = 0;
  float* output = (float*) malloc (noutput * sizeof(float));
  float error;
  float avg_error = 0.0;

  for (int i = 0; i < npatterns; i++) {
    /* compute the network for input [i] */
    net->compute ( input[i] , output);

    /* find the output error for target [i] */
    error = net->compute_output_error ( target [i]);
    avg_error += error;

    /* (if verbose) print everything: inputs -> outputs (targets) error */
    if (verbose) {
      print_input(input[i]);
      printf ("->");
      print_output(output);
      printf (" (");
      print_output(target[i]);
      printf (") %.5f ", error);
    }

    if (verbose) {
      if (accuracy_mode>0) {
	if (check(output,target[i])) {
	  printf(" RIGHT\n");
	} else {
	  printf(" WRONG!!\n");
	  wrong++;
	}
      }
    } else {
      if (accuracy_mode>0) {
	if (! check(output, target[i]) ) wrong++;
      }
    }
  }
  avg_error = avg_error / npatterns;
  printf("\n");
  printf("Average error : %.3f\n", avg_error); 
  if (accuracy_mode>0) {
    float perc = 100.0 * wrong / npatterns;
    printf("\nErrors: %i out of %i (%.3f\%)\n",wrong,npatterns, perc );
    if (accuracy_mode==1) {
      printf("Binary Acc: ");
    } else {
      printf("Max Guess Acc: ");
    }
    printf("%.3f\%\n", 100.0 - perc);
  }
  free(output);
  return avg_error;
}


string trainer::get_stopping_cause_string() const {
  switch (stopping_cause) {
  case trainer::STOPPING_MAX_EPOCHS:
    return "Max number of epochs reached";
  case trainer::STOPPING_MIN_ERROR_REACHED_ON_TRAINING:
    return "Min Error reached on training set";
  case trainer::STOPPING_MIN_ERROR_REACHED_ON_BEST_ERROR:
    return "Min Error reached on best error on validation set";
  case trainer::STOPPING_OVERFIT:
    return "Overfit";
  case trainer::STOPPING_MIN_ERROR_REACHED_ON_VALIDATION:
    return "Min error reached on validation set";
  case trainer::STOPPING_WANTED_ACCURACY_REACHED:
    return "Wanted accuracy reached on validation set";
  }
  return "Undefined Cause";
}


void trainer::set_epochs_report (int e) {
  if ((train_valid) && (e < epochs_checking_error)) {
    epochs_report = epochs_checking_error;
    return;
  }
  epochs_report = e;
}


void
trainer::set_epochs_checking_error (int epochs)
{
  epochs_checking_error = epochs;
  if (epochs_report < epochs) {
    epochs_report = epochs;
  }
}


void
trainer::set_error_filename (const string & filename)
{

  if (! filename.empty()) {
    ofstream err(filename.c_str());
    if (! err.good()) {
      cerr << "trainer: Could not open " << filename << " for writing. Unable to write logs!" << endl;
      error_filename = "";
      return;
    }
  }

  error_filename = filename;
}

void
trainer::set_accuracy_filename (const string & filename)
{
  if (! filename.empty()) {
    ofstream acc(filename.c_str());
    if (! acc.good() ) {
      cerr << "trainer: Could not open " << filename << " for writing. Unable to write accuracy logs!";
      accuracy_filename = "";
    }  
  }

  accuracy_filename = filename;
}



void trainer::print_vector(float* v, int n) {
  for (int i = 0; i < n; i++) {
    printf ("%.3f", v[i]);
    if (i != n-1) {
      printf(" ");
    }
  }
}
     

void trainer::print_input(float* v) const {
  print_vector(v, ninput);
}



void trainer::print_output(float* v) const {
  print_vector(v,noutput);
}

/*** MEMORY MANAGEMENT ***/

trainer::~trainer() {
  free_trainer();
}

void trainer::free_trainer() {
  destroy(npattern_training, training_set_input, training_set_target);

  destroy(npattern_validation, validation_set_input, validation_set_target);
  
  destroy(npattern_certification, certification_set_input, certification_set_target);
  delete best_net;
}


trainer::trainer(const trainer& t) {
  copy(t);
  check_files();
}

void trainer::copy(const trainer& t) {  
  ninput = t.ninput;
  noutput = t.noutput;
  npattern_training = t.npattern_training; 
  npattern_validation = t.npattern_validation;
  npattern_certification = t.npattern_certification;

  epoch = t.epoch;

  train_valid =t.train_valid;
  epochs_checking_error = t.epochs_checking_error;
  epochs_report = t.epochs_report;

  stop_on_overfit = t.stop_on_overfit;
  _higher_valid = t._higher_valid;
  accuracy_mode = t.accuracy_mode;
  _batches_not_saving = t._batches_not_saving;
  _best_epoch = t._best_epoch;
  
  wanted_accuracy = t.wanted_accuracy;

  stopping_cause = t.stopping_cause;
  
  error_on_training = t.error_on_training;
  error_on_validation = t.error_on_validation;
  best_error_on_validation = t.best_error_on_validation;
  error_on_certification = t.error_on_certification;
  accuracy_on_training = t.error_on_training;
  accuracy_on_validation = t.accuracy_on_validation;
  accuracy_on_certification = t.accuracy_on_certification;
  
  training_set_input=NULL;
  training_set_target=NULL;

  validation_set_input=NULL;
  validation_set_target=NULL;

  certification_set_input=NULL;
  certification_set_target=NULL;

  if (npattern_training > 0) {
    allocate_data (npattern_training, training_set_input, training_set_target);
    copy_data (npattern_training, training_set_input, training_set_target, t.training_set_input, t.training_set_target);
    
  }

  if (npattern_validation > 0) {
    allocate_data (npattern_validation, validation_set_input, validation_set_target);
    copy_data (npattern_validation, validation_set_input, validation_set_target,t.validation_set_input, t.validation_set_target);
  }

  if (npattern_certification > 0) {
    allocate_data (npattern_certification, certification_set_input, certification_set_target);
    copy_data (npattern_certification, certification_set_input, certification_set_target, t.certification_set_input, t.certification_set_target);
  }

  max_epochs = t.max_epochs;
  min_error = t.min_error;
  error_filename=t.error_filename;
  accuracy_filename=t.accuracy_filename;

 
  if (t.net==NULL) {
    net = NULL;
  } else { 
    net = new network(*(t.net));
  }

  if (t.best_net==NULL) {
    best_net = NULL;
  } else {
    best_net = new network(*(t.net));
  }
  iomanager = t.iomanager;
}




void trainer::copy_data(int n, float** input, float** target, float** srcinput, float** srctarget ) {
  for (int i = 0; i < n; i++) {
    memcpy(input[i], srcinput[i], ninput * sizeof(float));
  }

  for (int i = 0; i < n; i++) {
    memcpy(target[i], srctarget[i], noutput * sizeof(float));
  }
}



const trainer&
trainer::operator= (const trainer& other) {
  // guard against assignment to itself
  if (this == &other) {
    return *this;
  }
  
  free_trainer();
  copy(other);
  return *this;
}








