#include "hashset.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static const int defaultVectorAlloc = 4;
static const int kNotFound = -1;

void HashSetNew(hashset *h, int elemSize, int numBuckets,
		HashSetHashFunction hashfn, HashSetCompareFunction comparefn,
		HashSetFreeFunction freefn)
{
  assert(elemSize > 0 && numBuckets > 0 && hashfn != NULL && 
         comparefn != NULL);

  h->elemSize = elemSize;
  h->numBuckets = numBuckets;
  h->count = 0;
  h->hashfn = hashfn;
  h->compfn = comparefn;
  h->freefn = freefn;

  h->buckets = malloc(sizeof(vector) * numBuckets);

  for (int i = 0; i < numBuckets; i++) {
    void *srcAddr = (char*)h->buckets + i * sizeof(vector);
    VectorNew(srcAddr, elemSize, freefn, defaultVectorAlloc); // init vectors
  }
}

void HashSetDispose(hashset *h)
{
  for (int i = 0; i < h->numBuckets; i++) {
    VectorDispose(&h->buckets[i]);
  }

  free(h->buckets);
}

int HashSetCount(const hashset *h)
{
  return h->count;
}

void HashSetMap(hashset *h, HashSetMapFunction mapfn, void *auxData)
{
  assert(mapfn != NULL);

  for (int i = 0; i < h->numBuckets; i++) {
    VectorMap(&h->buckets[i],mapfn,auxData);
  }
}

static int vectorLookup(vector *v,const void *elemAddr,
                        HashSetCompareFunction compfn)
{
  return  VectorSearch(v,elemAddr,compfn,0,true);
}

void HashSetEnter(hashset *h, const void *elemAddr)
{
  assert(elemAddr != NULL);

  int hashNum = h->hashfn(elemAddr,h->numBuckets);
  assert(hashNum >= 0 && hashNum < h->numBuckets);

  int index = vectorLookup(&h->buckets[hashNum],elemAddr,h->compfn);

  if (index == kNotFound) {
    VectorAppend(&h->buckets[hashNum],elemAddr);
    h->count++;
  } else {    
    VectorReplace(&h->buckets[hashNum], elemAddr, index); // clobber existing element
  }
}

void *HashSetLookup(const hashset *h, const void *elemAddr)
{
  assert(elemAddr != NULL);

  int hashNum = h->hashfn(elemAddr,h->numBuckets);
  assert(hashNum >= 0 && hashNum < h->numBuckets);

  VectorSort(&h->buckets[hashNum],h->compfn);

  int index =  vectorLookup(&h->buckets[hashNum],elemAddr,h->compfn);

  if (index == kNotFound) 
    return NULL;
  else 
    return VectorNth(&h->buckets[hashNum],index);
}

