#include "vector.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

void VectorNew(vector *v, int elemSize, VectorFreeFunction freeFn, int initialAllocation)
{
  assert(elemSize > 0);
  v->elemSize = elemSize;
  v->allocLength = initialAllocation;
  v->logLength = 0;
  v->elems = malloc(initialAllocation * elemSize);
  assert(v->elems != NULL);
  v->VectorFreeFunction = freeFn;
}

void VectorDispose(vector *v)
{
  if (v->VectorFreeFunction != NULL) {
    for (int i = 0; i < v->logLength; i++) {
      v->VectorFreeFunction(VectorNth(v,i));
    }
  }

  free(v->elems);
}

static void DoubleCapacity(vector *v)
{
  v->allocLength *= 2;
  v->elems = realloc(v->elems, v->allocLength * v->elemSize);
  assert(v->elems != NULL);
}

int VectorLength(const vector *v)
{ 
  return v->logLength; 
}

void *VectorNth(const vector *v, int position)
{ 
  if (position >= v->logLength) return NULL;
  
  void *destAddr;
  void *srcAddr = (char*)v->elems + position * v->elemSize;
  memcpy(&destAddr, &srcAddr, v->elemSize);
  return destAddr; 
}

void VectorReplace(vector *v, const void *elemAddr, int position)
{
  if (position >= v->logLength) return;
  else {
    void *destAddr = (char*)v->elems + (position * v->elemSize);
    memcpy(destAddr,elemAddr,v->elemSize);
  }
}

void VectorInsert(vector *v, const void *elemAddr, int position)
{
  if (position > v->logLength) {
    return;
  } else if (position == v->logLength) {
    VectorAppend(v, elemAddr);
    return;
  }

  if (v->logLength == v->allocLength) DoubleCapacity(v);

  void *startShiftAddr = (char*)v->elems + (position * v->elemSize);
  void *endShiftAddr = (char*)v->elems + ((position+1) * v->elemSize);
  int blockSize = (v->logLength - position) * v->elemSize;
  memmove(endShiftAddr,startShiftAddr,blockSize);
  VectorReplace(v,elemAddr,position);
  v->logLength++;
}

void VectorAppend(vector *v, const void *elemAddr)
{
  if (v->logLength == v->allocLength) {
    DoubleCapacity(v);
  }
  
  void *destAddr = (char*)v->elems + v->logLength * v->elemSize;
  memcpy(destAddr, elemAddr, v->elemSize);
  v->logLength++;
}

void VectorDelete(vector *v, int position)
{
  if (position >= v->logLength) return;
  if (position == v->logLength) v->logLength--;

  void *startShiftAddr = (char*)v->elems + ((position+1) * v->elemSize);
  void *endShiftAddr = (char*)v->elems + (position * v->elemSize);
  int blockSize = (v->logLength - position - 1) * v->elemSize;
  memmove(endShiftAddr,startShiftAddr,blockSize);
  v->logLength--;  
}

void VectorSort(vector *v, VectorCompareFunction compare)
{
  qsort(v,v->logLength * v->elemSize,v->elemSize,compare);
}

void VectorMap(vector *v, VectorMapFunction mapFn, void *auxData)
{
  for (int i = 0; i < v->logLength; i++) {
    void *element;
    void *currAddr = (char*)v->elems + (i * v->elemSize);
    memcpy(&element,currAddr,v->elemSize);
    mapFn(&element,auxData);
  }
}


static const int kNotFound = -1;
int VectorSearch(const vector *v, const void *key, VectorCompareFunction searchFn, int startIndex, bool isSorted)
{ 
  if (isSorted) {
    void *found = bsearch(key,(char*)v->elems + (startIndex * v->elemSize),
			  (v->logLength-startIndex) * v->elemSize,v->elemSize,searchFn);
    if (found == NULL) return -1;
    else {
      for (int i = 0; i < v->logLength; i++) {
	if (memcmp(found,(char*)v->elems + (i * v->elemSize),v->elemSize) == 0)
	  return i;
      }
    }
  } else {
    for (int i = 0; i < v->logLength; i++) {
	if (memcmp(key,(char*)v->elems + (i * v->elemSize),v->elemSize) == 0)
	  return i;
    }
  }
  return -1; 
} 
