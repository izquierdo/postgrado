#ifndef BREADTHFIRSTTWNODE_H_
#define BREADTHFIRSTTWNODE_H_

#include "preproc_flags.h"
#include "ALMGraph.h"
#include "TWState.h"
#include "TWNode.h"

using namespace Adjacency_List_Matrix_Graph;

namespace Treewidth_Namespace {

class BreadthFirstTWNode : public TWNode {
private:
	BreadthFirstTWNode *ancest;
	TWState state;
	ushort nelim;
	ushort gval;
	ushort hval;
	short id;

public:

	BreadthFirstTWNode();
	BreadthFirstTWNode(ushort gv, ushort hv);
	BreadthFirstTWNode(const BreadthFirstTWNode *p, int vid, int vdeg,
			int mid_layer);
	BreadthFirstTWNode(const BreadthFirstTWNode &dummy, int mid_layer);

	ushort gValue() const;
	ushort hValue() const;
	ushort fValue() const;
	ushort nVertsElim() const;
	ushort nVertsRemaining() const;
	short vid() const;

	const TWState* statePtr() const;
	bool isState(const TWState &s) const;
	bool operator==(const TWNode &rhs) const;
	bool operator!=(const TWNode &rhs) const;
	bool operator<(const BreadthFirstTWNode &rhs) const;
	size_t hash() const;

	void expand(const ALMGraph &graph, ushort ub,
			const vector<int> &adjacentVerts,
			vector<BreadthFirstTWNode> &dummyChildren, int mid_layer) const;
	void expandSubset(const ALMGraph &graph,
			vector<BreadthFirstTWNode> &dummyChildren, int mid_layer,
			const BreadthFirstTWNode &goal) const;

