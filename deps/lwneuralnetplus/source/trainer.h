#ifndef TRAINER_H
#define TRAINER_H
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
#include <string>

#include "network.h"
#include "iomanage.h"
#include "iomanagelwnnfann.h"

namespace ANN {

	/*!\brief Class used for easily training a network with the standard techniques.
	  *
	  * The trainer uses a network and holds three different sets of input / target pairs:
	  * - The training set which is used to train the net
	  * - The validation set which can be used to check the error during training
	  * - The certification set which should not be used during training and which
	  *   gives the final estimate of the error
	  *
	  * The format of files where input/output pairs are stored depend on the
	  * iomanager, an object of class iomanage you can set by the method
	  * set_iomanager(). By default the trainer uses a iomanager of class
	  * iomanagelwnnfann.
	  *
	  * It is possible to write a class which inherits from trainer and which
	  * customizes some of its features.
	  * In particular it is possible to rewrite the code of method check()
	  * which implements the definition of accuracy, and of the method
	  * continue_training(), which implements the stopping condition.
	  */
	class trainer
	{
	public:
		/* Constructors */

		/*!\brief Constructor by a net
		 *\param net Pointer to the network object to be trained
		 *\param error_filename (default = "" means no log file) Name of log file for errors
		 *\param accuracy_filename (default = "" means no log file) Name of log file for accuracy
		 *
		 * If net==NULL throws a runtime_error exception.
		 */
		trainer(network * net, const ::std::string & error_filename =
			"", const ::std::string & accuracy_filename = "");

		/*!\brief Constructor by number of inputs and outputs
		 *\param inputs Number of inputs
		 *\param outputs Number of outputs
		 *\param hidden Number of neurons in the hidden layer
		 *\param error_filename (default = "" means no log file) Name of log file for errors
		 *\param accuracy_filename (default = "" means no log file) Name of log file for accuracy
		 *
		 * Creates a network with one hidden layer, with the logistic function as
		 * activation and uses it for training
		 */
		trainer(int inputs, int outputs, int hidden,
			const ::std::string & error_filename =
			"", const ::std::string & accuracy_filename = "");

		/*!\brief Constructor by specification file (training file)
		 *\param training_file
		 *\param hidden
		 *\param error_filename (default = "" means no log file) Name of log file for errors
		 *\param accuracy_filename (default = "" means no log file) Name of log file for accuracy
		 *\param iomanager the iomanager to be used for reading. If NULL (default) uses a constructor of class iomanagelwnnfann.
		 *
		 * Number of inputs and outputs are written in the specifications.
		 * If hidden == 0 we take hidden = number of inputs
		 *
		 * This constructor also constructs a network with 3 layers, and with right
		 * number od inputs and outputs.
		 */
		trainer(const ::std::string & training_file, int hidden =
			0, const ::std::string & error_filename =
			"", const ::std::string & accuracy_filename = "",
			iomanage* iomanager = NULL);

		/*!\brief Copy Constructor
		 *\param t The trainer you want to copy
		 */
		trainer(const trainer& t);


		/*!\brief Destructor
		 *
		 * When a trainer object is destroyed, the network is kept, so
		 * you can train a network inside a trainer, then delete the trainer
		 * and keep the network.
		 * If you want to delete the trainer and the network, be sure to
		 * get the network by the get_network() method and destroy both
		 * the net and the trainer.
		 */
		virtual ~trainer();

		/* Accessors and Mutators */


		/*!\brief Retrieve current epoch
		 *\return number of times the network was trained on the
		 *        training set
		 *
		 * Current epoch can be set at zero with method clear().
		 * It is set at zero also when
		 * load_training() is called
		 */
		int get_current_epoch();

		/*!\brief Start counting the epochs from epoch
		 *\param epoch epoch number
		 */
		void start_at_epoch(int epoch);

		/*!\brief Set the current epoch at zero and clear best error and best epoch
		 *
		 * This method also removes log files.
		 */
		void clear();


		/*!\brief Get the max number of epochs
		 *\return max number of epochs
		 */
		int get_max_epochs() const;

		/*!\brief Set the max number of epochs
		 *\param nepochs Max number of epochs
		 *
		 * If a train* method is called more than once
		 * the number of epochs is not restarted from 0
		 * but the max number of epoch is relative to
		 * every training session, so if you set
		 * max epochs to 1000 and train the network for
		 * 1000 epochs you can call again train and
		 * train the net for other 1000 epochs (starting
		 * from epoch 1001, of course), if another
		 * stopping condition does not hold before.
		 */
		void set_max_epochs(int nepochs);

