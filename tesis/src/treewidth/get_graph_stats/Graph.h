#include <set>
#include <map>
#include <iterator>
#include <algorithm>
#include <vector>
#include <climits>

using namespace std;

bool graph_h_verbose_mode = false;

////////////////////////////////////////////////////////////
// Adjacency-list graph class
////////////////////////////////////////////////////////////
class Graph
{
 public:
  typedef ushort vertex_type;

 private:
  typedef set<vertex_type> vertex_set;
  typedef map<vertex_type,vertex_set> adj_list;

 public:

#ifndef NDEBUG
  void verify() const
  {
    for (adj_list::const_iterator i = m_al.begin(); i != m_al.end(); ++i)
      {
	vertex_type v = i->first;
	for (vertex_set::const_iterator j = i->second.begin();
	     j != i->second.end(); ++j)
	  {
	    vertex_type w = *j;
	    assert(v != w);
	    adj_list::const_iterator x = m_al.find(w);
	    assert(x != m_al.end());
	    assert(x->second.count(v) == 1);
	  }
      }
  }
#endif
  
  //////////////////////////////
  // load graph from an "edge list" file where
  //  1. first line is:
  //       nv ne
  //     where nv in number of vertices, and ne is number of edges
  //  2. each subsequent line is:
  //       v w
  //     where vertices v and w are adjacent
  //  3. smallest vertex index should be 1
  //  4. largest vertex index should be nv
  //////////////////////////////
  void load(char *filename)
  {
    ifstream fin(filename);
    assert(fin.is_open());

    uint nv, ne;
    fin >> nv >> ne;
    
    uint v1, v2, cnt=0;
    while (fin >> v1 >> v2)
      {
	assert(v1>0 && v1<=nv);
	assert(v2>0 && v2<=nv);
	assert(m_al[v1].count(v2)==0);
	assert(m_al[v2].count(v1)==0);
	cnt++;
	m_al[v1].insert(v2);
	m_al[v2].insert(v1);
      }
    assert(cnt == ne);
/*     if (!(m_al.size() == nv)) */
/*       cerr << m_al.size() << " " << nv << endl; */
/*     assert(m_al.size() == nv); */
  }

  //////////////////////////////
  // contract the edge between vertices v and w
  //////////////////////////////
  void contractEdge(vertex_type &v, vertex_type &w)
  {
    assert(m_al.count(v)==1 && m_al[v].count(w)==1);
    assert(m_al.count(w)==1 && m_al[w].count(v)==1);
    
    adj_list::iterator vi = m_al.find(v);
    adj_list::iterator wi = m_al.find(w);

    // move edges from w to v
    for (vertex_set::iterator x = wi->second.begin(); x != wi->second.end(); 
	 ++x)
      {
	assert(m_al.count(*x)==1 && m_al[*x].count(w)==1);
	adj_list::iterator xi = m_al.find(*x);
	xi->second.erase(w);
	if (*x != v)
	  {
	    xi->second.insert(v);
	    vi->second.insert(*x);
	  }
      }
    // erase w
    m_al.erase(wi);

    assert(vi->second.count(w)==0);
  }

  //////////////////////////////
  // eliminate vertex v, and insert fill-in edges into fillins
  //////////////////////////////
  void eliminate(vertex_type v, map<vertex_type,uint> *fillins=NULL)
  {
    adj_list::iterator vi = m_al.find(v);
    assert(vi != m_al.end());
    // for each neighbor, remove v and add edges to other neighbors
    for (vertex_set::iterator w = vi->second.begin(); 
	 w != vi->second.end(); ++w)
      {
	adj_list::iterator wi = m_al.find(*w);
	assert(wi != m_al.end());
	// remove v
	wi->second.erase(v);
	// add edges to other neighbors of v
	vertex_set::iterator x = w;
	for (x++; x != vi->second.end(); ++x)
	  {
	    adj_list::iterator xi = m_al.find(*x);
	    assert(xi != m_al.end());
	    wi->second.insert(*x);
	    xi->second.insert(*w);
	  }
      }
    // update fillins
    if (fillins != NULL)
      {
	assert(fillins->count(v)==1);
	fillins->erase(v);
	for (vertex_set::iterator w = vi->second.begin();
	     w != vi->second.end(); ++w)
	  {
	    assert(fillins->count(*w)==1);
	    (*fillins)[*w] = fillin(*w);
	  }
/* 	vertex_set ws; */
/* 	for (vertex_set::iterator w = vi->second.begin(); */
/* 	     w != vi->second.end(); ++w) */
/* 	  { */
/* 	    assert(fillins->count(*w)==1); */
/* 	    ws.insert(*w); */
/* 	    adj_list::iterator wi = m_al.find(*w); */
/* 	    for (vertex_set::iterator x = wi->second.begin(); */
/* 		 x != wi->second.end(); ++x) */
/* 	      { */
/* 		assert(fillins->count(*x)==1); */
/* 		ws.insert(*x); */
/* 	      } */
/* 	  } */
/* 	for (vertex_set::iterator w = ws.begin(); w != ws.end(); ++w) */
/* 	  { */
/* 	    (*fillins)[*w] = fillin(*w); */
/* 	  } */
      }
    // delete v
    m_al.erase(vi);
  }

