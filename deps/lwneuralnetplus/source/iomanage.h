#ifndef IOMANAGE_H
#define IOMANAGE_H
/*
 * Lightweight Neural Net ++ - iomanage (abstract) class
 * http://lwneuralnetplus.sourceforge.net/
 * 
 * This class is part of lwnn++ library
 *
 * By Lorenzo Masetti <lorenzo.masetti@libero.it> and Luca Cinti <lucacinti@supereva.it>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 */
#include <stdexcept>
#include <string>
#include <vector>
//using namespace std;
using std::string;
using std::vector;

/*!\brief This is an abstract class which declares some methods 
 *        to be implemented by a iomanager.
 * To use your own input procedures you must write a derived class
 * of iomanage that implements the two methods, info_from_file and
 * load_patterns.
 *
 * The class implements two static methods to allocate and destroy 
 * input and output vectors. 
 * Method allocate_data can be used to call allocate_data and load_patterns
 * (virtually binded to the derived class method).
 * 
 * One example of a  derived class is iomanagelwnnfann.
 *
 */
class iomanage 
{
public:


  /*!\brief Read info from file
   *\param filename the name of a file of input/output pairs
   *\param npatterns Pointer to a int where is returned the number of 
   *       pairs in the file
   *\param ninput Pointer to a int where is returned the number of 
   *       inputs 
   *\param noutput Pointer to a int where is returned the number of 
   *       outputs
   *
   * Can throw a runtime_error exception if file does not exist.
   */
  virtual void info_from_file (const string & filename, int *npatterns,
			      int *ninput, int *noutput) = 0;

  /*!\brief Read patterns from file
   *\param filename the name of a file of input/output pairs
   *\param inputs  Read the inputs in this matrix
   *\param targets Reat the targets in this matrix
   *\param ninput Number of inputs wanted. 
   *\param noutput Number of outputs wanted
   *\param npatterns Number of patterns wanted
   *
   * Precondition: memory in inputs and outputs has been allocated for
   * npatterns with ninput and noutput vectors. (First index is the index
   * of pattern, second index is the index of neuron). 
   * You can use allocate_data() to allocate the memory.
   *
   * If the number of inputs or outputs of the patterns in the file
   * is not equal to ninput and noutput throws a runtime_error.
   * Can throw a runtime_error exception if the format of the file isn't right.
   *
   */
  virtual void load_patterns (const string & filename,
				       float **inputs, float **targets,
				       int ninput, int noutput, int npatterns) = 0;


  /*!\brief Allocate data for patterns 
   *\param npattern number of patterns
   *\param ninput number of inputs
   *\param noutput number of outputs
   *\param input vector of inputs (not allocated)
   *\param target vector of targets (not allocated)
   */
  static void allocate_data(int npattern, int ninput, int noutput, float** &input, float** &target);


  /*!\brief Allocate memory and load data from a file
   *\param filename the name of a file of input/output pairs
   *\param inputs  Read the inputs in this matrix
   *\param targets Reat the targets in this matrix
   *\param ninput Return Number of inputs. 
   *\param noutput Return Number of outputs
   *\param npatterns Return Number of patterns
   */
  virtual void allocate_and_load( const string& filename, float** &inputs, float** &targets, int* ninput, int* noutput, int* npatterns);


  /*!\brief free memory for input and output
   *\param npatterns Number of patterns
   *\param input Input vector to be freed
   *\param output Output vector to be freed
   */
  static void destroy(int npatterns, float** input, float** output);
};


#endif



