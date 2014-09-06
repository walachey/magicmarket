#ifndef SHUFFLE_H
#define SHUFFLE_H
#include <vector>
using namespace std;
/*
 * Lightweight Neural Net ++ - shuffle class
 * http://lwneuralnetplus.sourceforge.net/
 *
 * This C++ library provides the class shuffle wich implements a 
 * vector of size n which contains numbers form 0 to n-1 in random order
 *
 * By Lorenzo Masetti <lorenzo.masetti@libero.it> and Luca Cinti <lucacinti@supereva.it>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 */

/*!\brief This class implements a 
 * vector of size n which contains numbers form 0 to n-1 in random order
 *
 */
class shuffle {
public:
  /*!\brief Constructs a shuffle of length n
   *\param n length of shuffle
   */
  shuffle(int n);

  /*!\brief Redo the shuffle
   */
  void redo();
  
  /*!\brief operator[] read-only
   *\param x index
   */
  int operator[] (int x) const;
  
  /*!\brief Retrieve the size of the shuffle 
   *\return size 
   */
  int size() const;
  
private:
  void doshuffle();
  vector<int> _v;
  unsigned long int _count;
};

/*!\brief operator<< writes a shuffle on a stream
 */
ostream& operator<<(ostream& os, const shuffle& s);

inline
int shuffle::operator[] (int x) const {
    return _v[x];
}

inline
int shuffle::size() const {
  return _v.size();
}
#endif








