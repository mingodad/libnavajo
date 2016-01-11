//********************************************************
/**
 * @file  IpAddress.hh
 *
 * @brief Ip Address (V4 and V6) class definition
 *
 * @author T.Descombes (descombes@lpsc.in2p3.fr)
 *
 * @version 1
 * @date 19/02/15
 */
//********************************************************

#ifndef IPADDRESS_HH_
#define IPADDRESS_HH_

#ifdef USE_USTL

#include <libnavajo/with_ustl.h>

#else

#include <stdexcept>
#include <vector>
#include <string>
#include <sstream>
#include <libnavajo/with_ustl.h>

#endif // USE_USTL

#include <errno.h>
#include <string.h>

#include "pthread.h"

#ifdef WIN32

#include <winsock2.h>
#include <Windows.h>
#include <in6addr.h>
#include <ws2tcpip.h>
#define u_int32_t uint32_t
#define u_int8_t uint8_t
#define u_int16_t uint16_t
#include "stdint.h"

#define in_addr_t DWORD
#define index(s, c) strchr((char*)s, c)
#define rindex(s, c) strrchr((char*)s, c)

#define inet_ntop(f,b,l,o)         inet_ntop(f,(PVOID)(b),l,o)

#else

#include <netdb.h>
#include <arpa/inet.h>

#endif


#include <stdio.h>
#include <stdlib.h>

#define INET6_ADDRLEN 	16


/***********************************************************************
 * IpAdress struct definition
 * for IPv4 and IPv6 address
 */


class IpAddress
{
  public:

  typedef union
  {
    in_addr_t v4;
    in6_addr v6;
  } IP;

  IP ip;
  u_int8_t ipversion;

  inline void init() { ipversion=0; memset (ip.v6.s6_addr, 0, INET6_ADDRLEN); };

  IpAddress() { init(); };

  IpAddress(const in_addr_t &addrIPv4) :
	  ipversion(4) { ip.v4=addrIPv4; };

  IpAddress(const in6_addr &addrIPv6) :
	  ipversion(6) { memcpy ((void*)&(ip.v6), (void*)(&addrIPv6), INET6_ADDRLEN); };

  IpAddress(const nw::string& value)
  {
    init();
    if ( index(value.c_str(), ':') != NULL ) // IPv6
    {
      struct sockaddr_in6 tmp;
      if ( inet_pton(AF_INET6, value.c_str(), &(tmp.sin6_addr)) == 0 )
        return ;
      *this = tmp.sin6_addr;
    }
    else // IPv4
    {
      struct in_addr tmp;
      if ( inet_pton(AF_INET, value.c_str(), &tmp) == 0 )
        return ;

      *this = tmp.s_addr;
    }
  }

  inline bool isNull() const { return ipversion == 0; };

  inline IpAddress& operator= (const in_addr_t &addrIPv4) { this->ipversion=4; this->ip.v4=addrIPv4; return *this; };
  inline IpAddress& operator= (const in6_addr &addrIPv6) { this->ipversion=6; memcpy ((void*)&(ip.v6), (void*)(&addrIPv6), INET6_ADDRLEN); return *this; };
  inline IpAddress& operator= (const IpAddress& i) { if (i.ipversion == 4) *this=i.ip.v4; else *this=i.ip.v6; return *this;};

  inline bool operator== (IpAddress const &ipA) const
  {
    if (ipA.ipversion != this->ipversion) return false;

    if (this->ipversion == 4) return this->ip.v4 == ipA.ip.v4;

    // IPv6
    int i=INET6_ADDRLEN-1; bool res=true;
    for (; i>=0 && res; i--) res=ipA.ip.v6.s6_addr[i] == this->ip.v6.s6_addr[i];
    return (i == -1) && res;
  };

  bool operator<(const IpAddress& A) const
  {
 	  bool res=true;
	  if (ipversion == 4)
	    res = ip.v4 < A.ip.v4;
	  else //IPv6
	  {
 	    int i=0;
 	    for (; i<INET6_ADDRLEN-1 && ( ip.v6.s6_addr[i] == A.ip.v6.s6_addr[i] ); i++);

	    res = ip.v6.s6_addr[i] < A.ip.v6.s6_addr[i];
	  }

	  return res;
  };