	void setAllVerts();
	void setVerts(const vector<int> &toelim);
	void setGValue(ushort gv);
	void setHValue(ushort hv);
	BreadthFirstTWNode* getAncest() const;
	void setAncest(BreadthFirstTWNode *an);
	/*	bool sameState(const BreadthFirstTWNode *n) const;*/

#ifdef DEBUG_TW
	bool isAncest(const BreadthFirstTWNode *ptr) const;
#endif
};

/////////////////////////////////////////////////////////////////////////////
// Default constructor
/////////////////////////////////////////////////////////////////////////////
inline BreadthFirstTWNode::BreadthFirstTWNode() {
}

///////////////////////////////////////////////////////////////////////////////
// Root node constructor
///////////////////////////////////////////////////////////////////////////////
inline BreadthFirstTWNode::BreadthFirstTWNode(ushort gv, ushort hv) :
	ancest(NULL), nelim(0), gval(gv), hval(hv), id(-1) {
}

///////////////////////////////////////////////////////////////////////////////
// gValue - return g-value
///////////////////////////////////////////////////////////////////////////////
inline ushort BreadthFirstTWNode::gValue() const {
	return gval;
}

///////////////////////////////////////////////////////////////////////////////
// hValue - return h-value
///////////////////////////////////////////////////////////////////////////////
inline ushort BreadthFirstTWNode::hValue() const {
	return hval;
}

///////////////////////////////////////////////////////////////////////////////
// fValue - return f-value
///////////////////////////////////////////////////////////////////////////////
inline ushort BreadthFirstTWNode::fValue() const {
	return (gval>hval) ? gval : hval;
}

///////////////////////////////////////////////////////////////////////////////
// nVertsElim - return number of vertices eliminated in current state
///////////////////////////////////////////////////////////////////////////////
inline ushort BreadthFirstTWNode::nVertsElim() const {
	return nelim;
}

///////////////////////////////////////////////////////////////////////////////
// nVertsRemaining - return number of vertices remaining in current state
///////////////////////////////////////////////////////////////////////////////
inline ushort BreadthFirstTWNode::nVertsRemaining() const {
	return (uint)TWState::nVerts()-nelim;
}

///////////////////////////////////////////////////////////////////////////////
// vid - return id of vertex eliminated to generate node
///////////////////////////////////////////////////////////////////////////////
	inline short BreadthFirstTWNode::vid() const
	{
		return id;
	}

///////////////////////////////////////////////////////////////////////////////
// BreadthFirstTWNode statePtr - returns a pointer to node's state
///////////////////////////////////////////////////////////////////////////////
inline const TWState* BreadthFirstTWNode::statePtr() const {
	return &state;
}

///////////////////////////////////////////////////////////////////////////////
// BreadthFirstTWNode isState - is s the same as node's state
///////////////////////////////////////////////////////////////////////////////
inline bool BreadthFirstTWNode::isState(const TWState &s) const {
	return state==s;
}

///////////////////////////////////////////////////////////////////////////////
// BreadthFirstTWNode equivalence operator - checks is rhs has same state as
// this node
///////////////////////////////////////////////////////////////////////////////
inline bool BreadthFirstTWNode::operator==(const TWNode &rhs) const
{
	return rhs.isState(state);
}

inline bool BreadthFirstTWNode::operator!=(const TWNode &rhs) const
{
	return !rhs.isState(state);
}

inline bool BreadthFirstTWNode::operator<(const BreadthFirstTWNode &rhs) const
{
	return state<rhs.state;
}

///////////////////////////////////////////////////////////////////////////////
// BreadthFirstTWNode hash - calls the node's state's hash function
///////////////////////////////////////////////////////////////////////////////
inline size_t BreadthFirstTWNode::hash() const {
	return state.hash();
}

///////////////////////////////////////////////////////////////////////////////
// BreadthFirstTWNode setAllVerts - sets all verts in node's state
///////////////////////////////////////////////////////////////////////////////
inline void BreadthFirstTWNode::setAllVerts() {
	state.setAll();
	nelim=TWState::nVerts();
}

///////////////////////////////////////////////////////////////////////////////
// BreadthFirstTWNode setVerts - sets the verts in toelim in the node's state
// NOTE: function assumes none of the verts in toelim are already set
///////////////////////////////////////////////////////////////////////////////
inline void BreadthFirstTWNode::setVerts(const vector<int> &toelim) {
	state.setVerts(toelim);
	nelim+=toelim.size();
}

///////////////////////////////////////////////////////////////////////////////
// BreadthFirstTWNode setGValue - sets the node's g-value
///////////////////////////////////////////////////////////////////////////////
inline void BreadthFirstTWNode::setGValue(ushort gv) {
	gval=gv;
}

///////////////////////////////////////////////////////////////////////////////
// BreadthFirstTWNode setHValue - sets the node's h-value
///////////////////////////////////////////////////////////////////////////////
inline void BreadthFirstTWNode::setHValue(ushort hv) {
	hval=hv;
}

///////////////////////////////////////////////////////////////////////////////
// BreadthFirstTWNode getAncest - returns node's ancestor pointer
///////////////////////////////////////////////////////////////////////////////
inline BreadthFirstTWNode* BreadthFirstTWNode::getAncest() const {
	return ancest;
}

///////////////////////////////////////////////////////////////////////////////
// BreadthFirstTWNode setAncest - sets node's ancestor pointer
///////////////////////////////////////////////////////////////////////////////
inline void BreadthFirstTWNode::setAncest(BreadthFirstTWNode *an) {
	ancest=an;
}

/*///////////////////////////////////////////////////////////////////////////////
 // BreadthFirstTWNode sameState - check if two node's have the same state
 ///////////////////////////////////////////////////////////////////////////////
 inline bool BreadthFirstTWNode::sameState(const BreadthFirstTWNode *n) const {
 return state==n->state;
 }*/

#ifdef DEBUG_TW
inline bool BreadthFirstTWNode::isAncest(const BreadthFirstTWNode *ptr) const {
	return ancest==ptr;
}
#endif

inline size_t hash_value(const BreadthFirstTWNode &n)
{
  return n.hash();
}

}
;

#endif /*BREADTHFIRSTTWNODE_H_*/
