#ifndef ONSEM_COMMON_BINARY_ENUMMAPREADER_HPP
#define ONSEM_COMMON_BINARY_ENUMMAPREADER_HPP

#include <functional>
#include <onsem/common/binary/binaryloader.hpp>

namespace onsem {

inline static const unsigned char* readEnumMapFull(const unsigned char* pPtr,
                                                   const unsigned char* pBeginPtr,
                                                   unsigned char pKey,
                                                   std::size_t pNbOfEnumValues) {
    if (pKey == 0)
        return pPtr + (pNbOfEnumValues * 4);
    return pBeginPtr + binaryloader::loadInt(pPtr + (pKey * 4));
}

inline static const unsigned char* readEnumMap(const unsigned char* pPtr,
                                               unsigned char pKey,
                                               std::size_t pNbOfEnumValues) {
    const unsigned char* beginPtr = pPtr;
    unsigned char nbOfElts = *(pPtr++);
    if (nbOfElts == binarymasks::mask0To7)
        return readEnumMapFull(pPtr, beginPtr, pKey, pNbOfEnumValues);
    auto size = nbOfElts;
    const unsigned char* firstKeyPtr = pPtr;

    while (true) {
        if (size < 5) {
            for (std::size_t i = 0; i < size; ++i) {
                const unsigned char* newPtr = pPtr;
                pPtr += 4;
                char val = *(pPtr++);
                if (val == pKey) {
                    if (newPtr == firstKeyPtr)
                        return firstKeyPtr + (nbOfElts * 5);
                    return beginPtr + binaryloader::loadInt(newPtr);
                }
                if (val > pKey)
                    return nullptr;
            }
            return nullptr;
        }

        std::size_t newSize = size >> 1;    // -> size / 2;
        const unsigned char* newPtr = pPtr + (newSize * 5);
        char val = *(newPtr + 4);
        if (val == pKey)
            return beginPtr + binaryloader::loadInt(newPtr);

        if (val > pKey) {
            size = newSize;
            continue;
        }

        pPtr = newPtr + 5;
        size = size - newSize - 1;
    }
    return nullptr;
}

inline static void _readAllEnumMapFullValues(
    const unsigned char* pPtr,
    const unsigned char* pBeginPtr,
    std::size_t pNbOfEnumValues,
    const std::function<void(unsigned char, const unsigned char*)>& pOnEachElements) {
    if (pNbOfEnumValues > 0)
        pOnEachElements(0, pPtr + (pNbOfEnumValues * 4));
    for (unsigned char key = 1; key < pNbOfEnumValues; ++key)
        pOnEachElements(key, pBeginPtr + binaryloader::loadInt(pPtr + (key * 4)));
}

inline static void readAllEnumMapValues(
    const unsigned char* pPtr,
    std::size_t pNbOfEnumValues,
    const std::function<void(unsigned char, const unsigned char*)>& pOnEachElements) {
    const unsigned char* beginPtr = pPtr;
    unsigned char nbOfElts = *(pPtr++);
    if (nbOfElts == binarymasks::mask0To7) {
        _readAllEnumMapFullValues(pPtr, beginPtr, pNbOfEnumValues, pOnEachElements);
        return;
    }
    const unsigned char* firstKeyPtr = pPtr;
    for (unsigned char i = 0; i < nbOfElts; ++i) {
        const unsigned char* newPtr = pPtr;
        pPtr += 4;
        char key = *(pPtr++);
        if (i == 0)
            pOnEachElements(key, firstKeyPtr + (nbOfElts * 5));
        else
            pOnEachElements(key, beginPtr + binaryloader::loadInt(newPtr));
    }
}

inline static const unsigned char* jumpAfterEnumMap(const unsigned char* pPtr) {
    return pPtr + binaryloader::loadInt(pPtr + 1);
}

inline static const unsigned char* readIntMap(const unsigned char* pPtr, int pKey) {
    auto* beginPtr = pPtr;
    uint32_t nbOfElts = binaryloader::loadInt(pPtr);
    pPtr += 4;
    auto size = nbOfElts;
    auto* firstKeyPtr = pPtr;

    while (true) {
        if (size < 5) {
            for (std::size_t i = 0; i < size; ++i) {
                auto* newPtr = pPtr;
                pPtr += 4;
                int32_t val = binaryloader::loadInt(pPtr);
                pPtr += 4;
                if (val == pKey) {
                    if (newPtr == firstKeyPtr)
                        return firstKeyPtr + (nbOfElts * 8);
                    return beginPtr + binaryloader::loadInt(newPtr);
                }
                if (val > pKey)
                    return nullptr;
            }
            return nullptr;
        }

        std::size_t newSize = size >> 1;    // -> size / 2;
        auto* newPtr = pPtr + (newSize * 8);
        int32_t val = binaryloader::loadInt(newPtr + 4);
        if (val == pKey)
            return beginPtr + binaryloader::loadInt(newPtr);

        if (val > pKey) {
            size = newSize;
            continue;
        }

        pPtr = newPtr + 8;
        size = size - newSize - 1;
    }
    return nullptr;
}

inline static void readAllIntMapValues(const unsigned char* pPtr,
                                       const std::function<void(int, const unsigned char*)>& pOnEachElements) {
    const unsigned char* beginPtr = pPtr;
    uint32_t nbOfElts = binaryloader::loadInt(pPtr);
    pPtr += 4;
    const unsigned char* firstKeyPtr = pPtr;
    for (uint32_t i = 0; i < nbOfElts; ++i) {
        const unsigned char* newPtr = pPtr;
        pPtr += 4;
        uint32_t key = binaryloader::loadInt(pPtr);
        pPtr += 4;
        if (i == 0)
            pOnEachElements(key, firstKeyPtr + (nbOfElts * 8));
        else
            pOnEachElements(key, beginPtr + binaryloader::loadInt(newPtr));
    }
}

inline static const unsigned char* jumpAfterIntMap(const unsigned char* pPtr) {
    return pPtr + binaryloader::loadInt(pPtr + 4);
}

}    // End of namespace onsem

#endif    // ONSEM_COMMON_BINARY_ENUMMAPREADER_HPP
