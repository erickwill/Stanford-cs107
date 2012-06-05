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


/**
* Struct for bsearch comparison.
**/
struct Key {
  const char *keyString;
  const void *data;
  bool isMovie;
  int year;
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
  Key *k = (Key*)a; // Key struct
  int *offset = (int*)b; // pointer to comparing data
  char *test = (char*)k->data + *offset;
  int value =  strcmp(k->keyString, test);
 
  if (!k->isMovie) // if it is a film struct, must also check year
    return value;

  else if (value == 0){
    char testYearChar;
    int testLen = strlen(test)+1;
    memcpy(&testYearChar, (char*)k->data + testLen + *offset, sizeof(char)); // year byte
    int testYear = (int)testYearChar + 1900;
    int diff = k->year - testYear;

    if (diff > 0) return 1;
    else if (diff < 0) return -1;
    else return 0;

  } else 
    return value; // names of films did not match
}

// you should be implementing these two methods right here... 
bool imdb::getCredits(const string& player, vector<film>& films) const 
{
  int numActors; // total actors in data file
  memcpy(&numActors, (int*)actorFile, sizeof(int));

  Key k; // struct for bsearch comparison
  k.keyString = player.c_str();
  k.data = actorFile;
  k.isMovie = false;
  int beginActorsOffset = 1; // account for size storage entry

  // search for k.keyString in actorFile (starting at 1 offset)
  // search the total number of actors
  // each entry to be searched is the size of a pointer
  int *ptr = (int*)bsearch(&k, (int*)actorFile+beginActorsOffset, numActors, sizeof(int*), cmp);
  if (ptr == NULL) {
    return false;
  }

  int foundAt = *ptr; // found match at this address
  int nameLen = player.length()+1;
  int nameOffset = nameLen + (nameLen % 2); // actor name padding
  int moviesOffset = foundAt + nameOffset;
  short numMovies;
  memcpy(&numMovies, (char*)actorFile + moviesOffset, sizeof(short));

  moviesOffset += sizeof(short);
  if (moviesOffset % 4 != 0)
    moviesOffset += 2; // padding for films address
  for (int i = 0; i < numMovies; i++) {
    film movie; // new film to pass into vector
    int titleOffset;
    // total offset = foundAt address + actor name length  + number of movies + (total padding) + (i * sizeof(int))
    memcpy(&titleOffset, (char*)actorFile + moviesOffset + (i * sizeof(int)), sizeof(int));
    char *movieTitle = (char*)movieFile + titleOffset; // film title string
    char movieDateChar; // 1 byte film year
    memcpy(&movieDateChar, (char*)movieFile + titleOffset + strlen(movieTitle)+1, sizeof(char));
    int movieDate = 1900 + (int)movieDateChar; // cast char to int and add year delta
    movie.title = movieTitle;
    movie.year = movieDate;
    films.push_back(movie); // add to vector
  }
  return true;
  
 }
bool imdb::getCast(const film& movie, vector<string>& players) const 
{
  Key k; // struct for bsearch
  k.keyString = movie.title.c_str();
  k.data = movieFile;
  k.isMovie = true; // will need to compare film.year 
  k.year = movie.year;

  int numMovies; // total number of films
  memcpy(&numMovies, (int*)movieFile, sizeof(int));
  int beginMoviesOffset = 1; // account for first entry storing size

  // search for k.keyString in movieFile (starting at 1 offset)
  // will need to compare year as well as title
  // search the total number of films
  // each entry to be searched is the size of a pointer
  int *ptr = (int*)bsearch(&k, (int*)movieFile + beginMoviesOffset, numMovies, sizeof(int*), cmp);
  if (ptr == NULL) {
    return false;
  }

  int foundAt = *ptr; // found film at this address
  int nameLen = movie.title.length()+1;
  int actorsOffset = foundAt + nameLen + sizeof(char);
  if ((nameLen + sizeof(char)) % 2 != 0)
    actorsOffset++; // name padding

  short numActors;
  memcpy(&numActors, (char*)movieFile + actorsOffset, sizeof(short));
  actorsOffset += sizeof(short);
  if ((actorsOffset-foundAt) % 4 != 0)
    actorsOffset += 2; // padding to find actor address

  for (int i = 0; i < numActors; i++) {
    int actorNameOffset;
    memcpy(&actorNameOffset, (char*)movieFile + actorsOffset + (i * sizeof(int)), sizeof(int));
    char *actorName = (char*)actorFile + actorNameOffset;
    players.push_back(actorName);
  }
  return true;
}

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
