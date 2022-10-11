#include "conditionsadder.hpp"
#include <onsem/texttosemantic/dbtype/semanticexpressions.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticagentgrounding.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticrelativedurationgrounding.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticstatementgrounding.hpp>
#include <onsem/texttosemantic/tool/semexpgetter.hpp>
#include <onsem/texttosemantic/tool/semexpmodifier.hpp>
#include "simplesentencesplitter.hpp"
#include "../utility/semexpcreator.hpp"

namespace onsem
{
namespace conditionsAdder
{


void addConditonsForSomeTimedGrdExp(UniqueSemanticExpression& pSemExp,
                                    const std::string& pUserId)
{
  switch (pSemExp->type)
  {
  case SemanticExpressionType::GROUNDED:
  {
    GroundedExpression& grdExp = pSemExp->getGrdExp();
    if (grdExp->type == SemanticGroundingType::STATEMENT)
    {
      const SemanticStatementGrounding& statGrd = grdExp->getStatementGrounding();

      const GroundedExpression* objectGrdExpPtr = SemExpGetter::getGrdExpToDo(grdExp, statGrd, pUserId);
      if (objectGrdExpPtr != nullptr)
      {
        pSemExp = SemExpCreator::getImperativeAssociateFrom(*objectGrdExpPtr);
        return addConditonsForSomeTimedGrdExp(pSemExp, pUserId);
      }


      if (statGrd.verbTense == SemanticVerbTense::PUNCTUALPRESENT &&
          statGrd.requests.has(SemanticRequestType::ACTION))
      {
        auto itTimeChild = grdExp.children.find(GrammaticalType::TIME);
        if (itTimeChild != grdExp.children.end())
        {
          const GroundedExpression* timeGrdExpPtr = itTimeChild->second->getGrdExpPtr_SkipWrapperPtrs();
          if (timeGrdExpPtr != nullptr)
          {
            auto timeGrdType = timeGrdExpPtr->grounding().type;
            if (timeGrdType == SemanticGroundingType::STATEMENT ||
                timeGrdType == SemanticGroundingType::TIME)
            {
              auto timeSemExp = itTimeChild->second.extractContent();
              SemExpModifier::removeChildFromSemExp(*timeSemExp, GrammaticalType::INTRODUCTING_WORD);
              splitter::splitInVerySimpleSentences(timeSemExp, false);
              grdExp.children.erase(itTimeChild);
              pSemExp = std::make_unique<ConditionExpression>
                  (timeGrdType == SemanticGroundingType::STATEMENT, false,
                   std::move(timeSemExp), std::move(pSemExp));
              break;
            }
          }
        }

        auto itDurationChild = grdExp.children.find(GrammaticalType::DURATION);
        if (itDurationChild != grdExp.children.end())
        {
          const GroundedExpression* durationGrdExpPtr = itDurationChild->second->getGrdExpPtr_SkipWrapperPtrs();
          if (durationGrdExpPtr != nullptr)
          {
            auto relDurationGrdPtr = durationGrdExpPtr->grounding().getRelDurationGroundingPtr();
            if (relDurationGrdPtr != nullptr &&
                relDurationGrdPtr->durationType == SemanticRelativeDurationType::DELAYEDSTART)
            {
              auto durationSemExp = itDurationChild->second.extractContent();
              SemExpModifier::removeChildFromSemExp(*durationSemExp, GrammaticalType::INTRODUCTING_WORD);
              splitter::splitInVerySimpleSentences(durationSemExp, false);
              grdExp.children.erase(itDurationChild);
              pSemExp = std::make_unique<ConditionExpression>(false, false,
                                                                std::move(durationSemExp), std::move(pSemExp));
              break;
            }
          }
        }
      }
    }
    break;
  }
  case SemanticExpressionType::LIST:
  {
    ListExpression& listExp = pSemExp->getListExp();
    for (auto& currElt : listExp.elts)
      addConditonsForSomeTimedGrdExp(currElt, pUserId);
    break;
  }
  case SemanticExpressionType::INTERPRETATION:
  {
    addConditonsForSomeTimedGrdExp(pSemExp->getIntExp().interpretedExp, pUserId);
    break;
  }
  case SemanticExpressionType::FEEDBACK:
  {
    addConditonsForSomeTimedGrdExp(pSemExp->getFdkExp().concernedExp, pUserId);
    break;
  }
  case SemanticExpressionType::ANNOTATED:
  {
    addConditonsForSomeTimedGrdExp(pSemExp->getAnnExp().semExp, pUserId);
    break;
  }
  case SemanticExpressionType::METADATA:
  {
    auto& metadataExp = pSemExp->getMetadataExp();
    const SemanticAgentGrounding* agentPtr = nullptr;
    auto* authorSemExpPtr = metadataExp.getAuthorSemExpPtr();
    if (authorSemExpPtr != nullptr)
      agentPtr = SemExpGetter::extractAgentGrdPtr(*authorSemExpPtr);
    if (agentPtr != nullptr)
      addConditonsForSomeTimedGrdExp(metadataExp.semExp, agentPtr->userId);
    else
      addConditonsForSomeTimedGrdExp(metadataExp.semExp, pUserId);
    break;
  }
  case SemanticExpressionType::CONDITION:
  {
    auto& condExp = pSemExp->getCondExp();
    addConditonsForSomeTimedGrdExp(condExp.thenExp, pUserId);
    if (condExp.elseExp)
      addConditonsForSomeTimedGrdExp(*condExp.elseExp, pUserId);
    break;
  }
  case SemanticExpressionType::FIXEDSYNTHESIS:
  case SemanticExpressionType::COMMAND:
  case SemanticExpressionType::COMPARISON:
  case SemanticExpressionType::SETOFFORMS:
    break;
  }
}




} // End of namespace conditionsAdder
} // End of namespace onsem
