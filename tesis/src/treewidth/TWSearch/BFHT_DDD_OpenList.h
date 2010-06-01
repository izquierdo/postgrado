#ifndef BFHT_DD_OPENLIST_H_
#define BFHT_DD_OPENLIST_H_

#include "preproc_flags.h"
#include <vector>
#include "BreadthFirstTWNode.h"

namespace Treewidth_Namespace {

class BFHT_DDD_OpenList {
private:

	vector<BreadthFirstTWNode> ol;
	uint top_val;

public:

	BFHT_DDD_OpenList(uint min_size);

	int clear();

	BreadthFirstTWNode* topPtr();

	void pop();

	BreadthFirstTWNode* insert(const BreadthFirstTWNode &parent, int vid,
			int vdeg, int mid_layer);

	bool atEnd() const;
	int nLeft() const;

	bool empty() const;
	int size() const;

	void detectDuplicates();

	void setMidLayerAncestPtrs();

	void print(ostream &out) const;

};

inline int BFHT_DDD_OpenList::clear() {
	ol.clear();
	top_val=0;
	return 0;
}

inline BreadthFirstTWNode* BFHT_DDD_OpenList::topPtr() {
	if (atEnd())
		return NULL;
	return &ol[top_val];
}

inline void BFHT_DDD_OpenList::pop() {
	top_val++;
}

inline BreadthFirstTWNode* BFHT_DDD_OpenList::insert(
		const BreadthFirstTWNode &parent, int vid, int vdeg, int mid_layer) {
	ol.push_back(BreadthFirstTWNode(&parent, vid, vdeg, mid_layer));
#ifdef DEBUG_TW
	if (!ol.back().isAncest(NULL))
		assert(ol.back().getAncest()->nVertsElim()==mid_layer);
#endif
	return &ol.back();
}

inline bool BFHT_DDD_OpenList::atEnd() const {
	return top_val>=ol.size();
}

inline int BFHT_DDD_OpenList::nLeft() const {
	return ol.size()-top_val;
}

inline bool BFHT_DDD_OpenList::empty() const {
	return ol.empty();
}

inline int BFHT_DDD_OpenList::size() const {
	return ol.size();
}

}

#endif /*BFHT_DD_OPENLIST_H_*/
