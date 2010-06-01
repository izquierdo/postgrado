// GraphUtilities.cpp - definition file for graph utilities, including reading
//   and writing graph files and generating random graphs
//
// author: Alex Dow
// created on: 10/4/6

#include "preproc_flags.h"
#include <iostream>
#include <fstream>
#include <iterator>
#include <cstdlib>
#include "GraphUtilities.h"

namespace GraphUtilities {

///////////////////////////////////////////////////////////////////////////////
// readGraph - static function for creating a boolMatrix that stores the
//   adjacency matrix of a graph specified in a file in
//   the following format:
//     - first line - #vertices #edges
//     - following lines - v1 v2 - which specifies an edge exists between
//       vertices v1 and v2
///////////////////////////////////////////////////////////////////////////////
	boolMatrix FileOperations::readGraph (const char *filename)
	{
		// check input
		ifstream fin(filename);
		if (!fin.is_open()) {
			cerr << "Unable to open file input file "<<filename<<" for reading"<< endl;
	        exit(1);
		}
		// get graph parameters
		int nVerts;
		int nEdges;
		fin >> nVerts >> nEdges;
		// get graph
		boolMatrix graph(nVerts,boolVector(nVerts,false));
		int v1, v2, cnt=0;
		while (fin >> v1 >> v2) {
			graph[v1-1][v2-1] = true;
			graph[v2-1][v1-1] = true;
			cnt++;
		}
		// check parameters
		if (cnt!=nEdges) {
			cerr << "File " << filename << " does not contain " << nEdges << " edges as it states.\n";
			exit(1);
		}
		return graph;
	}



///////////////////////////////////////////////////////////////////////////////
// readGraphDIMACS - static function for creating a boolMatrix that stores the
//   adjacency matrix of a graph specified in a file in the DIMACS graph
//   format.
///////////////////////////////////////////////////////////////////////////////
	boolMatrix FileOperations::readGraphDIMACS(const char *filename)
	{
		// check input
		ifstream fin(filename);
		if (!fin.is_open()) {
			cerr << "Unable to open file input file "<<filename<<" for reading"<< endl;
	        exit(1);
		}
		// read past comments
		string s;
		while (s!="p" && !fin.eof())
			fin >> s;
		fin >> s; // format
		// read graph parameters
		int nverts, nedges;
		fin >> nverts;
		fin >> nedges;
		// read graph
		boolMatrix graph(nverts,boolVector(nverts,false));
		int v1,v2,cnt=0;
		while (!fin.eof()) {
			while (s!="e" && !fin.eof())
				fin >> s;
			if (fin.eof())
				break;
			s.clear();
			fin >> v1;
			fin >> v2;
			graph[v1-1][v2-1] = true;
			graph[v2-1][v1-1] = true;
			cnt++;
		}
		// check parameters
		if (cnt!=nedges) {
			cerr << "File " << filename << " does not contain " << nedges << " edges as it states.\n";
			exit(1);
		}
		return graph;
	}


///////////////////////////////////////////////////////////////////////////////
// FileOperations writeGraph - writes an edge-list file representing graph
///////////////////////////////////////////////////////////////////////////////
	void FileOperations::writeGraph(const char *filename, const boolMatrix &graph)
	{
		ofstream fout(filename);
		if (!fout.is_open()) {
			cerr << "Error: could not open file " << filename << " for output.\n";
			exit(1);
		}

		int edges=0;
		for (uint i=0; i<graph.size(); i++)
			for (uint j=i+1; j<graph[i].size(); j++)
				if (graph[i][j])
					edges++;

		fout << graph.size() << "\t" << edges << endl;

		for (uint i=0; i<graph.size(); i++)
			for (uint j=i+1; j<graph[i].size(); j++)
				if (graph[i][j])
					fout << i+1 << "\t" << j+1 << endl;
	}

///////////////////////////////////////////////////////////////////////////////
// FileOperations writeGraphDIMACS - writes a dimacs file representing graph
///////////////////////////////////////////////////////////////////////////////
	void FileOperations::writeGraphDIMACS(const char *filename, const boolMatrix &graph)
	{
		ofstream fout(filename);
		if (!fout.is_open()) {
			cerr << "Error: could not open file " << filename << " for output.\n";
			exit(1);
		}

		int edges=0;
		for (uint i=0; i<graph.size(); i++)
			for (uint j=i+1; j<graph[i].size(); j++)
				if (graph[i][j])
					edges++;

		fout << "p edge " << graph.size() << " " << edges << endl;

		for (uint i=0; i<graph.size(); i++)
			for (uint j=i+1; j<graph[i].size(); j++)
				if (graph[i][j])
					fout << "e " << i+1 << " " << j+1 << endl;
	}



///////////////////////////////////////////////////////////////////////////////
// printGraphMatrix - outputs the adjacency matrix of a graph
///////////////////////////////////////////////////////////////////////////////
	void FileOperations::printGraphMatrix (ostream &ost, boolMatrix graph)
	{
		boolMatrix::iterator iter=graph.begin();
		while (iter!=graph.end()) {
			copy(iter->begin(),iter->end(),ostream_iterator<bool>(ost," "));
			ost << endl;
			iter++;
		}
	}



///////////////////////////////////////////////////////////////////////////////
// printGraphEdges - outputs the graph specified as an adjacency matrix in the
//   following format:
//     - first line - #vertices #edges
//     - following lines - v1 v2 - which specifies an edge exists between
//       vertices v1 and v2
///////////////////////////////////////////////////////////////////////////////
	void FileOperations::printGraphEdges(ostream &ost, boolMatrix graph)
	{
		int verts=graph.size();
		int edges=0;
		for (uint i=0; i<graph.size(); i++)
			for (uint j=i; j<graph.size(); j++)
				if (graph[i][j])
					edges++;
		ost << verts << '\t' << edges << endl;
		for (uint i=0; i<graph.size(); i++)
			for (uint j=i; j<graph.size(); j++)
				if (graph[i][j])
					ost << i+1 << '\t' << j+1 << endl;
	}



///////////////////////////////////////////////////////////////////////////////
// generateGraph - generates a random graph by adding edges at random
//   NOTE: resulting graph may not be connected
// Inputs:
//   - nVerts
//   - nEdges
//   - seed - [optional] random seed (seed=0 will not be used)
///////////////////////////////////////////////////////////////////////////////
	boolMatrix RandomGraph::generateGraph (int nVerts, int nEdges, unsigned int seed)
	{
		// check inputs
		if (nEdges>nVerts*(nVerts-1)/2) {
			cerr << "[generateGraph] nEdges can be no more than nVerts*(nVerts-1)/2, input: nVerts("
				 << nVerts << ") nEdges(" << nEdges << ")\n";
			return boolMatrix();
		}

		if (seed!=0)
		    srand(seed);

	    boolMatrix graph(nVerts,boolVector(nVerts,false));

	    //  Generate edges uniformly at random
	    for(int i=0;i<nEdges;i++) {
	    		bool duplicate = true;
	    		int edgei = 0;
	    		while (duplicate) {
	        		duplicate = false;
	        		// Get edge index
	            edgei = rand()% (nVerts*nVerts);
	            // Discard if it results in an edge between the same vertices
	            if(edgei/nVerts == edgei%nVerts)
	                duplicate = true;
	            // Discard if the edge is already present
	            if(graph[edgei/nVerts][edgei%nVerts])
	              duplicate=true;
	    		}
	    		graph[edgei/nVerts][edgei%nVerts]= graph[edgei%nVerts][edgei/nVerts]= true;
	    }
	    return graph;
	}


///////////////////////////////////////////////////////////////////////////////
// generateConnectedGraph - generates a random connected graph by iteratively
//   adding edges that connect previously unconnected components. When only one
//   component remains, edges are added at random.
//   NOTE: there may be biases in this random model
// Inputs:
//   - nVerts
//   - nEdges
//   - seed - [optional] random seed (seed=0 will not be used)
///////////////////////////////////////////////////////////////////////////////
	boolMatrix RandomGraph::generateConnectedGraph (int nVerts, int nEdges, unsigned int seed)
	{
		// check inputs
		if (nEdges<nVerts-1) {
			cerr << "[generateConnectedGraph] nEdges must be >= nVerts-1\n, input: nVerts("
				 << nVerts << ") nEdges(" << nEdges << ")\n";
			return boolMatrix();
		}
		if (nEdges>nVerts*(nVerts-1)/2) {
			cerr << "[generateConnectedGraph] nEdges can be no more than nVerts*(nVerts-1)/2, input: nVerts("
				 << nVerts << ") nEdges(" << nEdges << ")\n";
			return boolMatrix();
		}
		// seed random number generator
		if (seed!=0)
			srand(seed);

	    boolMatrix graph(nVerts,boolVector(nVerts,false));
		int edgesDone=0;
		// first create a spanning tree
		vector<vector<int> > components(nVerts);
		for (int i=0; i<nVerts; i++)
			components[i].push_back(i);
		while (components.size()>1) {
			// randomly choose one component
			int c1=components.size();
			while (c1>=(int)components.size())
				c1 = (double(rand())/RAND_MAX)*components.size();
			// randomly choose another component
			int c2=components.size();
			while (c2>=(int)components.size() || c2==c1)
				c2 = (double(rand())/RAND_MAX)*components.size();
			// randomly choose one vertex in c1
			int v1=components[c1].size();
			while (v1>=(int)components[c1].size())
				v1=(double(rand())/RAND_MAX)*components[c1].size();
			// randomly choose one vertex in c2
			int v2=components[c2].size();
			while (v2>=(int)components[c2].size())
				v2=(double(rand())/RAND_MAX)*components[c2].size();
			// add edge (v1,v2) to graph
			graph[components[c1][v1]][components[c2][v2]]=true;
			graph[components[c2][v2]][components[c1][v1]]=true;
			edgesDone++;
			// combine components c1 and c2 (in c1)
			components[c1].insert(components[c1].end(),components[c2].begin(),components[c2].end());
			// remove c2
			components[c2]=components.back();
			components.pop_back();
		}
		components.clear();
		// now add the rest of the edges randomly
		while (edgesDone<nEdges) {
			int v1=nVerts, v2=nVerts;
			// randomly choose 2 valid vertices
			while (v1==nVerts || v2==nVerts || v1==v2 || graph[v1][v2]==true) {
				v1=(double(rand())/RAND_MAX)*nVerts;
				v2=(double(rand())/RAND_MAX)*nVerts;
			}
			graph[v1][v2]=true;
			graph[v2][v1]=true;
			edgesDone++;
		}

		return graph;
	}


///////////////////////////////////////////////////////////////////////////////
// normalizeGraph - removes disconnected vertices (those with no incident
//   edges), and resets vertex indices so that they ranged from 1 to nVerts
//   without gaps.
// Inputs:
//   - boolMatrix &graph - argument graph will be reallocated if necessary to
//       store normalized graph
//   - vector<int> vertex_mapping - stores a mapping of vertex indices in new
//       graph to vertex indices in old graph, any old vertices not present in
//       map were disconnected. vertex_mapping[i]=j means that vertex i in new
//		 graph is equal to vertex j in old graph.
///////////////////////////////////////////////////////////////////////////////
	void MiscUtilities::normalizeGraph ( boolMatrix &graph, vector<int> &vertex_mapping )
	{
		// find which vertices have edges, construct mapping
		for (uint i=0; i<graph.size(); i++) {
			for (uint j=0; j<graph.size(); j++) {
				if (graph[i][j]) {
					vertex_mapping.push_back(i);
					break;
				}
			}
		}
		// check whether any vertices are disconnected, if not, exit
		int nVerts = vertex_mapping.size();
		if (nVerts==(int)graph.size())
			return;
		// allocate new graph
		boolMatrix normGraph(nVerts,boolVector(nVerts,false));
		// fill new graph
		for (int i=0; i<nVerts; i++) {
			for (int j=i+1; j<nVerts; j++) {
				if (graph[vertex_mapping[i]][vertex_mapping[j]])
					normGraph[i][j] = normGraph[j][i] = true;
			}
		}
		graph = normGraph;
	}

	void MiscUtilities::normalizeGraph ( boolMatrix &graph )
	{
		vector<int> tmp;
		normalizeGraph(graph,tmp);
	}

};