		/*!\brief Get the minimum error
		 *\return Min error
		 */
		float get_min_error() const;

		/*!\brief Set the min error
		 *\param error Min error
		 *
		 * The min errror can be evaluated on training set or on
		 * validation set (see set_training_validation())
		 * When this error is reached, training stops.
		 */
		void set_min_error(float error);

		/*!\brief Set the wanted accuracy on validation set
		 *\param accuracy (between 0 and 1). Values outside this interval
		 *       disable wanted accuracy
		 *
		 *\return true if setting had success
		 *
		 * Training stops when this accuracy is reached.
		 * Accuracy is relative to the validation set so
		 * you have to call load_validation() before.
		 *
		 * Accuracy mode should also be setted before.
		 */
		bool set_wanted_accuracy(float accuracy);


		/*!\brief Get wanted accuracy
		 *\return accuracy between 0 and 1. -1 if it's not setted.
		 *
		 * Training stops when this accuracy is reached.
		 * Accuracy is relative to the validation set
		 */
		float get_wanted_accuracy();


		/*!\brief Set the filename of errors log
		 *\param filename ::std::string
		 *
		 * Use filename="" to disable error logging
		 */
		void set_error_filename(const ::std::string & filename);

		/*!\brief Get the filename of errors log
		 *\return ::std::string
		 */
		::std::string get_error_filename() const;


		/*!\brief  Set the filename of accuracy log
		 *\param filename ::std::string
		 *
		 * Use filename="" to disable accuracy logging
		 */
		void set_accuracy_filename(const ::std::string & filename);

		/*!\brief Get filename of accuracy log
		 *\return ::std::string
		 */
		::std::string get_accuracy_filename() const;

		/*!\brief Get the network in the trainer
		 *\return Network
		 */
		network *get_network() const;

		/*!\brief Use a new network in the trainer
		 *\param newnet Pointer to a neural network
		 *\return true if newnet has the right number of inputs and
		 *        outputs and has been setted, false otherwise
		 *        (network is not changed)
		 *
		 * If some training was made before, this method updates
		 * values of errors and accuracy for the new network
		 */
		bool set_network(network * newnet);

		/*!\brief Use in the trainer the best network stored
		 *\return true if operation had success
		 *
		 * This method also deletes the old network, sets the current
		 * epoch to best epoch and error on validation set to
		 * best error.
		 *
		 * Warning: old network is deleted after calling set_network_best()
		 * so you will need  get_network()
		 * to retrieve the network!
		 *
		 * You can't get direct access to the best network stored because
		 * of memory management. So if you need to work on
		 * network with overfit and on best network,
		 * use get_network() before
		 * calling this method, make a copy of that network
		 *
		 * network* copy = new network(trainer->get_network());
		 *
		 * then call set_network_best(), and get_network() will return
		 * the best network.
		 *
		 * Warning 2:
		 * Before calling set_network_best():
		 *   - get_network() returns the overfitted network
		 *   - get_error_on_* returns errors on overfitted network
		 *   - get_best_error()
		 *        returns error on validation set of best network
		 *   - get_current_epoch() returns the epoch when training stopped
		 *   - get_best_epoch() returns epoch when best network was found
		 *   - using_best_net() is true
		 *
		 * After calling set_network_best(), if it returns true:
		 *   - get_network() returns best network, which will be used in
		 *     following trainings
		 *   - get_error_on_* returns errors on best network
		 *   - get_best_error() returns -1.0
		 *   - get_current_epoch() returns the epoch of the best net
		 *   - get_best_epoch() returns 0
		 *   - using_best_net() is false
		 *
		 */
		bool set_network_best();


		/*!\brief Use validation for computing the error
		 *\param train_valid if true use validation set for computing the error
		 *       if false use training set
		 *
		 * The validation set is checked every a fixed number of epochs
		 * (see set_epochs_checking_error())
		 */
		void set_training_validation(bool train_valid);

		/*!\brief Are we computing the error on validation set?
		 *\return if true the trainer is using the validation set to
		 *        compute the error in the training. If false it is
		 *        using the training set.
		 *
		 * If true the validation set is checked every get_epochs_checking_error()
		 * epochs.
		 */
		bool get_training_validation() const;

