//********************************************************
/**
 * @file  LogFile.hh
 *
 * @brief Write log messages to a file
 *
 * @author T.Descombes (thierry.descombes@gmail.com)
 *
 * @version 1
 * @date 19/02/15
 */
//********************************************************

#ifndef LOGFILE_HH_
#define LOGFILE_HH_

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

using namespace nw;

  /**
  * LogFile - LogOutput
  */
  class LogFile : public LogOutput
  {
    public:
      LogFile(const char *filename);
      ~LogFile();

      void append(const NvjLogSeverity& l, const nw::string& m, const nw::string& details="");
      void initialize();

    private:
      char filename[30];
      ofstream *file;

  };


#endif
