#ifndef ONSEM_SEMANTICTOTEXT_SRC_SEMANTICMEMORY_SEMANTICMEMORYBLOCKVIEWER_HPP
#define ONSEM_SEMANTICTOTEXT_SRC_SEMANTICMEMORY_SEMANTICMEMORYBLOCKVIEWER_HPP

#include <string>
#include <onsem/semantictotext/semanticmemory/semanticmemoryblock.hpp>

namespace onsem {

struct SemanticMemoryBlockViewer {
    SemanticMemoryBlockViewer(SemanticMemoryBlock* pViewPtr,
                              const SemanticMemoryBlock& pConstView,
                              const std::string& pCurrentUserId)
        : constView(pConstView)
        , currentUserId(pCurrentUserId)
        , _viewPtr(pViewPtr) {}

    SemanticMemoryBlockViewer(const SemanticMemoryBlockViewer& pOther)
        : constView(pOther.constView)
        , currentUserId(pOther.currentUserId)
        , _viewPtr(nullptr) {}

    SemanticMemoryBlock* getViewPtr() { return _viewPtr; }
    const SemanticMemoryBlock* getViewPtr() const { return _viewPtr; }

    SemanticMemoryBlockPrivate* getViewPrivatePtr();
    const SemanticMemoryBlockPrivate* getViewPrivatePtr() const;
    const SemanticMemoryBlockPrivate& getConstViewPrivate() const;

    bool areSameUser(const std::string& pUserId1, const std::string& pUserId2, RelatedContextAxiom& pRelContextAxiom);

    const SemanticMemoryBlock& constView;
    const std::string currentUserId;

private:
    SemanticMemoryBlock* _viewPtr;
};

}    // End of namespace onsem

#endif    // ONSEM_SEMANTICTOTEXT_SRC_SEMANTICMEMORY_SEMANTICMEMORYBLOCKVIEWER_HPP
