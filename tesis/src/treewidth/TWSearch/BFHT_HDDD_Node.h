#ifndef BFHT_HDDD_NODE_H_
#define BFHT_HDDD_NODE_H_

#include "preproc_flags.h"
#include <vector>
#include <functional>
#include <string>
#include <sys/types.h>

#include <iostream>

using namespace std;

namespace Treewidth_Namespace {

class BFHT_HDDD_State {
private:
	static uint nVerts_val;
	uint bits[TW_NINTS];
public:
	BFHT_HDDD_State();
	BFHT_HDDD_State(const BFHT_HDDD_State &s);
	//BFHT_HDDD_State& operator=(const BFHT_HDDD_State &rhs);
	bool operator==(const BFHT_HDDD_State &rhs) const;
	//bool operator<(const BFHT_HDDD_State &rhs) const;
	static void nVerts(uint nv);
	static uint nVerts();
	void setVert(int v);
	//void setVerts(const vector<int> &vs);
	//void setAll(uint totalverts);
	ushort nVertsElim() const;
	void getEliminatedVertices(vector<int> &verts) const;
	void getRemainingVertices(vector<int> &verts) const;
	//bool checkVert(int v) const;
	//void getDiff(const BFHT_HDDD_State &s, vector<int> &dest) const;
	size_t hash2() const;
	//string printState() const;

	void clearVert(int v);

	string printState() const;

};

inline BFHT_HDDD_State::BFHT_HDDD_State() {
	for (int i=0; i<TW_NINTS; i++)
		bits[i]=0;
}

inline BFHT_HDDD_State::BFHT_HDDD_State(const BFHT_HDDD_State &s) {
	for (int i=0; i<TW_NINTS; i++)
		bits[i] = s.bits[i];
}

inline bool BFHT_HDDD_State::operator==(const BFHT_HDDD_State &rhs) const {
	for (int i=0; i<TW_NINTS; i++)
	if (bits[i]!=rhs.bits[i])
	return false;
	return true;
}

inline void BFHT_HDDD_State::nVerts(uint nv) {
	nVerts_val = nv;
}

inline uint BFHT_HDDD_State::nVerts() {
	return nVerts_val;
}

inline void BFHT_HDDD_State::setVert(int v) {
	bits[(v-1)/TW_INDEX_MOD] |= (1U<<((v-1)%TW_INDEX_MOD));
}

inline void BFHT_HDDD_State::clearVert(int v) {
	bits[(v-1)/TW_INDEX_MOD] &= ~(1U<<((v-1)%TW_INDEX_MOD));
}

inline ushort BFHT_HDDD_State::nVertsElim() const {
	ushort count = 0;
	uint ii=0, bi=0;
	for (uint i=0; i<nVerts_val; i++) {
		if (bits[ii]&(1U<<bi))
			count++;
		if (++bi>=TW_INDEX_MOD) {
			bi=0;
			ii++;
		}
	}
	return count;
}

inline void BFHT_HDDD_State::getEliminatedVertices(vector<int> &verts) const {
	verts.clear();
	uint ii=0, bi=0;
	for (uint i=0; i<nVerts_val; i++) {
		if (bits[ii]&(1U<<bi))
		verts.push_back(i+1);
		if (++bi>=TW_INDEX_MOD) {
			bi=0;
			ii++;
		}
	}
}

inline void BFHT_HDDD_State::getRemainingVertices(vector<int> &verts) const {
	verts.clear();
	uint ii=0, bi=0;
	for (uint i=0; i<nVerts_val; i++) {
		if (!(bits[ii]&(1U<<bi)))
		verts.push_back(i+1);
		if (++bi>=TW_INDEX_MOD) {
			bi=0;
			ii++;
		}
	}
}

inline size_t BFHT_HDDD_State::hash2() const {
	return bits[0];
}

class BFHT_HDDD_Node {
private:
	BFHT_HDDD_State state;
	ushort last_elim;
	ushort gval;
	ushort hval;
public:
	BFHT_HDDD_Node();
	BFHT_HDDD_Node(ushort gv, ushort hv);
	//BFHT_HDDD_Node(const BFHT_HDDD_Node *p, int vid, int vdeg, int mid_layer);
	//BFHT_HDDD_Node(const BFHT_HDDD_Node &dummy, int mid_layer);
	ushort gValue() const;
	ushort hValue() const;
	ushort fValue() const;
	ushort nVertsElim() const;
	//ushort nVertsRemaining() const;
	const BFHT_HDDD_State* statePtr() const;
	//bool isState(const TWState &s) const;
	bool operator==(const BFHT_HDDD_Node &rhs) const;
	size_t hash2() const;
	void setAllVerts(uint totalverts);
	void setVerts(const vector<int> &toelim);
	void setGValue(ushort gv);
	void setHValue(ushort hv);

	void copyDuplicate(const BFHT_HDDD_Node &duplicate);
	void generateChild(const BFHT_HDDD_Node *parent, uint vid, uint deg);

	ushort lastElim() const;

};

inline BFHT_HDDD_Node::BFHT_HDDD_Node() {
}

inline BFHT_HDDD_Node::BFHT_HDDD_Node(ushort gv, ushort hv) :
last_elim(0), gval(gv), hval(hv) {
}

inline ushort BFHT_HDDD_Node::gValue() const {
	return gval;
}

inline ushort BFHT_HDDD_Node::hValue() const {
	return hval;
}

inline ushort BFHT_HDDD_Node::fValue() const {
	return (gval<hval) ? hval : gval;
}

inline ushort BFHT_HDDD_Node::nVertsElim() const {
	return state.nVertsElim();
}

inline const BFHT_HDDD_State* BFHT_HDDD_Node::statePtr() const {
	return &state;
}

inline bool BFHT_HDDD_Node::operator==(const BFHT_HDDD_Node &rhs) const {
	return state==rhs.state;
}

inline size_t BFHT_HDDD_Node::hash2() const {
	return state.hash2();
}

inline void BFHT_HDDD_Node::setHValue(ushort hv) {
	hval=hv;
}

inline void BFHT_HDDD_Node::copyDuplicate(const BFHT_HDDD_Node &duplicate) {
	last_elim = duplicate.last_elim;
	gval = duplicate.gval;
	if (hval < duplicate.hval);
	hval = duplicate.hval;
}

inline void BFHT_HDDD_Node::generateChild(const BFHT_HDDD_Node *parent,
		uint vid, uint deg) {
	state = parent->state;
	state.setVert(vid);
	last_elim = vid;
	gval = (deg<parent->gval ? parent->gval : deg);
#ifdef MONOTONIC_H
	hval = 0;
#else
	hval = (deg<parent->hval ? parent->hval : 0);
#endif
}

inline ushort BFHT_HDDD_Node::lastElim() const {
	return last_elim;
}

class BFHT_HDDD_NodeHash : public unary_function<BFHT_HDDD_Node*,size_t> {
public:
	size_t operator()(const BFHT_HDDD_Node *n) const {
		return n->hash2();
	}
};

class BFHT_HDDD_NodeEq :
public binary_function<BFHT_HDDD_Node*,BFHT_HDDD_Node*,bool> {
public:
	bool operator()(const BFHT_HDDD_Node *n1, const BFHT_HDDD_Node *n2) const {
		return *n1==*n2;
	}
};

}

#endif /*BFHT_HDDD_NODE_H_*/
