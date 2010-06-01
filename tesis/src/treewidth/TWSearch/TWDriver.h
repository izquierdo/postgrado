#ifndef TWDRIVER_H_
#define TWDRIVER_H_

#include "preproc_flags.h"
#include "ALMGraph.h"
#include "GraphUtilities.h"

using namespace Adjacency_List_Matrix_Graph;
using namespace GraphUtilities;

namespace Treewidth_Namespace {

enum TWAlgorithm {
	ALG_BESTTW,
	ALG_BFHT_UB,
	ALG_BFHT_ID,
	ALG_BFHT_SDDD_UB,
	ALG_BFHT_SDDD_ID,
	ALG_BFHT_HDDD_UB,
	ALG_BFHT_HDDD_ID,
	ALG_DFBNB,
	ALG_DFID
	};

class TWDriver {
public:
	static void run(TWAlgorithm alg, HeuristicVersion heuristic,
			GraphFileType file_type, const char *graph_file, int timelim,
			int memlim, const char *result_file, const char *stat_file,
			uint lb=0, uint ub=UINT_MAX);
};

}

#endif /*TWDRIVER_H_*/
