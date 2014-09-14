#ifndef NETWORK_H
#define NETWORK_H
/*
 * Lightweight Neural Net ++ 
 * http://lwneuralnetplus.sourceforge.net/
 *
 * This C++ library provides the class network wich implements a 
 * feed forward neural network with backpropagation,
 * You can use logistic or tanh as sigmoidal function
 * Library provides on line training, momentum, 
 * batch training and superSAB training.
 *
 * By Lorenzo Masetti <lorenzo.masetti@libero.it> and Luca Cinti <lucacinti@supereva.it>
 * Based on lwneuralnet C library by Peter Van Rossum <petervr@debian.org>, Luca Cinti and Lorenzo Masetti 
 * http://lwneuralnet.sourceforge.net
 *
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 */
#define NET_LOGISTIC 0
#define NET_TANH 1


#include <stdio.h>
#include <stdexcept>
#include <iostream>
#include <vector>


/*! \brief Class implementing a feed forward neural network with backpropagation learning
 */
class network
{

public:

  /*!\brief Public constant for the logistic function */
  static const int LOGISTIC = NET_LOGISTIC;
  /*!\brief Public constant for the tanh function */
  static const int TANH = NET_TANH;


  /*!\brief Constructor for a network 
   * \param activ activation function (network::LOGISTIC or network::TANH)
   * \param no_of_layers Integer.
   * \param ... Sequence of integers.
   *
   * Allocate memory for a neural network with no_of_layers layers,
   * including the input and output layer. The number of neurons in each
   * layer is given as ..., starting with the input layer and ending with
   * the output layer.
   *
   * The parameters of the network are set to default values. 
   * (for example momentum is 0). 
   * You can change them later by the mutators methods.
   *
   * If no_of_layers < 2 throws a runtime_error exception
   */
    network (int activ, int no_of_layers, ...);


  /*!\brief Constructor for a network 
   * \param activ activation function (network::LOGISTIC or network::TANH)
   * \param layers vector of integers containing the number of neurons of each
   *        layer
   *
   * Allocate memory for a neural network with layers.size() layers,
   * including the input and output layer. The number of neurons in each
   * layer is given in the vector, starting with the input layer and ending with
   * the output layer.
   *
   * The parameters of the network are set to default values. 
   * (for example momentum is 0). 
   * You can change them later by the mutators methods.
   *
   * If layers.size() < 2 throws a runtime_error exception
   */
    network (int activ, std::vector<int> layers);


  /*!\brief Constructor. Load network from  file.
   * \param filename Pointer to name of file to load 
   * \param binary bool if true (default) the file is binary
   *        otherwise is a text file 
   * If filename does not exist throws a runtime_error exception
   */
    network (const char *filename, bool binary = true);

  /*! \brief Copy constructor
   */
    network (const network & b);

  /*!\brief Destructor. Free memory allocated for a network.
   */
   ~network ();

  /*!\brief Assign random values to all weights in the network.
   * \param range Floating point number.
   *
   * All weights in the neural network are assigned a random value
   * from the interval [-range, range].
   */
  void randomize (float range);

  /****************************************
   * Accessors
   ****************************************/

  /*!\brief Retrieve the momentum of a network.
   * \return Momentum of the neural work.
   */
  float get_momentum () const;

  /*!\brief Retrieve the momentum of a network.
   * \return Learning rate of the neural work.
   */
  float get_learning_rate () const;


  /*!\brief Retrieve the number of inputs of a network.
   * \return Number of neurons in the input layer of the neural network.
   */
  int get_no_of_inputs () const;


  /*!\brief Retrieve the number of outputs of a network.
   * \return Number of neurons in the output layer of the neural network.
   */
  int get_no_of_outputs () const;


  /*!\brief Retrieve the number of layers of a network.
   * \return Number of layers, including the input and output layers, of the 
   * neural network.
   */
  int get_no_of_layers () const;

  /*!\brief Retrieve number of neurons on a layer of a netwrwork
   * \param l layer index ( should be 0 <= l < get_no_of_layers() )
   * \return number of neurons on layer l
   */
  int get_no_of_neurons (int l) const;


