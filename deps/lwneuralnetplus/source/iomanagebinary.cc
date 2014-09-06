#include "iomanagebinary.h"

iomanagebinary::iomanagebinary() {

}

void iomanagebinary::info_from_file (const string & filename, int *npatterns,
		int *ninput, int *noutput) {  
  FILE *file = fopen (filename.c_str (), "r");
  if (file == NULL)
    throw runtime_error ("Cannot open file " + filename + " for reading");

  int ok =
    fread(npatterns, sizeof(int), 1, file) &&
    fread(ninput, sizeof(int), 1, file) && 
    fread(noutput, sizeof(int), 1, file);
  fclose(file);
  if ( ! ok ) {
    throw runtime_error ( "iomanagebinary: Wrong file format in " + filename);
  } 

}


void iomanagebinary::load_patterns (const string & filename,
				       float **inputs, float **targets,
				    int ninput, int noutput, int npatterns) {

  FILE *file = fopen (filename.c_str (), "r");
  if (file == NULL)
    throw runtime_error ("Cannot open file " + filename + " for reading");
  int np, ni, no;

  int ok =
    fread(&np, sizeof(int), 1, file) &&
    fread(&ni, sizeof(int), 1, file) && 
    fread(&no, sizeof(int), 1, file);

  if ( ! ok ) {
    fclose(file);
    throw runtime_error ( "iomanagebinary: Wrong file format in " + filename);
  } 


  if (npatterns != np) {
    fclose(file);
    throw runtime_error ("Patterns file contains a wrong number of patterns");
  }

  if ((ninput != ni) || (noutput != no))
    {
      fclose (file);
      throw runtime_error ("Patterns file does not fit the net");
    }
  
  size_t data_read;
  for (int i=0; i < npatterns; i++) {
    data_read = fread((inputs[i]), sizeof(float), ninput, file);
    if (data_read != ((size_t) ninput)) {
      fclose(file);
      throw runtime_error("Wrong file format");
    }
    data_read = fread((targets[i]), sizeof(float), noutput, file);
    if (data_read != ((size_t) noutput)) {
      fclose(file);
      throw runtime_error("Wrong file format");
    }
  }
  fclose(file);
  
}



void iomanagebinary::write_patterns (const string & filename,
				       float **inputs, float **targets,
				     int ninput, int noutput, int npatterns) {
  FILE *file = fopen (filename.c_str (), "w");
  if (file == NULL)
    throw runtime_error ("Cannot open file " + filename + " for writing");
  
  fwrite(&npatterns, sizeof(int), 1, file);
  fwrite(&ninput, sizeof(int), 1, file);
  fwrite(&noutput, sizeof(int), 1, file);
  for (int i = 0; i < npatterns; i++) {
    fwrite((inputs[i]), sizeof(float), ninput, file);
    fwrite((targets[i]), sizeof(float), noutput, file);
  }
  fclose(file);
}




void iomanagebinary::convert(const string& oldformatfn, const string& binformatfn, iomanage* iom) {
  float** inputs;
  float** targets;
  int ninput, noutput, npatterns;
  
  iom->allocate_and_load(oldformatfn, inputs, targets, &ninput, &noutput, &npatterns);
  
  try {
    write_patterns(binformatfn,inputs,targets,ninput, noutput, npatterns);
  } catch (runtime_error e) {
    destroy(npatterns, inputs, targets);
    throw e;
  }

  destroy(npatterns, inputs, targets);
 
}



