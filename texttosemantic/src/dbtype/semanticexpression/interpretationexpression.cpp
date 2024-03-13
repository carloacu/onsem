#include <onsem/texttosemantic/dbtype/semanticexpression/interpretationexpression.hpp>

namespace onsem {

InterpretationExpression::InterpretationExpression(InterpretationSource pSource,
                                                   UniqueSemanticExpression&& pInterpretedExp,
                                                   UniqueSemanticExpression&& pOriginalExp)
    : SemanticExpression(SemanticExpressionType::INTERPRETATION)
    , source(pSource)
    , interpretedExp(std::move(pInterpretedExp))
    , originalExp(std::move(pOriginalExp)) {}

void InterpretationExpression::assertEltsEqual(const InterpretationExpression& pOther) const {
    assert(source == pOther.source);
    interpretedExp->assertEqual(*pOther.interpretedExp);
    originalExp->assertEqual(*pOther.originalExp);
}

}    // End of namespace onsem
