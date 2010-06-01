// breadth-first-TW.cpp - definition file for divide-and-conquer breadth-first
//   search for treewidth
//
// author: Alex Dow
// created on: 11/20/6

#include "preproc_flags.h"
#include <pthread.h>
#include <iostream>
#include <fstream>
#include <map>
#include <cassert>
#include <sys/time.h>
#include <cmath>
#include <malloc.h>
#include <sys/stat.h>
#include <dirent.h>
#include "BreadthFirstTW.h"
#include "utils.h"

#define MAX2(a,b) ((a)<(b)) ? (b) : (a)
#define DIFFTIME(st,et) (((et).tv_sec-(st).tv_sec)+(double)((et).tv_usec-(st).tv_usec)/(double)1e6)
#define DIFFSECS(st,et) ((et).tv_sec-(st).tv_sec)

extern string g_graphname;

namespace Treewidth_Namespace {

TWExitStatus BreadthFirstTW::solve(ElimOrder &soln, const boolMatrix &adjmat,
		HeuristicVersion hversion, BFHTCutoffType cutoffType,
		BFHTDupeDetection dupeDetection, int timelim, int memlim, uint lb,
		uint ub) {

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
	struct timeval starttime, currtime;
	struct timezone tz;
	gettimeofday(&starttime, &tz);

	// set memory limit
	memlim*=(int)1e6; // mem limit in bytes

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
	TWExitStatus init_status = TWSearch::initSearch(adjmat, hversion, graph,
			elimprefix, elimprefix_width, postprefix_lb, vertexmap, ub_order,
			ub_mapped, stats, lb, ub);
	cout << "Vertices: " << graph.nVerts << endl;
	cout << "Lower bound: " << stats.getInitialLB() << endl;
	cout << "Upper bound: " << ub_order.width << endl;

	bmout.width(4);
	bmout << right << stats.getInitialLB() << " ";
	bmout.width(4);
	bmout << right << ub_order.width << " ";

	if (init_status==TW_SUCCESS) {
		cout << "Treewidth: " << ub_order.width << endl;
		soln=solution_val=ub_order;
		exit_status_val=TW_SUCCESS;

		bmout.width(6);
		bmout << right << "yes ";
		bmout.width(4);
		bmout << right << ub_order.width << " ";
		bmout.width(12);
		bmout << right << stats.getTotalExpanded() << " ";
		bmout.width(12);
		bmout << right << stats.getTotalGenerated() << " ";
		gettimeofday(&currtime, &tz);
		bmout.width(8);
		bmout << right << DIFFSECS(starttime, currtime);
		bmout.width(8);
		stats.setCurrMem(VmSize() >> 10);
		bmout << right << (stats.maxMem());
		bmout << endl;
		bmout.close();

		return exit_status_val;
	}
	assert(ub_mapped || ub_order.width==(int)ub);

#ifndef TW_ANY_VERTS
	// check size of graph
	if (graph.nVerts < TW_MIN_VERTS) {
		cerr << "Error: reduced graph has " << graph.nVerts
				<< " vertices, which is less than the minimum of "
				<< TW_MIN_VERTS
				<< " for the current compilation. Change preprocessing "
				<< "parameter TW_MIN_VERTS and recompile. Or recompile with "
				<< "TW_ANY_VERTS set.\n";

		bmout.width(6);
		bmout << right << "err ";
		bmout.width(4);
		bmout << right << ub_order.width << " ";
		bmout.width(12);
		bmout << right << stats.getTotalExpanded() << " ";
		bmout.width(12);
		bmout << right << stats.getTotalGenerated() << " ";
		gettimeofday(&currtime, &tz);
		bmout.width(8);
		bmout << right << DIFFSECS(starttime, currtime);
		bmout.width(8);
		stats.setCurrMem(VmSize() >> 10);
		bmout << right << (stats.maxMem());
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

		bmout.width(6);
		bmout << right << "err ";
		bmout.width(4);
		bmout << right << ub_order.width << " ";
		bmout.width(12);
		bmout << right << stats.getTotalExpanded() << " ";
		bmout.width(12);
		bmout << right << stats.getTotalGenerated() << " ";
		gettimeofday(&currtime, &tz);
		bmout.width(8);
		bmout << right << DIFFSECS(starttime, currtime);
		bmout.width(8);
		stats.setCurrMem(VmSize() >> 10);
		bmout << right << (stats.maxMem());
		bmout << endl;
		bmout.close();

		return TW_NONE;
	}
#endif

#ifdef BFHT_FASTGRAPH
	// map graph vertices to map MAX-DEGREE-FIRST heuristic ordering
	graph.mapVertices(MAPVERTS_MINDEGREE_FIRST, vertexmap);
	//graph.mapVertices(MAPVERTS_MAXDEGREE_FIRST, vertexmap);
#endif

	// initialize search, including...
	//   start and goal nodes
#ifdef DEBUG_TW
	assert(stats.currNodes()==0);
#endif
	BreadthFirstTWNode *start, *goal;
	initSearch(start, goal, elimprefix_width, postprefix_lb);

#if (defined GATHER_STATS_DEBUG || defined GATHER_STATS)
	stats.updateTotals();
#endif

	struct timeval itime, lastitime;
	gettimeofday(&itime, &tz);

	ushort cutoff;
	if (cutoffType == BFHT_ID_CUTOFF)
		cutoff = start->hValue()+1;
	else
		// cutoffType == BFHT_UB_CUTOFF
		cutoff = ub_order.width;

	while (cutoff<=ub_order.width) {
		cout << "Deepening cutoff: " << cutoff << endl;
		stats.setMinFillUB(cutoff);
		TWExitStatus exit_status = divideAndConquer(soln, starttime, timelim,
				memlim, hversion, graph, start, goal, cutoff, dupeDetection);

		lastitime = itime;
		gettimeofday(&itime, &tz);
		cout << "Iteration Time: " << DIFFSECS(lastitime, itime) << " seconds\n";
		cout << endl;

		// will soln.width>=cutoff when soln not found?
		if (exit_status==TW_NOTIME || exit_status==TW_NOMEM) {
			finalizeSearch_ddd(start, goal);
			exit_status_val=exit_status;

			bmout.width(6);
			if (exit_status_val == TW_NOTIME)
			  bmout << right << "time ";
			else
			  bmout << right << "mem ";
			bmout.width(4);
			bmout << right << ub_order.width << " ";
			bmout.width(12);
			bmout << right << stats.getTotalExpanded() << " ";
			bmout.width(12);
			bmout << right << stats.getTotalGenerated() << " ";
			gettimeofday(&currtime, &tz);
			bmout.width(8);
			bmout << right << DIFFSECS(starttime, currtime);
			bmout.width(8);
			stats.setCurrMem(VmSize() >> 10);
			bmout << right << (stats.maxMem());
			bmout << endl;
			bmout.close();

			return exit_status;
		}
		if (exit_status==TW_SUCCESS)
			break;
		//if (soln.width==-1)
		//	return exit_status;
		//if (soln.width<cutoff)
		//	break;
		start->setHValue(cutoff);
		cutoff++;
#if (defined GATHER_STATS_DEBUG || defined GATHER_STATS)
		if (cutoff<=ub_order.width)
			stats.anotherSearchForTWIteration();
#endif
	}
#if (defined GATHER_STATS_DEBUG || defined GATHER_STATS)
	stats.updateTotals();
#endif
	if (cutoff==ub_order.width+1) {
		soln=ub_order;
#if (defined GATHER_STATS_DEBUG || defined GATHER_STATS)
		stats.finishedSearchForTW();
#endif
	}
#ifdef DEBUG_TW
	if (cutoffType==BFHT_ID_CUTOFF)
		assert(soln.width==cutoff-1);
#endif
	setSolution(soln, elimprefix, vertexmap);
	finalizeSearch_ddd(start, goal);
	solution_val=soln;
	exit_status_val=TW_SUCCESS;

	stats.setCurrMem(VmSize() >> 10);
	gettimeofday(&itime, &tz);
	cout << "Total Time: " << DIFFSECS(starttime, itime) << " seconds\n";
	stats.printFinalOutput(cout);
	cout << endl;

	bmout.width(6);
	if (soln.width == ub_order.width)
	  bmout << right << "no ";
	else
	  bmout << right << "yes ";
	bmout.width(4);
	bmout << right << soln.width << " ";
	bmout.width(12);
	bmout << right << stats.getTotalExpanded() << " ";
	bmout.width(12);
	bmout << right << stats.getTotalGenerated() << " ";
	gettimeofday(&currtime, &tz);
	bmout.width(8);
	bmout << right << DIFFSECS(starttime, currtime);
	bmout.width(8);
	stats.setCurrMem(VmSize() >> 10);
	bmout << right << (stats.maxMem());
	bmout << endl;
	bmout.close();

	return exit_status_val;
}

///////////////////////////////////////////////////////////////////////////////
// BreadthFirstTW initSearch - initializes search by using adjmat to setup
// the other parameters
//
// Return:
//   BFHT_SUCCESS - implies that intial reduction found an optimal order. In
//     this case, ub_order holds the optimal order that was found.
//   BFHT_NONE - implies that the following postconditions apply.
//
// Postconditions (when BFHT_NONE returned):
//   ALMGraph graph
//	   a reduced and normalized version of graph specified by adjmat
//   OLHashTable *currOpen
//     points to a dynamically allocated, empty open list
//   OLHashTable *nextOpen
//     points to another dynamically allocated, empty open list
//   vector<Node*> midLayer
//     reserved vector or node pointers
//   Node *start
//     points to a dynamically allocated node to begin search from
//   Node *goal
//     points to a dynamically allocated node to end search at
//   vector<int> elimprefix
//     holds pre-normalization indices of vertices eliminated in initial
//     reduction
//   vector<int> vertexmap
//     maps normalized vertex indices to pre-normalization indices
//   ElimOrder ub_order
//     holds a minfill elimination order on reduced, normalized graph.
// NOTE: function finalizeSearch can be called to deallocate memory
// dynamically allocated in this function
///////////////////////////////////////////////////////////////////////////////
void BreadthFirstTW::initSearch(BreadthFirstTWNode *&start,
		BreadthFirstTWNode *&goal, int elimprefix_width, int postprefix_lb) {

	// allocate and initialize start and goal nodes
	start = new BreadthFirstTWNode((ushort)elimprefix_width,(ushort)postprefix_lb);
	stats.addNode();
	goal = new BreadthFirstTWNode(USHRT_MAX,0);
	stats.addNode();
	goal->setAllVerts();
#if (defined GATHER_STATS_DEBUG || defined GATHER_STATS)
	stats.nodeGenerated();
	stats.nodeGenerated();
#endif
}

///////////////////////////////////////////////////////////////////////////////
// BreadthFirstTW finilizeSearch - deallocates memory allocated in initSearch
///////////////////////////////////////////////////////////////////////////////
void BreadthFirstTW::finalizeSearch(BreadthFirstTWNode *start,
		BreadthFirstTWNode *goal) {
	delete start;
	delete goal;
}

///////////////////////////////////////////////////////////////////////////////
// BreadthFirstTW setSolution - takes a solution, maps the vertices with the
// given vertexmap, and appends the given prefix to the beginning
///////////////////////////////////////////////////////////////////////////////
void BreadthFirstTW::setSolution(ElimOrder &soln, const vector<int> &prefix,
		const vector<int> &vertexmap) {
	soln.order_prefix = soln.getOrder(prefix, vertexmap);
	soln.indiff_suffix.clear();
}

///////////////////////////////////////////////////////////////////////////////
// BreadthFirstTW divideAndConquer - function executes a series of breadth-
//   first searches saving only the necessary three layers. Each search
//   contributes a node to the solution path.
// Inputs:
//   starttime - time search started
//   hversion - version of heuristic to use
//   graph - graph to find tw of
//   currOpen - pointer to an open list (will be cleared and nodes deallocated)
//   nextOpen - pointer to another open list (will be cleared and nodes deallocated)
//   midLayer - vector for saving pointers to nodes in middle layer (will be cleared and nodes deallocated)
//   start - pointer to node corresponding to graph
//   goal - node corresponding to empty graph
//   ub - effective upper bound on treewidth, nodes that exceed it are pruned
// Output:
//   soln - optimal elimination order set according to return value
// Returns a BFHT_ExitStatus enumeration:
//   BFHT_SUCCESS - soln stores optimal order, and soln.width is treewidth
//   BFHT_NOTIME - exceeded time limit, soln not set
//   BFHT_NOMEM - exceeded memory limit, soln not set
//   BFHT_NONE - solution not found (probably because tw>=ub), soln not set
///////////////////////////////////////////////////////////////////////////////
TWExitStatus BreadthFirstTW::divideAndConquer(ElimOrder &soln,
		struct timeval starttime, int timelim, int memlim,
		HeuristicVersion hversion, const ALMGraph &graph,
		BreadthFirstTWNode *start, BreadthFirstTWNode *goal, int ub,
		BFHTDupeDetection dupeDetection) {

	// setup
	vector<BreadthFirstTWNode*> soln_path(graph.nVerts+1,
			(BreadthFirstTWNode*)NULL);
	soln_path[0]=start;
	soln_path[goal->nVertsElim()]=goal;
	stack<Task> tasks;
	static ALMGraph cur_graph;
	cur_graph.init(graph.nVerts);

	// perform initial search for treewidth
	cur_graph.copy(graph);
	int mid_layer = graph.nVerts/2;
	BFHTSearchForTW_ret sr;
	if (dupeDetection == BFHT_DELAYED_DD)
		sr = searchForTW_ddd(starttime, timelim, memlim, hversion, graph,
				cur_graph, *start, ub, mid_layer);
	else
		// if (dupeDetection == BFHT_IMMEDIATE_DD)
		sr = searchForTW_idd(starttime, timelim, memlim, hversion, graph,
				cur_graph, *start, ub, mid_layer);

	stats.printIterationOutput(cout);
#if (defined GATHER_STATS_DEBUG || defined GATHER_STATS)
	stats.updateLastIterationStats();
	stats.updateTotals();
#endif

	// check result
	if (sr.exit_status!=TW_SUCCESS)
		return sr.exit_status;

#if (defined GATHER_STATS_DEBUG || defined GATHER_STATS)
	stats.finishedSearchForTW();
#endif

	// process result
#ifdef DEBUG_BREADTHFTW
	assert(soln_path[sr.mid_node->nVertsElim()]==NULL);
#endif
	soln_path[sr.mid_node->nVertsElim()]=sr.mid_node; // save node ptr in soln path vector
	cout << "Treewidth: " << sr.tw << endl;
	goal->setGValue(sr.tw);
#if (defined GATHER_STATS_DEBUG || defined GATHER_STATS)
	stats.foundTW();
#endif
#ifdef DEBUG_BREADTHFTW
	assert(sr.mid_node->hValue()>0 || graph.nVerts-sr.mid_node->nVertsElim()-1
			<=sr.mid_node->gValue());
#endif

	// setup first reconstruction tasks
#ifdef BFHT_UB_AT_EVERY_NODE
	if (!sr.optsuffix_set)
		tasks.push(Task(sr.mid_node, goal));
#else
	tasks.push(Task(sr.mid_node, goal));
#endif
	if (sr.mid_node->nVertsElim()-start->nVertsElim()>1)
		tasks.push(Task(start, sr.mid_node));

	// start solution reconstruction searches
	while (!tasks.empty()) {
		Task task = tasks.top();
		tasks.pop();

		// try to reduce start node
		cur_graph.copy(graph);
		cur_graph.elimVertices(*(task.first->statePtr()));
		reduce_all_ret raret = cur_graph.reduceGraphAll(task.first->fValue(),
				hversion, ub, task.second->statePtr());
		// if any vertices were eliminated in reduction
		if (!raret.elimVerts.empty()) {
			// if start node reduces to goal node
			if (cur_graph.nVerts==graph.nVerts-task.second->nVertsElim())
				continue;
#ifdef DEBUG_BREADTHFTW
			assert(cur_graph.nVerts>graph.nVerts-task.second->nVertsElim());
#endif
			// generate the new reduced node
			BreadthFirstTWNode *red_node = new BreadthFirstTWNode(*task.first);
			stats.addNode();
#if (defined GATHER_STATS_DEBUG || defined GATHER_STATS)
	#ifdef TRACK_EXP_BY_FVAL
			stats.nodeExpanded(task.first->fValue()); // count reducing a node as expanding by genreating its "single" child
	#else
			stats.nodeExpanded();
	#endif
			stats.nodeReduced();
			stats.nodeGenerated();
#endif
			if (raret.maxdeg > red_node->gValue())
				red_node->setGValue(raret.maxdeg);
			red_node->setHValue(raret.hval);
			red_node->setVerts(raret.elimVerts);
#ifdef DEBUG_BREADTHFTW
			assert(soln_path[red_node->nVertsElim()]==NULL);
#endif
			task.first=red_node;
			soln_path[red_node->nVertsElim()]=red_node;
#ifdef DEBUG_BREADTHFTW
			assert(task.first->nVertsElim()+1<=task.second->nVertsElim());
#endif
			// if reduction is one away from goal, go to next task
			if (task.first->nVertsElim()+1 == task.second->nVertsElim())
				continue;
		}

		// search from start to goal
#if (defined GATHER_STATS_DEBUG || defined GATHER_STATS)
		stats.currLevel(task.first->nVertsElim());
#endif
		mid_layer = (task.first->nVertsElim() + task.second->nVertsElim())/2;
		BFHTSearchForSolnPath_ret sfspr;
		if (dupeDetection == BFHT_DELAYED_DD)
			sfspr = searchForSolnPath_ddd(starttime, timelim, memlim, hversion,
					graph, cur_graph, *task.first, *task.second, mid_layer);
		else
			// if (dupeDetection == BFHT_IMMEDIATE_DD)
			sfspr = searchForSolnPath_idd(starttime, timelim, memlim, hversion,
					graph, cur_graph, *task.first, *task.second, mid_layer);

		// check result
		if (sfspr.exit_status!=TW_SUCCESS) {
			deallocSolnPath(soln_path, start, goal);
			return sfspr.exit_status;
		}
#ifdef DEBUG_BREADTHFTW
		assert(sfspr.mid_node->nVertsElim()==mid_layer);
		assert(sfspr.mid_node->hValue()>0);
		assert(soln_path[sfspr.mid_node->nVertsElim()]==NULL);
#endif

		soln_path[sfspr.mid_node->nVertsElim()]=sfspr.mid_node; // save node ptr in soln path vector

		// add new tasks to stack
		if (sfspr.mid_node->nVertsElim()+1 < task.second->nVertsElim())
			tasks.push(Task(sfspr.mid_node, task.second));
		if (task.first->nVertsElim()+1 < sfspr.mid_node->nVertsElim())
			tasks.push(Task(task.first, sfspr.mid_node));

#if (defined GATHER_STATS_DEBUG || defined GATHER_STATS)
		if (!tasks.empty())
			stats.anotherSearchForSolnPath();
#endif
	}

	// get opt order from solution path
	soln.width = sr.tw;
#ifdef BFHT_UB_AT_EVERY_NODE
	getOrder(hversion, graph, soln_path, soln.order_prefix, sr.optsuffix_set, sr.optsuffix);
#else
	getOrder(hversion, graph, soln_path, soln.order_prefix);
#endif
	deallocSolnPath(soln_path, start, goal);

	return TW_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// BreadthFirstTW deallocSolnPath - helper function for divideAndConquer to
//   deallocate nodes (except for start and goal) in soln_path
///////////////////////////////////////////////////////////////////////////////
void BreadthFirstTW::deallocSolnPath(vector<BreadthFirstTWNode*> &soln_path,
		const BreadthFirstTWNode *start, const BreadthFirstTWNode *goal) {
	for (uint i=0; i<soln_path.size(); i++)
		if (soln_path[i]!=NULL && soln_path[i]!=start && soln_path[i]!=goal) {
			delete soln_path[i];
			stats.subNode();
			soln_path[i]=NULL;
		}
}

///////////////////////////////////////////////////////////////////////////////
// BreadthFirstTW search - performs a search from start to goal returning
//   to node on the solution path at the given mid_layer_depth as well as the
//   gvalue of the goal node when it is chosen for expansion.
///////////////////////////////////////////////////////////////////////////////
BFHTSearchForTW_ret BreadthFirstTW::searchForTW_idd(struct timeval starttime,
		int timelim, int memlim, HeuristicVersion hversion,
		const ALMGraph &orig_graph, ALMGraph &cur_graph,
		const BreadthFirstTWNode &start, int ub, int mid_layer_depth) {

  // TODO: There is a problem when the open list hash table rehashes
  // when we are already close to the memory limit. This may result in
  // an std::bad_alloc. The way to solve this would be to increase the
  // size of the hash table once we are at least half way to the
  // memory limit such that we know it will not have to be increased
  // again. This should prevent large memory requests when there is
  // little memory remaining.

#ifdef BFHT_UB_AT_EVERY_NODE
	cerr << "BFHT set to compute upper bound before every expansion.\n";
#endif
#if (defined GATHER_STATS_DEBUG || defined GATHER_STATS)
	stats.numLayersInSearchForTW(cur_graph.nVerts+1);
#endif
	// start timer for intermittent output
	struct timeval lasttime;
	struct timezone tz;
	gettimeofday(&lasttime, &tz);
	lasttime.tv_sec -= 60;

	// setup open lists
	BreadthFirstTWOpenList ol1, ol2;
	BreadthFirstTWOpenList *currOpen = &ol1;
	BreadthFirstTWOpenList *nextOpen = &ol2;
	vector<BreadthFirstTWNode*> midLayer;
#ifdef DEBUG_BREADTHFTW
	assert(currOpen->empty());
	assert(nextOpen->empty());
	assert(midLayer.empty());
#endif

	// more setup
	vector<BreadthFirstTWNode> dummyChildren;
	dummyChildren.reserve(cur_graph.nVerts);
	BreadthFirstTWNode *newub_node = NULL;
	vector<int> adjacentVerts;
#ifdef BFHT_UB_AT_EVERY_NODE
	bool newub_suffix_valid = false;
	ElimOrder newub_suffix;
#endif

#ifdef BFHT_FASTGRAPH
	// setup for intermediate graph derivation
	TWState cur_graph_state = *(start.statePtr());
	vector<vector<pair<int,int> > > added_edges(orig_graph.nVerts+1);
	vector<vector<int> > removed_edges(orig_graph.nVerts+1);
	vector<vector<int> > added_pSAAS(orig_graph.nVerts+1);
	vector<bool> removed_pSAAS(orig_graph.nVerts+1, false);
	//uint suffix_len_sum = 0;
#endif

	// expand start node into currOpen - note: start node should be fully reduced
#ifdef DEBUG_BREADTHFTW
	if (cur_graph.potentialSimpAndAlmostSimp[0].vid>0) {
		reduce_one_ret roret_debug = cur_graph.reduceGraphOne(start.fValue());
		assert(roret_debug.elimVert==-1);
	}
#endif
	start.expand(cur_graph, ub, adjacentVerts, dummyChildren, mid_layer_depth);
	allocAndInsertInOpen(dummyChildren, mid_layer_depth, currOpen);
#if (defined GATHER_STATS_DEBUG || defined GATHER_STATS)
	#ifdef TRACK_EXP_BY_FVAL
	stats.nodeExpanded(start.fValue());
	#else
	stats.nodeExpanded();
	#endif
#endif

	// start
	BreadthFirstTWNode *n=NULL;
	while (!currOpen->empty()) { // outer loop - once per layer

	  uint mem = (VmSize() << 10);
	  stats.setCurrMem(mem >> 20);

#ifdef BFHT_FASTGRAPH
	  // sort the open list
	  currOpen->sort();
#endif

		int curr_depth=currOpen->top()->nVertsElim();
		int min_gval=INT_MAX; // min gval for next layer
		cout << "expanding level " << curr_depth << "/" << orig_graph.nVerts
				<< ", with " << currOpen->size() << " nodes.\n";
#if (defined GATHER_STATS_DEBUG || defined GATHER_STATS)
		stats.currLevel(curr_depth);
		stats.setSearchForTWNodes(curr_depth, currOpen->size());
		stats.setSearchForTWMidNodes(curr_depth, midLayer.size());
#endif
		while (!currOpen->empty()) { // inner loop - once per node in currOpen
#ifdef DEBUG_BREADTHFTW
			if (newub_node==NULL)
				assert(stats.currNodes()==currOpen->size()+nextOpen->size()
						+midLayer.size()+2);
			else
				assert(stats.currNodes()==currOpen->size()+nextOpen->size()
										+midLayer.size()+3);
#endif
			struct timeval currtime;
			gettimeofday(&currtime, &tz);

			// check time limit
			if (DIFFSECS(starttime,currtime)>timelim) {
				cerr << "BreadthFirstTW search exceeded time limit of "
						<< timelim << " seconds.\n";
				return BFHTSearchForTW_ret(TW_NOTIME);
			}

			// periodic output
			if (DIFFSECS(lasttime,currtime)>30) {

				// check memory allocation limit
				uint mem = (VmSize() << 10);
				stats.setCurrMem(mem >> 20);
				//	stats.curr_mem = minfo.arena;
				//	if (stats.curr_mem > memlim) {
				if ((int)mem > memlim) {
					cerr
							<< "BreadthFirstTW search exceeded max memory usage of "
							<< (int)(memlim/1e6) << "MB.\n";
					return BFHTSearchForTW_ret(TW_NOMEM);
				}

				cerr << "  nodes: " << stats.currNodes() 
				     << " \tcurrOpen: " << currOpen->size() 
				     << " \tnextOpen: " << nextOpen->size()
				     << " \tmidLayer: " << midLayer.size() 
				     << " \tmem: " << (mem >> 20) << endl;				     
				lasttime=currtime;
			}

			// get a node from currOpen
			n = currOpen->toppop();

			// test for goal node
			if (n->nVertsElim()==orig_graph.nVerts) {
#ifdef DEBUG_BREADTHFTW
				assert(currOpen->empty());
#endif
				break; // done
			}

			// get node's graph
#ifdef BFHT_FASTGRAPH
			// find length of differing suffix
			uint suffix_len = cur_graph_state.diffSuffixLength(
					*(n->statePtr()));
			// uneliminate vertices in cur_graph_state's suffix
			cur_graph.reverseElimSuffix(cur_graph_state, suffix_len,
					removed_edges, added_edges, removed_pSAAS, added_pSAAS);
			// eliminate vertices in n->state's suffix
			cur_graph.elimSuffix(*(n->statePtr()), suffix_len,
					removed_edges, added_edges, removed_pSAAS, added_pSAAS);
			// update cur_graph_state
			cur_graph_state = *(n->statePtr());
#else
			int vid = n->vid();
			adjacentVerts.clear();
			cur_graph.copy(orig_graph);
			cur_graph.elimVertices(*(n->statePtr()), &vid, &adjacentVerts); // need to populate pSAAS bc don't know if node was reduced
#endif

			// compute node's hvalue, prune if possible
#ifdef MONOTONIC_H
			n->hValue = cur_graph.heuristic(hversion,ub);
#else
			int tmp_h = cur_graph.heuristic(hversion, ub);
			if (tmp_h>n->hValue())
				n->setHValue(tmp_h);
#endif
			if (n->fValue() >= ub) {
				delete n;
				stats.subNode();
				n=NULL;
				continue;
			}

#ifdef BFHT_UB_AT_EVERY_NODE
			// compute upper bound from this node
			ElimOrder new_ub_order = cur_graph.ubMinFill();
			if (new_ub_order.width < ub)
			{
				assert(n->gValue() < ub);
				ub = MAX2(n->gValue(), new_ub_order.width);
				if (newub_node != NULL) {
					delete newub_node;
					stats.subNode();
				}
				newub_node = new BreadthFirstTWNode(*n,
						mid_layer_depth);
				newub_node->setGValue(ub);
				newub_node->setHValue(ub);
				stats.addNode();
				newub_suffix_valid = true;
				newub_suffix = new_ub_order;
				cout << "!!!!!!!!!!!!!!!!!!!!!\n";
				cout << "New upper bound found: " << ub << endl;
			}
#endif

#ifdef BFHT_PRUNE_GRDC
			// try to eliminate a vertex with reduction rules
#ifdef BFHT_PRUNE2
			vector<bool> nopruneverts(cur_graph.vertices.size(),false);
			for (int i=0; i<adjacentVerts.size(); i++)
				if (adjacentVerts[i] < n->vid())
					nopruneverts[adjacentVerts[i]]=true;
			reduce_one_ret roret = cur_graph.findReduceGraphOne(n->fValue(),
					ub, REDUCE_LEAST_FIRST, n->vid(), &nopruneverts);
#else
			reduce_one_ret roret = cur_graph.findReduceGraphOne(n->fValue(),
					ub);
#endif
			// if a vertex can be eliminated through reduction rules,
			// insert single child in nextOpen
			if (roret.elimVert!=-1)
			{
#if (defined GATHER_STATS_DEBUG || defined GATHER_STATS)
				stats.nodeReduced();
	#ifdef TRACK_EXP_BY_FVAL
				stats.nodeExpanded(n->fValue()); // count reducing a node as expanding by generating its "single" child
	#else
				stats.nodeExpanded();
	#endif
#endif
				bool prune = roret.deg >= ub;

#ifdef BFHT_PRUNE2
				if (!prune)
					prune = (roret.elimVert < n->vid() && !nopruneverts[roret.elimVert]);
#endif

				if (!prune)
				{
					BreadthFirstTWNode dummyChild(n, roret.elimVert, roret.deg,
							mid_layer_depth);
					BreadthFirstTWNode *new_node = checkOpenAllocAndInsert(
							dummyChild, mid_layer_depth, nextOpen);
					if (new_node->gValue() < min_gval)
						min_gval=new_node->gValue();
					// check for improved upper bound
					if (new_node->gValue() >= cur_graph.nVerts-2)
					{
						ub = new_node->gValue();
						if (newub_node != NULL)
						{
							delete newub_node;
							stats.subNode();
						}
						newub_node = new BreadthFirstTWNode(*new_node,
								mid_layer_depth);
#ifdef BFHT_UB_AT_EVERY_NODE
						newub_suffix_valid = false;
#endif
						stats.addNode();
						cout << "New upper bound found: " << ub << endl;
					}
				}
			}
			// if reduction rules don't apply, expand node and insert children in nextOpen
			else
#endif
			{
#ifdef BFHT_PRUNE3
				n->expand(cur_graph, ub, adjacentVerts, dummyChildren,
						mid_layer_depth);
#else
				vector<int> tmp;
				n->expand(cur_graph, ub, tmp, dummyChildren, mid_layer_depth);
#endif
#if (defined GATHER_STATS_DEBUG || defined GATHER_STATS)
	#ifdef TRACK_EXP_BY_FVAL
				stats.nodeExpanded(n->fValue());
	#else
				stats.nodeExpanded();
	#endif
#endif
				for (size_t i=0; i<dummyChildren.size(); i++) {
					BreadthFirstTWNode *new_node = checkOpenAllocAndInsert(
							dummyChildren[i], mid_layer_depth, nextOpen);
					if (new_node->gValue() < min_gval)
						min_gval=new_node->gValue();
					// check for improved upper bound
					if (new_node->gValue() >= cur_graph.nVerts-2) {
						ub = new_node->gValue();
						if (newub_node != NULL) {
							delete newub_node;
							stats.subNode();
						}
						newub_node = new BreadthFirstTWNode(*new_node,
								mid_layer_depth);
#ifdef BFHT_UB_AT_EVERY_NODE
						newub_suffix_valid = false;
#endif
						stats.addNode();
						cout << "New upper bound found: " << ub << endl;
					}
				}
			}
			// if at middle layer, save expanded node
			if (curr_depth==mid_layer_depth) {
				midLayer.push_back(n);
				// otherwise delete expanded node
			} else {
				delete n;
				stats.subNode();
			}
			n=NULL;
		}
		// check for completion
		if (n!=NULL)
			break;
		// check if all gvals in next layer exceed new upper bound
		if (min_gval>=ub)
			break;
		// switch open lists
		BreadthFirstTWOpenList *olht_tmp=currOpen;
		currOpen=nextOpen;
		nextOpen=olht_tmp;
	}

	// prepare return structure
#ifdef DEBUG_BREADTHFTW
	if (n==NULL && newub_node==NULL)
		assert(currOpen->empty() && nextOpen->empty());
	else if (n!=NULL && n->nVertsElim()==orig_graph.nVerts)
		assert(!n->isAncest(NULL));
	assert(n==NULL || n->nVertsElim()==orig_graph.nVerts);
#endif
	BFHTSearchForTW_ret ret;
	if (n==NULL && newub_node==NULL) // no path found better than original ub
		ret = BFHTSearchForTW_ret(TW_NONE);
	else if (n==NULL) { // new ub gives opt path to goal ndoe
		if (newub_node->getAncest()==NULL ||
				newub_node->getAncest()==newub_node) // newub_node is the "middle node", actually return a copy of it so that it's not in nextOpen
		{
#ifdef BFHT_UB_AT_EVERY_NODE
			ret = BFHTSearchForTW_ret(TW_SUCCESS, newub_node,
					newub_node->gValue(), newub_suffix_valid, newub_suffix);
#else
			ret = BFHTSearchForTW_ret(TW_SUCCESS, newub_node,
					newub_node->gValue());
#endif
		}
		else
		{
			ret = BFHTSearchForTW_ret(TW_SUCCESS,
					new BreadthFirstTWNode(*newub_node->getAncest()),
					newub_node->gValue());
			stats.addNode();
			delete newub_node;
			stats.subNode();
		}
	}
	else { // found path to goal node
		ret = BFHTSearchForTW_ret(TW_SUCCESS, new BreadthFirstTWNode(*n->getAncest()), n->gValue());
		stats.addNode();
		delete n;
		stats.subNode();
		if (newub_node!=NULL) {
			delete newub_node;
			stats.subNode();
		}
	}

	// delete remaining nodes
	while (midLayer.size()>0) {
		delete midLayer.back();
		stats.subNode();
		midLayer.pop_back();
	}
	while (currOpen->size()>0) {
		BreadthFirstTWNode *n = currOpen->toppop();
		delete n;
		stats.subNode();
	}
	while (nextOpen->size()>0) {
		BreadthFirstTWNode *n = nextOpen->toppop();
		delete n;
		stats.subNode();
	}
	assert(currOpen->empty());
	assert(nextOpen->empty());
	assert(midLayer.empty());

	return ret;
}

///////////////////////////////////////////////////////////////////////////////
// BreadthFirstTW searchForSolnPath - searches from a start node s to a goal
//   node g, saving the node at the middle layer between them. Function assumes
//   that cur_graph is the graph corresponding to Node s.
// NOTE: function assumes that both s and g have a valid (and hopefully good)
//   hValue
// NOTE: this function assumes heuristic is monotonic (for pruning)
///////////////////////////////////////////////////////////////////////////////
BFHTSearchForSolnPath_ret BreadthFirstTW::searchForSolnPath_idd(
		struct timeval starttime, int timelim, int memlim,
		HeuristicVersion hversion, const ALMGraph &orig_graph,
		ALMGraph &cur_graph, const BreadthFirstTWNode &start,
		const BreadthFirstTWNode &goal, int mid_layer_depth) {

	// start timer for periodic output
	struct timeval lasttime;
	struct timezone tz;
	gettimeofday(&lasttime, &tz);
	lasttime.tv_sec-=60;

	// setup open lists
	BreadthFirstTWOpenList ol1, ol2;
	BreadthFirstTWOpenList *currOpen = &ol1;
	BreadthFirstTWOpenList *nextOpen = &ol2;
	vector<BreadthFirstTWNode*> midLayer;

	// setup
	vector<BreadthFirstTWNode> dummyChildren;
	dummyChildren.reserve(goal.nVertsElim()-start.nVertsElim());
#ifdef DEBUG_BREADTHFTW
	assert(currOpen->empty());
	assert(nextOpen->empty());
	assert(midLayer.empty());
	assert(cur_graph.nVerts==orig_graph.nVerts-start.nVertsElim());
	assert(mid_layer_depth>start.nVertsElim());
	assert(mid_layer_depth<goal.nVertsElim());
#endif

	// expand start node into currOpen - note: should be fully reduced
#ifdef DEBUG_BREADTHFTW
	if (cur_graph.potentialSimpAndAlmostSimp[0].vid>0) {
		reduce_one_ret roret_debug = cur_graph.reduceGraphOne(start.fValue());
		assert(roret_debug.elimVert==-1);
	}
#endif
	start.expandSubset(cur_graph, dummyChildren, mid_layer_depth, goal);
#if (defined GATHER_STATS_DEBUG || defined GATHER_STATS)
	#ifdef TRACK_EXP_BY_FVAL
			stats.nodeExpanded(start.fValue());
	#else
			stats.nodeExpanded();
	#endif
#endif
#ifdef DEBUG_BREADTHFTW
	assert(dummyChildren.size()<=goal.nVertsElim()-start.nVertsElim());
#endif
	allocAndInsertInOpen(dummyChildren, mid_layer_depth, currOpen);
#ifdef DEBUG_BREADTHFTW
	assert(!dummyChildren.empty());
	assert(!currOpen->empty());
#endif

	// start
	BreadthFirstTWNode *n;
	while (!currOpen->empty()) { // outer loop - once per layer
		int curr_depth=currOpen->top()->nVertsElim();
		cout << "expanding level " << curr_depth << "/" << orig_graph.nVerts
				<< ", with " << currOpen->size() << " nodes.\n";
#if (defined GATHER_STATS_DEBUG || defined GATHER_STATS)
		stats.setSearchForSolnPathNodes(currOpen->size());
		stats.setSearchForSolnPathMidNodes(midLayer.size());
		stats.setSearchForSolnPathLevels(curr_depth);
#endif
		while (!currOpen->empty()) { // inner loop - once per node in currOpen
			struct timeval currtime;
			gettimeofday(&currtime, &tz);

			// check time limit
			if (DIFFSECS(starttime,currtime)>timelim) {
				cerr << "BreadthFirstTW search exceeded time limit of "
						<< timelim << " seconds.\n";
				return BFHTSearchForSolnPath_ret(TW_NOTIME);
			}

			// periodic output
			if (DIFFSECS(lasttime,currtime)>30) {
				cout << "  nodes: " << stats.currNodes() << " \tcurrOpen: "
						<< currOpen->size() << " \tnextOpen: "
						<< nextOpen->size() << " \tmidLayer: "
						<< midLayer.size() << endl;
				lasttime=currtime;

				// check memory allocation limit
				uint mem = (VmSize() << 10);
				//	stats.curr_mem = minfo.arena;
				//	if (stats.curr_mem > memlim) {
				if ((int)mem > memlim) {
					cerr
							<< "BreadthFirstTW search exceeded max memory usage of "
							<< (int)(memlim/1e6) << "MB.\n";
					return BFHTSearchForSolnPath_ret(TW_NOMEM);
				}
			}

			// get a node from currOpen
			n = currOpen->toppop();

			// test for goal node <-- can probably move this to generation, since we know exact goal node
			if (n->nVertsElim()==goal.nVertsElim()) {
#ifdef DEBUG_BREADTHFTW
				assert(*n==goal);
				assert(n->gValue()<=goal.gValue());
				assert(currOpen->empty());
#endif
				break; // done
			}

			// get node's graph
			cur_graph.copy(orig_graph);
			cur_graph.elimVertices(*(n->statePtr()));

			// compute node's hvalue, prune if possible
#ifdef MONOTONIC_H
			n->hValue = cur_graph.heuristic(hversion,goal.fValue()+1);
#else
			int tmp_h = cur_graph.heuristic(hversion, goal.fValue()+1);
			if (tmp_h>n->hValue())
				n->setHValue(tmp_h);
#endif
			if (n->hValue() > goal.fValue()) { // this assumes cost function is monotonic... well, if not it may prune some nodes on an opt path to goal, though it will still leave the parent node that was actually used to generate goal
				delete n;
				stats.subNode();
				n=NULL;
				continue;
			}

			// try to eliminate a vertex with reduction rules
			reduce_one_ret roret = cur_graph.reduceGraphOne(n->fValue(),
					goal.gValue()+1, goal.statePtr());

			// if vertex eliminated through reduction rules, generate single child
			if (roret.elimVert!=-1) {
#if (defined GATHER_STATS_DEBUG || defined GATHER_STATS)
				stats.nodeReduced();
	#ifdef TRACK_EXP_BY_FVAL
				stats.nodeExpanded(start.fValue()); // count reducing a node as expanding by generating its "single" child
	#else
				stats.nodeExpanded();
	#endif
#endif
				if (roret.deg <= goal.gValue()) { // otherwise prune child
					BreadthFirstTWNode dummyChild(n, roret.elimVert, roret.deg,
							mid_layer_depth);
					checkOpenAllocAndInsert(dummyChild, mid_layer_depth,
							nextOpen);
				}

				// if reduction rules don't apply, expand node and insert children in nextOpen
			} else {
				n->expandSubset(cur_graph, dummyChildren, mid_layer_depth, goal);
#if (defined GATHER_STATS_DEBUG || defined GATHER_STATS)
	#ifdef TRACK_EXP_BY_FVAL
				stats.nodeExpanded(start.fValue());
	#else
				stats.nodeExpanded();
	#endif
#endif
#ifdef DEBUG_BREADTHFTW
				assert(dummyChildren.size()<=goal.nVertsElim()
						-start.nVertsElim());
#endif
				for (size_t i=0; i<dummyChildren.size(); i++)
					checkOpenAllocAndInsert(dummyChildren[i], mid_layer_depth,
							nextOpen);
			}

			// if at middle layer, save expanded node
			if (curr_depth==mid_layer_depth) {
				midLayer.push_back(n);
				// otherwise delete expanded node
			} else {
				delete n;
				stats.subNode();
			}
			n=NULL;
		}

		// check for completion
		if (n!=NULL)
			break;

		// switch open lists
		BreadthFirstTWOpenList *olht_tmp=currOpen;
		currOpen=nextOpen;
		nextOpen=olht_tmp;
	}

	// prepare return structure
#ifdef DEBUG_BREADTHFTW
	assert(n!=NULL);
	assert(!n->isAncest(NULL));
	assert(n->gValue()<=goal.gValue());
	assert(*n==goal);
#endif
	BFHTSearchForSolnPath_ret ret(TW_SUCCESS, n->getAncest());
	delete n;
	stats.subNode();
	return ret;
}

///////////////////////////////////////////////////////////////////////////////
// BreadthFirstTW allocAndInsertInOpen - given a vector of dummy nodes,
//   function dynamically allocates the nodes and inserts them in the given
//   open list. The dummy nodes must contain the correct states and gvalues for
//   all new nodes. If past the mid_layer, (i.e. curr_layer>mid_layer) then
//   dummy nodes must also have the correct ancest ptr for the new nodes.
///////////////////////////////////////////////////////////////////////////////
void BreadthFirstTW::allocAndInsertInOpen(
		const vector<BreadthFirstTWNode> &dummys, int mid_layer,
		BreadthFirstTWOpenList *openlist) {
#if (defined GATHER_STATS_DEBUG || defined GATHER_STATS)
	stats.nodeGenerated(dummys.size());
#endif
	for (size_t i=0; i<dummys.size(); i++) {
		BreadthFirstTWNode *n = new BreadthFirstTWNode(dummys[i],mid_layer);
		stats.addNode();
		openlist->insert(n);
	}
}

///////////////////////////////////////////////////////////////////////////////
// BreadthFirstTW checkOpenAllocAndInsert - given a dummy node with the state
//   and gvalue of a new node, check whether there is a duplicate in the open
//   list. If there is not, then allocate the new node, set values, and insert
//   in open list. If there is a duplicate and it is worse than the new node,
//   then change the values of the old node to match the new node. If there is
//   a duplicate and it is better than the new node, then don't do anything.
// Inputs:
//   dummy - a node from which certain values should be copied for the new
//     node. These include:
//      - state - equal to new node's state
//		- gValue - equal to new node's gValue
//		- ancest - IF past mid_layer (i.e. curr_layer>mid_layer) then this is
//          equal to new node's ancest. ELSE it is not used.
// Output:
//   Returns a ptr to the node. This may be the newly allocated node, the old
//   duplicate with the dummy's values, or the unchanged old duplicate.
//   Basically, the returned node is the node in the open list with the state
//   of the passed dummy node.
///////////////////////////////////////////////////////////////////////////////
BreadthFirstTWNode* BreadthFirstTW::checkOpenAllocAndInsert(
		const BreadthFirstTWNode &dummy, int mid_layer,
		BreadthFirstTWOpenList *openlist) {
	BreadthFirstTWNode *old = openlist->find(&dummy);
	if (old==NULL) { // no dupe in open list
#if (defined GATHER_STATS_DEBUG || defined GATHER_STATS)
		stats.nodeGenerated();
#endif
		BreadthFirstTWNode *n = new BreadthFirstTWNode(dummy,mid_layer);
		stats.addNode();
		openlist->insert(n);
		return n;
	} else { // duplicate in open list
		if (old->gValue()>dummy.gValue()) { // new node better, replace old
#if (defined GATHER_STATS_DEBUG || defined GATHER_STATS)
			stats.nodeGenerated();
#endif
			old->setGValue(dummy.gValue());
			if (dummy.nVertsElim()>mid_layer)
				old->setAncest(dummy.getAncest());
#ifdef DEBUG_BREADTHFTW
			assert(*old==dummy);
			if (dummy.nVertsElim()==mid_layer)
				assert(old->isAncest(old));
#endif
		}
#ifndef MONOTONIC_H
		if (dummy.hValue() > old->hValue())
			old->setHValue(dummy.hValue());
#endif
		return old;
	}
}

///////////////////////////////////////////////////////////////////////////////
// BreadthFirstTW getOrder - given a vector of node pointers corresponding
//   to a valid solution path, function fill destination vector with a vertex
//   elimination order represented by the solution path.
///////////////////////////////////////////////////////////////////////////////
#ifdef BFHT_UB_AT_EVERY_NODE
void BreadthFirstTW::getOrder(HeuristicVersion hversion,
		const ALMGraph &orig_graph,
		const vector<BreadthFirstTWNode*> &node_path, vector<int> &dest,
		bool optsuffix_valid, ElimOrder &optsuffix)
#else
void BreadthFirstTW::getOrder(HeuristicVersion hversion,
		const ALMGraph &orig_graph,
		const vector<BreadthFirstTWNode*> &node_path, vector<int> &dest)
#endif
{
	dest.clear();
	dest.reserve(node_path.size()-1);
	ALMGraph cur_graph;
	cur_graph.init(orig_graph.nVerts);
	for (int i=0; i<(int)node_path.size()-1; i++) {
#ifdef DEBUG_BREADTHFTW
		assert(node_path[i]!=NULL);
#endif
#ifdef BFHT_UB_AT_EVERY_NODE
		if (optsuffix_valid && i+1==node_path.size()-optsuffix.order_prefix.size())
		{
			dest.insert(dest.end(), optsuffix.order_prefix.begin(), optsuffix.order_prefix.end());
			break;
		}
#endif
		if (node_path[i+1]!=NULL) {
			vector<int> verts;
			node_path[i]->statePtr()->getDiff(*(node_path[i+1]->statePtr()), verts);
#ifdef DEBUG_BREADTHFTW
			assert(verts.size()==1);
#endif
			dest.push_back(verts[0]);
		} else {
			cur_graph.copy(orig_graph);
			cur_graph.elimVertices(*(node_path[i]->statePtr()));
#ifdef DEBUG_BREADTFTW
			assert(cur_graph.hMMDplus(heuristicVersion)==node_path[i]->hValue);
#endif
			int j;
			for (j=i+1; j<(int)node_path.size() && node_path[j]==NULL; j++)
				; // search for next node in path
#ifdef DEBUG_BREADTHFTW
			assert(j<node_path.size());
#endif
			reduce_all_ret raret = cur_graph.reduceGraphAll(
					node_path[i]->fValue(), hversion, INT_MAX,
					node_path[j]->statePtr());
#ifdef DEBUG_BREADTHFTW
			assert(i+raret.elimVerts.size()==j);
			TWState s = *node_path[i]->statePtr();
			s.setVerts(raret.elimVerts);
			assert(node_path[j]->isState(s));
			assert(raret.maxdeg<=node_path[j]->gValue());
			//assert(raret.hval==node_path[j]->hValue()); // not sure why this should be so, possible that mid node from searchForTW didn't have the hval computed
#endif
			dest.insert(dest.end(), raret.elimVerts.begin(),
					raret.elimVerts.end());
			i=j-1;
		}
	}
}

void BreadthFirstTW::writeResultFile(const char *outfile) {
	if (!run_val)
		return;

	ofstream fout;
	if (outfile==NULL) {
		fout.open("bfht.out");
		if (!fout.is_open()) {
			cerr << "Unable to open file bfht.out for writing results.\n";
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

	fout << stats.graphParamsLabelsString() << " exit_status";
	if (exit_status_val==TW_NOTIME || exit_status_val==TW_NOMEM) {
		fout << " tw_found";
		if (stats.isTWFound())
			fout << " tw";
		fout << " last_level\n";
	} else
		fout << " tw order\n";
	fout << stats.graphParamsString() << " ";
	if (exit_status_val==TW_NOTIME)
		fout << "1 ";
	else if (exit_status_val==TW_NOMEM)
		fout << "2 ";
	else
		fout << "0 ";
	if (exit_status_val==TW_NOTIME || exit_status_val==TW_NOMEM) {
		fout << stats.isTWFound() << ' ';
		if (stats.isTWFound())
			fout << solution_val.width << ' ' << stats.lastLevel() << endl;
		else
			fout << stats.lastLevel() << endl;
	} else {
		fout << solution_val.width << ' ';
		copy(solution_val.order_prefix.begin(),
				solution_val.order_prefix.end(), ostream_iterator<int>(fout,
						" "));
		fout << endl;
	}
	fout.close();
}

void BreadthFirstTW::writeStatFile(const char *statfile) {
	if (!run_val)
		return;

	ofstream fout;
	if (statfile==NULL) {
		fout.open("bfht.stt");
		if (!fout.is_open()) {
			cerr << "Unable to open file bfht.stt for writing results.\n";
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
	fout << "Error: preprocessing flags not set to keep statistics.\n";
#endif
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

}

