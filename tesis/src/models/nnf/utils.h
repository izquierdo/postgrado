/*
 *  Copyright (C) 2006 Universidad Simon Bolivar
 * 
 *  Permission is hereby granted to distribute this software for
 *  non-commercial research purposes, provided that this copyright
 *  notice is included with any such distribution.
 *  
 *  THIS SOFTWARE IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
 *  EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 *  PURPOSE.  THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE OF THE
 *  SOFTWARE IS WITH YOU.  SHOULD THE PROGRAM PROVE DEFECTIVE, YOU
 *  ASSUME THE COST OF ALL NECESSARY SERVICING, REPAIR OR CORRECTION.
 *  
 *  Blai Bonet, bonet@ldc.usb.ve
 *
 */

#ifndef UTILS_INCLUDE
#define UTILS_INCLUDE

#include <sys/resource.h>

namespace utils {
  template<typename T> inline const T& min( const T& a, const T& b ) { return( a <= b ? a : b ); }
  template<typename T> inline const T& max( const T& a, const T& b ) { return( a >= b ? a : b ); }
  inline float read_time_in_seconds()
  {
    struct rusage r_usage;
    getrusage(RUSAGE_SELF,&r_usage);
    return( (float)r_usage.ru_utime.tv_sec +
	    (float)r_usage.ru_stime.tv_sec +
	    (float)r_usage.ru_utime.tv_usec / (float)1000000 +
	    (float)r_usage.ru_stime.tv_usec / (float)1000000 );
  }
};

#endif // UTILS_INCLUDE

