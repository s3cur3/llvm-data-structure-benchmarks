/**
 * Defines replacements for vector, set, and map that are all write-once or "write-infrequently."
 * They deliberately eschew normal mutation operations (like push_back(), insert(), etc.)
 * and only support wholesale replacement. In exchange, you get:
 *
 *   - A dead-simple implementation.
 *   - Super fast iteration (you could even specialize them to have fixed-size,
 *     stack-based variants).
 *   - Lookups on map and set types that are O(n log n)---after all, if you're not modifying
 *     the data after initialization, you might as well sort it and do binary search lookups!
 *   - An API that makes it clear that, hey, you really shouldn't be manipulating this data.
 *     (Very useful for containers that are conceptually constant, but which you can't actually
 *     make const for whatever reason.)
 */

#ifndef LLVM_DATA_STRUCTURE_BENCHMARKS_ARRAYTYPES_H
#define LLVM_DATA_STRUCTURE_BENCHMARKS_ARRAYTYPES_H

#include <utility> // for std::pair
#include <initializer_list>

// NOTE: This is *not* a full implementation of the read-only portion of the std::vector API.
//       If you drop it in to production code, you'll probably have to add stuff.
//       The good news is, the implementation is simple, so doing so should be straightforward!
//
// NOTE ALSO: T must support std::move() semantics.
template<class T>
class FixedArray
{
public:
	typedef       T * iterator;
	typedef const T * const_iterator;

	template<class TContainer>  FixedArray(TContainer &container)               : m_begin(nullptr), m_end(nullptr) { replace(container.begin(), container.end()); }
	                            FixedArray(std::initializer_list<T> l)          : m_begin(nullptr), m_end(nullptr) { replace(l.begin(), l.end()); }
	template<class FwdIt>       FixedArray(FwdIt begin, FwdIt end, size_t size) : m_begin(nullptr), m_end(nullptr) { replace(begin, end, size); }
	template<class RndAccessIt> FixedArray(RndAccessIt begin, RndAccessIt end)  : m_begin(nullptr), m_end(nullptr) { replace(begin, end); }

	~FixedArray()                                 { clear(); }

	template<class TContainer>
	FixedArray & operator=(TContainer &container) { replace(container.begin(), container.end()); return *this; }

	      iterator begin()					      { return m_begin; }
	const_iterator begin() const			      { return m_begin; }
	      iterator end()					      { return m_end; }
	const_iterator end() const				      { return m_end; }

	const T & operator[](size_t idx) const        { return begin() + idx; };
	                                              
	size_t size() const                           { return m_end - m_begin; }
	bool   empty() const                          { return m_end == m_begin; }

	template<class RndAccessIt> void replace(RndAccessIt begin, RndAccessIt end)              { replace(begin, end, end - begin); }
	template<class It>          void replace(It          begin, It          end, size_t size)
	{
		for(T * next = m_begin; next < m_end; ++next)
		{
			next->~T(); // make sure we call the destructor!
		}

		if(size > 0)
		{
			if(size != (m_end - m_begin)) // need to allocate a new buffer
			{
				free(m_begin);
				m_begin = (T *)malloc(size * sizeof(T));
				m_end = m_begin + size;
			}

			T * next = m_begin;
			while(begin != end)
			{
				*next = std::move(*begin);
				++begin;
				++next;
			}
		}
		else
		{
			m_begin = nullptr;
			m_end = nullptr;
		}
	}

protected:
	void clear()
	{
		if(m_begin)
		{
			for(T * next = m_begin; next < m_end; ++next)
			{
				next->~T(); // make sure we call the destructor!
			}
			free(m_begin);
		}
		m_begin = nullptr;
		m_end = nullptr;
	}

private:
	T * m_begin;
	T * m_end; // one past the last element allocated
};

