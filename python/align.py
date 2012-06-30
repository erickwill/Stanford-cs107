#!/usr/bin/env python

import random # for seed, random
import sys    # for stdout
from copy import deepcopy

# Computes the score of the optimal alignment of two DNA strands.
def findOptimalAlignment(strand1, strand2, cache):
  alignment = {}
  
  # if one of the two strands is empty, then there is only
  # one possible alignment, and of course it's optimal
  if len(strand1) == 0: 
    alignment['strand1'] = (len(strand2) * ' ')
    alignment['strand2'] = strand2
    alignment['score'] = (len(strand2) * -2)
    return alignment
    
  if len(strand2) == 0: 
    alignment['strand1'] = strand1
    alignment['strand2'] = (len(strand1) * ' ')
    alignment['score'] = (len(strand1) * -2)
    return alignment
    
  key = ((strand1 + '_') + strand2)
  if cache.has_key(key):
    return deepcopy(cache[key])

  # There's the scenario where the two leading bases of
  # each strand are forced to align, regardless of whether or not
  # they actually match.
  bestWith = findOptimalAlignment(strand1[1:], strand2[1:], cache)
  if strand1[0] == strand2[0]: 
    bestWith['score'] += 1
  else:
    bestWith['score'] -= 1

  bestWith['strand1'] = (strand1[0] + bestWith['strand1'])
  bestWith['strand2'] = (strand2[0] + bestWith['strand2'])
  
  if strand1[0] == strand2[0]:
    cache[key] = deepcopy(bestWith)
    return bestWith
  
  # It's possible that the leading base of strand1 best
  # matches not the leading base of strand2, but the one after it.
  best = bestWith
  bestWithout = findOptimalAlignment(strand1, strand2[1:], cache)
  bestWithout['score'] -= 2 # penalize for insertion of space
  bestWithout['strand1'] = (' ' + bestWithout['strand1'])
  bestWithout['strand2'] = (strand2[0] + bestWithout['strand2'])
  
  if bestWithout['score'] > best['score']:
    best = bestWithout

  # opposite scenario
  bestWithout = findOptimalAlignment(strand1[1:], strand2, cache)
  bestWithout['score'] -= 2 # penalize for insertion of space  
  bestWithout['strand1'] = (strand1[0] + bestWithout['strand1'])
  bestWithout['strand2'] = (' ' + bestWithout['strand2'])
  
  if bestWithout['score'] > best['score']:
    best = bestWithout

  cache[key] = deepcopy(best)
  return deepcopy(best)

# Utility function that generates a random DNA string of
# a random length drawn from the range [minlength, maxlength]
def generateRandomDNAStrand(minlength, maxlength):
  assert minlength > 0, \
         "Minimum length passed to generateRandomDNAStrand" \
         "must be a positive number" # these \'s allow mult-line statements
  assert maxlength >= minlength, \
         "Maximum length passed to generateRandomDNAStrand must be at " \
         "as large as the specified minimum length"
  strand = ""
  length = random.choice(xrange(minlength, maxlength + 1))
  bases = ['A', 'T', 'G', 'C']
  for i in xrange(0, length):
    strand += random.choice(bases)
  return strand

# Method that just prints out the supplied alignment score.
# This is more of a placeholder for what will ultimately
# print out not only the score but the alignment as well.

def printAlignment(alignment, out = sys.stdout):  
  strand1 = alignment['strand1']
  strand2 = alignment['strand2']
  
  out.write((('Optimal alignment score = ' + str(alignment['score'])) + '\n\n'))
  out.write('+ ')
    
  for i in xrange(0, len(strand1)):
    if strand1[i] == strand2[i]:
      out.write('1')
    else:
      out.write(' ')
  
  out.write('\n')
  out.write('  ' + strand1 + '\n')
  out.write('  ' + strand2 + '\n')
  out.write('- ')  
    
  for i in xrange(0, len(strand1)):
    if strand1[i] == strand2[i]:
      out.write(' ')
    elif strand1[i] == ' ' or strand2[i] == ' ':
      out.write('2')
    else:
      out.write('1')

  out.write('\n\n')    

# Unit test main in place to do little more than
# exercise the above algorithm.  As written, it
# generates two fairly short DNA strands and
# determines the optimal alignment score.
#
# As you change the implementation of findOptimalAlignment
# to use memoization, you should change the 8s to 40s and
# the 10s to 60s and still see everything execute very
# quickly.
 
def main():
  while (True):
    sys.stdout.write("Generate random DNA strands? ")
    answer = sys.stdin.readline()
    if answer == "no\n": break
    strand1 = generateRandomDNAStrand(40, 60)
    strand2 = generateRandomDNAStrand(40, 60)
    sys.stdout.write("Aligning these two strands:\n\n" + strand1 + "\n")
    sys.stdout.write(strand2 + "\n\n")
    cache = {}
    alignment = findOptimalAlignment(strand1, strand2, cache)
    printAlignment(alignment)
    
if __name__ == "__main__":
  main()
