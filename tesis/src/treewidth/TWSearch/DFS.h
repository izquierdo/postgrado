#ifndef DFS_H_
#define DFS_H_

//#define DEBUG_OUTPUT

// DFS_PRUNE_IVPR - Define to prune nodes based on the Independent
// Vertex Pruning Rule. This rule based on Thms 6.1-6.3 of Gogate and
// Dechter (2004), and is described in Section 8.5.1 of the
// dissertation.
#define DFS_PRUNE_IVPR

#ifdef DFS_PRUNE_IVPR
// DFS_PRUNE_DVPR - Define to prune nodes based on the Dependant
// Vertex Pruning Rule. This rule seeks a "good enough" order
// for eliminating some set of "dependent vertices," and then ensures
// that only that order is used as long as the neighborhoods of the
// pair don't change. This complements IVPR, which allows dependent
// vertices to be eliminated in any order. Thus, DVPR can only be
// enabled when IVPR is also enabled. This rule is described in
// Section 8.5.3 of the dissertation.
#define DFS_PRUNE_DVPR
#endif

#ifdef DFS_PRUNE_DVPR
#include <tr1/unordered_set>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/member.hpp>
using namespace boost::multi_index;
#endif

// DFS_PRUNE_AVDC - Define to prune nodes based on the Adjacent Vertex
// Dominance Criterion. This criterion is based on a result in Kloks,
// attributed to Dirac, that says it is unnecessary to eliminate
// adjacent vertices consecutively. This rule is described in Section
// 6.3 of the dissertation.
#define DFS_PRUNE_AVDC

// DFS_PRUNE_GRDC - Define to prune nodes based on the Graph Reduction
// Dominance Criteria. These criteria invoke the Simplicial and Almost
// Simplicial rules to determine if the immediate elimination of any
// vertex dominates any other vertex elimination. These rules were
// developed by Bodlaender, Koster, and Eijkhof (2005), and are
// described in Section 6.2 of the dissertation.
#define DFS_PRUNE_GRDC

// DFS_TT - Define to enable the usage of a transposition table for
// DFS. Every node that is chosen for expansion will be inserted into
// the TT. If a duplicate is already in the TT then that node will be
// pruned. This technique is discussed in Section 8.4 of the
// dissertation.
//#define DFS_TT

#ifdef DFS_TT
// DFS_TT_MB_LIMIT - Defines the maximum amount of memory (in
// megabytes) that the TT is allowed to use. After this memory is
// exhausted an LRU replacement strategy is used.
#define DFS_TT_MB_LIMIT 1800
#include "anyspace_hash_table.h"
#endif

#include <list>
#include <map>
#include <boost/dynamic_bitset.hpp>
#include "TWSearch.h"
#include "DFSStats.h"
#include "preproc_flags.h"

typedef boost::dynamic_bitset<> dynamic_bitset;  

namespace Treewidth_Namespace
{

  typedef unsigned char uchar;

  enum DFSNode_Type
  {
    DFS_ROOT, DFS_REDUCED
  };

  class DFSNode
  {
  private:

    // vertex eliminated to generate this node
    ushort m_vertexID; 

    // degree of vertex eliminated to generate this node
    ushort m_vertexDegree;

    // node's g-value, i.e., cost of path to node
    ushort m_gValue; 

    // node's h-value, i.e., estimate of cost from node to goal
    ushort m_hValue;

    // flag signifying whether node was generated as a result of a
    // graph reduction rule, if true then the g-value and h-value of
    // this node are undefined and can be ignored
    bool m_wasReduced;

  public:

    // Constructor for generating a child node, where child is the
    // result of eliminating vertex vidv, with degree vdeg, from node
    // parent. Also, lb is the heuristic lower bound for the new
    // child.
    DFSNode(DFSNode &parent, ushort vidv, ushort vdeg, ushort lb) 
      : m_vertexID(vidv), m_vertexDegree(vdeg), m_wasReduced(false)
      {
	m_gValue = (vdeg < parent.m_gValue) ? parent.m_gValue : vdeg;
#ifdef MONOTONIC_H
	m_hValue = lb;
#else
	if (vdeg < parent.m_hValue)
	  m_hValue = (lb < parent.m_hValue) ? parent.m_hValue : lb;
	else
	  m_hValue = lb;
#endif
      }