  //////////////////////////////
  // print the adjacency lists for the graph
  //////////////////////////////
  void print(ostream &out) const
  {
    for (adj_list::const_iterator v = m_al.begin(); v != m_al.end(); ++v)
      {
	out << (uint)v->first << ": ";
	copy(v->second.begin(), v->second.end(), 
	     ostream_iterator<uint>(out," "));
	out << endl;
      }
  }

  //////////////////////////////
  // print the graph in "edge list graph" format, described above
  //////////////////////////////
  void print_elg(ostream &graph_out, ostream *vmap_out) const
  {
    // count number of edges
    uint ne = 0;
    for (adj_list::const_iterator v = m_al.begin(); v != m_al.end(); ++v)
      {
	ne += v->second.size();
      }    
    assert(ne%2==0);
    ne /= 2;

    // output elg header
    graph_out << m_al.size() << " " << ne << endl;

    // map vertices
    map<vertex_type,vertex_type> vmap;
    vector<vertex_type> new_to_old_vmap;
    uint vcnt = 1;
    for (adj_list::const_iterator v = m_al.begin(); v != m_al.end(); ++v)
      {
	vmap[v->first] = vcnt++;
	new_to_old_vmap.push_back(v->first);
      }
    
    // output new-to-old vertex index map
    assert(new_to_old_vmap.size()==vmap.size());
    if (vmap_out != NULL)
      {
	copy(new_to_old_vmap.begin(),new_to_old_vmap.end(),ostream_iterator<int>(*vmap_out," "));
	*vmap_out << endl;
      }

    for (adj_list::const_iterator v = m_al.begin(); v != m_al.end(); ++v)
      {
	assert(vmap.count(v->first)==1);
	vertex_type newv = vmap[v->first];
	for (vertex_set::const_iterator w = v->second.begin(); w != v->second.end(); ++w)
	  {
	    assert(vmap.count(*w)==1);
	    vertex_type neww = vmap[*w];
	    if (newv < neww)
	      {
		graph_out << newv << " " << neww << endl;
	      }
	  }
      }
  }

  //////////////////////////////
  // find a vertex with minimal degree (set min_deg_vertex), and its
  // degree (set min_deg)
  //////////////////////////////
  void findMinDegreeVertex(vertex_type &min_deg_vertex, uint &min_deg) const
  {
    min_deg = UINT_MAX;
    for (adj_list::const_iterator v = m_al.begin(); v != m_al.end(); ++v)
      {
	if (v->second.size() < min_deg)
	  {
	    min_deg_vertex = v->first;
	    min_deg = v->second.size();
	  }
      }
  }

  //////////////////////////////
  // return the number of vertices with degree > 0
  //////////////////////////////
  uint size() const
  {
    return m_al.size();
  }

  //////////////////////////////
  // return the number of edges
  //////////////////////////////
  uint num_edges() const
  {
    uint nedges = 0;
    for (adj_list::const_iterator v = m_al.begin(); v != m_al.end(); ++v)
      {
	nedges += v->second.size();
      }
    assert(nedges % 2 == 0);
    return nedges/2;
  }