template <typename T, typename F=std::less<typename T::first_type> >
struct pair_sort_first_functor {
	F									func;
	typedef typename T::first_type		key_type;
	bool operator()(const T& lhs, const T& rhs) const { return func(lhs.first,rhs.first); }
	bool operator()(const T& lhs, const key_type& rhs) const { return func (lhs.first,rhs); }
	bool operator()(const key_type& lhs, const T& rhs) const { return func (lhs,rhs.first); }
};

template<class FwdIt>
static size_t compute_fwd_it_dist(FwdIt begin, FwdIt end)
{
	size_t out = 0;
	while(begin != end)
	{
		++out;
		++begin;
	}
	return out;
}

template<class KeyT, class ValueT>
class ArrayMap : public FixedArray<std::pair<KeyT, ValueT>>
{
public:
	typedef std::pair<KeyT, ValueT>            value_type;
	typedef FixedArray<value_type>             base_type;
	typedef typename base_type::iterator       iterator;
	typedef typename base_type::const_iterator const_iterator;

	template<class TContainer>  ArrayMap(TContainer &container)               : base_type(container.begin(), container.end())          { std::sort(base_type::begin(), base_type::end()); }
	                            ArrayMap(std::initializer_list<value_type> l) : base_type(l.begin(), l.end())                          { std::sort(base_type::begin(), base_type::end()); }
	template<class FwdIt>       ArrayMap(FwdIt begin, FwdIt end)              : base_type(begin, end, compute_fwd_it_dist(begin, end)) { std::sort(base_type::begin(), base_type::end()); }
	                            ArrayMap(std::map<KeyT, ValueT> &m)           : base_type(m.begin(), m.end(), m.size())                { std::sort(base_type::begin(), base_type::end()); }

	~ArrayMap()                                       { base_type::clear(); }

	template<class TContainer>
	ArrayMap & operator=(const TContainer &rhs)       { replace(rhs.begin(), rhs.end()); return *this; }

	size_t         count(const KeyT &key) const       { return std::binary_search(base_type::begin(), base_type::end(), key, pair_sort_first_functor<value_type>());	}
	iterator       find( const KeyT &key)             { auto i = std::lower_bound(base_type::begin(), base_type::end(), key, pair_sort_first_functor<value_type>()); return (i == base_type::end() || i->first != key) ? base_type::end() : i; }
	const_iterator find( const KeyT &key) const       { auto i = std::lower_bound(base_type::begin(), base_type::end(), key, pair_sort_first_functor<value_type>()); return (i == base_type::end() || i->first != key) ? base_type::end() : i; }
	const ValueT & at(   const KeyT &key) const       { return operator[](key);	}
	const ValueT & operator[](const KeyT &key) const  { assert(find(key) != base_type::end()); return find(key)->second; }

	template<class It>
	void replace(It begin, It end) { base_type::replace(begin, end); std::sort(begin(), end()); }
};


template <typename T>
class ArraySet : public FixedArray<T>
{
public:
	typedef FixedArray<T>                      base_type;

	template<class TContainer> ArraySet(TContainer &container)      : base_type(container.begin(), container.end())          { std::sort(base_type::begin(), base_type::end()); }
	                           ArraySet(std::initializer_list<T> l) : base_type(l.begin(), l.end())                          { std::sort(base_type::begin(), base_type::end()); }
	template<class FwdIt>      ArraySet(FwdIt begin, FwdIt end)     : base_type(begin, end, compute_fwd_it_dist(begin, end)) { std::sort(base_type::begin(), base_type::end()); }

	template<class TContainer>
	ArraySet & operator=(const TContainer &rhs) { replace(rhs.begin(), rhs.end()); return *this; }

	inline bool count(const T& key) const { return std::binary_search(base_type::begin(), base_type::end(), key); }

	template<typename It>
	void replace(It begin, It end) { base_type::replace(begin, end); std::sort(base_type::begin(), base_type::end());	}
};

#endif //LLVM_DATA_STRUCTURE_BENCHMARKS_ARRAYTYPES_H
