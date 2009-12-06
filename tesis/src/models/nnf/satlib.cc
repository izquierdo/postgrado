/*
 *  Copyright (C) 2006 Universidad Simon Bolivar
 * 
 *  Permission is hereby granted to distribute this software for
 *  non-commercial research purposes, provided that this copyright
 *  notice is included with any such distribution.
 *  
 *  THIS SOFTWARE IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
 *  EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 *  PURPOSE.  THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE OF THE
 *  SOFTWARE IS WITH YOU.  SHOULD THE PROGRAM PROVE DEFECTIVE, YOU
 *  ASSUME THE COST OF ALL NECESSARY SERVICING, REPAIR OR CORRECTION.
 *  
 *  Blai Bonet, bonet@ldc.usb.ve
 *
 */

#include "nnf.h"
#include "satlib.h"

#include <iostream>
#include <ios>
#include <set>
#include <vector>
#include <stdio.h>
#include <ctype.h>
#include <fstream>
#include <string>
#include <string.h>

namespace satlib {

bool
read_line( std::istream &is, std::string &line )
{
  char c;
  line.clear();
  while( is.get(c) ) {
    if( c == '\n' ) return(true);
    line += c;
  }
  return(false);
}

void
read_nnf_file( std::istream &is, nnf::Manager &manager )
{
  size_t n = 0;
  int num_vars, num_nodes, num_edges;
  std::string line;
  const nnf::Node **nodes = 0;

  // first pass: determine parents' sizes
  size_t *psize = 0;
  while( read_line(is,line) ) {
    if( line[0] == 'c' )
      continue;
    else if( line[0] == 'n' ) {
      sscanf( line.c_str(), "nnf %d %d %d", &num_nodes, &num_edges, &num_vars );
      psize = new size_t[num_nodes];
      for( int i = 0; i < num_nodes; ++i ) psize[i] = 0;
    }
    else if( line[0] == '%' )
      break;
    else {
      char *str = strdup(line.c_str());
      char *ptr = strtok(str," ");
      char type = toupper(*ptr);
      ptr = strtok(0, " ");

      if( (type == 'A') || (type == 'O') ) {
        if( type == 'O' ) ptr = strtok(0," "); 
        size_t sz = atoi(ptr);
        if( sz != 0 ) {
          ptr = strtok(0," ");
          for( size_t i = 0; i < sz; ++i ) {
            ++psize[atoi(ptr)];
            ptr = strtok(0," ");
          }
          assert( ptr == 0 );
        }
      }
      free(str);
    }
  }

  // second pass: create nodes
  is.clear();
  is.seekg(0,std::ios_base::beg); 
  while( read_line(is,line) ) {
    if( line[0] == 'c' )
      continue;
    else if( line[0] == 'n' ) {
      sscanf( line.c_str(), "nnf %d %d %d", &num_nodes, &num_edges, &num_vars );
      manager.reserve(num_nodes);
      manager.set_num_vars(num_vars);
      nodes = new const nnf::Node*[num_nodes];
      for( int i = 0; i < num_nodes; ++i ) nodes[i] = 0;
    }
    else if( line[0] == '%' )
      break;
    else {
      size_t id = n++;
      char *str = strdup(line.c_str());
      char *ptr = strtok(str," ");
      char type = toupper(*ptr);
      ptr = strtok(0, " ");

      if( type == 'L' ) {
        int lit = atoi(ptr);
        assert( lit != 0 );
        unsigned var = (lit > 0 ? (lit<<1) : ((-lit)<<1)+1);
        nodes[id] = nnf::make_variable(&manager,var,psize[id]);
      }
      else if( (type == 'A') || (type == 'O') ) {
        if( type == 'O' ) ptr = strtok(0," "); 
        size_t sz = atoi(ptr);
        if( sz == 0 ) {
          nodes[id] = nnf::make_value(&manager,(type=='A'?true:false));
        }
        else {
          ptr = strtok(0," ");
          std::vector<const nnf::Node*> children;
          for( size_t i = 0; i < sz; ++i ) {
            children.push_back( nodes[atoi(ptr)] );
            ptr = strtok(0," ");
          }
          assert( ptr == 0 );
          if( type == 'A' )
            nodes[id] = nnf::make_and(&manager,children.size(),(const nnf::Node**)&children[0],psize[id]);
          else
            nodes[id] = nnf::make_or(&manager,children.size(),(const nnf::Node**)&children[0],psize[id]);
        }
      }
      free(str);
    }
  }

  manager.set_root(nodes[n-1]);
  manager.register_use(manager.root());
  manager.set_sorted();
  for( size_t i = 0; i < n; ++i ) manager.unregister_use(nodes[i]);
  delete[] nodes;
  delete[] psize;
}

}; // satlib namespace

