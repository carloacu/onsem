#include <onsem/texttosemantic/dbtype/misc/conditionspecification.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/conditionexpression.hpp>

namespace onsem {

std::unique_ptr<ConditionExpression> ConditionSpecification::convertToConditionExpression() const {
    auto condExp = std::make_unique<ConditionExpression>(
        isAlwaysActive, conditionShouldBeInformed, conditionExp.clone(), thenExp.clone());
    if (elseExpPtr != nullptr)
        condExp->elseExp = mystd::make_unique_pc<UniqueSemanticExpression>(elseExpPtr->clone());
    return condExp;
}

}    // End of namespace onsem
