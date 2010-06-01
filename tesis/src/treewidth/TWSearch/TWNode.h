#ifndef TWNODE_H_
#define TWNODE_H_

#include "preproc_flags.h"
#include "TWState.h"

namespace Treewidth_Namespace {

	//typedef unsigned short ushort;

	///////////////////////////////////////////////////////////////////////////
	// TWNode abstract base class
	///////////////////////////////////////////////////////////////////////////
	class TWNode
	{
	public:

		virtual ushort gValue() const =0;
		virtual ushort hValue() const =0;
		virtual ushort fValue() const =0;
		virtual ushort nVertsElim() const =0;
		virtual ushort nVertsRemaining() const =0;

		virtual const TWState* statePtr() const =0;
		virtual bool isState(const TWState &s) const =0;
		virtual bool operator==(const TWNode &rhs) const =0;
		virtual size_t hash() const =0;
	};

	///////////////////////////////////////////////////////////////////////////
	// TWNodePtr_worse_samef - type for comparing two pointers to TWNodes to
	//   determine which is "worse", i.e. lesser gValue, breaking ties with
	//   fewer vertices eliminated.
	///////////////////////////////////////////////////////////////////////////
	class TWNodePtr_worse_samef : public binary_function<TWNode*,TWNode*,bool>
	{
	public:
		bool operator() (const TWNode* x, const TWNode* y) const {
			if (x->gValue() < y->gValue()) // this favors nodes with higher gValues when fValues are tied
				return true;
			else if (x->gValue() > y->gValue())
				return false;
			else if (x->nVertsElim() < y->nVertsElim()) // this favors nodes with fewer verts (i.e. closer to solution) when gValues are tied
				return true;
			else
				return false;
		}
	};

	///////////////////////////////////////////////////////////////////////////
	// TWNodePtr_hash - type for computing hash of a pointer to a node
	///////////////////////////////////////////////////////////////////////////
	class TWNodePtr_hash : public unary_function<TWNode*,size_t>
	{
	public:
		size_t operator() (const TWNode* x) const {
			return x->hash();
		}
	};

	///////////////////////////////////////////////////////////////////////////
	// TWNodePtr_hash_eq - type for determining if two Nodes with the same
	//   hash are equivalent.
	///////////////////////////////////////////////////////////////////////////
	class TWNodePtr_hash_eq : public binary_function<TWNode*,TWNode*,bool>
	{
	public:
		bool operator() (const TWNode* x, const TWNode* y) const {
			return *x==*y;
		}
	};

};

#endif /*TWNODE_H_*/