  inline const bool isUndef() const { return ipversion == 0; };

  inline const nw::string str() const
  {
    nw::string res="";
    if (ipversion == 4)
    {
      struct in_addr iplocal;
      iplocal.s_addr=ip.v4;
      res+=inet_ntoa(iplocal);
    }
    else
    { // IPv6
      char ipStr[INET6_ADDRSTRLEN];
      if (inet_ntop(AF_INET6, &(ip.v6), ipStr, INET6_ADDRSTRLEN ) == NULL)
	res+="ERROR !";
      else
        res+=ipStr;
    }

    return res;
  };

  static pthread_mutex_t resolvIP_mutex;

  inline bool snresolve( char *hname, const size_t maxlength) const
  {
    int   error = 0;
    struct sockaddr_in sin;
    struct sockaddr_in6 sin6;

    pthread_mutex_lock( &resolvIP_mutex );

    if (ipversion == 4)
    {
      sin.sin_family=AF_INET;
      struct in_addr addr; addr.s_addr=ip.v4;
      sin.sin_addr=addr;
      error = getnameinfo((sockaddr*)&sin, sizeof sin, hname, maxlength, NULL, 0, NI_NAMEREQD);
    }
    else
    { // IPv6
      sin6.sin6_family=AF_INET6;
      sin6.sin6_addr=ip.v6;
      error = getnameinfo((sockaddr*)&sin6, sizeof sin6, hname, maxlength, NULL, 0, NI_NAMEREQD);
    }

    pthread_mutex_unlock( &resolvIP_mutex );

    return (!error);
  };

  inline static IpAddress* fromString(const nw::string& value)
  {
    IpAddress* newIp =  new IpAddress(value);

    if (newIp != NULL && !newIp->isNull() )
      return newIp;

    if (newIp != NULL) delete newIp;

    return NULL;
  }

};

/***********************************************************************
 * IpNetwork struct definition
 */

class IpNetwork
{
  public:

    IpAddress addr;
    u_int8_t mask;

    IpNetwork() { };
    IpNetwork(const IpAddress &A): addr(A)
      { if (A.ipversion == 4) mask=32; else mask=128; };
    IpNetwork(const IpAddress &a,const u_int8_t &m)
      { addr = a; mask=m;  };


  // TODO: IpNetwork(const string& value)


    inline bool operator<(const IpNetwork& A) const
    {
      if (A.addr.ipversion == 4)
      {
        if (addr.ipversion != 4) return false; // IpV6 > IpV4

        u_int32_t netmask=0, Anetmask=0;
     	  for (u_int8_t i=0; i < A.mask ; i++)
     	    Anetmask |= 1 << (31-i);
        Anetmask=htonl(Anetmask);

     	  for (u_int8_t i=0; i < mask ; i++)
     	    netmask |= 1 << (31-i);
        netmask=htonl(netmask);
        return (addr.ip.v4 & netmask) < (A.addr.ip.v4 & Anetmask);
      }

      if (A.addr.ipversion == 6)
      {
        if (addr.ipversion != 6) return true; // IpV6 > IpV4

        bool res=true;
        int i=0;
        u_int8_t netmask=0, Anetmask=0;

        for (; i<INET6_ADDRLEN-1 && res; i++)
                ;

        {
     	    for (u_int8_t j=i*8; j<(i+1)*8 ; j++)
          {
     	      if (j < A.mask) Anetmask |= 1 << (8-(j-i*8));
     	      if (j < mask) netmask |= 1 << (8-(j-i*8));
          }

          res = ( addr.ip.v6.s6_addr[i] & netmask ) == ( A.addr.ip.v6.s6_addr[i] & Anetmask );
        }
        return ( addr.ip.v6.s6_addr[i] & netmask ) < ( A.addr.ip.v6.s6_addr[i] & Anetmask );
      }

      return false;
    };

    inline nw::string strCIDR() const
    {
      nw::string netCIDR=addr.str()+"/";
      nw::ostringstream masklengthSs; masklengthSs << (int)mask;
      netCIDR+=masklengthSs.str();
      return netCIDR;
    };

    /**
      * Is this IP address belonging to this network?
      * @param ip is an ip address
      * @param net is a vector of IpNetwork
      * @return true if it belongs to local network, false otherwise
      */

