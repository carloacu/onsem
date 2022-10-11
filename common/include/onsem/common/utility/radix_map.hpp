#ifndef ONSEM_COMMON_UTILITY_RADIX_MAP_HPP
#define ONSEM_COMMON_UTILITY_RADIX_MAP_HPP

#include <cstring>
#include <functional>
#include <memory>
#include <stack>
#include <string>
#include <assert.h>
#include <initializer_list>
#include <onsem/common/utility/radix_map_forward_declaration.hpp>
#include <onsem/common/utility/vector_map.hpp>
#include "detail/searchendingpoint.hpp"

namespace onsem
{
namespace mystd
{

template<typename KEY_TYPE, typename KEY_CONTAINER_TYPE, typename VALUE_TYPE, typename VALUE_TYPE_OF_REF, typename MAP_TYPE>
struct radix_map_iterator;



template<typename KEY_TYPE, typename KEY_CONTAINER_TYPE, typename VALUE_TYPE>
struct radix_map
{
  radix_map();
  radix_map(radix_map&& pOther);
  radix_map(const radix_map& pOther);
  radix_map(std::initializer_list<std::pair<const KEY_CONTAINER_TYPE, VALUE_TYPE>> pInitList);
  radix_map& operator=(radix_map&& pOther);
  radix_map& operator=(const radix_map& pOther);
  VALUE_TYPE& operator[](const KEY_CONTAINER_TYPE& pKeys);

  typedef radix_map_iterator<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE, VALUE_TYPE, radix_map> iterator;
  typedef radix_map_iterator<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE, const VALUE_TYPE, const radix_map> const_iterator;

  template<typename... Args>
  void emplace(const KEY_CONTAINER_TYPE& pKey,
               Args&&... pArgs);
  std::pair<iterator, bool> insert(const KEY_CONTAINER_TYPE& pKeys);

  iterator begin();
  iterator beforeEnd();
  iterator end();
  iterator find(const KEY_CONTAINER_TYPE& pKeyContainer);
  VALUE_TYPE* find_ptr(const KEY_CONTAINER_TYPE& pKeys);
  iterator lower_bound(const KEY_CONTAINER_TYPE& pKeys);
  iterator upper_bound(const KEY_CONTAINER_TYPE& pKeys);
  iterator prev(const KEY_CONTAINER_TYPE& pKeys);
  void for_each(const std::function<void(const KEY_CONTAINER_TYPE&, VALUE_TYPE&)>& pFunction);
  std::size_t erase(const KEY_CONTAINER_TYPE& pKeys);
  iterator erase(const iterator& pIt);
  const_iterator erase(const const_iterator& pIt);

  const_iterator cbegin() const;
  const_iterator cend() const;
  const_iterator begin() const;
  const_iterator beforeEnd() const;
  const_iterator end() const;
  const_iterator find(const KEY_CONTAINER_TYPE& pKeyContainer) const;
  const VALUE_TYPE* find_ptr(const KEY_CONTAINER_TYPE& pKeys,
                             std::size_t pBegin,
                             std::size_t pSize) const;
  const VALUE_TYPE* find_ptr(const KEY_CONTAINER_TYPE& pKeys) const;
  std::size_t count(const KEY_CONTAINER_TYPE& pKeys) const;
  const_iterator lower_bound(const KEY_CONTAINER_TYPE& pKeys) const;
  const_iterator upper_bound(const KEY_CONTAINER_TYPE& pKeys) const;
  const_iterator prev(const KEY_CONTAINER_TYPE& pKeys) const;
  void for_each(const std::function<void(const KEY_CONTAINER_TYPE&, const VALUE_TYPE&)>& pFunction) const;
  const std::vector<std::pair<KEY_TYPE, radix_map>>& getChildrenContent() const;
  const KEY_CONTAINER_TYPE& getKeys() const { return _keys; }
  const VALUE_TYPE* getValuePtr() const { return _value_ptr ? &*_value_ptr : nullptr; }

  std::string printDebugTree(const std::function<std::string(const VALUE_TYPE&)>& pPrintFunc,
                             std::size_t pBegin = 0,
                             const KEY_TYPE* pFirstCharPtr = nullptr) const;

  std::string printDebugList(const std::function<std::string(const VALUE_TYPE&)>& pPrintFunc) const;

  void clear();
  bool empty() const { return _children.empty() && _value_ptr.get() == nullptr; }
  std::size_t nbChildren() const { return _children.size(); }

  std::size_t getMaxLength(const std::string& pKeys,
                           const std::size_t pKeys_begin_in) const;


private:
  KEY_CONTAINER_TYPE _keys;
  mystd::vector_map<KEY_TYPE, radix_map<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE>> _children;
  std::unique_ptr<VALUE_TYPE> _value_ptr;

  static VALUE_TYPE* _begin(
      KEY_CONTAINER_TYPE& pKeys,
      const radix_map<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE>& pMap);

  static VALUE_TYPE* _beforeEnd(
      KEY_CONTAINER_TYPE& pKeys,
      const radix_map<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE>& pMap);

  template<typename VALUE_TYPE_MAYBE_CONST, typename MAP_TYPE>
  static VALUE_TYPE_MAYBE_CONST* _find(MAP_TYPE& pMap,
                                       const KEY_CONTAINER_TYPE& pKeys,
                                       std::size_t pKeys_begin,
                                       std::size_t pKeys_size);

