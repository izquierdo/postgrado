// ALMGraph.h - header file for ALMGraph and associated classes
//
// ALMGraph is for adjacency list/matrix graph, because the graph is
// represented as both a matrx and a list. basically, the list elements
// exist at the corresponding location in the matrix.
//
// author: Alex Dow
// created on: 10/2/6

#ifndef ALMGRAPH_H_
#define ALMGRAPH_H_

#include "preproc_flags.h"
#include <vector>
#include <set>
#include <deque>
#include <utility>
#include <climits>
#include <boost/dynamic_bitset.hpp>
#include "GraphUtilities.h"
#include "TWState.h"
#include "BFHT_HDDD_Node.h"


namespace Adjacency_List_Matrix_Graph {
	using namespace std;
	using namespace __gnu_cxx;
	using namespace GraphUtilities;
	using Treewidth_Namespace::TWState;
	using Treewidth_Namespace::BFHT_HDDD_State;

	struct VertexList
	{
		VertexList *next;
		VertexList *prev;
		int vid;
		bool active;

		VertexList () : next(NULL), prev(NULL), vid(-1), active(false)
		 {}
		VertexList (int vertex_id) : next(NULL), prev(NULL), vid(vertex_id),
			active(false)
		 {}
	};

	/*
	class VertexListElem
	{
	private:
		VertexListElem *m_next;
		VertexListElem *m_prev;
		int m_vid;
		bool m_active;

	public:
		VertexListElem() :
			m_next(NULL), m_prev(NULL), m_vid(-1), m_active(false)
		{
		}
		VertexListElem(int vid) :
			m_next(NULL), m_prev(NULL), m_vid(vid), m_active(false)
		{
		}

		int vid() const
		{
			return m_vid;
		}

		bool active() const
		{
			return m_active;
		}

		const VertexListElem* next() const
		{
			return m_next;
		}
	};

	class VertexList
	{
	private:
		vector<VertexListElem> m_list;

	public:
		VertexList ()
		{
		}

		VertexList (int nverts)
		{
			m_list.reserve(nverts + 1);
			for (uint i = 0; i < nverts + 1; ++i)
			{
				m_list.push_back(VertexListElem(i));
			}
		}

		bool member(int v) const
		{
			return m_list[v].active();
		}

		const VertexListElem* front() const
		{
			return m_list[0].next();
		}
	};
	*/

	class VertexSet
	{
	private:
	  boost::dynamic_bitset<> m_set;
	  int m_count;
	public:
	  VertexSet(uint nverts)
	    : m_set(nverts + 1), m_count(0)
	    {
	      assert(nverts>0);
	      assert(m_set.count() == 0);
	    }
	  size_t size() const
	  {
	    return m_count;
	  }
	  size_t maxv() const
	  {
	    return m_set.size() - 1;
	  }
	  bool member(size_t v) const
	  {
	    assert(v>0 && v<m_set.size());
	    return m_set[v];
	  }
	  void insert(size_t v)
	  {
	    assert(v>0 && v<m_set.size());
	    if (!m_set[v])
	      {
		m_set.set(v);
		m_count++;
	      }
	  }
	  template<typename InputIterator>
	  void insert(InputIterator first, InputIterator last)
	  {
	    for (InputIterator it = first; it != last; ++it)
	      insert(*it);
	  }
	  void insert(const boost::dynamic_bitset<> &bits)
	  {
	    assert(m_set.size() == bits.size());
	    m_set |= bits;
	    m_count = m_set.count();
	  }
	  void remove(int v)
	  {
	    assert(v>0 && v<(int)m_set.size());
	    if (m_set[v]) 
	      {
		m_set.reset(v);
		m_count--;
	      }
	  }
	  void clear()
	  {
	    m_set.reset();
	    m_count = 0;
	  }
	  void print(ostream &out) const
	  {
	    for (size_t i=1; i<m_set.size(); i++)
	      if (m_set[i])
		cout << i << " ";
	  }
	};


