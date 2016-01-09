//****************************************************************************
/**
 * @file  HttpResponse.hh
 *
 * @brief The Http Response Parameters class
 *
 * @author T.Descombes (descombes@lpsc.in2p3.fr)
 *
 * @version 1
 * @date 27/01/15
 */
//****************************************************************************

#ifndef HTTPRESPONSE_HH_
#define HTTPRESPONSE_HH_


class HttpResponse
{
  unsigned char *responseContent;
  size_t responseContentLength;
  nw::vector<nw::string> responseCookies;
  bool zippedFile;
  nw::string mimeType;
  nw::string forwardToUrl;
  bool cors, corsCred;
  string corsDomain;

  public:
    HttpResponse(nw::string mime="") : responseContent (NULL), responseContentLength (0), zippedFile (false), mimeType(mime), forwardToUrl(""), cors(false), corsCred(false), corsDomain("")
    {
    }

    /************************************************************************/
    /**
    * set the response body
    * @param content: The content's buffer
    * @param length: The content's length
    */
    inline void setContent(unsigned char *content, size_t length)
    {
      responseContent = content;
      responseContentLength = length;
    }

    /************************************************************************/
    /**
    * Returns the response body of the HTTP method
    * @param content: The content's buffer
    * @param length: The content's length
    * @param cookies: The cookies entries
    * @param zip: set to true if the content is compressed (else: false)
    */
    inline void getContent(unsigned char **content, size_t *length, bool *zip)
    {
      *content = responseContent;
      *length = responseContentLength;
      *zip = zippedFile;
    }

    /************************************************************************/
    /**
    * Set if the content is compressed (zip) or not
    * @param b: true if the content is compressed, false if not.
    */
    inline void setIsZipped(bool b=true) { zippedFile=b; };

    /************************************************************************/
    /**
    * return true if the content is compressed (zip)
    */
    inline bool isZipped() const { return zippedFile; };

    /************************************************************************/
    /**
    * insert a cookie entry (rfc6265)
    *   format: "<name>=<value>[; <Max-Age>=<age>][; expires=<date>]
    *      [; domain=<domain_name>][; path=<some_path>][; secure][; HttpOnly]"
    * @param name: the cookie's name
    * @param value: the cookie's value
    * @param maxage: the cookie's max-age
    * @param expiresTime: the cookie's expiration date
    * @param path: the cookie's path
    * @param domain: the cookie's domain
    * @param secure: the cookie's secure flag
    * @param httpOnly: the cookie's httpOnly flag
    */

    inline void addCookie(const nw::string& name, const nw::string& value, const time_t maxage=0, const time_t expiresTime=0, const nw::string& path="/", const nw::string& domain="", const bool secure=false, bool httpOnly=false)
    {
      nw::string cookieEntry=name+'='+value;

      if (maxage)
      {
        nw::ostringstream maxageSs; maxageSs << maxage;
        cookieEntry+="; Max-Age="+maxageSs.str();
      }

      if (expiresTime)
      {
        char expBuf[100];
        struct tm timeinfo;
        gmtime_r ( &expiresTime, &timeinfo );
        strftime (expBuf,100,"%a, %d %b %Y %H:%M:%S GMT", &timeinfo);
        cookieEntry+="; expires="+nw::string(expBuf);
      }

      if (domain.length()) cookieEntry+="; domain="+domain;

      if (path != "/" && path.length()) cookieEntry+="; path="+path;

      if (secure) cookieEntry+="; secure";

      if (httpOnly) cookieEntry+="; HttpOnly";

      responseCookies.push_back(cookieEntry);
    }

    /************************************************************************/
    /**
    * insert the session's cookie
    * @param sid: the session id
    */
    inline void addSessionCookie(const nw::string& sid)
    {
      addCookie("SID",sid, HttpSession::getSessionLifeTime(), 0, "", "", false,  true);
    }

    /************************************************************************/
    /**
    * get the http response's cookies
    * @return the cookies vector
    */
    inline nw::vector<nw::string>& getCookies()
    {
      return responseCookies;
    };

    /************************************************************************/
    /**
    * set a new mime type (by default, mime type is automatically set)
    * @param mime: the new mime type
    */
    inline void setMimeType(const nw::string& mime)
    {
      mimeType=mime;
    }

    /************************************************************************/
    /**
    * get the current mime type
    * @return the mime type
    */
    inline const nw::string& getMimeType() const
    {
      return mimeType;
    }

    /************************************************************************/
    /**
    * Request redirection to a new url
    * @param url: the new url
    */
    void forwardTo(const nw::string& url)
    {
      forwardToUrl=url;
    }

    /************************************************************************/
    /**
    * get the new url
    * @return the new url
    */
    nw::string getForwardedUrl()
    {
      return forwardToUrl;
    }

    /************************************************************************/
    /**
    * allow Cross Site Request
    * @param cors: enabled or not
    * @param cred: enabled credentials or not
    */
    void setCORS(bool cors=true, bool cred=false, string domain="*")
    {
      this->cors=cors;
      corsCred=cred;
      corsDomain=domain;
    }

    /**
    * is Cross Site Request allowed ?
    * @return boolean
    */
    bool isCORS()
    {
      return cors;
    }

    bool isCORSwithCredentials()
    {
      return corsCred;
    }

    string& getCORSdomain()
    {
      return corsDomain;
    };
};



//****************************************************************************

#endif
