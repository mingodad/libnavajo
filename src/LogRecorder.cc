//********************************************************
/**
 * @file  LogRecorder.cc
 *
 * @brief Log Manager class
 *
 * @author T.Descombes (thierry.descombes@gmail.com)
 *
 * @version 1
 * @date 19/02/15
 */
//********************************************************



#include <pthread.h>
#include <time.h>
#include "libnavajo/LogRecorder.hh"


  /**
  * LogRecorder - static and unique log recorder object
  */
  LogRecorder * LogRecorder::theLogRecorder = NULL;

  /***********************************************************************/
  /**
  * getDateStr - return a string with the formatted date
  * \return string - formatted date
  */
  nw::string LogRecorder::getDateStr()
  {
    struct tm today;
    char tmpbuf[128];
    time_t ltime;

    time( &ltime );
    gmtime_r(&ltime, &today);

    nw::string ret_str;
    strftime( tmpbuf, 128, "[%Y-%m-%d %H:%M:%S] >  ", &today );
    ret_str=tmpbuf;
    return ret_str;

  }

  /***********************************************************************/
  /**
  * append - append an entry to the LogRecorder
  * \param l - type of entry
  * \param m - message
  */
  void LogRecorder::append(const NvjLogSeverity& l, const nw::string& m, const nw::string& details)
  {
    pthread_mutex_lock( &log_mutex );

    if (l != NVJ_DEBUG || debugMode)
    {
      for( nw::list<LogOutput *>::iterator it=logOutputsList_.begin();
           it!=logOutputsList_.end();
     it++ )
      {
        nw::string msg;

        if ((*it)->isWithDateTime())
          msg=getDateStr() + m;
        else msg=m;

        if ((*it)->isWithEndline())
          msg+= nw::string("\n") ;

        (*it)->append(l, msg, details);
      }
    }

    pthread_mutex_unlock( &log_mutex );

  }

  /***********************************************************************/
  /**
  * addLogOutput - ajout d'une sortie LogOutput où imprimer les logs
  */

  void LogRecorder::addLogOutput(LogOutput *output)
  {
    output->initialize();
    logOutputsList_.push_back(output);
  }

  /***********************************************************************/
  /**
  * removeLogOutputs - supprime toutes les sorties LogOutput
  */
  void LogRecorder::removeLogOutputs()
  {
    for( nw::list<LogOutput *>::iterator it=logOutputsList_.begin();
           it!=logOutputsList_.end();
     it++ )
      delete *it;

    logOutputsList_.clear();
  }

  /***********************************************************************/
  /**
  * LogRecorder - base constructor
  */

  LogRecorder::LogRecorder()
  {
    debugMode=false;
    pthread_mutex_init(&log_mutex, NULL);
  }

  /***********************************************************************/
  /**
  * ~LogRecorder - destructor
  */
  LogRecorder::~LogRecorder()
  {
    removeLogOutputs();
  }

  /***********************************************************************/


