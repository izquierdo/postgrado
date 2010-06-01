#include <fstream>
#include <iterator>
#include <sys/time.h>
#include <malloc.h>
#include "DFS.h"
#include "utils.h"

extern string g_graphname;

uint max_mem = 0;

namespace Treewidth_Namespace
{

#define DIFFTIME(st,et) (((et).tv_sec-(st).tv_sec)+(double)((et).tv_usec-(st).tv_usec)/(double)1e6)
#define DIFFSECS(st,et) ((et).tv_sec-(st).tv_sec)

  size_t hash_value(const Treewidth_Namespace::TWState &s)
  {
    return s.hash();
  }

TWExitStatus DFS::solve(DFSCutoffType cutofftype, ElimOrder &soln,
		const boolMatrix &adjmat, HeuristicVersion hversion, uint lb, uint ub,
		int timelim, int memlim)
{
	run_val = true;
	exit_status_val = TW_SUCCESS;
	m_stats.reset();

#ifdef DFS_TT
	if (cutofftype != DFS_ID_CUTOFF)
	{
		cerr << "Error: compiled with DFS_TT flag defined, DFS must be run with iterative deepening (i.e., dfid). To run dfbnb, recompile with DFS_TT undefined.\n";
		return TW_NONE;
	}
#endif

	// start timer
	struct timeval starttime, currtime;
	struct timezone tz;
	gettimeofday(&starttime, &tz);

	// initialize search, including...
	//   load graph, computer lower bound, reduce graph, normalize graph,
	//   and compute upper bound
	//	ALMGraph graph(TRACK_CN);
	ALMGraph graph;

	vector<int> elimprefix;
	int elimprefix_width;
	int postprefix_lb;
	vector<int> vertexmap;
	ElimOrder ub_order;
	bool ub_mapped;
	TWExitStatus init_status = initSearch(adjmat, hversion, graph, elimprefix,
			elimprefix_width, postprefix_lb, vertexmap, ub_order, ub_mapped,
			m_stats, lb, ub);
	cout << "Vertices: " << graph.nVerts << endl;
	cout << "Lower bound: " << m_stats.getInitialLB() << endl;
	cout << "Upper bound: " << ub_order.width << endl;
	if (init_status == TW_SUCCESS)
	{
		cout << "Treewidth: " << ub_order.width << endl;
		soln = solution_val = ub_order;
		exit_status_val = TW_SUCCESS;

		ofstream bmout("bm_output.txt", ios_base::app);
		if (!bmout.is_open())
		  {
		    cerr << "UNABLE TO OPEN bm_output.txt FOR READING\n";
		    return TW_NONE;
		  }
		size_t pos = g_graphname.rfind('/');
		if (pos == g_graphname.npos)
		  pos = 0;
		else
		  pos++;
		string gname = g_graphname.substr(pos);
		bmout.width(25);
		bmout << left << gname << " ";
		bmout.width(4);
		bmout << right << m_stats.getInitialLB() << " ";
		bmout.width(4);
		bmout << right << ub_order.width << " ";
		bmout.width(6);
		bmout << right << "yes ";
		bmout.width(4);
		bmout << right << ub_order.width << " ";
		bmout.width(12);
		bmout << right << m_stats.getTotalExpanded() << " ";
		bmout.width(12);
		bmout << right << m_stats.getTotalGenerated() << " ";
		bmout.width(8);
		gettimeofday(&currtime, &tz);
		bmout << right << DIFFSECS(starttime, currtime);
		uint mem = VmSize();
		if (mem > max_mem)
		  max_mem = mem;
		bmout.width(8);
		bmout << right << (max_mem>>10);
		bmout << endl;
		bmout.close();


		return exit_status_val;
	}

#if (defined DFS_TT) and (not defined TW_ANY_VERTS)
	// check size of graph
	if (graph.nVerts < TW_MIN_VERTS) {
		cerr << "Error: reduced graph has " << graph.nVerts
				<< " vertices, which is less than the minimum of "
				<< TW_MIN_VERTS
				<< " for the current compilation. Change preprocessing "
				<< "parameter TW_MIN_VERTS and recompile. Or recompile with "
				<< "TW_ANY_VERTS set.\n";
		return TW_NONE;
	}
	if (graph.nVerts > TW_MAX_VERTS) {
		cerr << "Error: reduced graph has " << graph.nVerts
				<< " vertices, which is greater than the maximum of "
				<< TW_MAX_VERTS
				<< " for the current compilation. Change preprocessing "
				<< "parameter DDD_MIN_VERTS to within 32 of "
				<< graph.nVerts << " and recompile. Or recompile with "
				<< "TW_ANY_VERTS set.\n";
		//return TW_NONE;
	}
#endif

	struct timeval itime, lastitime;
	gettimeofday(&itime, &tz);

#ifdef DEBUG_TW
	ALMGraph tmp_graph;
	tmp_graph.init(graph.nVerts);
	tmp_graph.copy(graph);
#endif

	uint orig_ub = ub_order.width;

	int cutoff;
	if (cutofftype == DFS_UB_CUTOFF)
		cutoff = ub_order.width;
	else
		cutoff = m_stats.getInitialLB() + 1;

	ofstream bmout("bm_output.txt", ios_base::app);
	if (!bmout.is_open())
	  {
	    cerr << "UNABLE TO OPEN bm_output.txt FOR READING\n";
	    return TW_NONE;
	  }
	size_t pos = g_graphname.rfind('/');
	if (pos == g_graphname.npos)
	  pos = 0;
	else
	  pos++;
	string gname = g_graphname.substr(pos);
	bmout.width(25);
	bmout << left << gname << " ";
	bmout.width(4);
	bmout << right << m_stats.getInitialLB() << " ";
	bmout.width(4);
	bmout << right << ub_order.width << " ";

	while (cutoff <= ub_order.width)
	{
#ifdef DEBUG_TW
		graph.copy(tmp_graph);
#endif
		// TODO for iterative deepening, want to reapply reduction rules
		// for each increased lower bound, because it may cause more vertices
		// to be almost simplicial.

		if (cutofftype == DFS_ID_CUTOFF)
			cout << "Deepening cutoff: " << cutoff << endl;
		ElimOrder tmp_ub_order;
		tmp_ub_order.width = cutoff;
		uint lb = (cutofftype == DFS_ID_CUTOFF) ? cutoff - 1
				: m_stats.getInitialLB();
		exit_status_val = solve_aux(graph, tmp_ub_order, ub_mapped, lb,
				hversion, starttime, timelim);
		if (tmp_ub_order.width < cutoff) // if a solution was found
		{
			assert(!tmp_ub_order.order_prefix.empty());
			ub_order = tmp_ub_order;
		}

		lastitime = itime;
		gettimeofday(&itime, &tz);
		cout << "Iteration Time: " << DIFFSECS(lastitime, itime) << " seconds\n";

		m_stats.printIterationOutput(cout);
		cout << endl;
		m_stats.nextIteration();

		cutoff++;
	}

	gettimeofday(&currtime, &tz);
	cout << "Total time: " << DIFFSECS(starttime, currtime) << " seconds\n";

	bmout.width(6);
	if (exit_status_val == TW_NOTIME)
	  bmout << right << "time ";
	else if (exit_status_val == TW_SUCCESS)
	  bmout << right << "yes ";
	else
	  bmout << right << "no ";
	bmout.width(4);
	bmout << right << ub_order.width << " ";
	bmout.width(12);
	bmout << right << m_stats.getTotalExpanded() << " ";
	bmout.width(12);
	bmout << right << m_stats.getTotalGenerated() << " ";
	bmout.width(8);
	bmout << right << DIFFSECS(starttime, currtime);
	uint mem = VmSize();
	if (mem > max_mem)
	  max_mem = mem;
	bmout.width(8);
	bmout << right << (max_mem>>10);
	bmout << endl;
	bmout.close();

	soln = ub_order;
	if (!vertexmap.empty() && ub_mapped)
		soln.remap(vertexmap);
	soln.appendPrefix(elimprefix);
	solution_val = soln;

	ofstream fout("result.txt");
	if (fout.is_open())
	{
		fout << g_graphname << " complete\n";
		fout << "Iteration Time: " << DIFFSECS(lastitime, itime) << " seconds\n";
		fout << "UB: " << orig_ub << endl;
		fout << "TW: " << ub_order.width << endl;
		fout << endl;
		m_stats.outputStats(fout);
		fout << endl;
#ifdef DFS_PRUNE_DVPR
		fout << "DVPR Enabled\n";
#endif
#ifdef DFS_TT
		fout << "TT Enabled";
#ifdef TT_REPLACE_RANDOM
		fout << ", Replace Random";
#endif
#ifdef TT_REPLACE_LRU
		fout << ", Replace LRU";
#endif
		fout << endl;
#endif
#ifdef DEBUG_TW
		fout << "DEBUG_TW Enabled\n";
#else
		fout << "DEBUG_TW Disabled\n";
#endif
		fout.close();
	}

	return exit_status_val;
}

// Backtrack to first ancestor that was not reduced further and does not
// exceed ub
#if (defined DFS_PRUNE_IVPR) and (defined DFS_PRUNE_DVPR) and (not defined DFS_TT)
void DFS::backtrack(ALMGraph &graph, 
		    vector<int> removed_edges[], 
		    vector<pair<int, int> > added_edges[], 
		    vector<VertexSet> &eclist, 
		    vector<dynamic_bitset> &noelimlist, 
		    vector<DFSNode> &path, 
		    int &d, 
		    ushort ub, DVPRData &dvsdata)
#elif (defined DFS_PRUNE_IVPR) and (not defined DFS_PRUNE_DVPR) and (not defined DFS_TT)
void DFS::backtrack(ALMGraph &graph, vector<int> removed_edges[], 
		    vector<pair<int, int> > added_edges[], 
		    vector<VertexSet> &eclist, 
		    vector<dynamic_bitset> &noelimlist, vector<DFSNode> &path, 
		    int &d, ushort ub)
#elif (not defined DFS_PRUNE_IVPR) and (not defined DFS_TT)
void DFS::backtrack(ALMGraph &graph, vector<int> removed_edges[], 
		    vector<pair<int, int> > added_edges[], 
		    vector<VertexSet> &eclist, vector<DFSNode> &path, int &d, 
		    ushort ub)
#elif (defined DFS_PRUNE_IVPR) and (defined DFS_PRUNE_DVPR) and (defined DFS_TT)
void DFS::backtrack(ALMGraph &graph, vector<int> removed_edges[], 
		    vector<pair<int, int> > added_edges[], 
		    vector<VertexSet> &eclist,
		    vector<dynamic_bitset> &noelimlist, vector<DFSNode> &path, 
		    int &d, ushort ub, DVPRData &dvsdata, TWState &state)
#elif (defined DFS_PRUNE_IVPR) and (not defined DFS_PRUNE_DVPR) and (defined DFS_TT)
void DFS::backtrack(ALMGraph &graph, vector<int> removed_edges[], 
		    vector<pair<int, int> > added_edges[], 
		    vector<VertexSet> &eclist,
		    vector<dynamic_bitset> &noelimlist, vector<DFSNode> &path, 
		    int &d, ushort ub, TWState &state)
#else
void DFS::backtrack(ALMGraph &graph, vector<int> removed_edges[], 
		    vector<pair<int, int> > added_edges[], 
		    vector<VertexSet> &eclist, vector<DFSNode> &path, int &d, 
		    ushort ub, TWState &state)
#endif
{
	do
	{
		// Revert eclist, noelimlist, and d
		eclist[d].clear();
#ifdef DFS_PRUNE_IVPR
		noelimlist[d].reset();
#endif
		d--;

		// Revert graph
		if (d >= 0)
		{
#ifdef DFS_PRUNE_DVPR
			dvsdata.updateAfterUnelim(d);
#endif
			graph.reverseElimVertex(path.back().vid(), removed_edges[d],
					added_edges[d]);
			removed_edges[d].clear();
			added_edges[d].clear();
#ifdef DFS_TT
			// Revert state
			state.clearVert(path.back().vid());
#endif
		}

		// Revert path
		path.pop_back();

	} while (!path.empty() && (path.back().reduced() || path.back().fValue()
			>= ub));
}

void DFS::writeResultFile(const char *outfile)
{
	if (!run_val)
		return;

	ofstream fout;
	if (outfile == NULL)
	{
		fout.open("dfbnb.out");
		if (!fout.is_open())
		{
			cerr << "Unable to open file dfbnb.out for writing results.\n";
			exit(1);
		}
	}
	else
	{
		fout.open(outfile);
		if (!fout.is_open())
		{
			cerr << "Unable to open file " << outfile
					<< " for writing results.\n";
			exit(1);
		}
	}

	fout << m_stats.graphParamsLabelsString() << " exit_status tw order\n";
	fout << m_stats.graphParamsString() << " ";
	if (exit_status_val == TW_NOTIME)
		fout << "1 ";
	else if (exit_status_val == TW_NOMEM)
		fout << "2 ";
	else
		fout << "0 ";

	// Output treewidth and optimal order if exist_status_val==0,
	// otherwise upper bound.
	fout << solution_val.width << ' ';
	copy(solution_val.order_prefix.begin(), solution_val.order_prefix.end(),
			ostream_iterator<int> (fout, " "));
	fout << endl;

	fout.close();
}

void DFS::writeStatFile(const char *statfile)
{
	if (!run_val)
		return;

	ofstream fout;
	if (statfile == NULL)
	{
		fout.open("dfbnb.stt");
		if (!fout.is_open())
		{
			cerr << "Unable to open file dfbnb.stt for writing results.\n";
			exit(1);
		}
	}
	else
	{
		fout.open(statfile);
		if (!fout.is_open())
		{
			cerr << "Unable to open file " << statfile
					<< " for writing results.\n";
			exit(1);
		}
	}

#if (defined GATHER_STATS_DEBUG || defined GATHER_STATS)
	m_stats.outputStats(fout);
#else
	fout << "Preprocessor flags not set to keep statistics.\n";
#endif
	fout.close();
}

TWExitStatus DFS::solve_aux(ALMGraph &graph, ElimOrder &ub_order,
		bool &ub_mapped, uint initiallb, HeuristicVersion hversion,
		const struct timeval starttime, const int timelim)
{
//   cout << "Graph:\n";
//   graph.print_graph(cout);
//   cout << endl;

	TWExitStatus exit_status_val = TW_SUCCESS;

	struct timeval lasttime;
	struct timezone tz;
	gettimeofday(&lasttime, &tz);

	uint ub = ub_order.width;

	// initialize number of vertices in original graph
	ushort nverts = graph.nVerts;

#ifdef DFS_TT
	// setup transposition table
	DFSTransTable tt;
	bool tt_half = false;
	bool tt_full = false;
#endif

	// initialize list of expanded children
	vector<VertexSet> eclist(nverts + 1, VertexSet(graph.vertices.size() - 1));

#ifdef DFS_PRUNE_IVPR
#ifdef DFS_PRUNE_DVPR
	DVPRData dvsdata(nverts);
#endif
	// initialize list of "no elim" vertices for tracking application of
	// pruning rules (combination of Gogate & Dechter 04, Thms 6.1-6.3
	typedef boost::dynamic_bitset<> dynamic_bitset;
	vector<dynamic_bitset> noelimlist(nverts + 1, dynamic_bitset(nverts + 1));
#endif

	// initialize lists of added and removed vertices
	vector<int> *removed_edges = new vector<int> [nverts];
	vector<pair<int, int> > *added_edges = new vector<pair<int, int> > [nverts];
	for (int i = 0; i < nverts; i++)
	{
		removed_edges[i].reserve(nverts);
		added_edges[i].reserve(nverts);
	}

	// initialize node stack for current path
	vector<DFSNode> path;

	// generate root node and push onto path
	path.push_back(DFSNode(initiallb));

#ifdef DFS_TT
	// setup root node's state
	TWState state;
#endif

#if (defined GATHER_STATS_DEBUG || defined GATHER_STATS)
	//m_stats.initProgressTracking(nverts, m_stats.getInitialLB());
#endif
	bool just_backtracked = false;

	// start search
	int d = 0;
	while (d >= 0)
	{
#ifdef DEBUG_TW
		assert(path.size()==(size_t)(d+1));
		assert(graph.vertices[0].vid==nverts-d);
#endif

		// check time limit
		struct timeval currtime;
		gettimeofday(&currtime, &tz);
		if (DIFFSECS(starttime, currtime) > timelim)
		{
			cerr << "DFBnB search exceeded max time limit of " << timelim
					<< " seconds.\n";
			exit_status_val = TW_NOTIME;
			break;
		}

		// periodic output
		if (DIFFSECS(lasttime, currtime) > 10)
		{
			/*
			 double lpruned = m_stats.getLeavesPruned();
			 double ltotal = m_stats.getTotalLeaves();
			 double ratio_pruned = lpruned / ltotal;
			 cerr << "  " << ratio_pruned * 100 << "% searched, " << lpruned
			 << "/" << ltotal << "\texpanded " << m_stats.getExpanded()
			 << "\t generated " << m_stats.getGenerated() << "\t";
			 */
			cerr << "  expanded " << m_stats.getExpanded() << "\t generated "
					<< m_stats.getGenerated() << "\t";
			uint mem = VmSize();
			if (mem > max_mem)
				max_mem = mem;
			mem >>= 10; // convert to MB
			cerr << " memory(mb) " << mem;
#ifdef DFS_TT
			cerr << "\t tt " << tt.size() 
			     << "\t tt-buckets " << tt.bucket_count();
#endif
			cerr << endl;
			lasttime = currtime;

#ifdef DFS_TT
			if (!tt_half && mem >= DFS_TT_MB_LIMIT/2)
			  {
			    tt_half = true;
			    cerr << "Half way to transposition table memory limit, "
				 << DFS_TT_MB_LIMIT << "MB\n";
			    tt.double_buckets();
			  }
			// If memory limit has been reached, stop growth of TT
			if (!tt_full && mem >= DFS_TT_MB_LIMIT)
			  {
			    tt_full = true;
			    cerr << "Transposition table memory limit reached, "
				 << DFS_TT_MB_LIMIT << "MB\n";
			    tt.set_noresize();
			  }
#endif
		}

		/////////////
		// MAYBE THIS SHOULDN"T BE COMMENTED
		////////////
/*
#ifdef DFS_TT
		// Insert expanded nodes into tt if this is the first time
		// one of its children will be generated
		if (!just_backtracked)
			tt.insert(state, d);
#endif
*/

#if (defined GATHER_STATS_DEBUG || defined GATHER_STATS)
		// Update stats if new node expanded
		if (!just_backtracked)
			m_stats.nodeExpanded();
#endif
		just_backtracked = false;

		// More children to generate
		if (eclist[d].size() < (size_t) (nverts - d))
		{
#ifdef DEBUG_TW
			for (uint i = 1; i < path.size(); ++i)
				assert(eclist[d].member(path[i].vid())==false);
#endif

			// Get vid of next child to generate
			int vid = graph.findVertex(FIND_MINFILL, eclist[d]);
			// int vid = graph.findVertex(FIND_MINDEG, eclist[d]);
			// int vid = graph.findVertex(FIND_ARBITRARY, eclist[d]);
			// int vid = graph.findVertex(FIND_FIXED, eclist[d]);

			// Update eclist
			eclist[d].insert(vid);

			// Get degree of vid
			ushort vdeg = graph.adjLM[vid][0].vid;

			// Try to bound child before generating it
			assert(path.back().fValue()<ub);
			if (vdeg >= ub)
			{
#if (defined GATHER_STATS_DEBUG || defined GATHER_STATS)
				//m_stats.pruned(d + 1);
#endif
				just_backtracked = true;
				continue;
			}

#ifdef DFS_TT
			// Update state
			state.setVert(vid);

			////////////////////////////////////
			// NOTE: may want find() to mark state as used
			////////////////////////////////////

			// Check the trans table and prune if a
			// duplicate
			if (tt.find(state) != tt.end())
			  {
			    state.clearVert(vid);
			    just_backtracked = true;
			    continue;
			  }

			// Insert new state in trans table
#ifdef DFS_PRUNE_GRDC
#warning "Seems unnecessary to store nodes that can be reduced in TT. Not sure how best to avoid this."
#endif
			pair<DFSTransTable::iterator,bool> insert_ret =
			  tt.insert(state);
			assert(insert_ret.second);
#endif

			// Generate child
			graph.elimVertex(vid, &removed_edges[d], &added_edges[d]);
			graph.addCommonNeighborEdges(ub, &added_edges[d]);
			ushort lb = graph.heuristic(hversion, ub);
			if (lb < initiallb)
			  lb = initiallb;
			DFSNode node(path.back(), vid, vdeg, lb);
#if (defined GATHER_STATS_DEBUG || defined GATHER_STATS)
			m_stats.nodeGenerated();
#endif
			assert ((lb >= ub && node.fValue() >= ub) ||
					(lb < ub && node.fValue() < ub));

			// Try to bound child
			if (node.fValue() >= ub)
			{
#ifdef DFS_PRUNE_DVPR
			  path.push_back(node);
			  dvsdata.updateAfterPrune(vid, vdeg, d, 
						   removed_edges[d], path);
			  path.pop_back();
#endif
				graph.reverseElimVertex(vid, removed_edges[d], added_edges[d]);
				removed_edges[d].clear();
				added_edges[d].clear();
#ifdef DFS_TT
				state.clearVert(vid);
#endif
#if (defined GATHER_STATS_DEBUG || defined GATHER_STATS)
				//m_stats.pruned(d + 1);
#endif
				just_backtracked = true;
				continue;
			}
			// Child does not exceed bound

			// Update path and d
			path.push_back(node);
			d++;

#ifdef DEBUG_OUTPUT
			cout << m_stats.getExpanded() << ": ";
			for (vector<DFSNode>::iterator i = path.begin(); i != path.end(); ++i)
			  cout << i->vid() << " ";
			cout << endl;
#endif

#ifdef DFS_PRUNE_DVPR
			// Update DVPR bookkeeping data, this node can
			// represent a "good enough" sequence of
			// dependent vertex eliminations
			dvsdata.updateAfterElim(vid, vdeg, d, removed_edges[d-1], path);
#endif

#ifdef DFS_PRUNE_IVPR
			// Update noelimlist
#ifdef DFS_PRUNE_DVPR
			assert(d == dvsdata.get_noelimlist_depth());
			updateNoElimList(noelimlist[d], noelimlist[d - 1], vid,
					 removed_edges[d - 1], dvsdata);
#else
			updateNoElimList(noelimlist[d], noelimlist[d - 1], vid,
					 removed_edges[d - 1]);
#endif
#endif

#ifdef DFS_PRUNE_GRDC
			assert(eclist[d].size()==0);
			// If any vertex in the graph can be
			// eliminated via reduction rules, then add
			// all other vertices to eclist such that only
			// the reduced vertex is eliminated
#ifdef DFS_PRUNE_IVPR
			// Check the vertices in noelimlist first
			int rvid = 
			  graph.findReduceableVertex(path.back().hValue(), 
						     noelimlist[d]);
#else
			int rvid = 
			  graph.findReduceableVertex(path.back().hValue());
#endif
			if (rvid > 0)
			  {
			    // mark node reduced
			    path.back().setReduced();
			    // if a reduceable vertex was found, then add
			    // all other vertices in graph to eclist so
			    // that they are not eliminated
			    for (VertexList *v = graph.vertices[0].next; v != NULL;
				 v = v->next)
			      {
				if (v->vid != rvid)
				  eclist[d].insert(v->vid);
			      }
			    assert((int)eclist[d].size() == (nverts - d - 1));
			  }
#endif

#ifdef DFS_PRUNE_AVDC
			// In Section 5.2 of (Gogate & Dechter 2004),
			// we see that we do not need to eliminate two
			// adjacent nodes consecutively unless the
			// graph is a clique. At this point the graph
			// cannot be a clique, thus we should prune
			// all nodes that result from eliminating
			// vertices adjacent to the last vertex in the
			// path.  To accomplish this we add the
			// indices of those vertices (stored in
			// removed_edges) to eclist (the expanded
			// children list).  If GRDC is also being used
			// and the graph can be reduced here, then we
			// cannot use AVDC to prune the only child of
			// this node.
			if (!path.back().reduced())
			  eclist[d].insert(removed_edges[d - 1].begin(), 
					   removed_edges[d - 1].end());
#endif

#ifdef DFS_PRUNE_IVPR
			// To ensure that noelim vertices are not eliminated we add them
			// to the eclist.
			eclist[d].insert(noelimlist[d]);
#endif

			// Check node for solution
			if (node.gValue() >= graph.nVerts - 1)
			{

				// Update upper bound
				assert(node.gValue() < ub);
				ub = ub_order.width = node.gValue();
				ub_order.order_prefix.clear();
				for (uint i = 1; i < path.size(); i++)
				{
					ub_order.order_prefix.push_back(path[i].vid());
				}
				ub_order.indiff_suffix = graph.getVertsSet();
				ub_mapped = true;
				cout << "NEW UPPER BOUND FOUND: " << ub << endl;

				ofstream ubout("new_ub_output.txt", ios_base::app);
				if (ubout.is_open())
				  {
				    size_t pos = g_graphname.rfind('/');
				    if (pos == g_graphname.npos)
				      pos = 0;
				    else
				      pos++;
				    string gname = g_graphname.substr(pos);
				    ubout.width(25);
				    ubout << left << gname << " ";
				    ubout.width(4);
				    ubout << right << ub << " ";
				    ubout.width(12);
				    ubout << right << m_stats.getExpanded() << " ";
				    ubout.width(12);
				    ubout << right << m_stats.getGenerated() << " ";
				    ubout.width(8);
				    gettimeofday(&currtime, &tz);
				    ubout << right << DIFFSECS(starttime, currtime);
				    uint mem = VmSize();
				    if (mem > max_mem)
				      max_mem = mem;
				    ubout.width(8);
				    ubout << right << (max_mem>>10);
				    ubout << endl;
				    ubout.close();
				  }
#ifdef DFS_PRUNE_DVPR
				// Update dvsdata, removing any valid
				// dependent vertex sequences with
				// cost >= the new ub
				dvsdata.updateAfterNewUB(ub);
#endif
#if (defined GATHER_STATS_DEBUG || defined GATHER_STATS)
				//m_stats.pruned(d);
#endif

				// Backtrack to next node to expand
#if (defined DFS_PRUNE_IVPR) and (defined DFS_PRUNE_DVPR) and (not defined DFS_TT)
				backtrack(graph, removed_edges, added_edges, 
					  eclist, noelimlist, path, d, ub, 
					  dvsdata);
#elif (defined DFS_PRUNE_IVPR) and (not defined DFS_PRUNE_DVPR) and (not defined DFS_TT)
				backtrack(graph, removed_edges, added_edges, 
					  eclist, noelimlist, path, d, ub);
#elif (not defined DFS_PRUNE_IVPR) and (not defined DFS_TT)
				backtrack(graph, removed_edges, added_edges, 
					  eclist, path, d, ub);
#elif (defined DFS_PRUNE_IVPR) and (defined DFS_PRUNE_DVPR) and (defined DFS_TT)
				backtrack(graph, removed_edges, added_edges, 
					  eclist, noelimlist, path, d, ub, 
					  dvsdata, state);
#elif (defined DFS_PRUNE_IVPR) and (not defined DFS_PRUNE_DVPR) and (defined DFS_TT)
				backtrack(graph, removed_edges, added_edges, 
					  eclist, noelimlist, path, d, ub, 
					  state);
#else
				backtrack(graph, removed_edges, added_edges, 
					  eclist, path, d, ub, state);
#endif
				just_backtracked = true;
			}

		}
		// No more children to generate
		else
		  {
		    // Backtrack to next node to expand
#if (defined DFS_PRUNE_IVPR) and (defined DFS_PRUNE_DVPR) and (not defined DFS_TT)
		    backtrack(graph, removed_edges, added_edges, eclist, 
			      noelimlist, path, d, ub, dvsdata);
#elif (defined DFS_PRUNE_IVPR) and (not defined DFS_PRUNE_DVPR) and (not defined DFS_TT)
		    backtrack(graph, removed_edges, added_edges, eclist, 
			      noelimlist, path, d, ub);
#elif (not defined DFS_PRUNE_IVPR) and (not defined DFS_TT)
		    backtrack(graph, removed_edges, added_edges, eclist, path, d, 
			      ub);
#elif (defined DFS_PRUNE_IVPR) and (defined DFS_PRUNE_DVPR) and (defined DFS_TT)
		    backtrack(graph, removed_edges, added_edges, eclist, 
			      noelimlist, path, d, ub, dvsdata, state);
#elif (defined DFS_PRUNE_IVPR) and (not defined DFS_PRUNE_DVPR) and (defined DFS_TT)
		    backtrack(graph, removed_edges, added_edges, eclist, 
			      noelimlist, path, d, ub, state);
#else
		    backtrack(graph, removed_edges, added_edges, eclist, path, 
			      d, ub, state);
#endif
		    just_backtracked = true;
		  }
	}

	delete[] removed_edges;
	delete[] added_edges;

#ifdef DFS_TT
	cout << "TT size: " << tt.size() << endl;
#endif
	cout << "Max memory usage: " << (max_mem>>10) << endl;

	return exit_status_val;
}

#ifdef DFS_PRUNE_IVPR
void DFS::updateNoElimList(dynamic_bitset &noelimlist, 
			   dynamic_bitset &parent_noelimlist, int vid, 
			   const vector<int> &adjacentverts
#ifdef DFS_PRUNE_DVPR
			   , const DVPRData &dvsdata
#endif
			   ) const
{
	// copy all vertices in parent_noelimlist to noelimlist,
	// excluding vid and any vertices adjacent to vid.
	assert(noelimlist.none());
	assert(noelimlist.size() == parent_noelimlist.size());
	assert(!parent_noelimlist[vid]);
	noelimlist = parent_noelimlist;
	for (size_t i = 0; i < adjacentverts.size(); ++i)
	  noelimlist.reset(adjacentverts[i]);

	// add vid to the parent noelimlist
	parent_noelimlist.set(vid);
#ifdef DFS_PRUNE_DVPR
	// add any other noelim verts found with DVPR pruning rule
	dvsdata.updateNoElimList(noelimlist);
#endif
}
#endif

#ifdef DFS_PRUNE_DVPR
/* Updates DVPRData after vertex vid is eliminated, where d is the depth
 * of the new node that results from the elimination, and adjverts is the set
 * of vertices in the graphs that were adjacent to vid before it was
 * eliminated. This function should be called if DFS is running iterative
 * deepening and the degree of vid when eliminated is less than the current ub.
 */
void DVPRData::updateAfterElim(int vid, int deg, int d, 
			      const vector<int> &adjverts,
			      const vector<DFSNode> &path)
{
#ifdef DEBUG_OUTPUT
  cout << "updateAfterElim, vid " << vid << ", depth " << d;
  cout << ", adjverts ";
  copy(adjverts.begin(), adjverts.end(), ostream_iterator<int>(cout," "));
  cout << endl;
#endif

  assert(((int)path.size() == d + 1));
  assert(path.back().vid() == vid);
  assert(m_table.get<depth>().lower_bound(d)==m_table.get<depth>().end());
  assert(m_adjverts_by_depth[d].none());
  assert(vid>0 && (uint)vid<m_adjverts_by_depth.size());
  assert(d>0 && (uint)d<m_adjverts_by_depth.size());

  m_noelimlist.reset();

  // Store adjverts data
  for (vector<int>::const_iterator v = adjverts.begin();
       v != adjverts.end(); ++v)
    {
      m_adjverts_by_depth[d].set(*v);
    }

  add(vid,deg,d,path);
  check(d,adjverts,path);

  m_noelimlist_depth = d;
}

// function finds and adds any new dependant-vertex-sequences that
// result from eliminating vertex vid (with degree deg) at depth d
// (via path path)
void DVPRData::add(int vid, int deg, int d, const vector<DFSNode> &path)
{
  // Add new good enough sequences to table
  deque<int> dependent_vertex_sequence;
  dependent_vertex_sequence.push_front(vid);
  SequenceState seq_state;
  seq_state.set(vid);
  int seq_cost = deg;

  // iteratate backwards through the path
  for (int i = d - 1; i >= 0; --i)
    {
      assert(seq_state.same_state(dependent_vertex_sequence));
      // check if this is the shallowest depth at which the current
      // sequence is valid, i.e., was the vertex eliminated at this
      // depth adjacent to any of the vertices in the current
      // sequence. if so, then we can add the sequence to the table
      bool anyadj = false;
      int adjv = NOVERT;
      int adjdeg = -1;
      for (deque<int>::iterator v = dependent_vertex_sequence.begin();
	   v != dependent_vertex_sequence.end() && !anyadj; ++v)
	{
	  if (m_adjverts_by_depth[i][*v])
	    {
	      anyadj = true;
	      adjv = path[i].vid();
	      adjdeg = path[i].degree();
	    }
	}
      // if this vertex is in the sequence, then update it
      if (i > 0 && anyadj)
	{
	  assert(anyadj || dependent_vertex_sequence.empty());
	  assert(adjv != NOVERT);
	  assert(adjdeg > -1);
	  dependent_vertex_sequence.push_front(adjv);
	  seq_state.set(adjv);
	  if (adjdeg > seq_cost)
	    seq_cost = adjdeg;

	  // Add sequence to table
	  if (dependent_vertex_sequence.size() > 1)
	    {
	      int seq_depth = i - 1;
	      
	      // Before adding the sequence to the table, first
	      // determine if it is already there
	      bool already_there = false;
	      pair<SequenceTableStateIndexIterator,
		SequenceTableStateIndexIterator> ret =
		m_temp_table.get<state>().equal_range(seq_state);
	      for (SequenceTableStateIndexIterator s = ret.first; 
		   s != ret.second; ++s)
		{
		  assert(s->second.depth() == seq_depth);
		  already_there = true;
		}

	      if (!already_there)
		{
		  m_temp_table.insert
		    (make_pair(seq_state, 
			       Sequence(dependent_vertex_sequence.begin(),
					dependent_vertex_sequence.end(), 
					seq_depth, seq_cost)));
		}
	    }
	}
    }
}

// function checks for any vertices that should not be eliminated
// because they contradict a valid sequence, and add them to
// noelimverts
void DVPRData::check(int d, const vector<int> &adjverts, const vector<DFSNode> &path)
{
  // for each vertex adjacent to vid, check to see if eliminating
  // that vertex next results in a nogood sequence
  for (vector<int>::const_iterator v = adjverts.begin();
       v != adjverts.end(); ++v)
    {
      deque<int> dependent_vertex_sequence;
      dependent_vertex_sequence.push_front(*v);
      SequenceState seq_state;
      seq_state.set(*v);

      // iteratate backwards through the path
      for (int i = d; i > 0; --i)
	{
	  assert(seq_state.same_state(dependent_vertex_sequence));
	  // if vertex at depth i is fadj, then extend the sequence
	  // and check for duplicates, otherwise if vertex at depth i
	  // is adjacent to any in the current sequence then no
	  // duplicate, if neither then continue
	  bool anyadj = false;
	  int adjv = NOVERT;
	  for (deque<int>::iterator w = dependent_vertex_sequence.begin();
	       w != dependent_vertex_sequence.end() && !anyadj; ++w)
	    {
	      if (m_adjverts_by_depth[i][*w])
		{
		  anyadj = true;
		  adjv = path[i].vid();
		}
	    }
	  if (anyadj)
	    {
	      assert(adjv != NOVERT);
	      dependent_vertex_sequence.push_front(adjv);
	      seq_state.set(adjv);
	      // Find all sequences with same state (duplicates)
	      pair<SequenceTableStateIndexIterator,
		SequenceTableStateIndexIterator> ret =
		m_table.get<state>().equal_range(seq_state);
	      // Find duplicate with deepest valid depth
	      SequenceTableStateIndexIterator goodseq = 
		m_table.get<state>().end();
	      int max_depth = -1;
	      for (SequenceTableStateIndexIterator s = ret.first; 
		   s != ret.second; ++s)
		{
		  assert(s->second.depth() < i);
		  // Update goodseq if deepest
		  if ((int)s->second.depth() > max_depth)
		    {
		      goodseq = s;
		      max_depth = s->second.depth();
		    }
		}
	      // See if dependent_vertex_sequence is nogood
      	      if (goodseq != m_table.get<state>().end())
		{
		  // If no vertices were eliminated after
		  // goodseq->depth and before the beginning of
		  // dependent_vertex_sequence (depth i-1) that were
		  // adjacent to any vertex in
		  // dependent_vertex_sequence, then
		  // dependent_vertex_sequence is a nogood duplicate
		  // of goodseq, i.e., v should not be eliminated.
		  bool anyadj = false;
		  for (int dpth = i-1; dpth > goodseq->second.depth() && 
			 !anyadj; --dpth)
		    {
		      for (deque<int>::iterator w = 
			     dependent_vertex_sequence.begin();
			   w != dependent_vertex_sequence.end() 
			     && !anyadj; ++w)
			{
			  if (m_adjverts_by_depth[dpth][*w])
			    {
			      anyadj = true;
			    }
			}
		    }
		  // Add v to noelimlist
		  if (!anyadj)
		    {
 		      assert(!goodseq->second.seq_eq(dependent_vertex_sequence));
		      m_noelimlist.set(*v);
		      // break from for-loop that seeks duplicates
		      // including *v, continue with next *v
		      break;
		    }
		}
	    }
	}
    }      
}

/* Updates DVPRData after backtracking, when vertex vid is un-eliminated, and d
 * is the depth of the node we're at after the unelimination, and adjverts are
 * the vertices now adjacent to vid.
 */
void DVPRData::updateAfterUnelim(int d)
{
#ifdef DEBUG_OUTPUT
  cout << "Erasing " << m_table.get<depth>().count(d+1) << " sequences at depth " << d+1 << endl;
  typedef SequenceTable::index_iterator<depth>::type It;
  pair<It,It> tmpret = m_table.get<depth>().equal_range(d+1);
  for (It iter = tmpret.first; iter != tmpret.second; iter++)
    {
      cout << "  ";
      copy(iter->second.begin(), iter->second.end(), ostream_iterator<int>(cout," "));
      cout << ", depth = " << (int)iter->second.depth();
      cout << endl;
    }
#endif

  // discard depth d+1 Sequences, no longer valid
  m_table.get<depth>().erase(Sequence(0,0,d+1,0));

  // clear depth d+1 in m_seqs_by_depth and m_adjverts_by_depth
  m_adjverts_by_depth[d+1].reset();

  // move depth d Sequences from temp_table to table
  typedef SequenceTable::index_iterator<depth>::type It;
  pair<It,It> ret = m_temp_table.get<depth>().equal_range(d);
  m_table.insert(ret.first, ret.second);
  m_temp_table.get<depth>().erase(Sequence(0,0,d,0));
}

/* Updates DVPRData when a vertex is eliminated and a valid node generated
 * (i.e., deg(vid)<ub), but the node is then pruned for another reason
 * (e.g., h(n)>=ub).
 *
 * NOTE: d should be the depth of the search before vid was eliminated and,
 * thus, the depth after it is pruned.
 *
 * This has the same effect as calling updateAfterElim(vid, d+1) and
 * updateAfterUnelim(vid, d) consecutively
 */
void DVPRData::updateAfterPrune(int vid, int deg, int depth, 
			       const vector<int> &adjverts,
			       const vector<DFSNode> &path)
{
#warning "if this is called many times, may want to optimize"
#ifdef DEBUG_OUTPUT
  cout << "Calling updateAfterPrune\n";
#endif

  updateAfterElim(vid, deg, depth + 1, adjverts, path);
  updateAfterUnelim(depth);
}

// Updates DVPRData after a new best solution is found, thus lowering
// the bound ub. Update implies removing all stored sequences with
// cost >= ub
void DVPRData::updateAfterNewUB(int ub)
{
#ifdef DEBUG_OUTPUT
  cout << "Calling updateAfterNewUB\n";
#endif
  SequenceTableDepthIndexIterator s = m_table.get<depth>().begin();
  while (s != m_table.get<depth>().end())
    {
      if ((int)s->second.cost() >= ub)
	s = m_table.get<depth>().erase(s);
      else
	++s;
    }
}

/* Add vertices in m_noelimlist to noelimlist */
void DVPRData::updateNoElimList(dynamic_bitset &noelimlist) const
{
  assert(noelimlist.size() == m_noelimlist.size());
  noelimlist |= m_noelimlist;
}
#endif

}