  /*!\brief Retrieve a weight of a network.
   * \param l Number of lower layer.
   * \param nl Number of neuron in the lower layer.
   * \param nu Number of neuron in the next layer.
   * \return Weight connecting the neuron numbered nl in the layer
   * numbered l with the neuron numbered nu in the layer numbered l+1.
   */
  float get_weight (int l, int nl, int nu) const;


  /*!\brief Retrieve the number of patterns in batch training
   * \return number of patterns
   */
  int get_no_of_patterns () const;


  /*!\brief Retrieve the activation function of network (network::LOGISTIC or network::TANH)
   * \return activation function
   */
  int get_activation () const;


  /*!\brief Retrieve the output error of a network.
     * \return Output error of the neural network.
     *
     * Before calling this routine, compute() and
     * compute_output_error() should have been called to compute outputs
     * for given inputs and to acually compute the output error. This
     * routine merely returns the output error (which is stored internally
     * in the neural network).
   */
  float get_output_error () const;

  /* Accessors for parameters of SuperSab */

  /*!\brief Retrieve maximum learning rate allowed in SuperSab mode 
   * \return float maximum learning rate
   *
   * Values of learning rates cannot be greater than this value
   */
  float get_max_learning_rate ();

  /*!\brief Retrieve minimum learning rate allowed in SuperSab mode 
   * \return float minimum learning rate
   *
   * Values of learning rates cannot be lesser than this value
   */
  float get_min_learning_rate ();

  /*!\brief Retrieve factor for increasing learning rate in SuperSab mode
   * \return float factor for increasing learning rate
   *
   * In SuperSab mode: if delta at this step has the same sign of delta at
   * the previous step, the learning rate of that weight is multiplied by
   * this value
   */
  float get_ssab_up_factor ();

  /*!\brief Retrieve factor for decreasing learning rate in SuperSab mode
   * \return float factor for decreasing learning rate
   *
   * In SuperSab mode: if delta at this step has the opposite  sign of delta at
   * the previous step, the learning rate of that weight is multiplied by
   * this value
   */
  float get_ssab_down_factor ();


  /****************************************
   * Mutators
   ****************************************/

  /*!\brief Change the learning rate of a network.
   * \param learning_rate Floating point number.
   */
  void set_learning_rate (float learning_rate);


  /*!\brief Set activation function of the network.
   * \param num_func Number of function (network::LOGISTIC or network::TANH)
   */
  void set_activation (int num_func);


  /*!\brief Change the momentum of a network.
   * \param momentum Floating point number.
   */
  void set_momentum (float momentum);

  /* Mutators for parameter of SuperSab training */


  /*!\brief Set maximum learning rate allowed in SuperSab mode 
   * \param max maximum learning rate
   *
   * Values of learning rates cannot be greater than this value.
   *
   * If the previous max learning rate was greater than the new one
   * and SuperSab mode is active, all the learning rates are changed to make
   * them lesser than the new maximum. 
   *
   * So, if you just want to change default max learning rate, 
   * call this method before begin_ssab().
   */
  void set_max_learning_rate (float max);
  

  /*!\brief Set minimum learning rate allowed in SuperSab mode 
   * \param min minimum learning rate
   *
   * Values of learning rates cannot be lesser than this value
   *
   * If the previous min learning rate was lesser  than the new one
   * and SuperSab mode is active, all the learning rates are changed to make
   * them greater than the new minimum. 
   *
   * So, if you just want to change default min learning rate, 
   * call this method before begin_ssab().
   */
  void set_min_learning_rate (float min);

  /*!\brief Set factor for increasing learning rate in SuperSab mode
   * \param factor (for increasing learning rate)
   *
   * In SuperSab mode: if delta at this step has the same sign of delta at
   * the previous step, the learning rate of that weight is multiplied by
   * this value ( should be factor > 1 )
   */
  void set_ssab_up_factor (float factor);

