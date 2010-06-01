// GraphUtilities.h - header file for graph utilities, including reading and
//   writing graph files and generating random graphs
//
// author: Alex Dow
// created on: 10/4/6

#ifndef GRAPHUTILITIES_H_
#define GRAPHUTILITIES_H_

#include "preproc_flags.h"
#include <vector>
#include <iostream>

namespace GraphUtilities {
using namespace std;

enum GraphFileType {GRAPH_EDGE_LIST, GRAPH_DIMACS};

typedef vector<bool> boolVector;
typedef vector<boolVector> boolMatrix;

class FileOperations {
public:
	static boolMatrix readGraph(const char *filename);
	static boolMatrix readGraphDIMACS(const char *filename);
	static boolMatrix readGraph(const char *filename, GraphFileType type);
	static void writeGraph(const char *filename, const boolMatrix &graph);
	static void writeGraphDIMACS(const char *filename, const boolMatrix &graph);
	static void writeGraph(const char *filename, GraphFileType type,
			const boolMatrix &graph);
	static void printGraphMatrix(ostream &ost, boolMatrix graph);
	static void printGraphEdges(ostream &ost, boolMatrix graph);
};

inline boolMatrix FileOperations::readGraph(const char *filename,
		GraphFileType type) {
	switch (type) {
	case GRAPH_EDGE_LIST:
		return readGraph(filename);
	case GRAPH_DIMACS:
		return readGraphDIMACS(filename);
	default:
		return boolMatrix();
	}
}

inline void FileOperations::writeGraph(const char *filename,
		GraphFileType type, const boolMatrix &graph) {
	switch (type) {
	case GRAPH_EDGE_LIST:
		writeGraph(filename, graph);
		break;
	case GRAPH_DIMACS:
		writeGraphDIMACS(filename, graph);
		break;
	}
}

class RandomGraph {
public:
	static boolMatrix
			generateGraph(int nVerts, int nEdges, unsigned int seed=0);
	static boolMatrix generateConnectedGraph(int nVerts, int nEdges,
			unsigned int seed=0);
};

class MiscUtilities {
public:
	static void normalizeGraph(boolMatrix &graph, vector<int> &vertex_mapping);
	static void normalizeGraph(boolMatrix &graph);
};

}
;

#endif /*GRAPHUTILITIES_H_*/
