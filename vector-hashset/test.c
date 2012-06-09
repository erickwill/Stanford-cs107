#include "vector.h"
#include <stdio.h>

int main()
{
  vector v;
  VectorNew(&v, 4, NULL, 4);
  int i = 1;
  VectorAppend(&v, &i);
  int *p = VectorNth(&v, 0);
  printf("%d \n", *p);
  printf("%d \n", VectorLength(&v));
  VectorDispose(&v);
  return 0;
}

