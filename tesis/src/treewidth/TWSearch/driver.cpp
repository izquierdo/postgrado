#include "preproc_flags.h"
#include <iostream>
#include <getopt.h>
#include <cassert>
#include "TWDriver.h"

using namespace Treewidth_Namespace;
using namespace std;

string g_graphname;

void print_usage_short() {
	cout << "Usage: " << endl
			<< "  1. TWSearch (alg) (heuristic) [-l lb] [-u ub] "
			<< "[-t time] [-m mem] [-o outfile] [-s statfile] "
			<< "{-e|-d graphfile}" << endl
			<< "  2. TWSearch --help" << endl << endl;
}

void print_usage_long() {
	print_usage_short();
	cout << "Arguments and options: " << endl
	     << "  algorithms" << endl
	     << "    --besttw (default) - Best-first search for treewidth" << endl
	     << "    --bfht-ub - Breadth-first heuristic search with an initial upper bound" << endl
	     << "    --bfht-id - Breadth-first heuristic search with iterative deepening" << endl
	     << "    --bfht-sddd-ub - BFHT-UB with sorting-based delayed duplicate detection" << endl
	     << "    --bfht-sddd-id - BFHT-ID with sorting-based delayed duplicate detection" << endl
	     << "    --bfht-hddd-ub - BFHT-UB with hash-based delayed duplicate detection" << endl
	     << "    --bfht-hddd-id - BFHT-ID with hash-based delayed duplicate detection" << endl
	     << "    --dfbnb - Depth-first branch-and-bound search" << endl
	     << "    --dfid - Depth-first iterative deepening" << endl
	     << "  heuristics" << endl
	     << "    --minDeg\n"
	     << "    --minMaxPair\n"
	     << "    --degenMinDeg\n"
	     << "    --degenMinMaxPair\n"
	     << "    --contractionMinDeg-minD\n"
	     << "    --contractionMinDeg-leastC (default)\n"
	     << "  (-l or --lb)        lb        - Lower bound (inclusive) on treewidth" << endl
	     << "  (-u or --ub)        ub        - Upper bound (exclusive) on treewidth" << endl
		 << "  (-t or --time)      time      - Time limit in seconds" << endl
		 << "  (-m or --mem)       mem       - Memory limit in megabytes" << endl
		 << "  (-o or --outfile)   outfile   - Results output file" << endl
		 << "  (-s or --statfile)  statfile  - Statistics output file" << endl
		 << "  (-e or --edge-list) graphfile - Input graph in edge-list format" << endl
		 << "  (-d or --dimacs)    graphfile - Input graph in DIMACS graph format" << endl
#ifndef GATHER_STATS
		 << endl << "NOTE: preprocessor flags set to not keep statistics."
#endif
		 << endl;
}

