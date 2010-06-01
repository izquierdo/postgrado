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

namespace Treewidth_Namespace {

TWExitStatus BreadthFirstTW::solve_w_HDDD(ElimOrder &soln,
		const boolMatrix &adjmat, HeuristicVersion hversion,
		BFHTCutoffType cutoffType, int timelim) {
	run_val=true;
	stats.reset();

	// start timer
	struct timeval starttime;
	struct timezone tz;
	gettimeofday(&starttime, &tz);

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
			ub_mapped, stats);
	cout << "Vertices: " << graph.nVerts << endl;
	cout << "Lower bound: " << stats.getInitialLB() << endl;
	cout << "Upper bound: " << ub_order.width << endl;
	if (init_status==TW_SUCCESS) {
		cout << "Treewidth: " << ub_order.width << endl;
		soln=solution_val=ub_order;
		exit_status_val=TW_SUCCESS;
		return exit_status_val;
	}
	assert(ub_mapped);

	// check size of graph
	if (graph.nVerts < DDD_HARD_MIN_VERTS) {
		cerr << "Error: reduced graph has " << graph.nVerts
				<< " vertices, which is less than the minimum of "
				<< DDD_HARD_MIN_VERTS
				<< " for the current implementation of hash-based delayed "
				<< "duplicate detection. Try another algorithm.\n";
		return TW_NONE;
	}
	if (graph.nVerts < DDD_MIN_VERTS) {
		cerr << "Error: reduced graph has " << graph.nVerts
				<< " vertices, which is less than the minimum of "
				<< DDD_MIN_VERTS
				<< " for the current compilation of hash-based delayed duplicate "
				<< "detection. Change preprocessing parameter TW_MIN_VERTS and "
				<< "recompile.\n";
		return TW_NONE;
	}
	if (graph.nVerts > DDD_MAX_VERTS) {
		cerr << "Error: reduced graph has " << graph.nVerts
				<< " vertices, which is greater than the maximum of "
				<< DDD_MAX_VERTS
				<< " for the current compilation of hash-based delayed duplicate "
				<< "detection. Change preprocessing parameter TW_MIN_VERTS to "
				<< "within 32 of " << graph.nVerts << " and recompile.\n";
		return TW_NONE;
	}

	// initialize start node
	BFHT_HDDD_Node start(elimprefix_width, postprefix_lb);
#ifdef DEBUG_TW
	assert(stats.currNodes()==0);
#endif
#if (defined GATHER_STATS_DEBUG || defined GATHER_STATS)
	stats.nodeGenerated();
	stats.updateTotals();
#endif

	ushort ub;
	if (cutoffType == BFHT_ID_CUTOFF)
		ub = start.hValue()+1;
	else
		// cutoffType == BFHT_UB_CUTOFF
		ub = ub_order.width;

	while (ub<=ub_order.width) {
		cout << "Deepening cutoff: " << ub << endl;
		stats.setMinFillUB(ub);
		TWExitStatus exit_status = search(soln, starttime, timelim, hversion,
				graph, start, ub);
		// will soln.width>=ub when soln not found?
#if (defined GATHER_STATS_DEBUG || defined GATHER_STATS)
		stats.updateLastIterationStats();
		stats.updateTotals();
#endif
		if (exit_status==TW_NOTIME || exit_status==TW_NOMEM) {
			cerr << "BFHT-HDDD exhausted time limit of " << timelim
					<< " seconds.\n";
			exit_status_val=exit_status;
			return exit_status;
		}
		if (exit_status==TW_SUCCESS)
			break;
		ub++;
#if (defined GATHER_STATS_DEBUG || defined GATHER_STATS)
		if (ub<=ub_order.width)
			stats.anotherSearchForTWIteration();
#endif
	}
	if (ub==ub_order.width+1)
		soln=ub_order;
#ifdef DEBUG_TW
	if (cutoffType==BFHT_ID_CUTOFF)
		assert(soln.width==ub-1);
#endif
#if (defined GATHER_STATS_DEBUG || defined GATHER_STATS)
	stats.finishedSearchForTW();
	stats.foundTW();
#endif
	setSolution(soln, elimprefix, vertexmap);
	solution_val=soln;
	exit_status_val=TW_SUCCESS;
	return exit_status_val;
}

