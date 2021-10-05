#ifndef ONSEM_COMMON_BINARY_ENUMMAPSAVER_HPP
#define ONSEM_COMMON_BINARY_ENUMMAPSAVER_HPP

#include <map>
#include <vector>
#include <functional>
#include <onsem/common/binary/binarysaver.hpp>

namespace onsem
{

template<typename ENUM_TYPE, typename VALUE_TYPE>
void writeEnumMapFull
(binarymasks::Ptr& pPtr,
 const std::map<ENUM_TYPE, VALUE_TYPE>& pMap,
 const std::vector<ENUM_TYPE>& pAllEnumValues,
 const std::function<void(binarymasks::Ptr&, const VALUE_TYPE*)>& pPrintValue)
{
  const binarymasks::Ptr beginPtr = pPtr;
  binarysaver::writeChar(pPtr.pchar++, binarymasks::mask0To7);
  int* endOffsetPtr = pPtr.pint;
  int* offsetsPtr = pPtr.pint;
  ++offsetsPtr;
  pPtr.pint += binarysaver::sizet_to_uchar(pAllEnumValues.size()); // nb of enum values
  bool firstIteration = true;
  for (const auto& currEnumVal : pAllEnumValues)
  {
    if (firstIteration)
      firstIteration = false;
    else
      binarysaver::writeInt(offsetsPtr++, pPtr.pchar - beginPtr.pchar);
    auto itMap = pMap.find(currEnumVal);
    if (itMap != pMap.end())
      pPrintValue(pPtr, &itMap->second);
    else
      pPrintValue(pPtr, nullptr);
  }
  binarysaver::writeInt(endOffsetPtr, pPtr.pchar - beginPtr.pchar);
}


template<typename ENUM_TYPE, typename VALUE_TYPE>
void writeEnumMap
(binarymasks::Ptr& pPtr,
 const std::map<ENUM_TYPE, VALUE_TYPE>& pMap,
 const std::vector<ENUM_TYPE>& pAllEnumValues,
 const std::function<void(binarymasks::Ptr&, const VALUE_TYPE*)>& pPrintValue)
{
  auto map_size = pMap.size();
  if (map_size * 2 > pAllEnumValues.size())
  {
    writeEnumMapFull(pPtr, pMap, pAllEnumValues, pPrintValue);
    return;
  }
  const binarymasks::Ptr beginPtr = pPtr;
  unsigned char nbOfElts = binarysaver::sizet_to_uchar(map_size);
  binarysaver::writeChar(pPtr.pchar++, nbOfElts);
  int* endOffsetPtr = pPtr.pint;
  if (nbOfElts == 0)
  {
    ++pPtr.pint;
    binarysaver::writeInt(endOffsetPtr, pPtr.pchar - beginPtr.pchar);
    return;
  }
  signed char* offsetsPtr = pPtr.pchar;
  offsetsPtr += 4;
  pPtr.pint += nbOfElts; // offsets
  pPtr.pchar += nbOfElts; // enum values
  bool firstIteration = true;
  for (const auto& currElt : pMap)
  {
    // write offsets
    if (firstIteration)
    {
      firstIteration = false;
    }
    else
    {
      binarysaver::writeInt(offsetsPtr, pPtr.pchar - beginPtr.pchar);
      offsetsPtr += 4;
    }
    // write enum values
    binarysaver::writeChar(offsetsPtr++, static_cast<char>(currElt.first));
    // write values
    pPrintValue(pPtr, &currElt.second);
  }
  binarysaver::writeInt(endOffsetPtr, pPtr.pchar - beginPtr.pchar);
}



template<typename VALUE_TYPE>
void writeIntMap
(binarymasks::Ptr& pPtr,
 const std::map<int, VALUE_TYPE>& pMap,
 const std::function<void(binarymasks::Ptr&, const VALUE_TYPE&)>& pPrintValue)
{
  const binarymasks::Ptr beginPtr = pPtr;
  int32_t map_size = pMap.size();
  binarysaver::writeInt(pPtr.pint++, map_size);
  int* endOffsetPtr = pPtr.pint;
  if (map_size == 0)
  {
    ++pPtr.pint;
    binarysaver::writeInt(endOffsetPtr, pPtr.pchar - beginPtr.pchar);
    return;
  }
  signed char* offsetsPtr = pPtr.pchar;
  offsetsPtr += 4;
  pPtr.pint += map_size * 2; // offsets + int key values
  bool firstIteration = true;
  for (const auto& currElt : pMap)
  {
    // write offsets
    if (firstIteration)
    {
      firstIteration = false;
    }
    else
    {
      binarysaver::writeInt(offsetsPtr, pPtr.pchar - beginPtr.pchar);
      offsetsPtr += 4;
    }
    // write enum values
    binarysaver::writeInt(offsetsPtr, currElt.first);
    offsetsPtr += 4;
    // write values
    pPrintValue(pPtr, currElt.second);
  }
  binarysaver::writeInt(endOffsetPtr, pPtr.pchar - beginPtr.pchar);
}

} // End of namespace onsem


#endif // ONSEM_COMMON_BINARY_ENUMMAPSAVER_HPP
