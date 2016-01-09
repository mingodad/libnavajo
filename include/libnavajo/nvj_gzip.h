//********************************************************
/**
 * @file  gzip.cc
 *
 * @brief zip compression facilities
 *
 * @author T.Descombes (thierry.descombes@gmail.com)
 *
 * @version 1
 * @date 19/02/15
 */
//********************************************************

#ifdef USE_USTL
#include <ustl.h>
namespace nw=ustl;
#else
#include <stdexcept>
namespace nw=std;
#endif // USE_USTL
#include "zlib.h"

#define CHUNK 16384

//********************************************************

inline size_t nvj_gzip( unsigned char** dst, const unsigned char* src, const size_t sizeSrc, bool rawDeflateData=false )
{
  z_stream strm;
  size_t sizeDst=CHUNK;

  /* allocate deflate state */
  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;
  strm.opaque = Z_NULL;

  if ( deflateInit2(&strm, Z_BEST_SPEED, Z_DEFLATED, rawDeflateData ? -15 : 16+MAX_WBITS, 9, Z_DEFAULT_STRATEGY) != Z_OK)
    throw nw::runtime_error(nw::string("gzip : inflateInit2 error") );

  if ( (*dst=(unsigned char *)malloc(CHUNK * sizeof (unsigned char))) == NULL )
    throw nw::runtime_error(nw::string("gzip : malloc error (1)") );

  strm.avail_in = sizeSrc;
  strm.next_in = (Bytef*)src;

  int i=0;

  do
  {
    strm.avail_out = CHUNK;
    strm.next_out = (Bytef*)*dst + i*CHUNK;
    sizeDst=CHUNK * (i+1);

     if (deflate(&strm, Z_FINISH ) == Z_STREAM_ERROR)  /* state not clobbered */
    {
      free (*dst);
      throw nw::runtime_error(nw::string("gzip : deflate error") );
    }

    i++;

    if (strm.avail_out == 0)
    {
      unsigned char* reallocDst = (unsigned char*) realloc (*dst, CHUNK * (i+1) * sizeof (unsigned char) );

      if (reallocDst!=NULL)
        *dst=reallocDst;
       else
      {
        free (reallocDst);
        free (*dst);
        throw nw::runtime_error(nw::string("gzip : (re)allocating memory") );
      }
    }
  }
  while (strm.avail_out == 0);

  /* clean up and return */
  (void)deflateEnd(&strm);
  return sizeDst - strm.avail_out;
}

//********************************************************

inline size_t nvj_gunzip( unsigned char** dst, const unsigned char* src, const size_t sizeSrc, bool rawDeflateData=false )
{
  z_stream strm;
  size_t sizeDst=CHUNK;
  int ret;

  if (src == NULL)
    throw nw::runtime_error(nw::string("gunzip : src == NULL !") );

  /* allocate inflate state */
  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;
  strm.opaque = Z_NULL;
  strm.avail_in = 0;
  strm.next_in = Z_NULL;


  if (inflateInit2(&strm, rawDeflateData ? -15 : 16+MAX_WBITS) != Z_OK)
    throw nw::runtime_error(nw::string("gunzip : inflateInit2 error") );

  if ( (*dst=(unsigned char *)malloc(CHUNK * sizeof (unsigned char))) == NULL )
    throw nw::runtime_error(nw::string("gunzip : malloc error (2)") );

  strm.avail_in = sizeSrc;
  strm.next_in = (Bytef*)src;

  int i=0;

  do
  {
    strm.avail_out = CHUNK;
    strm.next_out = (Bytef*)*dst + i*CHUNK;
    sizeDst=CHUNK * (i+1);

    ret = inflate(&strm, Z_NO_FLUSH);

    switch (ret)
    {
      case Z_STREAM_ERROR:
        free (*dst);
        throw nw::runtime_error(nw::string("gunzip : inflate Z_STREAM_ERROR") );
      case Z_NEED_DICT:
      case Z_DATA_ERROR:
// ICI
      case Z_MEM_ERROR:
        (void)inflateEnd(&strm);
        free (*dst);
        throw nw::runtime_error(nw::string("gunzip : inflate error") );
    }

    i++;
    //printf("i=%d\n",i);fflush(NULL);
     if (strm.avail_out == 0)
     {
      unsigned char* reallocDst = (unsigned char*) realloc (*dst, CHUNK * (i+1) * sizeof (unsigned char) );

      if (reallocDst!=NULL)
        *dst=reallocDst;
      else
      {
        free (reallocDst);
        free (*dst);
        throw nw::runtime_error(nw::string("gunzip : (re)allocating memory") );
      }
    }
  }
  while (strm.avail_out == 0);

  /* clean up and return */
  (void)inflateEnd(&strm);
  return sizeDst - strm.avail_out;
}





