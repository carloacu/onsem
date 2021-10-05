#ifndef ONSEM_SEMANTICTOTEXT_SEMANTICMEMORY_SEMANTICCONTEXTAXIOM_HPP
#define ONSEM_SEMANTICTOTEXT_SEMANTICMEMORY_SEMANTICCONTEXTAXIOM_HPP

#include "../api.hpp"
#include "semanticmemorysentencelist.hpp"
#include <onsem/common/enum/infomationtype.hpp>
#include <onsem/common/enum/listexpressiontype.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/semanticexpression.hpp>
#include <onsem/semantictotext/semanticmemory/referencesgetter.hpp>


namespace onsem
{
struct ExpressionHandleInMemory;
class SemanticTracker;
struct SemanticExpression;


struct ONSEMSEMANTICTOTEXT_API SemanticTriggerAxiomId
{
  SemanticTriggerAxiomId();
  SemanticTriggerAxiomId(std::size_t pNbOfAxioms,
                         std::size_t pIdOfAxiom,
                         SemanticExpressionType pSemExpType);
  SemanticTriggerAxiomId(std::size_t pNbOfAxioms,
                         std::size_t pIdOfAxiom,
                         ListExpressionType pListExpType);
  bool isEmpty() const;
  bool operator<(const SemanticTriggerAxiomId& pOther) const;

  std::size_t nbOfAxioms;
  std::size_t idOfAxiom;
  SemanticExpressionType semExpType;
  ListExpressionType listExpType;
};


struct ONSEMSEMANTICTOTEXT_API SemanticContextAxiom : public ReferencesGetter
{
  SemanticContextAxiom(
      InformationType pInformationType,
      ExpressionHandleInMemory& pSemExpWrappedForMemory);

  SemanticContextAxiom(const SemanticContextAxiom&) = delete;
  SemanticContextAxiom& operator=(const SemanticContextAxiom&) = delete;

  void setEnabled(bool pEnabled);
  void clear();
  bool isAnActionLinked() const;
  bool canOtherInformationTypeBeMoreRevelant(InformationType pInformationType) const;
  void getReferences(std::list<std::string>& pReferences) const override;

  ExpressionHandleInMemory& getSemExpWrappedForMemory() { return _semExpWrappedForMemory; }
  const ExpressionHandleInMemory& getSemExpWrappedForMemory() const { return _semExpWrappedForMemory; }

  /// Property to raise
  mystd::optional<std::shared_ptr<SemanticTracker>> semTracker;

  /// If the axiom is an assertion (so cannot be contradicted by the operator inform)
  const InformationType informationType;

  /// SemExp to do
  SemanticTriggerAxiomId triggerAxiomId;
  bool semExpToDoIsAlwaysActive;
  const SemanticExpression* semExpToDo;
  const SemanticExpression* semExpToDoElse;
  const SemanticExpression* infCommandToDo;

  SemanticMemorySentenceList memorySentences;

private:
  /// Parent semantic expression
  ExpressionHandleInMemory& _semExpWrappedForMemory;
};



} // End of namespace onsem


#endif // ONSEM_SEMANTICTOTEXT_SEMANTICMEMORY_SEMANTICCONTEXTAXIOM_HPP
