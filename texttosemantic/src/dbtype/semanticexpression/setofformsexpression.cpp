#include <onsem/texttosemantic/dbtype/semanticexpression/setofformsexpression.hpp>

namespace onsem {

QuestExpressionFrom::QuestExpressionFrom(UniqueSemanticExpression&& pUSemExp, bool pIsOriginalForm)
    : exp(std::move(pUSemExp))
    , isOriginalForm(pIsOriginalForm) {}

void QuestExpressionFrom::assertEltsEqual(const QuestExpressionFrom& pOther) const {
    exp->assertEqual(*pOther.exp);
    assert(isOriginalForm == pOther.isOriginalForm);
}

std::unique_ptr<SemanticExpression> SetOfFormsExpression::clone(
    const IndexToSubNameToParameterValue* pParams,
    bool pRemoveRecentContextInterpretations,
    const std::set<SemanticExpressionType>* pExpressionTypesToSkip) const {
    if (pExpressionTypesToSkip != nullptr
        && pExpressionTypesToSkip->find(SemanticExpressionType::SETOFFORMS) != pExpressionTypesToSkip->end())
        for (const auto& currRefExp : prioToForms)
            for (const auto& currRefForm : currRefExp.second)
                if (currRefForm->isOriginalForm)
                    return currRefForm->exp->clone(
                        pParams, pRemoveRecentContextInterpretations, pExpressionTypesToSkip);

    auto res = std::make_unique<SetOfFormsExpression>();
    for (const auto& currRefExp : prioToForms) {
        auto& formsToFill = res->prioToForms[currRefExp.first];
        for (const auto& currRefForm : currRefExp.second)
            formsToFill.emplace_back(std::make_unique<QuestExpressionFrom>(
                currRefForm->exp->clone(pParams, pRemoveRecentContextInterpretations), currRefForm->isOriginalForm));
    }
    return std::move(res);
}

UniqueSemanticExpression* SetOfFormsExpression::getOriginalForm() const {
    auto itFormsWithPrioOfTen = prioToForms.find(-10);    // because the original exp has a prio of -10
    if (itFormsWithPrioOfTen != prioToForms.end())
        for (const auto& currForm : itFormsWithPrioOfTen->second)
            if (currForm->isOriginalForm)
                return &currForm->exp;
    assert(false);
    return nullptr;
}

void SetOfFormsExpression::assertEltsEqual(const SetOfFormsExpression& pOther) const {
    assert(prioToForms.size() == pOther.prioToForms.size());
    auto itOtherPrioOfFrom = pOther.prioToForms.begin();
    for (const auto& currPrioToForm : prioToForms) {
        assert(currPrioToForm.first == itOtherPrioOfFrom->first);
        assert(currPrioToForm.second.size() == itOtherPrioOfFrom->second.size());
        auto itOtherQForm = itOtherPrioOfFrom->second.begin();
        for (const auto& currQForm : currPrioToForm.second) {
            assert(currQForm.operator bool() == itOtherQForm->operator bool());
            if (currQForm)
                currQForm->assertEltsEqual(**itOtherQForm);
            ++itOtherQForm;
        }
        ++itOtherPrioOfFrom;
    }
}

}    // End of namespace onsem
