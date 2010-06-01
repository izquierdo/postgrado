CXX = g++
CXXFLAGS = -Wall $(OPT) $(PROFILE) $(DEBUG)
OPT = -O3 -I../Boost/boost_1_43_0 -Wno-deprecated

OBJS = ALMGraph.o BestTW.o BestTWNode.o BestTWOpenList.o BestTWStats.o BFHT_DDD_OpenList.o BFHT_HDDD.o BFHT_HDDD_Node.o BreadthFirstTW.o BreadthFirstTW_DDD.o BreadthFirstTWNode.o BreadthFirstTWStats.o DFS.o driver.o GraphUtilities.o TWDriver.o TWSearch.o TWState.o lookup3.o

all: TWSearch

TWSearch: $(OBJS)
	$(CXX) -o "TWSearch" $(OBJS) $(PROFILE)

ALMGRAPH_H_INCLUDE = preproc_flags.h GraphUtilities.h TWState.h BFHT_HDDD_Node.h

ALMGraph.o: ALMGraph.cpp preproc_flags.h ALMGraph.h rdtsc.h $(ALMGRAPH_H_INCLUDE)
	$(CXX) $(CXXFLAGS) -c ALMGraph.cpp

TWSEARCH_H_INCLUDE = preproc_flags.h TWStats.h GraphUtilities.h ALMGraph.h $(ALMGRAPH_H_INCLUDE)
TWNODE_H_INCLUDE = preproc_flags.h TWState.h
BESTTWNODE_H_INCLUDE = preproc_flags.h TWNode.h $(TWNODE_H_INCLUDE) TWState.h ALMGraph.h $(ALMGRAPH_H_INCLUDE)
BESTTWSTATS_H_INCLUDE = preproc_flags.h TWStats.h
BESTTWOPENLIST_H_INCLUDE = preproc_flags.h BestTWNode.h $(BESTTWNODE_H_INCLUDE) BestTWStats.h $(BESTTWSTATS_H_INCLUDE)
BESTTW_H_INCLUDE = preproc_flags.h TWSearch.h $(TWSEARCH_H_INCLUDE) TWState.h BestTWNode.h $(BESTTWNODE_H_INCLUDE) BestTWOpenList.h $(BESTTWOPENLIST_H_INCLUDE) BestTWStats.h $(BESTTWSTATS_H_INCLUDE) ALMGraph.h $(ALMGRAPH_H_INCLUDE)

BestTW.o: BestTW.cpp preproc_flags.h BestTW.h $(BESTTW_H_INCLUDE) utils.h
	$(CXX) $(CXXFLAGS) -c BestTW.cpp

BestTWNode.o: BestTWNode.cpp preproc_flags.h BestTWNode.h $(BESTTW_H_INCLUDE)
	$(CXX) $(CXXFLAGS) -c BestTWNode.cpp

BestTWOpenList.o: BestTWOpenList.cpp preproc_flags.h BestTWOpenList.h $(BESTTWOPENLIST_H_INCLUDE)
	$(CXX) $(CXXFLAGS) -c BestTWOpenList.cpp

BestTWStats.o: BestTWStats.cpp preproc_flags.h BestTWStats.h $(BESTTWSTATS_H_INCLUDE)
	$(CXX) $(CXXFLAGS) -c BestTWStats.cpp

BREADTHFIRSTTWNODE_H_INCLUDE = preproc_flags.h ALMGraph.h $(ALMGRAPH_H_INCLUDE) TWState.h TWNode.h $(TWNODE_H_INCLUDE)
BFHT_DDD_OPENLIST_H_INCLUDE = preproc_flags.h BreadthFirstTWNode.h $(BREADTHFIRSTTWNODE_H_INCLUDE)

BFHT_DDD_OpenList.o: BFHT_DDD_OpenList.cpp preproc_flags.h BFHT_DDD_OpenList.h $(BFHT_DDD_OPENLIST_H_INCLUDE)
	$(CXX) $(CXXFLAGS) -c BFHT_DDD_OpenList.cpp

BREADTHFIRSTTWSTATS_H_INCLUDE = preproc_flags.h TWStats.h
BFHT_HDDD_H_INCLUDE = preproc_flags.h BFHT_HDDD_Node.h ALMGraph.h $(ALMGRAPH_H_INCLUDE) TWSearch.h $(TWSEARCH_H_INCLUDE) BreadthFirstTWStats.h $(BREADTHFIRSTTWSTATS_H_INCLUDE)

