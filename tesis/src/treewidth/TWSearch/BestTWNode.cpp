#include "preproc_flags.h"
#include <algorithm>
#include "BestTWNode.h"

namespace Treewidth_Namespace {

///////////////////////////////////////////////////////////////////////////////
// BestTWNode expand - expands this node assuming it corresponds to the passed
//   graph. If children nodes' gValues meet or exceed the passed upper bound,
//   ub, they will not be generated. The generated nodes are stored in the
//   passed vector dummyChildren. adjacentVerts is an unordered list of vertex
//   ids corresponding to those vertices that were adjacent to the vertex
//   eliminated to generate this node. Function will not expand any children
//   that result from eliminating one of these vertices. This is possible
//   because we can always find an optimal elimination order that, while the
//   graph is not a clique, only eliminates non-adjacent vertices
//   consecutively (Dirac? see Kloks 1994, page 8).
///////////////////////////////////////////////////////////////////////////////
void BestTWNode::expand(const ALMGraph &graph, ushort ub,
		const vector<int> &adjacentVerts, vector<BestTWNode> &dummyChildren) {

	// set up adjverts vector with constant time lookup
	vector<bool> adjverts(graph.vertices.size(), false);
	for (uint i=0; i<adjacentVerts.size(); i++)
		adjverts[adjacentVerts[i]] = true;

	// other setup
	dummyChildren.resize(graph.nVerts);
	int nChildren = 0;

	// iterate through vertices in graph
	VertexList *curr = graph.vertices[0].next;
	while (curr!=NULL) {

		// check adjverts
		if (!adjverts[curr->vid]) {

			// check upper bound
			int vdeg = graph.adjLM[curr->vid][0].vid;
			if (vdeg<ub)
				// generate child
				dummyChildren[nChildren++].generateDummy(this, curr->vid, vdeg);
		}
		curr=curr->next;
	}
	if (nChildren<(int)dummyChildren.size())
		dummyChildren.resize(nChildren);
}

///////////////////////////////////////////////////////////////////////////////
// BestTWNode recoverSolutionPath - follows parent pointers beginning from
//   this node to fill a vector with list of eliminated vertex ids.
///////////////////////////////////////////////////////////////////////////////
void BestTWNode::recoverSolutionPath(vector<int> &solution) const {
	solution.clear();
	const BestTWNode *n = this;
	while (n->parent!=NULL) {
		solution.push_back(n->id);
		n=n->parent;
	}
	reverse(solution.begin(), solution.end());
}

}