		/*!\brief Set the number of epochs between a couple of validations
		 *        on validation set
		 *
		 * This value is used if get_training_validation() == true
		 */
		void set_epochs_checking_error(int epochs);

		/*!\brief Get the number of epochs between a couple of validations
		 *        on validation set
		 *
		 * This value is used if get_training_validation() == true
		 */
		int get_epochs_checking_error() const;


		/*!\brief Get the number of epochs between a couple of report
		 *        on validation set
		 *
		 * This is the interval between two reports in verbose mode
		 *\return number of epochs
		 */
		int get_epochs_report() const;

		/*!\brief Set the number of epochs between a couple of reports
		 *        on validation set
		 *\param e number of epochs
		 *
		 * This is the interval between two reports in verbose mode
		 */
		void  set_epochs_report(int e);

		/*!\brief Make the learning stop on overfit
		 *\param stop If true the trainer stops the training
		 *             on overfit i.e. when error on validation set
		 *             starts to increase. If false, normal conditions are checked
		 *\param batches_increasing (only meaningful if stop == true) (optional)
		 *        sets the number of batches the error must be increasing to detect overfit
		 *\param valid_higher (only meaningful if stop == true) (optional)
		 *       sets how much the error on validation set must be higher than
		 *       the error on  training set as a stopping condition for overfitting
		 *
		 * The best net is the net with the minimum error on validation set
		 * The two following stopping conditions, if stop_on_overfit is set to true,
		 * must hold to stop training before max_epochs
		 *
		 * - For at least batches_increasing epochs the error on validation set
		 *   gets higher
		 * - The error on validation set at least is valid_higher times greater
		 *   than the error on training set
		 *
		 * After the training you will have to call set_network_best() to use
		 * the best network found.
		 *
		 */
		void set_stop_on_overfit(bool stop, int batches_increasing =
			0, float valid_higher = 0.0);

		/*!\brief Get  stop on overfit mode
		 *\return If true the trainer stops the training
		 *        on overfit i.e. when error on validation set
		 *        increases (see set_stop_on_overfit()).
		 *        If false, normal conditions are checked
		 */
		bool get_stop_on_overfit() const;


		/*!\brief Get error on validation set of the best net found
		 *\return best error on validation set. -1 if not using_best_net()
		 */
		float get_best_error() const;

		/*!\brief Get the epoch when the best net was saved
		 *\return epoch number
		 */
		int get_best_epoch() const;

		/*!\brief Get the accuracy mode
		 *\return mode
		 *           - 0 (trainer::NO_ACCURACY) = only compute the error
		 *           - 1 (trainer::BINARY_ACCURACY)  = check if (output > .5 <==> target >.5)
		 *           - 2 (trainer::MAXGUESS_ACCURACY)  = check if max neuron is right
		 */
		short int get_accuracy_mode() const;

		/*!\brief Set the accuracy mode
		 *\param mode
		 *
		 *       How to compute accuracy
		 *        - 0 (trainer::NO_ACCURACY)     = only compute the error
		 *        - 1 (trainer::BINARY_ACCURACY) = check if (output > .5 <==> target >.5)
		 *        - 2 (trainer::MAXGUESS_ACCURACY) = check if max neuron is right
		 *
		 * Default accuracy mode is 1.
		 *
		 * This method is virtual because you might need to rewrite it
		 * to give a different meaning to the accuracy mode (and to allow more
		 * than 3 modes).
		 */
		virtual void set_accuracy_mode(short int mode);

		/* Errors and accuracies Accessors */

		/*!\brief Get accuracy on training set
		 *
		 * test() should have been called before
		 */
		float get_accuracy_on_training() const;

		/*!\brief Get accuracy on validaton set
		 *
		 * validate() should have been called before
		 */
		float get_accuracy_on_validation() const;

		/*!\brief Get accuracy on certification set
		 *
		 * certificate() should have been called before
		 */
		float get_accuracy_on_certification() const;

		/*!\brief Get error on training set
		 *
		 * test() should have been called before
		 */
		float get_error_on_training() const;


		/*!\brief Get error on validation set
		 *
		 * validate() should have been called before
		 */
		float get_error_on_validation() const;


		/*!\brief Get error on certification set
		 *
		 * certificate() should have been called before
		 */
		float get_error_on_certification() const;

