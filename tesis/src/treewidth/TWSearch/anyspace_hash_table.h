#ifndef ANYSPACE_HASH_TABLE_H
#define ANYSPACE_HASH_TABLE_H

/**************************************************************************/
/* An anyspace hash table stores items until the noresize() function	  */
/* is called, at which point a replacement strategy is used to prevent	  */
/* the table from increasing in size. anyspace_hash_table is a template	  */
/* taking a single argument, MultiIndexHashTable. This type should be a	  */
/* multi-indexed container where the second index is hashed. The first	  */
/* index will depend on the particular replacement			  */
/* strategy. MultiIndexHashTable must also implement two static		  */
/* functions: insert and exchange.					  */
/* 									  */
/* Below are two implementations of MultiIndexHashTable: ReplaceLongest	  */
/* and ReplaceLRU. ReplaceLongest implements a hash table where, once	  */
/* noresize() is called, the items with the largest size() are removed to */
/* make room for new items with a smaller size(). ReplaceLRU implements	  */
/* least-recently-used replacement, where, once noresize() is called, the */
/* least-recently inserted item is removed to make room for any new item. */
/**************************************************************************/

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/sequenced_index.hpp>

using namespace boost::multi_index;

struct hash_index_tag {};
struct aux_index_tag {};

template<typename Item, typename Hash = boost::hash<Item>, 
  typename Pred = std::equal_to<Item> >
  struct ReplaceLongest
  {
    typedef multi_index_container<
    Item,
      indexed_by<
      // sort by greater<int> on size()
      ordered_non_unique<
      tag<aux_index_tag>,
      const_mem_fun<Item, std::size_t, &Item::size>,
      std::greater<std::size_t> 
      >,
    // index by Hash and Pred
      hashed_unique<
      tag<hash_index_tag>,
      identity<Item>, Hash, Pred
      >
      >
      > type;
    typedef Item item_type;
    typedef typename type::iterator iterator;

    // insert item into t, if resize=true then use replacement scheme
    static std::pair<iterator,bool> 
      insert(type& t, bool resize, const item_type& item)
    {
      if (!resize && t.begin()->size() == item.size())
	return std::pair<iterator,bool>(t.end(),false);

      std::pair<iterator,bool> p=t.insert(item);

      if (p.second && !resize)
	t.erase(t.begin());

      return p;
    }

    // replace *old with item in t, bypassing replacement scheme
    static void exchange(type& t, iterator &old, const item_type& item)
    {
#ifndef NDEBUG
      static typename type::template nth_index<1>::type::key_equal pred;
      assert(pred(*old, item));
#endif
      t.erase(old);
      std::pair<iterator,bool> p=t.insert(item);
      assert(p.second);
    }
  };

template<typename Item, typename Hash = boost::hash<Item>, 
  typename Pred = std::equal_to<Item> >
  struct ReplaceLRU
  {
    typedef multi_index_container<
    Item,
      indexed_by<
      // sequence
      sequenced<tag<aux_index_tag> >,
    // index by Hash and Pred
      hashed_unique<tag<hash_index_tag>, identity<Item>, Hash, Pred>
      >
      > type;
    typedef Item item_type;
    typedef typename type::iterator iterator;

    // insert item into t, if resize=true then use replacement scheme
    static std::pair<iterator,bool> 
      insert(type& t, bool resize, const item_type& item)
    {
      std::pair<iterator,bool> p=t.push_front(item);

      if(!p.second){                   /* duplicate item */
	t.relocate(t.begin(),p.first); /* put in front */
	return p;
      }
      else if(!resize){  /* don't increase the length */
	t.pop_back();
      }
      return p;
    }

    // replace *old with item in t, bypassing replacement scheme
    static void exchange(type& t, iterator &old, const item_type& item)
    {
#ifndef NDEBUG
      static typename type::template nth_index<1>::type::key_equal pred;
      assert(pred(*old, item));
#endif
      t.erase(old);
      std::pair<iterator,bool> p=t.push_front(item);
      assert(p.second);
    }

  };

template<typename Item, typename Hash = boost::hash<Item>, 
  typename Pred = std::equal_to<Item> >
  struct ReplaceNone
  {
    typedef multi_index_container<
    Item,
      indexed_by<
      // index by Hash and Pred
      hashed_unique<
      tag<aux_index_tag,hash_index_tag>, 
      identity<Item>, 
      Hash, 
      Pred
      >
      >
      > type;
    typedef Item item_type;
    typedef typename type::iterator iterator;

    // insert item into t, if resize=true then use replacement scheme
    static std::pair<iterator,bool> 
      insert(type& t, bool resize, const item_type& item)
    {
      if (!resize)
	{
	  return std::pair<iterator,bool>(t.end(),false);
	}
      else
	{
	  std::pair<iterator,bool> p=t.insert(item);
	  return p;
	}
    }

    // replace *old with item in t, bypassing replacement scheme
    static void exchange(type& t, iterator &old, const item_type& item)
    {
#ifndef NDEBUG
      static typename type::template nth_index<1>::type::key_equal pred;
      assert(pred(*old, item));
#endif
      t.erase(old);
      std::pair<iterator,bool> p=t.insert(item);
      assert(p.second);
    }
  };

template<typename MultiIndexHashTable>
class anyspace_hash_table
{
  typedef typename MultiIndexHashTable::type table_type;

 public:
  typedef typename table_type::value_type item_type;
  typedef typename table_type::iterator iterator;
  typedef typename table_type::const_iterator const_iterator;
  typedef typename table_type::size_type size_type;

 anyspace_hash_table():m_resize(true){}

  void double_buckets()
  {
    m_table.get<hash_index_tag>().rehash(bucket_count()+1);
  }

  void set_noresize()
  {
    m_resize = false;
  }

  bool check_noresize()
  {
    return !m_resize;
  }

  // return false if a duplicate
  std::pair<iterator,bool> insert(const item_type& item)
    {
      return MultiIndexHashTable::insert(m_table, m_resize, item);
    }

  iterator find(const item_type& item)
  {
    return m_table.project<aux_index_tag>
      (m_table.get<hash_index_tag>().find(item));
  }

  size_type erase(const item_type& item)
  {
    return m_table.get<hash_index_tag>().erase(item);
  }

  void exchange(iterator &old, const item_type& item)
  {
    MultiIndexHashTable::exchange(m_table, old, item);
  }

  iterator begin(){return m_table.begin();}
  iterator end(){return m_table.end();}

  const_iterator begin() const {return m_table.begin();}
  const_iterator end() const {return m_table.end();}

  size_type size() const
  {
    return m_table.size();
  }

  size_type bucket_count() const
  {
    /*       const typename table_type::template nth_index<1>::type& hi = m_table.get<1>(); */
    /*       return hi.bucket_count(); */
    return m_table.get<hash_index_tag>().bucket_count();
  }

 private:
  table_type m_table;
  bool m_resize;
};

#endif