void BreadthFirstTW::finalizeSearch_ddd(BreadthFirstTWNode *start,
		BreadthFirstTWNode *goal) {
	delete start;
	delete goal;
	stats.subNodes(2);
}

// ret.mid_node points to a node in one of currOpen, nextOpen, or midLayer
BFHTSearchForTW_ret BreadthFirstTW::searchForTW_ddd(struct timeval starttime,
		int timelim, int memlim, const HeuristicVersion hversion,
		const ALMGraph &orig_graph, ALMGraph &cur_graph,
		const BreadthFirstTWNode &start, int ub, int mid_layer_depth) {

#if (defined GATHER_STATS_DEBUG || defined GATHER_STATS)
	stats.numLayersInSearchForTW(cur_graph.nVerts+1);
#endif
	// start timer for intermittent output
	struct timeval lasttime;
	struct timezone tz;
	gettimeofday(&lasttime, &tz);
	lasttime.tv_sec -= 60;

	// setup open lists
	BFHT_DDD_OpenList *currOpen, *nextOpen, *midLayer;
	const int min_openlist_hash_size = (int)4e6;
	currOpen = new BFHT_DDD_OpenList(min_openlist_hash_size);
	nextOpen = new BFHT_DDD_OpenList(min_openlist_hash_size);
	midLayer = NULL;

	// expand start node into currOpen - note: start node should be fully reduced
#ifdef DEBUG_BREADTHFTW
	if (cur_graph.potentialSimpAndAlmostSimp[0].vid>0) {
		reduce_one_ret roret_debug = cur_graph.reduceGraphOne(start.fValue());
		assert(roret_debug.elimVert==-1);
	}
#endif
	expandNode(start, cur_graph, ub, mid_layer_depth, *currOpen);
#if (defined GATHER_STATS_DEBUG || defined GATHER_STATS)
	#ifdef TRACK_EXP_BY_FVAL
	stats.nodeExpanded(start.fValue());
	#else
	stats.nodeExpanded();
	#endif
#endif

	// start
	BreadthFirstTWNode *n=NULL;
	while (!currOpen->atEnd()) { // outer loop - once per layer
		int curr_depth=currOpen->topPtr()->nVertsElim();
		int min_gval=INT_MAX; // min gval for next layer
		BreadthFirstTWNode *min_gval_node=NULL; // node with min gval, also gval>=#verts remaining
		cout << "expanding level " << curr_depth << "/" << orig_graph.nVerts
				<< ", with " << currOpen->size() << " nodes.\n";
#if (defined GATHER_STATS_DEBUG || defined GATHER_STATS)
		stats.currLevel(curr_depth);
		stats.setSearchForTWNodes(curr_depth, currOpen->nLeft());
		if (midLayer==NULL)
			stats.setSearchForTWMidNodes(curr_depth, 0);
		else
			stats.setSearchForTWMidNodes(curr_depth, midLayer->nLeft());
#endif
		while (!currOpen->atEnd()) { // inner loop - once per node in currOpen
#ifdef DEBUG_BREADTHFTW
			assert(stats.currNodes()==2);
#endif
			struct timeval currtime;
			gettimeofday(&currtime, &tz);

			// check time limit
			if (DIFFSECS(starttime,currtime)>timelim) {
				cerr << "BreadthFirstTW search exceeded time limit of "
						<< timelim << " seconds.\n";
				delete currOpen;
				delete nextOpen;
				delete midLayer;
				return BFHTSearchForTW_ret(TW_NOTIME);
			}

			// periodic output
			if (DIFFSECS(lasttime,currtime)>30) {
				cout << "  nodes: " << stats.currNodes() << " \tcurrOpen: "
						<< currOpen->size() << " \tnextOpen: "
						<< nextOpen->size() << " \tmidLayer: ";
				cout << (midLayer==NULL ? 0 : midLayer->size()) << endl;
				lasttime=currtime;

				// check memory allocation limit
				uint mem = (VmSize() << 10);
				//	stats.curr_mem = minfo.arena;
				//	if (stats.curr_mem > memlim) {
				if ((int)mem > memlim) {
					cerr
							<< "BreadthFirstTW search exceeded max memory usage of "
							<< (int)(memlim/1e6) << "MB.\n";
					delete currOpen;
					delete nextOpen;
					delete midLayer;
					return BFHTSearchForTW_ret(TW_NOMEM);
				}
			}

			// get a node from currOpen
			n = currOpen->topPtr();

			// test for goal node
			if (n->nVertsElim()==orig_graph.nVerts) {
#ifdef DEBUG_BREADTHFTW
				assert(currOpen->size()==1);
#endif
				break; // done
			}

			// get node's graph
			cur_graph.copy(orig_graph);
			cur_graph.elimVertices(*(n->statePtr())); // need to populate pSAAS bc don't know if node was reduced

			// compute node's hvalue, prune if possible
#ifdef MONOTONIC_H
			n->setHValue(cur_graph.heuristic(hversion,ub));
#else
			int tmp_h = cur_graph.heuristic(hversion, ub);
			if (tmp_h>n->hValue())
				n->setHValue(tmp_h);
#endif
			if (n->fValue() >= ub) {
				currOpen->pop();
				n=NULL;
				continue;
			}

#ifdef BFHT_PRUNE_GRDC
			// try to eliminate a vertex with reduction rules
			reduce_one_ret roret = cur_graph.reduceGraphOne(n->fValue(), ub);
			// if vertex eliminated through reduction rules, insert single child in nextOpen
			if (roret.elimVert!=-1) {
#if (defined GATHER_STATS_DEBUG || defined GATHER_STATS)
				stats.nodeReduced();
	#ifdef TRACK_EXP_BY_FVAL
				stats.nodeExpanded(start.fValue()); // count reducing a node as expanding by generating its "single" child
	#else
				stats.nodeExpanded();
	#endif
#endif
				if (roret.deg<ub) // otherwise prune child
				{
					BreadthFirstTWNode *new_node = nextOpen->insert(*n,
							roret.elimVert, roret.deg, mid_layer_depth);
					if (new_node->gValue() < min_gval) {
						min_gval=new_node->gValue();
						if (min_gval >= cur_graph.nVerts)
							min_gval_node = new_node;
					}
				}
			}
			// if reduction rules don't apply, expand node and insert children in nextOpen
			else
#endif
			{
				int tmp_min_gval;
				BreadthFirstTWNode* tmp_min_gval_node = expandNode(*n,
						cur_graph, ub, mid_layer_depth, *nextOpen,
						&tmp_min_gval);
#if (defined GATHER_STATS_DEBUG || defined GATHER_STATS)
	#ifdef TRACK_EXP_BY_FVAL
				stats.nodeExpanded(start.fValue());
	#else
				stats.nodeExpanded();
	#endif
#endif
				if (tmp_min_gval < min_gval) {
					min_gval = tmp_min_gval;
					if (min_gval >= cur_graph.nVerts-1)
						min_gval_node = tmp_min_gval_node;
				}
			}
			// pop expanded node from open list
			currOpen->pop();
			n=NULL;
		}
		// check for completion
		if (n!=NULL)
			break;
		// check if a node in next layer is on opt path to goal with trivial path to goal
		if (min_gval_node!=NULL && min_gval==min_gval_node->gValue()) {
			n = min_gval_node;
			if (curr_depth+1==mid_layer_depth) // set ancest pointer if n is in mid_layer
				n->setAncest(n);
			break;
		}
		// detect and remove duplicated from nextOpen
		nextOpen->detectDuplicates();
		if (curr_depth+1 == mid_layer_depth) // update ancest pointers if nextOpen is mid_layer
			nextOpen->setMidLayerAncestPtrs();
		// switch open lists
		if (curr_depth == mid_layer_depth) {
			midLayer = currOpen;
		} else {
			delete currOpen;
		}
		currOpen = nextOpen;
		nextOpen = new BFHT_DDD_OpenList(min_openlist_hash_size);
#ifdef DEBUG_TW
		assert(nextOpen->empty());
#endif
	}

	// prepare return structure
#ifdef DEBUG_BREADTHFTW
	if (n==NULL)
		assert(currOpen->empty() && nextOpen->empty());
	else if (n->nVertsElim()==orig_graph.nVerts)
		assert(!n->isAncest(NULL));
#endif
	BFHTSearchForTW_ret ret;
	if (n==NULL) // no path found better than ub
		ret = BFHTSearchForTW_ret(TW_NONE);
	else if (n->nVertsElim()==orig_graph.nVerts) {// found path to goal node
		ret = BFHTSearchForTW_ret(TW_SUCCESS, new BreadthFirstTWNode(*n->getAncest()), n->gValue());
		stats.addNode();
	} else { // found path to a node from which path to goal is trivial (NOTE: may not be after mid layer)
		if (n->getAncest()==NULL || n->getAncest()==n) { // n is the "middle node", actually return a copy of it so that it's not in nextOpen
			ret = BFHTSearchForTW_ret(TW_SUCCESS, new BreadthFirstTWNode(*n), n->gValue());
			stats.addNode();
		} else {
			ret = BFHTSearchForTW_ret(TW_SUCCESS, new BreadthFirstTWNode(*n->getAncest()), n->gValue());
			stats.addNode();
		}
	}
	delete currOpen;
	delete nextOpen;
	delete midLayer;
	return ret;
}








