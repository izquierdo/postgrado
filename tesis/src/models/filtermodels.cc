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

#include <nnf.h>
#include <satlib.h>
#include <utils.h>

#include <string.h>

#include <iostream>
#include <fstream>

void
banner( std::ostream &os )
{
  os << "Model enumerator for d-DNNF theories." << std::endl
     << "Universidad Simon Bolivar, 2006." << std::endl;
}

void
usage( std::ostream &os )
{
  os << "usage: models [--mp] [--literal-counts] [--num <n>] [--verbose] [--write-models] [--write-all-literals] <nnf>" << std::endl;
}

void
read_simple_file( std::istream &is, std::vector<int> &contents )
{
  size_t n;
  is >> n;
  contents.clear();
  contents.reserve(n);
  for( size_t i = 0; i < n; ++i ) {
    int entry;
    is >> entry;
    contents.push_back(entry);
  }
}

int
main( int argc, char **argv )
{
  const char *forget_file = 0;
  float i_seconds, l_seconds, seconds;
  bool use_mp = false;
  bool literal_counts = false;
  bool all = false;
  bool output = false;
  bool verbose = false;
  size_t count = 0;
  banner(std::cout);

  ++argv;
  --argc;
  if( argc == 0 ) {
   print_usage:
    usage(std::cout);
    exit(-1);
  }
  for( ; argc && ((*argv)[0] == '-'); --argc, ++argv ) {
    if( !strcmp(*argv,"--mp") ) {
      use_mp = true;
    }
    else if( !strcmp(*argv,"--literal-counts") ) {
      literal_counts = true;
    }
    else if( !strcmp(*argv,"--num") ) {
      count = atoi(argv[1]);
      ++argv;
      --argc;
    }
    else if( !strcmp(*argv,"--verbose") ) {
      verbose = true;
    }
    else if( !strcmp(*argv,"--write-models") ) {
      output = true;
    }
    else if( !strcmp(*argv,"--write-all-literals") ) {
      all = true;
    }
    else {
        switch( (*argv)[1] ) {
            case 'f':
                forget_file = *++argv;
                --argc;
                break;
            default:
                usage(std::cout);
                exit(-1);
        }
    }
  }

  // read files
  std::vector<int> forgets;
  if( forget_file != 0 ) {
    std::ifstream is(forget_file);
    read_simple_file(is,forgets);
  }

  if( argc != 1 ) goto print_usage;
  std::string nnf_file = argv[0];

  // create managers, set start timer and read file
  nnf::Manager *nnf_theory = new nnf::Manager(0,verbose);
  i_seconds = l_seconds = utils::read_time_in_seconds();

  std::ifstream nnf_is(nnf_file.c_str());
  if( !nnf_is.is_open() ) {
    std::cout << "main: error opening file '" << nnf_file << "'" << std::endl;
    exit(-1);
  }

  try {
    std::cout << "main: reading file '" << nnf_file << "'" << std::flush;
    satlib::read_nnf_file(nnf_is,*nnf_theory);
  }
  catch( int e ) {
      std::cout << std::endl << "main: error reading file '" << nnf_file << "'" << std::endl;
      exit(-1);
  }
  seconds = utils::read_time_in_seconds();
  std::cout << " : " << seconds-l_seconds << " seconds" << std::endl;
  l_seconds = seconds;

  // print simple stats
  void *lc = 0;
  if( literal_counts ) {
    if( use_mp )
      lc = new mp::Int*[1+nnf_theory->num_vars()];
    else {
      lc = new float[1+nnf_theory->num_vars()];
    }
  }
  std::cout << "main: #nodes=" << nnf_theory->count_nodes()
            << ", #edges=" << nnf_theory->count_edges();
  if( use_mp )
    std::cout << ", #models=" << *(nnf_theory->mp_count_models( (mp::Int**)lc ));
  else
    std::cout << ", #models=" << nnf_theory->count_models( (float*)lc );
  seconds = utils::read_time_in_seconds();
  std::cout << " : " << seconds-l_seconds << " seconds" << std::endl;

  if( literal_counts ) {
    std::cout << "--- count table ---" << std::endl;
    for( size_t i = 0 ; i < nnf_theory->num_vars(); ++i ) {
      if( use_mp ) {
        if( ((mp::Int**)lc)[i+1] == 0 )
          std::cout << "literal " << i+1 << " : n/a" << std::endl;
        else
          std::cout << "literal " << i+1 << " : " << *((mp::Int**)lc)[i+1] << std::endl;
      }
      else {
        if( ((float*)lc)[i+1] == -1 )
          std::cout << "literal " << i+1 << " : n/a" << std::endl;
        else
          std::cout << "literal " << i+1 << " : " << ((float*)lc)[i+1] << std::endl;
      }
    }
    std::cout << "--- count table ---" << std::endl;
  }

  // enumerate models
  if( output ) {
    std::cout << "--- models begin ---" << std::endl;
    nnf_theory->enumerate_and_project_models(std::cout,count,all,forgets);
    std::cout << "---- models end ----" << std::endl;
  }

  // cleanup: don't needed since finishing...
  //nnf_theory->unregister_use( nnf_theory->root() );
  //delete nnf_theory;

  seconds = utils::read_time_in_seconds();
  std::cout << "main: total time " << seconds-i_seconds << " seconds" << std::endl;
  return(0);
}

