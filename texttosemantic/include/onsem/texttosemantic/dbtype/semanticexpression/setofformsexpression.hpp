#ifndef ALSEMEXPDATABASES_SETOFFORMSEXPRESSION_H
#define ALSEMEXPDATABASES_SETOFFORMSEXPRESSION_H


#include "semanticexpression.hpp"
#include "../../api.hpp"

namespace onsem
{


struct ONSEM_TEXTTOSEMANTIC_API QuestExpressionFrom
{
  QuestExpressionFrom(UniqueSemanticExpression&& pUSemExp,
                      bool pIsOriginalForm);

  template<typename TSEMEXP>
  QuestExpressionFrom(std::unique_ptr<TSEMEXP> pUSemExp,
                      bool pIsOriginalForm);

  QuestExpressionFrom(const QuestExpressionFrom&) = delete;
  QuestExpressionFrom& operator=(const QuestExpressionFrom&) = delete;

  bool operator==(const QuestExpressionFrom& pOther) const
  { return exp == pOther.exp && isOriginalForm == pOther.isOriginalForm; }
  void assertEltsEqual(const QuestExpressionFrom& pOther) const;

  UniqueSemanticExpression exp;
  bool isOriginalForm;
};




struct ONSEM_TEXTTOSEMANTIC_API SetOfFormsExpression : public SemanticExpression
{
  SetOfFormsExpression()
    : SemanticExpression(SemanticExpressionType::SETOFFORMS)
  {
  }

  SetOfFormsExpression(const SetOfFormsExpression&) = delete;
  SetOfFormsExpression& operator=(const SetOfFormsExpression&) = delete;

  SetOfFormsExpression& getSetOfFormsExp() override { return *this; }
  const SetOfFormsExpression& getSetOfFormsExp() const override { return *this; }
  SetOfFormsExpression* getSetOfFormsExpPtr() override { return this; }
  const SetOfFormsExpression* getSetOfFormsExpPtr() const override { return this; }

  bool operator==(const SetOfFormsExpression& pOther) const;
  bool isEqual(const SetOfFormsExpression& pOther) const;
  void assertEltsEqual(const SetOfFormsExpression& pOther) const;

  std::unique_ptr<SemanticExpression> clone(const IndexToSubNameToParameterValue* pParams = nullptr,
                                            bool pRemoveRecentContextInterpretations = false,
                                            const std::set<SemanticExpressionType>* pExpressionTypesToSkip = nullptr) const;
  UniqueSemanticExpression* getOriginalForm() const;

  std::map<int, std::list<std::unique_ptr<QuestExpressionFrom>>> prioToForms{};
};




template<typename TSEMEXP>
QuestExpressionFrom::QuestExpressionFrom
(std::unique_ptr<TSEMEXP> pUSemExp,
 bool pIsOriginalForm)
  : exp(std::move(pUSemExp)),
    isOriginalForm(pIsOriginalForm)
{
}



inline bool SetOfFormsExpression::operator==(const SetOfFormsExpression& pOther) const
{
  return isEqual(pOther);
}


inline bool SetOfFormsExpression::isEqual(const SetOfFormsExpression& pOther) const
{
  if (prioToForms.size() != pOther.prioToForms.size())
    return false;

  auto itOtherPrioOfFrom = pOther.prioToForms.begin();
  for (const auto& currPrioToForm : prioToForms)
  {
    if (currPrioToForm.first != itOtherPrioOfFrom->first ||
        currPrioToForm.second.size() != itOtherPrioOfFrom->second.size())
      return false;

    auto itOtherQForm = itOtherPrioOfFrom->second.begin();
    for (const auto& currQForm : currPrioToForm.second)
    {
      if (!areEquals(currQForm, *itOtherQForm))
        return false;
      ++itOtherQForm;
    }
    ++itOtherPrioOfFrom;
  }

  return true;
}


} // End of namespace onsem

#endif // ALSEMEXPDATABASES_SETOFFORMSEXPRESSION_H
