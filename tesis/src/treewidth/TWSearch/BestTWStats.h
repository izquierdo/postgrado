#ifndef BESTTWSTATS_H_
#define BESTTWSTATS_H_

#include "preproc_flags.h"
#include <vector>
#include <iostream>
#include <map>
#include "TWStats.h"

namespace Treewidth_Namespace {

using namespace std;

class BestTWStats : public TWStats {
private:

	int curr_open_inbins; // inc,dec
	int curr_open_uselessinbins;
	int curr_open_hashsize; // add,sub
	int curr_closed_expanded;
	int curr_closed_pruned;
	int curr_closed_hashsize;

	vector<int> open_inbins;
	vector<int> open_uselessinbins;
	vector<int> open_hashsize;
	vector<int> closed_expanded;
	vector<int> closed_pruned;
	vector<int> closed_hashsize;

	int nexp; // number of nodes expanded (by gen all children or reduction)
	int ngen; // number of nodes generated
	int nred; // number of nodes "expanded" by reduction rules

	int last_expanded_fval; // f-value of last expanded node

#ifdef TRACK_EXP_BY_FVAL
	map<int,int> nexp_by_fval; // entries: <fval, nexp with that fval>
#endif

public:

	BestTWStats();
	virtual ~BestTWStats();

#ifdef TRACK_EXP_BY_FVAL
	void nodeExpanded(int fval);
#else
	void nodeExpanded();
#endif
	void nodeGenerated();
	void nodeReduced();

	int getExpanded()
	{
	  return nexp;
	}

	int getGenerated()
	{
	  return ngen;
	}

	void lastExpandedFValue(int val);
	int lastExpandedFValue() const;

	void incCurrOpenInBins();
	void decCurrOpenInBins();
	void incCurrOpenUselessInBins();
	void decCurrOpenUselessInBins();
	void incCurrOpenHashSize();
	void decCurrOpenHashSize(int val=1);
	void incCurrClosedExpanded();
	void decCurrClosedExpanded();
	void incCurrClosedPruned();
	void decCurrClosedPruned();
	void incCurrClosedHashSize();
	void decCurrClosedHashSize();

	bool isCurrOpenHashsize(int val) const;
	bool isCurrClosedHashsize(int val) const;
	bool isCurrClosedHashsizeConsistent() const;
	bool isCurrOpenInBins(int val) const;
	bool isCurrNodesConsistent(int extra) const;

