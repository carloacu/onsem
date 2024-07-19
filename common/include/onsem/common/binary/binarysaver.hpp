#ifndef ONSEM_COMMON_SAVER_BINARYSAVER_HPP
#define ONSEM_COMMON_SAVER_BINARYSAVER_HPP

#include <cstddef>
#include <assert.h>
#include <ostream>
#include <cstring>
#include <cstdint>
#include "binarymasks.hpp"

namespace onsem {
namespace binarysaver {

static inline signed char writeBool(signed char* pPtr, bool val) {
    return *pPtr = val;
}

/**
 * @brief Write a char into the memory block that will be saved in the binary file.
 * @param pPtr The pointer where we will write our char.
 * @param val The char to write.
 * @return The char to write.
 */
static inline char writeChar(signed char* pPtr, unsigned char val) {
    return *pPtr = val;
}

static inline char writeChar_0To1(signed char* pPtr, unsigned char val) {
    assert(val < 4);    // 2^2
    return *pPtr = (*pPtr & binarymasks::mask2To7) + val;
}

static inline char writeChar_0To2(signed char* pPtr, unsigned char val) {
    assert(val < 8);    // 2^3
    return *pPtr = (*pPtr & binarymasks::mask3To7) + val;
}

static inline char writeChar_0To3(signed char* pPtr, unsigned char val) {
    assert(val < 16);    // 2^4
    return *pPtr = (*pPtr & binarymasks::mask4To7) + val;
}

static inline char writeChar_0To4(signed char* pPtr, unsigned char val) {
    assert(val < 32);    // 2^5
    return *pPtr = (*pPtr & binarymasks::mask5To7) + val;
}

static inline char writeChar_0To5(signed char* pPtr, unsigned char val) {
    assert(val < 64);    // 2^6
    return *pPtr = (*pPtr & binarymasks::mask6To7) + val;
}

static inline char writeChar_0To6(signed char* pPtr, unsigned char val) {
    assert(val < 128);    // 2^7
    return *pPtr = (*pPtr & binarymasks::mask7) + val;
}

static inline char writeChar_1To2(signed char* pPtr, unsigned char val) {
    assert(val < 8);    // 2^3
    val = val << 1;
    return *pPtr = (*pPtr & binarymasks::mask0) + val;
}

static inline char writeChar_1To7(signed char* pPtr, unsigned char val) {
    assert(val < 128);    // 2^7
    val = val << 1;
    return *pPtr = (*pPtr & binarymasks::mask0) + val;
}

static inline char writeChar_2To3(signed char* pPtr, unsigned char val) {
    assert(val < 4);    // 2^2
    val = val << 2;
    return *pPtr = (*pPtr & binarymasks::mask0To1) + val;
}

static inline char writeChar_2To7(signed char* pPtr, unsigned char val) {
    assert(val < 64);    // 2^6
    val = val << 2;
    return *pPtr = (*pPtr & binarymasks::mask0To1) + val;
}

static inline char writeChar_4To5(signed char* pPtr, unsigned char val) {
    assert(val < 4);    // 2^2
    val = val << 4;
    return *pPtr = (*pPtr & binarymasks::mask0To3) + val;
}

static inline char writeChar_4To7(signed char* pPtr, unsigned char val) {
    assert(val < 16);    // 2^4
    val = val << 4;
    return *pPtr = (*pPtr & binarymasks::mask0To3) + val;
}

static inline char writeChar_6To7(signed char* pPtr, unsigned char val) {
    assert(val < 4);    // 2^2
    val = val << 6;
    return *pPtr = (*pPtr & binarymasks::mask0To5) + val;
}

static inline char writeChar_0(signed char* pPtr, bool val) {
    *pPtr = *pPtr & binarymasks::mask1To7;
    if (val)
        return *pPtr += binarymasks::mask0;
    return *pPtr;
}

static inline char writeChar_1(signed char* pPtr, bool val) {
    *pPtr = *pPtr & binarymasks::maskNot1;
    if (val)
        return *pPtr += binarymasks::mask1;
    return *pPtr;
}

static inline char writeChar_2(signed char* pPtr, bool val) {
    *pPtr = *pPtr & binarymasks::maskNot2;
    if (val)
        return *pPtr += binarymasks::mask2;
    return *pPtr;
}

static inline char writeChar_3(signed char* pPtr, bool val) {
    *pPtr = *pPtr & binarymasks::maskNot3;
    if (val)
        return *pPtr += binarymasks::mask3;
    return *pPtr;
}

static inline char writeChar_4(signed char* pPtr, bool val) {
    *pPtr = *pPtr & binarymasks::maskNot4;
    if (val)
        return *pPtr += binarymasks::mask4;
    return *pPtr;
}

static inline char writeChar_5(signed char* pPtr, bool val) {
    *pPtr = *pPtr & binarymasks::maskNot5;
    if (val)
        return *pPtr += binarymasks::mask5;
    return *pPtr;
}

static inline char writeChar_6(signed char* pPtr, bool val) {
    *pPtr = *pPtr & binarymasks::maskNot6;
    if (val)
        return *pPtr += binarymasks::mask6;
    return *pPtr;
}

static inline char writeChar_7(signed char* pPtr, bool val) {
    *pPtr = *pPtr & binarymasks::mask0To6;
    if (val)
        return *pPtr += binarymasks::mask7;
    return *pPtr;
}

static inline unsigned char sizet_to_uchar(std::size_t pNb) {
    assert(pNb < 256);
    return static_cast<char>(pNb);
}

static inline bool intCanBeStoredInAChar(int pVal) {
    return pVal > -128 && pVal < 128;
}

static inline int alignedDecToSave(std::size_t pDec) {
    int intDec = static_cast<int>(pDec);
    assert(intDec < 67108864);    // 2^(8*3) * 4
    assert(intDec % 4 == 0);
    intDec = intDec / 4;
    assert((intDec & 0xFF000000) == 0);
    return intDec;
}

static inline void writeInThreeBytes(signed char* pPtr, uint32_t pVal) {
    assert(pVal < 16777216);
    int* intPtr = reinterpret_cast<int*>(pPtr);
    *intPtr = (*intPtr & 0xFF000000) + pVal;
}

static inline void writeInt(signed char* pPtr, int pVal) {
    int* intPtr = reinterpret_cast<int*>(pPtr);
    *intPtr = pVal;
}

static inline void writeInt(int* pIntPtr, int pVal) {
    *pIntPtr = pVal;
}

static inline void writeInt64(uint64_t* pIntPtr, uint64_t pVal) {
    *pIntPtr = pVal;
}

static inline void writePtr(std::ostream& pOutfile, std::size_t pOffset) {
    int printOffset = static_cast<int>(pOffset);
    pOutfile.write(reinterpret_cast<const char*>(&printOffset), sizeof(printOffset));
}

static inline void writeString(binarymasks::Ptr& pEndMemory, const std::string& pStr) {
    auto strLength = pStr.length();
    bool lengthCanBeWritenIn7Bits = strLength < 64;
    writeChar_0(pEndMemory.pchar, lengthCanBeWritenIn7Bits);
    if (lengthCanBeWritenIn7Bits) {
        writeChar_1To7(pEndMemory.pchar, static_cast<char>(strLength));
        ++pEndMemory.pchar;
    } else {
        ++pEndMemory.pchar;
        writeInt(pEndMemory.pint++, static_cast<unsigned int>(strLength));
    }
    memcpy(pEndMemory.pchar, pStr.data(), strLength);
    pEndMemory.pchar += strLength;
}

/**
 * @brief Align a pointer in the memory.
 * @param pEndMemory The pointer to align.
 * @return A new pointer aligned in memory.
 */
static inline binarymasks::Ptr alignMemory(binarymasks::Ptr pEndMemory) {
    while (pEndMemory.val % 4 != 0)
        writeChar(pEndMemory.pchar++, 0);
    return pEndMemory;
}

}    // End of namespace binarysaver
}    // End of namespace onsem

#endif    // ONSEM_COMMON_SAVER_BINARYSAVER_HPP