		/*!\brief Retrieve the number of inputs for the trainer.
		 * \return This must be the number of inputs of the network
		 *         beinh trained and of every set of patterns.
		 */
		int get_no_of_inputs() const;


		/*!\brief Retrieve the number of inputs for the trainer.
		 * \return This must be the number of output of the network
		 *         being trained and of every set of patterns.
		 */
		int get_no_of_outputs() const;

		/*!\brief Retrieve the cause of stopping of last training
		 *\return code of the cause
		 *
		 * Can be
		 * - trainer::STOPPING_MAX_EPOCHS;
		 * - trainer::STOPPING_MIN_ERROR_REACHED_ON_TRAINING;
		 * - trainer::STOPPING_MIN_ERROR_REACHED_ON_BEST_ERROR;
		 * - trainer::STOPPING_OVERFIT;
		 * - trainer::STOPPING_MIN_ERROR_REACHED_ON_VALIDATION;
		 * - trainer::STOPPING_WANTED_ACCURACY_REACHED;
		 */
		short int get_stopping_cause() const;

		/*!\brief Retrieve a ::std::string describing the cause of stopping of last training
		 *\return ::std::string description
		 *
		 * This method is declared virtual, so you can rewrit it if you have
		 * different stopping causes in a derived class which extends trainer.
		 */
		virtual ::std::string get_stopping_cause_string() const;


		/* Methods to load training, validation, certification set */

		/*!\brief Load Training data from a file
		 *\param filename Filename to read from
		 *
		 * Current epoch is set at 0 if the training set is
		 * loaded succesfully.
		 */
		void load_training(const ::std::string & filename);

		/*!\brief Load validation data from a file;
		 */
		void load_validation(const ::std::string & filename);

		/*!\brief Load certification data from a file
		 */
		void load_certification(const ::std::string & filename);



		/*!\brief Are we saving the best net?
		 *\return true if best net is used, false otherwise
		 */
		bool using_best_net() const;

		/*!\brief Train network in on-line mode until it reaches min_error
		 *        or the max number of epochs, or another stopping condition holds.
		 *\param verbose If true writes errors
		 *                (and accuracy, if an accuracy_mode is set) to stdout
		 *\return number of epochs of training in this session (will be
		 *        different by get_current_epoch() if it's not the first
		 *        training session)
		 *
		 * After the training use get_stopping_cause() to know why training session
		 * ended
		 *
		 * If current network is NULL throws a runtime_error exception
		 */
		int train_online(bool verbose);


		/*!\brief Train network in batch mode until it reaches min_error
		 *        or the max number of epochs, or another stopping condition holds.
		 *\param verbose If true writes errors
		 *                (and accuracy, if an accuracy_mode is set) to stdout
		 *\return number of epochs of training in this session (will be
		 *        different by get_current_epoch() if it's not the first
		 *        training session)
		 *
		 * After the training use get_stopping_cause() to know why training session
		 * ended
		 *
		 * If current network is NULL throws a runtime_error exception
		 */
		int train_batch(bool verbose = false);


		/*!\brief Train network in batch + Super SAB mode until it reaches min_error
		 *        or the max number of epochs, or another stopping condition holds..
		 *\param verbose If true writes errors
		 *                (and accuracy, if an accuracy_mode is set) to stdout
		 *\param reset_ssab (default = false) If set to true reset the learning
		 *       rates of the last SuperSAB training to the global learning rate value
		 *\return number of epochs of training in this session (will be
		 *        different by get_current_epoch() if it's not the first
		 *        training session)
		 *
		 * After the training use get_stopping_cause() to know why training session
		 * ended
		 *
		 * If current network is NULL throws a runtime_error exception
		 */
		int train_ssab(bool verbose = false, bool reset_ssab = false);

		/*!\brief Train network in on-line mode with shuffle for presenting
		 *        training set
		 *        until it reaches min_error or the max number of epochs,
		 *        or another stopping condition holds.
		 *\param verbose If true writes errors
		 *                (and accuracy, if an accuracy_mode is set) to stdout
		 *\return number of epochs of training in this session (will be
		 *        different by get_current_epoch() if it's not the first
		 *        training session)
		 *
		 * After the training use get_stopping_cause() to know why training session
		 * ended
		 *
		 * If current network is NULL throws a runtime_error exception
		 */
		int train_shuffle(bool verbose = false);

