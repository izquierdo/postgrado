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
#include <fstream>
#include <stdio.h>
#include <unistd.h>
#include <math.h>

namespace nnf {

size_t
Manager::count_edges() const
{
  for( size_t i = 0, isz = node_pools_.size(); i < isz; ++i ) {
    NodePool *pool = node_pools_[i];
    for( size_t j = 0, jsz = pool->boundary_; j < jsz; ++j ) {
      const Node *n = &pool->table_[j];
      assert( n->cache_.first_ == 0 );
      if( (n->type_ == And) || (n->type_ == Or) ) {
        size_t count = 0;
        for( const NodePtr *p = n->children_; *p != 0; ++p ) {
          count += 1 + (size_t)(*p)->cache_.first_;
          (*p)->cache_.set();
        }
        n->cache_.set( (const void*)count );
      }
    }
  }
  size_t count = (size_t)root_->cache_.first_;
  clean_node_cache();
  return(count);
}

void
Manager::sort()
{
  if( sorted_ ) return;
  char tname[20] = "/tmp/nnf.XXXXXX";
  char *fname = mktemp(tname);

  if( verbose_ ) std::cout << "nnf: sorting: dumping" << std::flush;
  std::ofstream os(fname); 
  dump(os);
  os.close();

  clear();

  if( verbose_ ) std::cout << ", reading" << std::flush;
  std::ifstream is(fname);
  satlib::read_nnf_file(is,*this);
  is.close();

  if( verbose_ ) std::cout << std::endl;
  unlink(fname);
}

/* count nnf models:
 *   if output = 0, just count all models, else count models for each literal and store in output
 *   if lit_map != 0, count models compatible with literals in lit_map
 */
float
Manager::count_models( float *output, const int *lit_map ) const
{
  // first pass (bottom-up): compute model count in each node
  for( size_t i = 0, isz = node_pools_.size(); i < isz; ++i ) {
    NodePool *pool = node_pools_[i];
    for( size_t j = 0, jsz = pool->boundary_; j < jsz; ++j ) {
      const Node *n = &pool->table_[j];
      if( n->ref_ > 0 ) {
        float count = -1;
        if( (n->type_ == And) || (n->type_ == Or) ) {
          for( const NodePtr *p = n->children_; *p != 0; ++p ) {
            float c = *(float*)&(*p)->cache_.first_;
            count = (count==-1?c:(n->type_==And?count*c:count+c));
          }
        }
        else if( n->type_ == Value )
          count = ((int)n->children_?1:0);
        else if( n->type_ == Variable ) {
          if( lit_map && lit_map[(int)n->children_^1] )
            count = 0;
          else
            count = 1;
        }
        n->cache_.set( *(const void**)&count );
      }
    }
  }
  float result = *(float*)&root_->cache_.first_;
  if( !output ) goto the_end;

  // second pass (top-down): differentiate with respect to each node (only for complete count)
  for( size_t i = 0; i <= num_vars_; ++ i ) output[i] = -1;
  for( size_t i = node_pools_.size(); i > 0; --i ) {
    NodePool *pool = node_pools_[i-1];
    for( size_t j = pool->boundary_; j > 0; --j ) {
      const Node *n = &pool->table_[j-1];
      if( n->ref_ > 0 ) {
        float pd = 0;
        float count = *(float*)&n->cache_.first_;
        if( (n->szparents_ == 0) || (count < 0) )
          pd = 1;
        else {
          for( NodePtr *p = n->parents_; *p != 0; ++p ) {
            assert( ((*p)->type_ == And) || ((*p)->type_ == Or) );
            float cpd = *(float*)&(*p)->cache_.second_;
            if( (*p)->type_ == And ) {
              for( NodePtr *q = (*p)->children_; *q != 0; ++q ) {
                if( *q != n ) cpd *= *(float*)&(*q)->cache_.first_;
              }
            }
            pd += cpd;
          }
        }
        n->cache_.second_ = *(const float**)&pd;
        if( n->type_ == Variable ) {
          if( ((int)n->children_%2 == 0) && (!lit_map || !lit_map[(int)n->children_^1]) )
            output[(int)n->children_>>1] = pd;
          else if( output[(int)n->children_>>1] == -1 )
            output[(int)n->children_>>1] = 0;
        }
      }
    }
  }
  output[0] = result;
the_end:
  clean_node_cache();
  return( result );
}

mp::Int*
Manager::mp_count_models( mp::Int **output, const int *lit_map ) const
{
  // first pass (bottom-up): compute model count in each node
  for( size_t i = 0, isz = node_pools_.size(); i < isz; ++i ) {
    NodePool *pool = node_pools_[i];
    for( size_t j = 0, jsz = pool->boundary_; j < jsz; ++j ) {
      const Node *n = &pool->table_[j];
      if( n->ref_ > 0 ) {
        mp::Int *count = new mp::Int(-1);
        if( (n->type_ == And) || (n->type_ == Or) ) {
          for( const NodePtr *p = n->children_; *p != 0; ++p ) {
            mp::Int *c = (mp::Int*)(*p)->cache_.first_;
            if( *count == -1 )
              *count = *c;
            else if( n->type_ == And )
              *count *= (*c);
            else
              *count += (*c);
          }
        }
        else if( n->type_ == Value )
          *count = ((int)n->children_?1:0);
        else if( n->type_ == Variable ) {
          if( lit_map && lit_map[(int)n->children_^1] )
            *count = 0;
          else
            *count = 1;
        }
        n->cache_.set( (const void*)count );
      }
    }
  }
  mp::Int *result = new mp::Int(*(mp::Int*)root_->cache_.first_);
  if( !output ) goto the_end;

  // second pass (top-down): differentiate with respect to each node (only for complete count)
  for( size_t i = 0; i <= num_vars_; ++ i ) output[i] = 0;
  for( size_t i = node_pools_.size(); i > 0; --i ) {
    NodePool *pool = node_pools_[i-1];
    for( size_t j = pool->boundary_; j > 0; --j ) {
      const Node *n = &pool->table_[j-1];
      if( n->ref_ > 0 ) {
        mp::Int *pd = new mp::Int;
        mp::Int *count = (mp::Int*)n->cache_.first_;
        if( (n->szparents_ == 0) || (*count < 0) )
          *pd = 1;
        else {
          for( NodePtr *p = n->parents_; *p != 0; ++p ) {
            assert( ((*p)->type_ == And) || ((*p)->type_ == Or) );
            mp::Int cpd = *(mp::Int*)(*p)->cache_.second_;
            if( (*p)->type_ == And ) {
              for( NodePtr *q = (*p)->children_; *q != 0; ++q ) {
                if( *q != n ) cpd *= *(mp::Int*)(*q)->cache_.first_;
              }
            }
            *pd += cpd;
          }
        }
        n->cache_.second_ = (const void*)pd;
        if( n->type_ == Variable ) {
          if( ((int)n->children_%2 == 0) && (!lit_map || !lit_map[(int)n->children_^1]) )
            output[(int)n->children_>>1] = new mp::Int(*pd);
          else if( output[(int)n->children_>>1] == 0 )
            output[(int)n->children_>>1] = new mp::Int;
        }
      }
    }
  }
  output[0] = new mp::Int(*result);
the_end:
  clean_typed_node_cache<mp::Int,mp::Int>();
  return( result );
}

std::pair<bool,const Node*>
Manager::enumerate_models_recursively( const Node *n, Model &m, const Node *last_or ) const
{
  bool stat = true;
  const Node *lor = last_or;
  if( n->type_ == And ) {
    for( NodePtr *p = n->children_; stat && (*p != 0); ++p ) {
      std::pair<bool,const Node*> rc = enumerate_models_recursively(*p,m,lor);
      stat = rc.first;
      lor = rc.second;
    }
  }
  else if( n->type_ == Or ) {
    int next = (int)n->cache_.first_;
    n->cache_.second_ = lor;
    std::pair<bool,const Node*> rc = enumerate_models_recursively(n->children_[next],m,n);
    stat = rc.first;
    lor = rc.second;
  }
  else if( n->type_ == Variable ) {
    m.insert((int)n->children_);
  }
  else if( n->type_ == Value ) {
    stat = ((int)n->children_ != 0);
  }
  return(std::make_pair(stat,lor));
}

void
Manager::enumerate_models( std::ostream &os, size_t count, bool all ) const
{
  Model m;
  bool next = true;
  size_t i = 0;
  while( next && ((count == 0) || (i < count)) ) {
    // generate model
    std::pair<bool,const Node*> rc = enumerate_models_recursively(root_,m,0);
    if( rc.first ) {
      m.print(os,all);
      os << std::endl;
      ++i;
    }
    m.clear();

    // advance state
    next = false;
    for( const Node *n = rc.second; !next && (n != 0); n = (const Node*)n->cache_.second_ ) {
      int i = 1+(int)n->cache_.first_;
      if( n->children_[i] == 0 )
        n->cache_.first_ = 0;
      else {
        n->cache_.first_ = (const void*)i;
        next = true;
      }
    }
  }
}

void
Manager::enumerate_models( ModelList &models, size_t count ) const
{
  Model m;
  bool next = true;
  size_t i = 0;
  while( next && ((count == 0) || (i < count)) ) {
    // generate model
    std::pair<bool,const Node*> rc = enumerate_models_recursively(root_,m,0);
    if( rc.first ) {
      models.push_back( new Model(m) );
      ++i;
    }
    m.clear();

    // advance state
    next = false;
    for( const Node *n = rc.second; !next && (n != 0); n = (const Node*)n->cache_.second_ ) {
      int i = 1+(int)n->cache_.first_;
      if( n->children_[i] == 0 )
        n->cache_.first_ = 0;
      else {
        n->cache_.first_ = (const void*)i;
        next = true;
      }
    }
  }
}

int
Manager::min_cost( const int *costs ) const
{
  // single bottom-up pass
  for( size_t i = 0, isz = node_pools_.size(); i < isz; ++i ) {
    NodePool *pool = node_pools_[i];
    for( size_t j = 0, jsz = pool->boundary_; j < jsz; ++j ) {
      const Node *n = &pool->table_[j];
      if( n->ref_ > 0 ) {
        int cost = 0;
        if( (n->type_ == And) || (n->type_ == Or) ) {
          bool first = true;
          for( const NodePtr *p = n->children_; *p != 0; ++p ) {
            int c = (int)(*p)->cache_.first_;
            if( n->type_ == And )
              cost = (c==INT_MAX?INT_MAX:cost+c);
            else {
              if( first ) {
                cost = c;
                first = false;
              }
              else
                cost = (c<cost?c:cost);
            }
          }
        }
        else if( n->type_ == Value )
          cost = ((int)n->children_?0:INT_MAX);
        else if( n->type_ == Variable )
          cost = (costs?costs[(int)n->children_]:0);
        n->cache_.set((void*)cost);
      }
    }
  }
  int result = (int)root_->cache_.first_;
  return(result);
}

void
Manager::recursive_min_model( const Node *n, Model &model ) const
{
  if( n->type_ == And ) {
    for( const NodePtr *p = n->children_; *p != 0; ++p )
      recursive_min_model(*p,model);
  }
  else if( n->type_ == Or ) {
    int mincost = 0;
    const NodePtr *minchild = 0;
    for( const NodePtr *p = n->children_; *p != 0; ++p ) {
      if( (minchild == 0) || ((int)(*p)->cache_.first_ < mincost) ) {
        mincost = (int)(*p)->cache_.first_;
        minchild = p;
      }
    }
    recursive_min_model(*minchild,model);
  }
  else if( n->type_ == Value )
    assert((int)n->children_);
  else if( n->type_ == Variable )
    model.insert((int)n->children_);
}

void
Manager::min_model( Model &model ) const
{
  model.clear();
  recursive_min_model(root_,model);
}

void
Manager::sorted_dump( std::ostream &os ) const
{
  size_t index = 0;
  for( size_t i = 0, isz = node_pools_.size(); i < isz; ++i ) {
    NodePool *pool = node_pools_[i];
    for( size_t j = 0, jsz = pool->boundary_; j < jsz; ++j ) {
      const Node *n = &pool->table_[j];
      if( n->ref_ > 0 ) {
        n->cache_.set( (const void*)index++ );
        if( n->type_ == Value )
          os << ((int)n->children_==0?"O 0 0":"A 0") << std::endl;
        else if( n->type_ == Variable )
          os << "L " << ((int)n->children_%2?-(((int)n->children_)>>1):((int)n->children_)>>1) << std::endl;
        else if( (n->type_ == And) || (n->type_ == Or) ) {
          size_t size = 0;
          for( const NodePtr *p = n->children_; *p != 0; ++p, ++size );
          os << (n->type_==And?"A ":"O 0 ") << size;
          for( const NodePtr *p = n->children_; *p != 0; ++p )
            os << " " << (size_t)(*p)->cache_.first_;
          os << std::endl;
        }
      }
    }
  }
}

void
Manager::recursive_dump( std::ostream &os, const Node *n, size_t &index ) const
{
  if( (int)n->cache_.second_ == 1 ) return;
  if( n->type_ == Value )
    os << ((int)n->children_==0?"O 0 0":"A 0") << std::endl;
  else if( n->type_ == Variable )
    os << "L " << ((int)n->children_%2?-(((int)n->children_)>>1):((int)n->children_)>>1) << std::endl;
  else if( (n->type_ == And) || (n->type_ == Or) ) {
    size_t size = 0;
    for( const NodePtr *p = n->children_; *p != 0; ++p, ++size )
      recursive_dump( os, *p, index );
    os << (n->type_==And?"A ":"O 0 ") << size;
    for( const NodePtr *p = n->children_; *p != 0; ++p )
      os << " " << (size_t)(*p)->cache_.first_;
    os << std::endl;
  }
  n->cache_.set( (const void*)index++, (const void*)1 );
}

}; // nnf namespace

