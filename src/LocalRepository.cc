//********************************************************
/**
 * @file  LocalRepository.cc
 *
 * @brief Handles local web repository
 *
 * @author T.Descombes (thierry.descombes@gmail.com)
 *
 * @version 1
 * @date 19/02/15
 */
//********************************************************


#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>

#ifdef USE_USTL

#include <libnavajo/with_ustl.h>

#else

#include <fstream>
#include <streambuf>
#include <sstream>
#include <libnavajo/with_ustl.h>

#endif // USE_USTL

#include "libnavajo/LogRecorder.hh"
#include "libnavajo/LocalRepository.hh"


/**********************************************************************/

bool LocalRepository::loadFilename_dir (const nw::string& alias, const nw::string& path, const nw::string& subpath="")
{
    struct dirent *entry;
    DIR *dir;
    struct stat s;
    nw::string fullPath=path+subpath;

    dir = opendir (fullPath.c_str());
    if (dir == NULL) return false;
    while ((entry = readdir (dir)) != NULL )
    {
      if (!strcmp(entry->d_name,".") || !strcmp(entry->d_name,"..") || !strlen(entry->d_name)) continue;

      nw::string filepath=fullPath + "/" + entry->d_name;

      if (stat(filepath.c_str(), &s) == -1)
      {
	NVJ_LOG->append(NVJ_ERROR,nw::string("LocalRepository - stat error : ")+nw::string(strerror(errno)));
        continue;
      }

      int type=s.st_mode & S_IFMT;
      if (type == S_IFREG || type == S_IFLNK)
      {
	nw::string filename=alias+subpath+"/"+entry->d_name;
	while (filename.size() && filename[0]=='/') filename.erase(0, 1);
	filenamesSet.insert(filename);
      }

      if (type == S_IFDIR)
        loadFilename_dir(alias, path, subpath+"/"+entry->d_name);
    }

    return true;
}

/**********************************************************************/

void LocalRepository::addDirectory( const nw::string& alias, const nw::string& dirPath)
{
  char resolved_path[4096];

  nw::string newalias=alias;
  while (newalias.size() && newalias[0]=='/') newalias.erase(0, 1);
  while (newalias.size() && newalias[newalias.size()-1]=='/') newalias.erase(newalias.size() - 1);

  if (realpath(dirPath.c_str(), resolved_path) == NULL)
    return ;

  if (!loadFilename_dir(newalias, resolved_path))
	  return ;

  aliasesSet.insert( nw::pair<nw::string, nw::string>(newalias, resolved_path) );

}

/**********************************************************************/

void LocalRepository::clearAliases()
{
  filenamesSet.clear();
  aliasesSet.clear();
}

/**********************************************************************/

bool LocalRepository::fileExist(const nw::string& url)
{
  return filenamesSet.find(url) != filenamesSet.end();
}

/**********************************************************************/

void LocalRepository::printFilenames()
{
  for (nw::set<nw::string>::iterator it = filenamesSet.begin(); it != filenamesSet.end(); it++)
    printf ("%s\n", it->c_str() );
}

/**********************************************************************/

bool LocalRepository::getFile(HttpRequest* request, HttpResponse *response)
{
  bool found=false;
  const nw::string *alias=NULL, *path=NULL;
  nw::string url = request->getUrl();
  size_t webpageLen;
  unsigned char *webpage;
  pthread_mutex_lock( &_mutex );

  if (!fileExist(url)) { pthread_mutex_unlock( &_mutex); return false; };

  for (nw::set< nw::pair<nw::string,nw::string> >::iterator it = aliasesSet.begin(); it != aliasesSet.end() && !found; it++)
  {
    alias=&(it->first);
    if (!(url.compare(0, alias->size(), *alias)))
    {
      path=&(it->second);
      found= true;
    }
  }

  pthread_mutex_unlock( &_mutex);
  if (!found) return false;

  nw::string resultat, filename=url;

  if (alias->size())
    filename.replace(0, alias->size(), *path);
  else
    filename=*path+'/'+filename;

  FILE *pFile = fopen ( filename.c_str() , "rb" );
  if (pFile==NULL)
  {
    char logBuffer[150];
    snprintf(logBuffer, 150, "Webserver : Error opening file '%s'", filename.c_str() );
    NVJ_LOG->append(NVJ_ERROR, logBuffer);
    return false;
  }

  // obtain file size.
  fseek (pFile , 0 , SEEK_END);
  webpageLen = ftell (pFile);
  rewind (pFile);

  if ( (webpage = (unsigned char *)malloc( webpageLen+1 * sizeof(char))) == NULL )
    return false;
  size_t nb=fread (webpage,1,webpageLen,pFile);
  if (nb != webpageLen)
  {
    char logBuffer[150];
    snprintf(logBuffer, 150, "Webserver : Error accessing file '%s'", filename.c_str() );
    NVJ_LOG->append(NVJ_ERROR, logBuffer);
    return false;
  }

  fclose (pFile);
  response->setContent (webpage, webpageLen);
  return true;
}




