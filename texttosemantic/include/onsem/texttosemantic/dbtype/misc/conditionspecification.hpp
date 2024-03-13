#ifndef ONSEM_TEXTTOSEMANTIC_TYPE_MISC_CONDITIONSPECIFICATION_HPP
#define ONSEM_TEXTTOSEMANTIC_TYPE_MISC_CONDITIONSPECIFICATION_HPP

#include "../../api.hpp"
#include <memory>

namespace onsem {
struct SemanticExpression;
struct ConditionExpression;

struct ONSEM_TEXTTOSEMANTIC_API ConditionSpecification {
    ConditionSpecification(bool pIsAlwaysActive,
                           bool pConditionPointsToAffirmations,
                           const SemanticExpression& pConditionExp,
                           const SemanticExpression& pThenExp,
                           const SemanticExpression* pElseExpPtr)
        : isAlwaysActive(pIsAlwaysActive)
        , conditionShouldBeInformed(pConditionPointsToAffirmations)
        , conditionExp(pConditionExp)
        , thenExp(pThenExp)
        , elseExpPtr(pElseExpPtr) {}

    std::unique_ptr<ConditionExpression> convertToConditionExpression() const;

    bool isAlwaysActive;
    bool conditionShouldBeInformed;
    const SemanticExpression& conditionExp;
    const SemanticExpression& thenExp;
    const SemanticExpression* elseExpPtr;
};

}    // End of namespace onsem

#endif    // ONSEM_TEXTTOSEMANTIC_TYPE_MISC_CONDITIONSPECIFICATION_HPP
