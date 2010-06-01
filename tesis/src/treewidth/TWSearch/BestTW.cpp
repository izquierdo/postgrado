#include "preproc_flags.h"
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <malloc.h>
#include <cassert>
#include <sys/time.h>
#include <map>
#include "BestTW.h"
#include "utils.h"

#define MAX2(a,b) (((a)<(b)) ? (b) : (a))
#define DIFFTIME(st,et) (((et).tv_sec-(st).tv_sec)+(double)((et).tv_usec-(st).tv_usec)/(double)1e6)
#define DIFFSECS(st,et) ((et).tv_sec-(st).tv_sec)

#if (defined GATHER_STATS && defined GATHER_STATS_DEBUG)
"ERROR: only one of GATHER_STATS and GATHER_STATS_DEBUG can be defined";
#endif

extern string g_graphname;

namespace Treewidth_Namespace {

///////////////////////////////////////////////////////////////////////////////
// BestTW setSolution - sets the given solution with the given width and order
///////////////////////////////////////////////////////////////////////////////
void BestTW::setSolution(int width, const vector<int> &order, ElimOrder &soln) {
	soln.width = width;
	soln.order_prefix.clear();
	soln.order_prefix.insert(soln.order_prefix.end(), order.begin(),
			order.end());
	soln.indiff_suffix.clear();
}

///////////////////////////////////////////////////////////////////////////////
// BestTW setSolution - sets the given solution with an order with the given
//   prefix followed by the given order mapped with the vertex map. Width of
//   solution will be the greater of the prefix width and the order's width.
///////////////////////////////////////////////////////////////////////////////
void BestTW::setSolution(const ElimOrder &order, int prefix_width,
		const vector<int> &prefix, const vector<int> &vertexmap,
		ElimOrder &soln) {
	soln.width = MAX2(prefix_width,order.width);
	soln.order_prefix = order.getOrder(prefix, vertexmap);
	soln.indiff_suffix.clear();
}

///////////////////////////////////////////////////////////////////////////////
// BestTW setSolution - sets the given solution with given width, order prefix,
//   and suffix. Order is mapped with given vertexmap.
///////////////////////////////////////////////////////////////////////////////
void BestTW::setSolution(int width, const vector<int> &order,
		const set<int> &suffix, const vector<int> &prefix,
		const vector<int> &vertexmap, ElimOrder &soln) {
	ElimOrder tmp_soln;
	tmp_soln.width = width;
	tmp_soln.order_prefix = order;
	tmp_soln.indiff_suffix = suffix;
	setSolution(tmp_soln, width, prefix, vertexmap, soln);
}

///////////////////////////////////////////////////////////////////////////////
// BestTW solve - run best-first search for treewidth algorithm on the graph
//   in adjMat using the hversion variant of the MMD+ heuristic and the given
//   time limit. If a solution is found (returns TW_SUCCESS), optimal order
//   is stored in soln.
///////////////////////////////////////////////////////////////////////////////
TWExitStatus BestTW::solve(ElimOrder &soln, const boolMatrix &adjMat,
		HeuristicVersion hversion, uint p_lb, uint p_ub, int timelim,
		int memlim)
{
	assert(p_lb==0 && p_ub==UINT_MAX); // function hasn't been updated to use lb and ub parameters

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

	run_val=true;
	stats.reset();

	// start timer
	struct timeval starttime, lasttime;
	struct timezone tz;
	gettimeofday(&starttime, &tz);
	lasttime = starttime;
	lasttime.tv_sec -= 60;

	// set memory limit
	int mem_limit = memlim*(int)1e6; // mem limit in bytes

	// initialize search, including...
	//   load graph, computer lower bound, reduce graph, normalize graph,
	//   and compute upper bound
	ALMGraph graph;
	vector<int> elimprefix;
	int elimprefix_width;
	int postprefix_lb;
	vector<int> vertexmap;
	ElimOrder ub_order;
	bool ub_mapped;
	TWExitStatus init_status = initSearch(adjMat, hversion, graph, elimprefix,
			elimprefix_width, postprefix_lb, vertexmap, ub_order, ub_mapped, stats);
	cout << "Lower bound: " << stats.getInitialLB() << endl;
	cout << "Upper bound: " << ub_order.width << endl;

	bmout.width(4);
	bmout << right << stats.getInitialLB() << " ";
	bmout.width(4);
	bmout << right << ub_order.width << " ";
	bmout.width(6);

	if (init_status==TW_SUCCESS) {
		cout << "Treewidth: " << ub_order.width << endl;
		soln=solution_val=ub_order;
		exit_status_val=TW_SUCCESS;

		cout << "Max memory usage: " << (stats.maxMem()>>20) << endl;
		struct timeval currtime;
		gettimeofday(&currtime, &tz);
		cout << "Time: " << DIFFSECS(starttime,currtime) 
		     << " seconds\n";
		cout << "Expanded " << stats.getExpanded() << ", Generated " 
		     << stats.getGenerated() << endl;

		bmout.width(6);
		bmout << right << "yes ";
		bmout.width(4);
		bmout << right << ub_order.width << " ";
		bmout.width(12);
		bmout << right << stats.getExpanded() << " ";
		bmout.width(12);
		bmout << right << stats.getGenerated() << " ";
		bmout.width(8);
		bmout << right << DIFFSECS(starttime, currtime);
		bmout.width(8);
		bmout << right << (stats.maxMem()>>20);
		bmout << endl;
		bmout.close();
	
		return exit_status_val;
	}
	assert(ub_mapped);

#ifndef TW_ANY_VERTS
	// check size of graph
	if (graph.nVerts < TW_MIN_VERTS) {
		cerr << "Error: reduced graph has " << graph.nVerts
				<< " vertices, which is less than the minimum of "
				<< TW_MIN_VERTS
				<< " for the current compilation. Change preprocessing "
				<< "parameter TW_MIN_VERTS and recompile. Or recompile with "
				<< "TW_ANY_VERTS set.\n";

		cout << "Max memory usage: " << (stats.maxMem()>>20) << endl;
		struct timeval currtime;
		gettimeofday(&currtime, &tz);
		cout << "Time: " << DIFFSECS(starttime,currtime) 
		     << " seconds\n";
		cout << "Expanded " << stats.getExpanded() << ", Generated " 
		     << stats.getGenerated() << endl;
	
		bmout.width(6);
		bmout << right << "err";
		bmout << " Error: Compiled for >= " << TW_MIN_VERTS << " vertices\"";
		bmout << endl;
		bmout.close();
	
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

		cout << "Max memory usage: " << (stats.maxMem()>>20) << endl;
		struct timeval currtime;
		gettimeofday(&currtime, &tz);
		cout << "Time: " << DIFFSECS(starttime,currtime) 
		     << " seconds\n";
		cout << "Expanded " << stats.getExpanded() << ", Generated " 
		     << stats.getGenerated() << endl;

		bmout.width(6);
		bmout << right << "err";
		bmout << " Error: Compiled for <= " << TW_MAX_VERTS << " vertices\"";
		bmout << endl;
		bmout.close();
	
		return TW_NONE;
	}
#endif

	// initialize open list
	int min_openNodes_hash_size = (int)5e5;
	BestTWOpenList openNodes(min_openNodes_hash_size, MAX2(elimprefix_width,
			postprefix_lb), ub_order.width);

	// initialize closed list
	BestTWNodeHash closedNodes;

	// generate root node
	BestTWNode *root = new BestTWNode(elimprefix_width,postprefix_lb);
	stats.addNode();
#if (defined GATHER_STATS_DEBUG || defined GATHER_STATS)
	stats.nodeGenerated();
#endif

	// insert into open list
	openNodes.insert(root);
#if (defined GATHER_STATS_DEBUG || defined GATHER_STATS)
	stats.incCurrOpenInBins();
	stats.incCurrOpenHashSize();
#endif

	// setup upper bound, newub_node is used if a new and improved upper bound
	// is found during the search
	int ub = ub_order.width;
	BestTWNode *newub_node = NULL;

	//setup buffers and extra datastructures
	ALMGraph cur_graph, graph_buffer;
	cur_graph.init(graph.vertices.size()-1);
	graph_buffer.init(cur_graph.vertices.size()-1);
	vector<int> adjacentVerts, pAdjacentVerts;

	// start search
	BestTWNode *n=NULL;
	while (openNodes.size()>0) {
		struct timeval currtime;
		gettimeofday(&currtime, &tz);
		// check time limit
		if (DIFFSECS(starttime,currtime)>timelim) {
			stats.lastExpandedFValue(openNodes.topValue());
			cerr << "BestTW search exceeded max time limit of " << timelim
					<< " seconds.\n";
			exit_status_val=TW_NOTIME;
			if (newub_node!=NULL) {
				delete newub_node;
				stats.subNode();
			}
			if (newub_node==NULL)
				soln = ub_order;
			else {
				soln.width = ub;
				soln.indiff_suffix.clear();
				newub_node->recoverSolutionPath(soln.order_prefix);
			}
			solution_val=soln;

			cout << "Max memory usage: " << (stats.maxMem()>>20) << endl;
			struct timeval currtime;
			gettimeofday(&currtime, &tz);
			cout << "Time: " << DIFFSECS(starttime,currtime) 
			     << " seconds\n";
			cout << "Expanded " << stats.getExpanded() 
			     << ", Generated " << stats.getGenerated() << endl;

			bmout.width(6);
			bmout << right << "time ";
			bmout.width(4);
			bmout << right << ub_order.width << " ";
			bmout.width(12);
			bmout << right << stats.getExpanded() << " ";
			bmout.width(12);
			bmout << right << stats.getGenerated() << " ";
			bmout.width(8);
			bmout << right << DIFFSECS(starttime, currtime);
			bmout.width(8);
			bmout << right << (stats.maxMem()>>20);
			bmout << endl;
			bmout.close();

			return exit_status_val;
		}
		// periodic output
		if (DIFFSECS(lasttime,currtime)>30) {
			// check memory allocation limit
			uint mem = (VmSize() << 10);
			stats.setCurrMem(mem);
			if ((int)mem > mem_limit) {
				stats.lastExpandedFValue(openNodes.topValue());
				cerr << "BestTW search exceeded max memory usage of "
						<< (int)(mem_limit/1e6) << "MB.\n";
				exit_status_val=TW_NOMEM;
				if (newub_node!=NULL) {
					delete newub_node;
					stats.subNode();
				}
				if (newub_node==NULL)
					soln = ub_order;
				else {
					soln.width = ub;
					soln.indiff_suffix.clear();
					newub_node->recoverSolutionPath(soln.order_prefix);
				}
				solution_val=soln;

				cout << "Max memory usage: " << (stats.maxMem()>>20) << endl;
				struct timeval currtime;
				gettimeofday(&currtime, &tz);
				cout << "Time: " << DIFFSECS(starttime,currtime) 
				     << " seconds\n";
				cout << "Expanded " << stats.getExpanded() << ", Generated " 
				     << stats.getGenerated() << endl;
	
				bmout.width(6);
				bmout << right << "mem ";
				bmout.width(4);
				bmout << right << ub_order.width << " ";
				bmout.width(12);
				bmout << right << stats.getExpanded() << " ";
				bmout.width(12);
				bmout << right << stats.getGenerated() << " ";
				bmout.width(8);
				bmout << right << DIFFSECS(starttime, currtime);
				bmout.width(8);
				bmout << right << (stats.maxMem()>>20);
				bmout << endl;
				bmout.close();

				return exit_status_val;
			}
#ifdef GATHER_STATS_DEBUG
			assert(stats.isCurrOpenHashsize(openNodes.ht_size()));
			assert(stats.isCurrClosedHashsize(closedNodes.size()));
			assert(stats.isCurrClosedHashsizeConsistent());
			assert(stats.isCurrOpenInBins(openNodes.size()));
			int extra = (newub_node==NULL) ? 0 : 1;
			assert(stats.isCurrNodesConsistent(extra));
#endif
#if (defined GATHER_STATS_DEBUG || defined GATHER_STATS)
			stats.storeCurrs();
			stats.outputCurrs(cout);
#else
			cout << "  nodes: " << stats.currNodes() << " \tmem: "
			<< (int)(stats.currMem()/1e6) << "MB" << endl;
#endif
			lasttime=currtime;
		}
		// choose best node in open list for expansion
#if (defined GATHER_STATS_DEBUG || defined GATHER_STATS)
		n=openNodes.toppop(&stats);
#else
		n=openNodes.toppop();
#endif
		// test for completion (trivial path to goal node or lb>=ub)
		int fValue = n->fValue();
		if (fValue>=ub) { // ub is optimal
			delete n;
			stats.subNode();
			n=NULL;
			break;
		}
		if (fValue>=n->nVertsRemaining()-1) // this node is on opt path
			break;
		// check closed list
		BestTWNodeHashCI cni=closedNodes.find(n);
		if (cni!=closedNodes.end()) {
			// if node chosen for expansion is in closed list, then a
			// better duplicate (diff node, same state) must have been
			// generated after this one. it would have replaced this one
			// in the open hashtable, but this one could not be removed
			// from the pqueue openlist bins.
#ifdef DEBUG_BFTW
			assert((*cni)->fValue()<=fValue);
#endif
#if (defined GATHER_STATS_DEBUG || defined GATHER_STATS)
			stats.decCurrOpenUselessInBins(); // node was removed from bins
#endif
			delete n;
			stats.subNode();
			n=NULL;
			continue;
		}
		// get node's graph
		int vid = n->vid();
		adjacentVerts.clear();
		int pvid = (n->parentPtr()==NULL) ? -1 : n->parentPtr()->vid();
		pAdjacentVerts.clear();
		cur_graph.copy(graph);
		cur_graph.elimVertices(*(n->statePtr()), &vid, &adjacentVerts, &pvid,
				&pAdjacentVerts); // have to fill pSAAS list bc don't know if node was reduced
		// expand node by reduction rules
		n = expandByReduction(n, cur_graph, openNodes, closedNodes, ub,
				newub_node, adjacentVerts, pAdjacentVerts, hversion,
				graph_buffer);
		// expand node (by generating children)
		if (n!=NULL) {
			// but first, test for completion again (trivial path to goal node or lb>=ub)
			int fValue = n->fValue();
			if (fValue>=ub) { // ub is optimal
				delete n;
				stats.subNode();
				n=NULL;
				break;
			}
			if (fValue>=n->nVertsRemaining()-1) // this node is on opt path
				break;
			expandNode(n, cur_graph, openNodes, closedNodes, ub, newub_node,
					adjacentVerts, hversion, graph_buffer);
			// insert expanded node into closed list
			closedNodes.insert(n);
#if (defined GATHER_STATS_DEBUG || defined GATHER_STATS)
			stats.incCurrClosedExpanded();
			stats.incCurrClosedHashSize();
#endif
			n=NULL;
		}
	}
	assert(n==NULL || n->parentPtr()!=NULL);
	//if (n==NULL || n->parentPtr()==NULL) {
	if (n==NULL) {
		// upper bound order gives exact treewidth
		if (newub_node==NULL) // no new ub found
			soln = ub_order;
		else {
			soln.width = ub;
			soln.indiff_suffix.clear();
			newub_node->recoverSolutionPath(soln.order_prefix);
		}
		if (!vertexmap.empty())
			soln.remap(vertexmap);
		soln.appendPrefix(elimprefix);
	} else {
		// n is goal, or path to goal from n is trivial
		soln.width = n->fValue();
		soln.indiff_suffix.clear();
		n->recoverSolutionPath(soln.order_prefix);
		if (!vertexmap.empty())
			soln.remap(vertexmap);
		soln.appendPrefix(elimprefix);
	}
	solution_val=soln;
	exit_status_val=TW_SUCCESS;

	if (newub_node!=NULL) {
		delete newub_node;
		stats.subNode();
	}

	uint mem = (VmSize() << 10);
	stats.setCurrMem(mem);
	cout << "Max memory usage: " << (stats.maxMem()>>20) << endl;
	struct timeval currtime;
	gettimeofday(&currtime, &tz);
	cout << "Time: " << DIFFSECS(starttime,currtime) 
	     << " seconds\n";
	cout << "Expanded " << stats.getExpanded() << ", Generated " 
	     << stats.getGenerated() << endl;
	
	bmout.width(6);
	if (soln.width < ub_order.width)
	  bmout << right << "yes ";
	else
	  bmout << right << "no ";
	bmout.width(4);
	bmout << right << soln.width << " ";
	bmout.width(12);
	bmout << right << stats.getExpanded() << " ";
	bmout.width(12);
	bmout << right << stats.getGenerated() << " ";
	bmout.width(8);
	bmout << right << DIFFSECS(starttime, currtime);
	bmout.width(8);
	bmout << right << (stats.maxMem()>>20);
	bmout << endl;
	bmout.close();

	return exit_status_val;
}

///////////////////////////////////////////////////////////////////////////////
// BestTW expandByReduction - function attempts to expand the given node by
//   generating a single child that results from the reduction rules.
//   Additionally, the function attempts to continue to succesively expand the
//   generated children as long as (a) the graph can be reduced further, and
//   (b) the generated child's f-value does not exceed the open list's top
//   fvalue. At completion
//    - all "expanded" nodes will be inserted in closed list
//    - if the last child to be generated exceeds the open list top, then it
//      will be inserted in the open list and NULL will be returned
//    - otherwise, a pointer to the last child will be returned. This child can
//      be expanded in the traditional sense (by generating each of its
//      children).
//    - cur_graph will correspond to the returned node (if not NULL)
//	  - if NULL is not returned, then adjacentVerts will hold the ids of the
//      vertices formerly adjacent to the vertex eliminated to generate the
//      returned vertex. NOTE: if no vertices are eliminated in reduction, then
//      adjacentVerts will not be changed. If NULL is returned there are no
//      guarantees on what adjacentVerts will hold.
///////////////////////////////////////////////////////////////////////////////
BestTWNode* BestTW::expandByReduction(BestTWNode *n, ALMGraph &cur_graph,
		BestTWOpenList &openNodes, BestTWNodeHash &closedNodes, int &ub,
		BestTWNode *&newub_node, vector<int> &adjacentVerts,
		const vector<int> &pAdjacentVerts, HeuristicVersion hversion,
		ALMGraph &graph_buffer) {

	BestTWNode dummyChild;

	vector<int> adjacentVerts2;
	vector<int> *currAdjVerts = &adjacentVerts2;
	vector<int> *lastAdjVerts = &adjacentVerts;

#ifdef BEST_PRUNE2
	vector<bool> nopruneverts(cur_graph.vertices.size(),false);
	for (uint i=0; i<pAdjacentVerts.size(); i++)
		if (pAdjacentVerts[i] < n->vid())
			nopruneverts[pAdjacentVerts[i]]=true;
	vector<bool> *npv_ptr = &nopruneverts;
#endif

	while (cur_graph.potentialSimpAndAlmostSimp[0].vid>0) {

		currAdjVerts->clear();
		reduce_one_ret roret;
#ifdef BEST_PRUNE2
		if (npv_ptr!=NULL) // first iteration
			roret = cur_graph.reduceGraphOne(n->fValue(), ub, NULL,
					currAdjVerts, REDUCE_LEAST_FIRST, n->vid(), npv_ptr);
		else // subsequent iterations
			roret = cur_graph.reduceGraphOne(n->fValue(), ub, NULL,
					currAdjVerts);
#else
		roret = cur_graph.reduceGraphOne(n->fValue(), ub, NULL, currAdjVerts);
#endif

		// if a vertex was eliminated in reduction
		if (roret.elimVert!=-1) {
#if (defined GATHER_STATS_DEBUG || defined GATHER_STATS)
	#ifdef TRACK_EXP_BY_FVAL
			stats.nodeExpanded(n->fValue()); // count reducing a node as expanding by genreating its "single" child
	#else
			stats.nodeExpanded();
	#endif
			stats.nodeReduced();
#endif
			// insert parent node in closed list
			closedNodes.insert(n);
#if (defined GATHER_STATS_DEBUG || defined GATHER_STATS)
			stats.incCurrClosedExpanded();
			stats.incCurrClosedHashSize();
#endif
#ifdef BEST_PRUNE2
			// if first iteration, elim vert id < n->vid, and elim vert id
			// is not in pAdjacentVerts, return NULL
			// (pruning rule 6.3 from Gogate and Dechter 2004)
			if (npv_ptr!=NULL) {
				if (roret.elimVert < n->vid() && !(*npv_ptr)[roret.elimVert])
					return NULL;
				npv_ptr = NULL;
			}
#endif

			// if degree of elim'ed vert exceeds ub, return NULL
			if (roret.deg>=ub)
				return NULL;

			// compute new node's hval, if it exceeds ub return NULL
			int tmph = cur_graph.heuristic(hversion, ub);
			if (tmph >= ub)
				return NULL;

			// otherwise generate child node
			dummyChild.generateDummy(n, roret.elimVert, roret.deg);

			// set hval
			if (tmph > dummyChild.hValue())
				dummyChild.setHValue(tmph);

			// check for new upper bound
			if (dummyChild.gValue() >= cur_graph.nVerts-1) {
				ub = dummyChild.gValue();
				if (newub_node!=NULL) {
					delete newub_node;
					stats.subNode();
				}
				newub_node = new BestTWNode(dummyChild);
				stats.addNode();
				cout << "New upper bound found: " << ub << endl;
				return NULL;
			}

			// check closed list
			BestTWNodeHashCI cnhi = closedNodes.find(&dummyChild);
			if (cnhi!=closedNodes.end()) // if in closed, prune
				return NULL;

			// if child f-val exceeds openNodes top fvalue, insert in open and return NULL
			if (dummyChild.fValue() > openNodes.topValue()) {

				// check open list
				if (!checkOpenList(dummyChild, openNodes)) { // if not in open list

					// allocate node
					BestTWNode *child = new BestTWNode(dummyChild);
					stats.addNode();
#if (defined GATHER_STATS_DEBUG || defined GATHER_STATS)
					stats.nodeGenerated();
#endif

//					// if hval exceeds ub, insert in closed list
//					if (child->hValue()>=ub) {
//						closedNodes.insert(child);
//#if (defined GATHER_STATS_DEBUG || defined GATHER_STATS)
//						stats.incCurrClosedPruned();
//						stats.incCurrClosedHashSize();
//#endif
//					}
//					// otherwise, insert in open list
//					else {
					openNodes.insert(child);
#if (defined GATHER_STATS_DEBUG || defined GATHER_STATS)
					stats.incCurrOpenInBins();
					stats.incCurrOpenHashSize();
#endif
//					}
				}
				return NULL;
			}

			// otherwise, allocate node
			BestTWNode *child = new BestTWNode(dummyChild);
			stats.addNode();
#if (defined GATHER_STATS_DEBUG || defined GATHER_STATS)
			stats.nodeGenerated();
#endif

			// if node has a duplicate in open, remove from hash (can't remove from bins)
#if (defined GATHER_STATS_DEBUG || defined GATHER_STATS)
			if (openNodes.removeFromHash(child)) {
				stats.decCurrOpenHashSize();
				stats.incCurrOpenUselessInBins();
			}
#else
			openNodes.removeFromHash(child);
#endif

			// save new adjacent verts list
			vector<int> *tmpAdjVerts = lastAdjVerts;
			lastAdjVerts = currAdjVerts;
			currAdjVerts = tmpAdjVerts;

			// test for completion (trivial path to goal node)
			if (n->gValue()>=child->nVertsRemaining()-1) {
				if (lastAdjVerts != &adjacentVerts)
					adjacentVerts = *lastAdjVerts;
				return child; // done
			}

			// set child to parent, repeat
			n=child;
		}
	}
	if (lastAdjVerts != &adjacentVerts)
		adjacentVerts = *lastAdjVerts;
	return n;
}

///////////////////////////////////////////////////////////////////////////////
// BestTW expandNodes - expands nodes by generating its children.
///////////////////////////////////////////////////////////////////////////////
// could be more efficient: instead of copying cur_graph to graph_buffer
// each time a child h-value is computed, just add and remove edges to
// cur_graph when necessary.
void BestTW::expandNode(BestTWNode *n, ALMGraph &cur_graph,
		BestTWOpenList &openNodes, BestTWNodeHash &closedNodes, int &ub,
		BestTWNode *&newub_node, const vector<int> &adjacentVerts,
		HeuristicVersion hversion, ALMGraph &graph_buffer) {
	static vector<BestTWNode> dummyChildren(TWState::nVerts());

	// expand node, fill dummyChildren with children to generate
#ifdef BEST_PRUNE3
	n->expand(cur_graph, ub, adjacentVerts, dummyChildren);
#else
	vector<int> emptyvec;
	n->expand(cur_graph, ub, emptyvec, dummyChildren);
#endif
#if (defined GATHER_STATS_DEBUG || defined GATHER_STATS)
	#ifdef TRACK_EXP_BY_FVAL
	stats.nodeExpanded(n->fValue());
	#else
	stats.nodeExpanded();
	#endif
#endif
	for (int i=0; i<(int)dummyChildren.size(); i++) {

		// check for new upper bound (gval < current ub, unless ub was just
		// lowered by a previous dummy child
		if (dummyChildren[i].gValue() >= cur_graph.nVerts-2) {
			if (dummyChildren[i].gValue() >= ub)
				continue;
			ub = dummyChildren[i].gValue();
			if (newub_node!=NULL) {
				delete newub_node;
				stats.subNode();
			}
			newub_node = new BestTWNode(dummyChildren[i]);
			stats.addNode();
			cout << "New upper bound found: " << ub << endl;
			continue;
		}

		// check closed list
		BestTWNodeHashCI cnhi = closedNodes.find(&dummyChildren[i]);
		if (cnhi!=closedNodes.end())
			continue;

		// check open list
		if (checkOpenList(dummyChildren[i], openNodes))
			continue;

		// not in closed list or open list
		// allocate child node and compute hval
		BestTWNode *child = new BestTWNode(dummyChildren[i]);
		stats.addNode();
#if (defined GATHER_STATS_DEBUG || defined GATHER_STATS)
		stats.nodeGenerated();
#endif
		child->setHValue(SHV_PARENTS_GRAPH, cur_graph, hversion, ub,
				graph_buffer);

		// if hval exceeds ub, insert in closed list
		if (child->hValue()>=ub) {
			closedNodes.insert(child);
#if (defined GATHER_STATS_DEBUG || defined GATHER_STATS)
			stats.incCurrClosedPruned();
			stats.incCurrClosedHashSize();
#endif
			continue;
		}

		// insert in open list
		openNodes.insert(child);
#if (defined GATHER_STATS_DEBUG || defined GATHER_STATS)
		stats.incCurrOpenInBins();
		stats.incCurrOpenHashSize();
#endif
	}
}

///////////////////////////////////////////////////////////////////////////////
// BestTW checkOpenList - checks if a dummy node is a duplicate of a node in
//   the open list, and if it is:
//    - if dummy is better, removes old node from open hash, allocates new
//      node, and inserts it into the open list, returns true
//    - if old node is better, returns true
//   If it's not a duplicate, returns false.
///////////////////////////////////////////////////////////////////////////////
/// removing old from hash could be made more efficient if iterator is
/// saved from initial find
bool BestTW::checkOpenList(BestTWNode &dummy, BestTWOpenList &openNodes) {
	BestTWNode *old = openNodes.find(&dummy);
	if (old==NULL)
		return false;
	else { // duplicate found in open list
		//	NHashI onhi = openNodes.ht.find(&dummy);
		//	if (onhi==openNodes.ht.end()) // not in open list
		//		return false;
		//	else { // duplicate found in open list
		//		Node *old = *onhi;

#ifdef MONOTONIC_H
		// set dummy's hvalue
		dummy.setHValue(old);
#else
		// give old and new nodes both best hvalue
		dummy.setHValue(old);
		old->setHValue(&dummy);
#endif

		if (old->hValue()<old->gValue() && old->gValue()>dummy.gValue()) { // new node better, replace old

			// allocate new node
			BestTWNode *newnode = new BestTWNode(dummy);
			stats.addNode();
#if (defined GATHER_STATS_DEBUG || defined GATHER_STATS)
			stats.nodeGenerated();
#endif

			// replace old with new in open list
			openNodes.removeFromHash(old);
			//		openNodes.ht.erase(onhi);
			openNodes.insert(newnode);
#if (defined GATHER_STATS_DEBUG || defined GATHER_STATS)
			stats.decCurrOpenHashSize(); // old removed from hash
			stats.incCurrOpenUselessInBins(); // old still in bins
#endif
		}
		return true;
	}
}


void BestTW::writeResultFile(const char *outfile) {
	if (!run_val)
		return;

	ofstream fout;
	if (outfile==NULL) {
		fout.open("besttw.out");
		if (!fout.is_open()) {
			cerr << "Unable to open file besttw.out for writing results.\n";
			exit(1);
		}
	} else {
		fout.open(outfile);
		if (!fout.is_open()) {
			cerr << "Unable to open file " << outfile
					<< " for writing results.\n";
			exit(1);
		}
	}

	fout << stats.graphParamsLabelsString() << " exit_status tw order\n";
	fout << stats.graphParamsString() << " ";
	if (exit_status_val==TW_NOTIME)
		fout << "1 ";
	else if (exit_status_val==TW_NOMEM)
		fout << "2 ";
	else
		fout << "0 ";
	fout << solution_val.width << ' ';
	copy(solution_val.order_prefix.begin(),
			solution_val.order_prefix.end(),
			ostream_iterator<int>(fout, " "));
	fout << endl;
	fout.close();

#ifdef TRACK_EXP_BY_FVAL
	{
		// output number of nodes expanded by fval
		ofstream fout("expbyf.txt");
		stats.outputExp(fout);
		fout.close();
	}
#endif
}

void BestTW::writeStatFile(const char *statfile) {
	if (!run_val)
		return;

	ofstream fout;
	if (statfile==NULL) {
		fout.open("besttw.stt");
		if (!fout.is_open()) {
			cerr << "Unable to open file besttw.stt for writing results.\n";
			exit(1);
		}
	} else {
		fout.open(statfile);
		if (!fout.is_open()) {
			cerr << "Unable to open file " << statfile
					<< " for writing results.\n";
			exit(1);
		}
	}

#if (defined GATHER_STATS_DEBUG || defined GATHER_STATS)
	stats.outputStats(fout);
#else
	fout << "Preprocessor flags not set to keep statistics.\n";
#endif
	fout.close();
}

/*	void BestTW_driver::run (bool is_dimacs, bool is_mind,
 const char *graphfilename, int timelim, int memlim,
 const char *outfilename, const char *statfilename)
 {
 if (timelim<=0)
 timelim=BESTTW_DEFAULT_TIME;
 if (memlim<=0)
 memlim=BESTTW_DEFAULT_MEM;

 boolMatrix adjmat;
 if (is_dimacs)
 adjmat = FileOperations::readGraphDIMACS(graphfilename);
 else
 adjmat = FileOperations::readGraph(graphfilename);

 MMDPlusVersion hversion;
 if (is_mind)
 hversion = MMDPLUS_MIND;
 else
 hversion = MMDPLUS_LEASTC;

 ElimOrder optorder;
 BestTW besttw;
 TWExitStatus exit_status = besttw.solve(optorder,adjmat,hversion,
 timelim,memlim);

 if (exit_status==TW_SUCCESS)
 cout << "Treewidth = " << optorder.width << endl;

 writeResultFile(outfilename,besttw,exit_status,optorder);
 writeStatFile(statfilename,besttw);
 }
 */
/*	void BestTW_driver::writeResultFile (const char *outfilename, int oVerts,
 int oEdges, int rVerts, int rEdges, int lb, int ub,
 int last_expanded_fval, TWExitStatus status, const ElimOrder &soln)
 {
 ofstream fout;
 if (outfilename==NULL) {
 fout.open("bftw.out");
 if (!fout.is_open()) {
 cerr << "Unable to open file bftw.out for writing results.\n";
 exit(1);
 }
 } else {
 fout.open(outfilename);
 if (!fout.is_open()) {
 cerr << "Unable to open file " << outfilename << " for writing results.\n";
 exit(1);
 }
 }

 fout << "orig_verts orig_edges reduced_verts reduced_edges ub lb exit_status tw order\n";

 fout << oVerts << ' '
 << oEdges << ' '
 << rVerts << ' '
 << rEdges << ' '
 << ub << ' '
 << lb << ' ';
 if (status==BFTW_NOTIME)
 fout << "1 " << last_expanded_fval << " 0\n";
 else if (status==BFTW_NOMEM)
 fout << "2 " << last_expanded_fval << " 0\n";
 else {
 fout << 0 << ' '
 << soln.width << ' ';
 copy(soln.order_prefix.begin(),soln.order_prefix.end(),
 ostream_iterator<int>(fout," "));
 fout << endl;
 }
 fout.close();
 }*/

/*	void BestTW_driver::writeStatFile (const char *statfilename)
 {
 ofstream fout;
 if (statfilename==NULL) {
 fout.open("bftw.stt");
 if (!fout.is_open()) {
 cerr << "Unable to open file bftw.stt for writing results.\n";
 exit(1);
 }
 } else {
 fout.open(statfilename);
 if (!fout.is_open()) {
 cerr << "Unable to open file " << statfilename << " for writing results.\n";
 exit(1);
 }
 }

 #if (defined GATHER_STATS_DEBUG || defined GATHER_STATS)
 stats.outputStats(fout);
 #else
 fout << "Preprocessor flags not set to keep statistics.\n";
 #endif
 fout.close();
 }
 */
}