		/*!\brief Train the network in some mode until it reaches min_error
		 *        or the max number of epochs, or another stopping condition holds.
		 *\param mode 0 = online 1 = batch 2 = batch+ssab 3 = online with shuffle
		 *\param verbose If true writes errors
		 *                (and accuracy, if an accuracy_mode is set) to file
		 *\return number of epochs of training in this session (will be
		 *        different by get_current_epoch() if it's not the first
		 *        training session)
		 *
		 * If current network is NULL throws a runtime_error exception
		 */
		int train(int mode = 0, bool verbose = false);


		/*!\brief Test error and accuracy on the training set.
		 *\return true if the training set was previously loaded,
		 *        false if not and could not compute the values
		 *
		 *
		 * If current network is NULL throws a runtime_error exception
		 */
		bool test();

		/*!\brief Test error and accuracy on the validation set.
		 *\return true if the valdiation set was previously loaded,
		 *        false if not and could not compute the values
		 *
		 * If current network is NULL throws a runtime_error exception
		 */
		bool validate();

		/*!\brief Test error and accuracy on the certification set.
		 *\return true if the certification set was previously loaded,
		 *        false if not and could not compute the values
		 *
		 * If current network is NULL throws a runtime_error exception
		 */
		bool certificate();

		/*!\brief Show results on the selected set computing the results on each of the inputs and comparing the results with the target
		 *\param verbose If true shows all input/output pairs and targets
		 *               If false shows only error and accuracy
		 *\param set What set? 0 = training, 1 = validation (default),
		 *           2 = cerTification
		 *\return mean error
		 *
		 * If current accuracy_mode == trainer::BINARY_ACCURACY or trainer::MAXGUESS_ACCURACY prints the number of right and wrong answers
		 *
		 * If current network is NULL throws a runtime_error exception
		 */
		float show_results(bool verbose = false, int set = 1) const;

		/*!\brief Set iomanager for the trainer
		 *\param iomanager Pointer to an object of an
		 *       iomanage-derived class
		 *
		 * The iomanager is an object of a class derived from
		 * iomanage that manages your file type.
		 *
		 * By default the trainer uses a iomanager of type
		 * iomanagelwnnfann.
		 *
		 * If iomanager == NULL throws a runtime_error exception.
		 *
		 */
		void set_iomanager(iomanage* iomanager);

		/*!\brief Get current iomanager.
		 * The iomanager is an object of a class derived from
		 * iomanage that manages your file type.
		 *
		 * By default the trainer uses a iomanager of type
		 * iomanagelwnnfann.
		 *
		 */
		iomanage* get_iomanager() const;

		/*!\brief Overloaded operator=
		 *\param other other trainer to be copied
		 */
		const trainer& operator= (const trainer& other);

		/*!\brief Costant meaning "no accuracy" */
		static const short int NO_ACCURACY = 0;
		/*!\brief Constant for computing accuracy as O_i < .5 <==> T_i >.5 */
		static const short int BINARY_ACCURACY = 1;

		/*!\brief Constant for computing accuracy as maxind(O) == maxind(T) */
		static const short int MAXGUESS_ACCURACY = 2;

		/*!\brief Constant meaning there is no stopping cause defined */
		static const short int STOPPING_UNDEFINED_CAUSE = 0;
		/*!\brief Training stopped because max number of epochs was reached */
		static const short int STOPPING_MAX_EPOCHS = 1;
		/*!\brief Training stopped because min erroor was reached on training set */
		static const short int STOPPING_MIN_ERROR_REACHED_ON_TRAINING = 2;
		/*!\brief Training stopped because min erroor was reached on best error on validation set (on stop_on_overfit mode) */
		static const short int STOPPING_MIN_ERROR_REACHED_ON_BEST_ERROR = 3;
		/*!\brief Training stopped because of overfit conditions */
		static const short int STOPPING_OVERFIT = 4;
		/*!\brief Training stopped because min error was reached on validation set */
		static const short int STOPPING_MIN_ERROR_REACHED_ON_VALIDATION = 5;
		/*!\brief Training stopped because wanted accuracy was reached on validation set */
		static const short int  STOPPING_WANTED_ACCURACY_REACHED = 6;
	protected:
		/*!\brief Print input data
		 *\param input vector of input
		 *
		 * A way for printing input data.
		 * This class provides a simple format as vector of float.
		 * You might want to make this method do something nicer deriving a class
		 * from trainer.
		 */
		virtual void print_input(float* input) const;

