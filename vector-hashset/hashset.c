#include "hashset.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void HashSetNew(hashset *h, int elemSize, int numBuckets,
		HashSetHashFunction hashfn, HashSetCompareFunction comparefn,
		HashSetFreeFunction freefn)
{
  h->elemSize = elemSize;
  h->numBuckets = numBuckets;
  h->hashfn = hashfn;
  h->compfn = comparefn;
  h->freefn = freefn;

  h->buckets = malloc(sizeof(vector) * numBuckets);

  for (int i = 0; i < numBuckets; i++) {
    void *srcAddr = (char*)h->buckets + i * sizeof(vector);
    VectorNew(srcAddr, elemSize, freefn, 4);
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
  int total = 0;
  for (int i = 0; i < h->numBuckets; i++) {
    total += VectorLength(&h->buckets[i]);
  }
  return total;
}

void HashSetMap(hashset *h, HashSetMapFunction mapfn, void *auxData)
{
  for (int i = 0; i < h->numBuckets; i++) {
    VectorMap(&h->buckets[i],mapfn,auxData);
  }
}

static int vectorLookup(vector *v,const void *elemAddr, 
			   HashSetCompareFunction compfn)
{
  return  VectorSearch(v,elemAddr,compfn,0,false);
}

void HashSetEnter(hashset *h, const void *elemAddr)
{
  int hashNum = h->hashfn(elemAddr,h->numBuckets);
  assert(hashNum < h->numBuckets);
  int index = vectorLookup(&h->buckets[hashNum],elemAddr,h->compfn);
  if (index == -1)
    VectorAppend(&h->buckets[hashNum],elemAddr);
  else
    VectorReplace(&h->buckets[hashNum], elemAddr, index);
}

void *HashSetLookup(const hashset *h, const void *elemAddr)
{
  int hashNum = h->hashfn(elemAddr,h->numBuckets);
  VectorSort(&h->buckets[hashNum],h->compfn);
  int index =  vectorLookup(&h->buckets[hashNum],elemAddr,h->compfn);
  if (index == -1) return NULL;
  else return VectorNth(&h->buckets[hashNum],index);
}

