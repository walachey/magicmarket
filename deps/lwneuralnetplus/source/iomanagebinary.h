#ifndef IOMANAGE_BINARY_H
#define IOMANAGE_BINARY_H
/*
 * Lightweight Neural Net ++ - iomanagebinary class
 * http://lwneuralnetplus.sourceforge.net/
 * 
 * This class is used to read and write target files in lwnn - binary format
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
#include <stdio.h>
#include <string>
#include <stdexcept>
#include <fstream>

namespace ANN
{
	/*!\brief Class for reading and writing binary patterns file
	 *
	 * This class extends iomanage and implements the reading from a binary file
	 * The method write_patterns make easy to write a binary pattern file and
	 * to convert any training set in this format
	 */
	class iomanagebinary : public iomanage
	{
	public:
		/*!\brief Constructor
		 * Does nothing, of course, because the class has no fields!
		 */
		iomanagebinary();

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
		 */
		virtual void info_from_file(const ::std::string & filename, int *npatterns,
			int *ninput, int *noutput);
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
		 * of pattern, second index is the index of neuron)
		 *
		 * If the number of inputs or outputs of the patterns or the number of
		 * patterns in the file
		 * is not equal to ninput and noutput and npatterns throws a runtime_error.
		 * Can throw a runtime_error exception if the format of the file isn't right.
		 *
		 */
		virtual void load_patterns(const ::std::string & filename,
			float **inputs, float **targets,
			int ninput, int noutput, int npatterns);

		/*!\brief Write patterns to a binary file file
		 *\param filename the name of the file to be written
		 *\param inputs  Read the inputs in this matrix
		 *\param targets Reat the targets in this matrix
		 *\param ninput Number of inputs.
		 *\param noutput Number of outputs
		 *\param npatterns Number of patterns
		 *
		 * Can throw a runtime_error exception if file is not writeable.
		 *
		 */
		void write_patterns(const ::std::string & filename,
			float **inputs, float **targets,
			int ninput, int noutput, int npatterns);

		/*!\brief Convert patterns from any format to binary format
		 *\param sourceformatfn filename containing patterns in original format
		 *\param binformatfn filename to be written in binary format
		 *\param sourceFormatIomanager iomanager managing file in original format
		 */
		void convert(const ::std::string& sourceformatfn, const ::std::string& binformatfn, iomanage* sourceFormatIomanager);

	};

};

#endif
