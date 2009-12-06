#include <nnf.h>
#include <satlib.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <iostream>
#include <fstream>
#include <functional>
#include <set>

int verbosity_level = 0;
std::vector<int> fvars;

void
banner( std::ostream &os )
{
  //os << "Compute best model from NNF." << std::endl;
}

void
usage( std::ostream &os )
{
  os << "usage: bestmodel [-f <file>] [-c <file>] <nnf>" << std::endl;
}

inline float
read_time_in_seconds( void )
{
  struct rusage r_usage;
  getrusage( RUSAGE_SELF, &r_usage );
  return( (float)r_usage.ru_utime.tv_sec +
	  (float)r_usage.ru_stime.tv_sec +
	  (float)r_usage.ru_utime.tv_usec / (float)1000000 +
	  (float)r_usage.ru_stime.tv_usec / (float)1000000 );
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

void
read_pairs_file( std::istream &is, std::vector<std::pair<int,int> > &contents )
{
  size_t n;
  is >> n;
  contents.clear();
  contents.reserve(n);
  for( size_t i = 0; i < n; ++i ) {
    int p, q;
    is >> p >> q;
    contents.push_back(std::make_pair(p,q));
  }
}

int
main( int argc, const char **argv )
{
  float i_seconds, l_seconds, seconds;
  const char *forget_file = 0, *cost_file = 0;
  banner(std::cout);

  // parse arguments
  ++argv;
  --argc;
  if( argc == 0 ) {
   print_usage:
    usage(std::cout);
    exit(-1);
  }
  for( ; argc && ((*argv)[0] == '-'); --argc, ++argv ) {
    switch( (*argv)[1] ) {
      case 'f':
        forget_file = *++argv;
        --argc;
        break;
      case 'c':
        cost_file = *++argv;
        --argc;
        break;
      case '?':
      default:
        goto print_usage;
    }
  }

  if( argc != 1 ) goto print_usage;
  std::string nnf_file = argv[0];

  // read files
  std::vector<int> forgets;
  if( forget_file != 0 ) {
    std::ifstream is(forget_file);
    read_simple_file(is,forgets);
  }

  std::vector<std::pair<int,int> > costs;
  if( cost_file != 0 ) {
    std::ifstream is(cost_file);
    read_pairs_file(is,costs);
  }

  // create managers and set start time
  nnf::Manager *nnf_theory = new nnf::Manager(); //0,true);
  i_seconds = l_seconds = read_time_in_seconds();

  // read nnfs from files
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
      std::cout << std::endl << "main: error reading nnf file '" << nnf_file << "'" << std::endl;
      exit(-1);
  }
  seconds = read_time_in_seconds();
  std::cout << " : " << seconds-l_seconds << " seconds" << std::endl;
  l_seconds = seconds;

  std::cout << "main: #nodes=" << nnf_theory->count_nodes()
            << ", #edges=" << nnf_theory->count_edges();
  seconds = read_time_in_seconds();
  std::cout << " : " << seconds-l_seconds << " seconds" << std::endl;

  //std::cout << "#vars=" << nnf_theory->num_vars() << std::endl;
  int *cost_vector = new int[2*nnf_theory->num_vars()+2];
  memset(cost_vector,0,2*(1+nnf_theory->num_vars())*sizeof(int));
  for( int i = 0, isz = costs.size(); i < isz; ++i ) {
    int lit = costs[i].first;
    lit = (lit>=0?2*lit:-2*lit+1);
    cost_vector[lit] = costs[i].second;
    //std::cout << "cost(" << lit << ")=" << cost_vector[lit] << std::endl;
  }

  nnf::Model min_model;
  int cost = nnf_theory->min_cost(cost_vector);
  nnf_theory->min_model(min_model);
  for( int i = 0, isz = forgets.size(); i < isz; ++i ) {
    int var = forgets[i]<<1;
    min_model.erase(var);
    min_model.erase(var+1);
  }
  std::cout << cost << " " << min_model << std::endl;

  delete[] cost_vector;

  // cleanup: don't needed since finishing...
  //nnf_theory->unregister_use( nnf_theory->root() );
  //delete nnf_theory;

  // total time
  //seconds = read_time_in_seconds();
  //std::cout << "main: total time " << seconds-i_seconds << " seconds" << std::endl;
  return(0);
}