  /*!\brief Set factor for decreasing learning rate in SuperSab mode
   * \param factor (for decreasing learning rate)
   *
   * In SuperSab mode: if delta at this step has the opposite  sign of delta at
   * the previous step, the learning rate of that weight is multiplied by
   * this value ( should be 0 < factor < 1 )
   */
  void set_ssab_down_factor (float factor);



  /****************************************
   * File I/O for binary files
   ****************************************/


  /*!\brief Write a network to a binary file.
   * \param filename Pointer to name of file to write to.
   *
   * If it is impossible to write on the file throws a 
   * runtime_error exception.
   */
  void save (const char *filename) const;


  /*!\brief Read a network from a binary file.
   * \param filename Pointer to name of file to read from.
   * If filename does not exist, or the format of 
   * the file is wrong throws a runtime_error exception.
   *
   * It is possible to import files in old format
   * used by C-library lwneuralnet
   */
  void load (const char *filename);

  /****************************************
   * Friendly printing
   ****************************************/

  /*!\brief Write a network to stdout in a friendly format.
   * \param show If show==true weights are displayed
   *
   * Se also operator<<() 
   */
  void friendly_print (const bool show = false) const;

  /*!\brief Export to a file in a format readable
   *        by graphviz http://graphviz.org
   *        in order to draw a graph which represents
   *        the network
   *\param filename filename to write to
   */
  void export_to_graphviz(const char* filename) const;

  /****************************************
   * File I/O for Text Files
   ****************************************/

  /* Please note that Text File format is provided for compatibility
   * with old lwneuralnet format but it should not be used.
   * Howewer textual format could be used
   * for portability between machines that use different binary encoding for
   * floating point numbers 
   *
   * NOTE FOR LWNEURALNET USERS:
   *
   * Text files containing networks created by lwneuralnet might have a 
   * different format, which does not have the number of sigmoidal function
   * as first information. But, since in old versions logistic function was 
   * the only one provided, you can convert those files by the command 
   * echo 0 > newfile.net; cat oldfile.net >> newfile.net
   *
   * 
   * Starting from version 0.88 textload method provides a solution to this
   * problem:
   * if the first number in text file is >= 2 it is interpreted as the number
   * of layers and the function is set to logistic.
   */

  /*!\brief Write a network to a stdout.
   */
  void print () const;



  /*!\brief Write a network to a text file.
   * \param filename Pointer to name of file to write to.
   *
   * If it is impossible to write on the file throws a 
   * runtime_error exception.
   */
  void textsave (const char *filename) const;


  /*!\brief Read a network from a text file.
   * \param filename Pointer to name of file to read from.
   *
   * If filename does not exist throws a runtime_error exception
   */
  void textload (const char *filename);


  /****************************************
   * Errors
   *
   * Before calling these routines, compute() should have been called to
   * compute the ouputs for a given input. This routine compares the
   * actual output of the neural network (which is stored internally in
   * the neural network) and the intended output (in target).
   *
   ****************************************/

  /*!\brief Compute the output error of a network.
   * \param target Pointer to a sequence of floating point numbers.
   * \return Output error of the neural network.
   *
   * The return value is the square of the Euclidean distance between the 
   * actual output and the target. This routine also prepares the network
   * for  backpropagation training by storing (internally in the neural
   * network) the errors associated with each of the outputs. */
  float compute_output_error (const float *target);


  /*!\brief Compute the average error of a network
   * \param target Pointer to a sequence of floating point numbers.
   * \return Average error of the neural network.
   *
   * The average error is defined as the average value of absolute
   * differences between output and target
   */
  float compute_average_error (const float *target) const;


  /*!\brief Compute the quadratic error a network
   * \param target Pointer to a sequence of floating point numbers.
   * \return Quadratic error of the neural network.
   *
   * The quadratic error is defined as 
   * sqrt(sum ( T_j - O_j )^2) / N where T_j are targets and O_j are outputs
   */
  float compute_quadratic_error (const float *target) const;


  /*!\brief Compute the max error a network
   * \param target Pointer to a sequence of floating point numbers.
   * \return Maximum error of the neural network.
   *
   * The maximum error is defined as the maximum of absolute differences
   * between outputs and targets.
   */
  float compute_max_error (const float *target) const;


