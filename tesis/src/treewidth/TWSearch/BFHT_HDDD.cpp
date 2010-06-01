#include "preproc_flags.h"
#include "BFHT_HDDD.h"
#include <cassert>
#include <iostream>
#include <dirent.h>
#include <sys/stat.h>
#include <cstring>

#include <iterator>
#include <algorithm>

#include <errno.h>

namespace Treewidth_Namespace {

BFHT_HDDD_NodeBuffer::BFHT_HDDD_NodeBuffer() : ht(BFHT_BUFFER_SIZE/2),
	use_ht_elems(true)
{
	ht_elems.reserve(0.1*ht.bucket_count());
	buffer = new BFHT_HDDD_Node[BFHT_BUFFER_SIZE];
#ifdef DEBUG_TW
	assert(ht_elems.capacity()==(int)(0.1*ht.bucket_count()));
	assert(buffer!=NULL);
#endif
	m_input_buff = buffer;
	m_input_buff_size = 0.45*BFHT_BUFFER_SIZE;
	m_unique_buff = m_input_buff + m_input_buff_size;
	m_unique_buff_size = 0.45*BFHT_BUFFER_SIZE;
	m_overflow_buff = m_unique_buff + m_unique_buff_size;
	m_overflow_buff_size = BFHT_BUFFER_SIZE - m_input_buff_size
			- m_unique_buff_size;
	e_input_buff = buffer;
	e_input_buff_size = 0.2*BFHT_BUFFER_SIZE;
	e_child_buffs = e_input_buff + e_input_buff_size;
	e_child_buffs_size = BFHT_BUFFER_SIZE - e_input_buff_size;
}

BFHT_HDDD_NodeFiles::BFHT_HDDD_NodeFiles(BFHT_HDDD_Node &start,
		deque<BFHT_HDDD_FileInfo> &expandQueue, const ALMGraph &orig_graph,
		ALMGraph &curr_graph, HeuristicVersion hversion, int ubval,
		BFHT_HDDD_NodeBuffer &nbuff, BreadthFirstTWStats &stats) :
			ubv(ubval), ub_node_valid(false), n_incomplete_files(0),
			curr_depth(0), unexpanded_files(&file_set1),
			generated_files(&file_set2)
{
	bool searchFromBeginning = true;
	DIR *dp = opendir("/d0/nodes");
	if (dp!=NULL) {
		cout << "Continue interrupted search? (y/n) ";
		char c;
		cin >> c;
		if (c=='y' || c=='Y')
			searchFromBeginning = !init("/d0/nodes", expandQueue);
		if (searchFromBeginning)
			removeAllNodeFiles("/d0/nodes");
		closedir(dp);
	}
	if (searchFromBeginning) {
		int ret = mkdir("/d0/nodes",
				(S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH));
		assert(ret==0);
		init(start, expandQueue, orig_graph, curr_graph, hversion, ubval,
				nbuff, stats);
	}
}

/* not thread-safe */
void BFHT_HDDD_NodeFiles::init(BFHT_HDDD_Node &start,
		deque<BFHT_HDDD_FileInfo> &expandQueue, const ALMGraph &orig_graph,
		ALMGraph &curr_graph, HeuristicVersion hversion, int &ubval,
		BFHT_HDDD_NodeBuffer &nbuff, BreadthFirstTWStats &stats) {

	// reset bookkeeping data
	ubv = ubval;
	ub_node_valid = false;
	n_incomplete_files = 0;
	curr_depth = 0;
	unexpanded_files->clear();
	generated_files->clear();
	expanded_files.clear();

	// expand start node in child buffers
	BFHT_HDDD_FileInfo expandFile(0, 0);
	vector<ChildFileInfo> childFileInfoTable;
	setupChildFileInfoTable(expandFile, childFileInfoTable,
			nbuff.e_child_buffs_size);
	vector<uint> child_buff_currs(NUM_CHILD_FILES, 0);
	uint child_buff_size = nbuff.e_child_buffs_size/NUM_CHILD_FILES;
	expandNode(&start, orig_graph, curr_graph, hversion, expandFile,
			childFileInfoTable, child_buff_currs, child_buff_size, nbuff,
			stats);

	// flush child buffers to disk
	flushChildBuffers(nbuff, expandFile, childFileInfoTable, child_buff_currs,
			child_buff_size);

	// add child files to expandQueue
	for (uint i=0; i<NUM_CHILD_FILES; i++) {
		const ChildFileInfo *cf = &childFileInfoTable[i];
		if (generated_files->find(cf->val)!=generated_files->end() &&
				childFileReadyToMerge(cf->val)) {
			BFHT_HDDD_FileInfo mergeFile(expandFile.depth+1, cf->val);
			expandQueue.push_back(mergeFile);
			n_incomplete_files--;
		}
	}

}

bool BFHT_HDDD_NodeFiles::init(const char *dirname,
		deque<BFHT_HDDD_FileInfo> &expandQueue) {

	// determine last fully generated layer
	DIR *dp = opendir(dirname);
	if (dp==NULL)
		return false;
	vector<string> layerdirs;
	struct dirent *ep;
	while ((ep = readdir(dp)))
		layerdirs.push_back(ep->d_name);
	closedir(dp);
	set <int> layers;
	int maxlayer = -1;
	string maxlayerdir;
	string currlayerdir;
	for (uint i=0; i<layerdirs.size(); i++) {
		int layer;
		int ret = sscanf(layerdirs[i].data(), "l%u", &layer);
		if (ret>0) {
			layers.insert(layer);
			if (layer>maxlayer) {
				if (maxlayer==layer-1)
					currlayerdir=maxlayerdir;
				maxlayer=layer;
				maxlayerdir=layerdirs[i];
			} else if (layer==maxlayer-1)
				currlayerdir=layerdirs[i];
		}
	}

	if (maxlayer<1) {
		cerr
				<< "Could not continue interrupted search because no layer was fully expanded.\n";
		return false;
	}
	for (int i=1; i<=maxlayer; i++) {
		if (layers.find(i)==layers.end()) {
			cerr
					<< "Could not continue interrupted search because layers were missing.\n";
			return false;
		}
	}

	// delete last partially generated layer
	int currlayer = maxlayer-1;
	char buff[1000];
	sprintf(buff, "%s/%s", dirname, maxlayerdir.data());
	dp = opendir(buff);
	assert(dp!=NULL);
	while ((ep = readdir(dp))) {
		if (strcmp(ep->d_name, ".")==0 || strcmp(ep->d_name, "..")==0)
			continue;
		char buff2[1000];
		sprintf(buff2, "%s/%s", buff, ep->d_name);
		int ret = remove(buff2);
		assert(ret==0);
	}
	closedir(dp);
	int ret = remove(buff);
	assert(ret==0);

	// add last layer to expandQueue and generated_files
	sprintf(buff, "%s/%s", dirname, currlayerdir.data());
	dp = opendir(buff);
	assert(dp!=NULL);
	vector<string> nodefilenames;
	while ((ep = readdir(dp)))
		nodefilenames.push_back(ep->d_name);
	closedir(dp);
	for (uint i=0; i<nodefilenames.size(); i++) {
		int fileval;
		int ret = sscanf(nodefilenames[i].data(), "%*u_%u.nodes", &fileval);
		if (ret>0) {
			expandQueue.push_back(BFHT_HDDD_FileInfo(currlayer, fileval));
			generated_files->insert(fileval);
		}
	}

	// update other bookkeeping structures
	n_incomplete_files = 0;
	curr_depth = currlayer;
	unexpanded_files->clear();

	// add previous layers to expanded_files
	dp = opendir(dirname);
	while ((ep = readdir(dp))) {
		if (strcmp(ep->d_name, currlayerdir.data())==0 || 
		    strcmp(ep->d_name, ".")==0 || strcmp(ep->d_name, "..")==0)
			continue;
		sprintf(buff, "%s/%s", dirname, ep->d_name);
		DIR *dp2 = opendir(buff);
		if (dp2!=NULL) {
			struct dirent *ep2;
			while ((ep2 = readdir(dp2))) {
				if (strcmp(ep2->d_name, ".")==0 || strcmp(ep2->d_name, "..")==0)
					continue;
				int depth, val;
				int ret = sscanf(ep2->d_name, "%u_%u.nodes", &depth, &val);
				if (ret>=2)
					expanded_files.push_back(BFHT_HDDD_FileInfo(depth, val));
			}
			closedir(dp2);
		}
	}
	closedir(dp);
	return true;
}

uint BFHT_HDDD_NodeFiles::merge(const BFHT_HDDD_FileInfo &mergeFile,
		BFHT_HDDD_NodeBuffer &nbuff, int *threadid) const {

	char buff1[MAX_FILENAME_SIZE];
	char buff2[MAX_FILENAME_SIZE];
	bool mergingOverflow = false;
	bool anyOverflowOnDisk = false;
	uint unique_curr = 0;
	uint overflow_curr = 0;
	uint nunique = 0;
	// open file to merge
	getFilename(mergeFile, buff1, buff2);
#ifdef DEBUG_TW
	DIR *tmp_dp = opendir(buff1);
	assert(tmp_dp!=NULL);
	closedir(tmp_dp);
#endif
	FILE *infile = fopen(buff2, "r");
	assert(infile!=NULL);
	do {
#ifdef DEBUG_TW
		assert(nbuff.ht.empty());
#endif
		anyOverflowOnDisk = false;
		// merge file
		while (feof(infile)==0) {
			// read merge file into input buffer
			size_t n = fread(nbuff.m_input_buff, sizeof(BFHT_HDDD_Node),
					nbuff.m_input_buff_size, infile);
			assert(ferror(infile)==0);
			assert(n>0);
			// step through nodes in buffer
			for (uint input_curr=0; input_curr<n; input_curr++) {
				NodePtrHTci iter;
				BFHT_HDDD_Node *tmp = &nbuff.m_input_buff[input_curr];
				iter = nbuff.ht.find(tmp);
				// no dupe, add to hash
				if (iter==nbuff.ht.end()) {
					nunique++;
					if (unique_curr < nbuff.m_unique_buff_size) {
						nbuff.m_unique_buff[unique_curr] =
							nbuff.m_input_buff[input_curr];
						pair<NodePtrHTi,bool> insert_ret =
							nbuff.ht.insert(&nbuff.m_unique_buff[unique_curr]);
						unique_curr++;
#ifdef DEBUG_TW
						assert(insert_ret.second==true);
#endif
						// if ht load factor still under 0.1, add to ht_elems
						if (nbuff.use_ht_elems && nbuff.ht_elems.size() <
								nbuff.ht_elems.capacity())
						{
							nbuff.ht_elems.push_back(insert_ret.first);
#ifdef DEBUG_TW
							assert(nbuff.ht_elems.capacity()==
								(int)(0.1*nbuff.ht.bucket_count()));
#endif
						}
						else
						{
							nbuff.use_ht_elems = false;
						}
					} else { // write node to overflow

						if (overflow_curr==0) {
							cout << "OVERFLOW\n";
						}

						if (overflow_curr >= nbuff.m_overflow_buff_size) { // flush overflow to disk

							cout << "PURGE OVERFLOW 1 (" << overflow_curr
									<< ")\n";

							anyOverflowOnDisk = true;
							getOverflowFilename(mergeFile, buff2, threadid);
							flushBuffer(nbuff.m_overflow_buff, nbuff.m_overflow_buff_size,
									".", buff2, true);
							overflow_curr=0;
						}
						nbuff.m_overflow_buff[overflow_curr++]
								= nbuff.m_input_buff[input_curr];
					}
				}
				// new node better than old dupe, replace old
				else if (nbuff.m_input_buff[input_curr].gValue() <=
						(*iter)->gValue()) {
					(*iter)->copyDuplicate(nbuff.m_input_buff[input_curr]);
				}
				// new node worse than old dupe, maybe copy hvalue
				else if (nbuff.m_input_buff[input_curr].hValue() >
						(*iter)->hValue()) {
					(*iter)->setHValue(nbuff.m_input_buff[input_curr].hValue());
				}
			}
		}
		// write merged file back to disk
		fclose(infile);
		FILE *outfile;
		getFilename(mergeFile, buff1, buff2);
		DIR *dp = opendir(buff1);
		if (dp==NULL) {
			int ret = mkdir(buff1,
					(S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH));
			assert(ret==0);
		} else
			closedir(dp);
		if (mergingOverflow)
			outfile = fopen(buff2, "a");
		else
			outfile = fopen(buff2, "w");
		assert(outfile!=NULL);
		fwrite(nbuff.m_unique_buff, sizeof(BFHT_HDDD_Node), unique_curr, outfile);
		assert(ferror(outfile)==0);
		unique_curr=0;
		// clear the hash table
		if (nbuff.use_ht_elems)
		{
			for (uint i=0; i<nbuff.ht_elems.size(); i++)
				nbuff.ht.erase(nbuff.ht_elems[i]);
		}
		else
		{
			nbuff.ht.clear();
			nbuff.use_ht_elems = true;
		}
		nbuff.ht_elems.clear();
#ifdef DEBUG_TW
		assert(nbuff.ht.empty());
#endif
		fclose(outfile);

		// flush overflow buffer
		if (overflow_curr > 0) {

			cout << "PURGE OVERFLOW 2 (" << overflow_curr << ")\n";

			anyOverflowOnDisk = true;
			getOverflowFilename(mergeFile, buff2, threadid);
			flushBuffer(nbuff.m_overflow_buff, overflow_curr, ".", buff2, true);
			overflow_curr=0;
		}
		// merge any overflow
		if (anyOverflowOnDisk) {
			mergingOverflow = true;
			// rename overflow file
			getOverflowFilename(mergeFile, buff1, threadid);
			getOverflowFilename2(mergeFile, buff2, threadid);
			rename(buff1, buff2);
			// open overflow file to merge
			infile = fopen(buff2, "r");
			assert(infile!=NULL);
		}
	} while (anyOverflowOnDisk);
	// remove overflow files from disk
	getOverflowFilename(mergeFile, buff1, threadid);
	remove(buff1);
	getOverflowFilename2(mergeFile, buff1, threadid);
	remove(buff1);

	return nunique;
}

/* mutex will be locked before access to member variables, mergeQueue, and stats */
TWExitStatus BFHT_HDDD_NodeFiles::expand(BFHT_HDDD_FileInfo &expandFile,
		const ALMGraph &orig_graph, ALMGraph &curr_graph,
		HeuristicVersion hversion, deque<BFHT_HDDD_FileInfo> &mergeQueue,
		BFHT_HDDD_NodeBuffer &nbuff, BreadthFirstTWStats &stats,
		pthread_mutex_t *main_mutex) {

	char buff1[MAX_FILENAME_SIZE];
	char buff2[MAX_FILENAME_SIZE];
	vector<uint> child_buff_currs(NUM_CHILD_FILES, 0);
	uint child_buff_size = nbuff.e_child_buffs_size/NUM_CHILD_FILES;
	vector<ChildFileInfo> childFileInfoTable;
	setupChildFileInfoTable(expandFile, childFileInfoTable,
			nbuff.e_child_buffs_size);

	// open file to expand
	getFilename(expandFile, buff1, buff2);
#ifdef DEBUG_TW
	DIR *tmp_dp = opendir(buff1);
	assert(tmp_dp!=NULL);
	closedir(tmp_dp);
#endif
	FILE *infile = fopen(buff2, "r");
	assert(infile!=NULL);
	// expand file
	while (feof(infile)==0) {
		// read expand file into input buffer
		size_t n_inputs = fread(nbuff.e_input_buff, sizeof(BFHT_HDDD_Node),
				nbuff.e_input_buff_size, infile);
		assert(ferror(infile)==0);
		assert(n_inputs>0);
		// step through nodes in buffer
		for (uint input_curr=0; input_curr<n_inputs; input_curr++) {
			BFHT_HDDD_Node *n = &nbuff.e_input_buff[input_curr];
			TWExitStatus es = expandNode(n, orig_graph, curr_graph, hversion,
					expandFile, childFileInfoTable, child_buff_currs,
					child_buff_size, nbuff, stats, main_mutex);
			if (es==TW_SUCCESS)
				return TW_SUCCESS;
		}
	}
	fclose(infile);

	// flush child buffers to disk, add to mergeQueue if possible
	flushChildBuffers(nbuff, expandFile, childFileInfoTable, child_buff_currs,
			child_buff_size, main_mutex);

	// mark expandFile as expanded and try to add children to mergeQueue
	if (main_mutex!=NULL)
		pthread_mutex_lock(main_mutex);
#ifdef DEBUG_TW
	assert(curr_depth==expandFile.depth);
	assert(unexpanded_files->find(expandFile.val)!=unexpanded_files->end());
#endif
	unexpanded_files->erase(expandFile.val);
	expanded_files.push_back(expandFile);
	for (uint i=0; i<NUM_CHILD_FILES; i++) {
		const ChildFileInfo *cf = &childFileInfoTable[i];
		if (generated_files->find(cf->val)!=generated_files->end() &&
				childFileReadyToMerge(cf->val)) {
			BFHT_HDDD_FileInfo mergeFile(expandFile.depth+1, cf->val);
			mergeQueue.push_back(mergeFile);
			n_incomplete_files--;
		}
	}
	if (main_mutex!=NULL)
		pthread_mutex_unlock(main_mutex);

	return TW_NONE;
}

/* locks mutex before access to member variables and stats */
TWExitStatus BFHT_HDDD_NodeFiles::expandNode(BFHT_HDDD_Node* n,
		const ALMGraph &orig_graph, ALMGraph &curr_graph,
		HeuristicVersion hversion, const BFHT_HDDD_FileInfo &expandFile,
		const vector<ChildFileInfo> &childFileInfoTable,
		vector<uint> &child_buff_currs, uint child_buff_size,
		BFHT_HDDD_NodeBuffer &nbuff, BreadthFirstTWStats &stats,
		pthread_mutex_t *main_mutex)
{
	// get local copy of ub
	if (main_mutex!=NULL)
		pthread_mutex_lock(main_mutex);
	uint ubl = ubv;
	if (main_mutex!=NULL)
		pthread_mutex_unlock(main_mutex);

	if (n->fValue() >= ubl)
		return TW_NONE;
	// get node's graph
	vector<int> adjacentVerts;
	int vid = n->lastElim();
	curr_graph.copy(orig_graph);
	curr_graph.elimVertices(*(n->statePtr()), &vid, &adjacentVerts);
	// test for goal node
	if (curr_graph.nVerts==0)
		return TW_SUCCESS;
	// compute node's hvalue, prune if possible
#ifdef MONOTONIC_H
	n->setHValue(cur_graph.heuristic(hversion,ubl));
#else
	int tmp_h = curr_graph.heuristic(hversion, ubl);
	if (tmp_h>n->hValue())
		n->setHValue(tmp_h);
#endif
	if (n->fValue() >= ubl)
		return TW_NONE;

	// local variables for keeping stats
	int nexp = 0;
	int ngen = 0;
	int nred = 0;

	// try to eliminate a vertex with reduction rules
	reduce_one_ret roret = curr_graph.reduceGraphOne(n->fValue(), ubl);
	// if vertex eliminated through reduction rules, generate single child
	if (roret.elimVert!=-1) {
#if (defined GATHER_STATS_DEBUG || defined GATHER_STATS)
		nred++;
		nexp++; // count reducing a node as expanding by generating its "single" child
	#ifdef TRACK_EXP_BY_FVAL
		stats.nodeExpanded(n->fValue());
	#else
		stats.nodeExpanded();
	#endif
#endif
		// test for improved ub
		uint gval = (roret.deg < n->gValue() ? n->gValue() : roret.deg);
		if ((int)gval >= curr_graph.nVerts-1 && gval < ubl) {
			if (main_mutex!=NULL)
				pthread_mutex_lock(main_mutex);
			ubl = ubv = gval;
			ub_node.generateChild(n, roret.elimVert, roret.deg);
			ub_node_valid = true;
			if (main_mutex!=NULL)
				pthread_mutex_unlock(main_mutex);
			cout << "NEW UPPER BOUND FOUND: " << ubl << endl;
		}
		// generate new child
		else if (roret.deg<(int)ubl) {
			generate(n, roret.elimVert, roret.deg, expandFile,
					childFileInfoTable, child_buff_currs, child_buff_size,
					nbuff, main_mutex);
#if (defined GATHER_STATS_DEBUG || defined GATHER_STATS)
			ngen++;
#endif
		}

	}
	// if reduction rules don't apply, expand node and generate children
	else {
#if (defined GATHER_STATS_DEBUG || defined GATHER_STATS)
		nexp++;
	#ifdef TRACK_EXP_BY_FVAL
		stats.nodeExpanded(n->fValue());
	#else
		stats.nodeExpanded();
	#endif
#endif
		// Setup adjverts vector to be used to avoid eliminating a vertex
		// that was adjacent to the vertex eliminated to generate n. This is
		// possible because we can always find an optimal elimination order
		// that, while the graph is not a clique, only eliminates non-adjacent
		// vertices consecutively (Dirac? see Kloks 1994, page 8).
		vector<bool> adjverts(curr_graph.vertices.size(), false);
		for (uint i=0; i<adjacentVerts.size(); i++)
			adjverts[adjacentVerts[i]] = true;

		// Try to generate each child node
		VertexList *curr = curr_graph.vertices[0].next;
		while (curr!=NULL) {
			int vdeg = curr_graph.adjLM[curr->vid][0].vid;
			// test for improved ub
			uint gval = (vdeg < n->gValue() ? n->gValue() : vdeg);
			if ((int)gval >= curr_graph.nVerts-2 && gval < ubl) {
				if (main_mutex!=NULL)
					pthread_mutex_lock(main_mutex);
				ubl = ubv = gval;
				ub_node.generateChild(n, curr->vid, vdeg);
				ub_node_valid = true;
				if (main_mutex!=NULL)
					pthread_mutex_unlock(main_mutex);
				cout << "NEW UPPER BOUND FOUND: " << ubl << endl;
			}
			// generate new child
			else if (vdeg<(int)ubl && !adjverts[curr->vid]) {
				generate(n, curr->vid, vdeg, expandFile, childFileInfoTable,
						child_buff_currs, child_buff_size, nbuff, main_mutex);
#if (defined GATHER_STATS_DEBUG || defined GATHER_STATS)
				ngen++;
#endif
			}
			curr=curr->next;
		}
	}

	// update stats
	if (main_mutex!=NULL)
		pthread_mutex_lock(main_mutex);
	stats.nodeGenerated(ngen);
	stats.nodeReduced(nred);
	if (main_mutex!=NULL)
		pthread_mutex_unlock(main_mutex);

	return TW_NONE;
}

/* locks mutex before accessing member variables */
void BFHT_HDDD_NodeFiles::generate(const BFHT_HDDD_Node *parent, uint vid,
		uint deg, const BFHT_HDDD_FileInfo &expandFile,
		const vector<ChildFileInfo> &childFileInfoTable,
		vector<uint> &child_buff_currs, uint child_buff_size,
		BFHT_HDDD_NodeBuffer &nbuff, pthread_mutex_t *main_mutex)
{
	char buff1[MAX_FILENAME_SIZE];
	char buff2[MAX_FILENAME_SIZE];
	uint cfindex = getChildFileIndex(expandFile, vid);
	const ChildFileInfo *cf = &childFileInfoTable[cfindex];
	BFHT_HDDD_Node *child_buff = &nbuff.e_child_buffs[cf->address];
	if (child_buff_currs[cfindex]>=child_buff_size) { // flush child buff to disk
		getChildFilename(expandFile, cf->val, buff1, buff2);

		// if creating a new file, update bookkeeping
		if (main_mutex!=NULL)
			pthread_mutex_lock(main_mutex);
		if (generated_files->find(cf->val)==generated_files->end())
		{
			n_incomplete_files++;
			generated_files->insert(cf->val);
		}
		if (main_mutex!=NULL)
			pthread_mutex_unlock(main_mutex);

		flushBuffer(child_buff, child_buff_size, buff1, buff2, true);
		child_buff_currs[cfindex]=0;
	}
	child_buff[child_buff_currs[cfindex]++].generateChild(parent, vid, deg);
}

/* locks mutex before access to member variables */
void BFHT_HDDD_NodeFiles::flushChildBuffers(BFHT_HDDD_NodeBuffer &nbuff,
		const BFHT_HDDD_FileInfo &expandFile,
		const vector<ChildFileInfo> &childFileInfoTable,
		vector<uint> &child_buff_currs, uint child_buff_size,
		pthread_mutex_t *main_mutex)
{
	// flush child buffers to disk, add to mergeQueue if possible
	char buff1[MAX_FILENAME_SIZE];
	char buff2[MAX_FILENAME_SIZE];
	for (uint i=0; i<NUM_CHILD_FILES; i++) {
		const ChildFileInfo *cf = &childFileInfoTable[i];
		// flush file to disk
		getChildFilename(expandFile, cf->val, buff1, buff2);
		if (child_buff_currs[i] > 0) {

			// if creating a new file, update bookkeeping
			if (main_mutex!=NULL)
				pthread_mutex_lock(main_mutex);
			if (generated_files->find(cf->val)==generated_files->end())
			{
				n_incomplete_files++;
				generated_files->insert(cf->val);
			}
			if (main_mutex!=NULL)
				pthread_mutex_unlock(main_mutex);

			flushBuffer(&nbuff.e_child_buffs[cf->address], child_buff_currs[i],
					buff1, buff2, true);
			child_buff_currs[i]=0;
		}
	}
}

void BFHT_HDDD_NodeFiles::flushBuffer(BFHT_HDDD_Node *buffer, uint size,
		const char* dirname, const char *filename, bool append)
{
	DIR *dp = opendir(dirname);
	if (dp==NULL) {
		int ret = mkdir(dirname,
				(S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH));
		assert(ret==0 || errno==EEXIST);
	} else
		closedir(dp);
	FILE *file = (append ? fopen(filename, "a") : fopen(filename, "w"));
	assert(file!=NULL);
	fwrite(buffer, sizeof(BFHT_HDDD_Node), size, file);
	if (ferror(file)!=0) {
		cout << "FERROR: writing to " << filename << endl;
		printf("  errno string (%i): %m\n", errno);
	}
	assert(ferror(file)==0);
	fclose(file);
}

/* not thread-safe */
bool BFHT_HDDD_NodeFiles::childFileReadyToMerge(uchar child_val) const
{
	if (unexpanded_files->find(child_val)!=unexpanded_files->end())
		return false;

	for (int i=0; i<8; i++) {
		uchar val = child_val ^ ((uchar)1<<i);
		if (unexpanded_files->find(val)!=unexpanded_files->end())
			return false;
	}
	return true;
}

/* not thread-safe */
void BFHT_HDDD_NodeFiles::reconstructSolution(ElimOrder &soln,
		BFHT_HDDD_NodeBuffer &nbuff) {

	vector <ushort> revsoln;
	char buff1[MAX_FILENAME_SIZE];
	char buff2[MAX_FILENAME_SIZE];
	assert(ub_node_valid);
	/*
	 // try to find goal node
	 BFHT_HDDD_FileInfo fileInfo(BFHT_HDDD_State::nVerts(), (uchar)-1);
	 char buff1[MAX_FILENAME_SIZE];
	 char buff2[MAX_FILENAME_SIZE];
	 getFilename(fileInfo, buff1, buff2);
	 FILE *infile = fopen(buff2, "r");
	 BFHT_HDDD_State state;
	 if (infile!=NULL) { // goal node found
	 // read goal node from file
	 size_t n = fread(buffer, sizeof(BFHT_HDDD_Node), BFHT_BUFFER_SIZE,
	 infile);
	 assert(ferror(infile)==0);
	 assert(n==1);
	 fclose(infile);
	 // record eliminated vertex
	 soln.width = buffer[0].gValue();
	 revsoln.push_back(buffer[0].lastElim());
	 state = *buffer[0].statePtr();
	 } else { // goal node not found, use ub
	 */
	BFHT_HDDD_FileInfo fileInfo(ub_node);
	soln.width = ubv;
	revsoln.push_back(ub_node.lastElim());
	BFHT_HDDD_State state = *ub_node.statePtr();
	// get indifferent suffix
	vector<int> verts;
	state.getRemainingVertices(verts);
	soln.indiff_suffix.insert(verts.begin(), verts.end());
	//}
	// recover the rest of the solution
	while (fileInfo.depth>1) {
		fileInfo.depth--;
		if (revsoln.back()>32)
			fileInfo.val ^= ((uchar)1<<((revsoln.back()-1)%8));
		state.clearVert(revsoln.back());
		// open next node's file
		getFilename(fileInfo, buff1, buff2);
#ifdef DEBUG_TW
		DIR *tmp_dp = opendir(buff1);
		assert(tmp_dp!=NULL);
		closedir(tmp_dp);
#endif
		FILE *infile = fopen(buff2, "r");
		assert(infile!=NULL);
		while (feof(infile)==0) {
			// read expand file into input buffer
			size_t n = fread(nbuff.buffer, sizeof(BFHT_HDDD_Node),
					nbuff.BFHT_BUFFER_SIZE, infile);
			assert(ferror(infile)==0);
			assert(n>0);
			// step through nodes in buffer
			bool node_found = false;
			for (uint i=0; i<n; i++)
				if (state==*nbuff.buffer[i].statePtr()) {
					revsoln.push_back(nbuff.buffer[i].lastElim());
					node_found = true;
				}
			if (node_found)
				break;
		}
		fclose(infile);
	}
	// copy revsoln to soln
	soln.order_prefix.resize(revsoln.size());
	copy(revsoln.rbegin(), revsoln.rend(), soln.order_prefix.begin());
}

void BFHT_HDDD_NodeFiles::clear() {

	char buff1[MAX_FILENAME_SIZE];
	char buff2[MAX_FILENAME_SIZE];

	// remove expanded files
	for (uint i=0; i<expanded_files.size(); i++) {
		getFilename(expanded_files[i], buff1, buff2);
		remove(buff2);
		remove(buff1);
	}

	// remove unexpanded files (at current layer)
	set<uchar>::iterator si = unexpanded_files->begin();
	while (si != unexpanded_files->end()) {
		getFilename(BFHT_HDDD_FileInfo(curr_depth, *si), buff1, buff2);
		remove(buff2);
		remove(buff1);
		si++;
	}

	// remove generated files (at next layer)
	si = generated_files->begin();
	while (si != generated_files->end()) {
		getFilename(BFHT_HDDD_FileInfo(curr_depth+1, *si), buff1, buff2);
		remove(buff2);
		remove(buff1);
		si++;
	}

	int ret = remove("/d0/nodes");
	assert(ret==0);
}

void BFHT_HDDD_NodeFiles::removeAllNodeFiles(const char *dirname) const {

	DIR *dp = opendir(dirname);
	if (dp==NULL)
		return;

	struct dirent *ep;
	while ((ep = readdir(dp))) {
		if (strcmp(ep->d_name, ".")==0 || strcmp(ep->d_name, "..")==0)
			continue;

		char buff[1000];
		sprintf(buff, "%s/%s", dirname, ep->d_name);
		DIR *dp2 = opendir(buff);
		if (dp2!=NULL) {
			struct dirent *ep2;
			while ((ep2 = readdir(dp2))) {
				if (strcmp(ep2->d_name, ".")==0 || strcmp(ep2->d_name, "..")==0)
					continue;

				char buff2[1000];
				sprintf(buff2, "%s/%s/%s", dirname, ep->d_name, ep2->d_name);
				int ret = remove(buff2);
				assert(ret==0);
			}
		}
		closedir(dp2);
		int ret = remove(buff);
		assert(ret==0);
	}
	closedir(dp);
	int ret = remove(dirname);
	assert(ret==0);
}



void BFHT_HDDD_NodeFiles::nextLevel()
{
#ifdef DEBUG_TW
		assert(unexpanded_files->empty());
#endif
		curr_depth++;
		set<uchar> *tmp_file_set = unexpanded_files;
		unexpanded_files = generated_files;
		generated_files = tmp_file_set;
}

}
