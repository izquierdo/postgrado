// ALMGraph.cpp - definition file for ALMGraph and associated classes
//
// ALMGraph is for adjacency list/matrix graph, because the graph is
// represented as both a matrx and a list. basically, the list elements
// exist at the corresponding location in the matrix.
//
// author: Alex Dow
// created on: 10/2/6

#include "preproc_flags.h"
#include <cassert>
#include <iostream> // only for testing
#include <iterator>
#include <map>
//#include <deque>
#include "ALMGraph.h"

#include "rdtsc.h"
#include <fstream>

namespace Adjacency_List_Matrix_Graph {

///////////////////////////////////////////////////////////////////////////////
// VertexDegList constructor - allocates and initializes member structures
///////////////////////////////////////////////////////////////////////////////
	VertexDegList::VertexDegList ( int maxverts, int maxdeg )
	 : vertices(maxverts+1,VertexDegLink()),
	   degLists(maxdeg+1,(VertexDegLink*)NULL),
	   mindeg(INT_MAX), empty(true)
	{
		for (int i=0; i<maxverts+1; i++)
			vertices[i].vid=i;
	}

///////////////////////////////////////////////////////////////////////////////
// VertexDegList addVertex - adds a vertex to the deg lists
// NOTE: function assumes vertex is not already in list, if it is then d.s.
//   will become corrupted
///////////////////////////////////////////////////////////////////////////////
	void VertexDegList::addVertex ( int vid, int deg )
	{
#ifdef DEBUG_ALMGRAPH
		assert(!vertices[vid].active);
#endif
		empty=false;
		// insert vid in proper list
		VertexDegLink *curr = degLists[deg];
		vertices[vid].active=true;
		vertices[vid].deg=deg;
		vertices[vid].prev=NULL;
		vertices[vid].next=curr;
		degLists[deg] = &(vertices[vid]);
		if (curr!=NULL)
			curr->prev = &(vertices[vid]);
		// update mindeg
		if (deg<mindeg)
			mindeg=deg;
	}

///////////////////////////////////////////////////////////////////////////////
// VertexDegList changeDeg - moves vertex from one degree list to another
// NOTE: function assumes vertex is already in a list, otherwise d.s. will
//   become corrupted
///////////////////////////////////////////////////////////////////////////////
	void VertexDegList::changeDeg ( int vid, int deg )
	{
#ifdef DEBUG_ALMGRAPH
		assert(vertices[vid].active);
#endif
		bool needNewMinDeg = false;
		// remove vert from current list
		if (vertices[vid].prev==NULL) { // first in list
			degLists[vertices[vid].deg] = vertices[vid].next;
			if (vertices[vid].next==NULL && vertices[vid].deg==mindeg) // actually, only one in mindeg list
				needNewMinDeg=true;
		} else // not first in list
			vertices[vid].prev->next = vertices[vid].next;
		if (vertices[vid].next!=NULL)
			vertices[vid].next->prev = vertices[vid].prev;
		// add vert to new list
		VertexDegLink *curr = degLists[deg];
		degLists[deg] = &vertices[vid];
		vertices[vid].prev = NULL;
		vertices[vid].next = curr;
		if (curr!=NULL)
			curr->prev = &vertices[vid];
		// update mindeg
		if (deg<mindeg) // mindeg is now smaller
			mindeg=deg;
		else if (needNewMinDeg) { // mindeg is now larger
			for (int i=vertices[vid].deg+1; i<(int)degLists.size(); i++) {
				if (degLists[i]!=NULL) {
					mindeg=i;
					break;
				}
			}
		}
		// update deg of vertex
		vertices[vid].deg=deg;
	}

	void VertexDegList::decDeg(int vid)
	{
#ifdef DEBUG_ALMGRAPH
		assert(vertices[vid].deg>=1);
#endif
		changeDeg(vid, vertices[vid].deg-1);
	}


///////////////////////////////////////////////////////////////////////////////
// VertexDegList removeVertex - removes a vertex from the degree lists
// NOTE: function assumes vertex is already in a list, otherwise d.s. will
//   become corrupted
///////////////////////////////////////////////////////////////////////////////
	void VertexDegList::removeVertex( int vid )
	{
#ifdef DEBUG_ALMGRAPH
		assert(vertices[vid].active);
#endif
		// de-activate vertex
		vertices[vid].active=false;
		// remove vert from current list
		if (vertices[vid].prev==NULL) { // first in list
			degLists[vertices[vid].deg] = vertices[vid].next;
			if (vertices[vid].next==NULL && vertices[vid].deg==mindeg) { // vid was only one with mindeg
				// search for new mindeg
				for (int i=mindeg+1; i<(int)degLists.size(); i++) {
					if (degLists[i]!=NULL) {
						mindeg=i;
						break;
					}
				}
				// if nothing found, list is empty
				if (vertices[vid].deg==mindeg) {
					empty=true;
					mindeg=INT_MAX;
				}
			}
		} else // not first in list
			vertices[vid].prev->next = vertices[vid].next;
		if (vertices[vid].next!=NULL) // not last in list
			vertices[vid].next->prev = vertices[vid].prev;
	}


///////////////////////////////////////////////////////////////////////////////
// VertexDegList getMinVert - returns id of a vertex with degree mindeg
// NOTE: function assumes list is not empty
///////////////////////////////////////////////////////////////////////////////
	int VertexDegList::getMinVert ()
	{
#ifdef DEBUG_ALMGRAPH
		assert(mindeg<degLists.size() && mindeg>=0);
#endif
#ifdef DEBUG_TW
		// debug mode: break ties with vid
		for (int v=1; v<vertices.size(); v++)
			if (vertices[v].active && vertices[v].deg==mindeg)
				return vertices[v].vid;
#else
		return degLists[mindeg]->vid;
#endif
	}

///////////////////////////////////////////////////////////////////////////////
// VertexDegList getMinVert_randomized - returns id of a vertex, chosen at
// random, with degree mindeg
// NOTE: function assumes list is not empty
///////////////////////////////////////////////////////////////////////////////
	inline int VertexDegList::getMinVert_randomized ()
	{
#ifdef DEBUG_ALMGRAPH
		assert(mindeg<degLists.size() && mindeg>=0);
#endif
#if N_DDD_THREADS > 1
		vector<int> minverts;
#else
		static vector<int> minverts;
#endif
		minverts.clear();
		minverts.reserve(degLists.size());
		VertexDegLink *curr = degLists[mindeg];
		while (curr!=NULL) {
			minverts.push_back(curr->vid);
			curr=curr->next;
		}
		int ri = (int)(((double)rand()/((double)RAND_MAX+1.0))*minverts.size());
		return minverts[ri];
	}

///////////////////////////////////////////////////////////////////////////////
// ElimOrder getOrder - returns an order represented by this ElimOrder, this
//   includes order_prefix followed by the vertices in indiff_suffix.
///////////////////////////////////////////////////////////////////////////////
	vector<int> ElimOrder::getOrder () const
	{
		vector<int> order;
		order.reserve(order_prefix.size()+indiff_suffix.size());
		order.insert(order.end(),order_prefix.begin(),order_prefix.end());
		order.insert(order.end(),indiff_suffix.begin(),indiff_suffix.end());
		return order;
	}

///////////////////////////////////////////////////////////////////////////////
// ElimOrder getOrder with extra prefix and vertex map - returns an order
//   represented by this ElimOrder after mapping the vertices with the given
//   vertex map and including the prefix parameter first in the order.
// NOTE: function assumes that vertexmap has a unique value for each vertex in
//   this order. also none of those values should be repeated in prefix.
///////////////////////////////////////////////////////////////////////////////
	vector<int> ElimOrder::getOrder (
		const vector<int> &prefix,
		const vector<int> &vertexmap) const
	{
#ifdef DEBUG_ALMGRAPH
		assert(order_prefix.size()+indiff_suffix.size()<=vertexmap.size() ||
			(prefix.size()==0 && vertexmap.size()==0));
#endif
		vector<int> order;
		order.reserve(prefix.size()+order_prefix.size()+indiff_suffix.size());
		order.insert(order.end(),prefix.begin(),prefix.end());
		if (vertexmap.empty()) {
			order.insert(order.end(),order_prefix.begin(),order_prefix.end());
			order.insert(order.end(),indiff_suffix.begin(),
				indiff_suffix.end());
		} else {
			for (int i=0; i<(int)order_prefix.size(); i++)
				order.push_back(vertexmap[order_prefix[i]-1]);
			set<int>::iterator suf = indiff_suffix.begin();
			while (suf!=indiff_suffix.end()) {
				order.push_back(vertexmap[*suf-1]);
				suf++;
			}
		}
		return order;
	}

///////////////////////////////////////////////////////////////////////////////
// ElimOrder remap - use the given vertexmap to map the ElimOrder's vertices.
// If vertexmap[i]=j, then change i in the ElimOrder to j
///////////////////////////////////////////////////////////////////////////////
	void ElimOrder::remap(const vector<int> &vertexmap) {
#ifdef DEBUG_ALMGRAPH
		assert(vertexmap.size()!=0);
#endif
		for (int i=0; i<(int)order_prefix.size(); i++)
			order_prefix[i] = vertexmap[order_prefix[i]-1];
		if (!indiff_suffix.empty()) {
			set<int> tmp_set = indiff_suffix;
			indiff_suffix.clear();
			set<int>::iterator iter = tmp_set.begin();
			while (iter!=tmp_set.end()) {
				indiff_suffix.insert(vertexmap[*iter-1]);
				iter++;
			}
		}
	}

///////////////////////////////////////////////////////////////////////////////
// ElimOrder appendPrefix - appends the given prefix to the front of the order
///////////////////////////////////////////////////////////////////////////////
	void ElimOrder::appendPrefix(const vector<int> &prefix) {
		order_prefix.insert(order_prefix.begin(),prefix.begin(),prefix.end());
	}