  /****************************************
   * Evaluation and Training
   ****************************************/

  /*!\brief Compute outputs of a network for given inputs.
   * \param input Pointer to sequence of floating point numbers.
   * \param output Pointer to sequence of floating point numbers or NULL.
   *
   * Compute outputs of a neural network for given inputs by forward
   * propagating the inputs through the layers. If output is non-NULL, the
   * outputs are copied to output (otherwise they are only stored
   * internally in the network).
   */
  void compute (const float *input, float *output);

  /*!\brief Train a network.
   *
   * Before calling this routine, compute() and
   * compute_output_error() should have been called to compute outputs
   * for given inputs and to prepare the neural network for training by
   * computing the output error. This routine performs the actual training
   * by backpropagating the output error through the layers.
   */
  void train ();


  /****************************************
   * SuperSab
   ****************************************/


  /*!\brief True if ssab is active
     \return true if supersab mode is active, false otherwise.
   */
  bool is_ssab_active () const;

  /*! \brief Count the number of weights of the network
   *  \return  number of weights
   */
  int count_weights () const;

  /*!\brief Begin SuperSab mode setting the nus to learning rate of the 
   *        network
   *
   * Precondition: (! is_ssab_active()) i.e. begin_ssab was not called before.
   *
   * If is_ssab_active() and you want to reset the values of nus, use 
   * reset_ssab() or if you want to free memory used for SuperSab, use
   * free_ssab()
   * \return  -1 on failure, number of weights of the net otherwise.
   */
  int begin_ssab ();

  /*!\brief Train a network in ssab mode
   *
   * Before calling this routine, begin_ssab() should have been called to
   * begin SuperSab training.
   *
   * Furthermore, for the current input/output pair,  compute() and
   * compute_output_error() should have been called to compute outputs
   * for given inputs and to prepare the neural network for training by
   * computing the output error. This routine performs the actual training
   * by backpropagating the output error through the layers and changing 
   * the weights.
   *           
   * The better way to use SuperSab is in combination with batch training,
   * using train_batch() for the training and end_batch_ssab() at the end of 
   * every epoch. 
   */
  void train_ssab ();

  /*!\brief Reset the values of learning rates of the network to learning_rate
   * in SuperSab mode. 
   *
   * Precondition: is_ssab_active()
   * \return int -1 on failure (SuperSab mode is not active), the number of 
   *         weights of the network otherwise.
   */
  int reset_ssab ();


  /*!\brief Free the memory used for SuperSab and end SuperSab mode
   *
   * After the call of free_ssab, the values of learning rates are 
   * lost and SuperSab mode is off.
   */
  void free_ssab ();


  /*!\brief Write SuperSab learning rates to a binary file.
   * \param filename Pointer to name of file to write to.
   * \return true on success, false on failure.
   */
  bool save_ssab (const char *filename) const;


  /*!\brief Load SuperSab learning rates from a binary file.
   * \param filename Pointer to name of file to read from.
   * \return true on success, false on failure.
   */
  bool load_ssab (const char *filename);


  /*!\brief Print learning rates for SuperSab mode
   * \return number of weights in the network, 
   * -1 if SSab mode is not active
   */
  int ssab_print_nus () const;


/*!\brief Make some statistics about learning rates in SuperSab mode.
 * \return -1 if SuperSab mode is not active, number of weights of the network otherwise
 * \param average the average of learning rates
 * \param max the max value of learning rates
 * \param min the min value of learning rates
 * \param n_max number of learning rates equal to max
 * \param n_min number of learning rates equal to min
 */
  int ssab_stats(float& average, float &max, float &min, int &n_max, int &n_min);

  /****************************************
   * Batch Training
   ****************************************/


  /*!\brief Begin training in batch mode.
   */
  void begin_batch ();

  /*!\brief Train a network in batch mode.
   *
   * Before calling this routine, begin_batch() should have been
   * called (at the start of the batch) to begin batch training.
   * Furthermore, for the current input/target pair, compute() and
   * compute_output_error() should have been called to compute outputs
   * for given the inputs and to prepare the neural network for training
   * by computing the output error using the given targets. This routine
   * performs the actual training by backpropagating the output error
   * through the layers, but does not change the weights. The weights
   * will be changed when (at the end of the batch) end_batch()  
   * (or end_batch_ssab()) is  called.
   */
  void train_batch ();


