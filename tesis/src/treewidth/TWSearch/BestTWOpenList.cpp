#include "preproc_flags.h"
#include "BestTWOpenList.h"

namespace Treewidth_Namespace {

///////////////////////////////////////////////////////////////////////////////
// BestTWOpenList toppop - removes and returns "best" node in the open list.
///////////////////////////////////////////////////////////////////////////////
BestTWNode* BestTWOpenList::toppop(BestTWStats *stats) {
	set<int>::iterator bi = used_bins.begin();
	int b = *bi; // get the index of the first nonempty bin
	BestTWNode *n = bins[b].top(); // get first node from first bin
	bins[b].pop(); // remove node from bin
#if (defined GATHER_STATS_DEBUG || defined GATHER_STATS)
	int erase_count = ht.erase(n);
#else
	ht.erase(n); // remove node from hashtable
#endif
	if (bins[b].empty()) // if bin is empty...
		used_bins.erase(bi); // remove from used_bins set
	size_val--; // decrement size of openlist
#if (defined GATHER_STATS_DEBUG || defined GATHER_STATS)
	if (stats!=NULL) {
		stats->decCurrOpenInBins();
		stats->decCurrOpenHashSize(erase_count);
	}
#endif
	return n;
}

///////////////////////////////////////////////////////////////////////////////
// TWOpenList insert - inserts a "reduced" node into the open list bins and
//   hashtable.
// NOTE: function assumes node is not already in openlist, if it is d.s. will
//   become corrupted.
///////////////////////////////////////////////////////////////////////////////
void BestTWOpenList::insert(BestTWNode *n) {
#ifdef DEBUG_TW
	assert(ht.find(n)==ht.end());
#endif
	int b = n->fValue() - offset; // find n's bin
#ifdef DEBUG_TW
	assert(b>=0);
#endif
	if (bins[b].empty()) // if bin is empty...
		used_bins.insert(b); //   insert it in used_bin set
	bins[b].push(n); // insert n in bin
#ifdef DEBUG_TW
	pair<BestTWNodeHashCI,bool> ht_ret = ht.insert(n);
	assert(ht_ret.second);
#else
	ht.insert(n); // insert n in hashtable
#endif
	size_val++; // increment size of openlist
}

}
;
