/*
 * Lightweight Neural Net ++ - iomanagelwnnfann  class
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
#include "iomanagelwnnfann.h"
#include <string>
#include <vector>
#include <stdexcept>

#define FANNSCANF "%f"
#define MAX_LINE_LENGTH 10000

namespace ANN
{
	using ::std::string;
	using ::std::vector;
	using ::std::runtime_error;
	iomanagelwnnfann::iomanagelwnnfann() {
		/* does nothing :) */
	}

	void
		iomanagelwnnfann::info_from_file(const string & filename, int *npatterns,
		int *ninput, int *noutput)
	{
#ifdef DEBUG_LOAD
		cout << "Calling info_from_file..." << endl;
#endif

		FILE *file = fopen(filename.c_str(), "r");
		if (file == NULL)
		{
			throw runtime_error("Unable to open train data file " + filename +
				" for reading.");
		}

		if (fscanf(file, "%i %i %i\n", npatterns, ninput, noutput) != 3)
		{
#ifdef DEBUG_LOAD
			cout << "Trying to read in lwnn format!" << endl;
#endif

			fclose(file);
			info_from_file_lwnn(filename, npatterns, ninput, noutput);
		}
		else
		{
			fclose(file);
		}
	}

	void
		iomanagelwnnfann::info_from_file_lwnn(const string & filename, int *npatterns,
		int *ninput, int *noutput)
	{
		char line[MAX_LINE_LENGTH + 1];
		FILE *file = fopen(filename.c_str(), "r");
		if (file == NULL)
			throw runtime_error("Cannot open file");
		/* read number of patterns */
		do
		{
			fgets(line, MAX_LINE_LENGTH, file);
		} while (line[0] == '#');
		if (sscanf(line, "%i\n", npatterns) != 1) {
			throw runtime_error("Wrong lwnn pattern file format");
		}

		do
		{
			fgets(line, MAX_LINE_LENGTH, file);
		} while (line[0] == '#');
		if (sscanf(line, "%i\n", ninput) != 1) {
			throw runtime_error("Wrong lwnn pattern file format");
		}

		/* read number of outputs */
		do
		{
			fgets(line, MAX_LINE_LENGTH, file);
		} while (line[0] == '#');
		if (sscanf(line, "%i\n", noutput) != 1) {
			throw runtime_error("Wrong lwnn pattern file format");
		}

		fclose(file);
	}

	void
		iomanagelwnnfann::info_from_file_fann(const string & filename, int *npatterns,
		int *ninput, int *noutput)
	{
		FILE *file = fopen(filename.c_str(), "r");
		if (file == NULL)
		{
			throw runtime_error("Unable to open train data file " + filename +
				" for reading.");
		}

		if (fscanf(file, "%i %i %i\n", npatterns, ninput, noutput) != 3)
		{
			throw runtime_error("Error reading info from train data file \"" +
				filename + "\"");
		}
		fclose(file);
	}

	void
		iomanagelwnnfann::load_patterns_lwnn(const string & filename, float **inputs,
		float **targets, int ninput, int noutput, int npatterns)
	{
		FILE *file;
		char line[MAX_LINE_LENGTH + 1], *index;
		int skipped, no_of_pairs, no_of_outputs, no_of_inputs;
		file = fopen(filename.c_str(), "r");

		/* sanity check */
		if (file == NULL)
			throw runtime_error("Cannot open file");
		/* read number of patterns */
		do
		{
			fgets(line, MAX_LINE_LENGTH, file);
		} while (line[0] == '#');
		if (sscanf(line, "%i\n", &no_of_pairs) != 1) {
			throw runtime_error("Wrong lwnn pattern file format");
		}


		if (npatterns != no_of_pairs) {
			fclose(file);
			throw runtime_error("Patterns file contains a wrong number of patterns");
		}

		/* read number of inputs */
		do
		{
			fgets(line, MAX_LINE_LENGTH, file);
		} while (line[0] == '#');
		if (sscanf(line, "%i\n", &no_of_inputs) != 1) {
			throw runtime_error("Wrong lwnn pattern file format");
		}



		/* read number of outputs */
		do
		{
			fgets(line, MAX_LINE_LENGTH, file);
		} while (line[0] == '#');
		if (sscanf(line, "%i\n", &no_of_outputs) != 1) {
			throw runtime_error("Wrong lwnn pattern file format");
		}

		if ((ninput != no_of_inputs) || (noutput != no_of_outputs))
		{
			fclose(file);
			throw runtime_error("Patterns file does not fit the net");
		}


#ifdef DEBUG
		cout << "Filename " << filename << endl;
		cout << endl << no_of_inputs << endl << no_of_outputs << endl << no_of_pairs
			<< endl;
#endif

		/* read pairs of data */
		fgets(line, MAX_LINE_LENGTH, file);
		int npatt = 0;

		while (npatt < no_of_pairs)
		{
			if (feof(file))
			{
				fclose(file);
				throw runtime_error("Input/Output error: not enough pattern");
			}
			if (line[0] != '#')
			{
#ifdef DEBUG
				cout << "Reading pattern no.: " << npatt << endl;
#endif
				/* inputs */
				index = line;
				for (int i = 0; i < no_of_inputs; i++)
				{
					sscanf(index, "%f%n", &inputs[npatt][i], &skipped);
					index += skipped;
				}
				/* targets */
				fgets(line, MAX_LINE_LENGTH, file);

				index = line;
				for (int i = 0; i < no_of_outputs; i++)
				{
					sscanf(index, "%f%n", &targets[npatt][i], &skipped);
					index += skipped;
				}
				/* next pair */
				npatt++;
#ifdef DEBUG
				cout << "Read pattern no.: " << npatt ;
#endif
			}
			fgets(line, MAX_LINE_LENGTH, file);
		}
		fclose(file);
	}

	/* load a file in the format of fann (http://fann.sourceforge.net)
	   fann library is written by Steffen Nissen (lukesky@diku.dk)
	   Some code of fann library was reused in this method
	   */
	void
		iomanagelwnnfann::load_patterns_fann(const string & filename, float **inputs,
		float **targets, int ninput, int noutput, int npatterns)
	{
		int num_input, num_output, num_data, i, j;
		unsigned int line = 1;
		FILE *file = fopen(filename.c_str(), "r");

		if (file == NULL)
			throw runtime_error("Unable to open train data file " + filename +
			" for reading.");

		if (fscanf(file, "%i %i %i\n", &num_data, &num_input, &num_output) != 3)
			throw runtime_error("Error reading info from train data file \"" +
			filename + "\"");

		line++;
		if ((ninput != num_input) || (noutput != num_output))
		{
			fclose(file);
			throw runtime_error("Patterns file does not fit the net");
		}

		if (npatterns != num_data) {
			fclose(file);
			throw runtime_error("Patterns file contains a wrong number of patterns");
		}

#ifdef DEBUG_LOAD
		cout << "iomanagelwnnfann: Importing file in fann library format" << endl;
#endif

		for (i = 0; i != num_data; i++)
		{
			for (j = 0; j != num_input; j++)
			{
				if (fscanf(file, FANNSCANF " ", &(inputs[i][j])) != 1)
				{
					throw
						runtime_error("Error reading info from train data file \"" +
						filename + "\"");
				}
			}
			line++;
			for (j = 0; j != num_output; j++)
			{
				if (fscanf(file, FANNSCANF " ", &(targets[i][j])) != 1)
				{
					throw
						runtime_error("Error reading info from train data file \"" +
						filename + "\"");
				}
			}
			line++;
		}
		fclose(file);
	}

	void
		iomanagelwnnfann::load_patterns(const string & filename, float **inputs,
		float **targets, int ninput, int noutput, int npatterns)
	{
		int num_input, num_output, num_data;
		FILE *file = fopen(filename.c_str(), "r");
		if (file == NULL)
			throw runtime_error("Unable to open train data file " + filename +
			" for reading.");
		if (fscanf(file, "%i %i %i\n", &num_data, &num_input, &num_output) != 3)
		{
			fclose(file);
			load_patterns_lwnn(filename, inputs, targets, ninput, noutput, npatterns);
		}
		else
		{
			fclose(file);
			load_patterns_fann(filename, inputs, targets, ninput, noutput, npatterns);
		}
	}

};