		/*!\brief Print output data
		 *\param output vector of input
		 *
		 * A way for printing input data.
		 * This class provides a simple format for printing as vector of float.
		 * You might want to make this method do something nicer deriving
		 * a class from trainer.
		 */
		virtual void print_output(float* output) const;

		/*!\brief Check if output is right according to target
		 *\param output The output of the net
		 *\param target The right target
		 *\return true if output should be counted as right for
		 *        accuracy, false otherwise.
		 *
		 * This method implements the definition of accuracy.
		 * In standard implementation it depends on accuracy_mode
		 * (see set_accuracy_mode()) but you can rewrite it in a
		 * derived class of trainer in order to customize the definition
		 * of accuracy.
		 *
		 * For example you could define a output to be right if the
		 * norm of the difference between output and target is lesser
		 * than a fixed threshold or, for a network which uses tanh
		 * as activaction function, you could use -1 and 1, instead
		 * of 0 and 1 for binary code.
		 *
		 */
		virtual bool check(float *output, float *target) const;

		/*!\brief Implementation of the stopping condition for the training
		 *\param start the starting epoch of the current training session
		 *\return true if training must continue, false if training must stop.
		 *
		 * This method implements the stopping condition for all the
		 * train_* methods.
		 * If you want a different stopping condition you can write
		 * a derived class from trainer and rewrite this method.
		 *
		 * The stopping condition has a lot of parameters in the standard
		 * implementation: be sure that you really need a different condition
		 * before starting to rewrite this method!
		 *
		 * This is the most critical point in network training so...
		 * be careful! :)
		 *
		 */
		virtual bool continue_training(int start);

		/* These methods are declared protected because you might
		 * need them in the implementation of continue_training()
		 */

		/*!\brief Set the best epoch
		 *\param epoch number of epoch
		 *
		 * This method is declared protected because you might
		 * need it in the implementation of continue_training()
		 */
		void set_best_epoch(int epoch);


		/*!\brief Set the best error on validation set
		 *\param error best error on validation set
		 *
		 * This method is declared protected because you might
		 * need it in the implementation of continue_training()
		 */
		void set_best_error_on_validation(float error);

		/*!\brief Set the stopping cause
		 *\param cause Code of stopping cause
		 *
		 * This method is declared protected because you might
		 * need it in the implementation of continue_training()
		 */
		void set_stopping_cause(short int cause);

		/*!\brief Get the best network
		 *\return Pointer to the best network saved. NULL if not using_best_net()
		 *
		 * This method is declared protected because you might
		 * need it in the implementation of continue_training()
		 */
		network *get_best_net() const;


		/*!\brief Set the best network
		 *\param best Pointer to the best network
		 *
		 * This method is declared protected because you might
		 * need it in the implementation of continue_training()
		 */
		void set_best_net(network* best);

	private:
		/* Print accuracy on file */
		void print_accuracy() const;
		void set_defaults(bool construct_iomanager = true);
		void destroy(int n, float **input, float **output);
		void allocate_data(int npattern, float **&input, float **&target);
		void verbose_report();

		float get_accuracy(int npatterns, float **input, float **target) const;
		float get_error(int npatterns, float **input, float **target) const;
		void compute_error_and_accuracy(float &error, float &accuracy,
			int npatterns, float **input,
			float **target);

		float compute_accuracy(int npatterns, float** input, float** target);

		void compute_accuracy_on_training();

		void reports(bool verbose);

		float show_on_set(bool verbose, int npatterns, float **input, float **target) const;

		static void print_vector(float* v, int n);
		void check_files();

		void copy_data(int n, float** input, float** target, float** srcinput, float** srctarget );
		void copy(const trainer& t);
		void free_trainer();

#ifdef DEBUG
		void print_input(float **, int np) const;
		void print_target(float **patt, int np) const;
#endif


		/* Input length */
		int ninput;
		/* Output length */
		int noutput;
		/* Number of trainig patterns */
		int npattern_training;
		/* Number of validation patterns */
		int npattern_validation;
		/* Number of certification patterns */
		int npattern_certification;

		/* current epoch */
		int epoch;

		/* computing error on validation set? */
		bool train_valid;
		/* each this number of epochs error is computed on validation set */
		int epochs_checking_error;

		/* epochs of report */
		int epochs_report;

