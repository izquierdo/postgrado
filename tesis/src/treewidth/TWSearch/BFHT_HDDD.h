#ifndef BFHT_HDDD_NODEFILES_H_
#define BFHT_HDDD_NODEFILES_H_

#include "preproc_flags.h"
#include <ext/hash_set>
#include <map>
#include <deque>
#include "BFHT_HDDD_Node.h"
#include "ALMGraph.h"
#include "TWSearch.h"
#include "BreadthFirstTWStats.h"

using namespace __gnu_cxx;
using namespace Adjacency_List_Matrix_Graph;

typedef unsigned char uchar;

namespace Treewidth_Namespace {

struct ChildFileInfo {
	uchar val;
	uint address;
};

struct BFHT_HDDD_FileInfo {
	ushort depth;
	uchar val;
	BFHT_HDDD_FileInfo(ushort d, uchar v) :
		depth(d), val(v) {
	}
	BFHT_HDDD_FileInfo(const BFHT_HDDD_Node &n);
};

inline BFHT_HDDD_FileInfo::BFHT_HDDD_FileInfo(const BFHT_HDDD_Node &n) :
	depth(n.nVertsElim()), val(0) {
	vector<int> elimverts;
	n.statePtr()->getEliminatedVertices(elimverts);
	for (uint i=0; i<elimverts.size(); i++) {
		int idx = elimverts[i]-1;
		if (idx >= 32)
			val ^= ((uchar)1<<(idx%8));
	}
}



typedef hash_set<BFHT_HDDD_Node*,BFHT_HDDD_NodeHash,BFHT_HDDD_NodeEq>
		NodePtrHT;
typedef NodePtrHT::const_iterator NodePtrHTci;
typedef NodePtrHT::iterator NodePtrHTi;

struct BFHT_HDDD_NodeBuffer {

	static const int BFHT_BUFFER_SIZE = ((int)DDD_MEMLIMIT_MB<<20)
			/(sizeof(BFHT_HDDD_Node)+4)*2/3/N_DDD_THREADS;

	// core data structures
	NodePtrHT ht;
	bool use_ht_elems;
	vector<NodePtrHTi> ht_elems;
	BFHT_HDDD_Node *buffer;

	// merge bookkeeping data
	BFHT_HDDD_Node *m_input_buff;
	uint m_input_buff_size;
	BFHT_HDDD_Node *m_unique_buff;
	uint m_unique_buff_size;
	BFHT_HDDD_Node *m_overflow_buff;
	uint m_overflow_buff_size;

	// expand bookkeeping data
	BFHT_HDDD_Node *e_input_buff;
	uint e_input_buff_size;
	BFHT_HDDD_Node *e_child_buffs;
	uint e_child_buffs_size;

	BFHT_HDDD_NodeBuffer();
	~BFHT_HDDD_NodeBuffer()
	{
		delete buffer;
	}

};


class BFHT_HDDD_NodeFiles {

/*	// constant parameters
	static const int BFHT_BUFFER_SIZE = ((int)DDD_MEMLIMIT_MB<<20)
			/(sizeof(BFHT_HDDD_Node)+4);

	// core data structures
	NodePtrHT ht;
	BFHT_HDDD_Node *buffer;//[BFHT_BUFFER_SIZE];

	// merge bookkeeping data
	BFHT_HDDD_Node *m_input_buff;
	uint m_input_buff_size;
	BFHT_HDDD_Node *m_unique_buff;
	uint m_unique_buff_size;
	BFHT_HDDD_Node *m_overflow_buff;
	uint m_overflow_buff_size;

	// expand bookkeeping data
	BFHT_HDDD_Node *e_input_buff;
	uint e_input_buff_size;
	BFHT_HDDD_Node *e_child_buffs;
	uint e_child_buffs_size;
*/
	// upper bound data
	uint ubv;
	BFHT_HDDD_Node ub_node;
	bool ub_node_valid;

	uint n_incomplete_files;

	uint curr_depth;
	set<uchar> *unexpanded_files;
	set<uchar> *generated_files;
	set<uchar> file_set1;
	set<uchar> file_set2;
	vector<BFHT_HDDD_FileInfo> expanded_files;

	static const uint MAX_FILENAME_SIZE = 1024;
	static const uint NUM_CHILD_FILES = 9;

	TWExitStatus expandNode(BFHT_HDDD_Node* n, const ALMGraph &orig_graph,
			ALMGraph &curr_graph, HeuristicVersion hversion,
			const BFHT_HDDD_FileInfo &expandFile,
			const vector<ChildFileInfo> &childFileInfoTable,
			vector<uint> &child_buff_currs, uint child_buff_size,
			BFHT_HDDD_NodeBuffer &nbuff, BreadthFirstTWStats &stats,
			pthread_mutex_t *main_mutex=NULL);

	void generate(const BFHT_HDDD_Node *parent, uint vid, uint deg,
			const BFHT_HDDD_FileInfo &expandFile,
			const vector<ChildFileInfo> &childFileInfoTable,
			vector<uint> &child_buff_currs, uint child_buff_size,
			BFHT_HDDD_NodeBuffer &nbuff, pthread_mutex_t *main_mutex=NULL);

	void flushChildBuffers(BFHT_HDDD_NodeBuffer &nbuff,
			const BFHT_HDDD_FileInfo &expandFile,
			const vector<ChildFileInfo> &childFileInfoTable,
			vector<uint> &child_buff_currs, uint child_buff_size,
			pthread_mutex_t *main_mutex=NULL);

	void static flushBuffer(BFHT_HDDD_Node *buffer, uint size,
			const char *dirname, const char *filename, bool append);

