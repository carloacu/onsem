#include <onsem/texttosemantic/dbtype/semanticexpression/listexpression.hpp>

namespace onsem {

void ListExpression::assertEltsEqual(const ListExpression& pOther) const {
    assert(listType == pOther.listType);
    assert(elts.size() == pOther.elts.size());
    auto it = elts.begin();
    auto itOther = pOther.elts.begin();
    while (it != elts.end()) {
        (*it)->assertEqual(**itOther);
        ++it;
        ++itOther;
    }
}

std::unique_ptr<ListExpression> ListExpression::clone(
    const IndexToSubNameToParameterValue* pParams,
    bool pRemoveRecentContextInterpretations,
    const std::set<SemanticExpressionType>* pExpressionTypesToSkip) const {
    auto res = std::make_unique<ListExpression>(listType);
    for (const auto& currRefElt : elts)
        res->elts.emplace_back(currRefElt->clone(pParams, pRemoveRecentContextInterpretations, pExpressionTypesToSkip));
    res->fromText = fromText;
    return res;
}

}    // End of namespace onsem
