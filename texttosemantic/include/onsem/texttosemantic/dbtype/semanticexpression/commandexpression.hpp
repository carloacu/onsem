#ifndef ONSEM_TEXTTOSEMANTIC_TYPES_SEMANTICEXPRESSION_COMMANDEXPRESSION_HPP
#define ONSEM_TEXTTOSEMANTIC_TYPES_SEMANTICEXPRESSION_COMMANDEXPRESSION_HPP

#include "semanticexpression.hpp"
#include "../../api.hpp"


namespace onsem
{


struct ONSEM_TEXTTOSEMANTIC_API CommandExpression : public SemanticExpression
{
  template<typename TSEMEXP>
  CommandExpression(std::unique_ptr<TSEMEXP> pSemExp);

  CommandExpression(UniqueSemanticExpression&& pSemExp);

  CommandExpression(const AnnotatedExpression&) = delete;
  CommandExpression& operator=(const AnnotatedExpression&) = delete;

  CommandExpression& getCmdExp() override { return *this; }
  const CommandExpression& getCmdExp() const override { return *this; }
  CommandExpression* getCmdExpPtr() override { return this; }
  const CommandExpression* getCmdExpPtr() const override { return this; }

  bool operator==(const CommandExpression& pOther) const;
  bool isEqual(const CommandExpression& pOther) const;
  void assertEltsEqual(const CommandExpression& pOther) const;

  std::unique_ptr<CommandExpression> clone(const IndexToSubNameToParameterValue* pParams = nullptr,
                                           bool pRemoveRecentContextInterpretations = false,
                                           const std::set<SemanticExpressionType>* pExpressionTypesToSkip = nullptr) const;


  UniqueSemanticExpression semExp;
  mystd::unique_propagate_const<UniqueSemanticExpression> description;
};



template<typename TSEMEXP>
CommandExpression::CommandExpression(std::unique_ptr<TSEMEXP> pSemExp)
  : SemanticExpression(SemanticExpressionType::COMMAND),
    semExp(std::move(pSemExp)),
    description()
{
}



} // End of namespace onsem

#endif // ONSEM_TEXTTOSEMANTIC_TYPES_SEMANTICEXPRESSION_COMMANDEXPRESSION_HPP