    inline bool isInside(const IpAddress& ip) const
    {
      bool res = false;

      if (ip.ipversion == 4)
      {
        if (addr.ipversion == 4)
        {
          u_int32_t netmask=0;
          for (u_int8_t j=0; j < mask ; j++)
     	      netmask |= 1 << (31-j);
          netmask=htonl(netmask);
          res = ( addr.ip.v4 & netmask ) == ( ip.ip.v4 & netmask ) ;
        }
      }

      if (ip.ipversion == 6)
      {
        if (addr.ipversion == 6)
        {
          res=true;
          int i=0;

          for (; i<INET6_ADDRLEN && res; i++)
          {
            u_int8_t netmask=0;
            for (u_int8_t j=i*8; j<(i+1)*8 ; j++)
     	        if (j < mask) netmask |= 1 << (8-(j-i*8));

            res = ( ( addr.ip.v6.s6_addr[i] & netmask ) == (ip.ip.v6.s6_addr[i] & netmask) );
          }
        }
      }

      return res;
    };

    inline static IpNetwork* fromString(const nw::string& value)
    {
      IpNetwork *ipNet=NULL;
      nw::string ipstr;
      size_t found=value.find_first_of('/');
      if (found == nw::string::npos)
      {
        IpAddress *addr=IpAddress::fromString(value);
        if (addr==NULL) return NULL;
        ipNet=new IpNetwork(*addr);
        delete addr;
      }
      else
      {
        ipstr=value.substr(0, found);

        IpAddress *addr=IpAddress::fromString(ipstr);

        if (addr==NULL) return NULL;

        u_int8_t maskDec=0;
        nw::string maskStr=value.substr(found+1);

        // Mask
        if ( maskStr.find_first_of('.') != nw::string::npos ) // mask is formating like "w.x.y.z"
        {
	        if (addr->ipversion == 6)
	        {
	          delete addr;
            return NULL;
          }

	        struct in_addr tmp;
	        if ( inet_pton(AF_INET, maskStr.c_str(), &tmp) == 0 )
        //	if (inet_aton(maskStr.c_str(), &tmp) == 0)
	        {
	          delete addr;
            return NULL;
	        }
	        else
	        {
	          u_int32_t netmask=ntohl(tmp.s_addr);
         	  bool thisistheend=false;
	          maskDec=0;
	          for (u_int8_t j=0; j < 32 ; j++)
	            if ((netmask >> (31-j)) & 1)
	            {
	              if (!thisistheend)
        		      maskDec++;
	              else
	              {
        		      delete addr;
	                return NULL;
	              }
	            }
	            else
      		      thisistheend=true;
	        }
        }
        else
        {
	        size_t s=0, e=0, j=0;

	        while (( maskStr[s] == ' ' || maskStr[s] == '\t' ) && s < maskStr.length())
	          s++;
	        e=s;

          if (e < maskStr.length())

    	    while ( e < maskStr.length() && maskStr[e] >= '0' && maskStr[e] <= '9'  )
		        e++;

	        j=e;

	        while (j < maskStr.length() && ( maskStr[j] == ' ' || maskStr[j] == '\t' || maskStr[j] == '\n' || maskStr[j] == '\r') )
	          j++;

	        if (e==s || j!=maskStr.length()) return NULL;

          maskDec=atoi(maskStr.substr(s,e-s+1).c_str());
        }

        if ((maskDec > 32 && addr->ipversion == 4) || (maskDec > 128 && addr->ipversion == 6))
        {
        	delete addr;
          return NULL;
        }

        ipNet=new IpNetwork(*addr, maskDec);
        delete addr;
      }
      return ipNet;
    }
} ;


/**
  * Is this IP address belonging to this list of networks ?
  * @param ip is an ip address
  * @param net is a vector of IpNetwork
  * @return true if it belongs to local network, false otherwise
  */

inline static bool isIpBelongToIpNetwork(const IpAddress& ip, const nw::vector<IpNetwork>& net)
{
  bool res = false;

  for( nw::vector<IpNetwork>::const_iterator i=net.begin(); i!=net.end() && !res; i++)
    res = i->isInside(ip);

  return res;
};

/***********************************************************************/

#endif
