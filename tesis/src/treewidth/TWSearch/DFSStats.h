#ifndef DFSSTATS_H_
#define DFSSTATS_H_

#include "TWStats.h"

namespace Treewidth_Namespace
{

class DFSStats : public TWStats
{
private:

	const static int MAX_VERTS_FOR_PROGRESS_TRACKING = 170;

	uint nexp;
	uint ngen;
	uint nred;

	uint totalexp;
	uint totalgen;
	uint totalred;

	double nleaves;
	set<double> nleaves_part;

	vector<double> leaves_table;
	uint soln_depth;

public:

	DFSStats() :
		nexp(0), ngen(0), nred(0), nleaves(0)
	{
	}

	void initProgressTracking(uint nverts, uint lb);

	void nodeExpanded(int n=1)
	{
		nexp+=n;
	}

	uint getExpanded() const
	{
		return nexp;
	}

	uint getTotalExpanded() const
	{
	  return totalexp;
	}

	void nodeGenerated(int n=1)
	{
		ngen+=n;
	}

	uint getGenerated() const
	{
		return ngen;
	}

	uint getTotalGenerated() const
	{
	  return totalgen;
	}

	void nodeReduced(int n=1)
	{
		nred+=n;
	}

	void pruned(uint depth)
	{
		if (leaves_table.empty())
			return;
		double newsum = nleaves + leaves_table[depth];
		if (newsum>nleaves && newsum>leaves_table[depth])
			nleaves = newsum;
		else
		{
			nleaves_part.insert(nleaves);
			nleaves = leaves_table[depth];
		}
	}

	double getLeavesPruned() const
	{
		if (leaves_table.empty())
			return 0;
		double total=0;
		set<double>::const_iterator ci = nleaves_part.begin();
		while (ci!=nleaves_part.end())
			total += *ci++;
		total += nleaves;
		return total;
	}

	double getTotalLeaves() const
	{
		if (leaves_table.empty())
			return 0;
		return leaves_table[0];
	}

	void outputStats(ostream &out) const
	{
		out << "STATS 1 overview\n" << "total_expanded \t"
				<< "generated \t" << "expanded_by_reduction\n";
		out << totalexp << " \t" << totalgen << " \t" << totalred << endl;
	}

	void nextIteration()
	{
		totalexp += nexp;
		totalgen += ngen;
		totalred += nred;
		nexp = ngen = nred = 0;
	}

	void printIterationOutput(ostream &out) const
	{
		out << "Expanded " << nexp << ", Generated " << ngen << endl;
		out << "Total Expanded " << nexp + totalexp << ", Total Generated " << ngen + totalgen << endl;
	}
};

inline void DFSStats::initProgressTracking(uint nverts, uint lb)
{
	nleaves = 0;
	soln_depth = nverts-lb;
	if (soln_depth<=(uint)MAX_VERTS_FOR_PROGRESS_TRACKING)
	{
		leaves_table.resize(soln_depth+1);
		leaves_table[soln_depth]=1;
		for (int i=soln_depth-1; i>=0; i--)
		{
			leaves_table[i] = leaves_table[i+1]*(nverts-i);
			assert(leaves_table[i]>leaves_table[i+1]);
		}
	}
}

}

#endif /*DFBNBSTATS_H_*/