struct DDDThreadData {
	// own data
	int threadid;
	BFHT_HDDD_NodeBuffer *nbuff;
	ALMGraph *graph;
	// shared data
	const ALMGraph *orig_graph;
	BFHT_HDDD_NodeFiles *nodeFiles;
	FileInfoDeque *currExpandQueue;
	FileInfoDeque *nextExpandQueue;
	FileInfoDeque *mergeQueue;
	uint *nunique_nodes;
	TWExitStatus *exit_status;
	HeuristicVersion hversion;
	struct timeval starttime;
	int timelim;
	BreadthFirstTWStats *stats;
	pthread_mutex_t *main_mutex;
	pthread_cond_t *cv;
	int *n_idle_threads;
};


void* dddWorkerThreadRoutine(void *arg)
{
	DDDThreadData *d = (DDDThreadData*)arg;
	struct timeval currtime;
	struct timezone tz;

	pthread_mutex_lock(d->main_mutex);
	while (*d->exit_status==TW_NONE && (!d->currExpandQueue->empty() ||
			!d->mergeQueue->empty() || *d->n_idle_threads+1<N_DDD_THREADS))
	{

		//cout << "   nIncompleteFiles " << d->nodeFiles->nIncompleteFiles() << endl;

		// merge a file
		if (!d->mergeQueue->empty())
		{
			// get a file to merge
			BFHT_HDDD_FileInfo mergeFile = d->mergeQueue->front();
			d->mergeQueue->pop_front();
			cout << d->threadid << ":\t\t\t\t\t\t\t\t" << mergeFile.depth
					<< " merge " << (int)mergeFile.val << "\t"
					<< d->mergeQueue->size() << "\t"
					<< d->nextExpandQueue->size() << endl;

			// merge
			pthread_mutex_unlock(d->main_mutex);
			uint nunique_nodes = d->nodeFiles->merge(mergeFile, *d->nbuff,
					&d->threadid);
//			pthread_mutex_unlock(d->main_mutex);
			pthread_mutex_lock(d->main_mutex);
			d->nunique_nodes += nunique_nodes;

			// add file to next expand queue
			d->nextExpandQueue->push_back(mergeFile);
		}
		// expand a file
		else if (!d->currExpandQueue->empty())
		{
			// get a file to expand
			BFHT_HDDD_FileInfo expandFile = d->currExpandQueue->front();
#if (defined GATHER_STATS_DEBUG || defined GATHER_STATS)
			d->stats->currLevel(expandFile.depth);
#endif
			d->currExpandQueue->pop_front();
			cout << d->threadid << ":" << expandFile.depth << " expand "
					<< (int)expandFile.val << "\t"
					<< d->currExpandQueue->size() << endl;

			// expand file
			//cout << d->threadid << "HERE\n";

			pthread_mutex_unlock(d->main_mutex);
			*d->exit_status = d->nodeFiles->expand(expandFile,
					*d->orig_graph, *d->graph, d->hversion,
					*d->mergeQueue, *d->nbuff, *d->stats, d->main_mutex);
/*			*d->exit_status = d->nodeFiles->expand(expandFile,
					*d->orig_graph, *d->graph, d->hversion,
					*d->mergeQueue, *d->nbuff, *d->stats, NULL);
			pthread_mutex_unlock(d->main_mutex);
*/			pthread_mutex_lock(d->main_mutex);

			//cout << d->threadid << "THERE\n";

			// if currExpandQueue is empty and mergeQueue includes more than
			// one file, then other threads may be waiting on condition
			// variable cv. wake them up to help merge.
			if (d->currExpandQueue->empty() && d->mergeQueue->size()>1) {
				cout << "Thread " << d->threadid << " added new merge files. " << d->mergeQueue->size() << endl;
				pthread_cond_broadcast(d->cv);
			}
		}
		// block until done or file to merge
		else if (*d->n_idle_threads+1 < N_DDD_THREADS)
		{
			cout << "Thread " << d->threadid << " idle.\n";
			(*d->n_idle_threads)++;
			pthread_cond_wait(d->cv, d->main_mutex);
			cout << "Thread " << d->threadid << " reactivated.\n";
			(*d->n_idle_threads)--;
		}

		// check time limit
		gettimeofday(&currtime, &tz);
		if (DIFFSECS(d->starttime,currtime)>d->timelim)
			*d->exit_status = TW_NOTIME;
	}
	cout << "Thread " << d->threadid << " complete. " << (*d->exit_status==TW_NONE) << " " << d->mergeQueue->empty() << endl;
	(*d->n_idle_threads)++;
	pthread_cond_broadcast(d->cv);
	pthread_mutex_unlock(d->main_mutex);
	pthread_exit((void*) 0);
}

