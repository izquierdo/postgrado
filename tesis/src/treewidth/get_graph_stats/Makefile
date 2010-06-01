CXX = g++
CXXFLAGS = -Wall $(OPT) $(DEBUG)
OPT = -O3
OBJS = driver.o

all: get_graph_stats

get_graph_stats: $(OBJS)
	$(CXX) -o "get_graph_stats" $(OBJS)

driver.o: driver.cpp Graph.h
	$(CXX) $(CXXFLAGS) -c driver.cpp

clean:
	-rm $(OBJS) get_graph_stats

debug:
	$(MAKE) $(MAKEFILE) OPT="-O0" DEBUG="-g"