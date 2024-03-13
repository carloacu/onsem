#ifndef ONSEM_COMPILERMODEL_LINGDBMULTIMEANINGSNODE_HPP
#define ONSEM_COMPILERMODEL_LINGDBMULTIMEANINGSNODE_HPP

#include <vector>
#include <list>
#include <onsem/typeallocatorandserializer/typeallocatorandserializer.hpp>
#include <onsem/common/enum/lingenumlinkedmeaningdirection.hpp>

namespace onsem {
template<typename T>
struct ForwardPtrList;
class LingdbMeaning;
class LingdbWordForms;
class LingdbDynamicTrieNode;

struct LingdbNodeLinkedMeaning {
    LingdbNodeLinkedMeaning(char pDirection, LingdbMeaning* pdMeaning)
        : direction(pDirection)
        , meaning(pdMeaning) {}

    void init(char pDirection, LingdbMeaning* pdMeaning) {
        direction = pDirection;
        meaning = pdMeaning;
    }

    char direction;    // type is LinkedMeaningDirection
    LingdbMeaning* meaning;

    static void getPointers(std::vector<const void*>& pRes, void* pVar) {
        pRes.emplace_back(&reinterpret_cast<LingdbNodeLinkedMeaning*>(pVar)->meaning);
    }
};

/// A node of the dynamic trie that store the words.
class LingdbMultiMeaningsNode {
public:
    LingdbMeaning* getRootMeaning() const { return fRootMeaning; }

    const ForwardPtrList<LingdbNodeLinkedMeaning>* getLinkedMeanings() const { return fLinkedMeanings; }

    bool isStrEqualToListOfLemmes(const std::string& pStr, std::size_t pBegin) const;

    static void getPointers(std::vector<const void*>& pRes, void* pVar);

private:
    LingdbMeaning* fRootMeaning;
    ForwardPtrList<LingdbNodeLinkedMeaning>* fLinkedMeanings;

private:
    friend class LingdbDynamicTrieNode;

    LingdbMultiMeaningsNode(LingdbMeaning* pRootMeaning);

    void xInit(CompositePoolAllocator& pAlloc,
               LingdbMeaning* pRootMeaning,
               std::list<std::pair<LingdbMeaning*, LinkedMeaningDirection> >& pLinkedMeanings);
};

}    // End of namespace onsem

#endif    // ONSEM_COMPILERMODEL_LINGDBMULTIMEANINGSNODE_HPP