// ret.mid_node points to a node in one of currOpen, nextOpen, or midLayer
TWExitStatus BreadthFirstTW::search(ElimOrder &soln, struct timeval starttime,
		int timelim, const HeuristicVersion hversion, const ALMGraph &graph,
		BFHT_HDDD_Node &start, int ub) {

#if (defined GATHER_STATS_DEBUG || defined GATHER_STATS)
	stats.numLayersInSearchForTW(graph.nVerts+1);
#endif
	int initial_ub = ub;
	// search to goal
	uint nunique_nodes = 0;
	TWExitStatus exit_status = TW_NONE;
	ALMGraph curr_graphs[N_DDD_THREADS];
	for (int i=0; i<N_DDD_THREADS; i++)
		curr_graphs[i].init(graph.nVerts);
	FileInfoDeque mergeQueue, expandQueue1, expandQueue2;
	FileInfoDeque *currExpandQueue = &expandQueue1;
	FileInfoDeque *nextExpandQueue = &expandQueue2;
	BFHT_HDDD_NodeBuffer nbuffs[N_DDD_THREADS];
	BFHT_HDDD_NodeFiles nodeFiles(start, *nextExpandQueue, graph,
			curr_graphs[0], hversion,ub, nbuffs[0], stats);
#if N_DDD_THREADS>1
	while (!nextExpandQueue->empty())
	{
#ifdef DEBUG_TW
		assert(currExpandQueue->empty());
		assert(mergeQueue.empty());
		assert(nodeFiles.nIncompleteFiles()==0);
#endif
		nodeFiles.nextLevel();

		// swap expansion queues
		FileInfoDeque *tmp = currExpandQueue;
		currExpandQueue = nextExpandQueue;
		nextExpandQueue = tmp;
#if (defined GATHER_STATS_DEBUG || defined GATHER_STATS)
		stats.setSearchForTWNodes(currExpandQueue->front().depth, nunique_nodes);
#endif
		nunique_nodes = 0;

		// initialize thread data
		pthread_t ddd_threads[N_DDD_THREADS];
		pthread_mutex_t main_mutex = PTHREAD_MUTEX_INITIALIZER;
		pthread_cond_t cv = PTHREAD_COND_INITIALIZER;
		int n_idle_threads = 0;
		DDDThreadData ddd_data[N_DDD_THREADS];
		for (int i=0; i<N_DDD_THREADS; i++)
		{
			ddd_data[i].threadid = i;
			ddd_data[i].nbuff = &nbuffs[i];
			ddd_data[i].graph = &curr_graphs[i];
			ddd_data[i].orig_graph = &graph;
			ddd_data[i].nodeFiles = &nodeFiles;
			ddd_data[i].currExpandQueue = currExpandQueue;
			ddd_data[i].nextExpandQueue = nextExpandQueue;
			ddd_data[i].mergeQueue = &mergeQueue;
			ddd_data[i].nunique_nodes = &nunique_nodes;
			ddd_data[i].exit_status = &exit_status;
			ddd_data[i].hversion = hversion;
			ddd_data[i].starttime = starttime;
			ddd_data[i].timelim = timelim;
			ddd_data[i].stats = &stats;
			ddd_data[i].main_mutex = &main_mutex;
			ddd_data[i].cv = &cv;
			ddd_data[i].n_idle_threads = &n_idle_threads;
		}

		// start worker threads
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
		for (int i=0; i<N_DDD_THREADS; i++)
			pthread_create(&ddd_threads[i], &attr, dddWorkerThreadRoutine,
					(void*)&ddd_data[i]);
		pthread_attr_destroy(&attr);

		// wait on worker threads
		void *status;
		for (int i=0; i<N_DDD_THREADS; i++)
			pthread_join(ddd_threads[i], &status);

		if (exit_status != TW_NONE)
			break;
	}
#else
	struct timeval currtime;
	struct timezone tz;
	while (!currExpandQueue->empty() || !nextExpandQueue->empty()) {
		assert(mergeQueue.empty());
		// new layer, swap expansion queues
		if (currExpandQueue->empty()) {
			nodeFiles.nextLevel();
			assert(nodeFiles.nIncompleteFiles()==0);
			FileInfoDeque *tmp = currExpandQueue;
			currExpandQueue = nextExpandQueue;
			nextExpandQueue = tmp;
#if (defined GATHER_STATS_DEBUG || defined GATHER_STATS)
			stats.setSearchForTWNodes(currExpandQueue->front().depth, nunique_nodes);
#endif
			nunique_nodes = 0;
		}
		// expand a file
		BFHT_HDDD_FileInfo expandFile = currExpandQueue->front();
#if (defined GATHER_STATS_DEBUG || defined GATHER_STATS)
		stats.currLevel(expandFile.depth);
#endif
		currExpandQueue->pop_front();
		cout << expandFile.depth << " expand " << (int)expandFile.val << "\t"
				<< currExpandQueue->size() << endl;
		exit_status = nodeFiles.expand(expandFile, graph, curr_graphs[0],
				hversion, mergeQueue, nbuffs[0], stats);
		if (exit_status == TW_SUCCESS)
			break;
		gettimeofday(&currtime, &tz);
		if (DIFFSECS(starttime,currtime)>timelim) {
			exit_status = TW_NOTIME;
			break;
		}
		// merge files in next layer
		while (!mergeQueue.empty()) {
			BFHT_HDDD_FileInfo mergeFile = mergeQueue.front();
			mergeQueue.pop_front();
			cout << "\t\t\t\t\t\t\t\t" << mergeFile.depth << " merge "
					<< (int)mergeFile.val << "\t" << mergeQueue.size() << "\t"
					<< nextExpandQueue->size() << endl;
			nunique_nodes += nodeFiles.merge(mergeFile, nbuffs[0]);
			nextExpandQueue->push_back(mergeFile);
			gettimeofday(&currtime, &tz);
			if (DIFFSECS(starttime,currtime)>timelim) {
				exit_status = TW_NOTIME;
				break;
			}
		}
		if (exit_status == TW_NOTIME)
			break;
	}
#endif
	// recover solution path
	if ((int)nodeFiles.bestUB()<initial_ub)
		exit_status = TW_SUCCESS;
	if (exit_status == TW_SUCCESS)
		nodeFiles.reconstructSolution(soln, nbuffs[0]);
	// remove files from disk
	nodeFiles.clear();
	return exit_status;
}

