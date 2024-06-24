#ifndef ONSEM_TEXTTOSEMANTIC_TYPES_SEMANTICEXPRESSION_ANNOTATEDEXPRESSION_HPP
#define ONSEM_TEXTTOSEMANTIC_TYPES_SEMANTICEXPRESSION_ANNOTATEDEXPRESSION_HPP

#include "semanticexpression.hpp"
#include "../../api.hpp"

namespace onsem {

struct ONSEM_TEXTTOSEMANTIC_API AnnotatedExpression : public SemanticExpression {
    template<typename TSEMEXP>
    AnnotatedExpression(std::unique_ptr<TSEMEXP> pSemExp);

    AnnotatedExpression(UniqueSemanticExpression&& pSemExp);

    AnnotatedExpression(const AnnotatedExpression&) = delete;
    AnnotatedExpression& operator=(const AnnotatedExpression&) = delete;

    std::unique_ptr<SemanticExpression> clone(const IndexToSubNameToParameterValue* pParams,
                                              bool pRemoveRecentContextInterpretations,
                                              const std::set<SemanticExpressionType>* pExpressionTypesToSkip) const;

    AnnotatedExpression& getAnnExp() override { return *this; }
    const AnnotatedExpression& getAnnExp() const override { return *this; }
    AnnotatedExpression* getAnnExpPtr() override { return this; }
    const AnnotatedExpression* getAnnExpPtr() const override { return this; }

    bool operator==(const AnnotatedExpression& pOther) const;
    bool isEqual(const AnnotatedExpression& pOther) const;
    void assertEltsEqual(const AnnotatedExpression& pOther) const;

    bool synthesizeAnnotations;
    std::map<GrammaticalType, UniqueSemanticExpression> annotations;
    UniqueSemanticExpression semExp;
};

template<typename TSEMEXP>
AnnotatedExpression::AnnotatedExpression(std::unique_ptr<TSEMEXP> pSemExp)
    : SemanticExpression(SemanticExpressionType::ANNOTATED)
    , synthesizeAnnotations(false)
    , annotations()
    , semExp(std::move(pSemExp)) {}

inline bool AnnotatedExpression::operator==(const AnnotatedExpression& pOther) const {
    return isEqual(pOther);
}

inline bool AnnotatedExpression::isEqual(const AnnotatedExpression& pOther) const {
    return synthesizeAnnotations == pOther.synthesizeAnnotations && annotations == pOther.annotations
        && semExp == pOther.semExp;
}

inline std::unique_ptr<SemanticExpression> AnnotatedExpression::clone(
    const IndexToSubNameToParameterValue* pParams,
    bool pRemoveRecentContextInterpretations,
    const std::set<SemanticExpressionType>* pExpressionTypesToSkip) const {
    if (pExpressionTypesToSkip != nullptr && pExpressionTypesToSkip->count(SemanticExpressionType::ANNOTATED) > 0)
        return semExp->clone(pParams, pRemoveRecentContextInterpretations, pExpressionTypesToSkip);

    auto res = std::make_unique<AnnotatedExpression>(
        semExp->clone(pParams, pRemoveRecentContextInterpretations, pExpressionTypesToSkip));
    res->synthesizeAnnotations = synthesizeAnnotations;
    for (const auto& currAnn : annotations)
        res->annotations.emplace(
            currAnn.first, currAnn.second->clone(pParams, pRemoveRecentContextInterpretations, pExpressionTypesToSkip));
    res->fromText = fromText;
    return res;
}

}    // End of namespace onsem

#endif    // ONSEM_TEXTTOSEMANTIC_TYPES_SEMANTICEXPRESSION_ANNOTATEDEXPRESSION_HPP