    // Constructor for generating the start (aka root) node, with an
    // initial lower bound of hValue. Note: any solution that is found
    // is guaranteed to a least-cost solution with cost >=
    // hValue. Thus, if hValue is greater than the minimum solution
    // cost, then the solution that is found may not be minimal.
    DFSNode(ushort hvalue)
      : m_vertexID(0), m_gValue(0), m_hValue(hvalue), m_vertexDegree(0), 
      m_wasReduced(false)
      {
      }

    ushort vid() const
    {
      return m_vertexID;
    }

    void setVid(ushort vidv)
    {
      m_vertexID = vidv;
    }

    ushort gValue() const
    {
      return m_gValue;
    }

    ushort hValue() const
    {
      return m_hValue;
    }

    ushort fValue() const
    {
      return (m_gValue > m_hValue) ? m_gValue : m_hValue;
    }

    ushort degree() const
    {
      return m_vertexDegree;
    }

    bool reduced() const
    {
      return m_wasReduced;
    }

    void setReduced()
    {
      m_wasReduced = true;
    }
  };

#ifdef DFS_PRUNE_DVPR
  class DVPRData
  {

    void add(int vid, int deg, int d, const vector<DFSNode> &path);
    /*   void check(int vid, int d, const vector<int> &adjverts, const vector<DFSNode> &path); */
    void check(int d, const vector<int> &adjverts, const vector<DFSNode> &path);

  private:
  
    // SequenceState represents the state of a Sequence, i.e., the set
    // of vertices in the sequence.
    class SequenceState
    {
    public:
      // constructor
    SequenceState()
      : m_hash(0)
	{
	  assert(m_bits.none());
	}
      // set vertex v, i.e., add it to the state
      void set(uint v)
      {
	assert(v > 0 && v <= TW_MAX_VERTS);
	assert(!m_bits[v - 1]);
	m_bits.set(v - 1);
	m_hash ^= (1 << ((v - 1) % (sizeof(size_t)*8)));
      }
      // return the state's hash value
      size_t hash() const
      {
	return m_hash;
      }
      // equivalence
      bool operator==(const SequenceState &s) const
      {
	return m_bits == s.m_bits;
      }
      // check whether t contains the same set of vertices as this state
      template<typename T>
	bool same_state(const T &t)
	{
	  if (t.size() != m_bits.count())
	    return false;
	  for (typename T::const_iterator iter = t.begin(); iter != t.end(); ++iter)
	    {
	      assert(*iter > 0);
	      assert(*iter <= TW_MAX_VERTS);
	      if (!m_bits[*iter - 1])
		return false;
	    }
	  return true;
	}
      // print
      void print(ostream &out)
      {
	for (uint i = 0; i < m_bits.size(); ++i)
	  if (m_bits[i])
	    out << i+1 << " ";
      }
    private:
      // bitset stores the state for quick lookup and update
      bitset<TW_MAX_VERTS> m_bits;
      // hash value of the state is computed and stored as the state is
      // modified, this seems to be faster than computing the hash value
      // every time it is needed and the space overhead isn't a big
      // deal.
      size_t m_hash;
    };

    // SequenceState hash lookup
    struct SequenceState_hash
    {
      size_t operator()(const SequenceState &s) const
      {
	return s.hash();
      }
    };

    // Sequence represents an ordered series of vertex eliminations and
    // a depth in the search space that is the shallowest at which it is
    // still valid. If the search backtracks beyond that depth, then
    // this Sequence is no longer valid and can be discarded.
    class Sequence
    {
    public:
      typedef vector<uchar>::const_iterator const_iterator;

    public:
      // construct a sequence with a range [first,last), depth depth,
      // and cost cost
      template<typename InputIterator>
	Sequence(InputIterator first, InputIterator last, uchar depth, 
		 uchar cost)
	: m_seq(first,last), m_depth(depth), m_cost(cost)
      { }
      const_iterator begin() const
      {
	return m_seq.begin();
      }
      const_iterator end() const
      {
	return m_seq.end();
      }
      uchar depth() const
      {
	return m_depth;
      }
      uchar cost() const
      {
	return m_cost;
      }
      template<typename T>
	bool seq_eq(const T &s) const
	{
	  if (s.size() != m_seq.size())
	    return false;
	  else
	    return equal(m_seq.begin(), m_seq.end(), s.begin());
	}
      // return true if this sequence is lexicographically less than
      // sequence represented by s
      template<typename T>
	bool seq_lt(const T &s) const
	{
	  assert(s.size() == m_seq.size());
	  pair<vector<uchar>::const_iterator,typename T::const_iterator> ret =
	    mismatch(m_seq.begin(), m_seq.end(), s.begin());
	  return ret.first != m_seq.end() && *ret.first < *ret.second;
	}

