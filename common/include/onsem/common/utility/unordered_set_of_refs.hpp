#ifndef ONSEM_COMMON_UTILITY_UNORDERED_SET_OF_REFS_HPP
#define ONSEM_COMMON_UTILITY_UNORDERED_SET_OF_REFS_HPP

#include <unordered_set>

namespace onsem
{

namespace mystd
{

template<typename T>
class unordered_set_of_refs
{
public:
class iterator;
class const_iterator
{
public:
  const_iterator(typename std::unordered_set<T*>::const_iterator pIt);

  const_iterator& operator++();
  const T* operator->();
  const T& operator*();
  bool operator==(const const_iterator& pOther) const;
  bool operator!=(const const_iterator& pOther) const;
  bool operator==(const iterator& pOther) const;
  bool operator!=(const iterator& pOther) const;

private:
  friend class unordered_set_of_refs<T>;
  typename std::unordered_set<T*>::const_iterator _it;
};
class iterator
{
public:
  iterator(typename std::unordered_set<T*>::iterator pIt);

  iterator& operator++();
  T* operator->();
  T& operator*();
  bool operator==(const iterator& pOther) const;
  bool operator!=(const iterator& pOther) const;
  bool operator==(const const_iterator& pOther) const;
  bool operator!=(const const_iterator& pOther) const;

private:
  friend class unordered_set_of_refs<T>;
  typename std::unordered_set<T*>::iterator _it;
};

  void insert(T& pElt);
  template<typename ITTYPE>
  void insert(ITTYPE pBegin, ITTYPE pEnd);
  bool empty() const;
  std::size_t size() const;
  bool operator==(const unordered_set_of_refs<T>& pOther) const;
  bool operator!=(const unordered_set_of_refs<T>& pOther) const;

  iterator begin();
  iterator end();
  iterator find(T& pElt);
  iterator erase(const iterator& pIt);
  iterator erase(const const_iterator& pIt);
  void erase(const T& pElt);

