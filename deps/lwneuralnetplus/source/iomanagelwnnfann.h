#ifndef IOMANAGE_LWNNFANN_H
#define IOMANAGE_LWNNFANN_H
/*
 * Lightweight Neural Net ++ - iomanagelwnnfann class
 * http://lwneuralnetplus.sourceforge.net/
 * 
 * This class is used to read target files in lwnn and fann format
 *
 * By Lorenzo Masetti <lorenzo.masetti@libero.it> and Luca Cinti <lucacinti@supereva.it>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 */
#include "iomanage.h"
#include <string>

namespace ANN {

	/*!\brief This class manages files in
	 *        lwnn format and fann format
	 *
	 * It is derived by iomanage and implemets reading in two
	 * textual formats.
	 *
	 * <strong>FORMAT OF INPUT FILES</strong>
	 * - lwnn:
	 * This type of files supports comments, ignoring lines beginning with #.<br>
	 * Comments are not allowed between a input line and a target line. <br>
	 * The first (not comment) line is the number of pairs in the file. <br>
	 * The second line is the number of inputs, the third one the number of outputs.<br>
	 * Then there should be 2 * no_of_pairs lines containing inputs and matching
	 * target. Numbers are delimited by spaces
	 *
	 * - fann:
	 * This is the format used by fann (Fast Artificial Neural Network) library
	 * ( see http://fann.sourceforge.net/ )  by Steffen Nissen (lukesky@diku.dk)
	 * The code for reading is borrowed and adapted from that library.<br>
	 * Format: No comments are supported. The first line should containt three
	 * integers being the number of pairs, the number of inputs and outputs
	 * Then there are no_of_pairs lines containing INPUTS and no_of_pairs lines
	 * containing outputs.
	 *
	 * The class guesses the format of the file and reads properly a file without
	 * having to know its type.
	 */
	class iomanagelwnnfann : public iomanage
	{
	public:
		/*!\brief Constructor
		 * Does nothing, of course, because the class has no fields!
		 */
		iomanagelwnnfann();

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
		 *
		 * Works with both formats and guesses the right format by itself
		 */
		virtual void info_from_file(const ::std::string & filename, int *npatterns,
			int *ninput, int *noutput);

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
		 *
		 * Works only with lwnn format
		 */
		static void info_from_file_lwnn(const ::std::string & filename, int *npatterns,
			int *ninput, int *noutput);

		/*!\brief Read patterns from file
		 *\param filename the name of a file of input/output pairs
		 *\param npatterns Pointer to a int where is returned the number of
		 *       pairs in the file
		 *\param ninput Pointer to a int where is returned the number of
		 *       inputs
		 *\param noutput Pointer to a int where is returned the number of
		 *       outputs
		 *
		 * Can throw a runtime_error exception if file does not exist.
		 *
		 * Works only with fann format
		 */
		static void info_from_file_fann(const ::std::string & filename, int *npatterns,
			int *ninput, int *noutput);

		/*!\brief Read info from file
		 *\param filename the name of a file of input/output pairs
		 *\param inputs  Read the inputs in this matrix
		 *\param targets Reat the targets in this matrix
		 *\param ninput Number of inputs wanted.
		 *\param noutput Number of outputs wanted
		 *\param npatterns Number of patterns wanted
		 *
		 * Precondition: memory in inputs and outputs has been allocated for
		 * npatterns with ninput and noutput vectors. (First index is the index
		 * of pattern, second index is the index of neuron)
		 *
		 * If the number of inputs or outputs of the patterns or the number of
		 * patterns in the file
		 * is not equal to ninput and noutput and npatterns throws a runtime_error.
		 * Can throw a runtime_error exception if the format of the file isn't right.
		 *
		 * Works with both formats and guesses the right format by itself
		 */
		virtual void load_patterns(const ::std::string & filename,
			float **inputs, float **targets,
			int ninput, int noutput, int npatterns);


		/*!\brief Read info from file
		 *\param filename the name of a file of input/output pairs
		 *\param inputs  Read the inputs in this matrix
		 *\param targets Reat the targets in this matrix
		 *\param ninput Number of inputs wanted.
		 *\param noutput Number of outputs wanted
		 *\param npatterns Number of patterns wanted
		 *
		 * Precondition: memory in inputs and outputs has been allocated for
		 * npatterns with ninput and noutput vectors. (First index is the index
		 * of pattern, second index is the index of neuron)
		 *
		 * Works only with lwnn format
		 */
		static void load_patterns_lwnn(const ::std::string & filename,
			float **inputs, float **targets,
			int ninput, int noutput, int npatterns);


		/*!\brief Read info from file
		 *\param filename the name of a file of input/output pairs
		 *\param inputs  Read the inputs in this matrix
		 *\param targets Reat the targets in this matrix
		 *\param ninput Number of inputs wanted.
		 *\param noutput Number of outputs wanted
		 *\param npatterns Number of patterns wanted
		 *
		 * Precondition: memory in inputs and outputs has been allocated for
		 * npatterns with ninput and noutput vectors. (First index is the index
		 * of pattern, second index is the index of neuron)
		 *
		 * If the number of inputs or outputs of the patterns in the file
		 * is not equal to ninput and noutput throws a runtime_error.
		 * Can throw a runtime_error exception if the format of the file isn't right.
		 *
		 * Works only with fann format
		 */
		static void load_patterns_fann(const ::std::string & filename,
			float **inputs, float **targets,
			int ninput, int noutput, int npatterns);

	};
};

#endif
