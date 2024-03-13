#ifndef ONSEM_TEXTTOSEMANTIC_TYPES_SEMANTICEXPRESSION_FEEDBACKEXPRESSION_HPP
#define ONSEM_TEXTTOSEMANTIC_TYPES_SEMANTICEXPRESSION_FEEDBACKEXPRESSION_HPP

#include "semanticexpression.hpp"
#include "../../api.hpp"

namespace onsem {

struct ONSEM_TEXTTOSEMANTIC_API FeedbackExpression : public SemanticExpression {
    template<typename TSEMEXP>
    FeedbackExpression(std::unique_ptr<TSEMEXP> pFeedbackExp, std::unique_ptr<TSEMEXP> pConcernedExp);

    FeedbackExpression(UniqueSemanticExpression&& pFeedbackExp, UniqueSemanticExpression&& pConcernedExp);

    FeedbackExpression(const FeedbackExpression&) = delete;
    FeedbackExpression& operator=(const FeedbackExpression&) = delete;

    FeedbackExpression& getFdkExp() override { return *this; }
    const FeedbackExpression& getFdkExp() const override { return *this; }
    FeedbackExpression* getFdkExpPtr() override { return this; }
    const FeedbackExpression* getFdkExpPtr() const override { return this; }

    bool operator==(const FeedbackExpression& pOther) const;
    bool isEqual(const FeedbackExpression& pOther) const;
    void assertEltsEqual(const FeedbackExpression& pOther) const;

    UniqueSemanticExpression feedbackExp;
    UniqueSemanticExpression concernedExp;
};

template<typename TSEMEXP>
FeedbackExpression::FeedbackExpression(std::unique_ptr<TSEMEXP> pFeedbackExp, std::unique_ptr<TSEMEXP> pConcernedExp)
    : SemanticExpression(SemanticExpressionType::FEEDBACK)
    , feedbackExp(std::move(pFeedbackExp))
    , concernedExp(std::move(pConcernedExp)) {}

inline bool FeedbackExpression::operator==(const FeedbackExpression& pOther) const {
    return isEqual(pOther);
}

inline bool FeedbackExpression::isEqual(const FeedbackExpression& pOther) const {
    return feedbackExp == pOther.feedbackExp && concernedExp == pOther.concernedExp;
}

}    // End of namespace onsem

#endif    // ONSEM_TEXTTOSEMANTIC_TYPES_SEMANTICEXPRESSION_FEEDBACKEXPRESSION_HPP
