//****************************************************************************
/**
 * @file  HttpRequest.hh
 *
 * @brief The Http Request Parameters class
 *
 * @author T.Descombes (descombes@lpsc.in2p3.fr)
 *
 * @version 1
 * @date 27/01/15
 */
//****************************************************************************

#ifndef HTTPREQUEST_HH_
#define HTTPREQUEST_HH_

#include "libnavajo/IpAddress.hh"
#ifdef USE_USTL
#include <ustl.h>
namespace nw=ustl;
#else
#include <map>
#include <vector>
#include <string>
#include <sstream>
namespace nw=std;
#endif // USE_USTL
#include <openssl/ssl.h>
#include "HttpSession.hh"

//****************************************************************************

typedef enum { UNKNOWN_METHOD = 0, GET_METHOD = 1, POST_METHOD = 2, PUT_METHOD = 3, DELETE_METHOD = 4 } HttpRequestMethod;
typedef enum { GZIP, ZLIB, NONE } CompressionMode;
typedef struct
{
  int socketId;
  IpAddress ip;
  CompressionMode compression;
  SSL *ssl;
  BIO *bio;
  string *peerDN;
} ClientSockData;

class HttpRequest
{
  typedef nw::map <nw::string, nw::string> HttpRequestParametersMap;
  typedef nw::map <nw::string, nw::string> HttpRequestCookiesMap;

  const char *url;
  const char *origin;
  ClientSockData *clientSockData;
  nw::string httpAuthUsername;
  HttpRequestMethod httpMethod;
  HttpRequestCookiesMap cookies;
  HttpRequestParametersMap parameters;
  nw::string sessionId;

  /**********************************************************************/
  /**
  * decode all http parameters and fill the parameters Map
  * @param p: raw string containing all the http parameters
  */
  inline void decodParams( const nw::string& p )
  {
    size_t start = 0, end = 0;
    nw::string paramstr=p;

    while ((end = paramstr.find_first_of("%+", start)) != nw::string::npos)
    {
      switch (paramstr[end])
      {
        case '%':
          if (paramstr[end+1]=='%')
            paramstr=paramstr.erase(end+1,1);
          else
          {
            unsigned int specar;
            nw::string hexChar=paramstr.substr(end+1,2);
            //nw::stringstream ss; ss << nw::hex << hexChar.c_str();
            //ss >> specar;
            sscanf(hexChar.c_str(), "%2X", &specar);
            paramstr[end] = (char)specar;
            paramstr=paramstr.erase(end+1,2);
          }
          break;

        case '+':
          paramstr[end]=' ';
          break;
      }

      start=end+1;
    }

    start = 0; end = 0;
    bool islastParam=false;
    while (!islastParam)
    {
      islastParam= (end = paramstr.find('&', start)) == nw::string::npos;
      if (islastParam) end=paramstr.size();

      nw::string theParam=paramstr.substr(start, end - start);

      size_t posEq=0;
      if ((posEq = theParam.find('=')) == nw::string::npos)
        parameters[theParam]="";
      else
        parameters[theParam.substr(0,posEq)]=theParam.substr(posEq+1);

      start = end + 1;
    }
  };

  /**********************************************************************/
  /**
  * decode all http cookies and fill the cookies Map
  * @param c: raw string containing all the cockies definitions
  */

  inline void decodCookies( const nw::string& c )
  {
    nw::istringstream ss(c);
    nw::string theCookie;
    while (nw::getline(ss, theCookie, ';'))
    {
      size_t posEq=0;
      if ((posEq = theCookie.find('=')) != nw::string::npos)
      {
        size_t firstC=0; while (!isgraph(theCookie[firstC]) && firstC < posEq) { firstC++; };

        if (posEq-firstC > 0 && theCookie.length()-posEq > 0) cookies[theCookie.substr(firstC,posEq-firstC)]=theCookie.substr(posEq+1, theCookie.length()-posEq);
      }
    }
  };


  public:

    /**********************************************************************/
    /**
    * get cookie value
    * @param name: the cookie name
    */

    inline nw::string getCookie( const nw::string& name ) const
    {
      nw::string res="";
      getCookie(name, res);
      return res;
    }

    /**********************************************************************/
    /**
    * get cookie value
    * @param name: the cookie name
    * @param value: the cookie value
    * @return true is the cookie exist
    */
    inline bool getCookie( const nw::string& name, nw::string &value ) const
    {
      if(!cookies.empty())
      {
        HttpRequestCookiesMap::const_iterator it;
        if((it = cookies.find(name)) != cookies.end())
        {
          value=it->second;
          return true;
        }
      }
      return false;
    }

    /**********************************************************************/
    /**
    * get cookies list
    * @return a vector containing all cookies names
    */
    inline nw::vector<nw::string> getCookiesNames() const
    {
      nw::vector<nw::string> res;
      for(HttpRequestCookiesMap::const_iterator iter=cookies.begin(); iter!=cookies.end(); ++iter)
       res.push_back(iter->first);
      return res;
    }