		/* stop on overfit */
		bool stop_on_overfit;
		/* how much higher must be the error on validation set for stopping ? */
		float _higher_valid;
		short int accuracy_mode;

		/* Wanted accuracy on validation set. Training stops if this accuracy is reached */
		float wanted_accuracy;


		/* how many batches should pass with validation error increasing to
		 * suppose overfitting?
		 */
		int _batches_not_saving;
		/* epoch of the best net */
		int _best_epoch;

		/* stopping cause */
		short int stopping_cause;

		float error_on_training;
		float error_on_validation;
		float best_error_on_validation;
		float error_on_certification;
		float accuracy_on_training;
		float accuracy_on_validation;
		float accuracy_on_certification;

		/* Training set */
		float **training_set_input;
		float **training_set_target;
		/* Validation set */
		float **validation_set_input;
		float **validation_set_target;
		/* Certification set */
		float **certification_set_input;
		float **certification_set_target;
		/* Max number of epochs */
		int max_epochs;
		/* Min average error */
		float min_error;
		/* File used to store error trend */
		::std::string error_filename;
		/* File used to store accuracy trend */
		::std::string accuracy_filename;
		/* Neural network to train */
		network *net;

		/* Best Neural Network */
		network* best_net;

		/* Iomanager used to read patterns */
		iomanage* iomanager;
	};

	/****************************************
	 * IMPLEMENTATION OF INLINE FUNCTIONS
	 * ACCESSORS AND MUTATORS
	 ****************************************/

	inline float
		trainer::get_min_error() const
	{
		return min_error;
	}


	inline int
		trainer::get_max_epochs() const
	{
		return max_epochs;
	}

	inline network * trainer::get_network() const
	{
		return net;
	}

	inline void
		trainer::set_min_error(float error)
	{
		min_error = error;
	}

	inline bool trainer::get_training_validation() const
	{
		return train_valid;
	}

	inline bool trainer::using_best_net() const
	{
		return (best_net != NULL);
	}


	inline float
		trainer::get_best_error() const
	{
		return best_error_on_validation;
	}




	inline bool trainer::get_stop_on_overfit() const
	{
		return stop_on_overfit;
	}

	inline int
		trainer::get_epochs_checking_error() const
	{
		return epochs_checking_error;
	}

	inline
		int trainer::get_epochs_report() const {
		return epochs_report;
	}

	inline int
		trainer::get_best_epoch() const
	{
		return _best_epoch;
	}

	inline short int
		trainer::get_accuracy_mode() const
	{
		return accuracy_mode;
	}

	inline float
		trainer::get_error_on_training() const
	{
		return error_on_training;
	}

	inline float
		trainer::get_error_on_validation() const
	{
		return error_on_validation;
	}

	inline float
		trainer::get_error_on_certification() const
	{
		return error_on_certification;
	}

	inline float
		trainer::get_accuracy_on_training() const
	{
		return accuracy_on_training;
	}

	inline float
		trainer::get_accuracy_on_validation() const
	{
		return accuracy_on_validation;
	}

	inline float
		trainer::get_accuracy_on_certification() const
	{
		return accuracy_on_certification;
	}

	inline
		int trainer::get_no_of_inputs() const {
		return ninput;
	}


	inline
		int trainer::get_no_of_outputs() const {
		return noutput;
	}

	inline
		iomanage* trainer::get_iomanager() const {
		return iomanager;
	}

	inline
		::std::string trainer::get_error_filename() const {
		return error_filename;
	}

	inline
		::std::string trainer::get_accuracy_filename() const {
		return accuracy_filename;
	}




	inline
		void trainer::set_best_epoch(int epoch) {
		_best_epoch = epoch;
	}


	inline
		void trainer::set_best_error_on_validation(float error) {
		best_error_on_validation = error;
	}

	inline
		int trainer::get_current_epoch() {
		return epoch;
	}

	inline
		void trainer::start_at_epoch(int e) {
		epoch = e;
	}

	inline
		short int trainer::get_stopping_cause() const {
		return stopping_cause;
	}

	inline
		void trainer::set_stopping_cause(short int cause) {
		stopping_cause = cause;
	}

	inline
		network* trainer::get_best_net() const {
		return best_net;
	}

	inline
		void trainer::set_best_net(network* best) {
		best_net = best;
	}


	inline
		float trainer::get_wanted_accuracy() {
		return wanted_accuracy;
	}
};

#endif