BreadthFirstTWNode* BreadthFirstTW::expandNode(const BreadthFirstTWNode &n,
		const ALMGraph &graph, ushort ub, int mid_layer,
		BFHT_DDD_OpenList &openlist, int *min_gval) {

	BreadthFirstTWNode *min_gval_node= NULL;
	int tmpint;
	if (min_gval==NULL)
		min_gval = &tmpint;
	*min_gval = INT_MAX;
	VertexList *curr = graph.vertices[0].next;
	while (curr!=NULL) {
		int vdeg = graph.adjLM[curr->vid][0].vid;
		if (vdeg<ub) {
			BreadthFirstTWNode *new_node = openlist.insert(n, curr->vid, vdeg,
					mid_layer);
			if (new_node->gValue() < *min_gval) {
				*min_gval = (int)new_node->gValue();
				min_gval_node = new_node;
			}
		}
		curr=curr->next;
	}
	return min_gval_node;
}

void BreadthFirstTW::expandNodeSubset(const BreadthFirstTWNode &n,
		const ALMGraph &graph, int mid_layer, const BreadthFirstTWNode &goal,
		BFHT_DDD_OpenList &openlist) {

	VertexList *curr = graph.vertices[0].next;
	while (curr!=NULL) {
		if (goal.statePtr()->checkVert(curr->vid)) {
			int vdeg = graph.adjLM[curr->vid][0].vid;
			if (vdeg <= goal.gValue())
				openlist.insert(n, curr->vid, vdeg, mid_layer);
		}
		curr=curr->next;
	}
}