	struct VertexDegLink
	{
		VertexDegLink *next;
		VertexDegLink *prev;
		int vid;
		bool active;
		int deg;

		VertexDegLink () : next(NULL), prev(NULL), vid(-1), active(false), deg(-1)
		 {}
	};


	class VertexDegList
	{
	private:
	public:
		vector<VertexDegLink> vertices; // can be made private
		vector<VertexDegLink*> degLists; // can be made private
		int mindeg;
		bool empty;

		VertexDegList ( int maxverts, int maxdeg );
		void addVertex (int vid, int deg );
		void changeDeg (int vid, int deg );
		void decDeg(int vid);
		void removeVertex (int vid);
		int getMinVert ();
		int getMinVert_randomized ();
	};


	struct reduce_one_ret {
		int deg; // degree of vertex eliminated, only valid if elimVert!=-1
		int elimVert; // vertex eliminated in reduction, -1 means none eliminated
	};

	struct reduce_all_ret {
		int maxdeg; // maximum degree among eliminated vertices
		int hval; // lower bound on tw of graph resulting from reduction
		vector<int> elimVerts; // vertices eliminated in reduction
	};

	enum SimpState { NONE, SIMP, ALSIMP };	// refers to whether a vertex is simplicial, almost simplicial, or neither
	struct simp_ret {
		SimpState state;
		int dvert; // disconnected neighbor vertex, if almost simplicial
	};


	class ElimOrder {
	private:
	public:
		int width;
		vector<int> order_prefix;
		set<int> indiff_suffix;

		ElimOrder () : width(-1) {}

		vector<int> getOrder () const;
		vector<int> getOrder (const vector<int> &prefix,
			const vector<int> &vertexmap) const;
		string printOrder() const;

		void remap (const vector<int> &vertexmap);
		void appendPrefix (const vector<int> &prefix);
	};

	enum CommonNeighborTrackingMode { DONT_TRACK_CN, TRACK_CN };
	enum MinDegTieBreaking { MINDEG_BREAK_TIES_ARBITRARILY, MINDEG_BREAK_TIES_RANDOMLY };
	enum MinFillTieBreaking { MINFILL_BREAK_TIES_ARBITRARILY, MINFILL_BREAK_TIES_RANDOMLY };
	enum FindVertexMethod { FIND_MINFILL, FIND_MINDEG, FIND_RANDOM, FIND_ARBITRARY, FIND_FIXED };
	enum ReduceGraphOrder { REDUCE_ARBITRARY, REDUCE_LEAST_FIRST };
	enum HeuristicVersion { H_MINDEG, H_MINPAIRMAX, H_DEGEN_MINDEG, H_DEGEN_MINPAIRMAX,
		H_CONTRACTION_MIND, H_CONTRACTION_LEASTC };
	enum DegenVersion { DEGEN_MINDEG, DEGEN_MINPAIRMAX, CONTRACTION_MIND,
		CONTRACTION_LEASTC };
	enum MapVerticesMethod { MAPVERTS_NONE, MAPVERTS_MINDEGREE_FIRST, MAPVERTS_MAXDEGREE_FIRST };

	class ALMGraph
	{
		private:

		void addEdge (int v, int w,
				CommonNeighborTrackingMode cntMode=DONT_TRACK_CN,
				int ub=INT_MAX, int exvid=0);
		void addDEdge (int v, int w);
		void addVertex_toVertexList (int v);
		void addVertex_toSimpList (int v);
		void removeEdge (int v, int w);
		void removeDEdge (int w, int v);
		void removeVertex_fromVertexList (int v);
		void removeVertex_fromSimpList (int v);
		void copyVertexList (const vector<VertexList> &src, vector<VertexList> &dst);
		void clearVertexList (vector<VertexList> &vlist);

