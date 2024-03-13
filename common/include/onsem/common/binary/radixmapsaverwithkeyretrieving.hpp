#ifndef ONSEM_COMMON_BINARY_RADIXMAPSAVERWITHKEYRETRIEVING_HPP
#define ONSEM_COMMON_BINARY_RADIXMAPSAVERWITHKEYRETRIEVING_HPP

#include <map>
#include "binarysaver.hpp"
#include <onsem/common/utility/radix_map.hpp>

namespace onsem {
namespace radixmap {

template<typename T>
binarymasks::Ptr _writeARadixMapNodeWKR(int pParentPtr,
                                        const mystd::radix_map_str<T>& pRadixMap,
                                        const binarymasks::Ptr pBeginMemory,
                                        binarymasks::Ptr pEndMemory,
                                        std::map<std::string, int>& pKeysToOffset,
                                        std::string& pKeyPrefix,
                                        const std::function<void(binarymasks::Ptr&, const T*)>& pPrintEndOfNode) {
    int decNodeOffset = pEndMemory.val - pBeginMemory.val;
    int decNode = binarysaver::alignedDecToSave(decNodeOffset);

    // write parent node
    binarysaver::writeInThreeBytes(pEndMemory.pchar, pParentPtr);
    pEndMemory.val += 3;

    // Print number of letters
    const std::string& keys = pRadixMap.getKeys();
    unsigned char nbOfKeys = binarysaver::sizet_to_uchar(keys.size());
    binarysaver::writeChar(pEndMemory.pchar++, nbOfKeys);
    pKeyPrefix += keys;

    // Print each letters
    for (unsigned char i = 0u; i < nbOfKeys; ++i)
        binarysaver::writeChar(pEndMemory.pchar++, keys[i]);

    // Print nb of children
    unsigned char nbOfChildren = binarysaver::sizet_to_uchar(pRadixMap.nbChildren());
    binarysaver::writeChar(pEndMemory.pchar++, nbOfChildren);

    // Memory allocation to write the children
    binarymasks::Ptr beginOfChildren = pEndMemory;
    pEndMemory.val += std::size_t(nbOfChildren * 4);

    // Print node content
    auto* valPtr = pRadixMap.getValuePtr();
    if (valPtr != nullptr) {
        pPrintEndOfNode(pEndMemory, valPtr);
        pKeysToOffset.emplace(pKeyPrefix, decNodeOffset);
    } else {
        pPrintEndOfNode(pEndMemory, nullptr);
    }

    // Print each child
    const auto& children = pRadixMap.getChildrenContent();
    for (auto i = 0u; i < children.size(); ++i) {
        pEndMemory = binarysaver::alignMemory(pEndMemory);
        // offset of the child node
        binarysaver::writeInThreeBytes(beginOfChildren.pchar,
                                       binarysaver::alignedDecToSave(pEndMemory.val - pBeginMemory.val));
        beginOfChildren.val += 3;
        // first letter of the child nod
        char firstChar = children[i].first;
        binarysaver::writeChar(beginOfChildren.pchar++, firstChar);
        auto subKeyPrefix = pKeyPrefix + firstChar;
        pEndMemory = _writeARadixMapNodeWKR(
            decNode, children[i].second, pBeginMemory, pEndMemory, pKeysToOffset, subKeyPrefix, pPrintEndOfNode);
    }
    return pEndMemory;
}

template<typename T>
binarymasks::Ptr writeWithKeyRetrieving(const mystd::radix_map_str<T>& pRadixMap,
                                        binarymasks::Ptr pEndMemory,
                                        std::map<std::string, int>& pKeysToOffset,
                                        const std::function<void(binarymasks::Ptr&, const T*)>& pPrintEndOfNode) {
    pEndMemory = binarysaver::alignMemory(pEndMemory);
    std::string keyPrefix;
    return _writeARadixMapNodeWKR(0, pRadixMap, pEndMemory, pEndMemory, pKeysToOffset, keyPrefix, pPrintEndOfNode);
}

}    // End of namespace radixmap
}    // End of namespace onsem

#endif    // ONSEM_COMMON_BINARY_RADIXMAPSAVERWITHKEYRETRIEVING_HPP
