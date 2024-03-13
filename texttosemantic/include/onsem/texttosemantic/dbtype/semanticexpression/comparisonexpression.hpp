#ifndef ONSEM_TEXTTOSEMANTIC_TYPES_SEMANTICEXPRESSION_COMPARISONEXPRESSION_HPP
#define ONSEM_TEXTTOSEMANTIC_TYPES_SEMANTICEXPRESSION_COMPARISONEXPRESSION_HPP

#include "semanticexpression.hpp"
#include "../../api.hpp"
#include <onsem/common/enum/comparisonoperator.hpp>
#include <onsem/common/enum/semanticverbtense.hpp>
#include <onsem/common/enum/semanticrequesttype.hpp>

namespace onsem {

struct ONSEM_TEXTTOSEMANTIC_API ComparisonExpression : public SemanticExpression {
    template<typename TSEMEXP1, typename TSEMEXP2>
    ComparisonExpression(ComparisonOperator pComparisonOperator, std::unique_ptr<TSEMEXP2> pLeftOperandExp);

    ComparisonExpression(ComparisonOperator pComparisonOperator, UniqueSemanticExpression&& pLeftOperandExp);

    ComparisonExpression(const ComparisonExpression&) = delete;
    ConditionExpression& operator=(const ComparisonExpression&) = delete;

    ComparisonExpression& getCompExp() override { return *this; }
    const ComparisonExpression& getCompExp() const override { return *this; }
    ComparisonExpression* getCompExpPtr() override { return this; }
    const ComparisonExpression* getCompExpPtr() const override { return this; }

    bool operator==(const ComparisonExpression& pOther) const;
    bool isEqual(const ComparisonExpression& pOther) const;
    void assertEltsEqual(const ComparisonExpression& pOther) const;

    ComparisonOperator op;
    SemanticVerbTense tense;
    SemanticRequestType request;
    mystd::unique_propagate_const<UniqueSemanticExpression> whatIsComparedExp;
    UniqueSemanticExpression leftOperandExp;
    mystd::unique_propagate_const<UniqueSemanticExpression> rightOperandExp;
};

template<typename TSEMEXP1, typename TSEMEXP2>
ComparisonExpression::ComparisonExpression(ComparisonOperator pComparisonOperator,
                                           std::unique_ptr<TSEMEXP2> pLeftOperandExp)
    : SemanticExpression(SemanticExpressionType::COMPARISON)
    , op(pComparisonOperator)
    , tense(SemanticVerbTense::PRESENT)
    , request(SemanticRequestType::NOTHING)
    , whatIsComparedExp()
    , leftOperandExp(std::move(pLeftOperandExp))
    , rightOperandExp() {}

inline bool ComparisonExpression::operator==(const ComparisonExpression& pOther) const {
    return isEqual(pOther);
}

inline bool ComparisonExpression::isEqual(const ComparisonExpression& pOther) const {
    return op == pOther.op && tense == pOther.tense && request == pOther.request
        && whatIsComparedExp == pOther.whatIsComparedExp && leftOperandExp == pOther.leftOperandExp
        && rightOperandExp == pOther.rightOperandExp;
}

}    // End of namespace onsem

#endif    // ONSEM_TEXTTOSEMANTIC_TYPES_SEMANTICEXPRESSION_COMPARISONEXPRESSION_HPP
