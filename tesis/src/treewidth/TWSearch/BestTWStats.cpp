#include "preproc_flags.h"
#include "BestTWStats.h"

namespace Treewidth_Namespace {

///////////////////////////////////////////////////////////////////////////////
// BestTWStats outputStats - outputs the archived stat values
///////////////////////////////////////////////////////////////////////////////
void BestTWStats::outputStats(ostream &out) const {

	out << "STATS 1 overview\n"
			<< "total_expanded \ttotal_generated \tmax_alloc "
			<< "\ttotal_expanded_by_reduction \tmax_mem \ttotal_reopend\n"
			<< nexp << " \t" << ngen << " \t" << maxNodes() << " \t" << nred
			<< " \t" << maxMem() << endl;

	out << "STATS 2 search\n"
			<< "nodes \topenInBins \topenUseless \topenHash \texpanded "
			<< "\tclosedPruned \tclosedHash \tmem\n";

	for (uint i=0; i<open_inbins.size(); i++)
		out << storedNodes(i) << " \t" << open_inbins[i] << " \t"
				<< open_uselessinbins[i] << " \t" << open_hashsize[i] << " \t"
				<< closed_expanded[i] << " \t" << closed_pruned[i] << " \t"
				<< closed_hashsize[i] << " \t" << storedMem(i) << endl;
}

}
