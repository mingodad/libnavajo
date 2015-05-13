//********************************************************
/**
 * @file  example.cc 
 *
 * @brief libnavajo example code.
 *
 * @author T.Descombes (descombes@lpsc.in2p3.fr)
 *
 * @version 1  
 * @date 27/01/15
 */
//********************************************************

#include <signal.h> 
#include <string.h> 
#include "navajo/libnavajo.hh"
#include "navajo/LogStdOutput.hh"

WebServer *webServer = NULL;

void exitFunction( int dummy )
{
   if (webServer != NULL) webServer->stopService();
}

/***********************************************************************/

  unsigned long cpu_work=0, cpu_total=0;
  
  int getCpuLoad(void)
  {
    char buf[1024];
    static FILE     *proc_stat_fd;
    int result = -1;
    unsigned long work=0, total=0;

    if ((proc_stat_fd = fopen("/proc/stat", "r")) == NULL)
      return -1;

    while ((fgets(buf, sizeof(buf), proc_stat_fd)) != NULL)
    {
      if (!strncmp("cpu ", buf, 4))
      {
        unsigned long  user=0, nice=0, sys=0, idle=0, iowait=0, irq=0, softirq=0;
        int n = sscanf(buf+5, "%lu %lu %lu %lu %lu %lu %lu", &user, &nice, &sys, &idle, &iowait, &irq, &softirq);
        if (n<5) return -1;
        work=user+nice+sys;
        total=work+idle;
        if (n>=5) total+=iowait;
        if (n>=6) total+=irq;
        if (n==7) total+=softirq;
      }
    }
    fclose (proc_stat_fd);

    if (cpu_work || cpu_total)
      result = 100 * ( (float)(work - cpu_work) / (float)(total - cpu_total));

    cpu_work = work; cpu_total = total;
    return result;
    
  }

/***********************************************************************/

class MyDynamicRepository : public DynamicRepository
{
    class MyDynamicPage : public DynamicPage
    {
      protected:
        bool isValidSession(HttpRequest* request)
        {
          void *myAttribute = request->getSessionAttribute("username");
          return myAttribute != NULL;
        }
        bool checkCredentials(const string& login, const string& password)
        {
          return login == "libnavajo" && password == "libnavajo";
        }
    };
    
    class CpuValue: public MyDynamicPage
    {
      bool getPage(HttpRequest* request, HttpResponse *response)
      {
        if (!isValidSession(request)) return false;
        ostringstream ss;
        ss << getCpuLoad();
        return fromString(ss.str(), response);
      }
    } cpuValue;

    class Controller: public MyDynamicPage
    {
      bool getPage(HttpRequest* request, HttpResponse *response)
      {
        string param;

        if (request->getParameter("pageId", param) && param == "LOGIN")
        {
          string login, password;
          if (request->getParameter("inputLogin", login) && request->getParameter("inputPassword", password)
              && checkCredentials(login, password))
          {
            char *username = (char*)malloc((login.length()+1)*sizeof(char));
            strcpy(username, login.c_str());
            request->setSessionAttribute ( "username", (void*)username );
            response->forwardTo("gauge.html"); 
            return true;
          }
        } 

        if (request->getParameter("disconnect", param)) // Button disconnect
          request->removeSession();

        if (!isValidSession(request))
          response->forwardTo("login.html");
        else
          response->forwardTo("gauge.html");       
        
        return true;
      }

    } controller;
     
  public:
    MyDynamicRepository() : DynamicRepository()
    {
      add("cpuvalue.txt",&cpuValue);
      add("index.html",&controller);
    }
};

/***********************************************************************/

int main()
{
  // connect signals
  signal( SIGTERM, exitFunction );
  signal( SIGINT, exitFunction );
  
  LOG->addLogOutput(new LogStdOutput);

  webServer = new WebServer;

  LocalRepository myLocalRepo;
  myLocalRepo.addDirectory("", "./html"); 
  webServer->addRepository(&myLocalRepo);

  MyDynamicRepository myRepo;
  webServer->addRepository(&myRepo);

  webServer->startService();

  webServer->wait();
  
  LogRecorder::freeInstance();
  return 0;
}


