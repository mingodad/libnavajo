//********************************************************
/**
 * @file  LogSyslog.hh
 *
 * @brief write log messages to syslog
 *
 * @author T.Descombes (thierry.descombes@gmail.com)
 *
 * @version 1
 * @date 19/02/15
 */
//********************************************************

#ifndef LOGSYSLOG_HH_
#define LOGSYSLOG_HH_

#ifdef USE_USTL
#include <ustl.h>
namespace nw=ustl;
#else
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
namespace nw=std;
#endif // USE_USTL


#include "libnavajo/LogOutput.hh"

using namespace std;

  /**
  * LogSyslog - LogOutput
  */
  class LogSyslog : public LogOutput
  {
    public:
      LogSyslog(const char *id="Navajo");
      ~LogSyslog();

      void append(const NvjLogSeverity& l, const nw::string& m, const nw::string& details="");
      void initialize();

    private:
      char ident[30];


  };


#endif
