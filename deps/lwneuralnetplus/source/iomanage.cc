#include "iomanage.h"

void iomanage::allocate_data(int npattern, int ninput, int noutput, float** &input, float** &target) {
  /* Allocation of data structure */
  input = (float**) malloc(npattern * sizeof(float*));

  for (int i=0; i < npattern; i++) {
    input[i] = (float*) malloc(ninput * sizeof(float));    
  }

  target = (float**) malloc(npattern * sizeof(float*));
  for (int i=0; i < npattern; i++) {
    target[i] = (float*) malloc(noutput * sizeof(float));    
  }
}


void iomanage::allocate_and_load( const string& filename, float** &inputs, float** &targets, int* ninput, int* noutput, int* npatterns) {
  info_from_file(filename,npatterns, ninput, noutput);
  
  allocate_data(*npatterns, *ninput, *noutput, inputs, targets);

  load_patterns(filename, inputs, targets, *ninput, *noutput, *npatterns);
}





void iomanage::destroy(int n, float** input, float** output) {
  if (n==0) {
    return;
  }

  if (input==NULL) {
    return;
  }


  for (int i=0; i < n; i++) {
    free(input[i]);
    free(output[i]);
  }
  free (input);
  free (output);
}

