		void elimVertexSimp (int vid, vector<int> *removed_edges=NULL); // eliminates a simplicial vertex
		void elimVertexAlSimp (int vid, int dvert,
				vector<int> *removed_edges=NULL,
				vector<pair<int,int> > *added_edges=NULL, int ub=INT_MAX); // eliminates an almost simplicial vertex

		int findMinFillVert (const VertexSet *excluded=NULL) const;
		int findMinFillVert_randomized () const;
		int findMinDegVert (const VertexSet *excluded=NULL) const;
		int findRandomVert (const VertexSet *excluded=NULL) const;

		int findMinDegreeNeighbor (int v, int mindeg);
		int findMinDegreeNeighbor_randomized (int v);
		int findLeastCommonNeighbor (int v);
		int findLeastCommonNeighbor_randomized (int v);

		int countCommonNeighbors(int vid, int wid) const;

		public:

		// element 0 of a vertex list is a ptr to the first vertex, therefore
		// a vertex list has nVerts+1 elements in it. also, for element 0, vid
		// field is a count of the number of active vertices in the list
		vector<vector<VertexList> > adjLM; // adjacency list/matrix
		vector<VertexList> vertices; // list of vertices
		int nVerts; // number of vertices
		vector<VertexList> potentialSimpAndAlmostSimp;
		vector<vector<int> > commonNeighbors;
		deque<pair<int,int> > cnlist;
		CommonNeighborTrackingMode graph_cnt_mode;

		ALMGraph (CommonNeighborTrackingMode m=DONT_TRACK_CN) : nVerts(0), graph_cnt_mode(m) {}

	    void load (const boolMatrix &matrix); // constructs graph from boolMatrx
		void init (int origVerts); // initializes empty graph with capacity for origVerts vertices
		void init_nopSAAS (int origVerts); // same as init w/o initializing pSAAS
		void copy (const ALMGraph &orig); // makes this graph a copy of orig without reallocation
		void copy_nopSAAS (const ALMGraph &orig); // same as copy, but excludes pSAAS list
		void normalize_SLOW (vector<int> &disconVerts, vector<int> &vertex_map); // removes discon verts and renumbers verts

		bool operator==(const ALMGraph &g) const;
		bool operator!=(const ALMGraph &g) const;

		void mapVertices(MapVerticesMethod method, vector<int> &map);

		void elimVertex (int vid, vector<int> *removed_edges=NULL,
				vector<pair<int,int> > *added_edges=NULL, int ub=INT_MAX,
				bool *removed_pSAAS=NULL, vector<int> *added_pSAAS=NULL);
		void elimVertex_nopSAAS (int vid); // eliminates a vertex without updating pSAAS list
		//void elimVertices_new (TWState s, const int *lastv=NULL,
		//		vector<int> *last_removed_edges=NULL, const int *pv=NULL,
		//		vector<int> *p_removed_edges=NULL);
		void elimVertices (TWState s, const int *lastv=NULL,
				vector<int> *last_removed_edges=NULL, const int *pv=NULL,
				vector<int> *p_removed_edges=NULL);
		//void elimVertices_nopSAAS (const TWState &s);
		void elimVertices (const BFHT_HDDD_State &s, const int *lastv=NULL,
				vector<int> *last_removed_edges=NULL);
		void reverseElimVertex (int vid, const vector<int> &removed_edges,
				const vector<pair<int,int> > &added_edges,
				bool removed_pSAAS=false,
				const vector<int> *added_pSAAS=NULL);
		void reverseElimVertices (const vector<int> vids,
				const vector<int> removed_edges[],
				const vector<pair<int,int> > added_edges[]);

		void elimSuffix (const TWState &s, uint suffix_len,
				vector<vector<int> > &removed_edges,
				vector<vector<pair<int,int> > > &added_edges,
				vector<bool> &removed_pSAAS,
				vector<vector<int> > &added_pSAAS);
		void reverseElimSuffix (const TWState &s, uint suffix_len,
				vector<vector<int> > &removed_edges,
				vector<vector<pair<int,int> > > &added_edges,
				vector<bool> &removed_pSAAS,
				vector<vector<int> > &added_pSAAS);