      void print_seq(ostream &out) const
      {
	copy(m_seq.begin(), m_seq.end(), ostream_iterator<int>(out," "));
	out << ": " << (int)m_depth << "," << (int)m_cost << endl;
      }

    private:
      // sequence
      vector<uchar> m_seq;
      // shallowest depth at which seq is valid
      uchar m_depth; 
      // cost of sequence
      uchar m_cost;
    };

    // less than predicate for comparing Sequence validity depth
    struct Sequence_depth_less
    {
      bool operator() (const Sequence &s, const Sequence &t) const
      {
	return s.depth() < t.depth();
      }
      bool operator() (const Sequence &s, uchar depth) const
      {
	return s.depth() < depth;
      }
      bool operator() (uchar depth, const Sequence &s) const
      {
	return depth < s.depth();
      }
    };

    static const int NOVERT = 0;
  
    typedef pair<SequenceState,Sequence> StateSeqPair;

    // SequenceTable is a multi_index_container that stores
    // StateSeqPairs indexed by SequenceState hash (state) and ordered
    // by Sequence depth (depth). Thus, we can look up a pair by the
    // unique state, or get all pairs where the Sequence has the same
    // depth.
    // NOTE: used
    // http://www.boost.org/doc/libs/1_36_0/libs/multi_index/example/bimap.cpp
    // as an expample. For some compilers something different must be
    // done for /member/ below.
    struct state { };
    struct depth { };
    typedef multi_index_container<
      StateSeqPair, 
      indexed_by<
      hashed_non_unique<
      tag<state>, 
      member<StateSeqPair,SequenceState,&StateSeqPair::first>,
      SequenceState_hash>, 
      ordered_non_unique<
      tag<depth>, 
      member<StateSeqPair,Sequence,&StateSeqPair::second>,
      Sequence_depth_less> 
      > > SequenceTable;

  private:
  
    SequenceTable m_table;
    SequenceTable m_temp_table;

    typedef SequenceTable::index_iterator<state>::type 
      SequenceTableStateIndexIterator;
    typedef SequenceTable::index_iterator<depth>::type 
      SequenceTableDepthIndexIterator;

    // m_adjverts_by_depth[d] is a bitset where bit i is set if vertex i
    // was adjacent to the vertex eliminated at depth d
    vector<dynamic_bitset> m_adjverts_by_depth;

    // m_noelimlist stores a list of vertices that would lead to nogood
    // sequences if they were eliminated after the last time
    // updateAfterElim was called. m_noelimlist_depth stores the depth
    // it corresponds to.
    dynamic_bitset m_noelimlist;
    int m_noelimlist_depth;

  public:

  DVPRData(uint nverts, uint depth_limit = UINT_MAX) 
    : m_adjverts_by_depth(nverts + 1, dynamic_bitset(nverts + 1)),
      m_noelimlist(nverts + 1)
	{ }

    void updateAfterElim(int vid, int deg, int depth, 
			 const vector<int> &adjverts,
			 const vector<DFSNode> &path);

    void updateAfterUnelim(int depth);

    void updateAfterPrune(int vid, int deg, int depth, 
			  const vector<int> &adjverts,
			  const vector<DFSNode> &path);

    void updateNoElimList(dynamic_bitset &noelimlist) const;

    void updateAfterNewUB(int ub);

    int get_noelimlist_depth() const
    {
      return m_noelimlist_depth;
    }
  };
#endif

#ifdef DFS_TT
  typedef anyspace_hash_table<ReplaceLRU<TWState> > DFSTransTable;
#endif

  enum DFSCutoffType
  {
    DFS_UB_CUTOFF, DFS_ID_CUTOFF
  };

  class DFS: public TWSearch
  {

  private:

    TWExitStatus exit_status_val;

    ElimOrder solution_val;

    DFSStats m_stats;

