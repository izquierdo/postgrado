#ifndef BestTW_H_
#define BestTW_H_

#define BEST_PRUNE2
#define BEST_PRUNE3

#include "preproc_flags.h"
#include <vector>
#include <set>
#include <queue>
#include <ext/hash_set>
#include <string>
#include "TWSearch.h"
#include "TWState.h"
#include "BestTWNode.h"
#include "BestTWOpenList.h"
#include "BestTWStats.h"
#include "ALMGraph.h"

#define MAX2(a,b) (((a)<(b)) ? (b) : (a))

namespace Treewidth_Namespace {

using namespace std;
using namespace __gnu_cxx;
using namespace Adjacency_List_Matrix_Graph;

class BestTW : public TWSearch {
private:

	bool run_val;
	TWExitStatus exit_status_val;
	ElimOrder solution_val;
	BestTWStats stats;

	void setSolution(int width, const vector<int> &order, ElimOrder &soln);

	void setSolution(const ElimOrder &order, int prefix_width,
			const vector<int> &prefix, const vector<int> &vertexmap,
			ElimOrder &soln);

	void setSolution(int width, const vector<int> &order,
			const set<int> &suffix, const vector<int> &prefix,
			const vector<int> &vertexmap, ElimOrder &soln);

	BestTWNode* expandByReduction(BestTWNode *n, ALMGraph &cur_graph,
			BestTWOpenList &openNodes, BestTWNodeHash &closedNodes, int &ub,
			BestTWNode *&newub_node, vector<int> &adjacentVerts,
			const vector<int> &pAdjacentVerts, HeuristicVersion hversion,
			ALMGraph &graph_buffer);

	void expandNode(BestTWNode *n, ALMGraph &cur_graph,
			BestTWOpenList &openNodes, BestTWNodeHash &closedNodes, int &ub,
			BestTWNode *&newub_node, const vector<int> &adjacentVerts,
			HeuristicVersion hversion, ALMGraph &inter_graph);

	bool checkOpenList(BestTWNode &child, BestTWOpenList &openNodes);

public:

	BestTW();

	TWExitStatus solve(ElimOrder &soln, const boolMatrix &adjMat,
			HeuristicVersion hversion, uint p_lb=0, uint p_ub=UINT_MAX,
			int timelim=INT_MAX, int memlim=INT_MAX);

	void writeResultFile(const char *outfile);
	void writeStatFile(const char *statfile);
};

inline BestTW::BestTW() :
	run_val(false) {
}

/*class BestTW_driver {
private:
	static const int BESTTW_DEFAULT_TIME= INT_MAX;
	static const int BESTTW_DEFAULT_MEM = 800;
public:
	static void run(bool is_dimacs, bool is_mind, const char *graphfilename,
			int timelim=0, int memlim=0, const char *outfilename=NULL,
			const char *statfilename=NULL);
	static void writeResultFile(const char *outfilename, const BestTW besttw,
			TWExitStatus status, const ElimOrder &soln);
	static void writeStatFile(const char *statfilename, const BestTW besttw);
};*/
}


#endif /*BestTW_H_*/
