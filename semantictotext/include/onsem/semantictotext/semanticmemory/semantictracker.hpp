#ifndef ONSEM_SEMANTICTOTEXT_SEMANTICMEMORY_SEMANTICTRACKER_HPP
#define ONSEM_SEMANTICTOTEXT_SEMANTICMEMORY_SEMANTICTRACKER_HPP

#include <onsem/common/utility/observable/observable.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/semanticexpression.hpp>
#include "../api.hpp"

namespace onsem {

class ONSEMSEMANTICTOTEXT_API SemanticTracker {
public:
    SemanticTracker() = default;

    mystd::observable::Observable<void(const UniqueSemanticExpression&)> val{};
};

}    // End of namespace onsem

#endif    // ONSEM_SEMANTICTOTEXT_SEMANTICMEMORY_SEMANTICTRACKER_HPP