    /**********************************************************************/
    /**
    * get parameter value
    * @param name: the parameter name
    * @param value: the parameter value
    * @return true is the parameter exist
    */
    inline bool getParameter( const nw::string& name, nw::string &value ) const
    {
      if(!parameters.empty())
      {
        HttpRequestParametersMap::const_iterator it;
        if((it = parameters.find(name)) != parameters.end())
        {
          value=it->second;
          return true;
        }
      }
      return false;
    }

    /**********************************************************************/
    /**
    * get parameter value
    * @param name: the parameter name
    * @return the parameter value
    */
    inline nw::string getParameter( const nw::string& name ) const
    {
      nw::string res="";
      getParameter(name, res);
      return res;
    }

    /**********************************************************************/
    /**
    * does the parameter exist ?
    * @param name: the parameter name
    * @return true is the parameter exist
    */
    inline bool hasParameter( const nw::string& name ) const
    {
      nw::string tmp;
      return getParameter(name, tmp);
    }

    /**********************************************************************/
    /**
    * get parameters list
    * @return a vector containing all parameters names
    */
    inline nw::vector<nw::string> getParameterNames() const
    {
      nw::vector<nw::string> res;
      for(HttpRequestParametersMap::const_iterator iter=parameters.begin(); iter!=parameters.end(); ++iter)
       res.push_back(iter->first);
      return res;
    }

    /**********************************************************************/
    /**
    * is there a session cookie
    * @param b: if true (default), create a server session if none exists
    */
    inline void getSession()
    {
      sessionId = getCookie("SID");

      if (sessionId.length() && HttpSession::find(sessionId))
        return;

      initSessionId();
    }

    /**
    * create the session cookie
    */
    inline void createSession()
    {
      HttpSession::create(sessionId);
    }

    /**
    * remove the session cookie
    */
    inline void removeSession()
    {
      if (sessionId == "") return;
      HttpSession::remove(sessionId);
    }

    /**
    * add an attribute to the session
    * @param name: the attribute name
    * @param value: the attribute value
    */
    void setSessionAttribute ( const nw::string &name, void* value )
    {
      if (sessionId == "") createSession();
      HttpSession::setAttribute(sessionId, name, value);
    }

    /**
    * get an attribute of the server session
    * @param name: the attribute name
    * @return the attribute value or NULL if not found
    */
    void *getSessionAttribute( const nw::string &name )
    {
      if (sessionId == "") return NULL;
      return HttpSession::getAttribute(sessionId, name);
    }

    /**
    * get the list of the attribute's Names of the server session
    * @return a vector containing all attribute's names
    */
    inline nw::vector<nw::string> getSessionAttributeNames()
    {
      if (sessionId == "") return nw::vector<nw::string>();;
      return HttpSession::getAttributeNames(sessionId);
    }

    /**
    * remove an attribute of the server session (if found)
    * @param name: the attribute name
    */
    inline void getSessionRemoveAttribute( const nw::string &name )
    {
      if (sessionId != "")
        HttpSession::removeAttribute( sessionId, name );
    }

    /**
    * initialize sessionId value
    */
    inline void initSessionId() { sessionId = ""; };

    /**
    * get sessionId value
    * @return the sessionId value
    */
    nw::string getSessionId() const { return sessionId; };

    /**********************************************************************/
    /**
    * HttpRequest constructor
    * @param type:  the Http Request Type ( GET/POST/...)
    * @param url:  the requested url
    * @param params:  raw http parameters string
    * @cookies params: raw http cookies string
    */
    HttpRequest(const HttpRequestMethod type, const char *url, const char *params, const char *cookies, const char *origin, const string &username, ClientSockData *client)
    {
      httpMethod = type;
      this->url = url;
      this->origin = origin;
      httpAuthUsername=username;
      this->clientSockData=client;

      if (params != NULL && strlen(params))
        decodParams(params);
      if (cookies != NULL && strlen(cookies))
        decodCookies(cookies);
      getSession();
    };

    /**********************************************************************/
    /**
    * get url
    * @return the requested url
    */
    inline const char *getUrl() const { return url; };

    /**********************************************************************/
    /**
    * get request type
    * @return the Http Request Type ( GET/POST/...)
    */
    inline HttpRequestMethod getRequestType() const { return httpMethod; };

    /**********************************************************************/
    /**
    * get request origin
    * @return the Http Request Origin
    */
    inline const char* getRequestOrigin() const { return origin; };

    /**********************************************************************/
    /**
    * get peer IP address
    * @return the ip address
    */
    inline IpAddress& getPeerIpAddress()
    {
      return clientSockData->ip;
    };

    /**********************************************************************/
    /**
    * get http authentification username
    * @return the login
    */
    inline string& getHttpAuthUsername()
    {
      return httpAuthUsername;
    };

    /**********************************************************************/
    /**
    * get peer x509 dn
    * @return the DN of the peer certificate
    */
    inline string& getX509PeerDN()
    {
      return *(clientSockData->peerDN);
    };

    /**********************************************************************/
    /**
    * get compression mode
    * @return the compression mode requested
    */
    inline CompressionMode getCompressionMode()
    {
      return clientSockData->compression;
    };

    /**********************************************************************/
    /**
    * get the http request client socket data
    * @return the clientSockData
    */
    ClientSockData *getClientSockData()
    {
      return clientSockData;
    }
};

//****************************************************************************

#endif
