#ifndef ONSEM_COMMON_BINARY_RADIXMAPREADERWITHKEYRETRIEVING_HPP
#define ONSEM_COMMON_BINARY_RADIXMAPREADERWITHKEYRETRIEVING_HPP

#include "binaryloader.hpp"
#include <onsem/common/utility/radix_map.hpp>

namespace onsem {
namespace radixmap {

const unsigned char* readWithKeyRetrieving(const unsigned char* pPtr,
                                           const unsigned char* pBeginMemoryPtr,
                                           const std::string& pKey,
                                           std::size_t pBeginPos,
                                           std::size_t pEndPos,
                                           const std::function<bool(const unsigned char*)>& pIsEndOfAWord) {
    pPtr += 3;
    unsigned char nbOfKeys = *(pPtr++);
    std::size_t keySize = pEndPos - pBeginPos;
    if (keySize < nbOfKeys || memcmp(pPtr, pKey.data() + pBeginPos, nbOfKeys) != 0)
        return nullptr;
    pPtr += nbOfKeys;
    unsigned char nbOfChildren = *(pPtr++);

    if (keySize == nbOfKeys) {
        int nbOfChildrenInt = nbOfChildren;
        pPtr = pPtr + (nbOfChildrenInt << 2);    // * 4
        return pIsEndOfAWord(pPtr) ? pPtr : nullptr;
    }

    std::size_t childKeyIndex = pBeginPos + nbOfKeys;
    char charToFind = pKey[childKeyIndex];

    for (unsigned char i = 0u; i < nbOfChildren; ++i) {
        const int* offsetOfTheCild = reinterpret_cast<const int*>(pPtr);
        pPtr += 3;
        char firstletter = *(pPtr++);
        if (charToFind == firstletter)
            return readWithKeyRetrieving(pBeginMemoryPtr + binaryloader::alignedDecToInt(*offsetOfTheCild),
                                         pBeginMemoryPtr,
                                         pKey,
                                         childKeyIndex + 1,
                                         pEndPos,
                                         pIsEndOfAWord);
        if (charToFind < firstletter)
            return nullptr;
    }
    return nullptr;
}

const unsigned char* readWithKeyRetrieving(const unsigned char* pBeginMemoryPtr,
                                           const std::string& pKey,
                                           const std::function<bool(const unsigned char*)>& pIsEndOfAWord) {
    return readWithKeyRetrieving(pBeginMemoryPtr, pBeginMemoryPtr, pKey, 0, pKey.size(), pIsEndOfAWord);
}

std::string readKey(const unsigned char* pPtr, int pNodeOffset, int pChildNodeOffset = 0) {
    const unsigned char* ptr = pPtr + pNodeOffset;
    const unsigned char* beginNodePtr = ptr;
    ptr += 3;
    unsigned char nbOfKeys = *(ptr++);
    std::string res(nbOfKeys, ' ');
    for (unsigned char i = 0u; i < nbOfKeys; ++i)
        res[i] = *(ptr++);
    if (pChildNodeOffset > 0) {
        ++ptr;    // number of children
        unsigned char i = 0u;
        while (true) {
            const int* offsetOfTheChild = reinterpret_cast<const int*>(ptr);
            ptr += 3;
            if (pChildNodeOffset == binaryloader::alignedDecToInt(*offsetOfTheChild)) {
                res += *(ptr++);
                break;
            }
            ++ptr;
            ++i;
        }
    }
    if (pNodeOffset == 0)
        return res;
    int parentOffset = binaryloader::alignedDecToInt(*reinterpret_cast<const int*>(beginNodePtr));
    return readKey(pPtr, parentOffset, pNodeOffset) + res;
}

}    // End of namespace radixmap
}    // End of namespace onsem

#endif    // ONSEM_COMMON_BINARY_RADIXMAPREADERWITHKEYRETRIEVING_HPP
