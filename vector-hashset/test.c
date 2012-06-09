#include "vector.h"
#include <stdio.h>
#include <string.h>

int main()
{
  vector v;
  VectorNew(&v, sizeof(int), NULL, 4);
  for (int i = 10; i >= 0; i--) {
    VectorAppend(&v,&i);
  }

  for (int i = 0; i < VectorLength(&v); i++) {
    int *p = VectorNth(&v, i);
    printf("%d\n", *p);
  }
  VectorDispose(&v);

  return 0;
}

