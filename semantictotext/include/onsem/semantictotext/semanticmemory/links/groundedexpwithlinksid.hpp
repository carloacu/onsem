#ifndef ONSEM_SEMANTICTOTEXT_SEMANTICMEMORY_GROUNDEDEXPWITHLINKSID_HPP
#define ONSEM_SEMANTICTOTEXT_SEMANTICMEMORY_GROUNDEDEXPWITHLINKSID_HPP

#include <cstdint>

namespace onsem {

using intSemId = uint32_t;
using intMemBlockId = uint64_t;

struct semIdAbs {
    semIdAbs(intMemBlockId pMemBlockId, intSemId pSemId)
        : memBlockId(pMemBlockId)
        , semId(pSemId) {}

    bool operator<(const semIdAbs& pOther) const {
        if (memBlockId != pOther.memBlockId)
            return memBlockId < pOther.memBlockId;
        return semId < pOther.semId;
    }

    intMemBlockId memBlockId;
    intSemId semId;
};

}    // End of namespace onsem

#endif    // ONSEM_SEMANTICTOTEXT_SEMANTICMEMORY_GROUNDEDEXPWITHLINKSID_HPP
