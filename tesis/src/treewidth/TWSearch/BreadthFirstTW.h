// BreadthFirstTW.h - breadth-first heuristic treewidth algorithms
//
// author: Alex Dow
// created on: 11/20/6

#ifndef BREADTHFIRSTTW_H_
#define BREADTHFIRSTTW_H_

//#define BFHT_PRUNE2 /* INCORRECT - do not enable this, it's pruning some opt solns */
#ifndef BFHT_FASTGRAPH
#define BFHT_PRUNE3 // AVDC, adj vert dominance criterion, only applicable without fastgraph
#endif

// BFHT_PRUNE_GRDC - Define to prune nodes based on the Graph Reduction
// Dominance Criteria. This criterion invokes the Simplicial and Almost
// Simplicial rules to determine if the immediate elimination of any vertex
// dominates any other vertex elimination.
#define BFHT_PRUNE_GRDC

#include "preproc_flags.h"
#include <pthread.h>
#include <stack>
#include <list>
#include <vector>
#include "ALMGraph.h"
#include "TWState.h"
#include "TWSearch.h"
#include "BreadthFirstTWNode.h"
#include "BreadthFirstTWOpenList.h"
#include "BreadthFirstTWStats.h"
#include "BFHT_DDD_OpenList.h"
#include "BFHT_HDDD_Node.h"
#include "BFHT_HDDD.h"

using namespace Adjacency_List_Matrix_Graph;

namespace Treewidth_Namespace {

typedef pair<BreadthFirstTWNode*,BreadthFirstTWNode*> Task;

typedef pair<BFHT_HDDD_Node*,BFHT_HDDD_Node*> BFHT_HDDD_Task;

typedef deque<BFHT_HDDD_FileInfo> FileInfoDeque;

struct BFHTSearchForTW_ret {
	TWExitStatus exit_status;
	BreadthFirstTWNode *mid_node;
	int tw;
#ifdef BFHT_UB_AT_EVERY_NODE
	bool optsuffix_set;
	ElimOrder optsuffix;
	BFHTSearchForTW_ret(TWExitStatus es=TW_NONE, BreadthFirstTWNode *n=NULL,
			int treewidth=-1, bool os_set=false, ElimOrder os=ElimOrder()) :
		exit_status(es), mid_node(n), tw(treewidth), optsuffix_set(os_set),
		optsuffix(os) {
	}
#else
	BFHTSearchForTW_ret(TWExitStatus es=TW_NONE, BreadthFirstTWNode *n=NULL,
			int treewidth=-1) :
		exit_status(es), mid_node(n), tw(treewidth) {
	}
#endif
};

struct BFHTSearchForSolnPath_ret {
	TWExitStatus exit_status;
	BreadthFirstTWNode *mid_node;
	BFHTSearchForSolnPath_ret(TWExitStatus es=TW_NONE,
			BreadthFirstTWNode *n=NULL) :
		exit_status(es), mid_node(n) {
	}
};

enum BFHTDupeDetection {BFHT_IMMEDIATE_DD, BFHT_DELAYED_DD};
enum BFHTCutoffType {BFHT_UB_CUTOFF, BFHT_ID_CUTOFF};

class BreadthFirstTW : public TWSearch {

	bool run_val;
	TWExitStatus exit_status_val;
	ElimOrder solution_val;
	BreadthFirstTWStats stats;

	void initSearch(BreadthFirstTWNode *&start, BreadthFirstTWNode *&goal,
			int elimprefix_width, int postprefix_lb);

	void finalizeSearch(BreadthFirstTWNode *start,
			BreadthFirstTWNode *goal);

	void
			finalizeSearch_ddd(BreadthFirstTWNode *start,
					BreadthFirstTWNode *goal);

	TWExitStatus divideAndConquer(ElimOrder &soln, struct timeval starttime,
			int timelim, int memlim, HeuristicVersion hversion,
			const ALMGraph &graph, BreadthFirstTWNode *start,
			BreadthFirstTWNode *goal, int ub, BFHTDupeDetection dupeDetection);

	TWExitStatus search(ElimOrder &soln, struct timeval starttime, int timelim,
			HeuristicVersion hversion, const ALMGraph &graph,
			BFHT_HDDD_Node &start, int ub);

	BFHTSearchForTW_ret searchForTW_idd(struct timeval starttime, int timelim,
			int memlim, HeuristicVersion hversion, const ALMGraph &orig_graph,
			ALMGraph &cur_graph, const BreadthFirstTWNode &start, int ub,
			int mid_layer_depth);

	BFHTSearchForTW_ret searchForTW_ddd(struct timeval starttime, int timelim,
			int memlim, HeuristicVersion hversion, const ALMGraph &orig_graph,
			ALMGraph &cur_graph, const BreadthFirstTWNode &start, int ub,
			int mid_layer_depth);

	BFHTSearchForSolnPath_ret searchForSolnPath_ddd(struct timeval starttime,
			int timelim, int memlim, HeuristicVersion hversion,
			const ALMGraph &orig_graph, ALMGraph &cur_graph,
			const BreadthFirstTWNode &start, const BreadthFirstTWNode &goal,
			int mid_layer_depth);

	BFHTSearchForSolnPath_ret searchForSolnPath_idd(struct timeval starttime,
			int timelim, int memlim, HeuristicVersion hversion,
			const ALMGraph &orig_graph, ALMGraph &cur_graph,
			const BreadthFirstTWNode &start, const BreadthFirstTWNode &goal,
			int mid_layer_depth);

	BreadthFirstTWNode* expandNode(const BreadthFirstTWNode &n,
			const ALMGraph &graph, ushort ub, int mid_layer,
			BFHT_DDD_OpenList &openlist, int *min_gval=NULL);

