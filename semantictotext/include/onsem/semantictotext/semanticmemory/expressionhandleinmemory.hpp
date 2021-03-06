#ifndef ONSEM_SEMANTICTOTEXT_SEMANTICMEMORY_EXPRESSIONWRAPPERFORMEMORY_HPP
#define ONSEM_SEMANTICTOTEXT_SEMANTICMEMORY_EXPRESSIONWRAPPERFORMEMORY_HPP

#include <list>
#include <map>
#include <memory>
#include "../api.hpp"
#include "semanticcontextaxiom.hpp"
#include <onsem/common/utility/radix_map.hpp>
#include <onsem/texttosemantic/dbtype/misc/conditionspecification.hpp>
#include <onsem/texttosemantic/dbtype/misc/truenessvalue.hpp>


namespace onsem
{
struct SemanticMemoryBlock;


struct ONSEMSEMANTICTOTEXT_API ExpressionHandleInMemory
{
  ExpressionHandleInMemory
  (SemanticMemoryBlock& pParentMemBloc,
   UniqueSemanticExpression pSemExp,
   const mystd::radix_map_str<std::string>* pLinkedInfosPtr = nullptr);

  ExpressionHandleInMemory(ExpressionHandleInMemory&& pOther) = delete;
  ExpressionHandleInMemory& operator=(ExpressionHandleInMemory&& pOther) = delete;
  ExpressionHandleInMemory(const ExpressionHandleInMemory&) = delete;
  ExpressionHandleInMemory& operator=(const ExpressionHandleInMemory&) = delete;

  void clearWrappings(SemanticMemoryBlock& pMemBlock,
                      const linguistics::LinguisticDatabase& pLingDb,
                      std::map<const SemanticContextAxiom*, TruenessValue>* pAxiomToConditionCurrentStatePtr);
  void removeContextAxiomsWithAnActionLinked();

  void addConditionToAnAction(InformationType pInformationType,
                              const ConditionSpecification& pCondExp,
                              const linguistics::LinguisticDatabase& pLingDb);
  void addConditionToAnInfo(InformationType pInformationType,
                            const ConditionSpecification& pCondExp,
                            const linguistics::LinguisticDatabase& pLingDb);

  SemanticContextAxiom* addAxiomFromGrdExp(InformationType pInformationType,
                                           const GroundedExpression& pGrdSemExpToAdd,
                                           const std::map<GrammaticalType, const SemanticExpression*>& pAnnotations,
                                           const linguistics::LinguisticDatabase& pLingDb);
  SemanticContextAxiom* tryToAddTeachFormulation(InformationType pInformationType,
                                                 const GroundedExpression& pGrdSemExpToAdd,
                                                 const std::map<GrammaticalType, const SemanticExpression*>& pAnnotations,
                                                 const linguistics::LinguisticDatabase& pLingDb);

  void addAxiomWhereGatherAllTheLinks(const GroundedExpression& pGrdSemExpToAdd,
                                      const linguistics::LinguisticDatabase& pLingDb);
  void addTriggerLinks(InformationType pInformationType,
                       const SemanticExpression& pSemExp,
                       const linguistics::LinguisticDatabase& pLingDb);

  void addAxiomListToMemory(const SemanticExpression& pSemExpToAdd,
                            std::shared_ptr<SemanticTracker>* pSemTracker,
                            InformationType pInformationType,
                            bool pActionToDoIsAlwaysActive,
                            const SemanticExpression* pActionToDo,
                            const SemanticExpression* pActionToDoElse,
                            const SemanticExpression* pSemExpToAddWithoutLinks,
                            const linguistics::LinguisticDatabase& pLingDb);

  SemanticMemoryBlock& getParentMemBloc() { return  _parentMemBloc; }
  const SemanticMemoryBlock& getParentMemBloc() const { return  _parentMemBloc; }
  intSemId getIdOfFirstSentence() const;

  mystd::radix_map_str<std::string> linkedInfos;
  UniqueSemanticExpression semExp;
  std::list<SemanticContextAxiom> contextAxioms;

  /// SemExp to do if it's a trigger
  mystd::unique_propagate_const<UniqueSemanticExpression> outputToAnswerIfTriggerHasMatched;

private:
  SemanticMemoryBlock& _parentMemBloc;

  void _addContextAxiom
  (SemanticContextAxiom& pContextAxiom,
   const SemanticExpression& pSemExpToAdd,
   const SemanticExpression* pSemExpToAddWithoutLinks,
   const linguistics::LinguisticDatabase& pLingDb);

  void _addTriggerGrdExpLinks(InformationType pInformationType,
                              const GroundedExpression& pTriggerGrdExp,
                              const std::function<SemanticTriggerAxiomId(std::size_t)>& pGetAxiomIdFromId,
                              const linguistics::LinguisticDatabase& pLingDb,
                              std::size_t pId = 0);
  void _addTriggerGrdExpsLinks(InformationType pInformationType,
                               const std::list<const GroundedExpression*>& pTriggerGrdExpPtrs,
                               const std::function<SemanticTriggerAxiomId(std::size_t)>& pGetAxiomIdFromId,
                               const linguistics::LinguisticDatabase& pLingDb);
};



} // End of namespace onsem


#endif // ONSEM_SEMANTICTOTEXT_SEMANTICMEMORY_EXPRESSIONWRAPPERFORMEMORY_HPP