	bool childFileReadyToMerge(uchar child_val) const;

	void static getFilename(const BFHT_HDDD_FileInfo &fileInfo, char *dirname,
			char *filename);
	void static getChildFilename(const BFHT_HDDD_FileInfo &fileInfo,
			uchar childVal, char *dirname, char *filename);
	void static getOverflowFilename(const BFHT_HDDD_FileInfo &fileInfo,
			char *filename, int *threadid=NULL);
	void static getOverflowFilename2(const BFHT_HDDD_FileInfo &fileInfo,
			char *filename, int *threadid=NULL);
	uint static getChildFileIndex(const BFHT_HDDD_FileInfo &fileInfo,
			uint elim_vert);
	void static setupChildFileInfoTable(const BFHT_HDDD_FileInfo &fileInfo,
			vector<ChildFileInfo> &table, uint child_buffs_size);

	void init(BFHT_HDDD_Node &start, deque<BFHT_HDDD_FileInfo> &expandQueue,
			const ALMGraph &orig_graph, ALMGraph &curr_graph,
			HeuristicVersion hversion, int &ub, BFHT_HDDD_NodeBuffer &nbuff,
			BreadthFirstTWStats &stats);

	bool init(const char *dirname, deque<BFHT_HDDD_FileInfo> &expandQueue);

	void removeAllNodeFiles(const char *dirname) const;

public:
	BFHT_HDDD_NodeFiles (BFHT_HDDD_Node &start,
			deque<BFHT_HDDD_FileInfo> &expandQueue, const ALMGraph &orig_graph,
			ALMGraph &curr_graph, HeuristicVersion hversion, int ubval,
			BFHT_HDDD_NodeBuffer &nbuff, BreadthFirstTWStats &stats);

	uint merge(const BFHT_HDDD_FileInfo &mergeFile,
			BFHT_HDDD_NodeBuffer &nbuff, int *threadid=NULL) const;
	TWExitStatus expand(BFHT_HDDD_FileInfo &expandFile,
			const ALMGraph &orig_graph, ALMGraph &curr_graph,
			HeuristicVersion hversion, deque<BFHT_HDDD_FileInfo> &mergeQueue,
			BFHT_HDDD_NodeBuffer &nbuff, BreadthFirstTWStats &stats,
			pthread_mutex_t *main_mutex=NULL);
	void reconstructSolution(ElimOrder &soln, BFHT_HDDD_NodeBuffer &nbuff);
	void clear();
	uint nIncompleteFiles() const;
	void nextLevel();
	uint bestUB() const;
};

inline uint BFHT_HDDD_NodeFiles::nIncompleteFiles() const {
	return n_incomplete_files;
}

inline void BFHT_HDDD_NodeFiles::getFilename(
		const BFHT_HDDD_FileInfo &fileInfo, char *dirname, char *filename) {
	sprintf(dirname, "/d0/nodes/l%03i", fileInfo.depth);
	sprintf(filename, "/d0/nodes/l%03i/%03i_%i.nodes", fileInfo.depth,
			fileInfo.depth, fileInfo.val);
}

inline void BFHT_HDDD_NodeFiles::getOverflowFilename(
		const BFHT_HDDD_FileInfo &fileInfo, char *filename, int *threadid) {
	if (threadid==NULL)
		sprintf(filename, "/d0/%03i_%i_overflow.nodes", fileInfo.depth,
				fileInfo.val);
	else
		sprintf(filename, "/d0/%03i_%i_%i_overflow.nodes", fileInfo.depth,
				fileInfo.val, *threadid);
}

inline void BFHT_HDDD_NodeFiles::getOverflowFilename2(
		const BFHT_HDDD_FileInfo &fileInfo, char *filename, int *threadid) {
	if (threadid==NULL)
		sprintf(filename, "/d0/%03i_%i_overflow2.nodes", fileInfo.depth,
				fileInfo.val);
	else
		sprintf(filename, "/d0/%03i_%i_%i_overflow2.nodes", fileInfo.depth,
				fileInfo.val, *threadid);
}

inline void BFHT_HDDD_NodeFiles::getChildFilename(
		const BFHT_HDDD_FileInfo &fileInfo, uchar childVal, char *dirname,
		char *filename) {
	sprintf(dirname, "/d0/nodes/l%03i", fileInfo.depth+1);
	sprintf(filename, "/d0/nodes/l%03i/%03i_%i.nodes", fileInfo.depth+1,
			fileInfo.depth+1, childVal);
}

inline uint BFHT_HDDD_NodeFiles::getChildFileIndex(
		const BFHT_HDDD_FileInfo &fileInfo, uint elim_vert) {
	if (elim_vert<=32)
		return 0;
	return (elim_vert-1)%8 + 1;
}

inline void BFHT_HDDD_NodeFiles::setupChildFileInfoTable(
		const BFHT_HDDD_FileInfo &fileInfo, vector<ChildFileInfo> &table,
		uint child_buffs_size) {
	uint child_buff_size = child_buffs_size/NUM_CHILD_FILES;
	table.resize(NUM_CHILD_FILES);
	table[0].val = fileInfo.val;
	table[0].address = 0;
	for (uint i=1; i<NUM_CHILD_FILES; i++) {
		table[i].val = fileInfo.val ^ ((uchar)1<<(i-1));
		table[i].address = i*child_buff_size;
	}
}

inline uint BFHT_HDDD_NodeFiles::bestUB() const
{
	return ubv;
}

}

#endif /*BFHT_HDDD_NODEFILES_H_*/
