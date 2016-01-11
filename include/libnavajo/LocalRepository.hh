//********************************************************
/**
 * @file  LocalRepository.hh
 *
 * @brief Handles local web repository
 *
 * @author T.Descombes (thierry.descombes@gmail.com)
 *
 * @version 1
 * @date 19/02/15
 */
//********************************************************

#ifndef LOCALREPOSITORY_HH_
#define LOCALREPOSITORY_HH_

#include "WebRepository.hh"

#ifdef USE_USTL

#include <libnavajo/with_ustl.h>

#else

#include <set>
#include <string>
#include <libnavajo/with_ustl.h>

#endif // USE_USTL

#include "libnavajo/thread.h"


class LocalRepository : public WebRepository
{
    pthread_mutex_t _mutex;

    nw::set< nw::string > filenamesSet; // list of available files
    nw::set< nw::pair<nw::string,nw::string> > aliasesSet; // alias name | Path to local directory

    bool loadFilename_dir(const nw::string& alias, const nw::string& path, const nw::string& subpath);
    bool fileExist(const nw::string& url);


  public:
    LocalRepository () { pthread_mutex_init(&_mutex, NULL); };
    virtual ~LocalRepository () { clearAliases(); };

    virtual bool getFile(HttpRequest* request, HttpResponse *response);
    virtual void freeFile(unsigned char *webpage) { ::free(webpage); };
    void addDirectory(const nw::string& alias, const nw::string& dirPath);
    void clearAliases();
    void printFilenames();
};

#endif

