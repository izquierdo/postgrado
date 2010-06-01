#include "preproc_flags.h"
#include "TWSearch.h"

namespace Treewidth_Namespace {

  /* When TW_SUCCESS returned, then ub_order is optimal solution. */
  TWExitStatus TWSearch::initSearch(const boolMatrix &adjmat,
				    HeuristicVersion hversion, ALMGraph &graph, vector<int> &elimprefix,
				    int &elimprefix_width, int &postprefix_lb, vector<int> &vertexmap,
				    ElimOrder &ub_order, bool &ub_mapped, TWStats &stats, uint lb,
				    uint ub) {

    // load graph
    graph.load(adjmat);
    stats.setOrigVerts(graph.nVerts);
    stats.setOrigEdges(graph.countEdges());

    // compute lower bound
    int tmp_lb = graph.heuristic(hversion);
    if (tmp_lb > (int)lb)
      lb = tmp_lb;
    stats.setInitialLB(lb);

    // apply "common neighbor" edge addition rule if selected
    if (ub < (uint)INT_MAX)
      ub_order.width = ub;
    else
      {
	cout << "No upper bound provided, computing one with minfill.\n";
	ub_order.width = INT_MAX;
      }
    ub_mapped = false;
    if (graph.graph_cnt_mode==TRACK_CN) {

      // compute upperbound
      if (ub >= (uint)INT_MAX)
	ub_order = graph.ubRandomizedMinFill(6);

      // add edges with "common neighbor" rule
      graph.populateCommonNeighborList_SLOW(ub_order.width);
      graph.addCommonNeighborEdges(ub_order.width);

      // recompute lower bound
      int lb_tmp = graph.hRandomized(hversion, 6);
      if (lb_tmp > (int)lb)
	lb = lb_tmp;
      if ((int)lb >= ub_order.width)
	return TW_SUCCESS;
    }

    // reduce graph
    elimprefix_width = 0;
    postprefix_lb = lb;
    reduce_all_ret raret = graph.reduceGraphAll(lb, hversion);
    stats.setReducedVerts(graph.nVerts);
    stats.setReducedEdges(graph.countEdges());
    if (!raret.elimVerts.empty()) {
      elimprefix.insert(elimprefix.end(), raret.elimVerts.begin(),
			raret.elimVerts.end());
      elimprefix_width = raret.maxdeg;
      // if reduction removed all vertices, then we're done
      if (graph.nVerts==0) {
	if (elimprefix_width < ub_order.width) {
	  ub_order.order_prefix=elimprefix;
	  ub_order.indiff_suffix.clear();
	  ub_order.width=elimprefix_width;
	}
	vertexmap.clear();
	stats.setMinFillUB(ub_order.width);
	return TW_SUCCESS;
      }
      // if width or lb of elimprefix exceeds ub, then we're done
      if (elimprefix_width >= ub_order.width || raret.hval >= ub_order.width)
	return TW_SUCCESS;
      postprefix_lb = raret.hval;
      if (postprefix_lb > (int)lb)
	lb = postprefix_lb;
      else if (elimprefix_width < (int)lb && postprefix_lb < (int)lb)
	postprefix_lb = lb;
    }

    // normalize graph
    graph.normalize_SLOW(elimprefix, vertexmap);

    // initilize state structures
    TWState::nVerts(graph.nVerts);
    BFHT_HDDD_State::nVerts(graph.nVerts);

    // compute upperbound
    if (ub >= (uint)INT_MAX)
      {
	ElimOrder ub_order_tmp = graph.ubRandomizedMinFill(6);
	if (ub_order_tmp.width < ub_order.width) {
	  ub_order = ub_order_tmp;
	  ub_mapped = true;
	}
      }

#ifdef DEBUG_TW
    assert(lb<=ub_order.width);
#endif
    if (elimprefix_width >= ub_order.width || (int)lb == ub_order.width) {
      if (!ub_order.order_prefix.empty())
	{
	  if (elimprefix_width > ub_order.width)
	    ub_order.width=elimprefix_width;
	  if (!vertexmap.empty())
	    ub_order.remap(vertexmap);
	  ub_order.appendPrefix(elimprefix);
	}
      stats.setMinFillUB(ub_order.width);
      return TW_SUCCESS;
    }
    stats.setMinFillUB(ub_order.width);

    return TW_NONE;
  }

}
