//********************************************************
/**
 * @file  DynamicPage.hh
 *
 * @brief Dynamic page définition (abstract class)
 *
 * @author T.Descombes (descombes@lpsc.in2p3.fr)
 *
 * @version 1
 * @date 19/02/15
 */
//********************************************************

#ifndef DYNAMICPAGE_HH_
#define DYNAMICPAGE_HH_

#ifdef USE_USTL
#include <ustl.h>
namespace nw=ustl;
#else
#include <string>
#include <typeinfo>
namespace nw=std;
#endif // USE_USTL

class DynamicPage
{

  public:
    DynamicPage() {};
    virtual bool getPage(HttpRequest* request, HttpResponse *response) = 0;


    /**********************************************************************/

    template<class T> static inline T getValue (string s)
    {
      if (!s.length())
       throw nw::bad_cast();

	     nw::istringstream iss(s);
	     T tmp; iss>>tmp;

	     if(iss.fail())
	       throw nw::bad_cast();

	      return tmp;

	  };

    /**********************************************************************/

    inline bool noContent( HttpResponse *response )
    {
      response->setContent (NULL, 0);
      return true;
    }

    /**********************************************************************/

    inline bool fromString( const string& resultat, HttpResponse *response )
    {
      size_t webpageLen;
      unsigned char *webpage;
      if ( (webpage = (unsigned char *)malloc( resultat.size()+1 * sizeof(char))) == NULL )
          return false;
      webpageLen=resultat.size();
      strcpy ((char *)webpage, resultat.c_str());
      response->setContent (webpage, webpageLen);
      return true;
    }


};

#endif
