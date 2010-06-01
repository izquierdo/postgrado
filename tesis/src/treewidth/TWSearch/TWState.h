#ifndef TWSTATE_H_
#define TWSTATE_H_

#include "preproc_flags.h"
#include <vector>
#include <string>
#include <sstream>
#include <bitset>
#include <cassert>
#include <cmath>
#include <climits>
#include <iostream>
#include <cstdlib>
#include "lookup3.h"

namespace Treewidth_Namespace {
	using namespace std;

	class TWState {
		private:
			static int nVerts_val;
#ifdef TW_ANY_VERTS
			static int nInts;
			static const int indexMod = 8*sizeof(unsigned int);
			unsigned int *bits;
#else
			static const int nInts = TW_NINTS;
			static const int indexMod = TW_INDEX_MOD;
			unsigned int bits[TW_NINTS];
#endif

		public:

			TWState ();

			TWState (const TWState &tws);

#ifdef TW_ANY_VERTS
			~TWState ();
#endif

			TWState& operator=(const TWState &rhs);

			bool operator==(const TWState &rhs) const;

			bool operator<(const TWState &rhs) const;

			static int nVerts (int nverts);

			static int nVerts ();

			void setVert (int v);

			void setVerts (const vector<int> &vs);

			void setAll ();

			void clearVert(int v);

			void clearVerts (const vector<int> &vs);

			void getEliminatedVertices(vector<int> &verts,
					int suffix_len=INT_MAX) const;

			void getRemainingVertices(vector<int> &verts) const;

			bool checkVert (int v) const;

			void getDiff (const TWState &s, vector<int> &dest) const;

			size_t hash () const;

			string printState() const;

			uint diffSuffixLength(const TWState &new_state) const;
	};

///////////////////////////////////////////////////////////////////////////////
// TWState default constructor
///////////////////////////////////////////////////////////////////////////////
	inline TWState::TWState ()
	{
#ifdef TW_ANY_VERTS
		bits = new unsigned int [nInts];
#endif
		for (int i=0; i<nInts; i++)
			bits[i]=0;
	}

///////////////////////////////////////////////////////////////////////////////
// TWState copy constructor
///////////////////////////////////////////////////////////////////////////////
	inline TWState::TWState (const TWState &tws)
	{
		if (this!=&tws) {
#ifdef TW_ANY_VERTS
			bits = new unsigned int [nInts];
#endif
			for (int i=0; i<nInts; i++)
				bits[i]=tws.bits[i];
		}
	}

#ifdef TW_ANY_VERTS
///////////////////////////////////////////////////////////////////////////////
// TWState destructor
///////////////////////////////////////////////////////////////////////////////
	inline TWState::~TWState ()
	{
		delete [] bits;
	}
#endif

///////////////////////////////////////////////////////////////////////////////
// TWState assignment operator
///////////////////////////////////////////////////////////////////////////////
	inline TWState& TWState::operator =(const TWState &rhs)
	{
		if (this!=&rhs) {
#ifdef TW_ANY_VERTS
			bits = new unsigned int [nInts];
#endif
			for (int i=0; i<nInts; i++)
				bits[i]=rhs.bits[i];
		}
		return *this;
	}

///////////////////////////////////////////////////////////////////////////////
// TWState equivalence operator
///////////////////////////////////////////////////////////////////////////////
	inline bool TWState::operator== (const TWState &rhs) const
	{
		for (int i=0; i<nInts; i++)
			if (bits[i]!=rhs.bits[i])
				return false;
		return true;
	}

