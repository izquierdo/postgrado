#ifndef TWSEARCH_H_
#define TWSEARCH_H_

#include "preproc_flags.h"
#include "TWStats.h"
#include "GraphUtilities.h"
#include "ALMGraph.h"

namespace Treewidth_Namespace {

  using namespace Adjacency_List_Matrix_Graph;
  using GraphUtilities::boolMatrix;

  enum TWExitStatus {TW_SUCCESS, TW_NOTIME, TW_NOMEM, TW_NONE};

  class TWSearch {
  public:
    static TWExitStatus initSearch(const boolMatrix &adjmat,
				   HeuristicVersion hversion, ALMGraph &graph, vector<int> &elimprefix,
				   int &elimprefix_width, int &postprefix_lb, vector<int> &vertexmap,
				   ElimOrder &ub_order, bool &ub_mapped, TWStats &stats, uint lb=0,
				   uint ub=UINT_MAX);

    virtual TWExitStatus
      solve(ElimOrder &soln, const boolMatrix &adjmat,
	    HeuristicVersion hversion, uint lb=0, uint ub=UINT_MAX,
	    int timelim=INT_MAX, int memlim=INT_MAX) =0;

    virtual void writeResultFile(const char *outfile) =0;
    virtual void writeStatFile(const char *statfile) =0;
  };
}
;

#endif /*TWSEARCH_H_*/
