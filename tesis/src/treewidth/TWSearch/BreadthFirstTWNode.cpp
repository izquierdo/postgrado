#include "preproc_flags.h"
#include "BreadthFirstTWNode.h"

#define MAX2(a,b) (((a)<(b)) ? (b) : (a))

namespace Treewidth_Namespace {

///////////////////////////////////////////////////////////////////////////////
// BreadthFirstTWNode half-generation constructor - "half generates" a node from its parent
//   by setting the g-value, and state, but not computing the h-value. Also
//   uses mid_layer to determine how to set ancest ptr.
// Inputs:
//   p - parent node
//   vid - id of vertex being eliminated
//   vdeg - degree of vertex begin eliminated, or any lb on gvalue
//   mid_layer - layer in search to save with ancest ptrs
///////////////////////////////////////////////////////////////////////////////
BreadthFirstTWNode::BreadthFirstTWNode(const BreadthFirstTWNode *p, int vid,
		int vdeg, int mid_layer) :
	state(p->state), nelim(p->nelim+1), id(vid) {
	state.setVert(vid);

	gval = MAX2(p->gval,vdeg);
#ifdef MONOTONIC_H
	hValue = 0;
#else
	if (vdeg < p->hval)
		hval = p->hval;
	else
		hval = 0;
#endif
	if (mid_layer<nelim) // past middle layer, save parents ancest ptr
		ancest = p->ancest;
	else if (mid_layer==nelim) // at middle layer, set ancest ptr to self
		ancest = this;
	else
		// before middle layer, set ancest to null
		ancest = NULL;
#ifdef DEBUG_TW
	if (ancest!=NULL)
		assert(ancest->nelim==mid_layer);
#endif
}

///////////////////////////////////////////////////////////////////////////////
// BreadthFirstTWNode constructor from dummy - generates a node by copying the state and
//   gvalue from the dummy node. If the current layer is past the mid_layer,
//   then the node is also assigned the ancest ptr from the dummy node.
///////////////////////////////////////////////////////////////////////////////
BreadthFirstTWNode::BreadthFirstTWNode(const BreadthFirstTWNode &dummy,
		int mid_layer) :
	state(dummy.state), nelim(dummy.nelim), gval(dummy.gval), hval(dummy.hval),
	id(dummy.id)
{
	if (mid_layer<nelim) // past middle layer, save dummy's ancest ptr
		ancest = dummy.ancest;
	else if (mid_layer==nelim) // at middle layer, set ancest ptr to self
		ancest = this;
	else
		// before middle layer, set ancest to null
		ancest = NULL;
}

///////////////////////////////////////////////////////////////////////////////
// BreadthFirstTWNode expand - expands this node assuming it corresponds to
//   the passed graph. If children nodes' gValues meet or exceed the passed
//   upper bound, ub, they will not be generated. The generated nodes are
//   stored in the passed vector dummyChildren. adjacentVerts is an unordered
//   list of vertex ids corresponding to those vertices that were adjacent to
//   the vertex eliminated to generate this node. Function will not expand any
//   children that result from eliminating one of these vertices. This is
//   possible because we can always find an optimal elimination order that,
//   while the graph is not a clique, only eliminates non-adjacent vertices
//   consecutively (Dirac? see Kloks 1994, page 8). Input param mid_layer holds
//   the middle layer in search, this is used to decide how to set ancest
//   pointer of children.
///////////////////////////////////////////////////////////////////////////////
void BreadthFirstTWNode::expand(const ALMGraph &graph, ushort ub,
		const vector<int> &adjacentVerts,
		vector<BreadthFirstTWNode> &dummyChildren, int mid_layer) const {

	// set up adjverts vector with constant time lookup
	vector<bool> adjverts(graph.vertices.size(), false);
	for (uint i=0; i<adjacentVerts.size(); i++)
		adjverts[adjacentVerts[i]] = true;

	// iterate through vertices in graph
	dummyChildren.clear();
	VertexList *curr = graph.vertices[0].next;
	while (curr!=NULL) {

		// check adjverts
		if (!adjverts[curr->vid]) {

			// check upper bound
			int vdeg = graph.adjLM[curr->vid][0].vid;
			if (vdeg<ub) {
				// generate child
				BreadthFirstTWNode n(this, curr->vid, vdeg, mid_layer);
				dummyChildren.push_back(n);
			}
		}
		curr=curr->next;
	}
}

///////////////////////////////////////////////////////////////////////////////
// BreadthFirstTWNode expandSubset - expands this node by only eliminating vertices from the
//   passed graph that correspond to vertices eliminated in the passed goal
//   node.
///////////////////////////////////////////////////////////////////////////////
void BreadthFirstTWNode::expandSubset(const ALMGraph &graph,
		vector<BreadthFirstTWNode> &dummyChildren, int mid_layer,
		const BreadthFirstTWNode &goal) const {
	dummyChildren.clear();
	VertexList *curr = graph.vertices[0].next;
	while (curr!=NULL) {
		if (goal.state.checkVert(curr->vid)) {
			int vdeg = graph.adjLM[curr->vid][0].vid;
			if (vdeg <= goal.gval) {
				BreadthFirstTWNode n(this, curr->vid, vdeg, mid_layer);
				dummyChildren.push_back(n);
			}
		}
		curr=curr->next;
	}
}

}
;
