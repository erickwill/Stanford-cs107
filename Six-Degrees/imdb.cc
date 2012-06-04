using namespace std;
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include "imdb.h"
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <string>

const char *const imdb::kActorFileName = "actordata";
const char *const imdb::kMovieFileName = "moviedata";

struct Key {
  const char *keyString;
  const void *data;
};

imdb::imdb(const string& directory)
{
  const string actorFileName = directory + "/" + kActorFileName;
  const string movieFileName = directory + "/" + kMovieFileName;
  
  actorFile = acquireFileMap(actorFileName, actorInfo);
  movieFile = acquireFileMap(movieFileName, movieInfo);
}

bool imdb::good() const
{
  return !( (actorInfo.fd == -1) || 
	    (movieInfo.fd == -1) ); 
}

int cmp(const void *a, const void *b) 
{  
  Key *k = (Key*)a;
  int *offset = (int*)b;
  char *test = (char*)k->data + *offset;  
  return strcmp(k->keyString, test);
}

// you should be implementing these two methods right here... 
bool imdb::getCredits(const string& player, vector<film>& films) const 
{
  int numActors;
  memcpy(&numActors, (int*)actorFile, sizeof(int));
  int actorStartOffset = 2;

  Key k;
  k.keyString = player.c_str();
  k.data = actorFile;

  int *ptr = (int*)bsearch(&k, (int*)actorFile+actorStartOffset,numActors, sizeof(int*), cmp);
  if (ptr == NULL) {
    return false;
  }

  int foundAt = *ptr;
  int nameLen = player.length()+1;
  int nameOffset = nameLen + (nameLen % 2);
  short numMovies;
  int moviesOffset = foundAt + nameOffset;
  memcpy(&numMovies, (char*)actorFile + moviesOffset, sizeof(short));

  moviesOffset += sizeof(short);
  if (moviesOffset % 4 != 0)
    moviesOffset += 2;
  for (int i = 0; i < numMovies; i++) {
    film movie;
    int titleOffset;
    memcpy(&titleOffset, (char*)actorFile + moviesOffset + (i * sizeof(int)), sizeof(int));
    char *movieTitle = (char*)movieFile + titleOffset;
    char movieDateChar;
    memcpy(&movieDateChar, (char*)movieFile + titleOffset + strlen(movieTitle)+1, sizeof(char));
    int movieDate = 1900 + (int)movieDateChar;
    movie.title = movieTitle;
    movie.year = movieDate;
    films.push_back(movie);
  }

  return true;


}
bool imdb::getCast(const film& movie, vector<string>& players) const { return false; }

imdb::~imdb()
{
  releaseFileMap(actorInfo);
  releaseFileMap(movieInfo);
}

// ignore everything below... it's all UNIXy stuff in place to make a file look like
// an array of bytes in RAM.. 
const void *imdb::acquireFileMap(const string& fileName, struct fileInfo& info)
{
  struct stat stats;
  stat(fileName.c_str(), &stats);
  info.fileSize = stats.st_size;
  info.fd = open(fileName.c_str(), O_RDONLY);
  return info.fileMap = mmap(0, info.fileSize, PROT_READ, MAP_SHARED, info.fd, 0);
}

void imdb::releaseFileMap(struct fileInfo& info)
{
  if (info.fileMap != NULL) munmap((char *) info.fileMap, info.fileSize);
  if (info.fd != -1) close(info.fd);
}