	virtual void storeCurrs();
	virtual void outputCurrs(ostream &out) const;
	virtual void outputStats(ostream &out) const;
	virtual void reset();

#ifdef TRACK_EXP_BY_FVAL
	void outputExp(ostream &out) const;
#endif
};

///////////////////////////////////////////////////////////////////////////////
// Default constructor
///////////////////////////////////////////////////////////////////////////////
inline BestTWStats::BestTWStats() :
	curr_open_inbins(0), curr_open_uselessinbins(0), curr_open_hashsize(0),
			curr_closed_expanded(0), curr_closed_pruned(0),
			curr_closed_hashsize(0), nexp(0), ngen(0), nred(0),
			last_expanded_fval(0) {
}

///////////////////////////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////////////////////////
inline BestTWStats::~BestTWStats() {
}

///////////////////////////////////////////////////////////////////////////////
// nodeExpanded - called when a node is expanded
///////////////////////////////////////////////////////////////////////////////
#ifdef TRACK_EXP_BY_FVAL
inline void BestTWStats::nodeExpanded(int fval)
#else
inline void BestTWStats::nodeExpanded()
#endif
{
	nexp++;
#ifdef TRACK_EXP_BY_FVAL
	map<int,int>::iterator iter = nexp_by_fval.find(fval);
	if (iter==nexp_by_fval.end())
		nexp_by_fval[fval] = 1;
	else
		iter->second++;
#endif
}

///////////////////////////////////////////////////////////////////////////////
// nodeGenerated - called when a node is generated
///////////////////////////////////////////////////////////////////////////////
inline void BestTWStats::nodeGenerated() {
	ngen++;
}

///////////////////////////////////////////////////////////////////////////////
// nodeReduced - called when a node is reduced
///////////////////////////////////////////////////////////////////////////////
inline void BestTWStats::nodeReduced() {
	nred++;
}


///////////////////////////////////////////////////////////////////////////////
// lastExpandedFValue - sets the last expanded f-value stat
///////////////////////////////////////////////////////////////////////////////
inline void BestTWStats::lastExpandedFValue(int val) {
	last_expanded_fval=val;
}

///////////////////////////////////////////////////////////////////////////////
// lastExpandedFVal - returns the last expanded f-value stat
///////////////////////////////////////////////////////////////////////////////
inline int BestTWStats::lastExpandedFValue() const {
	return last_expanded_fval;
}

///////////////////////////////////////////////////////////////////////////////
// a bunch of stat update functions
///////////////////////////////////////////////////////////////////////////////
inline void BestTWStats::incCurrOpenInBins() {
	curr_open_inbins++;
}
inline void BestTWStats::decCurrOpenInBins() {
	curr_open_inbins--;
}
inline void BestTWStats::incCurrOpenUselessInBins() {
	curr_open_uselessinbins++;
}
inline void BestTWStats::decCurrOpenUselessInBins() {
	curr_open_uselessinbins--;
}
inline void BestTWStats::incCurrOpenHashSize() {
	curr_open_hashsize++;
}
inline void BestTWStats::decCurrOpenHashSize(int val) {
	curr_open_hashsize-=val;
}
inline void BestTWStats::incCurrClosedExpanded() {
	curr_closed_expanded++;
}
inline void BestTWStats::decCurrClosedExpanded() {
	curr_closed_expanded--;
}
inline void BestTWStats::incCurrClosedPruned() {
	curr_closed_pruned++;
}
inline void BestTWStats::decCurrClosedPruned() {
	curr_closed_pruned--;
}
inline void BestTWStats::incCurrClosedHashSize() {
	curr_closed_hashsize++;
}
inline void BestTWStats::decCurrClosedHashSize() {
	curr_closed_hashsize--;
}

inline bool BestTWStats::isCurrOpenHashsize(int val) const {
	return val==curr_open_hashsize;
}
inline bool BestTWStats::isCurrClosedHashsize(int val) const {
	return val==curr_closed_hashsize;
}
inline bool BestTWStats::isCurrClosedHashsizeConsistent() const {
	return curr_closed_hashsize == curr_closed_expanded + curr_closed_pruned;
}
inline bool BestTWStats::isCurrOpenInBins(int val) const {
	return val==curr_open_inbins;
}
inline bool BestTWStats::isCurrNodesConsistent(int extra) const {
	return isCurrNodes(curr_closed_hashsize + curr_open_hashsize
			+ curr_open_uselessinbins + extra);
}

///////////////////////////////////////////////////////////////////////////////
// storeCurrs - archives the current stats
///////////////////////////////////////////////////////////////////////////////
inline void BestTWStats::storeCurrs() {
	TWStats::storeCurrs();
	open_inbins.push_back(curr_open_inbins);
	open_uselessinbins.push_back(curr_open_uselessinbins);
	open_hashsize.push_back(curr_open_hashsize);
	closed_expanded.push_back(curr_closed_expanded);
	closed_pruned.push_back(curr_closed_pruned);
	closed_hashsize.push_back(curr_closed_hashsize);
}

inline void BestTWStats::outputCurrs (ostream &out) const {
	out << "NODES " << currNodes() << " \t"
		<< "OBINS " << curr_open_inbins << " \t"
		<< "OUSLS " << curr_open_uselessinbins << " \t"
		<< "OHASH " << curr_open_hashsize << " \t"
		<< "CEXPD " << curr_closed_expanded << " \t"
		<< "CPRND " << curr_closed_pruned << " \t"
		<< "CHASH " << curr_closed_hashsize << " \t"
	        << "MEM " << (currMem()>>20) << endl;
}

///////////////////////////////////////////////////////////////////////////////
// reset - resets all stats and archived stats
///////////////////////////////////////////////////////////////////////////////
inline void BestTWStats::reset() {
	TWStats::reset();
	curr_open_inbins=curr_open_uselessinbins=curr_open_hashsize=0;
	curr_closed_expanded=curr_closed_pruned=curr_closed_hashsize=0;
	nexp=ngen=nred=last_expanded_fval=0;
	open_inbins.clear();
	open_uselessinbins.clear();
	open_hashsize.clear();
	closed_expanded.clear();
	closed_pruned.clear();
	closed_hashsize.clear();
#ifdef TRACK_EXP_BY_FVAL
	nexp_by_fval.clear();
#endif
}

#ifdef TRACK_EXP_BY_FVAL
inline void BestTWStats::outputExp(ostream &out) const
{
	map<int,int>::const_iterator iter;
	for (iter=nexp_by_fval.begin(); iter!=nexp_by_fval.end(); iter++)
		out << iter->first << "\t" << iter->second << endl;
}
#endif


}
;

#endif /*BESTTWSTATS_H_*/
