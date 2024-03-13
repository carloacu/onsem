#ifndef ONSEM_COMMON_BINARY_RADIXMAPSAVER_HPP
#define ONSEM_COMMON_BINARY_RADIXMAPSAVER_HPP

#include <map>
#include "binarysaver.hpp"
#include <onsem/common/utility/radix_map.hpp>

namespace onsem {
namespace radixmap {

template<typename T>
binarymasks::Ptr _writeARadixMapNode(const mystd::radix_map_str<T>& pRadixMap,
                                     const binarymasks::Ptr pBeginMemory,
                                     binarymasks::Ptr pEndMemory,
                                     const std::function<void(binarymasks::Ptr&, const T*)>& pPrintEndOfNode) {
    // Print number of letters
    const std::string& keys = pRadixMap.getKeys();
    const std::size_t keysSize = keys.size();
    {
        auto nbOfKeysToWrite = keysSize;
        while (nbOfKeysToWrite >= 255) {
            binarysaver::writeChar(pEndMemory.pchar++, 255u);
            nbOfKeysToWrite -= 255;
        }
        unsigned char nbOfKeys = binarysaver::sizet_to_uchar(nbOfKeysToWrite);
        binarysaver::writeChar(pEndMemory.pchar++, nbOfKeys);
    }

    // Print each letters
    for (std::size_t i = 0u; i < keysSize; ++i)
        binarysaver::writeChar(pEndMemory.pchar++, keys[i]);

    // Print nb of children
    unsigned char nbOfChildren = binarysaver::sizet_to_uchar(pRadixMap.nbChildren());
    binarysaver::writeChar(pEndMemory.pchar++, nbOfChildren);

    // Memory allocation to write the children
    binarymasks::Ptr beginOfChildren = pEndMemory;
    pEndMemory.val += std::size_t(nbOfChildren * 4);

    // Print node content
    pPrintEndOfNode(pEndMemory, pRadixMap.getValuePtr());

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
        pEndMemory = _writeARadixMapNode(children[i].second, pBeginMemory, pEndMemory, pPrintEndOfNode);
    }
    return pEndMemory;
}

template<typename T>
binarymasks::Ptr write(const mystd::radix_map_str<T>& pRadixMap,
                       binarymasks::Ptr pEndMemory,
                       const std::function<void(binarymasks::Ptr&, const T*)>& pPrintEndOfNode) {
    pEndMemory = binarysaver::alignMemory(pEndMemory);
    return _writeARadixMapNode(pRadixMap, pEndMemory, pEndMemory, pPrintEndOfNode);
}

}    // End of namespace radixmap
}    // End of namespace onsem

#endif    // ONSEM_COMMON_BINARY_RADIXMAPSAVER_HPP
