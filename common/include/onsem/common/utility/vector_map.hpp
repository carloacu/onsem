#ifndef ONSEM_COMMON_UTILITY_VECTOR_MAP_HPP
#define ONSEM_COMMON_UTILITY_VECTOR_MAP_HPP

#include <functional>
#include <string>
#include <vector>
#include <memory>
#include <assert.h>
#include <onsem/common/utility/make_unique.hpp>


namespace onsem
{
namespace mystd
{

template<typename KEY_TYPE, typename VALUE_TYPE, typename VALUE_TYPE_OF_REF, typename MAP_TYPE>
struct vector_map_iterator;

template<typename KEY_TYPE, typename VALUE_TYPE>
struct vector_map
{
  vector_map();
  vector_map(vector_map&& pOther);
  vector_map(const vector_map& pOther);
  vector_map& operator=(vector_map&& pOther);
  vector_map& operator=(const vector_map& pOther);
  VALUE_TYPE& operator[](const KEY_TYPE& pKeys);
  bool operator==(const vector_map& pOther) const { return _content == pOther._content; }
  bool operator!=(const vector_map& pOther) const { return !operator==(pOther); }

  typedef vector_map_iterator<KEY_TYPE, VALUE_TYPE, VALUE_TYPE, vector_map> iterator;
  typedef vector_map_iterator<KEY_TYPE, VALUE_TYPE, const VALUE_TYPE, const vector_map> const_iterator;

  template<typename... Args>
  void emplace(const KEY_TYPE& pKey,
               Args&&... pArgs);
  void emplace(const std::pair<const KEY_TYPE, VALUE_TYPE&>& pPair);
  void emplace(const std::pair<const KEY_TYPE, const VALUE_TYPE&>& pPair);

  iterator begin();
  iterator end();
  iterator find(const KEY_TYPE& pKey);
  iterator upper_bound(const KEY_TYPE& pKey);
  iterator next(std::size_t pIndex);
  iterator prev(std::size_t pIndex);
  VALUE_TYPE* find_ptr(const KEY_TYPE& pKey);

  const_iterator begin() const;
  const_iterator end() const;
  const_iterator find(const KEY_TYPE& pKey) const;
  const_iterator upper_bound(const KEY_TYPE& pKey) const;
  bool upper_bound_index(std::size_t& pIndex,
                         const KEY_TYPE& pKey) const;
  const_iterator next(std::size_t pIndex) const;
  const_iterator prev(std::size_t pIndex) const;
  const VALUE_TYPE* find_ptr(const KEY_TYPE& pKey) const;
  bool find_index(std::size_t& pIndex,
                  const KEY_TYPE& pKey) const;
  std::size_t count(const KEY_TYPE& pKey) const;
  std::size_t erase(const KEY_TYPE& pKey);
  void erase(const iterator& pIt);
  void erase(const const_iterator& pIt);

  void clear();
  bool empty() const;
  std::size_t size() const;

  std::string printDebugList(const std::function<std::string(const KEY_TYPE&)>& pPrintKeyFunc,
                             const std::function<std::string(const VALUE_TYPE&)>& pPrintValueFunc) const;

  std::vector<std::pair<KEY_TYPE, VALUE_TYPE>>& getContent() { return _content; }
  const std::vector<std::pair<KEY_TYPE, VALUE_TYPE>>& getContent() const { return _content; }

private:
  std::vector<std::pair<KEY_TYPE, VALUE_TYPE>> _content;

  template<typename... Args>
  VALUE_TYPE& _get(const KEY_TYPE& pKey,
                   Args&&... pArgs);
};



template<typename KEY_TYPE, typename VALUE_TYPE, typename VALUE_TYPE_OF_REF, typename MAP_TYPE>
struct vector_map_iterator
{
  typedef std::pair<const KEY_TYPE, VALUE_TYPE_OF_REF&> reference;

  vector_map_iterator(MAP_TYPE& pMap)
    : _mapPtr(&pMap),
      _index(0),
      _valuePtr(nullptr),
      _reference(mystd::make_unique<reference>(KEY_TYPE(), *_valuePtr))
  {
  }

  template <typename PAIR_TYPE>
  vector_map_iterator(MAP_TYPE& pMap,
                      std::size_t pIndex,
                      PAIR_TYPE& pPair)
    : _mapPtr(&pMap),
      _index(pIndex),
      _valuePtr(&pPair.second),
      _reference(mystd::make_unique<reference>(pPair.first, pPair.second))
  {
  }

  vector_map_iterator(const vector_map_iterator& pOther)
    : _mapPtr(pOther._mapPtr),
      _index(pOther._index),
      _valuePtr(pOther._valuePtr),
      _reference(mystd::make_unique<reference>(*pOther._reference))
  {
  }

