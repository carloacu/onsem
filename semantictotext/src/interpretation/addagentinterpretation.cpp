#include "addagentinterpretation.hpp"
#include <sstream>
#include <onsem/texttosemantic/dbtype/semanticexpressions.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticagentgrounding.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticstatementgrounding.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticnamegrounding.hpp>
#include <onsem/texttosemantic/tool/semexpgetter.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemory.hpp>


namespace onsem
{
namespace
{

void _addAgentInterpretations(UniqueSemanticExpression& pSemExp,
                              const SemanticMemory& pSemanticMemory,
                              const linguistics::LinguisticDatabase& pLingDb,
                              GrammaticalType pParentGrammaticalType,
                              bool& pIsMeFromMyName)
{
  switch (pSemExp->type)
  {
  case SemanticExpressionType::ANNOTATED:
  {
    AnnotatedExpression& annExp = pSemExp->getAnnExp();
    _addAgentInterpretations(annExp.semExp, pSemanticMemory, pLingDb,
                             pParentGrammaticalType, pIsMeFromMyName);
    for (auto& currAnn : annExp.annotations)
      _addAgentInterpretations(currAnn.second, pSemanticMemory, pLingDb,
                               pParentGrammaticalType, pIsMeFromMyName);
    break;
  }
  case SemanticExpressionType::COMMAND:
  {
    CommandExpression& cmdExp = pSemExp->getCmdExp();
    _addAgentInterpretations(cmdExp.semExp, pSemanticMemory, pLingDb,
                             pParentGrammaticalType, pIsMeFromMyName);
    break;
  }
  case SemanticExpressionType::FEEDBACK:
  {
    FeedbackExpression& fdkExp = pSemExp->getFdkExp();
    _addAgentInterpretations(fdkExp.concernedExp, pSemanticMemory, pLingDb,
                             pParentGrammaticalType, pIsMeFromMyName);
    break;
  }
  case SemanticExpressionType::GROUNDED:
  {
    GroundedExpression& grdExp = pSemExp->getGrdExp();
    if (grdExp.grounding().type == SemanticGroundingType::NAME)
    {
      if (pParentGrammaticalType == GrammaticalType::LOCATION)
        break;
      SemanticNameGrounding& nameGrd = grdExp->getNameGrounding();
      const std::string userIdWithoutContext = SemanticAgentGrounding::namesToUserId(nameGrd.nameInfos.names);
      std::string userId = pSemanticMemory.memBloc.getUserId(nameGrd.nameInfos.names, userIdWithoutContext);
      if (pSemanticMemory.memBloc.isItMe(userId))
        pIsMeFromMyName = true;
      grdExp.moveGrounding(std::make_unique<SemanticAgentGrounding>(userId, userIdWithoutContext, nameGrd.nameInfos));
    }
    else if (grdExp.grounding().type == SemanticGroundingType::GENERIC)
    {
      SemanticGenericGrounding& genGrd = grdExp->getGenericGrounding();
      if (SemExpGetter::isASpecificHuman(genGrd))
      {
        std::string userId = pSemanticMemory.memBloc.getUserIdFromGrdExp(grdExp, pLingDb);
        if (userId != SemanticAgentGrounding::userNotIdentified)
        {
          pSemExp = std::make_unique<InterpretationExpression>
              (InterpretationSource::AGENTGRDEXP,
               UniqueSemanticExpression(std::make_unique<GroundedExpression>
                                        (std::make_unique<SemanticAgentGrounding>(userId))),
               std::move(pSemExp));
          break;
        }
      }
    }

    SemanticGrounding& grounding = grdExp.grounding();
    for (auto& child : grdExp.children)
    {
      bool isMeFromMyName = false;
      auto subParentGrammaticalType = child.first;
      if (semanticGroundingsType_isRelativeType(grounding.type))
        subParentGrammaticalType = pParentGrammaticalType;
      _addAgentInterpretations(child.second, pSemanticMemory, pLingDb, subParentGrammaticalType, isMeFromMyName);
      // if the author is the subject of the sentence and if it was named by his name,
      // then we consider that it the imperative form
      if (child.first == GrammaticalType::SUBJECT &&
          isMeFromMyName &&
          grounding.type == SemanticGroundingType::STATEMENT)
      {
        SemanticStatementGrounding& statGrd = grounding.getStatementGrounding();
        if (isAPresentTense(statGrd.verbTense) &&
            statGrd.requests.empty())
        {
          statGrd.verbTense = SemanticVerbTense::PUNCTUALPRESENT;
          statGrd.requests.set(SemanticRequestType::ACTION);
        }
      }
    }
    break;
  }
  case SemanticExpressionType::INTERPRETATION:
  {
    InterpretationExpression& intExp = pSemExp->getIntExp();
    _addAgentInterpretations(intExp.interpretedExp, pSemanticMemory, pLingDb,
                             pParentGrammaticalType, pIsMeFromMyName);
    break;
  }
  case SemanticExpressionType::LIST:
  {
    ListExpression& listExp = pSemExp->getListExp();
    for (auto& elt : listExp.elts)
    {
      bool isMeFromMyName = false;
      _addAgentInterpretations(elt, pSemanticMemory, pLingDb,
                               pParentGrammaticalType, isMeFromMyName);
    }
    break;
  }
  case SemanticExpressionType::METADATA:
  {
    MetadataExpression& metaExp = pSemExp->getMetadataExp();
    if (metaExp.source)
    {
      bool isMeFromMyName = false;
      _addAgentInterpretations(*metaExp.source, pSemanticMemory, pLingDb,
                               pParentGrammaticalType, isMeFromMyName);
    }
    _addAgentInterpretations(metaExp.semExp, pSemanticMemory, pLingDb,
                             pParentGrammaticalType, pIsMeFromMyName);
    break;
  }
  case SemanticExpressionType::CONDITION:
  {
    ConditionExpression& condExp = pSemExp->getCondExp();
    {
      bool isMeFromMyName = false;
      _addAgentInterpretations(condExp.conditionExp, pSemanticMemory, pLingDb,
                               pParentGrammaticalType, isMeFromMyName);
    }
    {
      bool isMeFromMyName = false;
      _addAgentInterpretations(condExp.thenExp, pSemanticMemory, pLingDb,
                               pParentGrammaticalType, isMeFromMyName);
    }
    if (condExp.elseExp)
    {
      bool isMeFromMyName = false;
      _addAgentInterpretations(*condExp.elseExp, pSemanticMemory, pLingDb,
                               pParentGrammaticalType, isMeFromMyName);
    }
    break;
  }
  case SemanticExpressionType::FIXEDSYNTHESIS:
  {
    FixedSynthesisExpression& fSynthExp = pSemExp->getFSynthExp();
    _addAgentInterpretations(fSynthExp.getUSemExp(), pSemanticMemory, pLingDb,
                             pParentGrammaticalType, pIsMeFromMyName);
    break;
  }
  case SemanticExpressionType::COMPARISON:
  case SemanticExpressionType::SETOFFORMS:
    break;
  }
}

}


namespace agentInterpretations
{

void addAgentInterpretations(UniqueSemanticExpression& pSemExp,
                             const SemanticMemory& pSemanticMemory,
                             const linguistics::LinguisticDatabase& pLingDb)
{
  bool isMeFromMyName = false;
  _addAgentInterpretations(pSemExp, pSemanticMemory, pLingDb,
                           GrammaticalType::UNKNOWN, isMeFromMyName);
}


} // End of namespace agentInterpretations
} // End of namespace onsem
