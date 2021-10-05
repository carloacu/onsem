#include "userinfosfiller.hpp"
#include <onsem/texttosemantic/dbtype/semanticexpressions.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticstatementgrounding.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticagentgrounding.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/conceptset.hpp>
#include <onsem/texttosemantic/tool/semexpgetter.hpp>


namespace onsem
{

namespace SemExpUserInfosFiller
{

void _tryAddAgentNames
(UserInfosContainer& pUserInfosContainer,
 const std::string& pUserId,
 const std::vector<std::string>& pNames,
 const std::set<SemanticGenderType>& pPossibleGenders)
{
  if (!pNames.empty())
  {
    pUserInfosContainer.addNames(pUserId, pNames);
    if (!pPossibleGenders.empty())
      pUserInfosContainer.addGenders(pUserId, pPossibleGenders);
  }
}

void tryToAddUserInfosWithTense
(UserInfosContainer& pUserInfosContainer,
 const GroundedExpression& pGrdExp,
 SemanticVerbTense pRootVerbTense);


void _tryToAddUserInfosOnSemExp
(UserInfosContainer& pUserInfosContainer,
 const SemanticExpression& pSemExp,
 SemanticVerbTense pRootVerbTense)
{
  switch (pSemExp.type)
  {
  case SemanticExpressionType::ANNOTATED:
  {
    _tryToAddUserInfosOnSemExp(pUserInfosContainer, *pSemExp.getAnnExp().semExp, pRootVerbTense);
    break;
  }
  case SemanticExpressionType::COMMAND:
  {
    _tryToAddUserInfosOnSemExp(pUserInfosContainer, *pSemExp.getCmdExp().semExp, pRootVerbTense);
    break;
  }
  case SemanticExpressionType::COMPARISON:
  {
    const auto& compExp = pSemExp.getCompExp();
    _tryToAddUserInfosOnSemExp(pUserInfosContainer, *compExp.leftOperandExp, pRootVerbTense);
    if (compExp.rightOperandExp)
      _tryToAddUserInfosOnSemExp(pUserInfosContainer, **compExp.rightOperandExp, pRootVerbTense);
    break;
  }
  case SemanticExpressionType::CONDITION:
  {
    const auto& condExp = pSemExp.getCondExp();
    _tryToAddUserInfosOnSemExp(pUserInfosContainer, *condExp.conditionExp, pRootVerbTense);
    _tryToAddUserInfosOnSemExp(pUserInfosContainer, *condExp.thenExp, pRootVerbTense);
    if (condExp.elseExp)
      _tryToAddUserInfosOnSemExp(pUserInfosContainer, **condExp.elseExp, pRootVerbTense);
    break;
  }
  case SemanticExpressionType::FEEDBACK:
  {
    const auto& fdkExp = pSemExp.getFdkExp();
    _tryToAddUserInfosOnSemExp(pUserInfosContainer, *fdkExp.concernedExp, pRootVerbTense);
    break;
  }
  case SemanticExpressionType::GROUNDED:
  {
    const auto& grdExp = pSemExp.getGrdExp();
    tryToAddUserInfosWithTense(pUserInfosContainer, grdExp, pRootVerbTense);
    break;
  }
  case SemanticExpressionType::INTERPRETATION:
  {
    const auto& intExp = pSemExp.getIntExp();
    _tryToAddUserInfosOnSemExp(pUserInfosContainer, *intExp.interpretedExp, pRootVerbTense);
    break;
  }
  case SemanticExpressionType::LIST:
  {
    const auto& listExp = pSemExp.getListExp();
    for (const auto& currElt : listExp.elts)
      _tryToAddUserInfosOnSemExp(pUserInfosContainer, *currElt, pRootVerbTense);
    break;
  }
  case SemanticExpressionType::METADATA:
  {
    const auto& metaExp = pSemExp.getMetadataExp();
    if (metaExp.source)
      _tryToAddUserInfosOnSemExp(pUserInfosContainer, **metaExp.source, pRootVerbTense);
    _tryToAddUserInfosOnSemExp(pUserInfosContainer, *metaExp.semExp, pRootVerbTense);
    break;
  }
  case SemanticExpressionType::FIXEDSYNTHESIS:
  {
    const auto& fSynthExp = pSemExp.getFSynthExp();
    _tryToAddUserInfosOnSemExp(pUserInfosContainer, fSynthExp.getSemExp(), pRootVerbTense);
    break;
  }
  case SemanticExpressionType::SETOFFORMS:
    break;
  }
}


void tryToAddUserInfosWithTense
(UserInfosContainer& pUserInfosContainer,
 const GroundedExpression& pGrdExp,
 SemanticVerbTense pRootVerbTense)
{
  switch (pGrdExp->type)
  {
  case SemanticGroudingType::STATEMENT:
  {
    const auto& statGrd = pGrdExp->getStatementGrounding();
    if (pRootVerbTense == SemanticVerbTense::PRESENT)
    {
      if (!statGrd.requests.empty())
        break;
      std::string subjectUserId;
      std::string objectUserId;
      const GroundedExpression* subjectGrdExpPtr = nullptr;
      const GroundedExpression* objectGrdExpPtr = nullptr;
      if (SemExpGetter::getSubjectAndObjectUserIdsOfNameAssignement(subjectUserId, objectUserId,
                                                                    subjectGrdExpPtr, objectGrdExpPtr,
                                                                    pGrdExp))
      {
        if (subjectGrdExpPtr != nullptr)
          pUserInfosContainer.addGrdExpToUserId(*subjectGrdExpPtr, objectUserId);
        else if (objectGrdExpPtr != nullptr)
          pUserInfosContainer.addGrdExpToUserId(*objectGrdExpPtr, subjectUserId);
        else
          pUserInfosContainer.addEquivalentUserIds(subjectUserId, objectUserId);
      }
    }
    break;
  }
  case SemanticGroudingType::AGENT:
  {
    const SemanticAgentGrounding& agentGrd = pGrdExp->getAgentGrounding();
    if (agentGrd.nameInfos)
      _tryAddAgentNames(pUserInfosContainer, agentGrd.userId, agentGrd.nameInfos->names, agentGrd.nameInfos->possibleGenders);

    // extract equivalent names from sentence like: "Aaa called Bbb is a..."
    auto itSpecifier = pGrdExp.children.find(GrammaticalType::SPECIFIER);
    if (itSpecifier != pGrdExp.children.end())
    {
      std::list<const GroundedExpression*> specGrdExps;
      itSpecifier->second->getGrdExpPtrs_SkipWrapperLists(specGrdExps);
      for (const auto& currSpecGrdExp : specGrdExps)
      {
        const auto* statGrdPtr = currSpecGrdExp->grounding().getStatementGroundingPtr();
        if (statGrdPtr != nullptr &&
            ConceptSet::haveAnyOfConcepts(statGrdPtr->concepts, {"verb_action_say", "predicate_hasName", "predicate_hasName_call"}))
        {
          auto itCalledSubject = currSpecGrdExp->children.find(GrammaticalType::SUBJECT);
          if (itCalledSubject != currSpecGrdExp->children.end() &&
              SemExpGetter::isACoreference(*itCalledSubject->second, CoreferenceDirectionEnum::PARENT))
          {
            auto itCalledObject = currSpecGrdExp->children.find(GrammaticalType::OBJECT);
            if (itCalledObject != currSpecGrdExp->children.end())
            {
              auto* calledObjectAgentPtr = SemExpGetter::extractAgentGrdPtr(*itCalledObject->second);
              if (calledObjectAgentPtr != nullptr)
                pUserInfosContainer.addEquivalentUserIds(agentGrd.userId, calledObjectAgentPtr->userId);
            }
          }
        }
      }
    }
    break;
  }
  default:
    break;
  }
  for (auto& child : pGrdExp.children)
    _tryToAddUserInfosOnSemExp(pUserInfosContainer, *child.second, pRootVerbTense);
}

void tryToAddUserInfos
(UserInfosContainer& pUserInfosContainer,
 const GroundedExpression& pGrdExp)
{
  SemanticVerbTense rootVerbTense = SemanticVerbTense::UNKNOWN;
  const auto* grdStatPtr = pGrdExp->getStatementGroundingPtr();
  if (grdStatPtr != nullptr)
    rootVerbTense = grdStatPtr->verbTense;
  tryToAddUserInfosWithTense(pUserInfosContainer, pGrdExp, rootVerbTense);
}


} // End of namespace SemExpUserInfosFiller

} // End of namespace onsem
