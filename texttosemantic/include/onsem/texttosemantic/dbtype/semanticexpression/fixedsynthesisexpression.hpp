#ifndef ONSEM_TEXTTOSEMANTIC_TYPES_SEMANTICEXPRESSION_FIXEDSYNTHESISEXPRESSION_HPP
#define ONSEM_TEXTTOSEMANTIC_TYPES_SEMANTICEXPRESSION_FIXEDSYNTHESISEXPRESSION_HPP

#include "semanticexpression.hpp"
#include "../../api.hpp"

namespace onsem {

struct ONSEM_TEXTTOSEMANTIC_API FixedSynthesisExpression : public SemanticExpression {
    template<typename TSEMEXP>
    FixedSynthesisExpression(std::unique_ptr<TSEMEXP> pSemExp);

    FixedSynthesisExpression(UniqueSemanticExpression&& pSemExp);

    FixedSynthesisExpression(const FixedSynthesisExpression&) = delete;
    FixedSynthesisExpression& operator=(const FixedSynthesisExpression&) = delete;

    FixedSynthesisExpression& getFSynthExp() override { return *this; }
    const FixedSynthesisExpression& getFSynthExp() const override { return *this; }
    FixedSynthesisExpression* getFSynthExpPtr() override { return this; }
    const FixedSynthesisExpression* getFSynthExpPtr() const override { return this; }

    bool operator==(const FixedSynthesisExpression& pOther) const;
    bool isEqual(const FixedSynthesisExpression& pOther) const;
    void assertEltsEqual(const FixedSynthesisExpression& pOther) const;

    UniqueSemanticExpression& getUSemExp() { return _semExp; }
    const UniqueSemanticExpression& getUSemExp() const { return _semExp; }

    const SemanticExpression& getSemExp() const { return *_semExp; }
    SemanticExpression* getSemExpPtr() { return nullptr; }    // Because we cannot modify the sem exp!
    const SemanticExpression* getSemExpPtr() const { return &*_semExp; }

    std::unique_ptr<FixedSynthesisExpression> clone(
        const IndexToSubNameToParameterValue* pParams = nullptr,
        bool pRemoveRecentContextInterpretations = false,
        const std::set<SemanticExpressionType>* pExpressionTypesToSkip = nullptr) const;

    std::map<SemanticLanguageEnum, std::string> langToSynthesis;

private:
    UniqueSemanticExpression _semExp;
};

template<typename TSEMEXP>
FixedSynthesisExpression::FixedSynthesisExpression(std::unique_ptr<TSEMEXP> pSemExp)
    : SemanticExpression(SemanticExpressionType::FIXEDSYNTHESIS)
    , langToSynthesis()
    , _semExp(std::move(pSemExp)) {}

inline bool FixedSynthesisExpression::operator==(const FixedSynthesisExpression& pOther) const {
    return isEqual(pOther);
}

inline bool FixedSynthesisExpression::isEqual(const FixedSynthesisExpression& pOther) const {
    return langToSynthesis == pOther.langToSynthesis && _semExp == pOther._semExp;
}

}    // End of namespace onsem

#endif    // ONSEM_TEXTTOSEMANTIC_TYPES_SEMANTICEXPRESSION_FIXEDSYNTHESISEXPRESSION_HPP