    bool run_val;

#if (defined DFS_PRUNE_IVPR) and (defined DFS_PRUNE_DVPR) and (not defined DFS_TT)
    void backtrack(ALMGraph &graph, 
		   vector<int> removed_edges[], 
		   vector<pair<int, int> > added_edges[], 
		   vector<VertexSet> &eclist, 
		   vector<dynamic_bitset> &noelimlist, 
		   vector<DFSNode> &path, 
		   int &d, 
		   ushort ub, 
		   DVPRData &dvsdata);
#elif (defined DFS_PRUNE_IVPR) and (not defined DFS_PRUNE_DVPR) and (not defined DFS_TT)
    void backtrack(ALMGraph &graph, vector<int> removed_edges[], 
		   vector<pair<int, int> > added_edges[], 
		   vector<VertexSet> &eclist, 
		   vector<dynamic_bitset> &noelimlist, 
		   vector<DFSNode> &path, int &d, ushort ub);
#elif (not defined DFS_PRUNE_IVPR) and (not defined DFS_TT)
    void backtrack(ALMGraph &graph, vector<int> removed_edges[], 
		   vector<pair<int, int> > added_edges[], 
		   vector<VertexSet> &eclist, vector<DFSNode> &path, int &d, 
		   ushort ub);
#elif (defined DFS_PRUNE_IVPR) and (defined DFS_PRUNE_DVPR) and (defined DFS_TT)
    void backtrack(ALMGraph &graph, vector<int> removed_edges[], 
		   vector<pair<int, int> > added_edges[], 
		   vector<VertexSet> &eclist, 
		   vector<dynamic_bitset> &noelimlist, 
		   vector<DFSNode> &path, int &d, ushort ub, 
		   DVPRData &dvsdata, TWState &state);
#elif (defined DFS_PRUNE_IVPR) and (not defined DFS_PRUNE_DVPR) and (defined DFS_TT)
    void backtrack(ALMGraph &graph, vector<int> removed_edges[], 
		   vector<pair<int, int> > added_edges[], 
		   vector<VertexSet> &eclist,
		   vector<dynamic_bitset> &noelimlist, 
		   vector<DFSNode> &path, int &d, ushort ub, 
		   TWState &state);
#else
    void backtrack(ALMGraph &graph, vector<int> removed_edges[], 
		   vector<pair<int, int> > added_edges[], 
		   vector<VertexSet> &eclist, vector<DFSNode> &path, int &d, 
		   ushort ub, TWState &state);
#endif

#ifdef DFS_PRUNE_IVPR
    void updateNoElimList(dynamic_bitset &noelimlist, 
			  dynamic_bitset &parent_noelimlist, int vid,
			  const vector<int> &adjacentverts
#ifdef DFS_PRUNE_DVPR
			  , const DVPRData &dvsdata
#endif
			  ) const;
#endif

    TWExitStatus solve_aux(ALMGraph &graph, ElimOrder &ub_order,
			   bool &ub_mapped, uint initiallb, HeuristicVersion hversion,
			   const struct timeval starttime, const int timelim);

  public:

    TWExitStatus solve(DFSCutoffType cutofftype, ElimOrder &soln,
		       const boolMatrix &adjmat, HeuristicVersion hversion, uint lb = 0,
		       uint ub = UINT_MAX, int timelim = INT_MAX, int memlim = INT_MAX);

    void writeResultFile(const char *outfile);

    void writeStatFile(const char *statfile);

  };

  class DFBnB: public DFS
  {
  public:
    TWExitStatus solve(ElimOrder &soln, const boolMatrix &adjmat,
		       HeuristicVersion hversion, uint lb = 0, uint ub = UINT_MAX,
		       int timelim = INT_MAX, int memlim = INT_MAX)
    {
      return DFS::solve(DFS_UB_CUTOFF, soln, adjmat, hversion, lb, ub,
			timelim, memlim);
    }
  };

  class DFID: public DFS
  {
  public:
    TWExitStatus solve(ElimOrder &soln, const boolMatrix &adjmat,
		       HeuristicVersion hversion, uint lb = 0, uint ub = UINT_MAX,
		       int timelim = INT_MAX, int memlim = INT_MAX)
    {
      return DFS::solve(DFS_ID_CUTOFF, soln, adjmat, hversion, lb, ub,
			timelim, memlim);
    }
  };

}

#endif /*DFBNB_H_*/