  vector_map_iterator& operator=(const vector_map_iterator& pOther)
  {
    _mapPtr = pOther._mapPtr;
    _index = pOther._index;
    _valuePtr = pOther._valuePtr;
    _reference = mystd::make_unique<reference>(*pOther._reference);
    return *this;
  }

  bool operator==(const vector_map_iterator& pOther) const
  {
    return _mapPtr == pOther._mapPtr &&
        _index == pOther._index &&
        _valuePtr == pOther._valuePtr &&
        (_valuePtr == nullptr ||
         _reference->first == pOther._reference->first);
  }

  bool operator!=(const vector_map_iterator& pOther) const
  {
    return !operator==(pOther);
  }

  reference* operator->()  { return &*_reference; }
  const reference* operator->() const  { return &*_reference; }
  reference& operator*()  { return *_reference; }
  const reference& operator*() const  { return *_reference; }

  vector_map_iterator& operator++()
  {
    *this = _mapPtr->next(_index);
    return *this;
  }

  vector_map_iterator& operator--()
  {
    if (_valuePtr == nullptr)
      *this = _mapPtr->prev(_mapPtr->size());
    else
      *this = _mapPtr->prev(_index);
    return *this;
  }

  std::size_t getIndex() const { return _index; }

private:
  MAP_TYPE* _mapPtr;
  std::size_t _index;
  VALUE_TYPE_OF_REF* _valuePtr;
  std::unique_ptr<reference> _reference;
};



template<typename KEY_TYPE, typename VALUE_TYPE>
vector_map<KEY_TYPE, VALUE_TYPE>::vector_map()
  : _content()
{
}


template<typename KEY_TYPE, typename VALUE_TYPE>
vector_map<KEY_TYPE, VALUE_TYPE>::vector_map(vector_map&& pOther)
  : _content(std::move(pOther._content))
{
}

template<typename KEY_TYPE, typename VALUE_TYPE>
vector_map<KEY_TYPE, VALUE_TYPE>::vector_map(const vector_map& pOther)
  : _content(pOther._content)
{
}


template<typename KEY_TYPE, typename VALUE_TYPE>
vector_map<KEY_TYPE, VALUE_TYPE>& vector_map<KEY_TYPE, VALUE_TYPE>::operator=(vector_map&& pOther)
{
  _content = std::move(pOther._content);
  return *this;
}

template<typename KEY_TYPE, typename VALUE_TYPE>
vector_map<KEY_TYPE, VALUE_TYPE>& vector_map<KEY_TYPE, VALUE_TYPE>::operator=(const vector_map& pOther)
{
  _content = pOther._content;
  return *this;
}


template<typename KEY_TYPE, typename VALUE_TYPE>
bool vector_map<KEY_TYPE, VALUE_TYPE>::find_index(
    std::size_t& pIndex,
    const KEY_TYPE& pKey) const
{
  const auto keys_size = _content.size();
  auto size = keys_size;
  while (true)
  {
    if (size < 5)
    {
      auto afterLastIndex = pIndex + size;
      while (pIndex < afterLastIndex)
      {
        auto& val = _content[pIndex].first;
        if (val == pKey)
          return true;
        if (val > pKey)
          return false;
        ++pIndex;
      }
      return false;
    }

    std::size_t newSize = size >> 1; // -> size / 2;
    std::size_t newIndex = pIndex + newSize;
    auto& val = _content[newIndex].first;
    if (val == pKey)
    {
      pIndex = newIndex;
      return true;
    }

    if (val > pKey)
    {
      size = newSize;
      continue;
    }

    pIndex = newIndex + 1;
    size = size - newSize - 1;
  }
  assert(false);
  return false;
}


template<typename KEY_TYPE, typename VALUE_TYPE>
template<typename... Args>
VALUE_TYPE& vector_map<KEY_TYPE, VALUE_TYPE>::_get(
    const KEY_TYPE& pKey,
    Args&&... pArgs)
{
  std::size_t index = 0;
  if (find_index(index, pKey))
    return _content[index].second;
  _content.insert(_content.begin() + index, std::pair<const KEY_TYPE, VALUE_TYPE>(pKey, VALUE_TYPE(std::forward<Args>(pArgs)...)));
  return _content[index].second;
}


template<typename KEY_TYPE, typename VALUE_TYPE>
VALUE_TYPE& vector_map<KEY_TYPE, VALUE_TYPE>::operator[](
    const KEY_TYPE& pKey)
{
  return _get(pKey);
}


template<typename KEY_TYPE, typename VALUE_TYPE>
template<typename... Args>
void vector_map<KEY_TYPE, VALUE_TYPE>::emplace(
    const KEY_TYPE& pKey,
    Args&&... pArgs)
{
  _get(pKey, std::forward<Args>(pArgs)...);
}


template<typename KEY_TYPE, typename VALUE_TYPE>
void vector_map<KEY_TYPE, VALUE_TYPE>::emplace(
    const std::pair<const KEY_TYPE, VALUE_TYPE&>& pPair)
{
  _get(pPair.first, pPair.second);
}


template<typename KEY_TYPE, typename VALUE_TYPE>
void vector_map<KEY_TYPE, VALUE_TYPE>::emplace(
    const std::pair<const KEY_TYPE, const VALUE_TYPE&>& pPair)
{
  _get(pPair.first, pPair.second);
}


template<typename KEY_TYPE, typename VALUE_TYPE>
typename vector_map<KEY_TYPE, VALUE_TYPE>::iterator vector_map<KEY_TYPE, VALUE_TYPE>::begin()
{
  if (_content.empty())
    return vector_map<KEY_TYPE, VALUE_TYPE>::iterator(*this);
  return vector_map<KEY_TYPE, VALUE_TYPE>::iterator(*this, 0, _content[0]);
}

template<typename KEY_TYPE, typename VALUE_TYPE>
typename vector_map<KEY_TYPE, VALUE_TYPE>::iterator vector_map<KEY_TYPE, VALUE_TYPE>::end()
{
  return vector_map<KEY_TYPE, VALUE_TYPE>::iterator(*this);
}

template<typename KEY_TYPE, typename VALUE_TYPE>
typename vector_map<KEY_TYPE, VALUE_TYPE>::const_iterator vector_map<KEY_TYPE, VALUE_TYPE>::begin() const
{
  if (_content.empty())
    return vector_map<KEY_TYPE, VALUE_TYPE>::const_iterator(*this);
  return vector_map<KEY_TYPE, VALUE_TYPE>::const_iterator(*this, 0, _content[0]);
}

template<typename KEY_TYPE, typename VALUE_TYPE>
typename vector_map<KEY_TYPE, VALUE_TYPE>::const_iterator vector_map<KEY_TYPE, VALUE_TYPE>::end() const
{
  return vector_map<KEY_TYPE, VALUE_TYPE>::const_iterator(*this);
}


template<typename KEY_TYPE, typename VALUE_TYPE>
typename vector_map<KEY_TYPE, VALUE_TYPE>::iterator vector_map<KEY_TYPE, VALUE_TYPE>::find(const KEY_TYPE& pKey)
{
  std::size_t index = 0;
  if (find_index(index, pKey))
    return vector_map<KEY_TYPE, VALUE_TYPE>::iterator(*this, index, _content[index]);
  return vector_map<KEY_TYPE, VALUE_TYPE>::iterator(*this);
}

template<typename KEY_TYPE, typename VALUE_TYPE>
typename vector_map<KEY_TYPE, VALUE_TYPE>::iterator vector_map<KEY_TYPE, VALUE_TYPE>::upper_bound(const KEY_TYPE& pKey)
{
  std::size_t index = 0;
  if (upper_bound_index(index, pKey))
    return vector_map<KEY_TYPE, VALUE_TYPE>::iterator(*this, index, _content[index]);
  return vector_map<KEY_TYPE, VALUE_TYPE>::iterator(*this);
}

template<typename KEY_TYPE, typename VALUE_TYPE>
typename vector_map<KEY_TYPE, VALUE_TYPE>::iterator vector_map<KEY_TYPE, VALUE_TYPE>::next(std::size_t pIndex)
{
  ++pIndex;
  if (_content.size() > pIndex)
    return vector_map<KEY_TYPE, VALUE_TYPE>::iterator(*this, pIndex, _content[pIndex]);
  return vector_map<KEY_TYPE, VALUE_TYPE>::iterator(*this);
}

template<typename KEY_TYPE, typename VALUE_TYPE>
typename vector_map<KEY_TYPE, VALUE_TYPE>::iterator vector_map<KEY_TYPE, VALUE_TYPE>::prev(std::size_t pIndex)
{
  if (pIndex > 0)
  {
    --pIndex;
    return vector_map<KEY_TYPE, VALUE_TYPE>::iterator(*this, pIndex, _content[pIndex]);
  }
  return vector_map<KEY_TYPE, VALUE_TYPE>::iterator(*this);
}

template<typename KEY_TYPE, typename VALUE_TYPE>
VALUE_TYPE* vector_map<KEY_TYPE, VALUE_TYPE>::find_ptr(const KEY_TYPE& pKey)
{
  std::size_t index = 0;
  if (find_index(index, pKey))
    return &_content[index].second;
  return nullptr;
}

template<typename KEY_TYPE, typename VALUE_TYPE>
typename vector_map<KEY_TYPE, VALUE_TYPE>::const_iterator vector_map<KEY_TYPE, VALUE_TYPE>::find(const KEY_TYPE& pKey) const
{
  std::size_t index = 0;
  if (find_index(index, pKey))
    return vector_map<KEY_TYPE, VALUE_TYPE>::const_iterator(*this, index, _content[index]);
  return vector_map<KEY_TYPE, VALUE_TYPE>::const_iterator(*this);
}

template<typename KEY_TYPE, typename VALUE_TYPE>
typename vector_map<KEY_TYPE, VALUE_TYPE>::const_iterator vector_map<KEY_TYPE, VALUE_TYPE>::upper_bound(const KEY_TYPE& pKey) const
{
  std::size_t index = 0;
  if (upper_bound_index(index, pKey))
    return vector_map<KEY_TYPE, VALUE_TYPE>::const_iterator(*this, index, _content[index]);
  return vector_map<KEY_TYPE, VALUE_TYPE>::const_iterator(*this);
}

template<typename KEY_TYPE, typename VALUE_TYPE>
bool vector_map<KEY_TYPE, VALUE_TYPE>::upper_bound_index(
    std::size_t& pIndex,
    const KEY_TYPE& pKey) const
{
  if (find_index(pIndex, pKey))
    ++pIndex;
  return pIndex < _content.size();
}

template<typename KEY_TYPE, typename VALUE_TYPE>
typename vector_map<KEY_TYPE, VALUE_TYPE>::const_iterator vector_map<KEY_TYPE, VALUE_TYPE>::next(std::size_t pIndex) const
{
  ++pIndex;
  if (_content.size() > pIndex)
    return vector_map<KEY_TYPE, VALUE_TYPE>::const_iterator(*this, pIndex, _content[pIndex]);
  return vector_map<KEY_TYPE, VALUE_TYPE>::const_iterator(*this);
}

template<typename KEY_TYPE, typename VALUE_TYPE>
typename vector_map<KEY_TYPE, VALUE_TYPE>::const_iterator vector_map<KEY_TYPE, VALUE_TYPE>::prev(std::size_t pIndex) const
{
  if (pIndex > 0)
  {
    --pIndex;
    return vector_map<KEY_TYPE, VALUE_TYPE>::const_iterator(*this, pIndex, _content[pIndex]);
  }
  return vector_map<KEY_TYPE, VALUE_TYPE>::const_iterator(*this);
}

template<typename KEY_TYPE, typename VALUE_TYPE>
const VALUE_TYPE* vector_map<KEY_TYPE, VALUE_TYPE>::find_ptr(const KEY_TYPE& pKey) const
{
  std::size_t index = 0;
  if (find_index(index, pKey))
    return &_content[index].second;
  return nullptr;
}

template<typename KEY_TYPE, typename VALUE_TYPE>
std::size_t vector_map<KEY_TYPE, VALUE_TYPE>::count(const KEY_TYPE& pKey) const
{
  std::size_t index = 0;
  if (find_index(index, pKey))
    return 1;
  return 0;
}


template<typename KEY_TYPE, typename VALUE_TYPE>
std::size_t vector_map<KEY_TYPE, VALUE_TYPE>::erase(const KEY_TYPE& pKey)
{
  std::size_t index = 0;
  if (find_index(index, pKey))
  {
    _content.erase(_content.begin() + index);
    return 1;
  }
  return 0;
}


template<typename KEY_TYPE, typename VALUE_TYPE>
void vector_map<KEY_TYPE, VALUE_TYPE>::erase(const iterator& pIt)
{
  auto index = pIt.getIndex();
  _content.erase(_content.begin() + index);
}


template<typename KEY_TYPE, typename VALUE_TYPE>
void vector_map<KEY_TYPE, VALUE_TYPE>::erase(const const_iterator& pIt)
{
  auto index = pIt.getIndex();
  _content.erase(_content.begin() + index);
}



template<typename KEY_TYPE, typename VALUE_TYPE>
void vector_map<KEY_TYPE, VALUE_TYPE>::clear()
{
  _content.clear();
}


template<typename KEY_TYPE, typename VALUE_TYPE>
bool vector_map<KEY_TYPE, VALUE_TYPE>::empty() const
{
  return _content.empty();
}

template<typename KEY_TYPE, typename VALUE_TYPE>
std::size_t vector_map<KEY_TYPE, VALUE_TYPE>::size() const
{
  return _content.size();
}


template<typename KEY_TYPE, typename VALUE_TYPE>
std::string vector_map<KEY_TYPE, VALUE_TYPE>::printDebugList(
    const std::function<std::string(const KEY_TYPE&)>& pPrintKeyFunc,
    const std::function<std::string(const VALUE_TYPE&)>& pPrintValueFunc) const
{
  std::string res;
  for (const auto& currElt : *this)
    res += pPrintKeyFunc(currElt.first) + " -> " + pPrintValueFunc(currElt.second) + "\n";
  return res;
}



} // end namespace mystd
} // end namespace onsem



#endif // ONSEM_COMMON_UTILITY_VECTOR_MAP_HPP
