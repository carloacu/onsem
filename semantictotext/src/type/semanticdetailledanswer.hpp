#ifndef ONSEM_SEMANTICTOTEXT_SRC_TYPE_SEMANTICDETAILLEDANSWER_HPP
#define ONSEM_SEMANTICTOTEXT_SRC_TYPE_SEMANTICDETAILLEDANSWER_HPP

#include <map>
#include <list>
#include <memory>
#include <onsem/common/enum/semanticrequesttype.hpp>
#include <onsem/semantictotext/enum/semanticexpressioncategory.hpp>
#include <onsem/common/enum/contextualannotation.hpp>
#include <onsem/common/enum/listexpressiontype.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/conditionexpression.hpp>
#include <onsem/texttosemantic/dbtype/misc/conditionspecification.hpp>
#include <onsem/texttosemantic/dbtype/misc/truenessvalue.hpp>
#include <onsem/texttosemantic/dbtype/misc/typeofunity.hpp>
#include <onsem/texttosemantic/dbtype/interactioncontext.hpp>
#include <onsem/semantictotext/semanticmemory/referencesgetter.hpp>
#include "answerexp.hpp"


namespace onsem
{
struct LeafSemAnswer;
struct CompositeSemAnswer;




struct ConditionResult
{
  ConditionResult
  (const ConditionSpecification& pCondition,
   SemanticExpressionCategory pThenCategory,
   SemanticExpressionCategory pElseCategory)
    : condition(pCondition),
      thenCategory(pThenCategory),
      elseCategory(pElseCategory)
  {
  }

  ConditionSpecification condition;
  SemanticExpressionCategory thenCategory;
  SemanticExpressionCategory elseCategory;
};


struct ConditionForAUser
{
  ConditionForAUser() = default;
  ConditionForAUser(const std::string& pUser,
                    std::unique_ptr<SemanticExpression> pThenSemExp);

  ConditionForAUser(ConditionForAUser&& pOther);
  ConditionForAUser& operator=(ConditionForAUser&& pOther);
  ConditionForAUser(const ConditionForAUser&) = delete;
  ConditionForAUser& operator=(const ConditionForAUser&) = delete;

  std::string user{};
  std::unique_ptr<SemanticExpression> thenSemExp{};
};


struct SemAnswer
{
  virtual ~SemAnswer() {}

  virtual LeafSemAnswer* getLeafPtr() { return nullptr; }
  virtual const LeafSemAnswer* getLeafPtr() const { return nullptr; }
  virtual CompositeSemAnswer* getCompositePtr() { return nullptr; }
  virtual const CompositeSemAnswer* getCompositePtr() const { return nullptr; }

  virtual void getSourceContextAxiom(RelatedContextAxiom& pRes) const = 0;
  virtual bool containsOnlyResourcesOrTexts() const = 0;
};


struct QuestionAskedInformation
{
  QuestionAskedInformation(SemanticRequestType pRequest,
                           const mystd::optional<TypeOfUnity>& pTypeOfUnityOpt = mystd::optional<TypeOfUnity>());
  bool operator<(const QuestionAskedInformation& pOther) const;

  SemanticRequestType request;
  mystd::optional<TypeOfUnity> typeOfUnityOpt;
};

struct AllAnswerElts : public ReferencesGetter
{
  bool isEmpty() const { return answersFromMemory.empty() && answersGenerated.empty(); }
  void getReferences(std::list<std::string>& pReferences) const override;
  int getNbOfTimes() const;

  std::list<AnswerExp> answersFromMemory{};
  // elts of the reaction, not coming from memory
  std::list<AnswerExpGenerated> answersGenerated{};
};


struct LeafSemAnswer : public SemAnswer
{
  LeafSemAnswer() = default;

  explicit LeafSemAnswer(ContextualAnnotation pType);

  LeafSemAnswer(ContextualAnnotation pType,
                UniqueSemanticExpression pReaction);

  LeafSemAnswer(ContextualAnnotation pType,
                UniqueSemanticExpression pReaction,
                const mystd::optional<ConditionResult>& pCondition);

  LeafSemAnswer(ContextualAnnotation pType,
                UniqueSemanticExpression pReaction,
                ConditionForAUser pConditionForAUser);

  LeafSemAnswer(const LeafSemAnswer&) = delete;
  LeafSemAnswer& operator=(const LeafSemAnswer&) = delete;

  LeafSemAnswer* getLeafPtr() override { return this; }
  const LeafSemAnswer* getLeafPtr() const override { return this; }

  void getGroundedExps(std::list<const GroundedExpression*>& pGrdExps) const;
  void getSourceContextAxiom(RelatedContextAxiom& pRes) const override;
  bool containsOnlyResourcesOrTexts() const override;
  bool isEmpty() const;

  // Type of the reaction
  ContextualAnnotation type{ContextualAnnotation::ANSWERNOTFOUND};
  // the reaction
  mystd::unique_propagate_const<UniqueSemanticExpression> reaction{};

  // elts of the reaction
  std::map<QuestionAskedInformation, AllAnswerElts> answerElts{};

  // the condition to link
  mystd::optional<ConditionResult> condition{};
  ConditionForAUser conditionForAUser{};

  // interaction context to start
  std::unique_ptr<InteractionContextContainer> interactionContextContainer{};
};


struct CompositeSemAnswer : public SemAnswer
{
  CompositeSemAnswer(ListExpressionType pListType);

  CompositeSemAnswer* getCompositePtr() override { return this; }
  const CompositeSemAnswer* getCompositePtr() const override { return this; }

  static void getGrdExps(
      std::list<const GroundedExpression*>& pGrdExps,
      const std::list<std::unique_ptr<SemAnswer>>& pSemEnswers);

  std::unique_ptr<UniqueSemanticExpression> convertToSemExp() const;

  void getSourceContextAxiom(RelatedContextAxiom& pRes) const override;
  bool containsOnlyResourcesOrTexts() const override;

  TruenessValue getAgreementValue() const;
  void keepOnlyTheResourcesOrTexts();
  std::unique_ptr<InteractionContextContainer> getInteractionContextContainer();

  ListExpressionType listType;
  std::list<std::unique_ptr<SemAnswer>> semAnswers;
};


} // End of namespace onsem


#endif // ONSEM_SEMANTICTOTEXT_SRC_TYPE_SEMANTICDETAILLEDANSWER_HPP
