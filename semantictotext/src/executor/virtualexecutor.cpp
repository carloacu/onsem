#include <onsem/semantictotext/executor/virtualexecutor.hpp>
#include <future>
#include <sstream>
#include <onsem/texttosemantic/dbtype/semanticexpressions.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semantictimegrounding.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticresourcegrounding.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticstatementgrounding.hpp>
#include <onsem/texttosemantic/tool/semexpmodifier.hpp>
#include <onsem/texttosemantic/tool/semexpgetter.hpp>
#include <onsem/common/utility/random.hpp>
#include <onsem/semantictotext/semanticmemory/semantictracker.hpp>
#include <onsem/semantictotext/semexpoperators.hpp>
#include "../linguisticsynthesizer/linguisticsynthesizer.hpp"


namespace onsem
{


VirtualExecutor::VirtualExecutor
(SemanticSourceEnum pHowTheTextWillBeExposed,
 VirtualExecutorLogger* pLogOnSynchronousExecutionCasePtr)
  : _typeOfExecutor(pHowTheTextWillBeExposed),
    _syncLoggerPtr(pLogOnSynchronousExecutionCasePtr)
{
}


std::string VirtualExecutor::linkToStr(Link pLink)
{
  switch (pLink) {
  case Link::AND:
    return "AND";
  case Link::THEN:
    return "THEN";
  case Link::THEN_REVERSED:
    return "THEN_REVERSED";
  case Link::IN_BACKGROUND:
    return "IN_BACKGROUND";
  }
  return "";
}


FutureVoid VirtualExecutor::_sayAndAddDescriptionTree(
    const UniqueSemanticExpression& pUSemExp,
    std::shared_ptr<ExecutorContext> pExecutorContext,
    SemanticSourceEnum pFrom,
    ContextualAnnotation pContextualAnnotation)
{
  IndexToSubNameToParameterValue params;
  params[0][""] =
      std::make_unique<ReferenceOfSemanticExpressionContainer>(*pUSemExp);
  auto descExp = MetadataExpression::constructSourceFromSourceEnumInPresent
      (SemanticAgentGrounding::getRobotAgentPtr(),
       pFrom, pContextualAnnotation);
  static const std::set<SemanticExpressionType> expressionTypesToSkip =
  {SemanticExpressionType::ANNOTATED, SemanticExpressionType::SETOFFORMS};
  descExp = descExp->clone(&params, true, &expressionTypesToSkip);
  _assertPunctually(*descExp);
  auto descExpWithMetadata = std::make_shared<std::unique_ptr<MetadataExpression>>
      (std::make_unique<MetadataExpression>(SemanticSourceEnum::SEMREACTION,
                                              std::move(descExp), pUSemExp->clone()));
  (*descExpWithMetadata)->contextualAnnotation = pContextualAnnotation;

  auto exposureTime = std::make_shared<std::unique_ptr<SemanticTimeGrounding>>
      (SemanticTimeGrounding::nowInstance());

  std::list<std::unique_ptr<SynthesizerResult>> synthesizerResults;
  _convertToText(synthesizerResults, pUSemExp, pExecutorContext->textProcContext);
  FutureVoid exposeSynthesizedResultsFuture;

  for (const auto& currResult : synthesizerResults)
  {
    switch (currResult->type)
    {
    case SynthesizerResultEnum::TEXT:
    {
      const auto& syntText = *dynamic_cast<const SynthesizerText*>(&*currResult);
      exposeSynthesizedResultsFuture = exposeSynthesizedResultsFuture.then
          ([=] { return _exposeText(syntText.text, pExecutorContext->textProcContext.langType); });
      break;
    }
    case SynthesizerResultEnum::TASK:
    {
      const auto& syntTask = *dynamic_cast<const SynthesizerTask*>(&*currResult);
      exposeSynthesizedResultsFuture = exposeSynthesizedResultsFuture.then
          ([=] { return _exposeResource(syntTask.resource, pExecutorContext->inputSemExpPtr); });
      break;
    }
    }
  }

  return exposeSynthesizedResultsFuture.thenVoid([this, exposureTime, descExpWithMetadata]
  {
    (*exposureTime)->setEndOfThisTimeNow();

    if ((*descExpWithMetadata)->source)
      SemExpModifier::putInPastWithTimeAnnotation(*(*descExpWithMetadata)->source, std::move(*exposureTime));
    _teachInformation(std::move(*descExpWithMetadata));
  });
}



FutureVoid VirtualExecutor::_sayWithAnnotations(
    const UniqueSemanticExpression& pUSemExp,
    std::shared_ptr<ExecutorContext> pExecutorContext,
    SemanticSourceEnum pFrom,
    ContextualAnnotation pContextualAnnotation)
{
  if (pExecutorContext->annotations->empty())
    return _sayAndAddDescriptionTree(pUSemExp, pExecutorContext, pFrom, pContextualAnnotation);

  auto annExp = std::make_unique<AnnotatedExpression>(pUSemExp->clone());
  for (const auto& currAnnotation : *pExecutorContext->annotations)
    annExp->annotations.emplace(currAnnotation.first, currAnnotation.second->clone());
  UniqueSemanticExpression uSemExp(std::move(annExp));
  return _sayAndAddDescriptionTree(uSemExp, pExecutorContext, pFrom, pContextualAnnotation);
}



FutureVoid VirtualExecutor::_handleAndList(
    const ListExpression& pListExp,
    std::shared_ptr<ExecutorContext> pExecutorContext)
{
  std::list<FutureVoid> futuresToWait;
  _beginOfScope();
  bool firstIteration = true;
  for (auto& currElt : pListExp.elts)
  {
    if (!firstIteration)
      _insideScopeLink(Link::AND);
    futuresToWait.emplace_back(_runSemExp(currElt, pExecutorContext));
    firstIteration = false;
  }
  _endOfScope();
  return FutureVoid(futuresToWait);
}



FutureVoid VirtualExecutor::_handleThenList(
    const ListExpression& pListExp,
    std::shared_ptr<ExecutorContext> pExecutorContext)
{
  FutureVoid res;
  _beginOfScope();
  bool firstIteration = true;
  for (auto& currElt : pListExp.elts)
  {
    if (firstIteration)
    {
      res = res.then([this, &currElt, pExecutorContext]() mutable
      {
        return _runSemExp(currElt, pExecutorContext);
      });
      firstIteration = false;
    }
    else
    {
      res = res.then([this, &currElt, pExecutorContext]() mutable
      {
        _insideScopeLink(Link::THEN);
        return _runSemExp(currElt, pExecutorContext);
      });
    }
  }
  _endOfScope();
  return res;
}


FutureVoid VirtualExecutor::_handleThenReversedList(
    const ListExpression& pListExp,
    std::shared_ptr<ExecutorContext> pExecutorContext)
{
  FutureVoid res;
  _beginOfScope();
  bool firstIteration = true;
  for (auto it = pListExp.elts.rbegin(); it != pListExp.elts.rend(); ++it)
  {
    auto& currElt = *it;
    if (firstIteration)
    {
      res = res.then([this, &currElt, pExecutorContext]() mutable
      {
        return _runSemExp(currElt, pExecutorContext);
      });
      firstIteration = false;
    }
    else
    {
      res = res.then([this, &currElt, pExecutorContext]() mutable
      {
        _insideScopeLink(Link::THEN_REVERSED);
        return _runSemExp(currElt, pExecutorContext);
      });
    }
  }
  _endOfScope();
  return res;
}



FutureVoid VirtualExecutor::_runConditionExp(
    const ConditionExpression& pCondExp,
    std::shared_ptr<ExecutorContext> pExecutorContext)
{
  /*
  return _waitUntil(*pCondExp.conditionExp).then
      ([this, &pCondExp, pExecutorContext]() mutable
  {
    return _runSemExp(pCondExp.thenExp, pExecutorContext);
  });
  */
  return FutureVoid();
}

FutureVoid VirtualExecutor::_exposeResource(const SemanticResource& pResource,
                                            const SemanticExpression*)
{
  _addLogAutoResource(pResource, {});
  return FutureVoid();
}


FutureVoid VirtualExecutor::_exposeText(const std::string& pText,
                                        SemanticLanguageEnum)
{
  _addLogAutoSaidText(pText);
  return FutureVoid();
}

void VirtualExecutor::_beginOfScope()
{
  _addLogAutoSchedulingBeginOfScope();
}

void VirtualExecutor::_insideScopeLink(Link pLink)
{
  _addLogAutoScheduling(linkToStr(pLink));
}

void VirtualExecutor::_insideScopeRepetition(int pNumberOfRepetitions)
{
  if (_syncLoggerPtr != nullptr)
  {
    std::stringstream ss;
    ss << "NUMBER_OF_TIMES: " << pNumberOfRepetitions;
    _syncLoggerPtr->onMetaInformation(ss.str());
  }
}

void VirtualExecutor::_endOfScope()
{
  _addLogAutoSchedulingEndOfScope();
}

FutureVoid VirtualExecutor::_handleDurationAnnotations(bool&,
    const AnnotatedExpression&,
    std::shared_ptr<ExecutorContext>)
{
  return FutureVoid();
}


FutureVoid VirtualExecutor::_doExecutionUntil(
    const AnnotatedExpression& pAnnExp,
    std::shared_ptr<ExecutorContext> pExecutorContext,
    std::shared_ptr<int> pLimitOfRecursions)
{
  return _runSemExp(pAnnExp.semExp, pExecutorContext)
      .then([=, &pAnnExp]() mutable
  {
    if (*pLimitOfRecursions <= 0)
      return FutureVoid();
    --*pLimitOfRecursions;
    return _doExecutionUntil(pAnnExp, pExecutorContext, pLimitOfRecursions);
  });
}


FutureVoid VirtualExecutor::_runGrdExp(
    const UniqueSemanticExpression& pUSemExp,
    std::shared_ptr<ExecutorContext> pExecutorContext)
{
  const GroundedExpression& grdExp = pUSemExp->getGrdExp();
  const SemanticResourceGrounding* resourceGrdPtr = grdExp->getResourceGroundingPtr();
  if (resourceGrdPtr != nullptr)
    return _exposeResource(resourceGrdPtr->resource, pExecutorContext->inputSemExpPtr);
  return _sayWithAnnotations(pUSemExp, pExecutorContext,
                             _typeOfExecutor, pExecutorContext->contAnnotation);
}


FutureVoid VirtualExecutor::_reportAnError(const std::string&)
{
  return FutureVoid();
}


void VirtualExecutor::_assertPunctually(const SemanticExpression& pSemExp)
{
  _usageOfMemoryAndLingDb([&](SemanticMemory& pSemanticMemory,
                          const linguistics::LinguisticDatabase& pLingDb)
  {
    memoryOperation::notifyPunctually(pSemExp, InformationType::ASSERTION,
                                      pSemanticMemory, pLingDb);
  });
}


void VirtualExecutor::_teachInformation(UniqueSemanticExpression pUSemExp)
{
  _usageOfMemoryAndLingDb([&](SemanticMemory& pSemanticMemory,
                          const linguistics::LinguisticDatabase& pLingDb)
  {
    mystd::unique_propagate_const<UniqueSemanticExpression> reaction;
    memoryOperation::teach(reaction, pSemanticMemory, std::move(pUSemExp), pLingDb,
                           memoryOperation::SemanticActionOperatorEnum::INFORMATION);
  });
}

void VirtualExecutor::_assertPermanently(UniqueSemanticExpression pUSemExp)
{
  _usageOfMemoryAndLingDb([&](SemanticMemory& pSemanticMemory,
                          const linguistics::LinguisticDatabase& pLingDb)
  {
    memoryOperation::informAxiom(std::move(pUSemExp),
                                       pSemanticMemory, pLingDb);
  });
}


void VirtualExecutor::_convertToText(
    std::list<std::unique_ptr<SynthesizerResult>>& pRes,
    const UniqueSemanticExpression& pUSemExp,
    const TextProcessingContext& pTextProcContext)
{
  _usageOfMemblock([&](const SemanticMemoryBlock& pMemBlock, const std::string& pCurrUserId)
  {
    _usageOfLingDb([&](const linguistics::LinguisticDatabase& pLingDb)
    {
      synthesize(pRes, pUSemExp->clone(), false,
                 pMemBlock, pCurrUserId, pTextProcContext, pLingDb, nullptr);
    });
  });
}


void VirtualExecutor::_usageOfMemoryAndLingDb(std::function<void(SemanticMemory&, const linguistics::LinguisticDatabase&)> pFunction)
{
  _usageOfMemory([&](SemanticMemory& pSemanticMemory)
  {
    _usageOfLingDb([&](const linguistics::LinguisticDatabase& pLingDb)
    {
      pFunction(pSemanticMemory, pLingDb);
    });
  });
}


/*
FutureVoid VirtualExecutor::_waitUntil(const SemanticExpression& pSemExp,
                                       const FutureVoid& pStopRequest)
{
  auto res = std::make_shared<PromiseVoid>();
  auto semTracker = std::make_shared<SemanticTracker>();
  auto trackerConnection = std::make_shared<mystd::observable::Connection>();
  *trackerConnection = semTracker->val.connect([this, res]
                                               (const UniqueSemanticExpression&) mutable -> void
  {
    _synchronicityWrapper([res] { res->setFinished(); });
  });
  pStopRequest.thenVoid([res]() mutable
  {
    res->setFinished();
  });
  _usageOfMemoryAndLingDb([&](SemanticMemory& pSemanticMemory,
                          const linguistics::LinguisticDatabase& pLingDb)
  {
    memoryOperation::track(pSemanticMemory, pSemExp.clone(), semTracker, pLingDb);
  });

  return FutureVoid(res).thenVoid([this, res, trackerConnection, semTracker]() mutable -> void
  {
    semTracker->val.disconnect(*trackerConnection);
    _usageOfMemoryAndLingDb([&](SemanticMemory& pSemanticMemory,
                            const linguistics::LinguisticDatabase& pLingDb)
    {
      memoryOperation::untrack(pSemanticMemory.memBloc, semTracker, pLingDb, nullptr);
    });
  });
}
*/

FutureVoid VirtualExecutor::runSemExp(
    UniqueSemanticExpression pUSemExp,
    std::shared_ptr<ExecutorContext>& pExecutorContext)
{
  auto keepSemExpAlive = std::make_shared<UniqueSemanticExpression>
      (std::move(pUSemExp));
  return _runSemExp(*keepSemExpAlive, pExecutorContext).thenVoid
      ([keepSemExpAlive] {});
}


FutureVoid VirtualExecutor::_runSemExp(
    const UniqueSemanticExpression& pUSemExp,
    std::shared_ptr<ExecutorContext> pExecutorContext)
{
  switch (pUSemExp->type)
  {
  case SemanticExpressionType::GROUNDED:
    return _runGrdExp(pUSemExp, pExecutorContext);
  case SemanticExpressionType::LIST:
  {
    if (pExecutorContext->sayOrExecute)
      return _sayWithAnnotations(pUSemExp, pExecutorContext,
                                 _typeOfExecutor, pExecutorContext->contAnnotation);

    const ListExpression& listExp = pUSemExp->getListExp();
    switch (listExp.listType)
    {
    case ListExpressionType::AND:
    {
      return _handleAndList(listExp, pExecutorContext);
    }
    case ListExpressionType::THEN:
    case ListExpressionType::UNRELATED:
    {
      return _handleThenList(listExp, pExecutorContext);
    }
    case ListExpressionType::THEN_REVERSED:
    {
      return _handleThenReversedList(listExp, pExecutorContext);
    }
    case ListExpressionType::OR:
    {
      auto itRandElt = Random::advanceConstIterator(listExp.elts);
      return _runSemExp(*itRandElt, pExecutorContext);
    }
    }
    break;
  }
  case SemanticExpressionType::ANNOTATED:
  {
    const AnnotatedExpression& annExp = pUSemExp->getAnnExp();

    auto subContext = std::make_shared<ExecutorContext>(*pExecutorContext);
    subContext->updateAnnotation(annExp.annotations);
    SemanticLanguageEnum subLanguage = SemExpGetter::getLanguage(annExp.annotations);
    if (subLanguage != SemanticLanguageEnum::UNKNOWN)
      subContext->textProcContext.langType = subLanguage;

    /*
    const SemanticExpression* untilChildPtr = SemExpGetter::getUntilChild(annExp.annotations);
    if (untilChildPtr != nullptr)
    {
      auto executionStopper = std::make_shared<PromiseVoid>();
      auto waitStopper = std::make_shared<PromiseVoid>();
      pStopRequest.thenVoid([executionStopper, waitStopper]
      {
        executionStopper->setFinished();
        waitStopper->setFinished();
      });
      auto limitOfRecursions = std::make_shared<int>(1000);
      auto res = _doExecutionUntil(annExp, subContext,
                                   FutureVoid(executionStopper), limitOfRecursions).thenVoid([waitStopper]
      {
        waitStopper->setFinished();
      });
      _waitUntil(*untilChildPtr, FutureVoid(waitStopper)).thenVoid
          ([executionStopper]() mutable
      {
        executionStopper->setFinished();
      });
      return res;
    }
    */

    {
      bool isHandled = false;
      FutureVoid res = _handleDurationAnnotations(isHandled, annExp, subContext);
      if (isHandled)
        return res;
    }

    const UniqueSemanticExpression* backgroundSemExpPtr = nullptr;
    auto itBackground = annExp.annotations.find(GrammaticalType::IN_BACKGROUND);
    if (itBackground != annExp.annotations.end())
      backgroundSemExpPtr = &itBackground->second;

    if (backgroundSemExpPtr != nullptr)
      _beginOfScope();

    int nbOfRepetitions = SemExpGetter::getNumberOfRepetitions(annExp.annotations);
    if (nbOfRepetitions > 1)
      _beginOfScope();
    auto res = _runSemExp(annExp.semExp, subContext);

    if (nbOfRepetitions > 1)
    {
      _insideScopeRepetition(nbOfRepetitions);
      _endOfScope();
    }

    if (backgroundSemExpPtr != nullptr)
    {
      _insideScopeLink(Link::IN_BACKGROUND);
      _runSemExp(*backgroundSemExpPtr, pExecutorContext);
      _endOfScope();
    }
    return res;
  }
  case SemanticExpressionType::INTERPRETATION:
  {
    auto& intExp = pUSemExp->getIntExp();
    return _runSemExp(intExp.interpretedExp, pExecutorContext);
  }
  case SemanticExpressionType::METADATA:
  {
    auto& metadataExp = pUSemExp->getMetadataExp();
    auto subContext = std::make_shared<ExecutorContext>(*pExecutorContext);
    subContext->contAnnotation = metadataExp.contextualAnnotation;
    subContext->sayOrExecute = metadataExp.contextualAnnotation != ContextualAnnotation::BEHAVIOR;
    if (metadataExp.interactionContextContainer)
    {
      _usageOfMemory([&](SemanticMemory& pSemanticMemory)
      {
        pSemanticMemory.interactionContextContainer = metadataExp.interactionContextContainer->clone();
      });
    }
    return _runSemExp(metadataExp.semExp, subContext);
  }
  case SemanticExpressionType::SETOFFORMS:
  {
    UniqueSemanticExpression* originalFrom = pUSemExp->getSetOfFormsExp().getOriginalForm();
    if (originalFrom != nullptr)
      return _runSemExp(*originalFrom, pExecutorContext);
    break;
  }
  case SemanticExpressionType::CONDITION:
  {
    if (pExecutorContext->sayOrExecute)
    {
      return _sayWithAnnotations(pUSemExp, pExecutorContext, _typeOfExecutor,
                                 pExecutorContext->contAnnotation);
    }

    auto& condExp = pUSemExp->getCondExp();
    return _runConditionExp(condExp, pExecutorContext);
  }
  case SemanticExpressionType::FEEDBACK:
  case SemanticExpressionType::COMPARISON:
    return _sayWithAnnotations(pUSemExp, pExecutorContext, _typeOfExecutor,
                               pExecutorContext->contAnnotation);
  case SemanticExpressionType::COMMAND:
  {
    auto& cmdExp = pUSemExp->getCmdExp();
    if (cmdExp.description)
    {
      const GroundedExpression* grdExpPtr = (*cmdExp.description)->getGrdExpPtr_SkipWrapperPtrs();
      if (grdExpPtr != nullptr)
      {
        const GroundedExpression& grdExp = *grdExpPtr;
        const SemanticStatementGrounding* statementPtr = grdExp->getStatementGroundingPtr();
        if (statementPtr != nullptr &&
            statementPtr->concepts.count("verb_action_say"))
        {
          auto subContext = std::make_shared<ExecutorContext>(*pExecutorContext);
          subContext->sayOrExecute = true;
          return _runSemExp(cmdExp.semExp, subContext);
        }
        auto semExpToInform = std::make_shared<UniqueSemanticExpression>
            (SemExpModifier::fromActionDescriptionToSentenceInPresentTense(grdExp));
        _assertPunctually(**semExpToInform);

        auto exposureTime = std::make_shared<std::unique_ptr<SemanticTimeGrounding>>
                                                                                     (SemanticTimeGrounding::nowInstance());
        return _runSemExp(cmdExp.semExp, pExecutorContext).thenVoid
            ([this, exposureTime, semExpToInform]
        {
          (*exposureTime)->setEndOfThisTimeNow();

          SemExpModifier::putInPastWithTimeAnnotation(*semExpToInform, std::move(*exposureTime));
          _assertPermanently(std::move(*semExpToInform));
        });
      }
    }

    return _runSemExp(cmdExp.semExp, pExecutorContext);
  }
  case SemanticExpressionType::FIXEDSYNTHESIS:
    return _sayAndAddDescriptionTree(pUSemExp, pExecutorContext, _typeOfExecutor,
                                     pExecutorContext->contAnnotation);
  }
  assert("Bad semantic expression type");
  return _reportAnError("Bad semantic expression type");
}


} // End of namespace onsem
