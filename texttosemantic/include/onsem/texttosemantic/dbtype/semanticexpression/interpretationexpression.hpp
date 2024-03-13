#ifndef ONSEM_TEXTTOSEMANTIC_DBTYPE_SEMANTICEXPRESSION_INTERPRETATIONEXPRESSION_HPP
#define ONSEM_TEXTTOSEMANTIC_DBTYPE_SEMANTICEXPRESSION_INTERPRETATIONEXPRESSION_HPP

#include "semanticexpression.hpp"
#include "../../api.hpp"
#include <onsem/common/enum/interpretationsource.hpp>

namespace onsem {

struct ONSEM_TEXTTOSEMANTIC_API InterpretationExpression : public SemanticExpression {
    template<typename TSEMEXP>
    InterpretationExpression(InterpretationSource pSource,
                             std::unique_ptr<TSEMEXP> pInterpretedExp,
                             std::unique_ptr<TSEMEXP> pOriginalExp);

    InterpretationExpression(InterpretationSource pSource,
                             UniqueSemanticExpression&& pInterpretedExp,
                             UniqueSemanticExpression&& pOriginalExp);

    InterpretationExpression(const InterpretationExpression&) = delete;
    InterpretationExpression& operator=(const InterpretationExpression&) = delete;

    InterpretationExpression& getIntExp() override { return *this; }
    const InterpretationExpression& getIntExp() const override { return *this; }
    InterpretationExpression* getIntExpPtr() override { return this; }
    const InterpretationExpression* getIntExpPtr() const override { return this; }

    bool operator==(const InterpretationExpression& pOther) const;
    bool isEqual(const InterpretationExpression& pOther) const;
    void assertEltsEqual(const InterpretationExpression& pOther) const;

    InterpretationSource source;
    UniqueSemanticExpression interpretedExp;
    UniqueSemanticExpression originalExp;
};

template<typename TSEMEXP>
InterpretationExpression::InterpretationExpression(InterpretationSource pSource,
                                                   std::unique_ptr<TSEMEXP> pInterpretedExp,
                                                   std::unique_ptr<TSEMEXP> pOriginalExp)
    : SemanticExpression(SemanticExpressionType::INTERPRETATION)
    , source(pSource)
    , interpretedExp(std::move(pInterpretedExp))
    , originalExp(std::move(pOriginalExp)) {}

inline bool InterpretationExpression::operator==(const InterpretationExpression& pOther) const {
    return isEqual(pOther);
}

inline bool InterpretationExpression::isEqual(const InterpretationExpression& pOther) const {
    return source == pOther.source && interpretedExp == pOther.interpretedExp && originalExp == pOther.originalExp;
}

}    // End of namespace onsem

#endif    // ONSEM_TEXTTOSEMANTIC_DBTYPE_SEMANTICEXPRESSION_INTERPRETATIONEXPRESSION_HPP
