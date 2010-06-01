#include "preproc_flags.h"
#include <algorithm>
#include "BFHT_DDD_OpenList.h"

#include <iostream>

namespace Treewidth_Namespace {

BFHT_DDD_OpenList::BFHT_DDD_OpenList(uint min_size) :
	top_val(0) {

#ifdef DEBUG_TW
	assert(min_size-1<(1U<<31));
#endif
	ol.reserve(min_size);
}

void BFHT_DDD_OpenList::detectDuplicates() {

	if (ol.empty())
		return;

	cout << " size before: " << ol.size();

	// sort list
	sort(ol.begin(), ol.end());
	// detect and replace duplicates
	int open_i = 0;
	int best_dupe_i = 0;
#ifndef MONOTONIC_H
	int greatest_dupe_h = ol[0].hValue();
#endif
	for (uint i=1; i<ol.size(); i++) {
#ifdef MONOTONIC_H
		if (ol[i]!=ol[i-1]) { // not a duplicate of previous nodes
			ol[open_i] = ol[best_dupe_i]; // copy node into next open position in list
			open_i++;
			best_dupe_i = i;
		} else { // duplicate
			if (ol[i].gValue() < ol[best_dupe_i].gValue()) { // best so far
				best_dupe_i = i;
			}
		}
#else
		if (ol[i]!=ol[i-1]) { // not a duplicate of previous nodes
			ol[best_dupe_i].setHValue(greatest_dupe_h); // set h-value
			ol[open_i] = ol[best_dupe_i]; // copy node into next open position in list
			open_i++;
			best_dupe_i = i;
			greatest_dupe_h = ol[i].hValue();
		} else { // duplicate
			if (ol[i].hValue() > greatest_dupe_h)
				greatest_dupe_h = ol[i].hValue();
			if (ol[best_dupe_i].gValue() > greatest_dupe_h) // this node might be better than best so far
				if (ol[i].gValue() < ol[best_dupe_i].gValue()) // it is better
					best_dupe_i = i;
		}
#endif
	}
	ol[best_dupe_i].setHValue(greatest_dupe_h);
	ol[open_i] = ol[best_dupe_i];
	open_i++;
	// remove obsolete nodes from end of ol
	ol.resize(open_i);

	cout << "  after: " << ol.size() << endl;
}

void BFHT_DDD_OpenList::setMidLayerAncestPtrs() {
	for (uint i=0; i<ol.size(); i++)
		ol[i].setAncest(&ol[i]);
}

void BFHT_DDD_OpenList::print(ostream &out) const {
	for (uint i=0; i<ol.size(); i++)
		out << ol[i].statePtr()->printState() << " " << ol[i].gValue() << " " << ol[i].hValue() << " " << ol[i].hash() << endl;
}

}