  /*!\brief End training in batch mode adjusting weights.
   *
   * Adjust the weights in the neural network according to the average 
   * delta of all patterns in the batch.
   */
  void end_batch ();


  /*!\brief End training in batch mode adjusting weights with SuperSab.
   *
   *
   * Adjust the weights in the neural network according to the average 
   * delta of all patterns in the batch and with SuperSab.
   *
   * For using SuperSab  mode in batch training you should call once 
   * begin_ssab(), then begin_batch() at the beginning of every epoch, 
   * train the network with train_batch() and then 
   * call end_batch_ssab() at the end of every epoch.
   */
  void end_batch_ssab ();

  /****************************************
   * Modification
   ****************************************/

  /*!\brief Make small random changes to the weight of a network.
   * \param factor Floating point number.
   * \param range Floating point number.
   *
   * All weights in the neural network that are in absolute value smaller
   * than range become a random value from the interval [-range,range].
   * All other weights get multiplied by a random value from the interval
   * [1-factor,1+factor].
   */
  void jolt (float factor, float range);

  /****************************************
   * Overloaded operators
   ****************************************/

  /*! \brief Overloaded operator= 
   */
  const network & operator= (const network & b);


  /* PRIVATE */

private:
  /* [Internal]
   * Structs for neurons and layers 
   */

  typedef struct
  {
    float output;
    float error;
    float *weight;
    float *delta;
    float *sumdeltas;
  }
  neuron_t;

  typedef struct
  {
    int no_of_neurons;
    neuron_t *neuron;
  }
  layer_t;


  void reset_deltas ();
  void reset_sumdeltas ();
  void reset_deltas_and_sumdeltas ();

  void allocate_layer (layer_t * layer, int no_of_neurons);

  void allocate_weights (layer_t * lower, layer_t * upper);
  void allocate_l (int act, int layers, const int *arglist);

  void fbprint (FILE * file) const;
  void fbscan (FILE * file);

  void do_load (const char *filename);
  void do_textload (const char *filename);

  void fprint (FILE * file) const;

  void fscan (FILE * file);

  void set_input (const float *input);

  void get_output (float *output);

  static float sigmoidal (float x, int num_func);
  void propagate_layer (layer_t * lower, layer_t * upper);
  void forward_pass ();
  void backpropagate_layer (layer_t * lower, layer_t * upper);

  void backward_pass ();
  void adjust_weights ();

  void adjust_weights_ssab ();

  int fprint_ssab (FILE * file) const;

  int fscan_ssab (FILE * file);


  void adjust_sumdeltas_batch ();

  void adjust_weights_batch ();

  void adjust_weights_batch_ssab ();
  void copy (const network & b);
  void destroy ();


  int no_of_layers;
  float momentum;
  float learning_rate;
  float global_error;
  int no_of_patterns;
  layer_t *layer;
  layer_t *input_layer;
  layer_t *output_layer;
  int activation;
  float *nus;

  float maxnu;
  float minnu;
  float nuup;
  float nudown;

  /* operator<< is declared friend because it needs to access private fields */
  friend std::ostream & operator<< (std::ostream &, const network &);

};

/*! \brief Write a network on a stream  
 *
 * Same format as friendly_print() (friendly_print(false) i.e. weights are
 * not displayed)
 *
 * Usage:
 * os << net;
 */
std::ostream & operator<< (std::ostream & os, const network & net);


/****************************************
 * IMPLEMENTATION OF INLINE FUNCTIONS
 * ACCESSORS AND MUTATORS
 ****************************************/


/****************************************
 * Accessors
 ****************************************/

/*!\brief Retrieve the momentum of a network.
 * \return Momentum of the neural work.
 */
inline float
network::get_momentum () const
{
  return momentum;
}

/*!\brief Retrieve the momentum of a network.
 * \return Learning rate of the neural work.
 */
