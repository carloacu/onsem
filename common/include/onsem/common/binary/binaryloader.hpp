#ifndef ONSEM_COMMON_SAVER_BINARYLOADER_HPP
#define ONSEM_COMMON_SAVER_BINARYLOADER_HPP

#include <fstream>
#include "binarymasks.hpp"

namespace onsem
{
namespace binaryloader
{


static inline bool allocMemZone(signed char** pMem,
                                std::istream& pBinDatabaseFile,
                                std::size_t pSize)
{
  try
  {
    *pMem = new signed char[pSize];
  }
  catch (const std::bad_alloc&)
  {
    *pMem = nullptr;
    return false;
  }
  if (*pMem == nullptr)
  {
    return false;
  }
  pBinDatabaseFile.read(reinterpret_cast<char*>(*pMem), pSize);
  return true;
}


static inline bool allocMemZoneU(unsigned char** pMem,
                                 std::istream& pBinDatabaseFile,
                                 std::size_t pSize)
{
  try
  {
    *pMem = new unsigned char[pSize];
  }
  catch (const std::bad_alloc&)
  {
    *pMem = nullptr;
    return false;
  }
  if (*pMem == nullptr)
  {
    return false;
  }
  pBinDatabaseFile.read(reinterpret_cast<char*>(*pMem), pSize);
  return true;
}


static inline void deallocMemZone(signed char** pMem)
{
  if (*pMem != nullptr)
  {
    delete[](*pMem);
    *pMem = nullptr;
  }
}


static inline void deallocMemZoneU(unsigned char** pMem)
{
  if (*pMem != nullptr)
  {
    delete[](*pMem);
    *pMem = nullptr;
  }
}


/**
 * @brief Advance the given pointer in order to have one aligned in memory.
 * @param pPtr The given pointer.
 * @return The new pointer aligned in memory.
 */
static inline const signed char* alignMemory(const signed char* pPtr)
{
  std::size_t moduloFour = reinterpret_cast<std::size_t>(pPtr) % 4;
  return moduloFour == 0 ? pPtr : pPtr + 4 - moduloFour;
}

static inline const unsigned char* alignMemory(const unsigned char* pPtr)
{
  std::size_t moduloFour = reinterpret_cast<std::size_t>(pPtr) % 4;
  return moduloFour == 0 ? pPtr : pPtr + 4 - moduloFour;
}


static inline int32_t loadInt(const signed char* pPtr)
{
  return *reinterpret_cast<const int32_t*>(pPtr);
}

static inline uint32_t loadInt(const unsigned char* pPtr)
{
  return *reinterpret_cast<const uint32_t*>(pPtr);
}


static inline int32_t loadIntInThreeBytes(const unsigned char* pPtr)
{
  return *reinterpret_cast<const int32_t*>(pPtr) & 0x00FFFFFF;
}

static inline int32_t loadIntInThreeBytes(int32_t pPtr)
{
  return pPtr & 0x00FFFFFF;
}

static inline int32_t alignedDecToInt(int32_t pAlignedDec)
{
  return (pAlignedDec & 0x00FFFFFF) * 4;
}


static inline signed char const* getStrWithOffset(
    std::string& pOutStr,
    signed char const* pInputMem)
{
  pOutStr.clear();
  pOutStr.reserve(pInputMem[0]);
  for (char i = 0; i < pInputMem[0]; ++i)
    pOutStr += pInputMem[i + 1];
  return pInputMem + pInputMem[0] + 1;
}


static inline bool loadChar_0(const unsigned char* pPtr)
{
  return *pPtr & binarymasks::mask0;
}

static inline bool loadChar_1(const unsigned char* pPtr)
{
  return (*pPtr & binarymasks::mask1) >> 1;
}

static inline bool loadChar_2(const unsigned char* pPtr)
{
  return (*pPtr & binarymasks::mask2) >> 2;
}

static inline bool loadChar_3(const unsigned char* pPtr)
{
  return (*pPtr & binarymasks::mask3) >> 3;
}

static inline bool loadChar_4(const unsigned char* pPtr)
{
  return (*pPtr & binarymasks::mask4) >> 4;
}

static inline bool loadChar_5(const unsigned char* pPtr)
{
  return (*pPtr & binarymasks::mask5) >> 5;
}

static inline bool loadChar_6(const unsigned char* pPtr)
{
  return (*pPtr & binarymasks::mask6) >> 6;
}

static inline bool loadChar_7(const unsigned char* pPtr)
{
  return (*pPtr & binarymasks::mask7) >> 7;
}

static inline char loadChar_0To1(const unsigned char* pPtr)
{
  return *pPtr & binarymasks::mask0To1;
}

static inline char loadChar_0To2(const unsigned char* pPtr)
{
  return *pPtr & binarymasks::mask0To2;
}

static inline char loadChar_0To3(const unsigned char* pPtr)
{
  return *pPtr & binarymasks::mask0To3;
}

static inline char loadChar_0To4(const unsigned char* pPtr)
{
  return *pPtr & binarymasks::mask0To4;
}

static inline char loadChar_0To5(const unsigned char* pPtr)
{
  return *pPtr & binarymasks::mask0To5;
}

static inline char loadChar_0To6(const unsigned char* pPtr)
{
  return *pPtr & binarymasks::mask0To6;
}

static inline char loadChar_1To2(const unsigned char* pPtr)
{
  return (*pPtr & binarymasks::mask1To2) >> 1;
}

static inline char loadChar_1To7(const unsigned char* pPtr)
{
  return (*pPtr & binarymasks::mask1To7) >> 1;
}

static inline char loadChar_2To3(const unsigned char* pPtr)
{
  return (*pPtr & binarymasks::mask2To3) >> 2;
}

static inline char loadChar_2To7(const unsigned char* pPtr)
{
  return (*pPtr & binarymasks::mask2To7) >> 2;
}

static inline char loadChar_4To5(const unsigned char* pPtr)
{
  return (*pPtr & binarymasks::mask4To5) >> 4;
}

static inline char loadChar_4To7(const unsigned char* pPtr)
{
  return (*pPtr & binarymasks::mask4To7) >> 4;
}

static inline char loadChar_6To7(const unsigned char* pPtr)
{
  return (*pPtr & binarymasks::mask6To7) >> 6;
}


static inline std::string loadString(const unsigned char*& pPtr)
{
  bool lengthIsWritenIn7Bits = loadChar_0(pPtr);
  uint32_t length = 0;
  if (lengthIsWritenIn7Bits)
  {
    length = loadChar_1To7(pPtr);
    ++pPtr;
  }
  else
  {
    ++pPtr;
    length = loadInt(pPtr);
    pPtr += 4;
  }
  const char* beginOfStr = reinterpret_cast<const char*>(pPtr);
  pPtr += length;
  return {beginOfStr, length};
}

static inline void skipString(const unsigned char*& pPtr)
{
  bool lengthIsWritenIn7Bits = loadChar_0(pPtr);
  uint32_t length = 0;
  if (lengthIsWritenIn7Bits)
  {
    length = loadChar_1To7(pPtr);
    ++pPtr;
  }
  else
  {
    ++pPtr;
    length = loadInt(pPtr);
    pPtr += 4;
  }
  pPtr += length;
}

} // End of namespace binaryloader
} // End of namespace onsem


#endif // ONSEM_COMMON_SAVER_BINARYLOADER_HPP
