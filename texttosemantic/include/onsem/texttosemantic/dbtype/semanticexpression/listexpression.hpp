#ifndef ONSEM_TEXTTOSEMANTIC_DBTYPE_SEMANTICEXPRESSION_LISTEXPRESSION_HPP
#define ONSEM_TEXTTOSEMANTIC_DBTYPE_SEMANTICEXPRESSION_LISTEXPRESSION_HPP

#include "semanticexpression.hpp"
#include "../../api.hpp"
#include <onsem/common/enum/listexpressiontype.hpp>

namespace onsem {

struct ONSEM_TEXTTOSEMANTIC_API ListExpression : public SemanticExpression {
    ListExpression()
        : SemanticExpression(SemanticExpressionType::LIST)
        , listType(ListExpressionType::UNRELATED)
        , elts() {}

    ListExpression(ListExpressionType pListType)
        : SemanticExpression(SemanticExpressionType::LIST)
        , listType(pListType)
        , elts() {}

    ListExpression(const ListExpression&) = delete;
    ListExpression& operator=(const ListExpression&) = delete;

    ListExpression& getListExp() override { return *this; }
    const ListExpression& getListExp() const override { return *this; }
    ListExpression* getListExpPtr() override { return this; }
    const ListExpression* getListExpPtr() const override { return this; }

    bool operator==(const ListExpression& pOther) const { return isEqual(pOther); }
    bool isEqual(const ListExpression& pOther) const { return listType == pOther.listType && elts == pOther.elts; }
    void assertEltsEqual(const ListExpression& pOther) const;

    std::unique_ptr<ListExpression> clone(
        const IndexToSubNameToParameterValue* pParams = nullptr,
        bool pRemoveRecentContextInterpretations = false,
        const std::set<SemanticExpressionType>* pExpressionTypesToSkip = nullptr) const;

    ListExpressionType listType = ListExpressionType::UNRELATED;
    std::list<UniqueSemanticExpression> elts{};
};

}    // End of namespace onsem

#endif    // ONSEM_TEXTTOSEMANTIC_DBTYPE_SEMANTICEXPRESSION_LISTEXPRESSION_HPP