  //////////////////////////////
  // find the vertex adjacent to v with the least number of neighbors
  // in common
  //////////////////////////////
  vertex_type findLeastCommonNeighbor(vertex_type v) const
  {
    vertex_type lcn_vertex = 0;
    uint lcn = UINT_MAX;
    
    adj_list::const_iterator vi = m_al.find(v);
    for (vertex_set::const_iterator wi = vi->second.begin(); 
	 wi != vi->second.end(); ++wi)
      {
	uint cn = countCommonNeighbors(v, *wi);
	if (cn < lcn)
	  {
	    lcn = cn;
	    lcn_vertex = *wi;
	  }
      }
    return lcn_vertex;
  }
  
  //////////////////////////////
  // return the number of neighbors in common between v and w
  //////////////////////////////
  uint countCommonNeighbors(vertex_type v, vertex_type w) const
  {
    vertex_set cns;
    adj_list::const_iterator vi = m_al.find(v);
    adj_list::const_iterator wi = m_al.find(w);
    set_intersection(vi->second.begin(), vi->second.end(), wi->second.begin(), 
		     wi->second.end(), inserter(cns, cns.begin()));
    return cns.size();
  }

  //////////////////////////////
  // insert the list of vertex indices into the insert iterator ii
  //////////////////////////////
  template<typename T>
  void getVertices(insert_iterator<T> ii) const
  {
    for (adj_list::const_iterator v = m_al.begin(); v != m_al.end(); ++v)
      *ii = v->first;
  }

  //////////////////////////////
  // insert the list of vertices adjacent to v into ii
  //////////////////////////////
  template<typename T>
  void getNeighbors(vertex_type v, insert_iterator<T> ii) const
  {
    adj_list::const_iterator vi = m_al.find(v);    
    assert(vi != m_al.end());
    copy(vi->second.begin(), vi->second.end(), ii);
  }

  //////////////////////////////
  // does either reduction rule (simplicial or almost simplicial)
  // apply to vertex v with lower bound lb
  //////////////////////////////
  bool reduceable(vertex_type v, uint lb) const
  {
/*     cerr << "      is " << v << " reduceable? lb=" << lb << endl; */

    bool any_not_in_clique = false;
    vertex_type not1 = 0, not2 = 0;
    adj_list::const_iterator vi = m_al.find(v);
    assert(vi != m_al.end());
    for (vertex_set::const_iterator w = vi->second.begin(); 
	 w != vi->second.end(); ++w)
      {
	adj_list::const_iterator wi = m_al.find(*w);    
	vertex_set::const_iterator x = w;
	for (++x; x != vi->second.end(); ++x)
	  {
/* 	    cerr << "        check " << *w << " " << *x << endl; */

	    if (wi->second.find(*x) == wi->second.end())
	      {
/* 		cerr << "          not adj\n"; */

		if (!any_not_in_clique)
		  {
/* 		    cerr << "          not simplicial, try almost. deg(v)=" << degree(v) << endl; */

		    if (degree(v) > lb)
		      return false;
		    any_not_in_clique = true;
		    not1 = *w;
		    not2 = *x;
		  }
		else if (not1 == *w || not1 == *x)
		  {
/* 		    cerr << "          " << not1 << " is vertex to exclude\n"; */

		    not2 = 0;
		  }
		else if (not2 == *w || not2 == *x)
		  {
/* 		    cerr << "          " << not2 << " is vertex to exclude\n"; */

		    not1 = 0;
		  }
		else
		  {
/* 		    cerr << "          not almost simplicial either\n"; */

		    return false;
		  }
	      }
	  }
      }
    return true;
  }

  //////////////////////////////
  // erase vertex v from the graph
  //////////////////////////////
  void erase(vertex_type v)
  {
    adj_list::iterator vi = m_al.find(v);
    assert(vi != m_al.end());
    for (vertex_set::iterator wi = vi->second.begin(); 
	 wi != vi->second.end(); ++wi)
      {
	assert(m_al.count(*wi)==1);
	m_al[*wi].erase(v);
      }
    m_al.erase(v);
  }

  //////////////////////////////
  // return the degree of vertex v
  //////////////////////////////
  size_t degree(vertex_type v) const
  {
    assert(m_al.count(v)==1);
    return m_al.find(v)->second.size();
  }