BFHT_HDDD.o: BFHT_HDDD.cpp preproc_flags.h BFHT_HDDD.h $(BFHT_HDDD_H_INCLUDE)
	$(CXX) $(CXXFLAGS) -c BFHT_HDDD.cpp

BFHT_HDDD_Node.o: BFHT_HDDD_Node.cpp preproc_flags.h BFHT_HDDD_Node.h
	$(CXX) $(CXXFLAGS) -c BFHT_HDDD_Node.cpp

BREADTHFIRSTTWOPENLIST_H_INCLUDE = preproc_flags.h BreadthFirstTWNode.h $(BREADTHFIRSTTWNODE_H_INCLUDE)
BREADTHFIRSTTW_H_INCLUDE = preproc_flags.h ALMGraph.h $(ALMGRAPH_H_INCLUDE) TWState.h TWSearch.h $(TWSEARCH_H_INCLUDE) BreadthFirstTWNode.h $(BREADTHFIRSTTWNODE_H_INCLUDE) BreadthFirstTWOpenList.h $(BREADTHFIRSTTWOPENLIST_H_INCLUDE) BreadthFirstTWStats.h $(BREADTHFIRSTTWSTATS_H_INCLUDE) BFHT_DDD_OpenList.h $(BFHT_DDD_OPENLIST_H_INCLUDE) BFHT_HDDD_Node.h BFHT_HDDD.h $(BFHT_HDDD_H_INCLUDE)

BreadthFirstTW.o: BreadthFirstTW.cpp preproc_flags.h BreadthFirstTW.h $(BREADTHFIRSTTW_H_INCLUDE) utils.h
	$(CXX) $(CXXFLAGS) -c BreadthFirstTW.cpp

BreadthFirstTW_DDD.o: BreadthFirstTW_DDD.cpp preproc_flags.h BreadthFirstTW.h $(BREADTHFIRSTTW_H_INCLUDE) utils.h
	$(CXX) $(CXXFLAGS) -c BreadthFirstTW_DDD.cpp

BreadthFirstTWNode.o: BreadthFirstTWNode.cpp preproc_flags.h BreadthFirstTWNode.h $(BREADTHFIRSTTWNODE_H_INCLUDE)
	$(CXX) $(CXXFLAGS) -c BreadthFirstTWNode.cpp

BreadthFirstTWStats.o: BreadthFirstTWStats.cpp preproc_flags.h BreadthFirstTWStats.h $(BREADTHFIRSTTWSTATS_H_INCLUDE)
	$(CXX) $(CXXFLAGS) -c BreadthFirstTWStats.cpp

DFSSTATS_H_INCLUDE = TWStats.h
DFS_H_INCLUDE = preproc_flags.h TWSearch.h $(TWSEARCH_H_INCLUDE) DFSStats.h $(DFSSTATS_H_INCLUDE) anyspace_hash_table.h

DFS.o: DFS.cpp DFS.h $(DFS_H_INCLUDE) utils.h
	$(CXX) $(CXXFLAGS) -c DFS.cpp

TWDRIVER_H_INCLUDE = preproc_flags.h ALMGraph.h $(ALMGRAPH_H_INCLUDE) GraphUtilities.h

driver.o: driver.cpp preproc_flags.h TWDriver.h $(TWDRIVER_H_INCLUDE)
	$(CXX) $(CXXFLAGS) -c driver.cpp

GraphUtilities.o: GraphUtilities.cpp preproc_flags.h GraphUtilities.h
	$(CXX) $(CXXFLAGS) -c GraphUtilities.cpp

TWDriver.o: TWDriver.cpp preproc_flags.h TWDriver.h $(TWDRIVER_H_INCLUDE) BestTW.h $(BESTTW_H_INCLUDE) BreadthFirstTW.h $(BREADTHFIRSTTW_H_INCLUDE) DFS.h $(DFS_H_INCLUDE)
	$(CXX) $(CXXFLAGS) -c TWDriver.cpp

TWSearch.o: TWSearch.cpp preproc_flags.h TWSearch.h $(TWSEARCH_H_INCLUDE)
	$(CXX) $(CXXFLAGS) -c TWSearch.cpp

TWState.o: TWState.cpp preproc_flags.h TWState.h
	$(CXX) $(CXXFLAGS) -c TWState.cpp

lookup3.o: lookup3.c lookup3.h
	$(CXX) $(CXXFLAGS) -c lookup3.c

clean:
	-rm $(OBJS) TWSearch

profile:
	$(MAKE) $(MAKEFILE) OPT="-O3" PROFILE="-pg"

debug:
	$(MAKE) $(MAKEFILE) OPT="-O0" DEBUG="-g"
