#ifndef BESTTWOPENLIST_H_
#define BESTTWOPENLIST_H_

#include "preproc_flags.h"
#include <ext/hash_set>
#include <queue>
#include <set>
#include "BestTWNode.h"
#include "BestTWStats.h"

namespace Treewidth_Namespace {

typedef hash_set<BestTWNode*,TWNodePtr_hash,TWNodePtr_hash_eq> BestTWNodeHash;
typedef BestTWNodeHash::iterator BestTWNodeHashI;
typedef BestTWNodeHash::const_iterator BestTWNodeHashCI;

typedef priority_queue<BestTWNode*,vector<BestTWNode*>,
TWNodePtr_worse_samef> BestTWNodePqueue;

class BestTWOpenList {
private:

	BestTWNodeHash ht;
	set<int> used_bins;
	vector<BestTWNodePqueue> bins;
	int offset;
	int size_val;

public:

	BestTWOpenList(int list_size, int first_bin, int last_bin) :
		ht(list_size), bins(last_bin-first_bin+1), offset(first_bin),
				size_val(0) {
	}

	int size() const;
	int topValue() const;
	BestTWNode* find(BestTWNode *n);

	BestTWNode* toppop(BestTWStats *stats=NULL);
	void insert(BestTWNode *n);
	bool removeFromHash(BestTWNode *n);

#ifdef GATHER_STATS_DEBUG
	int ht_size() const;
#endif
};

///////////////////////////////////////////////////////////////////////////////
// BestTWOpenList size - returns number of nodes currently in open list
///////////////////////////////////////////////////////////////////////////////
inline int BestTWOpenList::size() const {
	return size_val;
}

///////////////////////////////////////////////////////////////////////////////
// BestTWOpenList topValue - returns the f-value of the top (or best) node in
//   open list.
///////////////////////////////////////////////////////////////////////////////
inline int BestTWOpenList::topValue() const {
	return *(used_bins.begin())+offset;
}

///////////////////////////////////////////////////////////////////////////////
// BestTWOpenList find - returns a pointer to a node in the list with the same
// state as n. If none, returns null.
///////////////////////////////////////////////////////////////////////////////
inline BestTWNode* BestTWOpenList::find(BestTWNode *n) {
	BestTWNodeHashI iter = ht.find(n);
	if (iter==ht.end())
		return NULL;
	else
		return *iter;
}

///////////////////////////////////////////////////////////////////////////////
// BestTWOpenList removeFromHash - if the given node is in the open list it
//   is removed from the hash table. It cannot be removed from the bins.
//   If node was in list, and thus was removed, function returns true, else
//   it returns false.
///////////////////////////////////////////////////////////////////////////////
inline bool BestTWOpenList::removeFromHash(BestTWNode *n) {
	if (ht.erase(n)==0)
		return false;
	else
		return true;
}

#ifdef GATHER_STATS_DEBUG
inline int BestTWOpenList::ht_size() const {
	return ht.size();
}
#endif

}

#endif /*BESTTWOPENLIST_H_*/