  //////////////////////////////
  // return the number of fill-in edges that would be added if v were
  // eliminated
  //////////////////////////////
  uint fillin(vertex_type v) const
  {
    uint fill = 0;
    adj_list::const_iterator vi = m_al.find(v);
    assert(vi != m_al.end());
    for (vertex_set::const_iterator w = vi->second.begin();
	 w != vi->second.end(); ++w)
      {
	adj_list::const_iterator wi = m_al.find(*w);
	assert(wi != m_al.end());
	vertex_set::const_iterator x = w;
	for (++x; x != vi->second.end(); ++x)
	  {
	    assert(m_al.count(*x) == 1 && 
		   wi->second.count(*x) == m_al.find(*x)->second.count(*w));
	    if (wi->second.count(*x)==0)
	      {
		fill++;
	      }
	  }
      }
    return fill;
  }

  //////////////////////////////
  // return a vertex with minimal fill-in number
  //////////////////////////////
  vertex_type getMinFillVertex() const
  {
    vertex_type minfill_vertex;
    uint minfill = UINT_MAX;
    for (adj_list::const_iterator v = m_al.begin(); v != m_al.end(); ++v)
      {
	uint fill = fillin(v->first);
	if (fill < minfill)
	  {
	    minfill = fill;
	    minfill_vertex = v->first;
	  }
      }
    return minfill_vertex;
  }

  //////////////////////////////
  // fill a map with the fill-in number for each vertex in the graph
  //////////////////////////////
  void getFillins(map<Graph::vertex_type,uint> &fillins) const
  {
    assert(fillins.empty());
    for (adj_list::const_iterator v = m_al.begin(); v != m_al.end(); ++v)
      {
	fillins[v->first] = fillin(v->first);
      }    
  }

 private:
  adj_list m_al;
};


////////////////////////////////////////////////////////////
// Some functions that operate on a Graph
////////////////////////////////////////////////////////////

//////////////////////////////
// returns a lower bound on the treewidth of g, computed with the
// MMD+(least-c) lower bound
//////////////////////////////
uint mmdplus_leastc(Graph g)
{
  uint lb = 0;
  bool is_disconnected = false;
  while (true)
    {
      if (graph_h_verbose_mode)
	{
	  if (g.size()%100==0)
	    cerr << "  " << g.size() << " vertices left\n";
	}

      // get min degree vertex
      Graph::vertex_type v;
      uint vdeg;
      g.findMinDegreeVertex(v, vdeg);
      
      // if vertex has degree 0, delete it
      if (vdeg <= 0)
	{
	  if (graph_h_verbose_mode && !is_disconnected)
	    {
	      cerr << "NOTE: graph is disconnected\n";
	      is_disconnected = true;
	    }
	  g.erase(v);
	  continue;
	}
      
      // update lb
      if (lb < vdeg)
	lb = vdeg;
      if (lb >= g.size() - 1)
	break;

      // get least-common neighbor of v
      Graph::vertex_type w;
      w = g.findLeastCommonNeighbor(v);

      // contract edge
      g.contractEdge(v, w);
    }
  return lb;
}

//////////////////////////////
// eliminates vertex identified by the reduction rules (simplicial and
// almost simplicial). Sets prefix to the vertices eliminated in the
// reduction process.
//////////////////////////////
uint reduce(Graph &g, uint lb, vector<Graph::vertex_type> *prefix = NULL)
{
  assert(prefix==NULL || prefix->empty());

  bool any=true;
  uint maxdeg=0;
  while (any)
    {
      any = false;
      set<Graph::vertex_type> vertices;
      g.getVertices(inserter(vertices, vertices.begin()));
      for (set<Graph::vertex_type>::iterator vi = vertices.begin();
	   vi != vertices.end(); ++vi)
	{
	  Graph::vertex_type v = *vi;
	  if (g.reduceable(v,lb))
	    {
	      any = true;
	      uint deg = g.degree(v);
	      if (deg > lb)
		lb = deg;
	      if (deg > maxdeg)
		maxdeg = deg;
	      g.eliminate(v);
	      if (prefix != NULL)
		prefix->push_back(v);
	    }
	}
    }
  return lb;
}

