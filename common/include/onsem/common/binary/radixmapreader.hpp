#ifndef ONSEM_COMMON_BINARY_RADIXMAPREADER_HPP
#define ONSEM_COMMON_BINARY_RADIXMAPREADER_HPP

#include <stack>
#include <cstring>
#include <string>
#include <functional>
#include "binaryloader.hpp"
#include <onsem/common/utility/detail/searchendingpoint.hpp>
#include <onsem/common/api.hpp>

namespace onsem
{
namespace radixmap
{

struct ONSEM_COMMON_API StaticRadixMapIterator
{
  StaticRadixMapIterator(std::string pKey,
                         const unsigned char* pValue)
    : key(pKey),
      value(pValue)
  {
  }
  bool empty() const { return value == nullptr; }

  std::string key;
  const unsigned char* value;
};


static inline std::size_t _readNbOfKeys(const unsigned char*& pPtr)
{
  unsigned char nbOfKeysC = *(pPtr++);
  std::size_t nbOfKeys = nbOfKeysC;
  while (nbOfKeysC == 255)
  {
    nbOfKeysC = *(pPtr++);
    nbOfKeys += nbOfKeysC;
  }
  return nbOfKeys;
}


static inline const unsigned char* read
(const unsigned char* pPtr,
 const std::string& pKey,
 std::size_t pBeginPos,
 std::size_t pEndPos,
 const std::function<bool(const unsigned char*)>& pIsEndOfAWord)
{
  pPtr = binaryloader::alignMemory(pPtr);
  const unsigned char* beginMemoryPtr = pPtr;
  bool continueLoop = false;
  do
  {
    continueLoop = false;
    std::size_t nbOfKeys = _readNbOfKeys(pPtr);
    std::size_t keySize = pEndPos - pBeginPos;
    if (keySize < nbOfKeys ||
        memcmp(pPtr, pKey.data() + pBeginPos, nbOfKeys) != 0)
      return nullptr;
    pPtr += nbOfKeys;
    unsigned char nbOfChildren = *(pPtr++);

    if (keySize == nbOfKeys)
    {
      int nbOfChildrenInt = nbOfChildren;
      pPtr = pPtr + (nbOfChildrenInt << 2); // * 4
      return pIsEndOfAWord(pPtr) ? pPtr : nullptr;
    }

    std::size_t childKeyIndex = pBeginPos + nbOfKeys;
    char charToFind = pKey[childKeyIndex];

    for (unsigned char i = 0u; i < nbOfChildren; ++i)
    {
      const int* offsetOfTheCild = reinterpret_cast<const int*>(pPtr);
      pPtr += 3;
      char firstletter = *(pPtr++);
      if (charToFind == firstletter)
      {
        pPtr = beginMemoryPtr + binaryloader::alignedDecToInt(*offsetOfTheCild);
        pBeginPos = childKeyIndex + 1;
        continueLoop = true;
        break;
      }
      if (charToFind < firstletter)
        return nullptr;
    }
  }
  while (continueLoop);
  return nullptr;
}


static inline std::string _getKeysStrFromRadixNodePtr(const unsigned char* pPtr)
{
  std::size_t nbOfKeys = _readNbOfKeys(pPtr);
  return {reinterpret_cast<const char*>(pPtr), nbOfKeys};
}


static inline StaticRadixMapIterator _read_begin
(const unsigned char* pPtr,
 const unsigned char* pBeginMemoryPtr,
 const std::string& pKeyPrefix,
 const std::function<bool(const unsigned char*)>& pIsEndOfAWord)
{
  std::string keysAfterPrefix;
  while (true)
  {
    keysAfterPrefix += _getKeysStrFromRadixNodePtr(pPtr);
    std::size_t nbOfKeys = _readNbOfKeys(pPtr);
    pPtr += nbOfKeys;
    unsigned char nbOfChildren = *(pPtr++);
    {
      int nbOfChildrenInt = nbOfChildren;
      const unsigned char* endOfWordPtr = pPtr + (nbOfChildrenInt << 2); // * 4
      if (pIsEndOfAWord(endOfWordPtr))
        return {pKeyPrefix + keysAfterPrefix, endOfWordPtr};
    }
    if (nbOfChildren > 0)
    {
      keysAfterPrefix += *(pPtr + 3);
      pPtr = pBeginMemoryPtr + binaryloader::alignedDecToInt(*reinterpret_cast<const int*>(pPtr));
      continue;
    }
    break;
  }
  return {"", nullptr};
}


static inline StaticRadixMapIterator _read_end(
    const unsigned char* pPtr,
    const unsigned char* pBeginMemoryPtr,
    const std::string& pKeyPrefix,
    const std::function<bool(const unsigned char*)>& pIsEndOfAWord)
{
  std::string keysAfterPrefix;
  while (true)
  {
    keysAfterPrefix += _getKeysStrFromRadixNodePtr(pPtr);
    std::size_t nbOfKeys = _readNbOfKeys(pPtr);
    pPtr += nbOfKeys;
    unsigned char nbOfChildren = *(pPtr++);
    if (nbOfChildren > 0)
    {
      int lastChildInt = nbOfChildren - 1;
      const unsigned char* lastChildPtr = pPtr + (lastChildInt << 2); // * 4
      keysAfterPrefix += *(lastChildPtr + 3);
      pPtr = pBeginMemoryPtr + binaryloader::alignedDecToInt(*reinterpret_cast<const int*>(lastChildPtr));
      continue;
    }
    int nbOfChildrenInt = nbOfChildren;
    const unsigned char* endOfWordPtr = pPtr + (nbOfChildrenInt << 2); // * 4
    if (pIsEndOfAWord(endOfWordPtr))
      return {pKeyPrefix + keysAfterPrefix, endOfWordPtr};
    break;
  }
  return {"", nullptr};
}


static inline const unsigned char* _read_value
(const unsigned char* pPtr,
 const std::function<bool(const unsigned char*)>& pIsEndOfAWord)
{
  std::size_t nbOfKeys = _readNbOfKeys(pPtr);
  pPtr += nbOfKeys;
  unsigned char nbOfChildren = *(pPtr++);
  int nbOfChildrenInt = nbOfChildren;
  const unsigned char* endOfWordPtr = pPtr + (nbOfChildrenInt << 2); // * 4
  if (pIsEndOfAWord(endOfWordPtr))
    return endOfWordPtr;
  return nullptr;
}


static inline const unsigned char* _getChildIdOfARadixNodePtr(const unsigned char* pPtr,
                                                              unsigned char pChildId)
{
  std::size_t nbOfKeys = _readNbOfKeys(pPtr);
  pPtr += nbOfKeys;
  unsigned char nbOfChildren = *(pPtr++);
  if (pChildId < nbOfChildren)
  {
    int pChildIdInt = pChildId;
    return pPtr + (pChildIdInt << 2); // * 4
  }
  return nullptr;
}


static inline char _getFirstLetterOfARadixNodeChildren(const unsigned char* pChildrenPtr)
{
  pChildrenPtr += 3;
  return *pChildrenPtr;
}



static inline StaticRadixMapIterator _advance
(const unsigned char* pPtr,
 const std::string& pKey,
 const std::function<bool(const unsigned char*)>& pIsEndOfAWord,
 std::size_t pBeginPos,
 std::size_t pEndPos,
 SearchEndingPoint pSearchEndingPoint)
{
  pPtr = binaryloader::alignMemory(pPtr);
  const unsigned char* beginMemoryPtr = pPtr;

  std::stack<std::pair<const unsigned char*, unsigned char>> parentNodes;
  std::stack<std::string> nextKeysStack;
  const std::string rootKeys = _getKeysStrFromRadixNodePtr(pPtr);
  if (!rootKeys.empty())
    nextKeysStack.emplace(rootKeys);
  auto getKeysPrefix = [&]
  {
    std::string res;
    while (!nextKeysStack.empty())
    {
      res = nextKeysStack.top() + res;
      nextKeysStack.pop();
    }
    return res;
  };

  while (true)
  {
    bool continueLoop = false;
    const unsigned char* beginOfNodePtr = pPtr;
    std::size_t nbOfKeys = _readNbOfKeys(pPtr);
    std::size_t keySize = pEndPos - pBeginPos;
    if (nbOfKeys <= keySize &&
        (nbOfKeys == 0 || memcmp(pPtr, pKey.data() + pBeginPos, nbOfKeys) == 0))
    {
      pPtr += nbOfKeys;
      unsigned char nbOfChildren = *(pPtr++);

      if (nbOfKeys == keySize)
      {
        switch (pSearchEndingPoint)
        {
        case SearchEndingPoint::PREV:
        {
          while (!parentNodes.empty())
          {
            nextKeysStack.pop();
            auto& parentNode = parentNodes.top();
            if (parentNode.second > 0)
            {
              --parentNode.second;
              auto* prevChildPtr = _getChildIdOfARadixNodePtr(parentNode.first, parentNode.second);
              if (prevChildPtr != nullptr)
              {
                std::string prevKeys = getKeysPrefix() + _getFirstLetterOfARadixNodeChildren(prevChildPtr);
                const int* offsetOfTheChild = reinterpret_cast<const int*>(prevChildPtr);
                return _read_end(beginMemoryPtr + binaryloader::alignedDecToInt(*offsetOfTheChild),
                                 beginMemoryPtr, prevKeys, pIsEndOfAWord);
              }
            }
            auto* valuePtr = _read_value(parentNode.first, pIsEndOfAWord);
            if (valuePtr != nullptr)
              return {getKeysPrefix(), valuePtr};
            parentNodes.pop();
          }
          return {"", nullptr};
        }
        case SearchEndingPoint::EQUAL_OR_NEXT:
        {
          int nbOfChildrenInt = nbOfChildren;
          return {pKey, pPtr + (nbOfChildrenInt << 2)}; // * 4
          break;
        }
        case SearchEndingPoint::NEXT:
          break;
        };
      }

      // so nbOfKeys < keySize
      std::size_t childKeyIndex = pBeginPos + nbOfKeys;
      char charToFind = pKey[childKeyIndex];

      for (unsigned char i = 0u; i < nbOfChildren; ++i)
      {
        const int* offsetOfTheChild = reinterpret_cast<const int*>(pPtr);
        pPtr += 3;
        char firstletter = *(pPtr++);
        if (charToFind == firstletter)
        {
          pPtr = beginMemoryPtr + binaryloader::alignedDecToInt(*offsetOfTheChild);
          pBeginPos = childKeyIndex + 1;
          parentNodes.emplace(std::make_pair(beginOfNodePtr, i));
          nextKeysStack.emplace(firstletter + _getKeysStrFromRadixNodePtr(pPtr));
          continueLoop = true;
          break;
        }
        if (charToFind < firstletter) // in this case firstletter is the upper_bound
        {
          return _read_begin(beginMemoryPtr + binaryloader::alignedDecToInt(*offsetOfTheChild),
                             beginMemoryPtr, getKeysPrefix() + firstletter, pIsEndOfAWord);
        }
      }
      if (continueLoop)
        continue;

      while (!parentNodes.empty())
      {
        nextKeysStack.pop();
        auto& parentNode = parentNodes.top();
        ++parentNode.second;
        auto* nextChildPtr = _getChildIdOfARadixNodePtr(parentNode.first, parentNode.second);
        if (nextChildPtr != nullptr)
        {
          std::string nextKeys = getKeysPrefix() + _getFirstLetterOfARadixNodeChildren(nextChildPtr);
          const int* offsetOfTheChild = reinterpret_cast<const int*>(nextChildPtr);
          return _read_begin(beginMemoryPtr + binaryloader::alignedDecToInt(*offsetOfTheChild),
                             beginMemoryPtr, nextKeys, pIsEndOfAWord);
        }
        parentNodes.pop();
      }
    }
    else if (keySize == 0 &&
             pSearchEndingPoint == SearchEndingPoint::EQUAL_OR_NEXT)
    {
      auto* valuePtr = _read_value(beginOfNodePtr, pIsEndOfAWord);
      if (valuePtr != nullptr)
      {
        return {getKeysPrefix(), valuePtr};
      }
      else
      {
        pPtr += nbOfKeys;
        unsigned char nbOfChildren = *(pPtr++);
        if (nbOfChildren > 0)
        {
          const int* offsetOfTheFirstChild = reinterpret_cast<const int*>(pPtr);
          pPtr += 3;
          char firstletter = *(pPtr++);
          return _read_begin(beginMemoryPtr + binaryloader::alignedDecToInt(*offsetOfTheFirstChild),
                             beginMemoryPtr, getKeysPrefix() + firstletter, pIsEndOfAWord);
        }
      }
    }
    break;
  }
  return {"", nullptr};
}



static inline StaticRadixMapIterator read_lower_bound
(const unsigned char* pPtr,
 const std::string& pKey,
 const std::function<bool(const unsigned char*)>& pIsEndOfAWord)
{
  return _advance(pPtr, pKey, pIsEndOfAWord, 0, pKey.size(), SearchEndingPoint::EQUAL_OR_NEXT);
}

static inline StaticRadixMapIterator read_upper_bound
(const unsigned char* pPtr,
 const std::string& pKey,
 const std::function<bool(const unsigned char*)>& pIsEndOfAWord)
{
  return _advance(pPtr, pKey, pIsEndOfAWord, 0, pKey.size(), SearchEndingPoint::NEXT);
}

static inline StaticRadixMapIterator read_previous
(const unsigned char* pPtr,
 const std::string& pKey,
 const std::function<bool(const unsigned char*)>& pIsEndOfAWord)
{
  return _advance(pPtr, pKey, pIsEndOfAWord, 0, pKey.size(), SearchEndingPoint::PREV);
}


static inline const unsigned char* read
(const unsigned char* pPtr,
 const std::string& pKey,
 const std::function<bool(const unsigned char*)>& pIsEndOfAWord)
{
  return read(pPtr, pKey, 0, pKey.size(), pIsEndOfAWord);
}




} // End of namespace radixmap
} // End of namespace onsem


#endif // ONSEM_COMMON_BINARY_RADIXMAPREADER_HPP