	inline bool TWState::operator< (const TWState &rhs) const
	{
		for (int i=nInts-1; i>=0; i--) {
			if (bits[i]<rhs.bits[i])
				return true;
			else if (bits[i]>rhs.bits[i])
				return false;
		}
		return false;
	}

///////////////////////////////////////////////////////////////////////////////
// TWState nVerts - static class initialization function.
///////////////////////////////////////////////////////////////////////////////
	inline int TWState::nVerts(int nverts)
	{
		//invalid input
		if (nverts<=0)
			return 0;
		else {
			nVerts_val=nverts;
#ifdef TW_ANY_VERTS
			return nInts=ceil(nVerts_val/(double)indexMod);
#else
			if (!(nverts > TW_MIN_VERTS && nverts <= TW_MAX_VERTS))
			  std::cerr << "Current compilation requires between "
			       << TW_MIN_VERTS + 1 << " and " << TW_MAX_VERTS
			       << " vertices, whereas this graph has " << nverts
			       << " vertices.\n";
			assert((nverts > TW_MIN_VERTS || TW_MIN_VERTS == 32) && nverts <= TW_MAX_VERTS);
			return nverts;
#endif
		}
	}

///////////////////////////////////////////////////////////////////////////////
// TWState nVerts accessor
///////////////////////////////////////////////////////////////////////////////
	inline int TWState::nVerts ()
	{
		return nVerts_val;
	}

///////////////////////////////////////////////////////////////////////////////
// TWState setVert - sets the specified vert in the state bitstring
///////////////////////////////////////////////////////////////////////////////
	inline void TWState::setVert (int v)
	{
		bits[(v-1)/indexMod] |= (1U<<((v-1)%indexMod));
	}

///////////////////////////////////////////////////////////////////////////////
// TWState setVerts - sets the vertices specified in the vector
///////////////////////////////////////////////////////////////////////////////
	inline void TWState::setVerts (const vector<int> &vs)
	{
		for (int i=0; i<(int)vs.size(); i++)
			setVert(vs[i]);
	}

///////////////////////////////////////////////////////////////////////////////
// TWState setAll - sets all bits corresponding to vertices
///////////////////////////////////////////////////////////////////////////////
	inline void TWState::setAll ()
	{
		for (int i=0; i<nInts-1; i++)
			bits[i]=(unsigned int)-1;
		int shiftamt = (nVerts_val-indexMod*(nInts-1));
		if (shiftamt < 32)
		  bits[nInts-1] = (1U << shiftamt) - 1;
		else
		  bits[nInts-1] = -1;
	}

///////////////////////////////////////////////////////////////////////////////
// TWState clearVert - clears the specified vert in the state bitstring
///////////////////////////////////////////////////////////////////////////////
	inline void TWState::clearVert (int v)
	{
		bits[(v-1)/indexMod] &= ~(1U<<((v-1)%indexMod));
	}

///////////////////////////////////////////////////////////////////////////////
// TWState clearVerts - clears the vertices specified in the vector
///////////////////////////////////////////////////////////////////////////////
	inline void TWState::clearVerts (const vector<int> &vs)
	{
		for (int i=0; i<(int)vs.size(); i++)
			clearVert(vs[i]);
	}

///////////////////////////////////////////////////////////////////////////////
// TWState getEliminatedVertices - returns a sorted list of eliminated vertices
// represented by this state (in the given vector). Note, if optional parameter
// suffix_len is set and it is less than the total size of the state (no.
// vertices in original graph), then only return the eliminated vertices with
// index <= suffix_len.
///////////////////////////////////////////////////////////////////////////////
	inline void TWState::getEliminatedVertices(vector<int> &verts,
			int suffix_len) const
	{
		verts.clear();
		unsigned int ii=0, bi=0;
		for (int i=0; i<nVerts_val && i<suffix_len; i++) {
			if (bits[ii]&(1U<<bi))
				verts.push_back(i+1);
			if (++bi>=(uint)indexMod) {
				bi=0;
				ii++;
			}
		}
	}

///////////////////////////////////////////////////////////////////////////////
// TWState getRemainingVertices - returns a sorted list of vertices that have
// not been eliminated in this state (in the given vector).
///////////////////////////////////////////////////////////////////////////////
	inline void TWState::getRemainingVertices(vector<int> &verts)
		const
	{
		verts.clear();
		unsigned int ii=0, bi=0;
		for (int i=0; i<nVerts_val; i++) {
			if ((bits[ii]&(1U<<bi))==0)
				verts.push_back(i+1);
			if (++bi>=(uint)indexMod) {
				bi=0;
				ii++;
			}
		}
	}

///////////////////////////////////////////////////////////////////////////////
// TWState checkVert - test if a given vertex is set in the bitstring
///////////////////////////////////////////////////////////////////////////////
	inline bool TWState::checkVert (int v) const
	{
		return (bits[(v-1)/indexMod] & (1U<<((v-1)%indexMod)))!=0;
	}

///////////////////////////////////////////////////////////////////////////////
// TWState getDiff - returns list of vertices eliminated in one state but not
// the other optimized for situation where only one state has elim verts that
// the other doesn't
///////////////////////////////////////////////////////////////////////////////
	inline void TWState::getDiff (const TWState &s, vector<int> &dest) const
	{
		dest.clear();
		dest.reserve(nVerts_val);
		for (int i=0; i<nInts; i++) {
			unsigned int diff = bits[i]^s.bits[i];
			int cnt = 0;
			while (diff!=0) {
				if (diff & 1U)
					dest.push_back(i*indexMod+cnt+1);
				diff >>= 1;
				cnt++;
			}
		}
	}

///////////////////////////////////////////////////////////////////////////////
// TWState hash - get the hash value for this state
///////////////////////////////////////////////////////////////////////////////
	inline size_t TWState::hash () const
	{
	  return hashword(bits, nInts, 0);
	}

	inline uint TWState::diffSuffixLength(const TWState &new_state) const
	{
		int diffint = 0;
		uint nshift = 0;
		for (int i=nInts-1; i>=0; i--)
		{
			uint xorval = bits[i] ^ new_state.bits[i];
			while (xorval > 0)
			{
				xorval >>= 1;
				nshift++;
			}
			if (nshift > 0)
			{
				diffint = i;
				break;
			}
		}
		uint suffix_len = diffint * indexMod + nshift;
		assert((int)suffix_len <= nVerts_val);
		return suffix_len;
	}

};
#endif /*TWSTATE_H_*/
