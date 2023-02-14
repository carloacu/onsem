#ifndef ONSEM_TEXTTOSEMANTIC_DETAIL_METAWORDTREEDB_HXX
#define ONSEM_TEXTTOSEMANTIC_DETAIL_METAWORDTREEDB_HXX

#include "metawordtreedb.hpp"
#include <onsem/common/binary/binaryloader.hpp>

namespace onsem
{

inline bool MetaWordTreeDb::xIsLoaded() const
{
  return _ptrPatriciaTrie != nullptr || _loadedWithoutStream;
}

inline bool MetaWordTreeDb::xIfEndOfAWord
(const signed char* pNode,
 bool) const
{
  return xNbMeanings(pNode) > 0;
}


inline unsigned char MetaWordTreeDb::xNbLetters
(const signed char* pNode) const
{
  return pNode[3];
}


inline signed char MetaWordTreeDb::xGetLetter
(const signed char* pNode, unsigned char i) const
{
  return pNode[3 + 1 + i];
}


inline unsigned char MetaWordTreeDb::xNbChildren
(const signed char* pNode) const
{
  return pNode[3 + 2 + xNbLetters(pNode)];
}

inline const signed char* MetaWordTreeDb::xGetNode
(const std::string& pWord,
 std::size_t pBeginPos,
 std::size_t pSizeOfWord,
 bool pOnlyWordWithWordFroms) const
{
  std::size_t maxLen;
  return xSearchInPatriciaTrie(maxLen, pWord, pBeginPos, pSizeOfWord,
                               pOnlyWordWithWordFroms,
                               SearchForLongestWordMode::DISABLED);
}



inline unsigned char MetaWordTreeDb::xNbMeanings
(const signed char* pNode) const
{
  return pNode[3 + 1 + xNbLetters(pNode)];
}



inline const int* MetaWordTreeDb::xGetFather
(const signed char* pNode) const
{
  return reinterpret_cast<const int*>(pNode);
}


inline const signed char* MetaWordTreeDb::xAlignedDecToPtr(signed char* pBeginMem,
 int pAlignedDec) const
{
  return pBeginMem + binaryloader::alignedDecToInt(pAlignedDec);
}


inline signed char MetaWordTreeDb::xGetCharAfterAlignedDec(int pAlignedDec) const
{
  return reinterpret_cast<const signed char*>(&pAlignedDec)[3];
}


inline const int32_t* MetaWordTreeDb::xGetFirstChild
(const signed char* pNode) const
{
  return reinterpret_cast<const int*>
      (binaryloader::alignMemory(pNode + 4 + xNbLetters(pNode) + 2));
}


inline const int* MetaWordTreeDb::xGetBeginOfEndingStruct
(const signed char* pNode) const
{
  return xGetFirstChild(pNode) + xNbChildren(pNode);
}



} // End of namespace onsem

#endif // ONSEM_TEXTTOSEMANTIC_DETAIL_METAWORDTREEDB_HXX