		void removeVertex_nopSAAS (int vid);
		void addVertex_nopSAAS (const ALMGraph &orig, int vid);

		reduce_one_ret reduceGraphOne (int lb, int ub=INT_MAX,
				const TWState *s=NULL, vector<int> *removed_edges=NULL,
				ReduceGraphOrder rgorder=REDUCE_ARBITRARY, int minvid=0,
				vector<bool> *nopruneverts=NULL);
		reduce_one_ret findReduceGraphOne (int lb, int ub=INT_MAX,
				ReduceGraphOrder rgorder=REDUCE_ARBITRARY, int minvid=0,
				vector<bool> *nopruneverts=NULL) const;
		reduce_all_ret reduceGraphAll (int lb, HeuristicVersion hversion, int ub=INT_MAX, const TWState *s=NULL);
		reduce_all_ret reduceGraphAll (int lb, HeuristicVersion hversion, int ub,
				vector<int> removed_edges[],
				vector<pair<int,int> > added_edges[],
				const boost::dynamic_bitset<> &noelimlist, 
				bool &first_in_list);
		simp_ret getSimpState (int vid) const; // NOTE: function can be made private

		int findReduceableVertex(int lb, const boost::dynamic_bitset<> &check_first = boost::dynamic_bitset<>()) const;

public:
		void contractEdge(int v, int w);

private:
		void contractEdge (int v, int w, VertexDegList &degList,
			int *remv=NULL, vector<int> *removed_edges=NULL, int *keptv=NULL,
			vector<int> *added_edges=NULL);
		void reverseContractEdge (int remv, vector<int> removed_edges,
			int keptv, vector<int> added_edges);
		void removeVertex(int vid, VertexDegList &degList);
public:

	int heuristic(HeuristicVersion hversion, int ub=INT_MAX,
		MinDegTieBreaking tiebreaking=MINDEG_BREAK_TIES_ARBITRARILY) const;
	int heuristic_UNSAFE(HeuristicVersion hversion, int ub=INT_MAX,
		MinDegTieBreaking tiebreaking=MINDEG_BREAK_TIES_ARBITRARILY);
	int hRandomized (HeuristicVersion version, int niterations,
			int ub=INT_MAX) const;
private:
		int hMinDeg() const;
		int hMinPairMax() const;
		int hDegen(DegenVersion version, int ub=INT_MAX,
			MinDegTieBreaking tiebreaking=MINDEG_BREAK_TIES_ARBITRARILY) const;
		int hDegen_UNSAFE(DegenVersion version, int ub=INT_MAX,
			MinDegTieBreaking tiebreaking=MINDEG_BREAK_TIES_ARBITRARILY);
public:

		ElimOrder ubRandomizedMinFill (int niterations);
		ElimOrder ubMinFill (MinFillTieBreaking tiebreaking=MINFILL_BREAK_TIES_ARBITRARILY);
		ElimOrder ubMinFill_UNSAFE (MinFillTieBreaking tiebreaking=MINFILL_BREAK_TIES_ARBITRARILY);

		int getWidth (const vector<int> &order);
		int getWidth_UNSAFE (const vector<int> &order);

		int countEdges () const;

		set<int> getVertsSet ();
		void getVertVector (vector<int> &dest);
		void getNeighborhoodVector (int vid, vector<int> &dest);

		int findVertex(FindVertexMethod method,
				const VertexSet &excluded) const;

		void populateCommonNeighborList_SLOW(int ub);
		void addCommonNeighborEdges(int ub,
				vector<pair<int,int> > *added_edges=NULL);

		void print_edges(ostream &out) const;

		void print_graph(ostream &out) const;
	};

};

#endif /*ALMGRAPH_H_*/
