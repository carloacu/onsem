#include "completewithcontext.hpp"
#include <onsem/common/utility/number.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpressions.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/feedbackexpression.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticgenericgrouding.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticstatementgrounding.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticrelativetimegrounding.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/conceptset.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase.hpp>
#include <onsem/texttosemantic/tool/semexpgetter.hpp>
#include <onsem/texttosemantic/tool/semexpmodifier.hpp>
#include <onsem/semantictotext/tool/semexpcomparator.hpp>
#include <onsem/semantictotext/tool/semexpagreementdetector.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemory.hpp>
#include <onsem/semantictotext/semanticconverter.hpp>
#include "../utility/semexpcreator.hpp"

namespace onsem
{

namespace
{

void _replaceAnnotationSpecificationCoreferenceByTheActualValue(UniqueSemanticExpression& pSemExp,
                                                                const SemanticExpression* pAnnSpecPtr = nullptr)
{
  switch (pSemExp->type)
  {
  case SemanticExpressionType::ANNOTATED:
  {
    AnnotatedExpression& annExp = pSemExp->getAnnExp();
    auto itSpec = annExp.annotations.find(GrammaticalType::SPECIFIER);
    if (itSpec != annExp.annotations.end())
      pAnnSpecPtr = &*itSpec->second;
    _replaceAnnotationSpecificationCoreferenceByTheActualValue(annExp.semExp, pAnnSpecPtr);
    break;
  }
  case SemanticExpressionType::FEEDBACK:
  {
    FeedbackExpression& fdkExp = pSemExp->getFdkExp();
    _replaceAnnotationSpecificationCoreferenceByTheActualValue(fdkExp.concernedExp, pAnnSpecPtr);
    break;
  }
  case SemanticExpressionType::GROUNDED:
  {
    GroundedExpression& grdExp = pSemExp->getGrdExp();
    if (pAnnSpecPtr != nullptr)
    {
      const SemanticStatementGrounding* statGrdExpPtr = grdExp->getStatementGroundingPtr();
      if (statGrdExpPtr != nullptr &&
          statGrdExpPtr->coreference &&
          (statGrdExpPtr->coreference->getDirection() == CoreferenceDirectionEnum::ANNOTATION_SPECIFICATIONS ||
           statGrdExpPtr->coreference->getDirection() == CoreferenceDirectionEnum::PARENT))
      {
        pSemExp = pAnnSpecPtr->clone();
        break;
      }
    }
    for (auto& currChild : grdExp.children)
      _replaceAnnotationSpecificationCoreferenceByTheActualValue(currChild.second, pAnnSpecPtr);
    break;
  }
  case SemanticExpressionType::INTERPRETATION:
  {
    InterpretationExpression& intExp = pSemExp->getIntExp();
    _replaceAnnotationSpecificationCoreferenceByTheActualValue(intExp.interpretedExp, pAnnSpecPtr);
    break;
  }
  case SemanticExpressionType::LIST:
  {
    ListExpression& listExp = pSemExp->getListExp();
    for (auto& currElt : listExp.elts)
      _replaceAnnotationSpecificationCoreferenceByTheActualValue(currElt, pAnnSpecPtr);
    break;
  }
  case SemanticExpressionType::METADATA:
  {
    MetadataExpression& metaExp = pSemExp->getMetadataExp();
    _replaceAnnotationSpecificationCoreferenceByTheActualValue(metaExp.semExp, pAnnSpecPtr);
    break;
  }
  case SemanticExpressionType::FIXEDSYNTHESIS:
  case SemanticExpressionType::COMMAND:
  case SemanticExpressionType::COMPARISON:
  case SemanticExpressionType::CONDITION:
  case SemanticExpressionType::SETOFFORMS:
    break;
  }
}

void _removeFirstRequestGrdExp(GroundedExpression& pGrdExp)
{
  auto& grd = pGrdExp.grounding();
  if (grd.type == SemanticGroudingType::STATEMENT)
    grd.getStatementGrounding().requests.removeFirst();
}

void _removeFirstRequest(SemanticExpression& pSemExp)
{
  GroundedExpression* grdExpPtr = pSemExp.getGrdExpPtr_SkipWrapperPtrs();
  if (grdExpPtr != nullptr)
    _removeFirstRequestGrdExp(*grdExpPtr);
}


void _fillWithActionThatAskAboutAction
(UniqueSemanticExpression& pSemExp,
 const GroundedExpression& pContextGrdExp,
 const GroundedExpression& pGrdExpToAdd)
{
  auto grdRootExp = pGrdExpToAdd.clone();
  for (const auto& currChild : pContextGrdExp.children)
  {
    if (grdRootExp->children.find(currChild.first) == grdRootExp->children.end())
    {
      grdRootExp->children.emplace(currChild.first,
                                   currChild.second->clone());
    }
  }
  pSemExp = mystd::make_unique<InterpretationExpression>
      (InterpretationSource::RECENTCONTEXT,
       UniqueSemanticExpression(std::move(grdRootExp)),
       std::move(pSemExp));
}


UniqueSemanticExpression _copyGrdExpForAChild(GrammaticalType pGramType,
                                              const GroundedExpression& pGrdExp)
{
  if (pGramType == GrammaticalType::TIME &&
      pGrdExp.children.empty())
  {
    auto* genGrdPtr = pGrdExp->getGenericGroundingPtr();
    if (genGrdPtr != nullptr &&
        genGrdPtr->quantity.type == SemanticQuantityType::NUMBER &&
        hasNotMoreThanANumberOfDigits(genGrdPtr->quantity.nb, 4))
    {
      auto res = mystd::make_unique<SemanticTimeGrounding>();
      res->date.year.emplace(genGrdPtr->quantity.nb);
      return mystd::make_unique<GroundedExpression>(std::move(res));
    }
  }
  return pGrdExp.clone();
}


void _addChild
(UniqueSemanticExpression& pSemExp,
 const GroundedExpression& pContextGrdExp,
 SemanticRequestType pContextRequest,
 const GroundedExpression& pGrdExpToAdd,
 const linguistics::LinguisticDatabase& pLingDb)
{
  GrammaticalType childTypeToAnswerContext = semanticRequestType_toSemGram(pContextRequest);

  // Check that pGrdExpToAdd can be added has a child of this type
  if (childTypeToAnswerContext == GrammaticalType::MANNER)
  {
    if (!ConceptSet::haveAConceptThatBeginWith(pGrdExpToAdd->concepts, "manner_"))
      return;
  }

  auto grdRootExp = pContextGrdExp.clone();
  _removeFirstRequestGrdExp(*grdRootExp);

  bool childAdded = false;
  auto itChild = grdRootExp->children.find(childTypeToAnswerContext);
  if (itChild != grdRootExp->children.end())
  {
    auto& semExpToReplace = SemExpGetter::getDirectObjectOrIdentityRecursively(*itChild->second);
    auto* grdExpToReplacePtr = semExpToReplace.getGrdExpPtr_SkipWrapperPtrs();
    if (grdExpToReplacePtr != nullptr)
    {
      auto* genGrdToReplacePtr = grdExpToReplacePtr->grounding().getGenericGroundingPtr();
      if (genGrdToReplacePtr != nullptr)
      {
        auto copyOfSemExpToAdd = pGrdExpToAdd.clone();
        if (SemExpModifier::putItAdjectival(*copyOfSemExpToAdd, pLingDb))
        {
          genGrdToReplacePtr->referenceType = SemanticReferenceType::DEFINITE;
          SemExpModifier::addChild(*grdExpToReplacePtr, GrammaticalType::SPECIFIER,
                                   std::move(copyOfSemExpToAdd));
          childAdded = true;
        }
      }
    }
  }
  if (!childAdded)
  {
    grdRootExp->children.erase(childTypeToAnswerContext);
    grdRootExp->children.emplace(childTypeToAnswerContext,
                                 _copyGrdExpForAChild(childTypeToAnswerContext, pGrdExpToAdd));
  }

  pSemExp = mystd::make_unique<InterpretationExpression>
      (InterpretationSource::RECENTCONTEXT,
       UniqueSemanticExpression(std::move(grdRootExp)),
       std::move(pSemExp));
}


void _replaceYesOrNoQuestionAccordingToTheAgreement
(UniqueSemanticExpression& pSemExp,
 const SemanticExpression& pContextSemExp,
 TruenessValue pAgreementVal)
{
  static const std::set<SemanticExpressionType> expressionTypesToSkip =
  {SemanticExpressionType::METADATA, SemanticExpressionType::FIXEDSYNTHESIS};
  UniqueSemanticExpression newSemExp = pContextSemExp.clone(nullptr, true, &expressionTypesToSkip);
  _removeFirstRequest(*newSemExp);
  _replaceAnnotationSpecificationCoreferenceByTheActualValue(newSemExp);
  if (pAgreementVal == TruenessValue::VAL_FALSE)
    SemExpModifier::invertPolarity(*newSemExp);
  SemExpCreator::replaceSemExpOrAddInterpretation(InterpretationSource::YES_NO_REPLACEMENT,
                                                  pSemExp, std::move(newSemExp));
}


bool _checkAgreementAndReplaceYesOrNoQuestion
(UniqueSemanticExpression& pSemExp,
 const SemanticExpression& pContextSemExp,
 const GroundedExpression& pCurrGrdExp)
{
  TruenessValue agreementVal =
      semExpAgreementDetector::getAgreementValue(pCurrGrdExp);
  if (agreementVal != TruenessValue::UNKNOWN)
  {
    _replaceYesOrNoQuestionAccordingToTheAgreement(pSemExp, pContextSemExp, agreementVal);
    return true;
  }
  return false;
}

UniqueSemanticExpression _generateSentenceWhatHappenedJustAfterThat()
{
  auto rootGrdExp = mystd::make_unique<GroundedExpression>
      ([]()
  {
    auto statementGrd = mystd::make_unique<SemanticStatementGrounding>();
    statementGrd->verbTense = SemanticVerbTense::PUNCTUALPAST;
    statementGrd->concepts["verb_action"] = 4;
    statementGrd->requests.set(SemanticRequestType::OBJECT);
    statementGrd->word.setContent(SemanticLanguageEnum::ENGLISH, "happen", PartOfSpeech::VERB);
    return statementGrd;
  }());

  rootGrdExp->children.emplace(GrammaticalType::TIME, []()
  {
    auto relTimeGrdExp = mystd::make_unique<GroundedExpression>
        (mystd::make_unique<SemanticRelativeTimeGrounding>(SemanticRelativeTimeType::JUSTAFTER));
    relTimeGrdExp->children.emplace(GrammaticalType::SPECIFIER,
                                    mystd::make_unique<GroundedExpression>
                                    (SemanticGenericGrounding::makeThingThatHasToBeCompletedFromContext()));
    return relTimeGrdExp;
  }());
  return std::move(rootGrdExp);
}

bool _tryToCompleteAnswerWithTheQuestion(
    UniqueSemanticExpression& pSemExp,
    const SemanticExpression& pContextSemExp,
    const GroundedExpression& pContextGrdExp,
    const SemanticStatementGrounding& pContextStatementGr,
    SemanticRequestType pContextRequest,
    const linguistics::LinguisticDatabase& pLingDb);

bool _tryToCompleteAnswerWithTheQuestionFromGrdExp
(UniqueSemanticExpression& pSemExp,
 GroundedExpression& pGrdExp,
 const SemanticExpression& pContextSemExp,
 const GroundedExpression& pContextGrdExp,
 const SemanticStatementGrounding& pContextStatementGr,
 SemanticRequestType pContextRequest,
 const linguistics::LinguisticDatabase& pLingDb)
{
  switch (pGrdExp->type)
  {
  case  SemanticGroudingType::STATEMENT:
  {
    const SemanticStatementGrounding& answStatement = pGrdExp->getStatementGrounding();
    if (!answStatement.requests.empty())
      return false;

    // if fill a yes or no question
    if (answStatement.concepts.count("mentalState_know") > 0 &&
        pGrdExp.children.count(GrammaticalType::OBJECT) == 0)
    {
      pGrdExp.children.emplace(GrammaticalType::OBJECT, pContextSemExp.clone());
      return true;
    }
    else if (pContextRequest == SemanticRequestType::YESORNO)
    {
      return _checkAgreementAndReplaceYesOrNoQuestion(pSemExp, pContextSemExp, pGrdExp);
    }
    else if (answStatement.verbTense == SemanticVerbTense::UNKNOWN)
    {
      _addChild(pSemExp, pContextGrdExp, pContextRequest, pGrdExp, pLingDb);
      return true;
    }
    // if not fill a yes or no question
    else if (semanticRequestType_toSemGram(pContextRequest) != GrammaticalType::UNKNOWN &&
             pContextStatementGr.concepts.count("verb_action") > 0)
    {
      _fillWithActionThatAskAboutAction(pSemExp, pContextGrdExp, pGrdExp);
      return true;
    }
    // for sentences "it's X" the "X" should be considered has potential answer
    else if (answStatement.verbTense == pContextStatementGr.verbTense &&
             answStatement.concepts.count("verb_equal_be") > 0 &&
             pContextStatementGr.concepts.count("verb_equal_be") > 0)
    {
      auto itSubject = pGrdExp.children.find(GrammaticalType::SUBJECT);
      if (itSubject != pGrdExp.children.end() &&
          SemExpGetter::isACoreference(*itSubject->second, CoreferenceDirectionEnum::BEFORE))
      {
        auto itObject = pGrdExp.children.find(GrammaticalType::OBJECT);
        if (itObject != pGrdExp.children.end())
        {
          auto newSemExp = itObject->second->clone();
          SemExpCreator::replaceSemExpOrAddInterpretation(InterpretationSource::RESTRUCTURING,
                                                          pSemExp, std::move(newSemExp));
          _tryToCompleteAnswerWithTheQuestion(pSemExp, pContextSemExp, pContextGrdExp,
                                              pContextStatementGr, pContextRequest, pLingDb);
        }
      }
      return true;
    }
    break;
  }
  case  SemanticGroudingType::GENERIC:
  {
    if (pContextRequest == SemanticRequestType::YESORNO)
    {
      return _checkAgreementAndReplaceYesOrNoQuestion(pSemExp, pContextSemExp, pGrdExp);
    }
    else
    {
      _addChild(pSemExp, pContextGrdExp, pContextRequest, pGrdExp, pLingDb);
      return true;
    }
    break;
  }
  case SemanticGroudingType::AGENT:
  case SemanticGroudingType::NAME:
  {
    if (pContextRequest != SemanticRequestType::YESORNO &&
        pContextRequest != SemanticRequestType::TIME)
    {
      _addChild(pSemExp, pContextGrdExp, pContextRequest, pGrdExp, pLingDb);
      return true;
    }
    break;
  }
  case SemanticGroudingType::TIME:
  {
    if (pContextRequest == SemanticRequestType::TIME)
    {
      _addChild(pSemExp, pContextGrdExp, pContextRequest, pGrdExp, pLingDb);
      return true;
    }
    break;
  }
  case SemanticGroudingType::DISTANCE:
  case SemanticGroudingType::DURATION:
  case SemanticGroudingType::LANGUAGE:
  case SemanticGroudingType::RELATIVELOCATION:
  case SemanticGroudingType::RELATIVETIME:
  case SemanticGroudingType::RELATIVEDURATION:
  case SemanticGroudingType::RESOURCE:
  case SemanticGroudingType::META:
  case SemanticGroudingType::TEXT:
  case SemanticGroudingType::CONCEPTUAL:
    break;
  }
  return false;
}


bool _tryToCompleteAnswerWithTheQuestion
(UniqueSemanticExpression& pSemExp,
 const SemanticExpression& pContextSemExp,
 const GroundedExpression& pContextGrdExp,
 const SemanticStatementGrounding& pContextStatementGr,
 SemanticRequestType pContextRequest,
 const linguistics::LinguisticDatabase& pLingDb)
{
  switch (pSemExp->type)
  {
  case SemanticExpressionType::GROUNDED:
  {
    auto& grdExp = pSemExp->getGrdExp();
    return _tryToCompleteAnswerWithTheQuestionFromGrdExp(pSemExp, grdExp, pContextSemExp,
                                                         pContextGrdExp, pContextStatementGr,
                                                         pContextRequest, pLingDb);
  }
  case SemanticExpressionType::ANNOTATED:
  {
    auto& annExp = pSemExp->getAnnExp();
    return _tryToCompleteAnswerWithTheQuestion(annExp.semExp, pContextSemExp,
                                               pContextGrdExp, pContextStatementGr,
                                               pContextRequest, pLingDb);
  }
  case SemanticExpressionType::FEEDBACK:
  {
    auto& fdkExp = pSemExp->getFdkExp();
    const GroundedExpression* fdkGrdExpPtr = fdkExp.feedbackExp->getGrdExpPtr_SkipWrapperPtrs();
    if (fdkGrdExpPtr != nullptr)
    {
      if (ConceptSet::haveAConceptThatBeginWith(fdkGrdExpPtr->grounding().concepts, "engagement_"))
        break;
      if (pContextRequest == SemanticRequestType::YESORNO)
      {
        TruenessValue agreementVal =
            semExpAgreementDetector::semExpToAgreementValue(*fdkExp.feedbackExp);
        if (agreementVal != TruenessValue::UNKNOWN)
        {
          _replaceYesOrNoQuestionAccordingToTheAgreement(pSemExp, pContextSemExp, agreementVal);
          break;
        }
      }
    }
    return _tryToCompleteAnswerWithTheQuestion(fdkExp.concernedExp, pContextSemExp,
                                               pContextGrdExp, pContextStatementGr,
                                               pContextRequest, pLingDb);
  }
  case SemanticExpressionType::INTERPRETATION:
  {
    auto& intExp = pSemExp->getIntExp();
    return _tryToCompleteAnswerWithTheQuestion(intExp.interpretedExp, pContextSemExp,
                                               pContextGrdExp, pContextStatementGr,
                                               pContextRequest, pLingDb);
  }
  case SemanticExpressionType::LIST:
  {
    auto& listExp = pSemExp->getListExp();
    if (!listExp.elts.empty())
      return _tryToCompleteAnswerWithTheQuestion(listExp.elts.front(), pContextSemExp,
                                                 pContextGrdExp, pContextStatementGr,
                                                 pContextRequest, pLingDb);
    return false;
  }
  case SemanticExpressionType::METADATA:
  {
    auto& metaExp = pSemExp->getMetadataExp();
    return _tryToCompleteAnswerWithTheQuestion(metaExp.semExp, pContextSemExp,
                                               pContextGrdExp, pContextStatementGr,
                                               pContextRequest, pLingDb);

  }
  case SemanticExpressionType::FIXEDSYNTHESIS:
  case SemanticExpressionType::COMMAND:
  case SemanticExpressionType::COMPARISON:
  case SemanticExpressionType::CONDITION:
  case SemanticExpressionType::SETOFFORMS:
    return false;
  }
  return false;
}


const SemanticExpression* _getSemExpIfCompatibleWithTheEntityType(const SemanticExpression& pContextSemExp,
                                                                  const SemanticGenericGrounding& pGenGrd,
                                                                  GrammaticalType pGramParentLink,
                                                                  const SemanticExpression* pAuthorPtr,
                                                                  const SemanticExpression* pReceiverPtr,
                                                                  const SemanticMemoryBlock& pMemBlock,
                                                                  const linguistics::LinguisticDatabase& pLingDb)
{
  if (pAuthorPtr != nullptr && SemExpComparator::semExpsAreEqual(pContextSemExp, *pAuthorPtr, pMemBlock, pLingDb))
    return nullptr;
  if (pReceiverPtr != nullptr && SemExpComparator::semExpsAreEqual(pContextSemExp, *pReceiverPtr, pMemBlock, pLingDb))
    return nullptr;

  const GroundedExpression* grdExpPtr = pContextSemExp.getGrdExpPtr_SkipWrapperPtrs();
  if (grdExpPtr != nullptr)
  {
    const SemanticGrounding& contextGrd = grdExpPtr->grounding();
    const SemanticGenericGrounding* contextObjGenGr = contextGrd.getGenericGroundingPtr();
    if (contextObjGenGr != nullptr)
    {
      if (!contextObjGenGr->coreference &&
          doesSemanticEntityTypeCanBeCompatible(contextObjGenGr->entityType, pGenGrd.entityType) &&
          SemExpComparator::getQuantityImbrication(contextObjGenGr->quantity, pGenGrd.quantity) == ImbricationType::EQUALS &&
          contextObjGenGr->referenceType == SemanticReferenceType::DEFINITE)
        return &pContextSemExp;
    }
    else if (contextGrd.type == SemanticGroudingType::STATEMENT)
    {
      if (pGenGrd.concepts.empty() &&
          pGramParentLink != GrammaticalType::LOCATION &&
          (pGenGrd.entityType == SemanticEntityType::SENTENCE ||
           pGenGrd.entityType == SemanticEntityType::THING))
        return &pContextSemExp;
      return nullptr;
    }
    else if (!pGenGrd.quantity.isPlural())
    {
      if (pGenGrd.entityType == SemanticEntityType::AGENTORTHING ||
          pGenGrd.entityType == SemanticEntityType::HUMAN ||
          pGenGrd.entityType == SemanticEntityType::SENTENCE)
      {
        if (contextGrd.type == SemanticGroudingType::NAME ||
            contextGrd.type == SemanticGroudingType::AGENT ||
            contextGrd.type == SemanticGroudingType::RESOURCE)
          return &pContextSemExp;
      }
      else
      {
        if (contextGrd.type == SemanticGroudingType::NAME ||
            contextGrd.type == SemanticGroudingType::AGENT)
          return nullptr;
        const auto* contextStatGrdPtr = contextGrd.getStatementGroundingPtr();
        if (contextStatGrdPtr != nullptr &&
            !contextStatGrdPtr->requests.empty())
          return nullptr;
        return &pContextSemExp;
      }
    }
  }
  return nullptr;
}



void _tryToCompleteInputQuestion(UniqueSemanticExpression& pSemExp,
                                 const GroundedExpression& pContextGrdExp)
{
  GroundedExpression* currGrdExp = pSemExp->getGrdExpPtr_SkipWrapperPtrs();
  if (currGrdExp != nullptr &&
      currGrdExp->grounding().type == SemanticGroudingType::STATEMENT &&
      currGrdExp->children.empty())
  {
    const SemanticStatementGrounding& statGrd = (*currGrdExp)->getStatementGrounding();
    if (statGrd.word.lemma.empty() &&
        !statGrd.requests.empty())
    {
      const SemanticStatementGrounding* contextStatement = pContextGrdExp->getStatementGroundingPtr();
      if (contextStatement != nullptr &&
          contextStatement->requests.empty())
      {
        pSemExp = mystd::make_unique<InterpretationExpression>
                    (InterpretationSource::RECENTCONTEXT,
                     [&pContextGrdExp, &statGrd]
        {
          auto intGrdExp = pContextGrdExp.clone();
          SemExpModifier::addRequest(intGrdExp->getGrdExp(), statGrd.requests.firstOrNothing());
          return intGrdExp;
        }(),
                     std::move(pSemExp));
      }
    }
  }
}


bool _isAndThenListExp(const ListExpression& pListExp)
{
  return pListExp.listType == ListExpressionType::THEN &&
      pListExp.elts.empty();
}


bool _mergeSemExp
(UniqueSemanticExpression& pSemExp,
 const SemanticExpression& pContextSemExp,
 bool pSameAuthor,
 const linguistics::LinguisticDatabase& pLingDb)
{
  const GroundedExpression* contextGrdExpPtr = pContextSemExp.getGrdExpPtr_SkipWrapperPtrs();
  if (contextGrdExpPtr == nullptr)
    return false;
  const GroundedExpression& contextGrdExp = *contextGrdExpPtr;

  if (contextGrdExp->type == SemanticGroudingType::STATEMENT)
  {
    const SemanticStatementGrounding& contextStatementGr = contextGrdExp->getStatementGrounding();
    if (!pSameAuthor && !contextStatementGr.requests.empty())
    {
      return _tryToCompleteAnswerWithTheQuestion(pSemExp, pContextSemExp, contextGrdExp,
                                                 contextStatementGr,
                                                 contextStatementGr.requests.first(), pLingDb);
    }
    else
    {
      const UniqueSemanticExpression* contextCorefSemExpPtr =
          SemExpGetter::getCoreferenceAfterFromGrdExp(contextGrdExp);
      if (contextCorefSemExpPtr != nullptr)
      {
        // TODO: improve condition to now if it's ok for replacement
        auto* grdExpPtr = pSemExp->getGrdExpPtr_SkipWrapperPtrs();
        if (grdExpPtr != nullptr)
        {
          auto* genGrdPtr = grdExpPtr->grounding().getGenericGroundingPtr();
          if (genGrdPtr != nullptr)
          {
            auto& genGrd = *genGrdPtr;
            auto newContextGrdExp = contextGrdExp.clone();
            UniqueSemanticExpression* newCorefSemExpPtr =
                SemExpGetter::getCoreferenceAfterFromGrdExp(*newContextGrdExp);
            if (newCorefSemExpPtr != nullptr)
            {
              *newCorefSemExpPtr = pSemExp->clone();
              if (genGrd.referenceType == SemanticReferenceType::UNDEFINED)
              {
                auto* contextCorefGrdExpPtr = (*contextCorefSemExpPtr)->getGrdExpPtr_SkipWrapperPtrs();
                if (contextCorefGrdExpPtr != nullptr)
                {
                  auto* contextCorefGenGrdPtr = contextCorefGrdExpPtr->grounding().getGenericGroundingPtr();
                  if (contextCorefGenGrdPtr != nullptr &&
                      contextCorefGenGrdPtr->referenceType != SemanticReferenceType::UNDEFINED)
                    SemExpModifier::setReferenceTypeOfSemExp(**newCorefSemExpPtr, contextCorefGenGrdPtr->referenceType);
                }
              }
              SemExpModifier::addChildrenOfAnotherSemExp(**newCorefSemExpPtr, **contextCorefSemExpPtr, ListExpressionType::UNRELATED);
              pSemExp = mystd::make_unique<InterpretationExpression>
                  (InterpretationSource::RECENTCONTEXT, std::move(newContextGrdExp),
                   std::move(pSemExp));
            }
          }
        }
      }
      else
      {
        _tryToCompleteInputQuestion(pSemExp, contextGrdExp);
      }
    }
    return true;
  }

  if (contextGrdExp->type == SemanticGroudingType::CONCEPTUAL)
  {
    auto itObject = contextGrdExp.children.find(GrammaticalType::OBJECT);
    if (itObject == contextGrdExp.children.end())
      return false;

    auto itPurpose = contextGrdExp.children.find(GrammaticalType::PURPOSE);
    if (itPurpose == contextGrdExp.children.end())
      return false;

    std::list<const GroundedExpression*> grdExpPtrs;
    pSemExp->getGrdExpPtrs_SkipWrapperLists(grdExpPtrs);
    bool hasATeachingElt = false;
    bool firstIteration = true;
    bool isNotATeachingList = false;
    UniqueSemanticExpression objectSemexp = itObject->second->clone();

    for (auto& currGrdExp : grdExpPtrs)
    {
      if (firstIteration)
      {
        firstIteration = false;
        if (SemExpGetter::isACoreference(*currGrdExp, CoreferenceDirectionEnum::BEFORE, true))
          continue;
      }
      if (SemExpGetter::isATeachingElement(*currGrdExp))
      {
        SemExpModifier::addNewSemExp(objectSemexp, currGrdExp->clone(), ListExpressionType::THEN);
        hasATeachingElt = true;
        continue;
      }
      isNotATeachingList = true;
      break;
    }

    if (hasATeachingElt && !isNotATeachingList)
    {
      SemExpCreator::replaceSemExpOrAddInterpretation(
            InterpretationSource::TEACHING_FOLLOW_UP,
            pSemExp,
            converter::constructTeachSemExp(itPurpose->second->clone(), std::move(objectSemexp)));
      return true;
    }
  }

  return false;
}


bool _canBeInTheSameEqualitySentence(const SemanticExpression& pSemExp1,
                                     const SemanticExpression& pSemExp2)
{
  const GroundedExpression* grdExp1Ptr = pSemExp1.getGrdExpPtr_SkipWrapperPtrs();
  const GroundedExpression* grdExp2Ptr = pSemExp2.getGrdExpPtr_SkipWrapperPtrs();
  if (grdExp1Ptr == nullptr || grdExp2Ptr == nullptr)
    return false;
  SemanticGroudingType grdType1 = grdExp1Ptr->grounding().type;
  SemanticGroudingType grdType2 = grdExp2Ptr->grounding().type;
  if (grdType1 == SemanticGroudingType::RESOURCE ||
      grdType2 == SemanticGroudingType::RESOURCE)
    return true;
  if (grdType1 == SemanticGroudingType::STATEMENT)
    return grdType2 == SemanticGroudingType::STATEMENT;

  SemanticEntityType objectEntityType = SemExpGetter::getEntity(pSemExp1);
  SemanticEntityType contextEntityType = SemExpGetter::getEntity(pSemExp2);
  return doesSemanticEntityTypeAreStronglyCompatible(contextEntityType, objectEntityType);
}

bool _hasToReplaceAndThenByQuestionWhatHappenedJustAfterThan
(const ListExpression& pListExp,
 const SemanticExpression& pContextSemExp,
 bool pSameAuthor)
{
  const GroundedExpression* contextGrdExp = pContextSemExp.getGrdExpPtr_SkipWrapperPtrs();
  if (contextGrdExp != nullptr &&
      contextGrdExp->grounding().type == SemanticGroudingType::STATEMENT)
  {
    const SemanticStatementGrounding& contextStatementGr = (*contextGrdExp)->getStatementGrounding();
    if (!pSameAuthor && contextStatementGr.requests.empty() &&
        _isAndThenListExp(pListExp))
      return true;
  }
  return false;
}

void _replaceRecentFromContext_fromSemExp
(UniqueSemanticExpression& pSemExp,
 GrammaticalType pGramParentLink,
 const GroundedExpression* pFatherGrdExpOfStatementPtr,
 const SemanticExpression& pContextSemExp,
 bool pSameAuthor,
 const SemanticExpression* pAuthorPtr,
 const SemanticExpression* pReceiverPtr,
 const SemanticMemoryBlock& pMemBlock,
 const linguistics::LinguisticDatabase& pLingDb)
{
  switch (pSemExp->type)
  {
  case SemanticExpressionType::GROUNDED:
  {
    GroundedExpression& grdExp = pSemExp->getGrdExp();
    switch (grdExp->type)
    {
    case SemanticGroudingType::GENERIC:
    {
      const SemanticGenericGrounding& genGrd = grdExp->getGenericGrounding();
      if (genGrd.coreference &&
          genGrd.coreference->getDirection() == CoreferenceDirectionEnum::BEFORE &&
          (genGrd.referenceType != SemanticReferenceType::DEFINITE || genGrd.word.isEmpty())) // TODO: hack to remove
      {
        const SemanticExpression* contextToAddPtr =
            _getSemExpIfCompatibleWithTheEntityType(pContextSemExp, genGrd, pGramParentLink, pAuthorPtr,
                                                    pReceiverPtr, pMemBlock, pLingDb);

        if (contextToAddPtr == nullptr)
        {
          const GroundedExpression* contextGrdExpPtr = pContextSemExp.getGrdExpPtr_SkipWrapperPtrs();
          if (contextGrdExpPtr != nullptr)
          {
            const SemanticExpression* contextObjSemExp =
                SemExpGetter::getChildFromSemExp(*contextGrdExpPtr, GrammaticalType::OBJECT);
            if (contextObjSemExp != nullptr)
              contextToAddPtr = _getSemExpIfCompatibleWithTheEntityType(*contextObjSemExp, genGrd, pGramParentLink,
                                                                        pAuthorPtr, pReceiverPtr, pMemBlock, pLingDb);
            if (contextToAddPtr == nullptr)
            {
              const SemanticExpression* contextSubjSemExp =
                  SemExpGetter::getChildFromSemExp(*contextGrdExpPtr, GrammaticalType::SUBJECT);
              if (contextSubjSemExp != nullptr)
                contextToAddPtr = _getSemExpIfCompatibleWithTheEntityType(*contextSubjSemExp, genGrd, pGramParentLink,
                                                                          pAuthorPtr, pReceiverPtr, pMemBlock, pLingDb);
            }
          }
        }

        if (contextToAddPtr != nullptr)
        {
          if (pGramParentLink == GrammaticalType::SUBJECT &&
              genGrd.entityType == SemanticEntityType::THING &&
              pFatherGrdExpOfStatementPtr != nullptr)
          {
            const SemanticStatementGrounding* fatherStatementPtr =
                pFatherGrdExpOfStatementPtr->grounding().getStatementGroundingPtr();
            if (fatherStatementPtr != nullptr &&
                fatherStatementPtr->concepts.count("verb_equal_be") > 0)
            {
              auto itObject = pFatherGrdExpOfStatementPtr->children.find(GrammaticalType::OBJECT);
              if (itObject != pFatherGrdExpOfStatementPtr->children.end() &&
                  !_canBeInTheSameEqualitySentence(*itObject->second, *contextToAddPtr))
                contextToAddPtr = nullptr;
            }
          }

          if (contextToAddPtr != nullptr)
          {
            static const std::set<SemanticExpressionType> expressionTypesToSkip = {SemanticExpressionType::SETOFFORMS};
            auto contextGrdExpCloned = contextToAddPtr->clone(nullptr, false, &expressionTypesToSkip);
            SemExpModifier::removeSpecificationsNotNecessaryForAnAnswerFromSemExp(*contextGrdExpCloned);
            pSemExp = mystd::make_unique<InterpretationExpression>
                (InterpretationSource::RECENTCONTEXT,
                 std::move(contextGrdExpCloned),
                 std::move(pSemExp));
            return;
          }
        }
      }
      break;
    }
    case SemanticGroudingType::STATEMENT:
    {
      SemanticStatementGrounding& statGrd = grdExp->getStatementGrounding();
      pFatherGrdExpOfStatementPtr = &grdExp;
      if (statGrd.coreference)
      {
        const GroundedExpression* contextGrdExpPtr = SemExpGetter::getLastGrdExpPtr(pContextSemExp);
        if (contextGrdExpPtr != nullptr)
        {
          const SemanticStatementGrounding* contextStatementGrdPtr = contextGrdExpPtr->grounding().getStatementGroundingPtr();
          if (contextStatementGrdPtr != nullptr)
          {
            SemanticRequests statementRequests;
            statGrd.requests.swap(statementRequests);
            auto grdExpCopied = grdExp.clone();
            grdExpCopied->moveGrounding(contextGrdExpPtr->cloneGrounding());
            SemExpModifier::swapRequests(*grdExpCopied, statementRequests);
            for (const auto& currChildFroContext : contextGrdExpPtr->children)
              SemExpModifier::addChild(*grdExpCopied, currChildFroContext.first,
                                       currChildFroContext.second->clone());
            pSemExp = mystd::make_unique<InterpretationExpression>
                (InterpretationSource::RECENTCONTEXT,
                 std::move(grdExpCopied),
                 std::move(pSemExp));
            return;
          }
        }
        return;
      }
      break;
    }
    case SemanticGroudingType::RELATIVETIME:
    {
      auto itSpecificationChild = grdExp.children.find(GrammaticalType::SPECIFIER);
      if (itSpecificationChild != grdExp.children.end())
      {
        const GroundedExpression* specificationGrdExpPtr = itSpecificationChild->second->getGrdExpPtr_SkipWrapperPtrs();
        if (specificationGrdExpPtr != nullptr)
        {
          const SemanticGenericGrounding* specificationGenGrdPtr = specificationGrdExpPtr->grounding().getGenericGroundingPtr();
          if (specificationGenGrdPtr != nullptr &&
              specificationGenGrdPtr->coreference)
          {
            const SemanticExpression* timeContextInfo = SemExpGetter::getTimeInfo(pContextSemExp);
            if (timeContextInfo != nullptr)
            {
              auto interpretation = mystd::make_unique<InterpretationExpression>
                  (InterpretationSource::RECENTCONTEXT,
                   timeContextInfo->clone(), std::move(itSpecificationChild->second));
              itSpecificationChild->second = std::move(interpretation);
              return;
            }
          }
        }
      }
      break;
    }
    default:
      break;
    }

    for (auto& currChild : grdExp.children)
    {
      if (currChild.first == GrammaticalType::OTHER_THAN)
      {
        pAuthorPtr = nullptr;
      }
      _replaceRecentFromContext_fromSemExp(currChild.second, currChild.first, pFatherGrdExpOfStatementPtr, pContextSemExp, pSameAuthor,
                                           pAuthorPtr, pReceiverPtr, pMemBlock, pLingDb);
    }
    return;
  }
  case SemanticExpressionType::LIST:
  {
    ListExpression& listExp = pSemExp->getListExp();
    if (_hasToReplaceAndThenByQuestionWhatHappenedJustAfterThan(listExp, pContextSemExp, pSameAuthor))
    {
      SemExpCreator::replaceSemExpOrAddInterpretation(
            InterpretationSource::ANDTHEN,
            pSemExp,
            _generateSentenceWhatHappenedJustAfterThat());
      _replaceRecentFromContext_fromSemExp(pSemExp, pGramParentLink, pFatherGrdExpOfStatementPtr,  pContextSemExp, pSameAuthor,
                                           pAuthorPtr, pReceiverPtr, pMemBlock, pLingDb);
    }
    else
    {
      for (auto& currElt : listExp.elts)
        _replaceRecentFromContext_fromSemExp(currElt, pGramParentLink, pFatherGrdExpOfStatementPtr,  pContextSemExp, pSameAuthor,
                                             pAuthorPtr, pReceiverPtr, pMemBlock, pLingDb);
    }
    return;
  }
  case SemanticExpressionType::CONDITION:
  {
    ConditionExpression& condExp = pSemExp->getCondExp();
    _replaceRecentFromContext_fromSemExp(condExp.conditionExp, GrammaticalType::UNKNOWN,
                                         pFatherGrdExpOfStatementPtr, pContextSemExp, pSameAuthor,
                                         pAuthorPtr, pReceiverPtr, pMemBlock, pLingDb);
    _replaceRecentFromContext_fromSemExp(condExp.thenExp, GrammaticalType::UNKNOWN,
                                         pFatherGrdExpOfStatementPtr, pContextSemExp, pSameAuthor,
                                         pAuthorPtr, pReceiverPtr, pMemBlock, pLingDb);
    if (condExp.elseExp)
    {
      _replaceRecentFromContext_fromSemExp(*condExp.elseExp, GrammaticalType::UNKNOWN,
                                           pFatherGrdExpOfStatementPtr, pContextSemExp, pSameAuthor,
                                           pAuthorPtr, pReceiverPtr, pMemBlock, pLingDb);
    }
    return;
  }
  case SemanticExpressionType::COMPARISON:
  {
    ComparisonExpression& compExp = pSemExp->getCompExp();
    if (compExp.whatIsComparedExp)
    {
      _replaceRecentFromContext_fromSemExp(*compExp.whatIsComparedExp, GrammaticalType::UNKNOWN,
                                           pFatherGrdExpOfStatementPtr, pContextSemExp, pSameAuthor,
                                           pAuthorPtr, pReceiverPtr, pMemBlock, pLingDb);
    }
    _replaceRecentFromContext_fromSemExp(compExp.leftOperandExp, GrammaticalType::UNKNOWN,
                                         pFatherGrdExpOfStatementPtr, pContextSemExp, pSameAuthor,
                                         pAuthorPtr, pReceiverPtr, pMemBlock, pLingDb);
    if (compExp.rightOperandExp)
    {
      _replaceRecentFromContext_fromSemExp(*compExp.rightOperandExp, GrammaticalType::UNKNOWN,
                                           pFatherGrdExpOfStatementPtr, pContextSemExp, pSameAuthor,
                                           pAuthorPtr, pReceiverPtr, pMemBlock, pLingDb);
    }
    return;
  }
  case SemanticExpressionType::INTERPRETATION:
  {
    InterpretationExpression& intExp = pSemExp->getIntExp();
    _replaceRecentFromContext_fromSemExp(intExp.interpretedExp, pGramParentLink, pFatherGrdExpOfStatementPtr, pContextSemExp, pSameAuthor,
                                         pAuthorPtr, pReceiverPtr, pMemBlock, pLingDb);
    return;
  }
  case SemanticExpressionType::FEEDBACK:
  {
    FeedbackExpression& fdkExp = pSemExp->getFdkExp();
    _replaceRecentFromContext_fromSemExp(fdkExp.concernedExp, pGramParentLink, pFatherGrdExpOfStatementPtr, pContextSemExp, pSameAuthor,
                                         pAuthorPtr, pReceiverPtr, pMemBlock, pLingDb);
    return;
  }
  case SemanticExpressionType::ANNOTATED:
  {
    AnnotatedExpression& annExp = pSemExp->getAnnExp();
    _replaceRecentFromContext_fromSemExp(annExp.semExp, pGramParentLink, pFatherGrdExpOfStatementPtr, pContextSemExp, pSameAuthor,
                                         pAuthorPtr, pReceiverPtr, pMemBlock, pLingDb);
    return;
  }
  case SemanticExpressionType::METADATA:
  {
    MetadataExpression& metadataExp = pSemExp->getMetadataExp();
    pReceiverPtr = metadataExp.getReceiverSemExpPtr();
    _replaceRecentFromContext_fromSemExp(metadataExp.semExp, pGramParentLink, pFatherGrdExpOfStatementPtr, pContextSemExp, pSameAuthor,
                                         pAuthorPtr, pReceiverPtr, pMemBlock, pLingDb);
    return;
  }
  case SemanticExpressionType::FIXEDSYNTHESIS:
  case SemanticExpressionType::COMMAND:
  case SemanticExpressionType::SETOFFORMS:
    return;
  }
  assert(false);
}


}


void completeWithContext(UniqueSemanticExpression& pSemExp,
                         GrammaticalType pGramParentLink,
                         const SemanticExpression& pContextSemExp,
                         bool pSameAuthor,
                         const SemanticExpression* pAuthorPtr,
                         const SemanticMemoryBlock& pMemBlock,
                         const linguistics::LinguisticDatabase& pLingDb)
{
  const ListExpression* listExp = pContextSemExp.getListExpPtr_SkipWrapperPtrs();
  if (listExp != nullptr &&
      listExp->listType == ListExpressionType::UNRELATED)
  {
    auto itExp = listExp->elts.end();
    bool alreadyReplaceRecentFromContext = false;
    while (itExp != listExp->elts.begin())
    {
      --itExp;
      if (!alreadyReplaceRecentFromContext)
      {
        _replaceRecentFromContext_fromSemExp(pSemExp, pGramParentLink,
                                             nullptr, **itExp, pSameAuthor, pAuthorPtr, nullptr, pMemBlock, pLingDb);
        alreadyReplaceRecentFromContext = true;
      }

      if (_mergeSemExp(pSemExp, **itExp, pSameAuthor, pLingDb))
        break;
    }
  }
  else
  {
    _replaceRecentFromContext_fromSemExp(pSemExp, pGramParentLink,
                                         nullptr, pContextSemExp, pSameAuthor, pAuthorPtr, nullptr, pMemBlock, pLingDb);
    _mergeSemExp(pSemExp, pContextSemExp, pSameAuthor, pLingDb);
  }
}


} // End of namespace onsem
