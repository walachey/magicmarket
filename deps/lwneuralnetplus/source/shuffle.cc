#include "shuffle.h"
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <sstream>


shuffle::shuffle(int n) : _v(n), _count() {
    for (int i=0; i<n; i++)
	_v[i]=i;
    doshuffle();
}

void shuffle::redo() {
    doshuffle();
}
    
void shuffle::doshuffle() {
    int i,j,tmp;
    int n=_v.size();
    
    srand(time(0)+_count);
    
    for (i=n-1; i>0; i--) {
	j=rand() % (i+1);
	tmp=_v[i]; _v[i]=_v[j]; _v[j]=tmp;
    }
    _count++;
}


ostream& operator<<(ostream& os, const shuffle& s) {
  stringstream buf;

  for (int i=0; i<s.size(); i++) 
	buf << s[i] << "   ";

  return os << buf.str() << endl;
}

