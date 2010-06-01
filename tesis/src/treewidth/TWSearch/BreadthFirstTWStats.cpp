#include "preproc_flags.h"
#include "BreadthFirstTWStats.h"
#include <cassert>

namespace Treewidth_Namespace {

///////////////////////////////////////////////////////////////////////////////
// BreadthFirstTWStats outputStats - outputs the archived stats
///////////////////////////////////////////////////////////////////////////////
void BreadthFirstTWStats::outputStats(ostream &out) const {

	out << "STATS 1 overview\n"
			<< "first_expanded \tfirst_generated \tfirst_reduced "
			<< "total_expanded \ttotal_generated \tmax_alloc "
			<< "\texpand_last_iter \tgenerated_last_iter\n" << firstexp << "\t"
			<< firstgen << "\t" << firstred << "\t" << totalexp << "\t"
			<< totalgen << "\t" << maxNodes() << "\t" << lastexp << '\t' << lastgen
			<< endl;

	out << "STATS 2 search_for_tw\n" << "iteration\tdepth\tnodes\tmidnodes\n";
	assert(searchForTW_nodes.size()==searchForTW_midnodes.size());
	for (uint i=0; i<searchForTW_nodes.size(); i++) {
		assert(searchForTW_nodes[i].size()==searchForTW_midnodes[i].size());
		for (uint j=0; j<searchForTW_nodes[i].size(); j++)
			out << i << "\t" << j << "\t" << searchForTW_nodes[i][j] << "\t"
					<< searchForTW_midnodes[i][j] << endl;
	}

	out << "STATS 3 search_for_soln_path\n"
			<< "iteration\tdepth\tnodes\tmidnodes\n";
	assert(searchForSolnPath_nodes.size()==searchForSolnPath_midnodes.size()&&searchForSolnPath_midnodes.size()==searchForSolnPath_levels.size());
	for (uint i=0; i<searchForSolnPath_nodes.size(); i++) {
		assert(searchForSolnPath_nodes[i].size()==searchForSolnPath_midnodes[i].size()&&searchForSolnPath_midnodes[i].size()==searchForSolnPath_levels[i].size());
		for (uint j=0; j<searchForSolnPath_nodes[i].size(); j++)
			out << i << " \t" << searchForSolnPath_levels[i][j] << "\t"
					<< searchForSolnPath_nodes[i][j] << "\t"
					<< searchForSolnPath_midnodes[i][j] << endl;
	}
}

}