// ret.mid_node points to a node in one of currOpen, nextOpen, or midLayer
BFHTSearchForSolnPath_ret BreadthFirstTW::searchForSolnPath_ddd(
		struct timeval starttime, int timelim, int memlim,
		HeuristicVersion hversion, const ALMGraph &orig_graph,
		ALMGraph &cur_graph, const BreadthFirstTWNode &start,
		const BreadthFirstTWNode &goal, int mid_layer_depth) {

	// start timer for periodic output
	struct timeval lasttime;
	struct timezone tz;
	gettimeofday(&lasttime, &tz);
	lasttime.tv_sec-=60;

	// setup
#ifdef DEBUG_BREADTHFTW
	assert(cur_graph.nVerts==orig_graph.nVerts-start.nVertsElim());
	assert(mid_layer_depth>start.nVertsElim());
	assert(mid_layer_depth<goal.nVertsElim());
#endif

	// setup open lists
	BFHT_DDD_OpenList *currOpen, *nextOpen, *midLayer;
	const int min_openlist_hash_size = (int)4e6;
	currOpen = new BFHT_DDD_OpenList(min_openlist_hash_size);
	nextOpen = new BFHT_DDD_OpenList(min_openlist_hash_size);
	midLayer = NULL;

	// expand start node into currOpen - note: should be fully reduced
#ifdef DEBUG_BREADTHFTW
	if (cur_graph.potentialSimpAndAlmostSimp[0].vid>0) {
		reduce_one_ret roret_debug = cur_graph.reduceGraphOne(start.fValue());
		assert(roret_debug.elimVert==-1);
	}
#endif
	expandNodeSubset(start, cur_graph, mid_layer_depth, goal, *currOpen);
	if (start.nVertsElim()+1 == mid_layer_depth) // update ancest pointers if currOpen is mid_layer
		currOpen->setMidLayerAncestPtrs();
#if (defined GATHER_STATS_DEBUG || defined GATHER_STATS)
	#ifdef TRACK_EXP_BY_FVAL
				stats.nodeExpanded(start.fValue());
	#else
				stats.nodeExpanded();
	#endif
#endif
#ifdef DEBUG_BREADTHFTW
	assert(!currOpen->empty());
	assert(!currOpen->atEnd());
#endif

	// start
	BreadthFirstTWNode *n;
	while (!currOpen->atEnd()) { // outer loop - once per layer
		int curr_depth=currOpen->topPtr()->nVertsElim();
		cout << "expanding level " << curr_depth << "/" << orig_graph.nVerts
				<< ", with " << currOpen->size() << " nodes.\n";
#if (defined GATHER_STATS_DEBUG || defined GATHER_STATS)
		stats.setSearchForSolnPathNodes(currOpen->size());
		stats.setSearchForSolnPathMidNodes(currOpen->size());
		stats.setSearchForSolnPathLevels(curr_depth);
#endif
		while (!currOpen->atEnd()) { // inner loop - once per node in currOpen
			struct timeval currtime;
			gettimeofday(&currtime, &tz);

			// check time limit
			if (DIFFSECS(starttime,currtime)>timelim) {
				cerr << "BreadthFirstTW search exceeded time limit of "
						<< timelim << " seconds.\n";
				delete currOpen;
				delete nextOpen;
				delete midLayer;
				return BFHTSearchForSolnPath_ret(TW_NOTIME);
			}

			// periodic output
			if (DIFFSECS(lasttime,currtime)>30) {
				cout << "  nodes: " << stats.currNodes() << " \tcurrOpen: "
						<< currOpen->size() << " \tnextOpen: "
						<< nextOpen->size() << " \tmidLayer: ";
				cout << (midLayer==NULL ? 0 : midLayer->size()) << endl;
				lasttime=currtime;

				// check memory allocation limit
				uint mem = (VmSize() << 10);
				//	stats.curr_mem = minfo.arena;
				//	if (stats.curr_mem > memlim) {
				if ((int)mem > memlim) {
					cerr
							<< "BreadthFirstTW search exceeded max memory usage of "
							<< (int)(memlim/1e6) << "MB.\n";
					delete currOpen;
					delete nextOpen;
					delete midLayer;
					return BFHTSearchForSolnPath_ret(TW_NOMEM);
				}
			}

			// get a node from currOpen
			n = currOpen->topPtr();

			// test for goal node <-- can probably move this to generation, since we know exact goal node
			if (n->nVertsElim()==goal.nVertsElim()) {
#ifdef DEBUG_BREADTHFTW
				assert(*n==goal);
				assert(n->gValue()==goal.gValue());
				assert(currOpen->size()==1);
#endif
				break; // done
			}

			// get node's graph
			cur_graph.copy(orig_graph);
			cur_graph.elimVertices(*(n->statePtr()));

			// compute node's hvalue, prune if possible
#ifdef MONOTONIC_H
			n->setHValue(cur_graph.heuristic(hversion,goal.fValue()+1));
#else
			int tmp_h = cur_graph.heuristic(hversion, goal.fValue()+1);
			if (tmp_h>n->hValue())
				n->setHValue(tmp_h);
#endif
			if (n->hValue() > goal.fValue()) { // this assumes cost function is monotonic... well, if not it may prune some nodes on an opt path to goal, though it will still leave the parent node that was actually used to generate goal
				currOpen->pop();
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
				stats.nodeExpanded(start.fValue());
	#else
				stats.nodeExpanded();
	#endif
#endif
				if (roret.deg <= goal.gValue()) // otherwise prune child
					BreadthFirstTWNode *new_node = nextOpen->insert(*n,
							roret.elimVert, roret.deg, mid_layer_depth);

			}
			// if reduction rules don't apply, expand node and insert children in nextOpen
			else {
				expandNodeSubset(*n, cur_graph, mid_layer_depth, goal,
						*nextOpen);
#if (defined GATHER_STATS_DEBUG || defined GATHER_STATS)
	#ifdef TRACK_EXP_BY_FVAL
				stats.nodeExpanded(start.fValue());
	#else
				stats.nodeExpanded();
	#endif
#endif
			}

			// pop expanded node from open list
			currOpen->pop();
			n=NULL;
		}

		// check for completion
		if (n!=NULL)
			break;

		// detect and remove duplicate nodes
		nextOpen->detectDuplicates();
		if (curr_depth+1 == mid_layer_depth) // update ancest pointers if nextOpen is mid_layer
			nextOpen->setMidLayerAncestPtrs();

		// switch open lists
		if (curr_depth == mid_layer_depth) {
			midLayer = currOpen;
		} else {
			delete currOpen;
		}
		currOpen = nextOpen;
		nextOpen = new BFHT_DDD_OpenList(min_openlist_hash_size);
#ifdef DEBUG_TW
		assert(!currOpen->atEnd());
		assert(!currOpen->empty());
		assert(nextOpen->empty());
#endif
	}

	// prepare return structure
#ifdef DEBUG_BREADTHFTW
	assert(n!=NULL);
	assert(!n->isAncest(NULL));
	assert(n->gValue()==goal.gValue());
	assert(*n==goal);
#endif
	BFHTSearchForSolnPath_ret ret(TW_SUCCESS, new BreadthFirstTWNode(*n->getAncest()));
	stats.addNode();
	delete currOpen;
	delete nextOpen;
	delete midLayer;
	return ret;
}

}