inline float
network::get_learning_rate () const
{
  return learning_rate;
}

/*!\brief Retrieve the number of inputs of a network.
 * \return Number of neurons in the input layer of the neural network.
 */
inline int
network::get_no_of_inputs () const
{
  return input_layer->no_of_neurons;
}

/*!\brief Retrieve the number of outputs of a network.
 * \return Number of neurons in the output layer of the neural network.
 */
inline int
network::get_no_of_outputs () const
{
  return output_layer->no_of_neurons;
}

/*!\brief Retrieve the number of layers of a network.
 * \return Number of layers, including the input and output layers, of the 
 * neural network.
 */
inline int
network::get_no_of_layers () const
{
  return no_of_layers;
}

/*!\brief Retrieve the number of patterns in batch training
 * \return number of patterns
 */
inline int
network::get_no_of_patterns () const
{
  return no_of_patterns;
}


/*!\brief Retrieve the activation function of network (network::LOGISTIC or network::TANH)
 * \return activation function
 */
inline int
network::get_activation () const
{
  return activation;
}

/*!\brief Retrieve the output error of a network.
 * \return Output error of the neural network.
 *
 * Before calling this routine, compute() and
 * compute_output_error() should have been called to compute outputs
 * for given inputs and to acually compute the output error. This
 * routine merely returns the output error (which is stored internally
 * in the neural network).
 */
inline float
network::get_output_error () const
{
  return global_error;
}

/*!\brief True if ssab is active
 *\return true if supersab mode is active, false otherwise.
 */
inline bool 
network::is_ssab_active () const
{
  return (nus != NULL);
}



/*!\brief Retrieve maximum learning rate allowed in SuperSab mode 
 * \return float maximum learning rate
 *
 * Values of learning rates cannot be greater than this value
 */
inline float
network::get_max_learning_rate ()
{
  return maxnu;
}

  /*!\brief Retrieve minimum learning rate allowed in SuperSab mode 
   * \return float minimum learning rate
   *
   * Values of learning rates cannot be lesser than this value
   */
inline float
network::get_min_learning_rate ()
{
  return minnu;
}



/*!\brief Retrieve factor for increasing learning rate in SuperSab mode
 * \return float factor for increasing learning rate
 *
 * In SuperSab mode: if delta at this step has the same sign of delta at
 * the previous step, the learning rate of that weight is multiplied by
 * this value
 */
inline float
network::get_ssab_up_factor ()
{
  return nuup;
}

/*!\brief Retrieve factor for decreasing learning rate in SuperSab mode
 * \return float factor for decreasing learning rate
 *
 * In SuperSab mode: if delta at this step has the opposite  sign of delta at
 * the previous step, the learning rate of that weight is multiplied by
 * this value
 */
inline float
network::get_ssab_down_factor ()
{
  return nudown;
}


/****************************************
 * Mutators
 ****************************************/

/*!\brief Change the learning rate of a network.
 * \param learning_rate Floating point number.
 */
inline void
network::set_learning_rate (float the_learning_rate)
{
  learning_rate = the_learning_rate;
}


/*!\brief Change the momentum of a network.
 * \param momentum Floating point number.
 */
inline void
network::set_momentum (float the_momentum)
{
  momentum = the_momentum;
}


/*!\brief Set factor for increasing learning rate in SuperSab mode
 * \param factor (for increasing learning rate)
 *
 * In SuperSab mode: if delta at this step has the same sign of delta at
 * the previous step, the learning rate of that weight is multiplied by
 * this value ( should be factor > 1 )
 */
inline void
network::set_ssab_up_factor (float factor)
{
  nuup = factor;
}

/*!\brief Set factor for decreasing learning rate in SuperSab mode
 * \param factor (for decreasing learning rate)
 *
 * In SuperSab mode: if delta at this step has the opposite  sign of delta at
 * the previous step, the learning rate of that weight is multiplied by
 * this value ( should be 0 < factor < 1 )
 */
inline void
network::set_ssab_down_factor (float factor)
{
  nudown = factor;
}

#endif /* NETWORK_H */




