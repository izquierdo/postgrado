#include "preproc_flags.h"
#include "TWDriver.h"
#include "BestTW.h"
#include "BreadthFirstTW.h"
#include "DFS.h"

namespace Treewidth_Namespace {

void TWDriver::run(TWAlgorithm alg, HeuristicVersion heuristic,
		GraphFileType file_type, const char *graph_file, int timelim,
		int memlim, const char *result_file, const char *stat_file,
		uint lb, uint ub) {

	if (timelim<=0)
		timelim=TW_DEFAULT_TIME;
	if (memlim<=0)
		memlim=TW_DEFAULT_MEM;

	boolMatrix adjmat = FileOperations::readGraph(graph_file, file_type);

	ElimOrder optorder;
	TWSearch *twsearch;
	switch (alg) {
	case ALG_BESTTW:
		twsearch = new BestTW();
		break;
	case ALG_BFHT_UB:
		twsearch = new BFHT_UpperBound_IDD();
		break;
	case ALG_BFHT_SDDD_UB:
		twsearch = new BFHT_UpperBound_SDDD();
		break;
	case ALG_BFHT_HDDD_UB:
		twsearch = new BFHT_UpperBound_HDDD();
		break;
	case ALG_BFHT_ID:
		twsearch = new BFHT_IterativeDeepening_IDD();
		break;
	case ALG_BFHT_SDDD_ID:
		twsearch = new BFHT_IterativeDeepening_SDDD();
		break;
	case ALG_BFHT_HDDD_ID:
		twsearch = new BFHT_IterativeDeepening_HDDD();
		break;
	case ALG_DFBNB:
		twsearch = new DFBnB();
		break;
	case ALG_DFID:
		twsearch = new DFID();
		break;
	default:
		return;
	}

	TWExitStatus exit_status = twsearch->solve(optorder, adjmat, heuristic,
			lb, ub, timelim, memlim);

	if (exit_status==TW_SUCCESS)
		cout << "Treewidth = " << optorder.width << endl;

	twsearch->writeResultFile(result_file);
	twsearch->writeStatFile(stat_file);

	delete twsearch;
}

}