  template<typename VALUE_TYPE_OF_REF, typename MAP_TYPE>
  static void _for_each(MAP_TYPE& pMap,
                        const std::function<void(const KEY_CONTAINER_TYPE&, VALUE_TYPE_OF_REF&)>& pFunction,
                        const KEY_CONTAINER_TYPE& pBeginOfKeysKeys);

  template<typename VALUE_TYPE_OF_REF, typename MAP_TYPE>
  static radix_map_iterator<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE, VALUE_TYPE_OF_REF, MAP_TYPE> _next(
      MAP_TYPE& pMap,
      MAP_TYPE* pCurrNode,
      const KEY_CONTAINER_TYPE& pKeyPrefix,
      std::size_t pChildIndex);

  template<typename VALUE_TYPE_OF_REF, typename MAP_TYPE>
  static radix_map_iterator<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE, VALUE_TYPE_OF_REF, MAP_TYPE> _advance(
      MAP_TYPE& pMap,
      const KEY_CONTAINER_TYPE& pKeys,
      SearchEndingPoint pSearchEndingPoint);

  std::unique_ptr<VALUE_TYPE>& _get(const KEY_CONTAINER_TYPE& pKeys);
};



template<typename KEY_TYPE, typename KEY_CONTAINER_TYPE, typename VALUE_TYPE, typename VALUE_TYPE_OF_REF, typename MAP_TYPE>
struct radix_map_iterator
{
  typedef std::pair<const KEY_CONTAINER_TYPE, VALUE_TYPE_OF_REF&> reference;

  radix_map_iterator(MAP_TYPE& pMap)
    : _mapPtr(&pMap),
      _valuePtr(nullptr),
      _reference(std::make_unique<reference>(KEY_CONTAINER_TYPE(), *_valuePtr))
  {
  }

  radix_map_iterator(MAP_TYPE& pMap,
                     const KEY_CONTAINER_TYPE& pKeyContainer,
                     VALUE_TYPE_OF_REF& pValue)
    : _mapPtr(&pMap),
      _valuePtr(&pValue),
      _reference(std::make_unique<reference>(pKeyContainer, pValue))
  {
  }

  radix_map_iterator(const radix_map_iterator& pOther)
    : _mapPtr(pOther._mapPtr),
      _valuePtr(pOther._valuePtr),
      _reference(std::make_unique<reference>(*pOther._reference))
  {
  }

  radix_map_iterator& operator=(const radix_map_iterator& pOther)
  {
    _mapPtr = pOther._mapPtr;
    _valuePtr = pOther._valuePtr;
    _reference = std::make_unique<reference>(*pOther._reference);
    return *this;
  }

  bool operator==(const radix_map_iterator& pOther) const
  {
    return _mapPtr == pOther._mapPtr &&
        _valuePtr == pOther._valuePtr &&
        (_valuePtr == nullptr ||
         _reference->first == pOther._reference->first);
  }

  bool operator!=(const radix_map_iterator& pOther) const
  {
    return !operator==(pOther);
  }

  reference* operator->()  { return &*_reference; }
  const reference* operator->() const  { return &*_reference; }
  reference& operator*()  { return *_reference; }
  const reference& operator*() const  { return *_reference; }


  radix_map_iterator& operator++()
  {
    *this = _mapPtr->upper_bound(_reference->first);
    return *this;
  }

