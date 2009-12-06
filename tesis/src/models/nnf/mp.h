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

#ifndef MP_INCLUDE
#define MP_INCLUDE

#ifdef MP

#include <gmp.h>

namespace mp {
  class Int {
    mpz_t z_;
  public:
    Int( int n = 0 ) { mpz_init_set_si(z_,n); }
    Int( const Int &a ) { mpz_init_set(z_,a.z_); }
    ~Int() { mpz_clear(z_); }
    void operator=( const Int &a ) { mpz_set(z_,a.z_); }
    void operator=( int n ) { mpz_set_si(z_,n); }
    bool operator==( const Int &a ) { return( mpz_cmp(z_,a.z_) == 0 ); }
    bool operator==( int n ) { return( mpz_cmp_si(z_,n) == 0 ); }
    bool operator<( const Int &a ) { return( mpz_cmp(z_,a.z_) < 0 ); }
    bool operator<( int n ) { return( mpz_cmp_si(z_,n) < 0 ); }
    void operator+=( const Int &a ) { mpz_add(z_,z_,a.z_); }
    void operator*=( const Int &a ) { mpz_mul(z_,z_,a.z_); }
    void operator^=( unsigned n ) { mpz_pow_ui(z_,z_,n); }
    void print( std::ostream &os ) const { char *s = mpz_get_str(0,10,z_); os << s; free(s); }
  };
}; 

#else

namespace mp {
  class Int {
  public:
    Int( int n = 0 ) { }
    Int( const Int &a ) { }
    ~Int() { }
    void operator=( const Int &a ) { }
    void operator=( int n ) { }
    bool operator==( const Int &a ) { return(true); }
    bool operator==( int n ) { return(true); }
    bool operator<( const Int &a ) { return(true); }
    bool operator<( int n ) { return(true); }
    void operator+=( const Int &a ) { }
    void operator*=( const Int &a ) { }
    void operator^=( unsigned n ) { }
    void print( std::ostream &os ) const { os << "<mp support not compiled>"; }
  };
};

#endif

inline std::ostream& operator<<( std::ostream &os, const mp::Int &a ) { a.print(os); return(os); }

#endif // MP_INCLUDE

