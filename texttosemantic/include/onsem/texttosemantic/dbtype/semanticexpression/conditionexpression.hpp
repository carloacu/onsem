#ifndef ONSEM_TEXTTOSEMANTIC_DBTYPE_SEMANTICEXPRESSION_CONDITIONEXPRESSION_HPP
#define ONSEM_TEXTTOSEMANTIC_DBTYPE_SEMANTICEXPRESSION_CONDITIONEXPRESSION_HPP

#include "semanticexpression.hpp"
#include <onsem/common/utility/unique_propagate_const.hpp>
#include "../../api.hpp"

namespace onsem {

struct ONSEM_TEXTTOSEMANTIC_API ConditionExpression : public SemanticExpression {
    template<typename TSEMEXP1, typename TSEMEXP2>
    ConditionExpression(bool pIsAlwaysActive,
                        bool pconditionPointsToAffirmations,
                        std::unique_ptr<TSEMEXP1> pConditionExp,
                        std::unique_ptr<TSEMEXP2> pThenExp);

    ConditionExpression(bool pIsAlwaysActive,
                        bool pconditionPointsToAffirmations,
                        UniqueSemanticExpression&& pConditionExp,
                        UniqueSemanticExpression&& pThenExp);

    ConditionExpression(const ConditionExpression&) = delete;
    ConditionExpression& operator=(const ConditionExpression&) = delete;

    ConditionExpression& getCondExp() override { return *this; }
    const ConditionExpression& getCondExp() const override { return *this; }
    ConditionExpression* getCondExpPtr() override { return this; }
    const ConditionExpression* getCondExpPtr() const override { return this; }

    bool operator==(const ConditionExpression& pOther) const;
    bool isEqual(const ConditionExpression& pOther) const;
    void assertEltsEqual(const ConditionExpression& pOther) const;

    std::unique_ptr<ConditionExpression> clone(
        const IndexToSubNameToParameterValue* pParams = nullptr,
        bool pRemoveRecentContextInterpretations = false,
        const std::set<SemanticExpressionType>* pExpressionTypesToSkip = nullptr) const;

    void toListOfGrdExpPtrs(std::list<const GroundedExpression*>& pGrdExpPtrs,
                            bool pFollowInterpretations = true) const;

    bool isAlwaysActive;
    // TODO: rename because here "affirmation" -> "sentence"
    bool conditionPointsToAffirmations;
    UniqueSemanticExpression conditionExp;
    std::string conditionStr;
    UniqueSemanticExpression thenExp;
    std::string thenStr;
    mystd::unique_propagate_const<UniqueSemanticExpression> elseExp;
    std::string elseStr;
};

template<typename TSEMEXP1, typename TSEMEXP2>
ConditionExpression::ConditionExpression(bool pIsAlwaysActive,
                                         bool pconditionPointsToAffirmations,
                                         std::unique_ptr<TSEMEXP1> pConditionExp,
                                         std::unique_ptr<TSEMEXP2> pThenExp)
    : SemanticExpression(SemanticExpressionType::CONDITION)
    , isAlwaysActive(pIsAlwaysActive)
    , conditionPointsToAffirmations(pconditionPointsToAffirmations)
    , conditionExp(std::move(pConditionExp))
    , conditionStr()
    , thenExp(std::move(pThenExp))
    , thenStr()
    , elseExp()
    , elseStr() {}

inline bool ConditionExpression::operator==(const ConditionExpression& pOther) const {
    return isEqual(pOther);
}

inline bool ConditionExpression::isEqual(const ConditionExpression& pOther) const {
    return isAlwaysActive == pOther.isAlwaysActive
        && conditionPointsToAffirmations == pOther.conditionPointsToAffirmations
        && conditionExp == pOther.conditionExp && conditionStr == pOther.conditionStr
        && thenExp == pOther.thenExp && thenStr == pOther.thenStr
        && elseExp == pOther.elseExp && elseStr == pOther.elseStr;
}

inline std::unique_ptr<ConditionExpression> ConditionExpression::clone(
    const IndexToSubNameToParameterValue* pParams,
    bool pRemoveRecentContextInterpretations,
    const std::set<SemanticExpressionType>* pExpressionTypesToSkip) const {
    auto res = std::make_unique<ConditionExpression>(
        isAlwaysActive,
        conditionPointsToAffirmations,
        conditionExp->clone(pParams, pRemoveRecentContextInterpretations, pExpressionTypesToSkip),
        thenExp->clone(pParams, pRemoveRecentContextInterpretations, pExpressionTypesToSkip));
    if (elseExp)
        res->elseExp = mystd::make_unique_pc<UniqueSemanticExpression>(
            (*elseExp)->clone(pParams, pRemoveRecentContextInterpretations, pExpressionTypesToSkip));
    return res;
}

inline void ConditionExpression::toListOfGrdExpPtrs(std::list<const GroundedExpression*>& pGrdExpPtrs,
                                                    bool pFollowInterpretations) const {
    conditionExp->getGrdExpPtrs_SkipWrapperLists(pGrdExpPtrs, pFollowInterpretations);
    thenExp->getGrdExpPtrs_SkipWrapperLists(pGrdExpPtrs, pFollowInterpretations);
}

}    // End of namespace onsem

#endif    // ONSEM_TEXTTOSEMANTIC_DBTYPE_SEMANTICEXPRESSION_CONDITIONEXPRESSION_HPP
