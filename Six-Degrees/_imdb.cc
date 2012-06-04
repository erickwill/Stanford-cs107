using namespace std;
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "imdb.h"

const char *const imdb::kActorFileName = "actordata";
const char *const imdb::kMovieFileName = "moviedata";

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

// you should be implementing these two methods right here... 

bool imdb::getCredits(const string& player, vector<film>& films) const
{
    int numberOfActors,data;
    char *actorName;
    memcpy(&numberOfActors,(int*)actorInfo.fileMap, 4);
    // loop though the number of actors
    for (int i =2; i < numberOfActors; i++)
    {
        memcpy(&data, ((int*)actorInfo.fileMap)+i,4);
        actorName = ((char*)actorInfo.fileMap + (data));
        if (strcmp(actorName,player.c_str())==0)
        {
            // find the padding lengths
            int actorNameLength = strlen(actorName)+1;
            int movieNumberPad = actorNameLength % 2;
            int fourOffset = (actorNameLength + movieNumberPad + 2) % 4;
            short numberMovies;
            memcpy(&numberMovies,((short*)((char*)actorInfo.fileMap + data + actorNameLength + movieNumberPad)),2);
            for (int j=0; j < numberMovies; j++)
            {
                int movieOffset;
                film theFilm;
                memcpy(&movieOffset, ((int*)((char*)actorInfo.fileMap + data + actorNameLength + movieNumberPad + (j*4) + fourOffset + 2)),4);
                theFilm.title = ((char*)movieInfo.fileMap + movieOffset);
                theFilm.year = 1900 + (int)((char*)movieInfo.fileMap + movieOffset + theFilm.title.length() + 1)[0];
                films.push_back(theFilm);
            }
            return true;
        }
    }
    return false;
}

bool imdb::getCast(const film& movie, vector<string>& players) const
{
    int numberOfMovies, data;
    film theFilm;
    char *movieName;
    memcpy(&numberOfMovies, (int*)movieInfo.fileMap, 4);
    for (int i = 1; i < numberOfMovies; i++)
    {
        memcpy(&data, (int*)movieInfo.fileMap+i,4);
        movieName = ((char*)movieInfo.fileMap + data);
        theFilm.title = movieName;
        theFilm.year = 1900 + (int)((char*)movieInfo.fileMap + data + theFilm.title.length() + 1)[0];
        if (movie == theFilm)
        {
            int paddingFirst = theFilm.title.length() % 2;
            // paddingSecond = paddingfirst + length of title + 2 (year + \0) and then + 2 for the actors number
            int paddingSecond = (paddingFirst + theFilm.title.length() +2 + 2) %4;
            short numberOfActors;
            memcpy(&numberOfActors, (short*)((char*)movieInfo.fileMap + data + theFilm.title.length() + 2 + paddingFirst),2);
 
            // get the actors offsets and insert into the players list
            for (int k =0; k < numberOfActors; k++)
            {
                int offset;
                memcpy(&offset, ((int*)((char*)movieInfo.fileMap + data + strlen(movieName) + 2 + paddingFirst + 2 + paddingSecond + (k*4))), 4);
                players.push_back((char*)actorInfo.fileMap + offset);
            }
            return true;
        }
    }
    return false;
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
