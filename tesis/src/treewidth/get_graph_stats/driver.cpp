
#define NDEBUG

#include <iostream>
#include <fstream>
#include <sstream>
#include <getopt.h>
#include <iomanip>
#include <cassert>
#include "Graph.h"

using namespace std;

int main (int argc, char *argv[])
{
  char usage_string[] = "\
Usage: get_graph_stats elg_file\n\
Options:\n\
  -b\n\
    Run \"buggy\" min-fill instead of min-fill\n\
  -g output_graph\n\
    Output reduced graph to file output_graph\n\
  -m vmap\n\
    Output mapping of old vertex indices to new vertex indices to file vmap. In\n\
    this file the ith value is equal to the old index of what is now vertex i.\n\
    This option is only available if the -g option is set as well.\n\
  -o output_order\n\
    Output the best elimination order found to file output_order\n\
  -p output_prefix\n\
    Output the optimal elimination order prefix found via the reduction\n\
    rules to file output_prefix\n\
  -q\n\
    Quiet mode, outputs a single line of space-seperated values in the\n\
    following order: # vertices in original graph, # edges in original\n\
    graph, # vertices in reduced graph, # edges in reduced graph, lower\n\
    bound, upper bound\n\
  -v\n\
    Verbose mode, outputs progress data to cerr\
";

  // parse/check optional arguments
  bool error_flag=false;
  char *graphfilename=NULL;
  char *outputgraph=NULL;
  char *outputorder=NULL;
  char *outputprefix=NULL;
  char *vmapfile=NULL;
  bool quiet_mode=false;
  bool verbose_mode=false;
  bool buggy_minfill_mode=false;
  int c;
  while ((c = getopt(argc,argv,"g:m:o:p:qvb")) != -1) {
    switch (c)
      {
      case 'g':
	outputgraph=optarg;
	break;
      case 'm':
	vmapfile = optarg;
	break;
      case 'o':
	outputorder=optarg;
	break;
      case 'p':
	outputprefix=optarg;
	break;
      case 'q':
	quiet_mode = true;
	break;
      case 'v':
	verbose_mode = true;
	graph_h_verbose_mode = true;
	break;
      case 'b':
	buggy_minfill_mode = true;
	break;
      case '?':
	cerr << "Invalid option.\n" << usage_string << endl;
	exit(1);
      default:
	abort();
      }
  }

  if (optind < argc)
    {
      graphfilename = argv[optind];
    }

  if (graphfilename == NULL || error_flag || optind + 1 < argc) 
    {
      cerr << usage_string << endl;
      exit(1);
    }

  // the stats we are seeking
  int nverts = 0;
  int nedges = 0;
  int rverts = 0;
  int redges = 0;
  int lb = 0;
  int ub = 0;

  Graph graph;
  assert(graph_edgelist_flag);
  graph.load(graphfilename);
#ifndef NDEBUG
  graph.verify();
#endif
  nverts = graph.size();
  nedges = graph.num_edges();
  if (verbose_mode)
    {
      cerr << "loaded, " << graph.size() << " vertices\n"
	   << "computing lb\n";
    } 
  lb = mmdplus_leastc(graph);
  if (verbose_mode)
    cerr << "lb = " << lb << endl;
  vector<Graph::vertex_type> optimal_prefix;
  lb = reduce(graph, lb, &optimal_prefix);
  rverts = graph.size();
  redges = graph.num_edges();
  if (verbose_mode)
    {
      cerr << "reduced, " << graph.size() << " verts remaining, new lb = " << lb << endl;
      cerr << "computing ub\n";
    }
  vector<Graph::vertex_type> minfill_order;
  if (rverts <= 4096) // don't run minfill for really large graphs
		      // because it takes forever
    {
      if (!buggy_minfill_mode)
	ub = minfill(graph, &minfill_order);
      else
	ub = buggy_minfill(graph, &minfill_order);
    }
  else
    {
      ub = INT_MAX;
      if (verbose_mode)
	cerr << "not computing ub because graph is so large it will take forever\n";
    }
  if (ub < lb)
    ub = lb;

  // output stats
  if (!quiet_mode)
    {
      cout << "number of vertices in original graph: " << setw(4) << right << nverts << endl
	   << "number of edges in original graph:    " << setw(4) << right << nedges << endl
	   << "number of vertices in reduced graph:  " << setw(4) << right << rverts << endl
	   << "number of edges in reduced graph:     " << setw(4) << right << redges << endl
	   << "MMD+(least-c) lower bound:            " << setw(4) << right << lb << endl
	   << "min-fill upper bound:                 " << setw(4) << right << ub << endl;
    }
  else
    {
      cout << nverts << " "
	   << nedges << " "
	   << rverts << " "
	   << redges << " "
	   << lb << " "
	   << ub << endl;
    }

  // output reduced graph
  if (outputgraph != NULL)
    {
      ofstream graphout(outputgraph);
      if (graphout.is_open())
	{
	  if (vmapfile != NULL)
	    {
	      ofstream vmapout(vmapfile);
	      graph.print_elg(graphout, &vmapout);
	      vmapout.close();
	    }
	  else
	    {
	      graph.print_elg(graphout, NULL);
	    }
	  graphout.close();
	}
      else
	{
	  cerr << "Error: Could not open " << outputgraph 
	       << " for writing.\n";
	}
    }

  //output best elimination order found
  if (outputorder != NULL)
    {
      ofstream orderout(outputorder);
      if (orderout.is_open())
	{
	  copy(optimal_prefix.begin(), optimal_prefix.end(),
	       ostream_iterator<int>(orderout," "));
	  copy(minfill_order.begin(), minfill_order.end(), 
	       ostream_iterator<int>(orderout," "));
	  orderout << endl;
	}
      else
	{
	  cerr << "Error: Could not open file " << outputorder
	       << " for writing.\n";
	}
    }

  //output the optimal elimination order prefix found by reduction rules
  if (outputprefix != NULL)
    {
      ofstream prefixout(outputprefix);
      if (prefixout.is_open())
	{
	  copy(optimal_prefix.begin(), optimal_prefix.end(),
	       ostream_iterator<int>(prefixout," "));
	  prefixout << endl;
	}
      else
	{
	  cerr << "Error: Could not open file " << outputprefix
	       << " for writing.\n";
	}
    }

  return 0;
}
