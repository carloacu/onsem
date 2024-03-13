#ifndef ONSEM_TEXTTOSEMANTIC_TOOL_LISTITER_HPP
#define ONSEM_TEXTTOSEMANTIC_TOOL_LISTITER_HPP

#include <list>
#include "chunkshandler.hpp"

namespace onsem {
namespace linguistics {

struct ChunkLinkIter {
    ChunkLinkIter(const ChunkLinkWorkingZone& pWorkingZone, std::list<ChunkLink>::iterator pIt)
        : workingZone(pWorkingZone)
        , it(pIt) {}

    ChunkLinkIter(std::list<ChunkLink>& pSyntTree, std::list<ChunkLink>::iterator pIt)
        : workingZone(ChunkLinkWorkingZone(pSyntTree, pSyntTree.begin(), pSyntTree.end()))
        , it(pIt) {}

    bool operator==(const ChunkLinkIter& pOther) const { return workingZone == pOther.workingZone && it == pOther.it; }

    bool operator!=(const ChunkLinkIter& pOther) const { return !(*this == pOther); }

    ChunkLinkIter& operator++() {
        ++it;
        return *this;
    }

    ChunkLinkIter& operator--() {
        if (it == workingZone.begin())
            it = workingZone.end();
        else
            --it;
        return *this;
    }

    ChunkLink& operator*() { return *it; }

    ChunkLink* operator->() { return &*it; }

    bool atEnd() const { return it == workingZone.end(); }

    std::list<ChunkLink>* getList() { return workingZone.syntTreePtr(); }

    std::list<ChunkLink>::iterator getIt() const { return it; }

    void eraseIt() { it = workingZone.syntTree().erase(it); }

    void setItAtEnd() { it = workingZone.end(); }

private:
    ChunkLinkWorkingZone workingZone;
    std::list<ChunkLink>::iterator it;
};

}    // End of namespace linguistics
}    // End of namespace onsem

#endif    // ONSEM_TEXTTOSEMANTIC_TOOL_LISTITER_HPP