  radix_map_iterator& operator--()
  {
    if (_valuePtr == nullptr)
      *this = _mapPtr->beforeEnd();
    else
      *this = _mapPtr->prev(_reference->first);
    return *this;
  }



private:
  MAP_TYPE* _mapPtr;
  VALUE_TYPE_OF_REF* _valuePtr;
  std::unique_ptr<reference> _reference;
};



template<typename KEY_TYPE, typename KEY_CONTAINER_TYPE, typename VALUE_TYPE>
radix_map<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE>::radix_map()
  : _keys(),
    _children(),
    _value_ptr()
{
}


template<typename KEY_TYPE, typename KEY_CONTAINER_TYPE, typename VALUE_TYPE>
radix_map<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE>::radix_map(radix_map&& pOther)
  : _keys(std::move(pOther._keys)),
    _children(std::move(pOther._children)),
    _value_ptr(std::move(pOther._value_ptr))
{
}

template<typename KEY_TYPE, typename KEY_CONTAINER_TYPE, typename VALUE_TYPE>
radix_map<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE>::radix_map(const radix_map& pOther)
  : _keys(pOther._keys),
    _children(pOther._children),
    _value_ptr()
{
  if (pOther._value_ptr)
    _value_ptr = std::make_unique<VALUE_TYPE>(*pOther._value_ptr);
}

template<typename KEY_TYPE, typename KEY_CONTAINER_TYPE, typename VALUE_TYPE>
radix_map<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE>::radix_map(std::initializer_list<std::pair<const KEY_CONTAINER_TYPE, VALUE_TYPE>> pInitList)
{
  for (const auto& e : pInitList)
    operator[](e.first) = e.second;
}


template<typename KEY_TYPE, typename KEY_CONTAINER_TYPE, typename VALUE_TYPE>
radix_map<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE>& radix_map<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE>::operator=(radix_map&& pOther)
{
  _keys = std::move(pOther._keys);
  _children = std::move(pOther._children);
  _value_ptr = std::move(pOther._value_ptr);
  return *this;
}

template<typename KEY_TYPE, typename KEY_CONTAINER_TYPE, typename VALUE_TYPE>
radix_map<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE>& radix_map<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE>::operator=(const radix_map& pOther)
{
  _keys = pOther._keys;
  _children = pOther._children;
  if (pOther._value_ptr)
    _value_ptr = std::make_unique<VALUE_TYPE>(*pOther._value_ptr);
  return *this;
}


template<typename KEY_TYPE, typename KEY_CONTAINER_TYPE, typename VALUE_TYPE>
VALUE_TYPE* radix_map<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE>::_begin(
    KEY_CONTAINER_TYPE& pKeys,
    const radix_map<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE>& pMap)
{
  auto* currNode = &pMap;
  while (true)
  {
    pKeys += currNode->_keys;
    if (currNode->_value_ptr)
      return &*currNode->_value_ptr;

    if (!currNode->_children.empty())
    {
      auto& firstChild = currNode->_children.getContent()[0];
      pKeys += firstChild.first;
      currNode = &firstChild.second;
      continue;
    }
    break;
  }
  return nullptr;
}


template<typename KEY_TYPE, typename KEY_CONTAINER_TYPE, typename VALUE_TYPE>
VALUE_TYPE* radix_map<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE>::_beforeEnd(
    KEY_CONTAINER_TYPE& pKeys,
    const radix_map<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE>& pMap)
{
  auto* currNode = &pMap;
  while (true)
  {
    pKeys += currNode->_keys;
    if (!currNode->_children.empty())
    {
      auto& lastChild = currNode->_children.getContent().back();
      pKeys += lastChild.first;
      currNode = &lastChild.second;
      continue;
    }
    if (currNode->_value_ptr)
      return &*currNode->_value_ptr;
    break;
  }
  return nullptr;
}


template<typename KEY_TYPE, typename KEY_CONTAINER_TYPE, typename VALUE_TYPE>
template<typename VALUE_TYPE_MAYBE_CONST, typename MAP_TYPE>
VALUE_TYPE_MAYBE_CONST* radix_map<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE>::_find(
    MAP_TYPE& pMap,
    const KEY_CONTAINER_TYPE& pKeys,
    std::size_t pKeys_begin,
    std::size_t pKeys_size)
{
  auto* currNode = &pMap;
  while (true)
  {
    auto keys_size = currNode->_keys.size();
    if (keys_size <= pKeys_size &&
        (keys_size == 0 || memcmp(currNode->_keys.data(), pKeys.data() + pKeys_begin, keys_size) == 0))
    {
      if (keys_size == pKeys_size)
      {
        if (currNode->_value_ptr)
          return &*currNode->_value_ptr;
        return nullptr;
      }
      // so keys_size < pKeys_size
      auto* childPtr = currNode->_children.find_ptr(pKeys[pKeys_begin + keys_size]);
      if (childPtr != nullptr)
      {
        pKeys_size = pKeys_size - keys_size - 1;
        pKeys_begin += keys_size + 1;
        currNode = childPtr;
        continue;
      }
    }
    break;
  }
  return nullptr;
}


template<typename KEY_TYPE, typename KEY_CONTAINER_TYPE, typename VALUE_TYPE>
template<typename VALUE_TYPE_OF_REF, typename MAP_TYPE>
void radix_map<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE>::_for_each(
    MAP_TYPE& pMap,
    const std::function<void(const KEY_CONTAINER_TYPE&, VALUE_TYPE_OF_REF&)>& pFunction,
    const KEY_CONTAINER_TYPE& pBeginOfKeysKeys)
{
  auto newKeys = pBeginOfKeysKeys + pMap._keys;
  if (pMap._value_ptr)
    pFunction(newKeys, *pMap._value_ptr);
  auto& childrenContent = pMap._children.getContent();
  for (auto i = 0u; i < childrenContent.size(); ++i)
    _for_each(childrenContent[i].second, pFunction, newKeys + childrenContent[i].first);
}


template<typename KEY_TYPE, typename KEY_CONTAINER_TYPE, typename VALUE_TYPE>
template<typename VALUE_TYPE_OF_REF, typename MAP_TYPE>
radix_map_iterator<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE, VALUE_TYPE_OF_REF, MAP_TYPE> radix_map<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE>::_next(
    MAP_TYPE& pMap,
    MAP_TYPE* pCurrNode,
    const KEY_CONTAINER_TYPE& pKeyPrefix,
    std::size_t pChildIndex)
{
  KEY_CONTAINER_TYPE keysAfterPrefix;
  while (true)
  {
    auto& childPair = pCurrNode->_children.getContent()[pChildIndex];
    pCurrNode = &childPair.second;
    keysAfterPrefix += childPair.first + pCurrNode->_keys;
    if (pCurrNode->_value_ptr)
      return radix_map_iterator<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE, VALUE_TYPE_OF_REF, MAP_TYPE>
          (pMap, pKeyPrefix + keysAfterPrefix, *pCurrNode->_value_ptr);
    pChildIndex = 0;
  }
  return radix_map_iterator<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE, VALUE_TYPE_OF_REF, MAP_TYPE>(pMap);
}


template<typename KEY_TYPE, typename KEY_CONTAINER_TYPE, typename VALUE_TYPE>
template<typename VALUE_TYPE_OF_REF, typename MAP_TYPE>
radix_map_iterator<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE, VALUE_TYPE_OF_REF, MAP_TYPE> radix_map<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE>::_advance(
    MAP_TYPE& pMap,
    const KEY_CONTAINER_TYPE& pKeys,
    SearchEndingPoint pSearchEndingPoint)
{
  std::size_t pKeys_begin = 0;
  std::size_t pKeys_size = pKeys.size();
  std::stack<std::pair<MAP_TYPE*, std::size_t>> parentNodes;
  std::stack<KEY_CONTAINER_TYPE> nextKeysStack;
  auto* currNode = &pMap;
  if (!pMap._keys.empty())
    nextKeysStack.emplace(pMap._keys);
  auto getKeysPrefix = [&]
  {
    KEY_CONTAINER_TYPE res;
    while (!nextKeysStack.empty())
    {
      res = nextKeysStack.top() + res;
      nextKeysStack.pop();
    }
    return res;
  };
  auto goOnNext = [&] {
    if (currNode->_value_ptr)
    {
      KEY_CONTAINER_TYPE keysAfterPrefix;
      return radix_map_iterator<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE, VALUE_TYPE_OF_REF, MAP_TYPE>
          (pMap, getKeysPrefix(), *currNode->_value_ptr);
    }
    else
    {
      auto& nodeContent = currNode->_children.getContent();
      if (!nodeContent.empty())
        return _next<VALUE_TYPE_OF_REF, MAP_TYPE>(pMap, currNode, getKeysPrefix(), 0);
    }
    return radix_map_iterator<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE, VALUE_TYPE_OF_REF, MAP_TYPE>(pMap);
  };
  auto goOnNextFromParent = [&] {
    while (!parentNodes.empty())
    {
      nextKeysStack.pop();
      auto& parentNode = parentNodes.top();
      ++parentNode.second;
      auto& parentContent = parentNode.first->_children.getContent();
      if (parentNode.second < parentContent.size())
      {
        auto& parentPair = parentContent[parentNode.second];
        KEY_CONTAINER_TYPE nextKeys = getKeysPrefix() + parentPair.first;
        auto* valuePtr = _begin(nextKeys, parentPair.second);
        if (valuePtr != nullptr)
          return radix_map_iterator<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE, VALUE_TYPE_OF_REF, MAP_TYPE>(pMap, nextKeys, *valuePtr);
        break;
      }
      parentNodes.pop();
    }
    return radix_map_iterator<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE, VALUE_TYPE_OF_REF, MAP_TYPE>(pMap);
  };

  while (true)
  {
    auto keys_size = currNode->_keys.size();
    if (keys_size <= pKeys_size &&
        (keys_size == 0 || memcmp(currNode->_keys.data(), pKeys.data() + pKeys_begin, keys_size) == 0))
    {
      auto& keyOfChild = pKeys[pKeys_begin + keys_size];
      if (keys_size == pKeys_size)
      {
        if (currNode->_value_ptr)
        {
          switch (pSearchEndingPoint)
          {
          case SearchEndingPoint::PREV:
          {
            while (!parentNodes.empty())
            {
              nextKeysStack.pop();
              auto& parentNode = parentNodes.top();
              auto& parentContent = parentNode.first->_children.getContent();
              if (parentNode.second > 0)
              {
                --parentNode.second;
                auto& parentPair = parentContent[parentNode.second];
                KEY_CONTAINER_TYPE prevKeys = getKeysPrefix() + parentPair.first;
                auto* valuePtr = _beforeEnd(prevKeys, parentPair.second);
                if (valuePtr != nullptr)
                  return radix_map_iterator<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE, VALUE_TYPE_OF_REF, MAP_TYPE>(pMap, prevKeys, *valuePtr);
                break;
              }
              if (parentNode.first->_value_ptr)
                return radix_map_iterator<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE, VALUE_TYPE_OF_REF, MAP_TYPE>
                    (pMap, getKeysPrefix(), *parentNode.first->_value_ptr);
              parentNodes.pop();
            }
            return radix_map_iterator<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE, VALUE_TYPE_OF_REF, MAP_TYPE>(pMap);
          }
          case SearchEndingPoint::EQUAL_OR_NEXT:
            return radix_map_iterator<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE, VALUE_TYPE_OF_REF, MAP_TYPE>(pMap, pKeys, *currNode->_value_ptr);
          case SearchEndingPoint::NEXT:
            break;
          };
        }
      }
      else
      {
        // so keys_size < pKeys_size
        std::size_t childIndex = 0;
        if (currNode->_children.find_index(childIndex, keyOfChild))
        {
          pKeys_size = pKeys_size - keys_size - 1;
          pKeys_begin += keys_size + 1;
          parentNodes.emplace(std::make_pair(currNode, childIndex));
          auto& childRef = currNode->_children.getContent()[childIndex].second;
          nextKeysStack.emplace(keyOfChild + childRef._keys);
          currNode = &childRef;
          continue;
        }
      }

      // next
      std::size_t childIndex = 0;
      if (currNode->_children.upper_bound_index(childIndex, keyOfChild))
        return _next<VALUE_TYPE_OF_REF, MAP_TYPE>(pMap, currNode, getKeysPrefix(), childIndex);
      return goOnNextFromParent();
    }
    else if (pSearchEndingPoint == SearchEndingPoint::EQUAL_OR_NEXT ||
             pSearchEndingPoint == SearchEndingPoint::NEXT)
    {
      if (pKeys_size == 0 ||
          (pKeys_size < keys_size && memcmp(currNode->_keys.data(), pKeys.data() + pKeys_begin, pKeys_size) == 0))
      {
        return goOnNext();
      }
      else
      {
        std::size_t i = 0;
        bool nodeKeysAreBeforeOrAfter =  keys_size < pKeys_size;
        while (pKeys_size > 0 && keys_size > 0)
        {
          const auto& nodeChar = currNode->_keys.data()[i];
          const auto& inChar = pKeys.data()[pKeys_begin + i];
          if (nodeChar != inChar)
          {
            nodeKeysAreBeforeOrAfter = nodeChar < inChar;
            break;
          }
          ++i;
          --pKeys_size;
          --keys_size;
        }
        if (nodeKeysAreBeforeOrAfter)
          return goOnNextFromParent();
        return goOnNext();
      }
    }
    break;
  }
  return radix_map_iterator<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE, VALUE_TYPE_OF_REF, MAP_TYPE>(pMap);
}


template<typename KEY_TYPE, typename KEY_CONTAINER_TYPE, typename VALUE_TYPE>
std::unique_ptr<VALUE_TYPE>& radix_map<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE>::_get(
    const KEY_CONTAINER_TYPE& pKeys)
{
  auto* currNode = this;
  std::size_t pKeys_begin = 0;
  std::size_t pKeys_size = pKeys.size();

  while (true)
  {
    auto keys_size = currNode->_keys.size();
    std::size_t lastPosNotInCommon = 0;

    if (keys_size <= pKeys_size)
    {
      if (keys_size == 0 || memcmp(currNode->_keys.data(), pKeys.data() + pKeys_begin, keys_size) == 0)
      {
        if (keys_size == pKeys_size)
          return currNode->_value_ptr;

        if (!currNode->_value_ptr && currNode->_children.empty())
        {
          currNode->_keys.insert(currNode->_keys.end(), pKeys.begin() + pKeys_begin, pKeys.end());
          return currNode->_value_ptr;
        }
        // so keys_size < pKeys_size
        pKeys_size = pKeys_size - keys_size - 1;
        pKeys_begin += keys_size;
        currNode = &currNode->_children[pKeys[pKeys_begin]];
        ++pKeys_begin;
        continue;
      }

      while (lastPosNotInCommon < keys_size &&
             currNode->_keys[lastPosNotInCommon] == pKeys[lastPosNotInCommon + pKeys_begin])
        ++lastPosNotInCommon;
    }
    else
    {
      while (lastPosNotInCommon < pKeys_size &&
             currNode->_keys[lastPosNotInCommon] == pKeys[lastPosNotInCommon + pKeys_begin])
        ++lastPosNotInCommon;
    }

    {
      auto nextChildren = std::move(currNode->_children);
      auto& oldNode = currNode->_children[currNode->_keys[lastPosNotInCommon]];
      if (lastPosNotInCommon < keys_size)
        oldNode._keys.insert(oldNode._keys.end(), currNode->_keys.begin() + lastPosNotInCommon + 1, currNode->_keys.end());
      oldNode._children = std::move(nextChildren);
      oldNode._value_ptr = std::move(currNode->_value_ptr);
    }

    if (lastPosNotInCommon < keys_size)
    {
      currNode->_keys.erase(currNode->_keys.begin() + lastPosNotInCommon, currNode->_keys.end());
      if (pKeys_size > lastPosNotInCommon)
      {
        auto& newNode = currNode->_children[pKeys[pKeys_begin + lastPosNotInCommon]];
        newNode._keys.insert(newNode._keys.end(), pKeys.begin() + pKeys_begin + lastPosNotInCommon + 1, pKeys.end());
        return newNode._value_ptr;
      }
    }
    return currNode->_value_ptr;
  }
  assert(false);
  return _value_ptr;
}


template<typename KEY_TYPE, typename KEY_CONTAINER_TYPE, typename VALUE_TYPE>
VALUE_TYPE& radix_map<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE>::operator[](
    const KEY_CONTAINER_TYPE& pKeys)
{
  auto& valPtr = _get(pKeys);
  if (!valPtr)
    valPtr = std::make_unique<VALUE_TYPE>();
  return *valPtr;
}


template<typename KEY_TYPE, typename KEY_CONTAINER_TYPE, typename VALUE_TYPE>
template<typename... Args>
void radix_map<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE>::emplace(
    const KEY_CONTAINER_TYPE& pKeys,
    Args&&... pArgs)
{
  auto& valPtr = _get(pKeys);
  if (!valPtr)
    valPtr = std::make_unique<VALUE_TYPE>(std::forward<Args>(pArgs)...);
}


template<typename KEY_TYPE, typename KEY_CONTAINER_TYPE, typename VALUE_TYPE>
std::pair<typename radix_map<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE>::iterator, bool> radix_map<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE>::insert(
    const KEY_CONTAINER_TYPE& pKeys)
{
  bool inserted = false;
  auto& valPtr = _get(pKeys);
  if (!valPtr)
  {
    valPtr = std::make_unique<VALUE_TYPE>();
    inserted = true;
  }
  return {radix_map<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE>::iterator(*this, pKeys, *valPtr), inserted};
}


template<typename KEY_TYPE, typename KEY_CONTAINER_TYPE, typename VALUE_TYPE>
typename radix_map<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE>::iterator radix_map<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE>::begin()
{
  KEY_CONTAINER_TYPE keys;
  auto* valPtr = _begin(keys, *this);
  if (valPtr != nullptr)
    return radix_map<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE>::iterator(*this, keys, *valPtr);
  return radix_map<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE>::iterator(*this);
}

template<typename KEY_TYPE, typename KEY_CONTAINER_TYPE, typename VALUE_TYPE>
typename radix_map<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE>::iterator radix_map<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE>::beforeEnd()
{
  KEY_CONTAINER_TYPE keys;
  auto* valPtr = _beforeEnd(keys, *this);
  if (valPtr != nullptr)
    return radix_map<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE>::iterator(*this, keys, *valPtr);
  return radix_map<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE>::iterator(*this);
}

template<typename KEY_TYPE, typename KEY_CONTAINER_TYPE, typename VALUE_TYPE>
typename radix_map<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE>::iterator radix_map<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE>::end()
{
  return radix_map<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE>::iterator(*this);
}

template<typename KEY_TYPE, typename KEY_CONTAINER_TYPE, typename VALUE_TYPE>
typename radix_map<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE>::const_iterator radix_map<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE>::cbegin() const
{
  KEY_CONTAINER_TYPE keys;
  auto* valPtr = _begin(keys, *this);
  if (valPtr != nullptr)
    return radix_map<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE>::const_iterator(*this, keys, *valPtr);
  return radix_map<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE>::const_iterator(*this);
}

template<typename KEY_TYPE, typename KEY_CONTAINER_TYPE, typename VALUE_TYPE>
typename radix_map<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE>::const_iterator radix_map<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE>::cend() const
{
  return radix_map<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE>::const_iterator(*this);
}

template<typename KEY_TYPE, typename KEY_CONTAINER_TYPE, typename VALUE_TYPE>
typename radix_map<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE>::const_iterator radix_map<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE>::begin() const
{
  KEY_CONTAINER_TYPE keys;
  auto* valPtr = _begin(keys, *this);
  if (valPtr != nullptr)
    return radix_map<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE>::const_iterator(*this, keys, *valPtr);
  return radix_map<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE>::const_iterator(*this);
}

template<typename KEY_TYPE, typename KEY_CONTAINER_TYPE, typename VALUE_TYPE>
typename radix_map<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE>::const_iterator radix_map<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE>::beforeEnd() const
{
  KEY_CONTAINER_TYPE keys;
  auto* valPtr = _beforeEnd(keys, *this);
  if (valPtr != nullptr)
    return radix_map<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE>::const_iterator(*this, keys, *valPtr);
  return radix_map<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE>::const_iterator(*this);
}

template<typename KEY_TYPE, typename KEY_CONTAINER_TYPE, typename VALUE_TYPE>
typename radix_map<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE>::const_iterator radix_map<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE>::end() const
{
  return radix_map<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE>::const_iterator(*this);
}


template<typename KEY_TYPE, typename KEY_CONTAINER_TYPE, typename VALUE_TYPE>
typename radix_map<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE>::iterator radix_map<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE>::find(const KEY_CONTAINER_TYPE& pKeys)
{
  auto* valPtr = _find<VALUE_TYPE>(*this, pKeys, 0, pKeys.size());
  if (valPtr != nullptr)
    return radix_map<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE>::iterator(*this, pKeys, *valPtr);
  return radix_map<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE>::iterator(*this);
}


template<typename KEY_TYPE, typename KEY_CONTAINER_TYPE, typename VALUE_TYPE>
typename radix_map<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE>::const_iterator radix_map<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE>::find(const KEY_CONTAINER_TYPE& pKeys) const
{
  auto* valPtr = _find<const VALUE_TYPE>(*this, pKeys, 0, pKeys.size());
  if (valPtr != nullptr)
    return radix_map<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE>::const_iterator(*this, pKeys, *valPtr);
  return radix_map<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE>::const_iterator(*this);
}


template<typename KEY_TYPE, typename KEY_CONTAINER_TYPE, typename VALUE_TYPE>
typename radix_map<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE>::iterator radix_map<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE>::lower_bound(const KEY_CONTAINER_TYPE& pKeys)
{
  return _advance<VALUE_TYPE>(*this, pKeys, SearchEndingPoint::EQUAL_OR_NEXT);
}

template<typename KEY_TYPE, typename KEY_CONTAINER_TYPE, typename VALUE_TYPE>
typename radix_map<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE>::const_iterator radix_map<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE>::lower_bound(const KEY_CONTAINER_TYPE& pKeys) const
{
  return _advance<const VALUE_TYPE, const radix_map>(*this, pKeys, SearchEndingPoint::EQUAL_OR_NEXT);
}


template<typename KEY_TYPE, typename KEY_CONTAINER_TYPE, typename VALUE_TYPE>
typename radix_map<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE>::iterator radix_map<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE>::upper_bound(const KEY_CONTAINER_TYPE& pKeys)
{
  return _advance<VALUE_TYPE>(*this, pKeys, SearchEndingPoint::NEXT);
}

template<typename KEY_TYPE, typename KEY_CONTAINER_TYPE, typename VALUE_TYPE>
typename radix_map<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE>::const_iterator radix_map<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE>::upper_bound(const KEY_CONTAINER_TYPE& pKeys) const
{
  return _advance<const VALUE_TYPE, const radix_map>(*this, pKeys, SearchEndingPoint::NEXT);
}


template<typename KEY_TYPE, typename KEY_CONTAINER_TYPE, typename VALUE_TYPE>
typename radix_map<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE>::iterator radix_map<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE>::prev(const KEY_CONTAINER_TYPE& pKeys)
{
  return _advance<VALUE_TYPE>(*this, pKeys, SearchEndingPoint::PREV);
}

template<typename KEY_TYPE, typename KEY_CONTAINER_TYPE, typename VALUE_TYPE>
typename radix_map<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE>::const_iterator radix_map<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE>::prev(const KEY_CONTAINER_TYPE& pKeys) const
{
  return _advance<const VALUE_TYPE, const radix_map>(*this, pKeys, SearchEndingPoint::PREV);
}

template<typename KEY_TYPE, typename KEY_CONTAINER_TYPE, typename VALUE_TYPE>
VALUE_TYPE* radix_map<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE>::find_ptr(const KEY_CONTAINER_TYPE& pKeys)
{
  return _find<VALUE_TYPE>(*this, pKeys, 0, pKeys.size());
}

template<typename KEY_TYPE, typename KEY_CONTAINER_TYPE, typename VALUE_TYPE>
const VALUE_TYPE* radix_map<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE>::find_ptr(const KEY_CONTAINER_TYPE& pKeys,
    std::size_t pBegin,
    std::size_t pSize) const
{
  return _find<const VALUE_TYPE>(*this, pKeys, pBegin, pSize);
}


template<typename KEY_TYPE, typename KEY_CONTAINER_TYPE, typename VALUE_TYPE>
const VALUE_TYPE* radix_map<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE>::find_ptr(const KEY_CONTAINER_TYPE& pKeys) const
{
  return _find<const VALUE_TYPE>(*this, pKeys, 0, pKeys.size());
}

template<typename KEY_TYPE, typename KEY_CONTAINER_TYPE, typename VALUE_TYPE>
std::size_t radix_map<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE>::count(const KEY_CONTAINER_TYPE& pKeys) const
{
  return _find<const VALUE_TYPE>(*this, pKeys, 0, pKeys.size()) != nullptr ? 1 : 0;
}

template<typename KEY_TYPE, typename KEY_CONTAINER_TYPE, typename VALUE_TYPE>
void radix_map<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE>::for_each(
    const std::function<void(const KEY_CONTAINER_TYPE&, VALUE_TYPE&)>& pFunction)
{
  KEY_CONTAINER_TYPE beginOfKeysKeys;
  _for_each(*this, pFunction, beginOfKeysKeys);
}

template<typename KEY_TYPE, typename KEY_CONTAINER_TYPE, typename VALUE_TYPE>
void radix_map<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE>::for_each(
    const std::function<void(const KEY_CONTAINER_TYPE&, const VALUE_TYPE&)>& pFunction) const
{
  KEY_CONTAINER_TYPE beginOfKeysKeys;
  _for_each(*this, pFunction, beginOfKeysKeys);
}

template<typename KEY_TYPE, typename KEY_CONTAINER_TYPE, typename VALUE_TYPE>
const std::vector<std::pair<KEY_TYPE, radix_map<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE>>>& radix_map<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE>::getChildrenContent() const
{
  return _children.getContent();
}


template<typename KEY_TYPE, typename KEY_CONTAINER_TYPE, typename VALUE_TYPE>
std::size_t radix_map<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE>::erase(
    const KEY_CONTAINER_TYPE& pKeys)
{
  radix_map* parentNode = nullptr;
  KEY_TYPE firstKey;
  auto* currNode = this;
  std::size_t pKeys_begin = 0;
  std::size_t pKeys_size = pKeys.size();
  while (true)
  {
    auto keys_size = currNode->_keys.size();
    if (keys_size <= pKeys_size &&
        (keys_size == 0 || memcmp(currNode->_keys.data(), pKeys.data() + pKeys_begin, keys_size) == 0))
    {
      if (keys_size == pKeys_size)
      {
        if (currNode->_value_ptr)
        {
          currNode->_value_ptr.reset();
          std::size_t currNodeChildrenSize = currNode->_children.size();
          radix_map* nodeToMergeWithHisFirstChild = nullptr;
          if (currNodeChildrenSize == 0)
          {
            if (parentNode != nullptr)
            {
              parentNode->_children.erase(firstKey);
              if (!parentNode->_value_ptr &&
                  parentNode->_children.size() == 1)
                nodeToMergeWithHisFirstChild = parentNode;
            }
            else
            {
              currNode->_keys.clear();
            }
          }
          else if (currNodeChildrenSize == 1)
          {
            nodeToMergeWithHisFirstChild = currNode;
          }

          if (nodeToMergeWithHisFirstChild != nullptr)
          {
            auto& firstChild = nodeToMergeWithHisFirstChild->_children.getContent()[0];
            currNode = &firstChild.second;
            nodeToMergeWithHisFirstChild->_keys += firstChild.first + currNode->_keys;
            nodeToMergeWithHisFirstChild->_value_ptr = std::move(currNode->_value_ptr);
            auto currNodeChildren =  std::move(currNode->_children);
            nodeToMergeWithHisFirstChild->_children =  std::move(currNodeChildren);
          }
          return 1;
        }
        return 0;
      }
      // so keys_size < pKeys_size
      firstKey = pKeys[pKeys_begin + keys_size];
      std::size_t childIndex = 0;
      if (currNode->_children.find_index(childIndex, firstKey))
      {
        pKeys_size = pKeys_size - keys_size - 1;
        pKeys_begin += keys_size + 1;
        parentNode = currNode;
        currNode = &currNode->_children.getContent()[childIndex].second;
        continue;
      }
    }
    break;
  }
  return 0;
}


template<typename KEY_TYPE, typename KEY_CONTAINER_TYPE, typename VALUE_TYPE>
typename radix_map<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE>::iterator radix_map<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE>::erase(const iterator& pIt)
{
  auto nextIt = upper_bound(pIt->first);
  erase(pIt->first);
  return nextIt;
}

template<typename KEY_TYPE, typename KEY_CONTAINER_TYPE, typename VALUE_TYPE>
typename radix_map<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE>::const_iterator radix_map<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE>::erase(const const_iterator& pIt)
{
  const auto& constThis = *this;
  auto nextIt = constThis.upper_bound(pIt->first);
  erase(pIt->first);
  return nextIt;
}


template<typename KEY_TYPE, typename KEY_CONTAINER_TYPE, typename VALUE_TYPE>
std::string radix_map<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE>::printDebugTree(
    const std::function<std::string(const VALUE_TYPE&)>& pPrintFunc,
    std::size_t pBegin,
    const KEY_TYPE* pFirstCharPtr) const
{
  std::string res;
  for (std::size_t i = 0; i < pBegin; ++i)
    res += " ";
  auto offset = pBegin + _keys.size();
  if (pFirstCharPtr != nullptr)
  {
    res += *pFirstCharPtr;
    ++offset;
  }
  res += _keys;
  if (_value_ptr)
    res += " -> " + pPrintFunc(*_value_ptr) + "\n";
  else if (pFirstCharPtr != nullptr)
    res += "\n";
  for (const auto& currChild : _children)
    res += currChild.second.printDebugTree(pPrintFunc, offset, &currChild.first);
  return res;
}


template<typename KEY_TYPE, typename KEY_CONTAINER_TYPE, typename VALUE_TYPE>
std::string radix_map<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE>::printDebugList(
    const std::function<std::string(const VALUE_TYPE&)>& pPrintFunc) const
{
  std::string res;
  for_each([&](const KEY_CONTAINER_TYPE& pKeyContainer, const VALUE_TYPE& pValue) mutable {
    res += pKeyContainer + " -> " + pPrintFunc(pValue) + "\n";
  });
  return res;
}


template<typename KEY_TYPE, typename KEY_CONTAINER_TYPE, typename VALUE_TYPE>
void radix_map<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE>::clear()
{
  _keys.clear();
  _children.clear();
  _value_ptr.reset();
}


template<typename KEY_TYPE, typename KEY_CONTAINER_TYPE, typename VALUE_TYPE>
std::size_t radix_map<KEY_TYPE, KEY_CONTAINER_TYPE, VALUE_TYPE>::getMaxLength(const std::string& pKeys,
                                                                              const std::size_t pKeys_begin_in) const
{
  std::size_t res = 0;
  std::size_t pKeys_begin = pKeys_begin_in;
  std::size_t pKeys_end = pKeys.size();
  auto* currNode = this;
  while (true)
  {
    auto keys_size = currNode->_keys.size();
    auto pKeys_size = pKeys_end - pKeys_begin;
    std::size_t pos = pKeys_begin + keys_size;
    if (keys_size <= pKeys_size &&
        (keys_size == 0 || memcmp(currNode->_keys.data(), pKeys.data() + pKeys_begin, keys_size) == 0))
    {
      if (currNode->_value_ptr)
        res = pos - pKeys_begin_in;
      if (keys_size == pKeys_size)
        return res;

      // so keys_size < pKeys_size
      std::size_t childIndex = 0;
      if (currNode->_children.find_index(childIndex, pKeys[pos]))
      {
        pKeys_begin = pos + 1;
        currNode = &currNode->_children.getContent()[childIndex].second;
        continue;
      }
    }
    break;
  }
  return res;
}



} // end namespace mystd
} // end namespace onsem



#endif // ONSEM_COMMON_UTILITY_RADIX_MAP_HPP
