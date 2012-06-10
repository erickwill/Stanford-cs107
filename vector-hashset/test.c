#include "vector.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

int cmp(const void *a, const void*b)
{
  int one;
  memcpy(&one,(int*)a,sizeof(int));
  printf("ONE = %d\n", one);
  int two;
  memcpy(&two,(int*)b,sizeof(int));
  printf("TWO = %d\n", two);

  if (one > two) return 1;
  if (two > one) return -1;
  return 0;
}

void mapFn(void *elemAddr, void* auxData)
{
  printf("elem = %d\n", *(int*)elemAddr);
}

int main()
{
  vector v;
  VectorNew(&v, sizeof(char), NULL, 4);
  int num = 0;
  for (char i = 'A'; i <= 'Z'; i++) {
    printf("%c",i);
    VectorAppend(&v,&i);
    printf("%c , %d\n",*(char*)VectorNth(&v,num), num);
    num++;
  }

  /*
  for (int i = 0; i < VectorLength(&v); i++) {
    char *elem = (char*) VectorNth(&v,i);
    *elem = tolower(*elem);
  }
  */
  printf("length = %d\n", VectorLength(&v));
  for (int i = 0; i < VectorLength(&v); i++) {
    char *elem = (char*)VectorNth(&v,i);
    printf("%d  ", i);
    printf("\n%c\n", *elem);
  }

  VectorDispose(&v);
  return 0;
}