  const_iterator begin() const;
  const_iterator end() const;
  const_iterator cbegin() const;
  const_iterator cend() const;
  const_iterator find(const T& pElt) const;

private:
  std::unordered_set<T*> _elts{};
};





template<typename T>
unordered_set_of_refs<T>::const_iterator::const_iterator(typename std::unordered_set<T*>::const_iterator pIt)
 : _it(pIt)
{
}

template<typename T>
typename unordered_set_of_refs<T>::const_iterator& unordered_set_of_refs<T>::const_iterator::operator++()
{
  ++_it;
  return *this;
}

template<typename T>
const T* unordered_set_of_refs<T>::const_iterator::operator->()
{
  return *_it;
}

template<typename T>
const T& unordered_set_of_refs<T>::const_iterator::operator*()
{
  return **_it;
}

template<typename T>
bool unordered_set_of_refs<T>::const_iterator::operator==(const typename unordered_set_of_refs<T>::const_iterator& pOther) const
{
  return _it == pOther._it;
}

template<typename T>
bool unordered_set_of_refs<T>::const_iterator::operator!=(const typename unordered_set_of_refs<T>::const_iterator& pOther) const
{
  return _it != pOther._it;
}

template<typename T>
bool unordered_set_of_refs<T>::const_iterator::operator==(const typename unordered_set_of_refs<T>::iterator& pOther) const
{
  return _it == pOther._it;
}

template<typename T>
bool unordered_set_of_refs<T>::const_iterator::operator!=(const typename unordered_set_of_refs<T>::iterator& pOther) const
{
  return _it != pOther._it;
}


template<typename T>
unordered_set_of_refs<T>::iterator::iterator(typename std::unordered_set<T*>::iterator pIt)
 : _it(pIt)
{
}

template<typename T>
typename unordered_set_of_refs<T>::iterator& unordered_set_of_refs<T>::iterator::operator++()
{
  ++_it;
  return *this;
}

template<typename T>
T* unordered_set_of_refs<T>::iterator::operator->()
{
  return *&*_it;
}

template<typename T>
T& unordered_set_of_refs<T>::iterator::operator*()
{
  return **_it;
}

template<typename T>
bool unordered_set_of_refs<T>::iterator::operator==(const typename unordered_set_of_refs<T>::iterator& pOther) const
{
  return _it == pOther._it;
}

template<typename T>
bool unordered_set_of_refs<T>::iterator::operator!=(const typename unordered_set_of_refs<T>::iterator& pOther) const
{
  return _it != pOther._it;
}

template<typename T>
bool unordered_set_of_refs<T>::iterator::operator==(const typename unordered_set_of_refs<T>::const_iterator& pOther) const
{
  return _it == pOther._it;
}

template<typename T>
bool unordered_set_of_refs<T>::iterator::operator!=(const typename unordered_set_of_refs<T>::const_iterator& pOther) const
{
  return _it != pOther._it;
}


template<typename T>
void unordered_set_of_refs<T>::insert(T& pElt)
{
  _elts.insert(&pElt);
}

template<typename T>
template<typename ITTYPE>
void unordered_set_of_refs<T>::insert(ITTYPE pBegin, ITTYPE pEnd)
{
  for (auto it = pBegin; it != pEnd; ++it)
    insert(*it);
}

template<typename T>
bool unordered_set_of_refs<T>::empty() const
{
  return _elts.empty();
}

template<typename T>
std::size_t unordered_set_of_refs<T>::size() const
{
  return _elts.size();
}

template<typename T>
bool unordered_set_of_refs<T>::operator==(const unordered_set_of_refs<T>& pOther) const
{
  return _elts == pOther._elts;
}

template<typename T>
bool unordered_set_of_refs<T>::operator!=(const unordered_set_of_refs<T>& pOther) const
{
    return _elts != pOther._elts;
}


template<typename T>
typename unordered_set_of_refs<T>::iterator unordered_set_of_refs<T>::begin()
{
  return unordered_set_of_refs<T>::iterator(_elts.begin());
}

template<typename T>
typename unordered_set_of_refs<T>::iterator unordered_set_of_refs<T>::end()
{
  return unordered_set_of_refs<T>::iterator(_elts.end());
}

template<typename T>
typename unordered_set_of_refs<T>::iterator unordered_set_of_refs<T>::find(T& pElt)
{
  return unordered_set_of_refs<T>::iterator(_elts.find(&pElt));
}

template<typename T>
typename unordered_set_of_refs<T>::iterator unordered_set_of_refs<T>::erase(const typename unordered_set_of_refs<T>::iterator& pIt)
{
  return unordered_set_of_refs<T>::iterator(_elts.erase(pIt._it));
}

template<typename T>
typename unordered_set_of_refs<T>::iterator unordered_set_of_refs<T>::erase(const typename unordered_set_of_refs<T>::const_iterator& pIt)
{
  return unordered_set_of_refs<T>::iterator(_elts.erase(pIt._it));
}

template<typename T>
void unordered_set_of_refs<T>::erase(const T& pElt)
{
  _elts.erase(const_cast<T*>(&pElt)); // very rare case where I think it's ok to do a const_cast
}


template<typename T>
typename unordered_set_of_refs<T>::const_iterator unordered_set_of_refs<T>::begin() const
{
  return unordered_set_of_refs<T>::const_iterator(_elts.begin());
}

template<typename T>
typename unordered_set_of_refs<T>::const_iterator unordered_set_of_refs<T>::end() const
{
  return unordered_set_of_refs<T>::const_iterator(_elts.end());
}

template<typename T>
typename unordered_set_of_refs<T>::const_iterator unordered_set_of_refs<T>::cbegin() const
{
  return unordered_set_of_refs<T>::const_iterator(_elts.begin());
}

template<typename T>
typename unordered_set_of_refs<T>::const_iterator unordered_set_of_refs<T>::cend() const
{
  return unordered_set_of_refs<T>::const_iterator(_elts.end());
}

template<typename T>
typename unordered_set_of_refs<T>::const_iterator unordered_set_of_refs<T>::find(const T& pElt) const
{
  return unordered_set_of_refs<T>::const_iterator(_elts.find(const_cast<T*>(&pElt))); // very rare case where I think it's ok to do a const_cast
}


} // End of namespace mystd
} // End of namespace onsem



#endif // ONSEM_COMMON_UTILITY_UNORDERED_SET_OF_REFS_HPP