//////////////////////////////
// finds an elimination order found with the min-fill heuristic,
// returned value is width of order
//////////////////////////////
uint minfill(Graph g, vector<Graph::vertex_type> *order = NULL)
{
  assert(order==NULL || order->empty());
  uint maxdeg = 0;

  // get the fill-in number for each vertex
  map<Graph::vertex_type,uint> fillins;
  g.getFillins(fillins);

  // until the graph is empty
  while (g.size() > 0)
    {
      if (graph_h_verbose_mode)
	{
	  if (g.size()%100==0)
	    cerr << "  " << g.size() << " vertices left\n";
	}

      // find the vertex with minimal fill-in value
      uint minfill = UINT_MAX;
      Graph::vertex_type v;
      for (map<Graph::vertex_type,uint>::iterator vf = fillins.begin();
	   vf != fillins.end(); ++vf)
	{
	  if (vf->second < minfill)
	    {
	      minfill = vf->second;
	      v = vf->first;
	    }
	}
      uint deg = g.degree(v);
      if (deg > maxdeg)
	maxdeg = deg;
      assert(fillins[v]==g.fillin(v));

      // after elimination only need to recompute fill-in numbers for
      // vertex adjacent to v or vertex adjacent to those
      // vertices. get a list of those vertices
      typedef set<Graph::vertex_type> vertex_set;
      vertex_set update_verts;
      g.getNeighbors(v, inserter(update_verts, update_verts.begin()));
      vertex_set neighbors = update_verts;
      for (vertex_set::iterator w = neighbors.begin(); 
	   w != neighbors.end(); ++w)
	{
	  g.getNeighbors(*w, inserter(update_verts, update_verts.begin()));
	}
      update_verts.erase(v);

      // eliminate v, and update order
      g.eliminate(v);
/*       g.eliminate(v, &fillins); */
      if (order != NULL)
	order->push_back(v);

      // update fill-in numbers for the update_verts
      for (vertex_set::iterator w = update_verts.begin();
	   w != update_verts.end(); ++w)
	{
	  fillins[*w] = g.fillin(*w);
	}
      fillins.erase(v);
    }
  return maxdeg;
}

//////////////////////////////
// finds an elimination order found with a heuristic inspired by a
// buggy implementation of min-fill, returned value is width of order
//////////////////////////////
uint buggy_minfill(Graph g, vector<Graph::vertex_type> *order = NULL)
{
  assert(order==NULL || order->empty());
  uint maxdeg = 0;

  // get the fill-in number for each vertex
  map<Graph::vertex_type,uint> fillins;
  g.getFillins(fillins);

  // until the graph is empty
  while (g.size() > 0)
    {
      if (graph_h_verbose_mode)
	{
	  if (g.size()%100==0)
	    cerr << "  " << g.size() << " vertices left\n";
	}

      // find the vertex with minimal fill-in value
      uint minfill = UINT_MAX;
      Graph::vertex_type v;
      for (map<Graph::vertex_type,uint>::iterator vf = fillins.begin();
	   vf != fillins.end(); ++vf)
	{
	  if (vf->second < minfill)
	    {
	      minfill = vf->second;
	      v = vf->first;
	    }
	}
      uint deg = g.degree(v);
      if (deg > maxdeg)
	maxdeg = deg;
      /* assert(fillins[v]==g.fillin(v)); */ // note that this does
					     // not hold for dowfill

      // after elimination only need to recompute fill-in numbers for
      // vertex adjacent to v. note that this is where dowfill is
      // different from minfill, because minfill also must update the
      // fill-in numbers for those two-degrees from v.
      typedef set<Graph::vertex_type> vertex_set;
      vertex_set update_verts;
      g.getNeighbors(v, inserter(update_verts, update_verts.begin()));
      update_verts.erase(v);

      // eliminate v, and update order
      g.eliminate(v);
      if (order != NULL)
	order->push_back(v);

      // update fill-in numbers for the update_verts
      for (vertex_set::iterator w = update_verts.begin();
	   w != update_verts.end(); ++w)
	{
	  fillins[*w] = g.fillin(*w);
	}
      fillins.erase(v);
    }
  return maxdeg;
}

//////////////////////////////
// returns the width of the given order in the given graph
//////////////////////////////
uint get_width(Graph g, const vector<Graph::vertex_type> &order)
{
  uint width = 0;
  for (uint i = 0; i < order.size(); ++i)
    {
      uint deg = g.degree(order[i]);
      if (deg > width)
	width = deg;
      g.eliminate(order[i]);
    }
  return width;
}
