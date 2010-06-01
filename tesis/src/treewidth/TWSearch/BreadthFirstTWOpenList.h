#ifndef BREADTHFIRSTTWOPENLIST_H_
#define BREADTHFIRSTTWOPENLIST_H_

#include "preproc_flags.h"
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/random_access_index.hpp>
#include <boost/ref.hpp>
#include <boost/foreach.hpp>
#include <algorithm>
#include "BreadthFirstTWNode.h"

namespace Treewidth_Namespace {

  class BreadthFirstTWOpenList {
  private:

    struct hash_index_tag {};
    struct random_access_index_tag {};

#ifdef BFHT_FASTGRAPH    
    typedef boost::multi_index_container<
      BreadthFirstTWNode*,
      boost::multi_index::indexed_by<
        boost::multi_index::hashed_unique<
          boost::multi_index::tag<hash_index_tag>,
          boost::multi_index::identity<BreadthFirstTWNode>,
          boost::hash<BreadthFirstTWNode>,
          equal_to<BreadthFirstTWNode>
        >,
        boost::multi_index::random_access<
          boost::multi_index::tag<random_access_index_tag>
        >
      >
    > BFHT_OL_type;

    typedef BFHT_OL_type::index<random_access_index_tag>::type ol_by_random_access;

#else
    typedef boost::multi_index_container<
      BreadthFirstTWNode*,
      boost::multi_index::indexed_by<
        boost::multi_index::hashed_unique<
          boost::multi_index::tag<hash_index_tag>,
          boost::multi_index::identity<BreadthFirstTWNode>,
          boost::hash<BreadthFirstTWNode>,
          equal_to<BreadthFirstTWNode>
        >
      >
    > BFHT_OL_type;
#endif

    struct greater_BreadthFirstTWNode_ptr 
      : public binary_function<BreadthFirstTWNode*, BreadthFirstTWNode*, bool>
      {
	bool operator()(const BreadthFirstTWNode* x, const BreadthFirstTWNode* y)
	{
	  return *x < *y;
	}
      };

  public:

    void insert(BreadthFirstTWNode *n)
    {
      m_ol.insert(n);
    }

    BreadthFirstTWNode *toppop()
    {
#ifdef BFHT_FASTGRAPH
      // for fast removal, access from the back of the random access index
      BreadthFirstTWNode *n = m_ol.get<random_access_index_tag>().back();
      m_ol.get<random_access_index_tag>().pop_back();
      return n;
#else
      BFHT_OL_type::iterator ni = m_ol.begin();
      BreadthFirstTWNode *n = *ni;
      m_ol.erase(ni);
      return n;
#endif
    }

    const BreadthFirstTWNode *top() const
    {
#ifdef BFHT_FASTGRAPH    
      return m_ol.get<random_access_index_tag>().back();
#else
      return *m_ol.begin();
#endif
    }

    BreadthFirstTWNode *find(const BreadthFirstTWNode *n) const
    {
      BFHT_OL_type::const_iterator ni = m_ol.get<hash_index_tag>().find(*n);
      if (ni == m_ol.get<hash_index_tag>().end())
	return NULL;
      return *ni;
    }

    bool empty() const
    {
      return m_ol.empty();
    }

    BFHT_OL_type::size_type size() const
      {
	return m_ol.size();
      }

    BFHT_OL_type::size_type bucket_count() const
      {
	return m_ol.bucket_count();
      }

#ifdef BFHT_FASTGRAPH
    BFHT_OL_type::size_type capacity() const
      {
	return m_ol.get<random_access_index_tag>().capacity();
      }

    void sort()
    {
      vector<boost::reference_wrapper<BreadthFirstTWNode* const> > v;
      BOOST_FOREACH(BreadthFirstTWNode* const &i, m_ol.get<random_access_index_tag>())v.push_back(boost::cref(i));
      std::sort(v.begin(), v.end(), greater_BreadthFirstTWNode_ptr());
      m_ol.get<random_access_index_tag>().rearrange(v.begin());
    }
#endif

  private:
	
    BFHT_OL_type m_ol;

  };

};
#endif /*BREADTHFIRSTTWOPENLIST_H_*/
