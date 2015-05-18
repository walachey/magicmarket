/*
 * lwneuralnet++ : C++ implementation of class network
 * By Luca Cinti and Lorenzo Masetti, based on 
 * lightweight neural net library written in C by
 * Peter van Rossum.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 */
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include <time.h>

#include <sstream>
#include <string>

#include "network.h"

namespace ANN
{
using ::std::vector;
using ::std::ostream;
using ::std::runtime_error;
using ::std::string;
using ::std::cerr;
using ::std::stringstream;
using ::std::endl;

/****************************************
 * Compile-time options
 ****************************************/
/*!\brief [Internal] Default momentum of a network.
 */
#define DEFAULT_MOMENTUM 0.0
/*!\brief [Internal] Default learning rate of a network (for logistic and tanh)
 */
#define DEFAULT_LEARNING_RATE_LOGISTIC 0.3
#define DEFAULT_LEARNING_RATE_TANH 0.1


/*!\brief [Internal] Default range with which weights of a network will be
 * initialized.
 */
#define RANDOM_LARGE_WEIGHTS_LOGISTIC 0.1
#define RANDOM_LARGE_WEIGHTS_TANH 0.05
/*! 
  \brief [Internal] default Max value for a learning rate in SuperSab mode
  \brief [Internal] default Min value for a learning rate in SuperSab mode
  \brief [Internal] default nu_up in SuperSab mode
  \brief [Internal] default nu_down in SuperSab mode
 */
#define DEFAULT_MAX_NU 30
#define DEFAULT_MIN_NU 0.00000001
#define DEFAULT_NUUP 1.2
#define DEFAULT_NUDOWN 0.8
#define min(X, Y)  ((X) < (Y) ? (X) : (Y))
#define max(X, Y)  ((X) > (Y) ? (X) : (Y))

/****************************************
 * Initialization
 ****************************************/

/*!\brief Assign random values to all weights in the network.
 * \param range Floating point number.
 *
 * All weights in the neural network are assigned a random value
 * from the interval [-range, range].
 */
void
network::randomize (float range)
{
  int l, nu, nl;
  srand (time (0));
  range = 2.0 * range;
  for (l = 1; l < no_of_layers; l++)
    {
      for (nu = 0; nu < layer[l].no_of_neurons; nu++)
	{
	  for (nl = 0; nl < layer[l - 1].no_of_neurons; nl++)
	    {
	      layer[l].neuron[nu].weight[nl] =
		 range * ((float) rand () / RAND_MAX - 0.5);
	    }
	}
    }
}

#if 0
/*!\brief Set weights of the network to 0.
 */
void
network::reset_weights ()
{
  int l, nu, nl;

  for (l = 1; l < no_of_layers; l++)
    {
      for (nu = 0; nu < layer[l].no_of_neurons; nu++)
	{
	  for (nl = 0; nl < layer[l - 1].no_of_neurons; nl++)
	    {
	      layer[l].neuron[nu].weight[nl] = 0.0;
	    }
	}
    }
}
#endif

/*!\brief[Internal] Set deltas of the network to 0.
 */
void
network::reset_deltas ()
{
  int l, nu, nl;

  for (l = 1; l < no_of_layers; l++)
    {
      for (nu = 0; nu < layer[l].no_of_neurons; nu++)
	{
	  for (nl = 0; nl < layer[l - 1].no_of_neurons; nl++)
	    {
	      layer[l].neuron[nu].delta[nl] = 0.0;
	    }
	}
    }
}

/*!\brief[Internal] Set sumdeltas of the network to 0.
 */
void
network::reset_sumdeltas ()
{
  int l, nu, nl;

  for (l = 1; l < no_of_layers; l++)
    {
      for (nu = 0; nu < layer[l].no_of_neurons; nu++)
	{
	  for (nl = 0; nl < layer[l - 1].no_of_neurons; nl++)
	    {
	      layer[l].neuron[nu].sumdeltas[nl] = 0.0;
	    }
	}
    }
}


/*!\brief[Internal] Set deltas and sumdeltas of the network to 0.
 */
void
network::reset_deltas_and_sumdeltas ()
{
  int l, nu, nl;

  for (l = 1; l < no_of_layers; l++)
    {
      for (nu = 0; nu < layer[l].no_of_neurons; nu++)
	{
	  for (nl = 0; nl < layer[l - 1].no_of_neurons; nl++)
	    {
	      layer[l].neuron[nu].delta[nl] = 0.0;
	      layer[l].neuron[nu].sumdeltas[nl] = 0.0;
	    }
	}
    }
}

/****************************************
 * Memory Management
 ****************************************/

/*!\brief [Internal] Allocate memory for the neurons in a layer of a network.
 * \param layer Pointer to layer of a neural network.
 * \param no_of_neurons Integer.
 *
 * Allocate memory for a list of no_of_neuron neurons in the specified
 * layer.
 */
void
network::allocate_layer (layer_t * layer, int no_of_neurons)
{
  if (no_of_neurons<1) {
    throw runtime_error("A layer must have at least one neuron");
  }
  layer->no_of_neurons = no_of_neurons;
  layer->neuron = (neuron_t *) calloc (no_of_neurons, sizeof (neuron_t));
}

/*!\brief [Internal] Allocate memory for the weights connecting two layers of a network.
 * \param lower Pointer to one layer of a neural network.
 * \param upper Pointer to the next layer of a neural network.
 *
 * Allocate memory for the weights connecting two layers of a neural
 * network. The neurons in these layers should previously have been
 * allocated with allocate_layer().
 */
void
network::allocate_weights (layer_t * lower, layer_t * upper)
{
  int n;

  for (n = 0; n < upper->no_of_neurons; n++)
    {
      upper->neuron[n].weight =
	(float *) calloc (lower->no_of_neurons, sizeof (float));
      upper->neuron[n].delta =
	(float *) calloc (lower->no_of_neurons, sizeof (float));
      upper->neuron[n].sumdeltas =
	(float *) calloc (lower->no_of_neurons, sizeof (float));
    }
}

/*!\brief[Internal] Allocate memory for a network.
 * \param act (network::LOGISTIC or network::TANH)
 * \param layers Integer.
 * \param arglist Pointer to sequence of integers.
 *
 * Allocate memory for a neural network with no_of_layer layers,
 * including the input and output layer. The number of neurons in each
 * layer is given in arglist, with arglist[0] being the number of
 * neurons in the input layer and arglist[no_of_layers-1] the number of
 * neurons in the output layer.
 */
void
network::allocate_l (int act, int layers, const int *arglist)
{
  int l;

  /* sanity check */
  if (layers < 2)
    {
      throw runtime_error ("A network must have at least two layers");
    }

  /* allocate memory for the network */
  no_of_layers = layers;
  layer = (layer_t *) calloc (no_of_layers, sizeof (layer_t));
  for (l = 0; l < no_of_layers; l++)
    {
      allocate_layer (&layer[l], arglist[l]);
    }
  for (l = 1; l < no_of_layers; l++)
    {
      allocate_weights (&layer[l - 1], &layer[l]);
    }

  /* abbreviations for input and output layer */
  input_layer = &layer[0];
  output_layer = &layer[no_of_layers - 1];

  /* default values for network constants */
  momentum = DEFAULT_MOMENTUM;
  learning_rate = ((act==NET_LOGISTIC)?DEFAULT_LEARNING_RATE_LOGISTIC:DEFAULT_LEARNING_RATE_TANH);

  /* Set the activation function */
  activation = act;
  
  /* default parameters for SuperSab training */
  nus = NULL;
  maxnu = DEFAULT_MAX_NU;
  minnu = DEFAULT_MIN_NU;
  nuup = DEFAULT_NUUP;
  nudown = DEFAULT_NUDOWN;

  /* default (meaningless) values for other fields */
  no_of_patterns = 0;
  global_error = 0.0;

  /* initialize weights and deltas */
  randomize ((act == NET_LOGISTIC)?RANDOM_LARGE_WEIGHTS_LOGISTIC:RANDOM_LARGE_WEIGHTS_TANH);
  reset_deltas_and_sumdeltas ();
}


/*!\brief Constructor for a network 
 * \param activ network::LOGISTIC or network::TANH
 * \param layers Integer.
 * \param ... Sequence of integers.
 *
 * Allocate memory for a neural network with no_of_layer layers,
 * including the input and output layer. The number of neurons in each
 * layer is given as ..., starting with the input layer and ending with
 * the output layer.
 */
network::network (int activ, int layers, ...)
{
  int l, *arglist;
  va_list args;

  /* sanity check */
  if (layers < 2)
    {
      throw runtime_error ("A network must have at least two layers");
    }
  no_of_layers = layers;


  arglist = (int *) calloc (no_of_layers, sizeof (int));
  /* the list of var. length starts from parameter layers */
  va_start (args, layers);
  for (l = 0; l < no_of_layers; l++)
    {
      arglist[l] = va_arg (args, int);
    }
  va_end (args);
  allocate_l (activ, no_of_layers, arglist);
  free (arglist);
}

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
 * layers.size() < 2 throws a runtime_error exception
 */
network::network (int activ, vector<int> layers) {
 /* sanity check */
  if (layers.size() < 2)
    {
      throw runtime_error ("A network must have at least two layers");
    }
  no_of_layers = layers.size();

  int* arglist = (int *) calloc (no_of_layers, sizeof (int));
  
  for (int l = 0; l<no_of_layers; l++) {
    arglist[l] = layers[l];
  }

  allocate_l (activ, no_of_layers, arglist);
  free (arglist);
}


/*!\brief Constructor. Load network from a binary file.
 * If filename does not exist throws an exception
 */
network::network (const char *filename, bool binary)
{
  if (binary)
    {
      do_load (filename);
    }
  else
    {
      do_textload (filename);
    }
}

/*!\brief Destructor. Free memory allocated for a network.
 */
network::~network ()
{
  destroy ();
}

/*!\brief Code for the destructor
 */
void
network::destroy ()
{
  int l, n;

  for (l = 0; l < no_of_layers; l++)
    {
      if (l != 0)
	{
	  for (n = 0; n < layer[l].no_of_neurons; n++)
	    {
	      free (layer[l].neuron[n].weight);
	      free (layer[l].neuron[n].delta);
	      free (layer[l].neuron[n].sumdeltas);
	    }
	}
      free (layer[l].neuron);
    }
  free (layer);
  free (nus);
}

/********************************
 * Accessors and mutators (not inline)
 ********************************/

/*!\brief Retrieve number of neurons on a layer of a network
  * \param l layer index 
  * \return number of neurons on layer l
  */
int
network::get_no_of_neurons (int l) const
{
  /* sanity check */
  if ((l < 0) || (l >= no_of_layers))
    {
      return 0;
    }

  return layer[l].no_of_neurons;
}


/*!\brief Retrieve a weight of a network.
 * \param l Number of lower layer.
 * \param nl Number of neuron in the lower layer.
 * \param nu Number of neuron in the next layer.
 * \return Weight connecting the neuron numbered nl in the layer
 * numbered l-1 with the neuron numbered nu in the layer numbered l.
 */
float
network::get_weight (int l, int nl, int nu) const
{
  if ((l <= 0) || (l >= no_of_layers)) {
    return 0;
  }
  if ((nu<0) || (nu >= layer[l].no_of_neurons)) {
    return 0;
  }

  if ((nl<0) || (nl >= layer[l-1].no_of_neurons)) {
    return 0;
  }

  return layer[l].neuron[nu].weight[nl];
}

void
network::set_weight(int l, int nl, int nu, float weight)
{
	if ((l <= 0) || (l >= no_of_layers)) {
		return;
	}
	if ((nu<0) || (nu >= layer[l].no_of_neurons)) {
		return;
	}

	if ((nl<0) || (nl >= layer[l - 1].no_of_neurons)) {
		return;
	}

	layer[l].neuron[nu].weight[nl] = weight;
}

/*!\brief Set activation function of the network.
 * \param num_func Number of function (network::LOGISTIC | network::TANH)
 */
void
network::set_activation (int num_func)
{
  if (num_func != NET_LOGISTIC && num_func != NET_TANH)
    {
      activation = NET_LOGISTIC;
      fprintf (stderr,
	       "network: Warning. Impossible to set activation function number %d \n",
	       num_func);
      fprintf (stderr, "network: Warning. Activation function is network::LOGISTIC");
    }
  else
    {
      activation = num_func;
    }
}



/****************************************
 * File I/O for binary files
 ****************************************/

/*!\brief[Internal] Write a network to a file.
 * \param file Pointer to file descriptor.
 */
void
network::fbprint (FILE * file) const
{
  int l, nu;

  /* write activation funct. and no. of layers */
  fwrite(&activation, sizeof(int), 1, file);
  fwrite(&no_of_layers, sizeof(int), 1, file);

  /* write network dimensions */
  for (l = 0; l < no_of_layers; l++)
    {
      fwrite(&(layer[l].no_of_neurons), sizeof(int), 1, file);
    }

  float constants[3];
  /* write network constants */
  constants[0] = momentum;
  constants[1] = learning_rate;
  constants[2] = global_error;


  fwrite (constants, sizeof (float), 3, file);

  /* write network weights */
  for (l = 1; l < no_of_layers; l++)
    {
      for (nu = 0; nu < layer[l].no_of_neurons; nu++)
	{
	  fwrite (layer[l].neuron[nu].weight, sizeof (float),
		  layer[l - 1].no_of_neurons, file);
	}
    }
}


/*!\brief[Internal] Read a network from a binary file.
 * \param file Pointer to a file descriptor.
 */
void
network::fbscan (FILE * file)
{
  int no_of_layers, l, nu, funct, *arglist;

  int read_ok;

  /* read function */
  read_ok =  fread (&funct, sizeof (int), 1, file);

  if ( read_ok == 0) {
    throw runtime_error("network: No enough information on file");
  }
  /* tricky solution for importing files with old format 
   * (used by C - lwneuralnet) 
   * without the number of the function at the beginning:
   * Function number can be 0 or 1 while number of layers 
   * must be >= 2. So if funct>1 the file should be in 
   * old format and we can set funct = 0 (logistic) and
   * take the first number as no_of_layers
   */ 
  if (funct > NET_TANH) {
    fprintf(stderr, "network: Warning importing file of old lwneuralnet format.\n\n");
    no_of_layers = funct;
    funct = NET_LOGISTIC;
  } else {
    /* read network dimensions */
    read_ok = fread (&no_of_layers, sizeof (int), 1, file);
    if ( read_ok == 0) {
      throw runtime_error("network: No enough information on file");
    }
  }

  if (no_of_layers<2) {
      throw runtime_error ("Error in network file format. Number of layers must be at least 2.");
  }

  arglist = (int *) calloc (no_of_layers, sizeof (int));
  if (arglist == NULL)
    {
      throw runtime_error ("Error in network file format");
    }

  if (fread (arglist, sizeof (int), no_of_layers, file) < ((size_t) no_of_layers)) {
    free(arglist);
    throw runtime_error ("Error in network file format. No information about layers");
  };

  /* allocate memory for the network */
  allocate_l (funct, no_of_layers, arglist);
  free (arglist);

  /* read network constants */
  read_ok = fread (&momentum, sizeof (float), 1, file) &&
    fread (&learning_rate, sizeof (float), 1, file) &&
    fread (&global_error, sizeof (float), 1, file);

  if ( read_ok == 0) {
    throw runtime_error("network: No enough information on file");
  }

  set_activation (funct);

  read_ok = 0;
  /* read network weights */
  for (l = 1; l < no_of_layers; l++)
    {
      for (nu = 0; nu < layer[l].no_of_neurons; nu++)
	{
	  read_ok += fread (layer[l].neuron[nu].weight, sizeof (float),
		 layer[l - 1].no_of_neurons, file);
	}
    }

  if (read_ok != count_weights()) {
    throw runtime_error("network: File too short");
  }
}


/*!\brief Write a network to a binary file.
 * \param filename Pointer to name of file to write to.
 */
void network::save (const char *filename) const
{
  FILE *
    file;

  file = fopen (filename, "w");
  if (file == NULL)
    {
      throw runtime_error("network: could not open " + string(filename) + " for writing");
    }
  fbprint (file);

  if (fclose (file) != 0) {
    throw runtime_error("network: error on saving to " + string(filename));
  }
}


/*!\brief Read a network from a binary file.
 * \param filename Pointer to name of file to read from.
 * If filename does not exist throws an exception
 */
void
network::load (const char *filename)
{
  destroy ();
  do_load (filename);
}

/*! \brief[Internal] Read a network from a binary file.
 * \param filename Pointer to name of file to read from.
 * If filename does not exist throws an exception
 */
void
network::do_load (const char *filename)
{
  FILE *file;

  file = fopen (filename, "r");
  if (file == NULL)
    {
      throw runtime_error ("network: File " + string (filename) + " not found ");
    }
  fbscan (file);
  fclose (file);
}


/****************************************
 * File I/O for Text Files
 ****************************************/

/*!\brief[Internal] Write a network to a file.
 * \param file Pointer to file descriptor.
 */
void
network::fprint (FILE * file) const
{
  int l, nu, nl;

  /* write function */
  fprintf (file, "%i\n", get_activation ());

  /* write network dimensions */
  fprintf (file, "%i\n", no_of_layers);
  for (l = 0; l < no_of_layers; l++)
    {
      fprintf (file, "%i\n", layer[l].no_of_neurons);
    }

  /* write network constants */
  fprintf (file, "%f\n", momentum);
  fprintf (file, "%f\n", learning_rate);
  fprintf (file, "%f\n", global_error);

  /* write network weights */
  for (l = 1; l < no_of_layers; l++)
    {
      for (nu = 0; nu < layer[l].no_of_neurons; nu++)
	{
	  for (nl = 0; nl < layer[l - 1].no_of_neurons; nl++)
	    {
	      fprintf (file, "%f\n", layer[l].neuron[nu].weight[nl]);
	    }
	}
    }

}

/*!\brief[Internal] Read a network from a file.
 * \param file Pointer to a file descriptor.
 */
void
network::fscan (FILE * file)
{
  int no_of_layers, l, nu, nl, funct, *arglist;
  int read_ok;
  /* read function */
  read_ok = fscanf (file, "%i", &funct);
  
  if ( read_ok == 0) {
    throw runtime_error("network: No enough information on file");
  }
  /* tricky solution for importing files with old format 
   * without the number of the function at the beginning:
   * Function number can be 0 or 1 while number of layers 
   * must be >= 2. So if funct>1 the file should be in 
   * old format and we can set funct = 0 (logistic) and
   * take the first number as no_of_layers
   */ 
  if (funct > NET_TANH) {
    no_of_layers = funct;
    funct = NET_LOGISTIC;
  } else {
    /* read number of layers */
    read_ok = fscanf (file, "%i", &no_of_layers);
    if ( read_ok == 0) {
      throw runtime_error("network: No enough information on file");
    }
  }

  if (no_of_layers < 2) {
    throw runtime_error("Error in text file format");
  }

  /* read number of neurons in each layer */
  arglist = (int *) calloc (no_of_layers, sizeof (int));
  if (arglist == NULL)
    {
      throw runtime_error ("Error in text file format");
    }
  read_ok = 0;
  for (l = 0; l < no_of_layers; l++)
    {
      read_ok += fscanf (file, "%i", &arglist[l]);
    }

  if ( read_ok < no_of_layers) {
    throw runtime_error("network: No enough information on file");
  }

  /* allocate memory for the network */
  allocate_l (funct, no_of_layers, arglist);

  /* read network constants */
  read_ok = fscanf (file, "%f", &momentum) &&
    fscanf (file, "%f", &learning_rate) &&
    fscanf (file, "%f", &global_error);

  if ( read_ok == 0) {
    throw runtime_error("network: No enough information on file");
  }

  read_ok = 0;
  /* read network weights */
  for (l = 1; l < no_of_layers; l++)
    {
      for (nu = 0; nu < layer[l].no_of_neurons; nu++)
	{
	  for (nl = 0; nl < layer[l - 1].no_of_neurons; nl++)
	    {
	      read_ok += fscanf (file, "%f", &layer[l].neuron[nu].weight[nl]);
	    }
	}
    }

  if ( read_ok < count_weights()) {
    throw runtime_error("network: Text file too short");
  }

  free (arglist);
}

/*!\brief Write a network to a stdout.
 */
void
network::print () const
{
  fprint (stdout);
}

/*!\brief Write a network to a stdout in a friendly format.
 * \param show If show==true weights are displayed
 */
void
network::friendly_print (const bool show) const
{
  int l, nu, nl;

  /* write network dimensions */
  printf ("No of layers: %i\n", no_of_layers);
  for (l = 0; l < no_of_layers; l++)
    {
      printf ("No of neurons on layer %i: %i\n", l, layer[l].no_of_neurons);
    }

  /* write network constants */
  printf ("Learning rate: %f\n", learning_rate);
  printf ("Momentum: %f\n", momentum);
  printf ("Global Error: %f\n", global_error);

  printf ("Activation Func: %s\n",
	  activation  == NET_LOGISTIC ? " Logistic" : "Tanh");
  if (is_ssab_active()) {
    printf("SuperSab mode is active.");
    printf("Max Learning rate: %f\n", maxnu);
    printf("Min Learning rate: %f\n", minnu);
    printf("nu_up (factor for increasing): %f\n",nuup);
    printf("nu_down (factor for decreasing): %f\n",nudown);
  }

  if (show)
    {
      printf ("Weights: \n\n");
      /* write network weights */
      for (l = 1; l < no_of_layers; l++)
	{
	  printf ("Weights from layer %d to %d\n", l - 1, l);
	  for (nu = 0; nu < layer[l].no_of_neurons; nu++)
	    {
	      for (nl = 0; nl < layer[l - 1].no_of_neurons; nl++)
		{
		  printf ("W(%d,%d) = %f\n", nl, nu,
			  layer[l].neuron[nu].weight[nl]);
		}
	    }
	}
    }
}


void network::export_to_graphviz(const char* filename) const {
  FILE* out;
  out = fopen(filename, "w");
  if (out == NULL)
    {
      throw runtime_error("network: could not open " + string(filename) + " for writing");
    }

  fprintf(out,"# FILE FOR GRAPHVIZ generated by Lightweight Neural Network\n");
  fprintf(out,"# To get the picture download package graphviz from http://www.graphviz.org\n\n");
  fprintf(out,"# Example: to get a PNG image use the command:\n");
  fprintf(out,"# dot %s -Tpng -o %s.png\n\n", filename, filename); 
  fprintf(out,"digraph neural_network {\n");
  fprintf(out,"size=\"2\";\n");

  for (int l = 0; l<no_of_layers; l++) {
    for (int n=0; n<layer[l].no_of_neurons; n++) {
      fprintf (out,"\t n%d_%d [label=\"%d\"];\n", l,n,n);
    } 
  }

  int nu, nl;
  for (int l = 1; l < no_of_layers; l++)
    {
      fprintf (out,"subgraph g%d_%d {\n", l - 1, l);
	  for (nl = 0; nl < layer[l - 1].no_of_neurons; nl++)
	    {
	      for (nu = 0; nu < layer[l].no_of_neurons; nu++)
		{

		  fprintf (out,"\t n%d_%d -> n%d_%d [label=%4.2f]\n", l-1, nl, l , nu, 
			   layer[l].neuron[nu].weight[nl]);
		}
	    }
      fprintf(out,"}\n");
    }
  fprintf(out,"}");
  if (fclose (out) != 0) {
    throw runtime_error("network: error on saving to " + string(filename));
  }
}


/*!\brief Write a network to a text file.
 * \param filename Pointer to name of file to write to.
 */
void network::textsave (const char *filename) const
{
  FILE *
    file;

  file = fopen (filename, "w");
  if (file == NULL)
    {
      throw runtime_error("network: could not open " + string(filename) + " for writing");
    }
  fprint (file);

  if (fclose (file) != 0) {
    throw runtime_error("network: error on saving to " + string(filename));
  }

}

/*!\brief Read a network from a text file.
 * \param filename Pointer to name of file to read from.
 * If filename does not exist throws a runtime_error exception
 */
void
network::textload (const char *filename)
{
  destroy ();
  do_textload (filename);
}


/*!\brief[Internal] Read a network from a text file.
 * \param filename Pointer to name of file to read from.
 * If filename does not exist throws a runtime_error exception
 */
void
network::do_textload (const char *filename)
{
  FILE *file;

  file = fopen (filename, "r");
  if (file == NULL)
    {
      throw runtime_error ("File " + string (filename) + " not found or could not be opened for reading");
    }
  fscan (file);
  fclose (file);
}

/****************************************
 * Input and Output
 ****************************************/

/*!\brief [Internal] Copy inputs to input layer of a network.
 */
void
network::set_input (const float *input)
{
  int n;

  for (n = 0; n < input_layer->no_of_neurons; n++)
    {
      input_layer->neuron[n].output = input[n];
    }
}

/*!\brief [Interal] Copy outputs from output layer of a network.
 */
void
network::get_output (float *output)
{
  int n;

  for (n = 0; n < output_layer->no_of_neurons; n++)
    {
      output[n] = output_layer->neuron[n].output;
    }
}

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
float
network::compute_output_error (const float *target)
{
  int n;
  float output, error;
  global_error = 0.0;
  for (n = 0; n < output_layer->no_of_neurons; n++)
    {
      output = output_layer->neuron[n].output;
      error = target[n] - output;
      if (activation == NET_LOGISTIC)
	{
	  output_layer->neuron[n].error = output * (1 - output) * error;
	}
      else
	{
	  output_layer->neuron[n].error = (1 - output * output) * error;
	}
      global_error += error * error;
    }
  return global_error;
}

/*!\brief Compute the average error of a network
 * \param target Pointer to a sequence of floating point numbers.
 * \return Average error of the neural network.
 *
 * The average error is defined as the average value of absolute
 * differences between output and target
 */
float
network::compute_average_error (const float *target) const
{
  int n;
  float output;
  float error = 0.0;
  for (n = 0; n < output_layer->no_of_neurons; n++)
    {
      output = output_layer->neuron[n].output;
      error += fabs (target[n] - output);
    }
  return error / output_layer->no_of_neurons;
}

/*!\brief Compute the quadratic error a network
 * \param target Pointer to a sequence of floating point numbers.
 * \return Quadratic error of the neural network.
 *
 * The quadratic error is defined as 
 * sqrt(sum ( T_j - O_j )^2) / N where T_j are targets and O_j are outputs
 */
float
network::compute_quadratic_error (const float *target) const
{
  int n;
  float output;
  float error = 0.0;
  for (n = 0; n < output_layer->no_of_neurons; n++)
    {
      output = output_layer->neuron[n].output;
      error += (target[n] - output) * (target[n] - output);
    }
  return sqrt (error) / output_layer->no_of_neurons;
}

/*!\brief Compute the max error a network
 * \param target Pointer to a sequence of floating point numbers.
 * \return Maximum error of the neural network.
 *
 * The maximum error is defined as the maximum of absolute differences
 * between outputs and targets.
 */
float
network::compute_max_error (const float *target) const
{
  int n;
  float output;
  float error = 0.0;
  for (n = 0; n < output_layer->no_of_neurons; n++)
    {
      output = output_layer->neuron[n].output;
      error = max (error, fabs (target[n] - output));
    }
  return error;
}

/****************************************
 * Sigmoidal functions
 ****************************************/

#if 0
/* THIS IS ONLY FOR REFERENCE !! 
 *  Sigmoidal function is implemented with table lookup!
 */

/* reference implementation for sigmoidal */
/*!\brief [Internal] Activation function of a neuron.
 * \param x point where function should be evaluated
 * \param num_func type of sigmoidal function (network::LOGISTIC, network::TANH)
 */
inline float
network::sigmoidal (float x, int num_func)
{
// IT'S NOT REALLY IMPLEMENTED THIS WAY!!
  if (num_func == network::LOGISTIC)
    {
      return 1.0 / (1.0 + exp (-x)); 
    }
  else if (num_function = network::TANH)
    {
      return tanh (x);
    }
}
#else
/* implementation of sigmoidal with table lookup */
#include "sigmoidal.cc"

/*!\brief [Internal] Activation function of a neuron.
 * \param x point where function should be evaluated
 * \param num_func type of sigmoidal function (network::LOGISTIC, network::TANH)
 */
inline float
network::sigmoidal (float x, int num_func)
{
  int index = (int) ((x - min_entry[num_func]) * invinterval[num_func]);
  if (index <= 0)
    {
      return lowbound[num_func];
    }
  else if (index >= num_entries)
    {
      return highbound[num_func];
    }
  else
    {
      return interpolation[index][num_func];
    }
}
#endif



/****************************************
 * Forward and Backward Propagation
 ****************************************/

/*!\brief [Internal] Forward propagate inputs from one layer to next layer.
 */
void
network::propagate_layer (layer_t * lower, layer_t * upper)
{
  int nu, nl;
  float value;

  for (nu = 0; nu < upper->no_of_neurons; nu++)
    {
      value = 0.0;
      for (nl = 0; nl < lower->no_of_neurons; nl++)
	{
	  value += upper->neuron[nu].weight[nl] * lower->neuron[nl].output;
	}
      upper->neuron[nu].output = sigmoidal (value, activation);
    }
}

/*!\brief [Internal] Forward propagate inputs through a network.
 */
void
network::forward_pass ()
{
  for (int l = 1; l < no_of_layers; l++)
    {
      propagate_layer (&layer[l - 1], &layer[l]);
    }
}

/*!\brief [Internal] Backpropagate error from one layer to previous layer.
 */
void
network::backpropagate_layer (layer_t * lower, layer_t * upper)
{
  int nl, nu;
  float output, error;

  for (nl = 0; nl < lower->no_of_neurons; nl++)
    {
      error = 0.0;
      for (nu = 0; nu < upper->no_of_neurons; nu++)
	{
	  error += upper->neuron[nu].weight[nl] * upper->neuron[nu].error;
	}
      output = lower->neuron[nl].output;
      if (activation == NET_LOGISTIC)
	{
	  lower->neuron[nl].error = output * (1 - output) * error;
	}
      else
	{
	  lower->neuron[nl].error = (1 - output * output) * error;
	}
    }
}

/*!\brief [Internal] Backpropagate output error through a network.
 */
void
network::backward_pass ()
{
  for (int l = no_of_layers - 1; l > 1; l--)
    {
      backpropagate_layer (&layer[l - 1], &layer[l]);
    }
}

/*!\brief [Internal] Adjust weights based on (backpropagated) output error.
 */
inline void
network::adjust_weights ()
{
  int l, nu, nl;
  float learning_factor, delta;

  for (l = 1; l < no_of_layers; l++)
    {
      for (nu = 0; nu < layer[l].no_of_neurons; nu++)
	{
	  learning_factor = learning_rate * layer[l].neuron[nu].error;
	  for (nl = 0; nl < layer[l - 1].no_of_neurons; nl++)
	    {
	      delta =
		 learning_factor *
		layer[l - 1].neuron[nl].output +
		momentum * layer[l].neuron[nu].delta[nl];
	      layer[l].neuron[nu].weight[nl] += delta;
	      layer[l].neuron[nu].delta[nl] = delta;
	    }
	}
    }
}

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
void
network::compute (const float *input, float *output)
{
  set_input (input);
  forward_pass ();
  if (output != NULL)
    {
      get_output (output);
    }
}

/*!\brief Train a network.
 *
 * Before calling this routine, compute() and
 * compute_output_error() should have been called to compute outputs
 * for given inputs and to prepare the neural network for training by
 * computing the output error. This routine performs the actual training
 * by backpropagating the output error through the layers.
 */
void
network::train ()
{
  backward_pass ();
  adjust_weights ();
}


/****************************************
 * SuperSab
 ****************************************/

/*!\brief [Internal] Adjust weights with SuperSAB
 */
void
network::adjust_weights_ssab ()
{
  int l, nu, nl;
  float error, delta;
  int nuind = 0;
  for (l = 1; l < no_of_layers; l++)
    {
      for (nu = 0; nu < layer[l].no_of_neurons; nu++)
	{
	  error = layer[l].neuron[nu].error;
	  for (nl = 0; nl < layer[l - 1].no_of_neurons; nl++)
	    {
	      delta =
		nus[nuind] * error * layer[l -
					   1].neuron[nl].output +
		momentum * layer[l].neuron[nu].delta[nl];

	      layer[l].neuron[nu].weight[nl] += delta;
	      if (layer[l].neuron[nu].delta[nl] * delta > 0)
		{
		  nus[nuind] = min (nus[nuind] * nuup, maxnu);
		}
	      else
		{
		  nus[nuind] = max (nus[nuind] * nudown, minnu);
		}
	      layer[l].neuron[nu].delta[nl] = delta;
	      nuind++;
	    }
	}
    }
}


/* !\brief Count the number of weights of the network net
 * \return int number of weights
 */
int
network::count_weights () const
{
  int nweights = 0;
  for (int l = 1; l < no_of_layers; l++)
    {
      nweights += layer[l - 1].no_of_neurons * layer[l].no_of_neurons;
    }
  return nweights;
}

/*!\brief Begin SuperSab mode setting the nus to learning rate of the 
 *        network
 * Precondition: (! is_ssab_active()) i.e. begin_ssab was not called before.
 * If is_ssab_active() and you want to reset the values of nus, use 
 * reset_ssab or free_ssab
 *
 * \return int -1 on failure, number of weights of the net otherwise.
 */
int
network::begin_ssab ()
{
  int i;
  int nw;
  if (nus != NULL)
    {
      return -1;
    }
  nw = count_weights ();
  nus = (float *) malloc (nw * sizeof (float));
  for (i = 0; i < nw; i++)
    {
      nus[i] = learning_rate;
    }
  return nw;
}


/*!\brief Train a network in ssab mode
 *
 * Before calling this routine, compute() and
 * compute_output_error() should have been called to compute outputs
 * for given inputs and to prepare the neural network for training by
 * computing the output error. 
 */
void
network::train_ssab ()
{
  backward_pass ();
  adjust_weights_ssab ();
}

/*!\brief Reset the values of learning rates of the network in SuperSab mode
 * \return int -1 on failure (SuperSab mode is not active), the number of 
 *         weights of the network otherwise.
 */
int
network::reset_ssab ()
{
  int i, nw;
  if (nus == NULL)
    {
      return -1;
    }
  nw = count_weights ();
  for (i = 0; i < nw; i++)
    {
      nus[i] = learning_rate;
    }
  return nw;
}

/*!\brief Free the memory used for SuperSab and end SuperSab mode
 */
void
network::free_ssab ()
{
  free (nus);
  nus = NULL;
}

/*!\brief Save SuperSab learning rates to a file.
 * \param file Pointer to file descriptor.
 * 
 * \return -1 on failure, number of weights of the net otherwise.
 */
int
network::fprint_ssab (FILE * file) const
{
  if (nus == NULL)
    {
      return -1;
    }
  int nw = count_weights ();
  fwrite (&nw, sizeof (int), 1, file);
  fwrite (nus, sizeof (float), nw, file);
  fwrite (&maxnu, sizeof(float), 1, file);
  fwrite (&minnu, sizeof(float), 1, file);
  fwrite (&nuup, sizeof(float), 1, file);
  fwrite (&nudown, sizeof(float), 1, file);
  return nw;
}


/*!\brief Read  SuperSab learning rates from a file.
 * \param file Pointer to file descriptor.
 * \return -1 on failure, number of weights of the net otherwise.
 */
int
network::fscan_ssab (FILE * file)
{
  int nw;
  if (nus != NULL)
    {
      return -1;
    }
  fread (&nw, sizeof (int), 1, file);

  int net_nw = count_weights();
  
  if (net_nw != nw) {
    cerr << "network:  Wrong number of weights in file";
    return -1;
  }

  nus = (float *) malloc (nw * sizeof (float));
  if (fread (nus, sizeof (float), nw, file) != ((size_t) nw)) {
    cerr << "network: Error in format of file ";
    free(nus);
    nus = NULL;
    return -1;
  }

  int flag = fread (&maxnu, sizeof(float), 1, file) && 
    fread (&minnu, sizeof(float), 1, file) &&
    fread (&nuup, sizeof(float), 1, file) &&
    fread (&nudown, sizeof(float), 1, file);
  
  /* If the file does not contain values of parameters (old format)
   * set them to the default ones 
   */
  if (! flag) {
    maxnu = DEFAULT_MAX_NU;
    minnu = DEFAULT_MIN_NU;
    nuup = DEFAULT_NUUP;
    nudown = DEFAULT_NUDOWN;
  }
  return nw;
}

/*!\brief Write SuperSab learning rates to a binary file.
 * \param filename Pointer to name of file to write to.
 * \return true on success, false on failure.
 */
bool network::save_ssab (const char *filename) const
{
  FILE *
    file;
  int
    nw;

  file = fopen (filename, "w");
  if (file == NULL)
    {
      return false;
    }
  nw = fprint_ssab (file);
  if (nw == -1)
    {
      fclose (file);
      return false;
    }
  return (fclose (file) == 0);
}


/*!\brief Load SuperSab learning rates from a binary file.
 * \param filename Pointer to name of file to read from.
 * \return true on success, false on failure.
 */
bool network::load_ssab (const char *filename)
{
  FILE *
    file;
  int
    nw;

  file = fopen (filename, "r");
  if (file == NULL)
    {
      return false;
    }
  nw = fscan_ssab (file);
  
  if (nw == -1)
    {
      cerr << "network: Error in reading " << filename << endl;
      nus = NULL;
      fclose (file);
      return false;
    }
  return (fclose (file) == 0);
}



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
void
network::set_max_learning_rate (float max)
{
  if (is_ssab_active()) {
    if (max < maxnu) {
      int nw = count_weights();
      for (int i=0; i<nw; i++) {
	if (nus[i]>max) nus[i]=max;
      }
    }
  }
  maxnu = max;
}

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
void
network::set_min_learning_rate (float min)
{
  if (is_ssab_active()) {
    if (min > minnu) {
      int nw = count_weights();
      for (int i=0; i<nw; i++) {
	if (nus[i]<min) nus[i]=min;
      }
    }
  }
  minnu = min;
}


/*!\brief Print learning rates for SuperSab mode
 * \return number of weights
 */
int
network::ssab_print_nus () const
{
  int i, nweights;
  if (nus == NULL)
    {
      cerr << "network: Warning SuperSAB is not active" << endl;
      return -1;
    }
  else
    {
      nweights = count_weights ();
      for (i = 0; i < nweights; i++)
	{
	  printf ("%f   ", nus[i]);
	}
      printf ("\n");
      return nweights;
    }
}

/*!\brief Make some statistics about learning rates in SuperSab mode
 * \return -1 if ( ! is_ssab_active() ) , number of weights of the network otherwise
 * \param average the average of learning rates
 * \param max the max value of learning rates
 * \param min the min value of learning rates
 * \param n_max number of learning rates equal to max
 * \param n_min number of learning rates equal to min
 */
int 
network::ssab_stats(float& average, float &max, float &min, int &n_max, int &n_min) {
  if (! is_ssab_active() ) {
    return -1;
  }
  int nw = count_weights();
  
  float sum = 0.0;
  max = minnu;
  min = maxnu;
  for (int i = 0; i < nw  ; i++) {
    sum += nus[i];
    if (nus[i]>max) max = nus[i];
    if (nus[i]<min) min = nus[i];
  }
  average = sum / nw;

  n_min = 0;
  n_max = 0;
  for (int i = 0; i < nw  ; i++) {
    if (nus[i]==max) n_max++;
    if (nus[i]==min) n_min++;
  }
  return nw;
}

/****************************************
 * Batch Training
 ****************************************/


/*!\brief [Internal] Adjust deltas based on (backpropagated) output error.
 */
void
network::adjust_sumdeltas_batch ()
{
  int l, nu, nl;
  float error;

  for (l = 1; l < no_of_layers; l++)
    {
      for (nu = 0; nu < layer[l].no_of_neurons; nu++)
	{
	  error = layer[l].neuron[nu].error;
	  for (nl = 0; nl < layer[l - 1].no_of_neurons; nl++)
	    {
	      layer[l].neuron[nu].sumdeltas[nl] +=
		 error * layer[l - 1].neuron[nl].output;
	    }
	}
    }
}

/*!\brief [Internal] Adjust weights based on deltas determined by batch
 * training.
 */
void
network::adjust_weights_batch ()
{
  int l, nu, nl;
  float delta;

  float learning_factor =  learning_rate /( (float) no_of_patterns);

  for (l = 1; l < no_of_layers; l++)
    {
      for (nu = 0; nu < layer[l].no_of_neurons; nu++)
	{
	  for (nl = 0; nl < layer[l - 1].no_of_neurons; nl++)
	    {
	      delta = learning_factor * (layer[l].neuron[nu].sumdeltas[nl]) 
		+ momentum * layer[l].neuron[nu].delta[nl];
	      layer[l].neuron[nu].weight[nl] += delta;
	      layer[l].neuron[nu].delta[nl] = delta;
	    }
	}
    }
}

/*!\brief Begin training in batch mode.
 */
void
network::begin_batch ()
{
  no_of_patterns = 0;
  reset_sumdeltas ();
}


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
 * will be changed when (at the end of the batch) end_batch() is
 * called.
 */
void
network::train_batch ()
{
  no_of_patterns++;
  backward_pass ();
  adjust_sumdeltas_batch ();
}


/*!\brief End training in batch mode adjusting weights.
 *
 * Adjust the weights in the neural network according to the average 
 * delta of all patterns in the batch.
 */
void
network::end_batch ()
{
  adjust_weights_batch ();
}

/***************************************
 * Batch and SuperSab
 ***************************************/

/*!\brief [Internal] Adjust weights based on deltas determined by batch
 * training with SuperSab learning rates.
 */
void
network::adjust_weights_batch_ssab ()
{
  int l, nu, nl;
  float delta;

  float inv_no_of_patterns = 1.0 /( (float) no_of_patterns);
  int nuind = 0;

  for (l = 1; l < no_of_layers; l++)
    {
      for (nu = 0; nu < layer[l].no_of_neurons; nu++)
	{
	  for (nl = 0; nl < layer[l - 1].no_of_neurons; nl++)
	    {
	      delta = nus[nuind] * (layer[l].neuron[nu].sumdeltas[nl]) * inv_no_of_patterns + momentum * layer[l].neuron[nu].delta[nl];

	      layer[l].neuron[nu].weight[nl] += delta;
	  
	      if (layer[l].neuron[nu].delta[nl] * delta > 0)
		{
		  nus[nuind] = min (nus[nuind] * nuup, maxnu);
		}
	      else
		{
		  nus[nuind] = max (nus[nuind] * nudown, minnu);
		}
	      layer[l].neuron[nu].delta[nl] = delta;
	      nuind++;
	    }
	}
    }
}



/*!\brief End training in batch mode adjusting weights with SuperSab.
 *
 * For using SuperSab  mode in batch training you should call begin_ssab() 
 * and begin_batch(), train the network with train_batch() and then 
 * call end_batch_ssab() at the end of every epoch.
 * 
 * Adjust the weights in the neural network according to the average 
 * delta of all patterns in the batch and with SuperSab.
 */
void network::end_batch_ssab () 
{
  if ( ! is_ssab_active() ) {
    cerr << "lwneuralnet++: Warning: using end_batch_ssab but not in SuperSab mode. Ignoring SuperSab" << endl; 
    adjust_weights_batch ();
  } else {
    adjust_weights_batch_ssab();
  }
}

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
void
network::jolt (float factor, float range)
{
  int l, nu, nl;

  /* sanity check */
  if ((factor < 0) || (range < 0))
    {
      return;
    }

  /* modify weights */
  for (l = 1; l < no_of_layers; l++)
    {
      for (nu = 0; nu < layer[l].no_of_neurons; nu++)
	{
	  for (nl = 0; nl < layer[l - 1].no_of_neurons; nl++)
	    {
	      if (fabs (layer[l].neuron[nu].weight[nl]) < range)
		{
		  layer[l].neuron[nu].weight[nl] =
		    2.0 * range * ((float) rand () / RAND_MAX - 0.5);
		}
	      else
		{
		  layer[l].neuron[nu].weight[nl] *=
		    1.0 + 2.0 * factor * ((float) rand () / RAND_MAX - 0.5);
		}
	    }
	}
    }
}


/* \brief Writes a network on a stream  
 * Same format as friendly_print(false)
 * Usage:
 * os << net;
 */
ostream & operator<< (ostream & os, const network & net)
{
  stringstream buf;

  /* Note that this function is declared as friend by the 
   * network class and hence can access its private fields
   */

  /* write network dimensions */
  buf << "No of layers: " << net.no_of_layers << endl;
  for (int l = 0; l < net.no_of_layers; l++)
    {
      buf << "No of neurons on layer " << l << ": " << net.layer[l].no_of_neurons << endl;
    }

  /* write network constants */
  buf << "Learning rate: " << net.learning_rate << endl;
  buf << "Momentum: " << net.momentum << endl;
  buf << "Global Error: " << net.global_error << endl;

  buf << "Activation Func: " <<
    (net.activation == network::LOGISTIC ? " Logistic" : "Tanh") << endl;

  if (net.is_ssab_active()) {
    buf << "SuperSab mode is active." << endl;
    buf << "Max Learning rate: " <<  net.maxnu << endl;
    buf << "Min Learning rate: " <<  net.minnu << endl;
    buf << "nu_up (factor for increasing): " << net.nuup << endl;
    buf << "nu_down (factor for decreasing): " << net.nudown << endl;
  }
  return os << buf.str ();
}


/*********************************
 * MEMORY MANAGEMENT
 ********************************/

/* \brief Overloaded operator= 
 */
const network &
network::operator= (const network & b)
{
  // guard against assignment to itself
  if (this == &b)
    return *this;

  // destroy the left-hand side
  destroy ();
  // copy the right-hand side
  copy (b);
  return *this;
}

/* \brief Copy constructor
 */
network::network (const network & b)
{
  copy (b);
}


/* \brief[Internal] 
 * Used to copy a network (common code of operator= and copy constructor)
 */
void
network::copy (const network & b)
{
  int l, nu, nl;

  no_of_layers = b.no_of_layers;
  layer = (layer_t *) calloc (no_of_layers, sizeof (layer_t));

  for (l = 0; l < no_of_layers; l++)
    {
      allocate_layer (&layer[l], b.layer[l].no_of_neurons);
    }

  for (l = 1; l < no_of_layers; l++)
    {
      allocate_weights (&layer[l - 1], &layer[l]);
    }


  /* abbreviations for input and output layer */
  input_layer = &layer[0];
  output_layer = &layer[no_of_layers - 1];

  /* copy values of network constants */
  momentum = b.momentum;
  learning_rate = b.learning_rate;
  activation = b.activation;

  /* copy Super Sab learning rates */
  if (b.nus == NULL)
    {
      nus = NULL;
    }
  else
    {
      int nw = b.count_weights ();
      nus = (float *) malloc (nw * sizeof (float));
      memcpy (nus, b.nus, nw * sizeof (float));
    }

  /* copy parameters of SuperSab */
  minnu = b.minnu;
  maxnu = b.maxnu;
  nuup = b.nuup;
  nudown = b.nudown;

  /* copy other fields */
  global_error = b.global_error;
  no_of_patterns = b.no_of_patterns;

  /* copy the  network's weights, deltas and sumdeltas */
  for (l = 1; l < no_of_layers; l++)
    {
      for (nu = 0; nu < layer[l].no_of_neurons; nu++)
	{
	  for (nl = 0; nl < layer[l - 1].no_of_neurons; nl++)
	    {
	      layer[l].neuron[nu].weight[nl] =
		b.layer[l].neuron[nu].weight[nl];
	      layer[l].neuron[nu].delta[nl] = b.layer[l].neuron[nu].delta[nl];
	      layer[l].neuron[nu].sumdeltas[nl] = b.layer[l].neuron[nu].sumdeltas[nl];
	    }
	}
    }

}

};