	void expandNodeSubset(const BreadthFirstTWNode &n, const ALMGraph &graph,
			int mid_layer, const BreadthFirstTWNode &goal,
			BFHT_DDD_OpenList &openlist);

	void allocAndInsertInOpen(const vector<BreadthFirstTWNode> &dummys,
			int mid_layer, BreadthFirstTWOpenList *openlist);

	BreadthFirstTWNode* checkOpenAllocAndInsert(
			const BreadthFirstTWNode &dummy, int mid_layer,
			BreadthFirstTWOpenList *openlist);

#ifdef BFHT_UB_AT_EVERY_NODE
	void getOrder(HeuristicVersion hversion, const ALMGraph &orig_graph,
			const vector<BreadthFirstTWNode*> &node_path, vector<int> &dest,
			bool optsuffix_valid, ElimOrder &optsuffix);
#else
	void getOrder(HeuristicVersion hversion, const ALMGraph &orig_graph,
			const vector<BreadthFirstTWNode*> &node_path, vector<int> &dest);
#endif

	void getOrder(HeuristicVersion hversion, const ALMGraph &orig_graph,
			const vector<BFHT_HDDD_Node*> &node_path, vector<int> &dest);

	void deallocSolnPath(vector<BreadthFirstTWNode*> &soln_path,
			const BreadthFirstTWNode *start, const BreadthFirstTWNode *goal);

	void deallocSolnPath(vector<BFHT_HDDD_Node*> &soln_path,
			const BFHT_HDDD_Node *start, const BFHT_HDDD_Node *goal);

	void setSolution(ElimOrder &soln, const vector<int> &prefix,
			const vector<int> &vertexmap);

protected:
	TWExitStatus solve(ElimOrder &soln, const boolMatrix &adjMat,
			HeuristicVersion hversion, BFHTCutoffType cutoffType,
			BFHTDupeDetection dupeDetection, int timelim=INT_MAX,
			int memlim=INT_MAX, uint lb=0, uint ub=UINT_MAX);

	TWExitStatus solve_w_HDDD(ElimOrder &soln, const boolMatrix &adjMat,
			HeuristicVersion hversion, BFHTCutoffType cutoffType,
			int timelim=INT_MAX);

public:

	BreadthFirstTW();

	virtual void writeResultFile(const char *outfile);
	virtual void writeStatFile(const char *statfile);
};

inline BreadthFirstTW::BreadthFirstTW() :
	run_val(false) {
}

class BFHT_UpperBound_IDD : public BreadthFirstTW {
public:
	TWExitStatus solve(ElimOrder &soln, const boolMatrix &adjmat,
			HeuristicVersion hversion, uint lb=0, uint ub=UINT_MAX,
			int timelim=INT_MAX, int memlim=INT_MAX)
	{
		return BreadthFirstTW::solve(soln, adjmat, hversion, BFHT_UB_CUTOFF,
				BFHT_IMMEDIATE_DD, timelim, memlim, lb, ub);
	}
};

class BFHT_UpperBound_SDDD : public BreadthFirstTW {
public:
	TWExitStatus solve(ElimOrder &soln, const boolMatrix &adjmat,
			HeuristicVersion hversion, uint lb=0, uint ub=UINT_MAX,
			int timelim=INT_MAX, int memlim=INT_MAX)
	{
		return BreadthFirstTW::solve(soln, adjmat, hversion, BFHT_UB_CUTOFF,
				BFHT_DELAYED_DD, timelim, memlim, lb, ub);
	}
};

class BFHT_UpperBound_HDDD : public BreadthFirstTW {
public:
	TWExitStatus solve(ElimOrder &soln, const boolMatrix &adjmat,
			HeuristicVersion hversion, uint lb=0, uint ub=UINT_MAX,
			int timelim=INT_MAX, int memlim=INT_MAX) {
		assert(lb==0 && ub==UINT_MAX); // function hasn't been updated to use lb and ub parameters
		return BreadthFirstTW::solve_w_HDDD(soln, adjmat, hversion,
				BFHT_UB_CUTOFF, timelim);
	}
};

class BFHT_IterativeDeepening_IDD : public BreadthFirstTW {
public:
	TWExitStatus solve(ElimOrder &soln, const boolMatrix &adjmat,
			HeuristicVersion hversion, uint lb=0, uint ub=UINT_MAX,
			int timelim=INT_MAX, int memlim=INT_MAX) {
		return BreadthFirstTW::solve(soln, adjmat, hversion, BFHT_ID_CUTOFF,
				BFHT_IMMEDIATE_DD, timelim, memlim, lb, ub);
	}
};

class BFHT_IterativeDeepening_SDDD : public BreadthFirstTW {
public:
	TWExitStatus solve(ElimOrder &soln, const boolMatrix &adjmat,
			HeuristicVersion hversion, uint lb=0, uint ub=UINT_MAX,
			int timelim=INT_MAX, int memlim=INT_MAX) {
		return BreadthFirstTW::solve(soln, adjmat, hversion, BFHT_ID_CUTOFF,
				BFHT_DELAYED_DD, timelim, memlim, lb, ub);
	}
};

class BFHT_IterativeDeepening_HDDD : public BreadthFirstTW {
public:
	TWExitStatus solve(ElimOrder &soln, const boolMatrix &adjmat,
			HeuristicVersion hversion, uint lb=0, uint ub=UINT_MAX,
			int timelim=INT_MAX, int memlim=INT_MAX) {
		assert(lb==0 && ub==UINT_MAX); // function hasn't been updated to use lb and ub parameters
		return BreadthFirstTW::solve_w_HDDD(soln, adjmat, hversion,
				BFHT_ID_CUTOFF, timelim);
	}
};

}

#endif /*BREADTHFIRSTTW_H_*/
