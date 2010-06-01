#ifndef BREADTHFIRSTTWSTATS_H_
#define BREADTHFIRSTTWSTATS_H_

#include "preproc_flags.h"
#include <vector>
#include <iostream>
#include <cassert>
#include <map>
#include "TWStats.h"

namespace Treewidth_Namespace {

using namespace std;

class BreadthFirstTWStats : public TWStats {
private:

	int curr_level; // current level in search for tw, or current start level of divide and conquer search
	bool found_tw; // tw found

	vector<vector<int> > searchForTW_nodes;
	vector<vector<int> > searchForTW_midnodes;
	vector<vector<int> > searchForSolnPath_nodes;
	vector<vector<int> > searchForSolnPath_midnodes;
	vector<vector<int> > searchForSolnPath_levels;

	int nexp;
	int ngen;
	int nred;

	// these store stats for search for tw (as opposed to soln path)
	int firstexp;
	int firstgen;
	int firstred;

	// these only used by iterative deepening, store counts over ALL iterations
	int totalexp;
	int totalgen;
	int totalred;

	// for iterative deepening, store counts for last iteration
	int lastexp;
	int lastgen;
	int lastred;

#ifdef TRACK_EXP_BY_FVAL
	map<int,int> nexp_by_fval; // entries: <fval, nexp with that fval>
#endif

public:
	BreadthFirstTWStats();

	bool isTWFound() const;
	int lastLevel() const;

	void foundTW();
	void currLevel(int n);

#ifdef TRACK_EXP_BY_FVAL
	void nodeExpanded(int fval);
#else
	void nodeExpanded();
#endif
	void nodeGenerated(int n=1);
	void nodeReduced(int n=1);
	void updateLastIterationStats();
	void updateTotals();

	int getTotalExpanded() const
	{
	  return totalexp + nexp;
	}

	int getTotalGenerated() const
	{
	  return totalgen + ngen;
	}

	void numLayersInSearchForTW(int n);
	void setSearchForTWNodes(int depth, int n);
	void setSearchForTWMidNodes(int depth, int n);
	void setSearchForSolnPathNodes(int n);
	void setSearchForSolnPathMidNodes(int n);
	void setSearchForSolnPathLevels(int n);
	void anotherSearchForTWIteration();
	void anotherSearchForSolnPath();
	void finishedSearchForTW();

	virtual void outputStats(ostream &out) const;

	virtual void reset();

#ifdef TRACK_EXP_BY_FVAL
	void outputExp(ostream &out) const;
#endif

	int getExpanded() const
	{
		return nexp;
	}

	void printIterationOutput(ostream &out) const
	{
		out << "Expanded " << nexp << ", Generated " << ngen << endl;
		out << "Total Expanded " << nexp + totalexp << ", Total Generated " << ngen + totalgen << endl;
		out << "Max Memory Used " << maxMem() << endl;
	}

	void printFinalOutput(ostream &out) const
	{
		out << "Total Expanded " << nexp + totalexp << ", Total Generated " << ngen + totalgen << endl;
		out << "Max Memory Used " << maxMem() << endl;
	}	  
};

inline BreadthFirstTWStats::BreadthFirstTWStats() :
	curr_level(0), found_tw(false), searchForTW_nodes(1),
			searchForTW_midnodes(1), searchForSolnPath_nodes(1),
			searchForSolnPath_midnodes(1), searchForSolnPath_levels(1),
			nexp(0), ngen(0), nred(0), firstexp(0), firstgen(0), firstred(0),
			totalexp(0), totalgen(0), totalred(0), lastexp(0), lastgen(0), lastred(0) {
}

inline void BreadthFirstTWStats::foundTW() {
	found_tw = true;
}

inline bool BreadthFirstTWStats::isTWFound() const {
	return found_tw;
}

inline int BreadthFirstTWStats::lastLevel() const {
	return curr_level;
}

inline void BreadthFirstTWStats::currLevel(int n) {
	curr_level=n;
}

#ifdef TRACK_EXP_BY_FVAL
inline void BreadthFirstTWStats::nodeExpanded(int fval)
{
	nexp++;
	map<int,int>::iterator iter = nexp_by_fval.find(fval);
	if (iter==nexp_by_fval.end())
		nexp_by_fval[fval] = 1;
	else
		iter->second++;
}
#else
inline void BreadthFirstTWStats::nodeExpanded()
{
	nexp++;
}
#endif

inline void BreadthFirstTWStats::nodeGenerated(int n) {
	ngen+=n;
}

inline void BreadthFirstTWStats::nodeReduced(int n) {
	nred+=n;
}

inline void BreadthFirstTWStats::updateLastIterationStats() {
	lastexp=nexp;
	lastgen=ngen;
	lastred=nred;
}

inline void BreadthFirstTWStats::updateTotals() {
	totalexp+=nexp;
	nexp=0;
	totalgen+=ngen;
	ngen=0;
	totalred+=nred;
	nred=0;
}

inline void BreadthFirstTWStats::numLayersInSearchForTW(int n) {
	searchForTW_nodes.back().resize(n);
	searchForTW_midnodes.back().resize(n);
}

inline void BreadthFirstTWStats::setSearchForTWNodes(int depth, int n) {
	searchForTW_nodes.back()[depth]=n;
}

inline void BreadthFirstTWStats::setSearchForTWMidNodes(int depth, int n) {
	searchForTW_midnodes.back()[depth]=n;
}

inline void BreadthFirstTWStats::setSearchForSolnPathNodes(int n) {
	searchForSolnPath_nodes.back().push_back(n);
}

inline void BreadthFirstTWStats::setSearchForSolnPathMidNodes(int n) {
	searchForSolnPath_midnodes.back().push_back(n);
}

inline void BreadthFirstTWStats::setSearchForSolnPathLevels(int n) {
	searchForSolnPath_levels.back().push_back(n);
}

inline void BreadthFirstTWStats::anotherSearchForTWIteration() {
	searchForTW_nodes.push_back(vector<int>());
	searchForTW_midnodes.push_back(vector<int>());
}

inline void BreadthFirstTWStats::anotherSearchForSolnPath() {
	searchForSolnPath_nodes.push_back(vector<int>());
	searchForSolnPath_midnodes.push_back(vector<int>());
	searchForSolnPath_levels.push_back(vector<int>());
}

inline void BreadthFirstTWStats::finishedSearchForTW() {
	firstexp = totalexp;
	firstgen = totalgen;
	firstred = totalred;
}

inline void BreadthFirstTWStats::reset() {
	curr_level=0;
	found_tw=false;
	nexp=0;
	ngen=0;
	nred=0;
	firstexp=0;
	firstgen=0;
	firstred=0;
	totalexp=0;
	totalgen=0;
	totalred=0;
	searchForTW_nodes.resize(1);
	searchForTW_nodes.back().clear();
	searchForTW_midnodes.resize(1);
	searchForTW_midnodes.back().clear();
	searchForSolnPath_nodes.resize(1);
	searchForSolnPath_nodes.back().clear();
	searchForSolnPath_midnodes.resize(1);
	searchForSolnPath_midnodes.back().clear();
	searchForSolnPath_levels.resize(1);
	searchForSolnPath_levels.back().clear();
}

#ifdef TRACK_EXP_BY_FVAL
inline void BreadthFirstTWStats::outputExp(ostream &out) const
{
	map<int,int>::const_iterator iter;
	for (iter=nexp_by_fval.begin(); iter!=nexp_by_fval.end(); iter++)
		out << iter->first << "\t" << iter->second << endl;
}
#endif

}

#endif /*STATISTICS_H_*/