int main(int argc, char *argv[]) {

	bool error_flag=false;
	int timelim=-1;
	int memlim=-1;
	char *outfilename=NULL;
	char *statfilename=NULL;
	char *graphfilename=NULL;
	int tw=-1;
	int help_flag=0;
	int alg_flag=(int)ALG_BESTTW;
	int heuristic_flag=(int)H_CONTRACTION_LEASTC;
	GraphFileType graph_type=GRAPH_EDGE_LIST;
	uint lb = 0;
	uint ub = UINT_MAX;

	struct option long_options[] = {
			{ "besttw", no_argument, &alg_flag, (int)ALG_BESTTW },
			{ "bfht-ub", no_argument, &alg_flag, (int)ALG_BFHT_UB },
			{ "bfht-id", no_argument, &alg_flag, (int)ALG_BFHT_ID },
			{ "bfht-sddd-ub", no_argument, &alg_flag, (int)ALG_BFHT_SDDD_UB },
			{ "bfht-sddd-id", no_argument, &alg_flag, (int)ALG_BFHT_SDDD_ID },
			{ "bfht-hddd-ub", no_argument, &alg_flag, (int)ALG_BFHT_HDDD_UB },
			{ "bfht-hddd-id", no_argument, &alg_flag, (int)ALG_BFHT_HDDD_ID },
			{ "dfbnb", no_argument, &alg_flag, (int)ALG_DFBNB },
			{ "dfid", no_argument, &alg_flag, (int)ALG_DFID },
			{ "minDeg", no_argument,  &heuristic_flag, (int)H_MINDEG },
			{ "minPairMax", no_argument,  &heuristic_flag, (int)H_MINPAIRMAX },
			{ "degenMinDeg", no_argument,  &heuristic_flag, (int)H_DEGEN_MINDEG },
			{ "degenMinPairMax", no_argument,  &heuristic_flag, (int)H_DEGEN_MINPAIRMAX },
			{ "contractionMinDeg-minD", no_argument, &heuristic_flag, (int)H_CONTRACTION_MIND },
			{ "contractionMinDeg-leastC", no_argument, &heuristic_flag, (int)H_CONTRACTION_LEASTC },
			{ "time", required_argument, NULL, 't' },
			{ "mem", required_argument, NULL, 'm' },
			{ "outfile", required_argument, NULL, 'o' },
			{ "statfile", required_argument, NULL, 's' },
			{ "edge-list", required_argument, NULL, 'e' },
			{ "dimacs", required_argument, NULL, 'd' },
			{ "lb", required_argument, NULL, 'l' },
			{ "ub", required_argument, NULL, 'u' },
			{ "help", no_argument, &help_flag, 1 },
			{ 0, 0, 0, 0 } };

	// parse/check optional arguments
	int option_index=0;
	int c;
	while ((c = getopt_long(argc, argv, "t:m:o:s:e:d:l:u:h", long_options,
			&option_index)) != -1) {
		switch (c) {
		case 0:
			break;
		case 't':
			timelim=atoi(optarg);
			if (timelim<=0) {
				cerr << "Error: invalid time argument.\n";
				error_flag=true;
			}
			break;
		case 'm':
			memlim=atoi(optarg);
			if (memlim<=0) {
				cerr << "Error: invalid mem argument.\n";
				error_flag=true;
			}
			break;
		case 'o':
			outfilename=optarg;
			break;
		case 's':
			statfilename=optarg;
			break;
		case 'e':
			if (graphfilename!=NULL) {
				cerr << "Error: only one graph file allowed.\n";
				error_flag=true;
			} else {
				graph_type=GRAPH_EDGE_LIST;
				graphfilename=optarg;
			}
			break;
		case 'd':
			if (graphfilename!=NULL) {
				cerr << "Error: only one graph file allowed.\n";
				error_flag=true;
			} else {
				graph_type=GRAPH_DIMACS;
				graphfilename=optarg;
			}
			break;
		case 'l':
			lb = atoi(optarg);
			break;
		case 'u':
			ub = atoi(optarg);
			break;
		case 'h':
			help_flag=1;
			break;
		case '?':
			error_flag=true;
			break;
		default:
			abort();
		}
	}

	// parse/check non-optional arguments
	if (optind < argc) {
		cerr << "Error: invalid extra arguments\n";
		error_flag=true;
	}

	// determine call type and check that arguments agree
	int call_type=0;
	if (!error_flag) {
		// type 1 call
		if (!help_flag) {
			call_type=1;
			if (graphfilename==NULL) {
				cerr << "Error: input graph argument missing.\n";
				error_flag=true;
			}
		// type 2 call
		} else if (help_flag) {
			call_type=2;
		} else
			error_flag=true;
	}

	if (error_flag) {
		print_usage_short();
	} else {

		int seed = 1197503591; //time(NULL);
		srand(seed);
		cout << "seed " << seed << endl;

		switch (call_type) {
		case 1: // find tw and elim order for a graph
			g_graphname = graphfilename;
			TWDriver::run((TWAlgorithm)alg_flag,
					(HeuristicVersion)heuristic_flag, graph_type, graphfilename,
					timelim, memlim, outfilename, statfilename, lb, ub);
			break;
		case 2: // print usage
			print_usage_long();
			break;
		default:
			print_usage_short();
		}
	}
	return 0;
}