	string ElimOrder::printOrder() const {
		ostringstream ost;
		copy(order_prefix.begin(), order_prefix.end(), ostream_iterator<int>(ost," "));
		ost << " {";
		copy(indiff_suffix.begin(), indiff_suffix.end(), ostream_iterator<int>(ost, " "));
		ost << "}";
		return ost.str();
	}

///////////////////////////////////////////////////////////////////////////////
// addEdge - function adds an edge between vertices v and w. function assumes
//   that both vertices are active and that the edge is not already active, if
//   they aren't or it is d.s. will be corrupted.
// Optional parameters: cnt_mode, ub, exvid
// cnt_mode - if ==TRACK_CN and graph_cnt_mode==TRACK_CN then track
//   effects to common neighbor counts and cnlist.
// ub - used as upper bound if cnt_mode==TRACK_CN
// exvid - when updating common neighbor counts, ignore (exclude)
//   vertex exvid. That is, any common neighbor counts with this vertex will
//   not be updated.
///////////////////////////////////////////////////////////////////////////////
	inline void ALMGraph::addEdge (int v, int w,
			CommonNeighborTrackingMode cnt_mode, int ub, int exvid)
	{
#ifdef DEBUG_ALMGRAPH
		assert(vertices[v].active);
		assert(vertices[w].active);
		assert(!adjLM[v][w].active);
		assert(!adjLM[w][v].active);
#endif
		// update common neighbor tracking and pair list
		if (cnt_mode == TRACK_CN && graph_cnt_mode == TRACK_CN) {

			// each neighbor of v has one more cn with w
			VertexList *x = adjLM[v][0].next;
			while (x!=NULL) {
				if (x->vid != exvid) {
					if (x->vid < w) {
						int cn = ++commonNeighbors[x->vid][w];
						if (cn == ub && !adjLM[x->vid][w].active)
							cnlist.push_back(pair<int,int>(x->vid,w));

					} else if (x->vid > w) {
						int cn = ++commonNeighbors[w][x->vid];
						if (cn == ub && !adjLM[x->vid][w].active)
							cnlist.push_back(pair<int,int>(w,x->vid));

					}
				}
				x = x->next;
			}
			// each neighbor of w has one more cn with v
			x = adjLM[w][0].next;
			while (x!=NULL) {
				if (x->vid != exvid) {
					if (x->vid < v) {
						int cn = ++commonNeighbors[x->vid][v];
						if (cn == ub && !adjLM[x->vid][v].active)
							cnlist.push_back(pair<int,int>(x->vid,v));
					} else if (x->vid > v) {
						int cn = ++commonNeighbors[v][x->vid];
						if (cn == ub && !adjLM[x->vid][v].active)
							cnlist.push_back(pair<int,int>(v,x->vid));

					}
				}
				x = x->next;
			}
		}

		// add edges to adjLM lists
		adjLM[v][w].active = adjLM[w][v].active = true;
		VertexList *tmpV = adjLM[v][0].next;
		adjLM[v][0].next = &adjLM[v][w];
		adjLM[v][w].prev = &adjLM[v][0];
		adjLM[v][w].next = tmpV;
		if (tmpV!=NULL)
			tmpV->prev = &adjLM[v][w];
		tmpV = adjLM[w][0].next;
		adjLM[w][0].next = &adjLM[w][v];
		adjLM[w][v].prev = &adjLM[w][0];
		adjLM[w][v].next = tmpV;
		if (tmpV!=NULL)
			tmpV->prev = &adjLM[w][v];

		// increment vertex adjacency counts
		adjLM[v][0].vid++;
		adjLM[w][0].vid++;
	}

///////////////////////////////////////////////////////////////////////////////
// addDEdge - function adds an edge from vertex v to vertex w. function
//   assumes that both vertices are active and the edge is not already in the
//   graph, otherwise d.s. will be corrupted.
///////////////////////////////////////////////////////////////////////////////
	inline void ALMGraph::addDEdge (int v, int w)
	{
#ifdef DEBUG_ALMGRAPH
		assert(!adjLM[v][w].active);
		assert(vertices[v].active);
		assert(vertices[w].active);
#endif
		// add edge to adjLM list
		adjLM[v][w].active = true;
		VertexList *tmpV = adjLM[v][0].next;
		adjLM[v][0].next = &adjLM[v][w];
		adjLM[v][w].prev = &adjLM[v][0];
		adjLM[v][w].next = tmpV;
		if (tmpV!=NULL)
			tmpV->prev = &adjLM[v][w];
		// increment vertex adjacency count
		adjLM[v][0].vid++;
	}


///////////////////////////////////////////////////////////////////////////////
// addVertex_toVertexList - adds a vertex to the graph. function assumes that
//   vertex is not already in graph, if it is then d.s. will become corrupted.
///////////////////////////////////////////////////////////////////////////////
	inline void ALMGraph::addVertex_toVertexList (int v)
	{
#ifdef DEBUG_ALMGRAPH
		assert(!vertices[v].active);
#endif
		// add vertex to list
		vertices[v].active=true;
		VertexList *tmpV = vertices[0].next;
		vertices[0].next = &vertices[v];
		vertices[v].prev = &vertices[0];
		vertices[v].next = tmpV;
		if (tmpV!=NULL)
			tmpV->prev = &vertices[v];
		// increment vertex count
		vertices[0].vid++;
	}

///////////////////////////////////////////////////////////////////////////////
// addVertex_toSimpList - adds a vertex to the list of potential simplicial
//   and almost simplicial vertices. function assumes that vertex is not
//   already in list, if it is then d.s. will become corrupted.
///////////////////////////////////////////////////////////////////////////////
	inline void ALMGraph::addVertex_toSimpList (int v)
	{
#ifdef DEBUG_ALMGRAPH
		assert(!potentialSimpAndAlmostSimp[v].active);
#endif
		// add vertex to list
		potentialSimpAndAlmostSimp[v].active=true;
		VertexList *tmpV = potentialSimpAndAlmostSimp[0].next;
		potentialSimpAndAlmostSimp[0].next = &potentialSimpAndAlmostSimp[v];
		potentialSimpAndAlmostSimp[v].prev = &potentialSimpAndAlmostSimp[0];
		potentialSimpAndAlmostSimp[v].next = tmpV;
		if (tmpV!=NULL)
			tmpV->prev = &potentialSimpAndAlmostSimp[v];
		// increment vertex count
		potentialSimpAndAlmostSimp[0].vid++;
	}



///////////////////////////////////////////////////////////////////////////////
// removeEdge - remove edge between two vertices.
// NOTE: function assumes that the edge is currently active, if not,
//   data structure will become corrupted.
///////////////////////////////////////////////////////////////////////////////
	inline void ALMGraph::removeEdge (int v, int w)
	{
#ifdef DEBUG_ALMGRAPH
		assert(vertices[v].active);
		assert(vertices[w].active);
		assert(adjLM[v][w].active);
		assert(adjLM[w][v].active);
#endif
		adjLM[v][w].active = adjLM[w][v].active = false; // disable edge
		adjLM[v][0].vid--; // decrement degree of v
		adjLM[w][0].vid--; // decrement degree of w
		// remove links from lists
		adjLM[v][w].prev->next = adjLM[v][w].next;
		if (adjLM[v][w].next!=NULL)
			adjLM[v][w].next->prev = adjLM[v][w].prev;
		adjLM[w][v].prev->next = adjLM[w][v].next;
		if (adjLM[w][v].next!=NULL)
			adjLM[w][v].next->prev = adjLM[w][v].prev;
	}

///////////////////////////////////////////////////////////////////////////////
// removeDEdge - remove directed edge, that is, remove reference to edge (w,v)
//   as stored in row w of adjacency matrix, but not the reference in row v.
//   These graphs are actually designed and tested for undirected graphs, and
//   the only reason we don't remove the reference in row v is because vertex v
//   is about to be removed from graph all together.
// NOTE: function assumes that the edge is currently active, if not,
//   data structure will become corrupted.
///////////////////////////////////////////////////////////////////////////////
	inline void ALMGraph::removeDEdge (int w, int v)
	{
#ifdef DEBUG_ALMGRAPH
		assert(adjLM[w][v].active);
#endif
		adjLM[w][v].active=false; // disable edge
		adjLM[w][0].vid--; // decrement degree of w
		// remove link from list
		adjLM[w][v].prev->next = adjLM[w][v].next;
		if (adjLM[w][v].next!=NULL)
			adjLM[w][v].next->prev = adjLM[w][v].prev;
	}



///////////////////////////////////////////////////////////////////////////////
// removeVertex - removes the specified vertex from the vertices list
// NOTE: this function does not make any changes to adjLM, including edges
//   indicident to the removed vertex. also it does not remove the vertex from
//   the potentialSimpAndAlmostSimp vertex list.
// NOTE: function assumes that v is currently active, if not, data structure
//   will become corrupted.
///////////////////////////////////////////////////////////////////////////////
	inline void ALMGraph::removeVertex_fromVertexList( int v )
	{
#ifdef DEBUG_ALMGRAPH
		assert(vertices[v].active);
#endif
		vertices[0].vid--; // decrement no. of verts in graph
		vertices[v].active=false; // disable vertex
		// remove link from list
		vertices[v].prev->next = vertices[v].next;
		if (vertices[v].next!=NULL)
			vertices[v].next->prev = vertices[v].prev;
	}

///////////////////////////////////////////////////////////////////////////////
// removeVertex_fromSimpList - removes the specified vertex from the pSAAS list
// NOTE: function assumes that v is currently active, if not, data structure
//   will become corrupted.
///////////////////////////////////////////////////////////////////////////////
	inline void ALMGraph::removeVertex_fromSimpList ( int v )
	{
#ifdef DEBUG_ALMGRAPH
		assert(potentialSimpAndAlmostSimp[v].active);
#endif
		potentialSimpAndAlmostSimp[0].vid--; // decrement no. of verts in graph
		potentialSimpAndAlmostSimp[v].active=false; // disable vertex
		// remove link from list
		potentialSimpAndAlmostSimp[v].prev->next = potentialSimpAndAlmostSimp[v].next;
		if (potentialSimpAndAlmostSimp[v].next!=NULL)
			potentialSimpAndAlmostSimp[v].next->prev = potentialSimpAndAlmostSimp[v].prev;
	}



///////////////////////////////////////////////////////////////////////////////
// copyVertexList - copies the structure of src to dst so that they represent
//   the same list of vertices
// NOTE: function assumes src and dst are the same size
///////////////////////////////////////////////////////////////////////////////
	inline void ALMGraph::copyVertexList (const vector<VertexList> &src, vector<VertexList> &dst)
	{
#ifdef DEBUG_ALMGRAPH
		assert(src.size()==dst.size());
#endif
#if 1
		// more efficient to follow linked lists
//		if (src[0].vid+dst[0].vid < (int)src.size())
		{
			// de-activate active vertices in dst list
			clearVertexList(dst);
		//	VertexList *d_curr = dst[0].next;
		//	while (d_curr!=NULL) {
		//		d_curr->active=false;
		//		d_curr=d_curr->next;
		//	}
		//	d_curr=NULL;
			VertexList *d_prev = &dst[0];
			VertexList *s_curr = src[0].next;
			while (s_curr!=NULL) {
				// copy link
				VertexList *d_curr = &dst[s_curr->vid];
				d_curr->active=true;
				d_prev->next=d_curr;
				d_curr->prev=d_prev;
				// go to next link in list
				d_prev = d_curr;
				s_curr=s_curr->next;
			}
			d_prev->next=NULL;
			dst[0].vid = src[0].vid;
		// more efficient to iterate through entire vectors
		}
//		else
#endif
#if 0
		{
			VertexList *d_prev = &dst[0];
			for (int i=1; i<(int)src.size(); i++) {
				if (src[i].active) {
					dst[i].active=true;
					d_prev->next = &dst[i];
					dst[i].prev = d_prev;
					d_prev = &dst[i];
				} else
					dst[i].active=false;
			}
			d_prev->next = NULL;
			dst[0].vid = src[0].vid;
		}
#endif
	}

///////////////////////////////////////////////////////////////////////////////
// ALMGraph clearVertexList - clears the list.
// NOTE: function assumes that argument is a VALID vertex list. that is, a
// linked list beginning from element 0. All elements in the list have
// active=true, while all elements not in the list have active=false. If this
// is not the case, the d.s. will become corrupted.
///////////////////////////////////////////////////////////////////////////////
	inline void ALMGraph::clearVertexList (vector<VertexList> &vlist)
	{
		VertexList *curr = vlist[0].next;
		while (curr!=NULL) {
			curr->active=false;
			curr=curr->next;
		}
		vlist[0].next=NULL;
	}

///////////////////////////////////////////////////////////////////////////////
// ALMGraph getSimpState - determines whether the specified vertex is
//   simplicial, almost simplicial, or neither.
// NOTE: function assumes that vertex is in graph
///////////////////////////////////////////////////////////////////////////////
	simp_ret ALMGraph::getSimpState (int vid) const
	{
#ifdef DEBUG_ALMGRAPH
		assert(vertices[vid].active);
#endif
		simp_ret ret;
		ret.state=SIMP;
		ret.dvert=-1;
		int dvert1=-1, dvert2=-1; // vertices potentially disconnected from other neighbors
		// for each neighbor w of v
		VertexList *w = adjLM[vid][0].next;
		while (w!=NULL) {
			// check if w has already been determined to be disconnected from other neighbors
			if (dvert1==w->vid && dvert2==-1) {
				w = w->next;
				continue;
			}
			// for each neighbor x of v, that comes after w in list
			VertexList *x = w->next;
			while (x!=NULL) {
				// check if x has already been determined to be disconnected from other neighbors
				if (dvert1==x->vid && dvert2==-1) {
					x=x->next;
					continue;
				}
				// if w and x are not connected
				if (!adjLM[w->vid][x->vid].active) {
					if (ret.state==SIMP) { // first disconnected verts found, may still be ALSIMP if at least one is connected to all other neigbors
						ret.state=ALSIMP;
						dvert1=w->vid;
						dvert2=x->vid;
					} else { // some previously encountered vertices were disconnected
						if (dvert2==-1) { // discon vert was already found and it wasn't w or x
							ret.state=NONE;
							return ret;
						}
						if (dvert1==w->vid || dvert2==w->vid) { // w is definitely the discon vert, or state is NONE
							dvert1=w->vid;
							dvert2=-1;
							break;
						} else if (dvert1==x->vid || dvert2==x->vid) { // x is definitely the discon vert, or state is NONE
							dvert1=x->vid;
							dvert2=-1;
						} else { // neither potential discon vert is w or x
							ret.state=NONE;
							return ret;
						}
					}
				}
				x = x->next;
			}
			w = w->next;
		}
		ret.dvert = dvert1;
		return ret;
	}




///////////////////////////////////////////////////////////////////////////////
// ALMGraph elimVertexSimp - eliminates the specified simplicial vertex by
//   removing it and all indicental edges from the graph. No triangulation is
//   necessary because vertex is simplicial. If removed_edges is not NULL,
//   then the id of any vertex formerly adjacent to vid will be pushed back.
// NOTE: function assumes vertex is simplicial, if it is not then elimination
//   will not result in the correct vertex
///////////////////////////////////////////////////////////////////////////////
	void ALMGraph::elimVertexSimp (int vid, vector<int> *removed_edges)
	{
#ifdef DEBUG_ALMGRAPH
		assert(vertices[vid].active);
#endif
		// update common neighbor tracking for removed vertex
		// all neighbors of vid will have one less cn with each other
		if (graph_cnt_mode==TRACK_CN) {
			VertexList *w = adjLM[vid][0].next;
			while (w!=NULL) {
				VertexList *x = w->next;
				while (x!=NULL) {
					if (w->vid < x->vid)
						commonNeighbors[w->vid][x->vid]--;
					else
						commonNeighbors[x->vid][w->vid]--;
					x=x->next;
				}
				w=w->next;
			}
		}

		// now eliminate vertex
		VertexList *w = adjLM[vid][0].next;
		while (w!=NULL) {
			int wid = w->vid;
			// remove edge from w to v (v to w doesn't matter)
			removeDEdge(wid,vid);
			if (removed_edges!=NULL)
				removed_edges->push_back(wid);
			// add w and n(w) to simp/almost simp list
			if (!potentialSimpAndAlmostSimp[wid].active)
				addVertex_toSimpList(wid);
			VertexList *x = adjLM[wid][0].next;
			while (x!=NULL) {
				int xid = x->vid;
				if (!potentialSimpAndAlmostSimp[xid].active)
					addVertex_toSimpList(xid);
				x = x->next;
			}
			// get next vertex in n(v)
			w = w->next;
		}
		nVerts--;
		removeVertex_fromVertexList(vid);
		if (potentialSimpAndAlmostSimp[vid].active)
			removeVertex_fromSimpList(vid);
	}

///////////////////////////////////////////////////////////////////////////////
// ALMGraph elimVertexAlSimp - eliminates the specified almost simplicial
//   vertex by triangulating it, then removing it and all indicental edges
//   from the graph. Since vertex is almost simplicial it is triangulated by
//   connecting the single disconnected vertex (dvert) to all of its other
//   neighbors.
// If removed_edges is not NULL, then the id of any vertex formerly adjacent
//   to vid will be pushed back. If added_edges in not NULL, then a pair that
//   contains the two vertex ids for each added edge will be pushed back.
// NOTE: function assumes vid is almost simplicial and dvert is the vertex that
//   when connected to the other neighbors of vid will make it simplicial. If
//   not then the reduction will not produce the correct graph.
///////////////////////////////////////////////////////////////////////////////
	void ALMGraph::elimVertexAlSimp (int vid, int dvert,
			vector<int> *removed_edges, vector<pair<int,int> > *added_edges,
			int ub)
	{
		// update common neighbor tracking for removed vertex
		// all neighbors of vid will have one less cn with each other
		if (graph_cnt_mode==TRACK_CN) {
			VertexList *w = adjLM[vid][0].next;
			while (w!=NULL) {
				VertexList *x = w->next;
				while (x!=NULL) {
					if (w->vid < x->vid)
						commonNeighbors[w->vid][x->vid]--;
					else
						commonNeighbors[x->vid][w->vid]--;
					x=x->next;
				}
				w=w->next;
			}
		}

		// now eliminate vertex
		VertexList *w = adjLM[vid][0].next;
		while (w!=NULL) {
			int wid = w->vid;
			// remove edge from w to v (v to w doesn't matter)
			removeDEdge(wid,vid);
			if (removed_edges!=NULL)
				removed_edges->push_back(wid);
			// add w and n(w) to simp/almost simp list
			if (!potentialSimpAndAlmostSimp[wid].active)
				addVertex_toSimpList(wid);
			VertexList *x = adjLM[wid][0].next;
			while (x!=NULL) {
				int xid = x->vid;
				if (!potentialSimpAndAlmostSimp[xid].active)
					addVertex_toSimpList(xid);
				x = x->next;
			}
			// triangulate by connecting to dvert
			if (!adjLM[wid][dvert].active && wid!=dvert) {
				// now add the edge
				addEdge(wid,dvert,TRACK_CN,ub,vid);
				if (added_edges!=NULL)
					added_edges->push_back(pair<int,int>(wid,dvert));
			}
			// get next vertex in n(v)
			w = w->next;
		}
		nVerts--;
		removeVertex_fromVertexList(vid);
		if (potentialSimpAndAlmostSimp[vid].active)
			removeVertex_fromSimpList(vid);
	}


///////////////////////////////////////////////////////////////////////////////
// elimVertex_nopSAAS - 	eliminates the specified vertex by triangulating it,
//   then removing it and all indicental edges from the graph. This version
//   differs from elimVertex in that it does not make any changes to the
//   potentialSimpAndAlmostSimp list.
///////////////////////////////////////////////////////////////////////////////
	void ALMGraph::elimVertex_nopSAAS (int vid)
	{
#ifdef DEBUG_ALMGRAPH
		assert(vertices[vid].active);
#endif
		VertexList *w = adjLM[vid][0].next;
		while (w!=NULL) {
			int wid = w->vid;
			// remove edge from w to v (v to w doesn't matter)
			removeDEdge(wid,vid);
			// triangulate, i.e. connect w to all of n(v)
			// actually, only need to connect those that appear after w
			VertexList *x = w->next;
			while (x!=NULL) {
				int xid = x->vid;
				if (!adjLM[wid][xid].active) {
#ifdef DEBUG_ALMGRAPH
					assert(!adjLM[xid][wid].active);
#endif
					addEdge(wid,xid);
				}
				x = x->next;
			}
			// get next vertex in n(v)
			w = w->next;
		}
		nVerts--;
		removeVertex_fromVertexList(vid);
	}

///////////////////////////////////////////////////////////////////////////////
// ALMGraph findMinFillVert - finds a min-fill vertex, i.e. a vertex whose
//   elimination adds the min number of edges to the graph. If excluded is not
//   NULL, then omit any vertices in the set excluded points to.
///////////////////////////////////////////////////////////////////////////////
	int ALMGraph::findMinFillVert (const VertexSet *excluded) const
	{
		int minnfill=INT_MAX, minv;
		// for each vertex v in graph
		VertexList *v = vertices[0].next;
		while (v!=NULL) {
			if (excluded==NULL || !excluded->member(v->vid)) {
				int nfill=0;
				// for each pair of neighbors w,x of v
				VertexList *w = adjLM[v->vid][0].next;
				while (w!=NULL) {
					VertexList *x = w->next;
					while (x!=NULL) {
						if (!adjLM[w->vid][x->vid].active)
							nfill++;
						x=x->next;
					}
					w=w->next;
				}
				// update min
				if (nfill<minnfill) {
					minnfill=nfill;
					minv=v->vid;
					if (minnfill==0)
						break;
				}
			}
			v=v->next;
		}
		return minv;
	}

///////////////////////////////////////////////////////////////////////////////
// ALMGraph findMinFillVert_randomized - finds a min-fill vertex, i.e. a
//   vertex whose elimination adds the min number of edges to the graph. Ties
//   are broken at random.
///////////////////////////////////////////////////////////////////////////////
	int ALMGraph::findMinFillVert_randomized () const
	{
		int minnfill=INT_MAX;
		vector<int> minvs;
		minvs.reserve(vertices[0].vid);
		// for each vertex v in graph
		VertexList *v = vertices[0].next;
		while (v!=NULL) {
			int nfill=0;
			// for each pair of neighbors w,x of v
			VertexList *w = adjLM[v->vid][0].next;
			while (w!=NULL) {
				VertexList *x = w->next;
				while (x!=NULL) {
					if (!adjLM[w->vid][x->vid].active)
						nfill++;
					x=x->next;
				}
				w=w->next;
			}
			// update min
			if (nfill<minnfill) {
				minnfill=nfill;
				minvs.clear();
				minvs.push_back(v->vid);
			} else if (nfill==minnfill)
				minvs.push_back(v->vid);
			v=v->next;
		}
		// randomly choose one of minvs
			int ri = (int)(((double)rand()/((double)RAND_MAX+1.0))*minvs.size());
		return minvs[ri];
	}

///////////////////////////////////////////////////////////////////////////////
// ALMGraph findMinDegVert - finds a min degree vertex. If excluded is not
//   NULL, then omit any vertices in the set excluded points to.
///////////////////////////////////////////////////////////////////////////////
	int ALMGraph::findMinDegVert (const VertexSet *excluded) const
	{
		int minndeg=INT_MAX, minv;
		// for each vertex v in graph
		VertexList *v = vertices[0].next;
		while (v!=NULL) {
			if (excluded==NULL || !excluded->member(v->vid)) {
				int ndeg = adjLM[v->vid][0].vid;
				// update min
				if (ndeg<minndeg) {
					minndeg=ndeg;
					minv=v->vid;
					if (minndeg<=1)
						break;
				}
			}
			v=v->next;
		}
		return minv;
	}

///////////////////////////////////////////////////////////////////////////////
// ALMGraph findRandomVert - finds a random vertex. If excluded is not
//   NULL, then omit any vertices in the set excluded points to.
///////////////////////////////////////////////////////////////////////////////
	int ALMGraph::findRandomVert (const VertexSet *excluded) const
	{
		// number of included vertices
		int nv = vertices[0].vid;
		if (excluded!=NULL)
			nv -= excluded->size();
		// random index for vertex to return
		int ri = (int)(((double)rand()/((double)RAND_MAX+1.0))*nv);
		// step through included vertices to find (ri)th
		int ci = 0;
		VertexList *v = vertices[0].next;
		while (v!=NULL) {
			if (excluded==NULL || !excluded->member(v->vid)) {
				if (ci==ri)
					return v->vid;
				ci++;
			}
			v=v->next;
		}
		assert(false);
	}

///////////////////////////////////////////////////////////////////////////////
// ALMGraph findMinDegreeNeighbor returns the index of a neighbor of vertex
//   v that has the minimum degree. Input parameter mindeg is a hint on the
//   the lowest degree possible, thus if a neighbor is found with deg<=mindeg
//   the search can stop immediately.
// Complexity: O(degree of v)
///////////////////////////////////////////////////////////////////////////////
	int ALMGraph::findMinDegreeNeighbor ( int v, int mindeg )
	{
		int w;
		int w_mindeg=INT_MAX;
		VertexList *curr = adjLM[v][0].next;
		while (curr!=NULL) {
#ifdef DEBUG_TW
			// debug mode: break ties with vid
			if (adjLM[curr->vid][0].vid < w_mindeg || (adjLM[curr->vid][0].vid==w_mindeg && curr->vid<w)) {
#else
			if (adjLM[curr->vid][0].vid < w_mindeg) {
#endif
				w = curr->vid;
				w_mindeg = adjLM[w][0].vid;
#ifndef DEBUG_TW
				if (w_mindeg<=mindeg)
					break;
#endif
			}
			curr=curr->next;
		}
		return w;
	}

///////////////////////////////////////////////////////////////////////////////
// ALMGraph findMinDegreeNeighbor_randomized returns the index of a neighbor of
//   vertex v, chosen at random, that has the minimum degree.
// Complexity: O(degree of v)
///////////////////////////////////////////////////////////////////////////////
	int ALMGraph::findMinDegreeNeighbor_randomized ( int v )
	{
#if N_DDD_THREADS > 1
		vector<int> minverts;
#else
		static vector<int> minverts;
#endif
		minverts.clear();
		minverts.reserve(vertices.size());
		int mindeg=INT_MAX;
		VertexList *curr = adjLM[v][0].next;
		while (curr!=NULL) {
			if (adjLM[curr->vid][0].vid < mindeg) {
				minverts.clear();
				minverts.push_back(curr->vid);
				mindeg = adjLM[curr->vid][0].vid;
			} else if (adjLM[curr->vid][0].vid == mindeg)
				minverts.push_back(curr->vid);
			curr=curr->next;
		}
		int ri = (int)(((double)rand()/((double)RAND_MAX+1.0))*minverts.size());
		return minverts[ri];
	}

///////////////////////////////////////////////////////////////////////////////
// ALMGraph findLeastCommonNeighbor returns the index of a neighbor of vertex
//   v that has the least number of neighbors in common with v.
// Complexity: O(degree of v * min(degree of v, degree of max degree neighbor))
///////////////////////////////////////////////////////////////////////////////
	int ALMGraph::findLeastCommonNeighbor (int v)
	{
		int w;
		int w_ncommon=INT_MAX;
		VertexList *curr = adjLM[v][0].next;
		while (curr!=NULL) {
			int ncommon=0;
			if (adjLM[v][0].vid <= adjLM[curr->vid][0].vid) {
				VertexList *curr2 = adjLM[v][0].next;
				while (curr2!=NULL) {
					if (curr2->vid!=curr->vid && adjLM[curr->vid][curr2->vid].active)
						ncommon++;
					curr2=curr2->next;
				}
			} else {
				VertexList *curr2 = adjLM[curr->vid][0].next;
				while (curr2!=NULL) {
					if (curr2->vid!=v && adjLM[v][curr2->vid].active)
						ncommon++;
					curr2=curr2->next;
				}
			}
#ifdef DEBUG_TW
			// debug mode: break ties with vid
			if (ncommon<w_ncommon || (ncommon==w_ncommon && curr->vid<w)) {
#else
			if (ncommon<w_ncommon) {
#endif
				w = curr->vid;
				w_ncommon = ncommon;
#ifndef DEBUG_TW
				if (ncommon==0)
					break;
#endif
			}
			curr=curr->next;
		}
		return w;
	}

///////////////////////////////////////////////////////////////////////////////
// ALMGraph findLeastCommonNeighbor_randomized returns the index of a neighbor
//   of vertex v, chosen at random, that has the least number of neighbors in
//   common with v.
// Complexity: O(degree of v * min(degree of v, degree of max degree neighbor))
///////////////////////////////////////////////////////////////////////////////
	int ALMGraph::findLeastCommonNeighbor_randomized (int v)
	{
#if N_DDD_THREADS > 1
		vector<int> minverts;
#else
		static vector<int> minverts;
#endif
		minverts.clear();
		minverts.reserve(vertices.size());
		int mincommon=INT_MAX;
		VertexList *curr = adjLM[v][0].next;
		while (curr!=NULL) {
			int ncommon=0;
			if (adjLM[v][0].vid <= adjLM[curr->vid][0].vid) {
				VertexList *curr2 = adjLM[v][0].next;
				while (curr2!=NULL) {
					if (curr2->vid!=curr->vid && adjLM[curr->vid][curr2->vid].active)
						ncommon++;
					curr2=curr2->next;
				}
			} else {
				VertexList *curr2 = adjLM[curr->vid][0].next;
				while (curr2!=NULL) {
					if (curr2->vid!=v && adjLM[v][curr2->vid].active)
						ncommon++;
					curr2=curr2->next;
				}
			}
			if (ncommon<mincommon) {
				minverts.clear();
				minverts.push_back(curr->vid);
				mincommon = ncommon;
			} else if (ncommon==mincommon)
				minverts.push_back(curr->vid);
			curr=curr->next;
		}
		int ri = (int)(((double)rand()/((double)RAND_MAX+1.0))*minverts.size());
		return minverts[ri];
	}

///////////////////////////////////////////////////////////////////////////////
// countCommonNeighbors - returns the number of vertices adjacent to both vid
// and wid.
// NOTE: function assumes that both vid and wid are active vertices, if they
// are not, then the returned value will be invalid.
///////////////////////////////////////////////////////////////////////////////
	int ALMGraph::countCommonNeighbors(int vid, int wid) const
	{
		int ncommon = 0;

		// iterate through lower degree vertex
		int aid = vid;
		int bid = wid;
		if (adjLM[vid][0].vid > adjLM[wid][0].vid) {
			aid = wid;
			bid = vid;
		}

		// count common vertices
		VertexList *c = adjLM[aid][0].next;
		while (c!=NULL) {
			if (adjLM[bid][c->vid].active)
				ncommon++;
			c=c->next;
		}

		return ncommon;
	}

///////////////////////////////////////////////////////////////////////////////
// load - creates new ALMGraph from an adjacency matrix
// Input:
//   vector<vector<bool>> &matrix - adjacency matrix where all vertices have
//     at least one edge
// NOTE: member function used instead of constructor because ptrs in ds make
//   it difficult to re-construct with assignment, i.e. g = ALMGraph(adjmat);
///////////////////////////////////////////////////////////////////////////////
	void ALMGraph::load (const boolMatrix &matrix)
	{
		nVerts = matrix.size();
		// initialize adjLM, vertices, and pSAAS lists
		init(nVerts);
		// build the graph
		for (int i=0; i<nVerts; i++) {
			int v=i+1;
			for (int j=i+1; j<nVerts; j++) {
				int w=j+1;
				if (matrix[i][j]) {
					// add vertices to graph, if necessary
					if (!vertices[v].active) {
						addVertex_toVertexList(v);
						clearVertexList(adjLM[v]);
					}
					if (!vertices[w].active) {
						addVertex_toVertexList(w);
						clearVertexList(adjLM[w]);
					}
					// add edge to graph
					addEdge(v,w,TRACK_CN);
					// update pSAAS tracking
					if (!potentialSimpAndAlmostSimp[v].active)
						addVertex_toSimpList(v);
					if (!potentialSimpAndAlmostSimp[w].active)
						addVertex_toSimpList(w);
				}
			}
		}
		nVerts = vertices[0].vid;

#ifdef DEBUG_ALMGRAPH
		if (graph_cnt_mode==TRACK_CN) {
			VertexList *v = vertices[0].next;
			while (v!=NULL) {
				VertexList *w = v->next;
				while (w!=NULL) {
					int cn = countCommonNeighbors(v->vid,w->vid);
					//cout << "COMMON NEIGHBORS (" << v->vid << "," << w->vid << ") "
					//	<< commonNeighbors[v->vid][w->vid] << "  "
					//	<< commonNeighbors[w->vid][v->vid] << "  cn: " << cn << endl;
					if (v->vid < w->vid)
						assert(commonNeighbors[v->vid][w->vid]==cn);
					else
						assert(commonNeighbors[w->vid][v->vid]==cn);
					w=w->next;
				}
				v=v->next;
			}
		}
#endif
	}


///////////////////////////////////////////////////////////////////////////////
// init_nopSAAS - allocates and initializes adjLM and vertices data
//   structures
///////////////////////////////////////////////////////////////////////////////
	void ALMGraph::init (int origVerts)
	{
		// init adjLM and vertices
		init_nopSAAS(origVerts);
		// init pSAAS
		potentialSimpAndAlmostSimp = vector<VertexList> (origVerts+1,VertexList());
		potentialSimpAndAlmostSimp[0].vid=0; // counts no. of active verts in list
		for (int v=1; v<=origVerts; v++) {
			vertices[v].vid=v;
			potentialSimpAndAlmostSimp[v].vid=v;
			adjLM[v][0].vid=0; // counts no. of active verts in list
			for (int w=1; w<=origVerts; w++)
				adjLM[v][w].vid=w;
		}
		// init common neighbor tracking
		if (graph_cnt_mode==TRACK_CN) {
			commonNeighbors = vector<vector<int> > (origVerts+1,
					vector<int>(origVerts+1, 0));
			cnlist.clear();
		}
	}

///////////////////////////////////////////////////////////////////////////////
// init_nopSAAS - allocates and initializes adjLM and vertices data
//   structures
///////////////////////////////////////////////////////////////////////////////
	void ALMGraph::init_nopSAAS (int origVerts)
	{
		adjLM = vector<vector<VertexList> > (origVerts+1,vector<VertexList>(origVerts+1,VertexList()));
		vertices = vector<VertexList> (origVerts+1,VertexList());
		vertices[0].vid=0; // counts no. of active verts in list
		for (int v=1; v<=origVerts; v++) {
			vertices[v].vid=v;
			adjLM[v][0].vid=0; // counts no. of active verts in list
			for (int w=1; w<=origVerts; w++)
				adjLM[v][w].vid=w;
		}
	}


///////////////////////////////////////////////////////////////////////////////
// copy - makes this graph into a copy of the argument graph without
//   reallocating any internal data structures.
// NOTE: function assumes both graphs have data structures of the same size
///////////////////////////////////////////////////////////////////////////////
	void ALMGraph::copy ( const ALMGraph &orig )
	{
		// copy vertices and adjLM
		copy_nopSAAS(orig);
		// copy pSAAS
		copyVertexList(orig.potentialSimpAndAlmostSimp,potentialSimpAndAlmostSimp);
		// copy commonNeighors
		if (graph_cnt_mode==TRACK_CN) {
			VertexList *v = orig.vertices[0].next;
			while (v!=NULL) {
				VertexList *w = orig.adjLM[v->vid][0].next;
				while (w!=NULL) {
					if (v->vid < w->vid)
						commonNeighbors[v->vid][w->vid] = orig.commonNeighbors[v->vid][w->vid];
					else
						commonNeighbors[w->vid][v->vid] = orig.commonNeighbors[w->vid][v->vid];
					w=w->next;
				}
				v=v->next;
			}
			cnlist = orig.cnlist;
		}
	}

///////////////////////////////////////////////////////////////////////////////
// copy_nopSAAS - same as copy except potentialSimpAndAlmostSimp list is not
//   copied.
// NOTE: function assumes both graphs have data structures of the same size
///////////////////////////////////////////////////////////////////////////////
	void ALMGraph::copy_nopSAAS ( const ALMGraph &orig )
	{
#ifdef DEBUG_ALMGRAPH
		assert(adjLM.size()==orig.adjLM.size());
		assert(vertices.size()==orig.vertices.size());
#endif
		nVerts=orig.nVerts;
		// copy vertices
		copyVertexList(orig.vertices,vertices);
		// copy adjLM
		VertexList *curr = orig.vertices[0].next;
		while (curr!=NULL) {
			copyVertexList(orig.adjLM[curr->vid],adjLM[curr->vid]);
			curr=curr->next;
		}
	}



///////////////////////////////////////////////////////////////////////////////
// normalize - removes any vertices with no incident edges from graph, this
//   may require a change in vertex ids, where the mapping used will be set in
//   the vertex_map input parameter.
// Input:
//  - vector<int> &disconVerts -
//     postcondition: vertices that were found to be disconnected and were
//       therefore removed from the graph are appended to the end of this
//       vector.
//  - vector<int> &vertex_map -
//     postcondition: contains mapping of new vids to old vids, if an old vid
//       doesn't appear that's because the vertex was disconnected and
//       therefore removed
//     if input vector is not empty, must contain an older mapping used to get
//       the current graph, and therefore must contain an element for every
//       vertex in current graph. postcondition is that it will contain mapping
//       from new vids to vids of original graph.
//     format: map is 0-indexed, so if vertex_map[i]=j that means that vertex
//       with vid i+1 in new graph maps to vertex with vid j in old graph
//  NOTE: ths is a SLOW operation because it may require reallocating
//  significant internal data structures and there is probably a better way to
//  do it.
///////////////////////////////////////////////////////////////////////////////
	void ALMGraph::normalize_SLOW ( vector<int> &disconVerts, vector<int> &vertex_map )
	{
#ifdef DEBUG_ALMGRAPH
		assert(vertex_map.empty() || vertex_map.size()==nVerts); // input map is valid
#endif
		// check if normalization is necessary, it not, return
		if ((int)vertices.size()-1==nVerts)
			return;
		// get new map -keep verts in same order as before
		vector<int> new_map;
		for (int v=1; v<(int)vertices.size(); v++) {
			if (vertices[v].active && adjLM[v][0].vid>0)
				new_map.push_back(v);
			else if (adjLM[v][0].vid==0)
				disconVerts.push_back(v);
		}
		// make copies of old graph data structures
		vector<vector<VertexList> > old_adjLM = adjLM;
		vector<VertexList> old_vertices = vertices;
		vector<VertexList> old_pSAAS = potentialSimpAndAlmostSimp;
		// re-initialize primary graph data structures
		nVerts = new_map.size();
		adjLM = vector<vector<VertexList> >(nVerts+1,vector<VertexList>(nVerts+1,VertexList()));
		vertices =vector<VertexList>(nVerts+1,VertexList());
		potentialSimpAndAlmostSimp = vector<VertexList>(nVerts+1,VertexList());
		vertices[0].vid=0; // counts no. of active verts in list
		potentialSimpAndAlmostSimp[0].vid=0; // counts no. of active verts in list
		for (int v=1; v<=nVerts; v++) {
			vertices[v].vid=v;
			potentialSimpAndAlmostSimp[v].vid=v;
			adjLM[v][0].vid=0; // counts no. of active verts in list
			for (int w=1; w<=nVerts; w++)
				adjLM[v][w].vid=w;
		}
		if (graph_cnt_mode==TRACK_CN)
			commonNeighbors = vector<vector<int> >(nVerts+1, vector<int>(nVerts+1, 0));
		// fill graph data structures
		for (int i=0; i<nVerts; i++) {
			int v=i+1;
			for (int j=i+1; j<nVerts; j++) {
				int w=j+1;
				if (old_adjLM[new_map[i]][new_map[j]].active) {
					if (!vertices[v].active) {
						addVertex_toVertexList(v);
						clearVertexList(adjLM[v]);
					}
					if (!vertices[w].active) {
						addVertex_toVertexList(w);
						clearVertexList(adjLM[w]);
					}
					addEdge(v,w);
				}
			}
			if (graph_cnt_mode==TRACK_CN) {
				for (int i=0; i<nVerts; i++)
					for (int j=i+1; j<nVerts; j++)
						commonNeighbors[i+1][j+1] =
							countCommonNeighbors(i+1,j+1);
			}
			if (old_pSAAS[new_map[i]].active)
				addVertex_toSimpList(v);
		}
#ifdef DEBUG_ALMGRAPH
		assert(vertices[0].vid==nVerts); // all verts have at least one edge
#endif
		// set vertex_map
		if (vertex_map.empty()) {
			vertex_map = new_map;
		} else {
			vector<int> final_map;
			for (int i=0; i<(int)new_map.size(); i++)
				final_map.push_back(vertex_map[new_map[i]]);
			vertex_map = final_map;
		}
	}



	void ALMGraph::mapVertices(MapVerticesMethod method, vector<int> &map)
	{
		// find vertex order
		assert((uint)nVerts == vertices.size()-1);
		vector<int> newmap(vertices.size()-1);
		vector<int> rmap(vertices.size()-1);
		if (method == MAPVERTS_NONE)
		{
			return;
		}
		else if (method == MAPVERTS_MINDEGREE_FIRST or method == MAPVERTS_MAXDEGREE_FIRST)
		{
			multimap<int,int> degvertmap;
			for (uint v=1; v<vertices.size(); ++v)
				degvertmap.insert(pair<int,int>(adjLM[v][0].vid, v));
			int newv = (method==MAPVERTS_MINDEGREE_FIRST) ? 1 : vertices.size()-1;
			multimap<int,int>::iterator iter;
			for (iter = degvertmap.begin(); iter != degvertmap.end(); ++iter)
			{
				int oldv = iter->second;
				assert(oldv>0 && oldv<=(int)vertices.size());
				assert(newv>0 && newv<=(int)vertices.size());
				newmap[newv-1] = oldv;
				rmap[oldv-1] = newv;
				newv += (method==MAPVERTS_MINDEGREE_FIRST) ? 1 : -1;
			}
		}
		else
			assert(false);

		// make a copy of the graph
		ALMGraph g;
		g.init(vertices.size()-1);
		g.copy(*this);

		// clear all edges from graph
		for (uint v=1; v<vertices.size(); v++)
		{
			clearVertexList(adjLM[v]);
			adjLM[v][0] = 0;
		}

		// add edges between mapped vertices
		for (uint oldv=1; oldv<vertices.size(); oldv++)
		{
			for (uint oldw=oldv+1; oldw<vertices.size(); oldw++)
			{
				if (g.adjLM[oldv][oldw].active)
				{
					addEdge(rmap[oldv-1], rmap[oldw-1]);
				}
			}
		}

		assert(potentialSimpAndAlmostSimp[0].vid==0); // otherwise need to map this took

		// set vertex_map
		if (map.empty()) {
			map = newmap;
		} else {
			vector<int> finalmap;
			for (int i=0; i<(int)newmap.size(); i++)
				finalmap.push_back(map[newmap[i]-1]);
			map = finalmap;
		}
	}

///////////////////////////////////////////////////////////////////////////////
// elimVertex - 	eliminated the specified vertex by triangulating it, then
//   removing it and all indicental edges from the graph. Also adds all
//   vertices within two hops of eliminated vertex to pSAAS list.
// Inputs:
//   vid - vertex to eliminate
//   removed_edges - [optional] set to list of former neighbors of vid
//   added_edges - [optional] set to list of edges added in elimination
// NOTE: eliminating a vertex does not cause the corresponding row in the adj
// matrix to be clear. instead the vertex is removed from the vertices list
// and the adj mat row is kept the same. if the vertex is later added back into
// the graph, then its row can be reset first
///////////////////////////////////////////////////////////////////////////////
	void ALMGraph::elimVertex (int vid, vector<int> *removed_edges,
		vector<pair<int,int> > *added_edges, int ub, bool *removed_pSAAS,
		vector<int> *added_pSAAS)
	{
#ifdef DEBUG_ALMGRAPH
		assert(vertices[vid].active);
#endif
		// update common neighbor tracking for removed vertex
		// all neighbors of vid will have one less cn with each other
		if (graph_cnt_mode==TRACK_CN) {
			VertexList *w = adjLM[vid][0].next;
			while (w!=NULL) {
				VertexList *x = w->next;
				while (x!=NULL) {
					if (w->vid < x->vid)
						commonNeighbors[w->vid][x->vid]--;
					else
						commonNeighbors[x->vid][w->vid]--;
					x=x->next;
				}
				w=w->next;
			}
		}

		// now eliminate vertex
		VertexList *w = adjLM[vid][0].next;
		while (w!=NULL) {
			int wid = w->vid;
			// remove edge from w to v (v to w doesn't matter)
			removeDEdge(wid,vid);
			if (removed_edges!=NULL)
				removed_edges->push_back(wid);
			// add w and n(w) to simp/almost simp list
			if (!potentialSimpAndAlmostSimp[wid].active)
			{
				addVertex_toSimpList(wid);
				if (added_pSAAS != NULL)
					added_pSAAS->push_back(wid);
			}
			VertexList *x = adjLM[wid][0].next;
			while (x!=NULL)
			{
				int xid = x->vid;
				if (!potentialSimpAndAlmostSimp[xid].active)
				{
					addVertex_toSimpList(xid);
					if (added_pSAAS != NULL)
						added_pSAAS->push_back(xid);
				}
				x = x->next;
			}
			// triangulate, i.e. connect w to all of n(v)
			// actually, only need to connect those that appear after w
			x = w->next;
			while (x!=NULL) {
				int xid = x->vid;
				if (!adjLM[wid][xid].active) {
#ifdef DEBUG_ALMGRAPH
					assert(!adjLM[xid][wid].active);
#endif
					addEdge(wid,xid,TRACK_CN,ub,vid);
					if (added_edges!=NULL)
						added_edges->push_back(pair<int,int>(wid,xid));
				}
				x = x->next;
			}
			// get next vertex in n(v)
			w = w->next;
		}
		nVerts--;
		removeVertex_fromVertexList(vid);
		if (potentialSimpAndAlmostSimp[vid].active)
		{
			removeVertex_fromSimpList(vid);
			if (removed_pSAAS!=NULL)
				*removed_pSAAS = true;
		}
	}

/*
///////////////////////////////////////////////////////////////////////////////
// ALMGraph elimVertices - eliminates all vertices from this graph that are
//   marked as eliminated in the given State. If lastv is not NULL, then
//   vertex *lastv will be the last one eliminated, and last_removed_edges
//   will be filled the vertex ids of all the vertices that had edges removed
//   when *lastv was eliminated (note: these are the vertices formerly
//   adjacent to *lastv).
// NOTE: function assumes that graph has a vertex for each vertex represented
//   in the state. function also assumes that, if lastv is a valid vid, then
//   it is in the graph and the state.
///////////////////////////////////////////////////////////////////////////////
	void ALMGraph::elimVertices_new (TWState s, const int *lastv,
			vector<int> *last_removed_edges, const int *pv,
			vector<int> *p_removed_edges)
	{
#ifdef DEBUG_ALMGRAPH
		assert(nVerts==TWState::nVerts());
		assert(lastv==NULL || *lastv<1 || *lastv>=vertices.size() || s.checkVert(*lastv));
		assert(lastv==NULL || *lastv<1 || *lastv>=vertices.size() || vertices[*lastv].active);
#endif
#if N_DDD_THREADS > 1
		vector<int> verts(vertices.size()-1);
#else
		static vector<int> verts(vertices.size()-1);
#endif

		//cout << "  elim vertices\n";
		if (lastv!=NULL && *lastv>=1 && *lastv<vertices.size())
		{
			s.clearVert(*lastv);
			if (pv!=NULL && *pv>=1 && *pv<vertices.size())
				s.clearVert(*pv);
		}
		s.getEliminatedVertices(verts);
		vector<bool> eliminated(vertices.size(),false);
		for (int i=0; i<verts.size(); i++)
		{
			if (!eliminated[verts[i]])
			{
				eliminated[verts[i]] = true;
				vector<int> incomponent;
				incomponent.reserve(vertices.size()-1);
				incomponent.push_back(verts[i]);
				vector<int> toconnect;
				vector<bool> toconnect_added(vertices.size(),false);
				deque<int> tocheck;
				tocheck.push_back(verts[i]);
				while (!tocheck.empty())
				{
					int vid = tocheck.front();
					tocheck.pop_front();
					VertexList *w = adjLM[vid][0].next;
					while (w!=NULL)
					{
						bool instate = s.checkVert(w->vid);
						if (instate && !eliminated[w->vid])
						{
							// w is a new vertex in this component
							eliminated[w->vid] = true;
							incomponent.push_back(w->vid);
							tocheck.push_back(w->vid);
						}
						else if (!instate)
						{
							// w is adjacent to this component
							if (!toconnect_added[w->vid])
							{
								toconnect.push_back(w->vid);
								toconnect_added[w->vid] = true;
							}
							// remove edge from w to v
							removeDEdge(w->vid, vid);
						}
						w = w->next;
					}
				}
				// remove incomponent vertices, already disconnected
				for (int i=0; i<incomponent.size(); i++)
				{
					removeVertex_fromVertexList(incomponent[i]);
					if (potentialSimpAndAlmostSimp[incomponent[i]].active)
						removeVertex_fromSimpList(incomponent[i]);
					//cout << "    elim " << incomponent[i] << " " << potentialSimpAndAlmostSimp[incomponent[i]].active << endl;
				}
				// connect toconnect vertices
				for (int i=0; i<toconnect.size(); i++)
				{
					for (int j=i+1; j<toconnect.size(); j++)
						if (!adjLM[toconnect[i]][toconnect[j]].active)
							addEdge(toconnect[i], toconnect[j]);
					// add vert and neighbors to simp/almost simp list
					if (!potentialSimpAndAlmostSimp[toconnect[i]].active)
						addVertex_toSimpList(toconnect[i]);
					VertexList *x = adjLM[toconnect[i]][0].next;
					while (x!=NULL) {
						int xid = x->vid;
						if (!potentialSimpAndAlmostSimp[xid].active)
							addVertex_toSimpList(xid);
						x = x->next;
					}
				}
			}
		}
		if (lastv!=NULL && *lastv>=1 && *lastv<vertices.size())
		{
			if (pv!=NULL && *pv>=1 && *pv<vertices.size())
				elimVertex(*pv, p_removed_edges);
			elimVertex(*lastv, last_removed_edges);
		}
////old implementation
//		s.getEliminatedVertices(verts);
//		if (lastv==NULL || *lastv<1 || *lastv>=vertices.size()) {
//			for (int i=0; i<(int)verts.size(); i++)
//				elimVertex(verts[i]);
//		} else {
//			for (int i=0; i<(int)verts.size(); i++)
//				if (verts[i]!=*lastv && (pv==NULL || verts[i]!=*pv))
//					elimVertex(verts[i]);
//			if (pv!=NULL && *pv>=1 && *pv<vertices.size())
//				elimVertex(*pv, p_removed_edges);
//			elimVertex(*lastv, last_removed_edges);
//		}
//
	}
*/
	void ALMGraph::elimVertices (TWState s, const int *lastv,
			vector<int> *last_removed_edges, const int *pv,
			vector<int> *p_removed_edges)
	{
#ifdef DEBUG_ALMGRAPH
		assert(nVerts==TWState::nVerts());
		assert(lastv==NULL || *lastv<1 || *lastv>=vertices.size() || s.checkVert(*lastv));
		assert(lastv==NULL || *lastv<1 || *lastv>=vertices.size() || vertices[*lastv].active);
#endif
#if N_DDD_THREADS > 1
		vector<int> verts;
#else
		static vector<int> verts;
#endif
		verts.reserve(vertices.size() - 1);
		s.getEliminatedVertices(verts);
		if (lastv==NULL || *lastv<1 || *lastv>=(int)vertices.size()) {
			for (int i=0; i<(int)verts.size(); i++)
				elimVertex(verts[i]);
		} else {
			for (int i=0; i<(int)verts.size(); i++)
				if (verts[i]!=*lastv && (pv==NULL || verts[i]!=*pv))
					elimVertex(verts[i]);
			if (pv!=NULL && *pv>=1 && *pv<(int)vertices.size())
				elimVertex(*pv, p_removed_edges);
			elimVertex(*lastv, last_removed_edges);
		}
	}

	void ALMGraph::elimVertices(const BFHT_HDDD_State &s,
			const int *lastv, vector<int> *last_removed_edges)
	{
#if N_DDD_THREADS > 1
		vector<int> verts;
#else
		static vector<int> verts;
#endif
		verts.reserve(vertices.size() - 1);
		s.getEliminatedVertices(verts);
		if (lastv==NULL || *lastv<1 || *lastv>=(int)vertices.size()) {
			for (int i=0; i<(int)verts.size(); i++)
				elimVertex(verts[i]);
		} else {
			for (int i=0; i<(int)verts.size(); i++)
				if (verts[i]!=*lastv)
					elimVertex(verts[i]);
			elimVertex(*lastv, last_removed_edges);
		}
	}

/*
///////////////////////////////////////////////////////////////////////////////
// ALMGraph elimVertices_nopSAAS - eliminates all vertices from this graph that
//   are marked as eliminated in the given TWState, but does not add any
//   vertices (or make any changes) to the pSAAS list (this included removing
//   eliminated vertices).
// NOTE: function assumes that graph has a vertex for each vertex represented
//   in the state.
///////////////////////////////////////////////////////////////////////////////
	void ALMGraph::elimVertices_nopSAAS (const TWState &s)
	{
#ifdef DEBUG_ALMGRAPH
		assert(nVerts==TWState::nVerts());
#endif
#if N_DDD_THREADS > 1
		vector<int> verts(vertices.size()-1);
#else
		static vector<int> verts(vertices.size()-1);
#endif
		s.getEliminatedVertices(verts);
		for (int i=0; i<(int)verts.size(); i++)
			elimVertex_nopSAAS(verts[i]);
	//	unsigned int ii=0, bi=0;
	//	for (int i=0; i<TWState::nVerts; i++) {
	//		if (s.bits[ii]&(1U<<bi))
	//			elimVertex_nopSAAS(i+1);
	//		if (++bi>=TWState::indexMod) {
	//			bi=0;
	//			ii++;
	//		}
	//	}
	}
*/
///////////////////////////////////////////////////////////////////////////////
// ALMGraph reverseElimVertex - reverses the elimination of vertex vid from
//   the graph, assuming removed_edges stores all of vid's former neighbors
//   and added_edges stores all of the edges added to the graph
// NOTE: function assumes that vid's row in adjLM is the same as it was prior
// to elimination. That is, still contains a valid linked list of the vertices
// in removed_edges and the corresponding degree value.
///////////////////////////////////////////////////////////////////////////////
	void ALMGraph::reverseElimVertex (int vid,
		const vector<int> &removed_edges,
		const vector<pair<int,int> > &added_edges,
		bool removed_pSAAS, const vector<int> *added_pSAAS)
	{
		// remove added edges
		for (int i=0; i<(int)added_edges.size(); i++) {
#ifdef DEBUG_ALMGRAPH
			assert(adjLM[added_edges[i].first][added_edges[i].second].active);
			assert(adjLM[added_edges[i].second][added_edges[i].first].active);
#endif
			removeEdge(added_edges[i].first,added_edges[i].second);

			// update common neighbor tracking
			// every neighbor of one vertex on removed edge has one
			// less common neighbor with other vertex on removed edge
			if (graph_cnt_mode==TRACK_CN) {
				int wid = added_edges[i].first;
				int xid = added_edges[i].second;
				VertexList *y = adjLM[wid][0].next;
				while (y!=NULL) {
					if (y->vid < xid)
						commonNeighbors[y->vid][xid]--;
					else
						commonNeighbors[xid][y->vid]--;
					y=y->next;
				}
				y = adjLM[xid][0].next;
				while (y!=NULL) {
					if (y->vid < wid)
						commonNeighbors[y->vid][wid]--;
					else
						commonNeighbors[wid][y->vid]--;
					y=y->next;
				}
			}
		}

		// add removed vertex back
		nVerts++;
		addVertex_toVertexList(vid);

		// add removed edges back
		for (int i=0; i<(int)removed_edges.size(); i++) {
#ifdef DEBUG_ALMGRAPH
			assert(adjLM[vid][removed_edges[i]].active);
#endif
			addDEdge(removed_edges[i],vid);
		}

		// update common neighbor tracking
		// all neighbors of vid will have one more common neighbor
		// with each other
		if (graph_cnt_mode==TRACK_CN) {
			VertexList *w = adjLM[vid][0].next;
			while (w!=NULL) {
				VertexList *x = w->next;
				while (x!=NULL) {
					if (w->vid < x->vid)
						commonNeighbors[w->vid][x->vid]++;
					else
						commonNeighbors[x->vid][w->vid]++;
					x=x->next;
				}
				w=w->next;
			}
		}

		// remove added vertices from pSAAS
		if (added_pSAAS!=NULL)
		{
			for (uint i=0; i<added_pSAAS->size(); i++)
			{
				assert(potentialSimpAndAlmostSimp[(*added_pSAAS)[i]].active);
				removeVertex_fromSimpList((*added_pSAAS)[i]);
			}
		}
		// add removed vertex to pSSAS
		if (removed_pSAAS)
		{
			assert(!potentialSimpAndAlmostSimp[vid].active);
			addVertex_toSimpList(vid);
		}
	}


///////////////////////////////////////////////////////////////////////////////
// ALMGraph reverseElimVertices - reverses the elimination of vertices vids
//   from the graph. vids should be in the order the vertices were eliminated,
//   therefore they will be "reverse eliminated" in the reverse order. For
//   vids[i], removed_edges[i] stores all of vids[i]'s former neighbors and
//   added_edges[i] stores all of the edges added to the graph when vids[i]
//   was eliminated.
// NOTE: function assumes that vid[i]'s row in adjLM is the same as it was
// prior to elimination. That is, still contains a valid linked list of the
// vertices in removed_edges and the corresponding degree value.
///////////////////////////////////////////////////////////////////////////////
	void ALMGraph::reverseElimVertices (const vector<int> vids,
			const vector<int> removed_edges[],
			const vector<pair<int,int> > added_edges[])
	{
		for (int i=vids.size()-1; i>=0; i--)
			reverseElimVertex(vids[i], removed_edges[i], added_edges[i]);
	}



	void ALMGraph::elimSuffix (const TWState &s, uint suffix_len,
			vector<vector<int> > &removed_edges,
			vector<vector<pair<int,int> > > &added_edges,
			vector<bool> &removed_pSAAS,
			vector<vector<int> > &added_pSAAS)
	{
		// get the sorted set of vertices to eliminate
		vector<int> toelim;
		toelim.reserve(suffix_len);
		s.getEliminatedVertices(toelim, suffix_len);

		// eliminate vertices, starting with the highest index
		//cout << "  Elim: ";
		for (int i=toelim.size()-1; i>=0; i--)
		{
			//cout << toelim[i] << " ";
			assert(removed_edges[i].empty());
			assert(added_edges[i].empty());
			assert(removed_pSAAS[i]==false);
			assert(added_pSAAS[i].empty());
			bool removed = removed_pSAAS[toelim[i]];
			elimVertex(toelim[i], &removed_edges[toelim[i]],
				&added_edges[toelim[i]], INT_MAX, &removed,
				&added_pSAAS[toelim[i]]);
			removed_pSAAS[toelim[i]] = removed;
		}
		//cout << endl;
	}

	void ALMGraph::reverseElimSuffix (const TWState &s, uint suffix_len,
			vector<vector<int> > &removed_edges,
			vector<vector<pair<int,int> > > &added_edges,
			vector<bool> &removed_pSAAS,
			vector<vector<int> > &added_pSAAS)
	{
		// get the sorted set of vertices to uneliminate
		vector<int> tounelim;
		tounelim.reserve(suffix_len);
		s.getEliminatedVertices(tounelim, suffix_len);

		// uneliminate vertices, starting with lowest index
		//cout << "  Unelim: ";
		for (uint i=0; i<tounelim.size(); i++)
		{
			//cout << tounelim[i] << " ";
			reverseElimVertex(tounelim[i], removed_edges[tounelim[i]],
				added_edges[tounelim[i]], removed_pSAAS[tounelim[i]],
				&added_pSAAS[tounelim[i]]);
			removed_edges[tounelim[i]].clear();
			added_edges[tounelim[i]].clear();
			removed_pSAAS[tounelim[i]] = false;
			added_pSAAS[tounelim[i]].clear();
		}
		//cout << endl;
	}

///////////////////////////////////////////////////////////////////////////////
// ALMGraph removeVertex_nopSAAS - removes the given vertex and all incident
//   edges from the graph, without making any changes to the pSAAS list.
// NOTE: like with elimVertex, this does not actually reset the vertex's row
// in the adj matrix. it is removed from the vertex list. if the vertex is
// later added back into the graph, then its row can be reset then.
// NOTE: function assumes vertex is currently in graph, if not d.s. will be
// corrupted.
///////////////////////////////////////////////////////////////////////////////
	void ALMGraph::removeVertex_nopSAAS (int vid)
	{
#ifdef DEBUG_ALMGRAPH
		assert(vertices[vid].active);
#endif
		// remove vertex
		nVerts--;
		removeVertex_fromVertexList(vid);
		// remove edges from remaining vertices
		VertexList *w = adjLM[vid][0].next;
		while (w!=NULL) {
			// remove edge from w->v
			removeDEdge(w->vid,vid);
			w=w->next;
		}
	}

///////////////////////////////////////////////////////////////////////////////
// ALMGraph addVertex_nopSAAS - function vertex vid in orig graph to this
//   graph. This includes adding it to the vertices list and adding the edges
//   in orig between vid and any vertices in this graph to adjLM.
// NOTE: this function does NOT assume vid's row in adjLM is clear, though
// it DOES assume that if it is not clear then the active vertices are in a
// linked list. this way it can traverse the list to clear it.
// NOTE: function assumes vertex is not already in this graph, if it is then
// d.s. will be corrupted.
///////////////////////////////////////////////////////////////////////////////
	void ALMGraph::addVertex_nopSAAS (const ALMGraph &orig, int vid)
	{
#ifdef DEBUG_ALMGRAPH
		assert(!vertices[vid].active);
		assert(orig.vertices[vid].active);
#endif
		// activate vertex
		nVerts++;
		addVertex_toVertexList(vid);
		// reset vid's row in adjLM
		VertexList *curr = adjLM[vid][0].next;
		while (curr!=NULL) {
			curr->active=false;
			curr=curr->next;
		}
		adjLM[vid][0].vid=0;
		adjLM[vid][0].next=NULL;
		// add edges in orig btwn vid and verts in graph
		curr = orig.adjLM[vid][0].next;
		while (curr!=NULL) {
			if (vertices[curr->vid].active) // if vertex in graph
				addEdge(vid,curr->vid);
			curr=curr->next;
		}
	}


///////////////////////////////////////////////////////////////////////////////
// ALMGraph contractEdge - contracts edge (v,w), where new vertex has index of
//   argument vertex with the lesser degree (if equal, then v). also, function
//   updates degList with any changes to vertex degrees made by contraction.
// NOTE: function does not make any changes to pSAAS list, therefore after
//   calling this function pSAAS will not reflect graph
// NOTE: function makes no changes to the rows in adjLM for innactive vertices
//   and removed vertex.
///////////////////////////////////////////////////////////////////////////////
	void ALMGraph::contractEdge (int v, int w, VertexDegList &degList,
		int *remv, vector<int> *removed_edges, int *keptv,
		vector<int> *added_edges)
	{
		// ensure that d(v)<=d(w), swap if necessary
		if (adjLM[w][0].vid<adjLM[v][0].vid) {
			int tmp=v;
			v=w;
			w=tmp;
		}
		if (remv!=NULL)
			*remv=v;
		if (keptv!=NULL)
			*keptv=w;
		// connect n(v) to w and vice-versa
		// also remove directed edge from each of n(v) to v
		int oldWDeg = adjLM[w][0].vid;
		VertexList *curr = adjLM[v][0].next;
		while (curr!=NULL) {
			// remove edge to v
			removeDEdge(curr->vid,v);
			if (removed_edges!=NULL)
				removed_edges->push_back(curr->vid);
			// add edge to w if not already active (and it's not w)
			if (!adjLM[w][curr->vid].active && w!=curr->vid) {
				addEdge(w,curr->vid);
				if (added_edges!=NULL)
					added_edges->push_back(curr->vid);
			}
			// if edge to w is already active (and it's not w) degree is decreased by 1
			else if (w!=curr->vid)
				degList.changeDeg(curr->vid,adjLM[curr->vid][0].vid);
			curr=curr->next;
		}
		// update w degree
		if (adjLM[w][0].vid!=oldWDeg) {
			// if w degree = 0, remove w
			if (adjLM[w][0].vid==0) {
				nVerts--;
				removeVertex_fromVertexList(w);
				degList.removeVertex(w);
			} else
				degList.changeDeg(w,adjLM[w][0].vid);
		}
		// remove v
		nVerts--;
		removeVertex_fromVertexList(v);
		degList.removeVertex(v);
	}


	void ALMGraph::contractEdge (int v, int w)
	{
		// ensure that v>w, swap if necessary
		if (v<w) {
			int tmp=v;
			v=w;
			w=tmp;
		}
		// connect n(v) to w and vice-versa
		// also remove directed edge from each of n(v) to v
		int oldWDeg = adjLM[w][0].vid;
		VertexList *curr = adjLM[v][0].next;
		while (curr!=NULL) {
			// remove edge to v
			removeDEdge(curr->vid,v);
			// add edge to w if not already active (and it's not w)
			if (!adjLM[w][curr->vid].active && w!=curr->vid) {
				addEdge(w,curr->vid);
			}
			curr=curr->next;
		}
		// update w degree
		if (adjLM[w][0].vid!=oldWDeg) {
			// if w degree = 0, remove w
			if (adjLM[w][0].vid==0) {
				nVerts--;
				removeVertex_fromVertexList(w);
			}
		}
		// remove v
		nVerts--;
		removeVertex_fromVertexList(v);
	}


	/* removes vertex. does not update potentialSimpAndAlmostSimp list
	 * or commonNeighbors list. does update degList. */
	void ALMGraph::removeVertex(int vid, VertexDegList &degList)
	{
		// update degree for vertices adjacent to v, and remove edge to v
		VertexList *w = adjLM[vid][0].next;
		while (w!=NULL) {
			// remove edge from w to v (v to w doesn't matter)
			removeDEdge(w->vid, vid);
			degList.decDeg(w->vid);
			w = w->next;
		}

		// remove vertex from vertices list and degList
		degList.removeVertex(vid);
		removeVertex_fromVertexList(vid);
		nVerts--;
	}

///////////////////////////////////////////////////////////////////////////////
// ALMGraph reverseContractEdge - adds remv back to graph along with edges to
//   vertices in removed_edges. Also removes edges between keptv and vertices
//   in added_edges.
// NOTE: function assumes that row of adjLM for remv is the same as it was
// prior to contraction (that is, contains all verts in removed_edges).
///////////////////////////////////////////////////////////////////////////////
	void ALMGraph::reverseContractEdge (int remv, vector<int> removed_edges,
		int keptv, vector<int> added_edges)
	{
		nVerts++;
		addVertex_toVertexList(remv);
		for (int i=0; i<(int)removed_edges.size(); i++)
			addDEdge(removed_edges[i],remv);
		for (int i=0; i<(int)added_edges.size(); i++)
			removeEdge(keptv,added_edges[i]);
	}


	/* function for calling heuristics. ub is only used by degeneracy or
	 * contraction degeneracy-based heuristics, and tiebreaking only used by
	 * those that are min-degree-based heuristics. */
	int ALMGraph::heuristic(HeuristicVersion hversion, int ub,
			MinDegTieBreaking tiebreaking) const
	{
		switch (hversion) {
		case H_CONTRACTION_MIND:
			return hDegen(CONTRACTION_MIND, ub, tiebreaking);
		case H_CONTRACTION_LEASTC:
			return hDegen(CONTRACTION_LEASTC, ub, tiebreaking);
		case H_DEGEN_MINDEG:
			return hDegen(DEGEN_MINDEG, ub, tiebreaking);
		case H_DEGEN_MINPAIRMAX:
			return hDegen(DEGEN_MINPAIRMAX, ub);
		case H_MINDEG:
			return hMinDeg();
		case H_MINPAIRMAX:
			return hMinPairMax();
		default:
			assert(false);
		}
	}

	int ALMGraph::heuristic_UNSAFE(HeuristicVersion hversion, int ub,
			MinDegTieBreaking tiebreaking)
	{
		switch (hversion) {
		case H_CONTRACTION_MIND:
			return hDegen_UNSAFE(CONTRACTION_MIND, ub, tiebreaking);
		case H_CONTRACTION_LEASTC:
			return hDegen_UNSAFE(CONTRACTION_LEASTC, ub, tiebreaking);
		case H_DEGEN_MINDEG:
			return hDegen_UNSAFE(DEGEN_MINDEG, ub, tiebreaking);
		case H_DEGEN_MINPAIRMAX:
			return hDegen_UNSAFE(DEGEN_MINPAIRMAX, ub);
		case H_MINDEG:
			return hMinDeg();
		case H_MINPAIRMAX:
			return hMinPairMax();
		default:
			assert(false);
		}
	}

///////////////////////////////////////////////////////////////////////////////
// ALMGraph hRandomized - computed the heuristic lower bound by running a
//   randomized version of the specified heuristic the specified number of
//   times. The greatest lower bound is returned.
// NOTE: the only heuristics that can be randomized are H_CONTRACTION_MIND,
// H_CONTRACTION_LEASTC, and H_DEGEN_MINDEG.
///////////////////////////////////////////////////////////////////////////////
	int ALMGraph::hRandomized (HeuristicVersion version, int niterations,
		int ub) const
	{
		//return heuristic(version, ub);

		if (version!=H_CONTRACTION_MIND && version!=H_CONTRACTION_LEASTC &&
				version!=H_DEGEN_MINDEG)
			return heuristic(version, ub);

		int curr, best=0;
		for (int i=0; i<niterations; i++) {
			curr = heuristic(version, ub, MINDEG_BREAK_TIES_RANDOMLY);
			if (curr > best)
				best=curr;
		}
		return best;
	}

	int ALMGraph::hMinDeg() const
	{
		if (nVerts==0)
			return 0;
		int mindeg = INT_MAX;
		VertexList *curr = vertices[0].next;
		while (curr!=NULL) {
			if (adjLM[curr->vid][0].vid<mindeg) {
				mindeg = adjLM[curr->vid][0].vid;
				if (mindeg<=1)
					return mindeg;
			}
			curr = curr->next;
		}
		return mindeg;
	}

	int ALMGraph::hMinPairMax() const
	{
		if (nVerts==0)
			return 0;
		// Sort list of vertices by degree
		multimap<int,int> degVidMap;
		VertexList *v = vertices[0].next;
		while (v!=NULL)
		{
			degVidMap.insert(pair<int,int>(adjLM[v->vid][0].vid, v->vid));
			v = v->next;
		}

		// Find first vertex not adjacent to all preceeding
		vector<int> neighborCount(vertices.size(),0);
		multimap<int,int>::const_iterator iter = degVidMap.begin();
		int i=0;
		while (iter != degVidMap.end() && i == neighborCount[iter->second])
		{
			VertexList *v = adjLM[iter->second][0].next;
			while (v!=NULL)
			{
				neighborCount[v->vid]++;
				v = v->next;
			}
			i++;
			iter++;
		}

		// Graph is a clique
		if (iter == degVidMap.end())
			return nVerts-1;

		// Return min-pair-max value
		return iter->first;
	}

///////////////////////////////////////////////////////////////////////////////
// ALMGraph hDegen - a SAFE version of hDegen_UNSAFE; it DOES NOT change the
//   graph itself in the computation process.
// NOTE: function assumes that min degree vertex in graph has degree < ub, i.e.
//   it is not obvious that lb==ub.
///////////////////////////////////////////////////////////////////////////////
	int ALMGraph::hDegen (DegenVersion version, int ub,
		MinDegTieBreaking tiebreaking) const
	{
		if (nVerts==0)
			return 0;
//// following implementation makes a copy of the graph, then it calls a
//// destructive version of the heuristic function on that copy.
		// copy graph
#if N_DDD_THREADS > 1
		ALMGraph g;
#else
		static ALMGraph g;
#endif
		if (g.adjLM.size()!=adjLM.size())
			g.init_nopSAAS(adjLM.size()-1);
		g.copy_nopSAAS(*this);
		// destroy copy to compute value
		return g.hDegen_UNSAFE(version,ub,tiebreaking);
//// following implementation tracks added and deleted edges and vertices,
//// after heuristic computation completes, graph is reverted to original
//// form by reversing contractions.
//#ifdef MONOTONIC_H
//		assert(false); // this heuristic has not been shown to be monotonic
//#endif
//		// buffers for storing removed/kept vertices and added/removed edges
//		static vector<int> remvs;
//		static vector<int> keptvs;
//		static vector<vector<int> > removed_edges;
//		static vector<vector<int> > added_edges;
//		remvs.resize(nVerts);
//		keptvs.resize(nVerts);
//		removed_edges.resize(nVerts);
//		added_edges.resize(nVerts);
//		int nc = 0; // no. of contractions so far
//		// setup lists of vertices by degree
//		VertexDegList degList(vertices.size()-1,nVerts-1);
//		VertexList *curr = vertices[0].next;
//		while (curr!=NULL) {
//			degList.addVertex(curr->vid,adjLM[curr->vid][0].vid);
//			curr=curr->next;
//		}
//		while (!degList.empty && degList.mindeg==0) {
//			int v=degList.getMinVert();
//			nVerts--;
//			removeVertex_fromVertexList(v);
//			degList.removeVertex(v);
//		}
//		// begin
//		int lb=degList.mindeg; // lower bound
//		while (lb<nVerts) {
//			// find min degree vertex
//			int v;
//			if (tiebreaking==MMDPLUS_BREAK_TIES_RANDOMLY)
//				v=degList.getMinVert_randomized();
//			else
//				v=degList.getMinVert();
//			// find neighbor to contract depending on version
//			int w;
//			if (version==MMDPLUS_MIND) {
//				if (tiebreaking==MMDPLUS_BREAK_TIES_RANDOMLY)
//					w = findMinDegreeNeighbor_randomized(v);
//				else
//					w = findMinDegreeNeighbor(v,degList.mindeg);
//			} else {
//				if (tiebreaking==MMDPLUS_BREAK_TIES_RANDOMLY)
//					w = findLeastCommonNeighbor_randomized(v);
//				else
//					w = findLeastCommonNeighbor(v);
//			}
//			// contract edge
//			removed_edges[nc].clear();
//			added_edges[nc].clear();
//			contractEdge(v,w,degList,&remvs[nc],&removed_edges[nc],
//				&keptvs[nc],&added_edges[nc]);
//			nc++;
//			// update lb
//			if (degList.mindeg>lb && !degList.empty) {
//				lb = degList.mindeg;
//				if (lb>=ub) {// test for upper bound
//					lb=ub;
//					break;
//				}
//			}
//		}
//		remvs.resize(nc);
//		keptvs.resize(nc);
//		removed_edges.resize(nc);
//		added_edges.resize(nc);
//		// reconstruct graph
//		for (int i=nc-1; i>=0; i--)
//			reverseContractEdge(remvs[i],removed_edges[i],keptvs[i],
//				added_edges[i]);
//		// done
//		if (lb==INT_MAX) // if graph was empty to start
//			return 0;
//		else
//			return lb;
	}

///////////////////////////////////////////////////////////////////////////////
// ALMGraph hDegen_UNSAFE - computes the a lower bound on tw of graph using
// one of several degeneracy- or contraction degeneracy-based methods.
// version = DEGEN_MINDEG
//   return minimum degree among set of subgraphs derived by continuely
//   removing min-deg vertex
// version = DEGEN_MINPAIRMAX
//   returns the minimum maximum degree among all pairs of vertices in a set
//   of subgraphs derived by continuely removing all vertices of lesser degree
// version = CONTRACTION_MIND
//   return minimum degree among set of graph minors derived by continuely
//   contracting edge between min-deg vertex and min-deg neighbor
// version = CONTRACTION_LEASTC
//   return minimum degree among set of graph minors derived by continuely
//   contracting edge between min-deg vertex and neighbor with least number
//   of common neighbors
// Function is UNSAFE because it destroys the ALMGraph in the process of
//   computing the return value. Optional input paramter ub specifies an upper
//   bound on the tw of the graph. If function finds that returned lb will
//   equal or exceed ub it will stop execution and return ub as the lb.
// NOTE: function assumes that min degree vertex in graph has degree < ub, i.e.
//   it is not obvious that lb==ub.
// Background:
//   All of the methods are described in the following papers.
//   DEGEN_MINDEG - MMD in [1], deltaD in [3]
//   DEGEN_MINPAIRMAX - gammaRD in [3], based on method in [4]
//   CONTRACTION_MIND - MMD+(min-d) in [1], MMW in [2]
//   CONTRACTION_LEASTC - MMD+(least-c) in [1], deltaC(least-c) in [3]
// References:
//  [1] Bodlaender, Koster, and Wolle, "Contraction and treewidth lower bounds"
//      (2004).
//  [2] Gogate and Dechter, "A complete anytime algorithm for treewidth"
//      (UAI 2004).
//  [3] Koster, Wolle, and Bodlaender, "Degree-based treewidth lower bounds"
//      (2004).
//  [4] Ramachandramurthi, "The structure of obstructions to treewidth"
//      (SIAM J. Discrete Math. 1997).
///////////////////////////////////////////////////////////////////////////////
	int ALMGraph::hDegen_UNSAFE (DegenVersion version, int ub,
		MinDegTieBreaking tiebreaking)
	{
#ifdef MONOTONIC_H
		assert(false); // this heuristic has not been shown to be monotonic
#endif
		// setup lists of vertices by degree
		VertexDegList degList(vertices.size()-1,nVerts-1);
		VertexList *curr = vertices[0].next;
		while (curr!=NULL) {
			degList.addVertex(curr->vid,adjLM[curr->vid][0].vid);
			curr=curr->next;
		}
		while (!degList.empty && degList.mindeg==0) {
			int v=degList.getMinVert();
			nVerts--;
			removeVertex_fromVertexList(v);
			degList.removeVertex(v);
		}
		// begin
		int lb=degList.mindeg; // lower bound
		while (lb<nVerts)
		{
			// minDeg-based heuristics
			if (version==CONTRACTION_MIND || version==CONTRACTION_LEASTC ||
					version==DEGEN_MINDEG)
			{
				// find min degree vertex
				int v;
				if (tiebreaking==MINDEG_BREAK_TIES_RANDOMLY)
					v=degList.getMinVert_randomized();
				else
					v=degList.getMinVert();

				if (version==CONTRACTION_MIND || version==CONTRACTION_LEASTC)
				{
					// find neighbor to contract depending on version
					int w;
					if (version==CONTRACTION_MIND)
					{
						if (tiebreaking==MINDEG_BREAK_TIES_RANDOMLY)
							w = findMinDegreeNeighbor_randomized(v);
						else
							w = findMinDegreeNeighbor(v,degList.mindeg);
					}
					else
					{
						if (tiebreaking==MINDEG_BREAK_TIES_RANDOMLY)
							w = findLeastCommonNeighbor_randomized(v);
						else
							w = findLeastCommonNeighbor(v);
					}

					// contract edge
					contractEdge(v,w,degList);
				}
				else if (version==DEGEN_MINDEG)
				{
					// remove mindeg vertex
					removeVertex(v, degList);
				}

				// update lb
				if (degList.mindeg>lb && !degList.empty) {
					lb = degList.mindeg;
					if (lb>=ub) // test for upper bound
						return ub;
				}
			}
			// minPairMax-based heuristics
			else if (version==DEGEN_MINPAIRMAX)
			{
				// find first vertex not adjacent to all preceeding
				vector<int> neighborCount(vertices.size(),0);
				vector<int> vertsToRemove;
				int curdeg = degList.mindeg;
				VertexDegLink *v = degList.degLists[degList.mindeg];
				assert(v!=NULL);
				int i=0;
				while (v != NULL && i == neighborCount[v->vid])
				{
					vertsToRemove.push_back(v->vid);
					VertexList *w = adjLM[v->vid][0].next;
					while (w!=NULL)
					{
						neighborCount[w->vid]++;
						w = w->next;
					}
					i++;
					v = v->next;
					while (v == NULL && curdeg < (int)degList.degLists.size())
						v = degList.degLists[++curdeg];

					if (curdeg >= (int)degList.degLists.size())
						v = NULL;
				}
				// if graph is a clique, update lb and return
				if (v==NULL)
				{
					if (nVerts-1 > lb)
						lb = nVerts-1;
					return lb;
				}
				// update lb
				if (curdeg > lb)
				{
					lb = curdeg;
					if (lb >= ub) // test for ub
						return ub;
				}
				// remove vertices preceeding v
				for (int i=0; i<(int)vertsToRemove.size(); i++)
					removeVertex(vertsToRemove[i], degList);
			}
		}
		if (lb==INT_MAX) // if graph was empty to start
			return 0;
		else
			return lb;
	}




///////////////////////////////////////////////////////////////////////////////
// ALMGraph ubRandomizedMinFill - finds an upperbound on treewidth of the graph
//   by running the minfill heuristic niterations times, where on each
//   iteration ties are broken randomly. This function is SAFE because it does
//   not make any changes to the graph itself.
///////////////////////////////////////////////////////////////////////////////
	ElimOrder ALMGraph::ubRandomizedMinFill (int niterations)
	{
		ElimOrder curr, best;
		for (int i=0; i<niterations; i++) {
			curr = ubMinFill(MINFILL_BREAK_TIES_RANDOMLY);
			if (best.width==-1 || curr.width<best.width)
				best=curr;
		}
		return best;
	}

///////////////////////////////////////////////////////////////////////////////
// ALMGraph ubMinFill - finds an upperbound on treewidth of the graph by using
//   the minfill heuristic, breaking ties arbitrarily or at random as specified
//   by input parameter. This version is SAFE because it does not make any
//   changes to the graph while comupting the bound.
///////////////////////////////////////////////////////////////////////////////
	ElimOrder ALMGraph::ubMinFill (MinFillTieBreaking tiebreaking)
	{
		// copy graph
#if N_DDD_THREADS > 1
		ALMGraph g;
#else
		static ALMGraph g;
#endif
		if (g.vertices.size()!=vertices.size())
			g.init_nopSAAS(vertices.size()-1);
		g.copy_nopSAAS(*this);
		// run unsafe version on copy
		return g.ubMinFill_UNSAFE(tiebreaking);
	}

///////////////////////////////////////////////////////////////////////////////
// ALMGraph ubMinFill_UNSAFE - finds an upperbound on treewidth of the graph
//   by using the minfill heuristic, breaking ties arbitrarily or at random as
//   specified by input parameter. This version is UNSAFE because it destroys
//   the graph itself while computing the bound.
///////////////////////////////////////////////////////////////////////////////
	ElimOrder ALMGraph::ubMinFill_UNSAFE (MinFillTieBreaking tiebreaking)
	{
		ElimOrder order;
		order.width=0;
		order.order_prefix.reserve(nVerts);
		while (nVerts>order.width) {
			// choose vert to eliminate
			int v;
			if (tiebreaking == MINFILL_BREAK_TIES_RANDOMLY)
				v = findMinFillVert_randomized();
			else
				v = findMinFillVert();
			// update order
			order.order_prefix.push_back(v);
			if (adjLM[v][0].vid>order.width)
				order.width=adjLM[v][0].vid;
			// eliminate vertex
			elimVertex_nopSAAS(v);
		}
		// add remaining verts to end of order
		VertexList *curr = vertices[0].next;
		while (curr!=NULL) {
			order.order_prefix.push_back(curr->vid);
			curr=curr->next;
		}
		return order;
	}

///////////////////////////////////////////////////////////////////////////////
// ALMGraph getWidth - finds the width of the given order on this graph. This
//   version of getWidth is SAFE, in that it does not make any changes to the
//   graph itself.
// NOTE: function assumes that every vertex graph appears in order once and
//   that all vertices in order appear in graph.
///////////////////////////////////////////////////////////////////////////////
	int ALMGraph::getWidth (const vector<int> &order)
	{
		// copy graph
#if N_DDD_THREADS > 1
		ALMGraph g;
#else
		static ALMGraph g;
#endif
		if (g.vertices.size()!=vertices.size())
			g.init_nopSAAS(vertices.size()-1);
		g.copy_nopSAAS(*this);
		// destroy copy to compute value
		return g.getWidth_UNSAFE(order);
	}


///////////////////////////////////////////////////////////////////////////////
// ALMGraph getWidth_UNSAFE - finds the width of the given order on this graph.
//   This version of getWidth is UNSAFE, in that it destroys the graph itself
//   in the process of computing the width.
// NOTE: function assumes that every vertex graph appears in order once and
//   that all vertices in order appear in graph.
///////////////////////////////////////////////////////////////////////////////
	int ALMGraph::getWidth_UNSAFE (const vector<int> &order)
	{
		int maxdeg=0;
		// eliminate vertices in order
		for (int i=0; i<(int)order.size(); i++) {
			if (adjLM[order[i]][0].vid>maxdeg)
				maxdeg=adjLM[order[i]][0].vid;
			elimVertex_nopSAAS(order[i]);
		}
		// eliminate remaining vertices in arbitrary order
		while (nVerts>0) {
			int v = vertices[0].next->vid;
			if (adjLM[v][0].vid>maxdeg)
				maxdeg=adjLM[v][0].vid;
			elimVertex_nopSAAS(v);
		}
		return maxdeg;
	}


///////////////////////////////////////////////////////////////////////////////
// ALMGraph reduceGraphOne - invokes the Simplicial and the Almost Simplicial
//   rules on vertices in the pSAAS list, limited to eliminating a single
//   vertex from the graph.
// Inputs:
//   lb - lower bound on tw of ORIGINAL graph. almost simplicial rule will not
//     apply to any vertex with degree > lb.
//   s - [optional] pointer to a state. If provided then only vertices set in
//     the state will be candidates for elimination.
// Output:
//   reduce_one_ret structure, specifies what (if any) vertex was eliminated
//   in reduction and the degree of the vertex before elimination.
// NOTE: function assumes that pSAAS is not empty.
///////////////////////////////////////////////////////////////////////////////
	reduce_one_ret ALMGraph::reduceGraphOne(int lb, int ub, const TWState *s,
			vector<int> *removed_edges, ReduceGraphOrder rgorder,
			int minvid, vector<bool> *nopruneverts)
	{
		reduce_one_ret ret;
		ret.elimVert=-1;

		// Determine which vertex to consider for reduction
		VertexList *curr = NULL;
		if (rgorder == REDUCE_LEAST_FIRST) {
			for (int i=1; i<minvid; i++) {
				if (potentialSimpAndAlmostSimp[i].active &&
						(nopruneverts==NULL || !(*nopruneverts)[i]))
				{
					curr = &potentialSimpAndAlmostSimp[i];
					break;
				}
			}
			if (curr == NULL)
				rgorder = REDUCE_ARBITRARY;
		}
		if (rgorder == REDUCE_ARBITRARY)
			curr = potentialSimpAndAlmostSimp[0].next;

		// consider each vertex in pSAAS list
		while (curr!=NULL) {
			VertexList *next = curr->next;
			// if no state given, or vertex eliminated in given state
			if (s==NULL || s->checkVert(curr->vid)) {
				// determine if either rule applies
				simp_ret simp = getSimpState(curr->vid);
				// apply that rule
				if (simp.state==SIMP) {
					ret.elimVert = curr->vid;
					ret.deg = adjLM[curr->vid][0].vid;
					if (ret.deg>=ub)
						break;
					elimVertexSimp(curr->vid, removed_edges);
					break;
				} else if (simp.state==ALSIMP && adjLM[curr->vid][0].vid<=lb) {
					ret.elimVert = curr->vid;
					ret.deg = adjLM[curr->vid][0].vid;
					elimVertexAlSimp(curr->vid,simp.dvert, removed_edges);
					break;
				}
				// remove vert from pSAAS
				removeVertex_fromSimpList(curr->vid);
			}

			// Determine which vertex to consider next for reduction
			if (rgorder == REDUCE_LEAST_FIRST) {
				for (int i=curr->vid+1; i<minvid; i++) {
					if (potentialSimpAndAlmostSimp[i].active &&
							(nopruneverts==NULL || !(*nopruneverts)[i])) {
						next = &potentialSimpAndAlmostSimp[i];
						break;
					}
				}
				if (next == NULL || next->vid >= minvid) {
					rgorder = REDUCE_ARBITRARY;
					next = potentialSimpAndAlmostSimp[0].next;
				}
			}
			curr=next;
		}
		return ret;
	}

///////////////////////////////////////////////////////////////////////////////
// ALMGraph findReduceGraphOne - just like reduceGraphOne except it doesn't
// actually eliminate the vertex. It just finds out if a vertex can be reduced
// and returns its index and degree.
///////////////////////////////////////////////////////////////////////////////
	reduce_one_ret ALMGraph::findReduceGraphOne(int lb, int ub,
			ReduceGraphOrder rgorder, int minvid, vector<bool> *nopruneverts)
	const
	{
		reduce_one_ret ret;
		ret.elimVert=-1;

		ReduceGraphOrder orig_rgorder = rgorder;

		// Determine which vertex to consider for reduction
		const VertexList *curr = NULL;
		if (rgorder == REDUCE_LEAST_FIRST) {
			for (int i=1; i<minvid; i++) {
				if (potentialSimpAndAlmostSimp[i].active &&
						(nopruneverts==NULL || !(*nopruneverts)[i]))
				{
					curr = &potentialSimpAndAlmostSimp[i];
					break;
				}
			}
			if (curr == NULL)
				rgorder = REDUCE_ARBITRARY;
		}
		if (rgorder == REDUCE_ARBITRARY)
			curr = potentialSimpAndAlmostSimp[0].next;

		// consider each vertex in pSAAS list
		while (curr!=NULL)
		{
			// determine if either rule applies
			simp_ret simp = getSimpState(curr->vid);

			// apply that rule
			if (simp.state==SIMP)
			{
				ret.elimVert = curr->vid;
				ret.deg = adjLM[curr->vid][0].vid;
				break;
			}
			else if (simp.state==ALSIMP && adjLM[curr->vid][0].vid<=lb)
			{
				ret.elimVert = curr->vid;
				ret.deg = adjLM[curr->vid][0].vid;
				break;
			}

			// Determine which vertex to consider next for reduction
			const VertexList *next = NULL;
			if (rgorder == REDUCE_LEAST_FIRST) {
				for (int i=curr->vid+1; i<minvid; i++) {
					if (potentialSimpAndAlmostSimp[i].active &&
							(nopruneverts==NULL || !(*nopruneverts)[i])) {
						next = &potentialSimpAndAlmostSimp[i];
						break;
					}
				}
				if (next == NULL || next->vid >= minvid) {
					rgorder = REDUCE_ARBITRARY;
					next = potentialSimpAndAlmostSimp[0].next;
				}
			}
			else
			{
				next = curr->next;
				if (orig_rgorder == REDUCE_LEAST_FIRST)
					while (next!=NULL && next->vid<minvid &&
							!(*nopruneverts)[next->vid])
						next = next->next;
			}

			curr = next;
		}
		return ret;
	}

/*
///////////////////////////////////////////////////////////////////////////////
// ALMGraph reduceGraphOne_fromSubset - same as reduceGraphOne except vertices
//   eligible for elimination are limited to those already eliminated in State
//   s.
///////////////////////////////////////////////////////////////////////////////
	reduce_one_ret ALMGraph::reduceGraphOne_fromSubset (const State &s, int lb, int ub)
	{
#ifdef DEBUG_ALMGRAPH
		assert(potentialSimpAndAlmostSimp[0].vid>0);
#endif
		reduce_one_ret ret;
		ret.elimVert=-1;
		// consider each vertex in pSAAS list
		VertexList *curr = potentialSimpAndAlmostSimp[0].next;
		while (curr!=NULL) {
			VertexList *next = curr->next;
			if (s.checkVert(curr->vid)) { // only consider vertices set in state s
				// determine if either rule applies
				simp_ret simp = getSimpState(curr->vid);
				// apply that rule
				if (simp.state==SIMP) {
					ret.elimVert = curr->vid;
					ret.deg = adjLM[curr->vid][0].vid;
					if (ret.deg>=ub)
						break;
					elimVertexSimp(curr->vid);
					break;
				} else if (simp.state==ALSIMP && adjLM[curr->vid][0].vid<=lb) {
					ret.elimVert = curr->vid;
					ret.deg = adjLM[curr->vid][0].vid;
					elimVertexAlSimp(curr->vid,simp.dvert);
					break;
				}
				// remove vert from pSAAS
				removeVertex_fromSimpList(curr->vid);
			}
			// get next vert in list
			curr = next;
		}
		return ret;
	}
*/

///////////////////////////////////////////////////////////////////////////////
// ALMGraph reduceGraphAll - invokes the Simplicial and the Almost Simplicial
//   rules on vertices in the pSAAS list. Function continues until the pSAAS
//   list is empty. In order to properly apply the almost simplicial rule,
//   function gets the h-function lower bound on the graph after any
//   elimination.
// Inputs:
//   lb - lower bound on tw of ORIGINAL graph. almost simplicial rule will not
//     apply to any vertex with degree > lb.
//   s - [optional] pointer to a state. If provided then only vertices set in
//     the state will be candidates for elimination.
// Output:
//   reduce_all_ret structure, specifies what (if any) vertices were
//   eliminated in reduction, the maximum degree among eliminated vertices,
//   and the computed h-value lb on resulting graph.
///////////////////////////////////////////////////////////////////////////////
	reduce_all_ret ALMGraph::reduceGraphAll( int lb, HeuristicVersion hversion, int ub, const TWState *s )
	{
#ifndef MONOTONIC_H
		int last_deg=0;
#endif
		reduce_all_ret ret;
		ret.maxdeg=0;
		ret.hval=0;
		bool anyElim=false; // any elim since last time lb was calculated?
		// while there are still vertices in pSAAS list
		while (potentialSimpAndAlmostSimp[0].vid>0) {
			// consider each vertex currently in pSAAS list
#ifndef DEBUG_TW
			VertexList *curr = potentialSimpAndAlmostSimp[0].next;
			while (curr!=NULL) {
				// get next vert in list now, in case curr invalidated by elimination
				VertexList *next = curr->next;
#else
			VertexList *curr = NULL;
			for (int i=1; i<potentialSimpAndAlmostSimp.size(); i++) {
				while (!potentialSimpAndAlmostSimp[i].active) i++;
				if (i>=potentialSimpAndAlmostSimp.size())
					break;
				curr = &potentialSimpAndAlmostSimp[i];
				assert(curr->active);
				assert(curr->vid==i);
#endif
				// if no state given, or vertex eliminated in given state
				if (s==NULL || s->checkVert(curr->vid)) {
					// determine if either rule applies
					simp_ret simp = getSimpState(curr->vid);
					// apply that rule
					if (simp.state==SIMP) {
						ret.elimVerts.push_back(curr->vid);
						int deg = adjLM[curr->vid][0].vid;
						elimVertexSimp(curr->vid);
						anyElim=true;
						if (deg>ret.maxdeg) {
							ret.maxdeg=deg;
							if (ret.maxdeg>=ub)
								return ret;
						}
#ifndef MONOTONIC_H
						if (last_deg<deg) // need this because lb was not updated (i.e. it's an lb for an earlier node)
							last_deg=deg;
#endif
					} else if (simp.state==ALSIMP) {
						if (anyElim) { // recompute lb
#ifdef MONOTONIC_H
							ret.hval = heuristic(hversion,ub);
#else
							int hval_tmp = heuristic(hversion,ub);
							if (last_deg>=ret.hval || ret.hval<hval_tmp)
								ret.hval = hval_tmp;
#endif
							if (ret.hval>=ub)
								return ret;
							if (ret.hval>lb)
								lb=ret.hval;
							anyElim=false;
						}
						int deg = adjLM[curr->vid][0].vid;
						if (deg<=lb) {
							ret.elimVerts.push_back(curr->vid);
							elimVertexAlSimp(curr->vid,simp.dvert);
							anyElim=true;
							if (deg>ret.maxdeg)
								ret.maxdeg=deg;
						}
#ifndef MONOTONIC_H
						last_deg=deg;
#endif
					}
				}
				if (curr->active) {
					// remove vert from pSAAS
					removeVertex_fromSimpList(curr->vid);
				}
#ifndef DEBUG_TW
				curr=next;
#endif
			}
		}
		if (anyElim) {
#ifdef MONOTONIC_H
			ret.hval = heuristic(hversion,ub);
#else
			int hval_tmp = heuristic(hversion,ub);
			if (last_deg>=ret.hval || ret.hval<hval_tmp)
				ret.hval = hval_tmp;
#endif
		}
		return ret;
	}


///////////////////////////////////////////////////////////////////////////////
// another version of reduceGraphAll that tracks edges added to and removed
// from the graph. removed_edges and added_edges are arrays that are at least
// as long as the number of vertices remaining in the graph. when the ith
// vertex is eliminated from the graph, removed_edges[i] stores the indices of
// each vertex it had an adjacent edge to, and added_edges[i] stores the pairs
// of indices corresonding to each edge that was added to the graph.
// Other parameters:
//  - prunelist - set of indices of vertices that should be tested for
//    reduction first. if any vertex in the list is (almost) simplicial then
//    return without eliminating it, i.e., "prune".
//  - first_in_list - a flag that is set to true if a vertex in prunelist was
//    found to be (almost) simplicial
///////////////////////////////////////////////////////////////////////////////
reduce_all_ret ALMGraph::reduceGraphAll(int lb, HeuristicVersion hversion,
		int ub, vector<int> removed_edges[],
		vector<pair<int, int> > added_edges[],
		const boost::dynamic_bitset<> &prunelist,
		bool &first_in_list)
{
#ifndef MONOTONIC_H
	int last_deg = 0;
#endif
	reduce_all_ret ret;
	ret.maxdeg = 0;
	ret.hval = 0;
	uint nelim = 0;
	bool anyElim = false; // any elim since last time lb was calculated?

#ifdef DEBUG_TW
	// consider vertices in lexicographic order
	VertexList *curr = NULL;
	for (size_t i = 1; i < potentialSimpAndAlmostSimp.size(); ++i)
	  if (potentialSimpAndAlmostSimp[i].active)
	    {
	      curr = &potentialSimpAndAlmostSimp[i];
	      assert(curr->vid == i);
	      break;
	    }
	assert(curr != NULL);
	assert(curr->active);
#else
	VertexList *curr = potentialSimpAndAlmostSimp[0].next;
#endif

	size_t prune_pos = prunelist.find_first();
	for (; prune_pos != prunelist.npos; 
	     prune_pos = prunelist.find_next(prune_pos))
	  {
	    assert(prune_pos > 0 && 
		   prune_pos < potentialSimpAndAlmostSimp.size());
	    if (potentialSimpAndAlmostSimp[prune_pos].active)
	      {
		curr = &potentialSimpAndAlmostSimp[prune_pos];
		break;
	      }
	  }

	while (curr != NULL)
	{
		// get next vert in list now, in case curr->next invalidated by elimination
		VertexList *next;
		if (prune_pos != prunelist.npos)
			next = NULL;
		else
		  {
#ifdef DEBUG_TW
		    next = NULL;
#else
		    next = curr->next;
#endif
	       }

#ifdef DEBUG_TW
		assert(curr->active);
		assert(next==NULL || next->active);
		assert(cnlist.empty());
#endif
		// determine if either rule applies
		simp_ret simp = getSimpState(curr->vid);
		// apply that rule
		if (simp.state == SIMP)
		{
			ret.elimVerts.push_back(curr->vid);
			int deg = adjLM[curr->vid][0].vid;

			// if vert in prunelist, prune
			if (prune_pos != prunelist.npos)
			{
			  assert(prune_pos == curr->vid);
			  assert(!anyElim);
			  ret.maxdeg = deg;
			  first_in_list = true;
			  return ret;
			}

			elimVertexSimp(curr->vid, &removed_edges[nelim]);
			nelim++;
			anyElim = true;
			if (deg > ret.maxdeg)
			{
				ret.maxdeg = deg;
				if (ret.maxdeg >= ub)
					return ret;
			}
#ifndef MONOTONIC_H
			if (last_deg < deg) // need this because lb was not updated (i.e. it's an lb for an earlier node)
				last_deg = deg;
#endif
		}
		else if (simp.state == ALSIMP)
		{
			if (anyElim)
			{ // recompute lb
#ifdef MONOTONIC_H
				ret.hval = heuristic(hversion,ub);
#else
				int hval_tmp = heuristic(hversion, ub);
				if (last_deg >= ret.hval || ret.hval < hval_tmp)
					ret.hval = hval_tmp;
#endif
				if (ret.hval >= ub)
					return ret;
				if (ret.hval > lb)
					lb = ret.hval;
				anyElim = false;
			}
			int deg = adjLM[curr->vid][0].vid;
			if (deg <= lb)
			{
				ret.elimVerts.push_back(curr->vid);

				// if vert in prunelist, prune
				if (prune_pos != prunelist.npos)
				{
				  assert(prune_pos == curr->vid);
				  assert(!anyElim);
				  ret.maxdeg = deg;
				  first_in_list = true;
				  return ret;
				}

				elimVertexAlSimp(curr->vid, simp.dvert, &removed_edges[nelim],
						&added_edges[nelim], ub);
				nelim++;
				anyElim = true;
				if (deg > ret.maxdeg)
					ret.maxdeg = deg;
			}
#ifndef MONOTONIC_H
			last_deg = deg;
#endif
		}

		if (curr->active)
		{
			// remove vert from pSAAS
			removeVertex_fromSimpList(curr->vid);
		}

		// add edges with "common neighbor" rule
		if (graph_cnt_mode == TRACK_CN && !cnlist.empty())
			addCommonNeighborEdges(ub, &added_edges[nelim - 1]);

		// Determine which vertex to consider next for reduction
		if (prune_pos != prunelist.npos)
		  {
		    for (prune_pos = prunelist.find_next(prune_pos); 
			 prune_pos != prunelist.npos; 
			 prune_pos = prunelist.find_next(prune_pos))
		      {
			assert(prune_pos > 0 && 
			       prune_pos < potentialSimpAndAlmostSimp.size());
			if (potentialSimpAndAlmostSimp[prune_pos].active)
			  {
			    next = &potentialSimpAndAlmostSimp[prune_pos];
			    break;
			  }
		      }
		  }
		if (next == NULL)
		  {
#ifdef DEBUG_TW
		    // consider vertices in lexicographic order
		    curr = NULL;
		    for (size_t i = 1; i < potentialSimpAndAlmostSimp.size(); ++i)
		      if (potentialSimpAndAlmostSimp[i].active)
			{
			  curr = &potentialSimpAndAlmostSimp[i];
			  assert(curr->vid == i);
			  break;
			}
#else
		    curr = potentialSimpAndAlmostSimp[0].next;
#endif
		  }
		else
			curr = next;
		assert(curr == NULL || curr->active);
	}

	if (anyElim)
	{
#ifdef MONOTONIC_H
		ret.hval = heuristic(hversion,ub);
#else
		int hval_tmp = heuristic(hversion, ub);
		if (last_deg >= ret.hval || ret.hval < hval_tmp)
			ret.hval = hval_tmp;
#endif
	}
	return ret;
}


/*
///////////////////////////////////////////////////////////////////////////////
// ALMGraph reduceGraphAll_fromSubset - same as reduceGraphAll except vertices
//   eligible for elimination are limited to those already eliminated in State
//   s.
///////////////////////////////////////////////////////////////////////////////
	reduce_all_ret ALMGraph::reduceGraphAll_fromSubset (const State &s, int lb, MMDPlusVersion hversion, int ub)
	{
#ifdef DEBUG_ALMGRAPH
		assert(potentialSimpAndAlmostSimp[0].vid>0);
#endif
#if not defined MONOTONIC_H
		int last_deg=0;
#endif
		reduce_all_ret ret;
		ret.maxdeg=0;
		ret.hval=0;
		bool anyElim=false; // any elim since last time lb was calculated?
		// while there are still vertices in pSAAS list
		while (potentialSimpAndAlmostSimp[0].vid>0) {
			// consider each vertex currently in pSAAS list
			VertexList *curr = potentialSimpAndAlmostSimp[0].next;
			while (curr!=NULL) {
				// get next vert in list now, in case curr invalidated by elimination
				VertexList *next = curr->next;
				if (s.checkVert(curr->vid)) { // only consider vertices set in state s
					// determine if either rule applies
					simp_ret simp = getSimpState(curr->vid);
					// apply that rule
					if (simp.state==SIMP) {
						ret.elimVerts.push_back(curr->vid);
						int deg = adjLM[curr->vid][0].vid;
						elimVertexSimp(curr->vid);
						anyElim=true;
						if (deg>ret.maxdeg) {
							ret.maxdeg=deg;
							if (ret.maxdeg>=ub)
								return ret;
						}
#if not defined MONOTONIC_H
						if (last_deg<deg) // need this because lb was not updated (i.e. it's an lb for an earlier node)
							last_deg=deg;
#endif
					} else if (simp.state==ALSIMP) {
						if (anyElim) { // recompute lb
#ifdef MONOTONIC_H
							ret.hval = hMMDplus(hversion,ub);
#else
							int hval_tmp = hMMDplus(hversion,ub);
							if (last_deg>=ret.hval || ret.hval<hval_tmp)
								ret.hval = hval_tmp;
#endif
							if (lb>=ub) {
								ret.lb=lb;
								return ret;
							}
							anyElim=false;
						}
						int deg = adjLM[curr->vid][0].vid;
						if (deg<=lb) {
							ret.elimVerts.push_back(curr->vid);
							elimVertexAlSimp(curr->vid,simp.dvert);
							anyElim=true;
							if (deg>ret.maxdeg)
								ret.maxdeg=deg;
						}
#if not defined MONOTONIC_H
						last_deg=deg;
#endif
					}
				}
				if (curr->active) {
					// remove vert from pSAAS
					removeVertex_fromSimpList(curr->vid);
				}
				curr=next;
			}
		}
		if (anyElim) {
#ifdef MONOTONIC_H
			lb = hMMDplus(hversion,ub);
#else
			int lb_tmp = hMMDplus(hversion,ub);
			if (last_deg>=lb || lb<lb_tmp)
				lb = lb_tmp;
#endif
		}
		ret.lb=lb;
		return ret;
	}
*/

 

int ALMGraph::findReduceableVertex
  (int lb, const boost::dynamic_bitset<> &check_first) const
 {
   for (size_t pos = check_first.find_first(); pos != check_first.npos; 
	pos = check_first.find_next(pos))
     {
       // determine if any reduction rule applies to vertex pos
       simp_ret simp = getSimpState(pos);
       if (simp.state == SIMP || 
	   (simp.state == ALSIMP && adjLM[pos][0].vid <= lb))
	 {
	   return pos;
	 } 
     }       

#ifdef DEBUG_TW
   for (size_t i = 1; i < potentialSimpAndAlmostSimp.size(); ++i)
     {
       const VertexList *v = NULL;
       if (potentialSimpAndAlmostSimp[i].active)
	 v = &potentialSimpAndAlmostSimp[i];
       else
	 continue;
#else
   for (VertexList *v = potentialSimpAndAlmostSimp[0].next; v != NULL; 
	v = v->next)
     {
#endif
       // skip any verts in check_first because they have already been
       // checked
       assert(check_first.empty() || v->vid < check_first.size());
       if (!check_first.empty() && check_first[v->vid])
	 continue;

       // determine if any reduction rule applies to vertex v
       simp_ret simp = getSimpState(v->vid);
       if (simp.state == SIMP || 
	   (simp.state == ALSIMP && adjLM[v->vid][0].vid <= lb))
	 {
	   return v->vid;
	 } 
     }
   return -1;
 }


///////////////////////////////////////////////////////////////////////////////
// ALMGraph countEdges - returns the number of unique, undirected edges
///////////////////////////////////////////////////////////////////////////////
	int ALMGraph::countEdges() const
	{
		int count=0;
		VertexList *v = vertices[0].next;
		while (v!=NULL) {
			VertexList *w = adjLM[v->vid][0].next;
			while (w!=NULL) {
				if (v->vid < w->vid)
					count++;
				w=w->next;
			}
			v=v->next;
		}
		return count;
	}



///////////////////////////////////////////////////////////////////////////////
// ALMGraph getVertsSet - returns a set that includes the id of each vertex in
//   graph
///////////////////////////////////////////////////////////////////////////////
	set<int> ALMGraph::getVertsSet()
	{
		set<int> v;
		VertexList *curr = vertices[0].next;
		while (curr!=NULL) {
			v.insert(curr->vid);
			curr=curr->next;
		}
		return v;
	}

///////////////////////////////////////////////////////////////////////////////
// ALMGraph getVertVector - sets given vector to include all vertices in
// graph.
///////////////////////////////////////////////////////////////////////////////
	void ALMGraph::getVertVector (vector<int> &dest)
	{
		dest.clear();
		dest.reserve(nVerts);
		VertexList *curr = vertices[0].next;
		while (curr!=NULL) {
			dest.push_back(curr->vid);
			curr=curr->next;
		}
	}

	void ALMGraph::getNeighborhoodVector (int vid, vector<int> &dest)
	{
		dest.clear();
		dest.reserve(adjLM[vid][0].vid);
		VertexList *curr = adjLM[vid][0].next;
		while (curr!=NULL)
		{
			dest.push_back(curr->vid);
			curr = curr->next;
		}
	}

///////////////////////////////////////////////////////////////////////////////
// ALMGraph findVertex - Returns the index of a vertex in the graph chosen
// according to the specified method, omitting vertices in the excluded set.
///////////////////////////////////////////////////////////////////////////////
	int ALMGraph::findVertex(FindVertexMethod method,
			const VertexSet &excluded) const
	{
		int vid = -1;
		switch (method) {
		case FIND_MINFILL:
			vid = findMinFillVert(&excluded);
			break;
		case FIND_MINDEG:
			vid = findMinDegVert(&excluded);
			break;
		case FIND_RANDOM:
			vid = findRandomVert(&excluded);
			break;
		case FIND_ARBITRARY:
			for (VertexList *v = vertices[0].next; v!=NULL; v = v->next)
			{
				if (!excluded.member(v->vid))
				{
					vid = v->vid;
					break;
				}
			}
			assert(vid != -1);
			break;
		case FIND_FIXED:
		  for (size_t v = 1; v < vertices.size() && vid == -1; ++v)
		    {
		      if (vertices[v].active && !excluded.member(v))
			vid = v;
		    }
		  break;
		default:
			assert(false);
		}
		return vid;
	}

///////////////////////////////////////////////////////////////////////////////
// ALMGraph populateCommonNeighborList_SLOW - function clears cnlist and fills
// it with pairs of vertices that currently satify the common neighbor edge
// addition rule of (Gogate and Dechter 04), section 5.3. Rule says that if
// two vertices have >=ub common neighbors, then an edge can be added between
// them without affecting the graph's treewidth.
// NOTE: this function relies on the commonNeighbors member data, therefore it
// will only run if graph_cnt_mode == TRACK_CN, otherwise it does nothing.
///////////////////////////////////////////////////////////////////////////////
	void ALMGraph::populateCommonNeighborList_SLOW(int ub)
	{
		if (graph_cnt_mode!=TRACK_CN)
			return

		cnlist.clear();
		VertexList *v = vertices[0].next;
		while (v!=NULL) {
			VertexList *w = v->next;
			while (w!=NULL) {
				if (!adjLM[v->vid][w->vid].active) {
					if (v->vid < w->vid &&
							commonNeighbors[v->vid][w->vid] >= ub)
						cnlist.push_back(pair<int,int>(v->vid,w->vid));
					else if (commonNeighbors[w->vid][v->vid] >= ub)
						cnlist.push_back(pair<int,int>(w->vid,v->vid));
				}
				w = w->next;
			}
			v = v->next;
		}
	}


///////////////////////////////////////////////////////////////////////////////
// ALMGraph addCommonNeighborEdges - function invokes the "edge addition rule"
// to add edges between any pairs of vertices in cnlist.
///////////////////////////////////////////////////////////////////////////////
	void ALMGraph::addCommonNeighborEdges(int ub,
			vector<pair<int,int> > *added_edges)
	{
		if (graph_cnt_mode!=TRACK_CN)
			return;

		// Iterate through list, adding edges
		while (!cnlist.empty()) {

			// get and remove a pair
			int vid = cnlist.front().first;
			int wid = cnlist.front().second;
			cnlist.pop_front();
#ifdef DEBUG_ALMGRAPH
			assert(vid<wid);
#endif

			// add edge
			// note: edge may have become active since it was added to cnlist
			// note: # common neighbors may have decreased since it was added
			//       (e.g., if node pruned before cn edges added)
			if (!adjLM[vid][wid].active && commonNeighbors[vid][wid]>=ub) {
				addEdge(vid, wid, TRACK_CN, ub);
				if (added_edges!=NULL)
					added_edges->push_back(pair<int,int>(vid,wid));
				}
		}
	}

	bool ALMGraph::operator==(const ALMGraph &g) const
	{
		if (nVerts != g.nVerts)
			return false;
		VertexList *v = vertices[0].next;
		while (v != NULL)
		{
			if (!g.vertices[v->vid].active)
				return false;
			VertexList *w = adjLM[v->vid][0].next;
			while (w != NULL)
			{
				if (!g.adjLM[v->vid][w->vid].active)
					return false;
				w = w->next;
			}
			v = v->next;
		}
		return true;
	}

	bool ALMGraph::operator!=(const ALMGraph &g) const
	{
		return !(*this == g);
	}



	void ALMGraph::print_edges(ostream &out) const
	{
		out << nVerts << " " << countEdges() << endl;
		VertexList *v = vertices[0].next;
		while (v!=NULL) {
			VertexList *w = v->next;
			while (w!=NULL) {
				if (adjLM[v->vid][w->vid].active)
					out << v->vid << " " << w->vid << endl;
				w=w->next;
			}
			v=v->next;
		}
	}

	void ALMGraph::print_graph(ostream &out) const
	{
		for (uint v=1; v<vertices.size(); ++v)
		{
			if (vertices[v].active)
			{
				out << v << ": ";
				for (uint w=1; w<adjLM[v].size(); ++w)
				{
					if (adjLM[v][w].active)
						out << w << " ";
				}
				out << endl;
			}
		}
	}

};
