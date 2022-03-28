#include <onsem/semantictotext/executor/virtualexecutor.hpp>
#include <future>
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
    _syncLoggerPtr(pLogOnSynchronousExecutionCasePtr),
    _stopper(std::make_shared<PromiseVoid>())
{
}


FutureVoid VirtualExecutor::_sayAndAddDescriptionTree(
    const UniqueSemanticExpression& pUSemExp,
    std::shared_ptr<ExecutorContext> pExecutorContext,
    const FutureVoid& pStopRequest,
    SemanticSourceEnum pFrom,
    ContextualAnnotation pContextualAnnotation)
{
  IndexToSubNameToParameterValue params;
  params[0][""] =
      mystd::make_unique<ReferenceOfSemanticExpressionContainer>(*pUSemExp);
  auto descExp = MetadataExpression::constructSourceFromSourceEnumInPresent
      (SemanticAgentGrounding::getRobotAgentPtr(),
       pFrom, pContextualAnnotation);
  static const std::set<SemanticExpressionType> expressionTypesToSkip =
  {SemanticExpressionType::ANNOTATED, SemanticExpressionType::SETOFFORMS};
  descExp = descExp->clone(&params, true, &expressionTypesToSkip);
  _assertPunctually(*descExp);
  auto descExpWithMetadata = std::make_shared<std::unique_ptr<MetadataExpression>>
      (mystd::make_unique<MetadataExpression>(SemanticSourceEnum::SEMREACTION,
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
          ([=] { return _exposeText(syntText.text, pExecutorContext->textProcContext.langType, pStopRequest); });
      break;
    }
    case SynthesizerResultEnum::TASK:
    {
      const auto& syntTask = *dynamic_cast<const SynthesizerTask*>(&*currResult);
      exposeSynthesizedResultsFuture = exposeSynthesizedResultsFuture.then
          ([=] { return _exposeResource(syntTask.resource, pStopRequest); });
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
    const FutureVoid& pStopRequest,
    SemanticSourceEnum pFrom,
    ContextualAnnotation pContextualAnnotation)
{
  if (pExecutorContext->annotations->empty())
    return _sayAndAddDescriptionTree(pUSemExp, pExecutorContext, pStopRequest,
                                     pFrom, pContextualAnnotation);

  auto annExp = mystd::make_unique<AnnotatedExpression>(pUSemExp->clone());
  for (const auto& currAnnotation : *pExecutorContext->annotations)
    annExp->annotations.emplace(currAnnotation.first, currAnnotation.second->clone());
  UniqueSemanticExpression uSemExp(std::move(annExp));
  return _sayAndAddDescriptionTree(uSemExp, pExecutorContext, pStopRequest,
                                   pFrom, pContextualAnnotation);
}



FutureVoid VirtualExecutor::_handleAndList(
    ListExpression& pListExp,
    std::shared_ptr<ExecutorContext> pExecutorContext,
    const FutureVoid& pStopRequest)
{
  std::list<FutureVoid> futuresToWait;
  _addLogAutoSchedulingBeginOfScope();
  bool firstIteration = true;
  for (auto& currElt : pListExp.elts)
  {
    if (!firstIteration)
      _addLogAutoScheduling("AND");
    futuresToWait.emplace_back(_runSemExp(currElt, pExecutorContext, pStopRequest));
    firstIteration = false;
  }
  _addLogAutoSchedulingEndOfScope();
  return FutureVoid(futuresToWait);
}



FutureVoid VirtualExecutor::_handleThenList(ListExpression& pListExp,
    std::shared_ptr<ExecutorContext> pExecutorContext,
    const FutureVoid& pStopRequest)
{
  FutureVoid res;
  _addLogAutoSchedulingBeginOfScope();
  bool firstIteration = true;
  for (auto& currElt : pListExp.elts)
  {
    if (firstIteration)
    {
      res = res.then([this, &currElt, pExecutorContext, pStopRequest]() mutable
      {
        return _runSemExp(currElt, pExecutorContext, pStopRequest);
      });
      firstIteration = false;
    }
    else
    {
      res = res.then([this, &currElt, pExecutorContext, pStopRequest]() mutable
      {
        _addLogAutoScheduling("THEN");
        return _runSemExp(currElt, pExecutorContext, pStopRequest);
      });
    }
  }
  _addLogAutoSchedulingEndOfScope();
  return res;
}


FutureVoid VirtualExecutor::_runConditionExp(
    ConditionExpression& pCondExp,
    std::shared_ptr<ExecutorContext> pExecutorContext,
    const FutureVoid& pStopRequest)
{
  return _waitUntil(*pCondExp.conditionExp, pStopRequest).then
      ([this, &pCondExp, pExecutorContext, pStopRequest]() mutable
  {
    if (pStopRequest.isFinished())
      return FutureVoid();
    return _runSemExp(pCondExp.thenExp, pExecutorContext, pStopRequest);
  });
}

FutureVoid VirtualExecutor::_exposeResource(const SemanticResource& pResource,
                                            const FutureVoid&)
{
  _addLogAutoResource(pResource);
  return FutureVoid();
}


FutureVoid VirtualExecutor::_exposeText(
    const std::string& pText,
    SemanticLanguageEnum,
    const FutureVoid&)
{
  _addLogAutoSaidText(pText);
  return FutureVoid();
}

FutureVoid VirtualExecutor::_handleDurationAnnotations(
    bool&,
    const AnnotatedExpression&,
    std::shared_ptr<ExecutorContext>,
    const FutureVoid&)
{
  return FutureVoid();
}


FutureVoid VirtualExecutor::_runSemExpNTimes(
    UniqueSemanticExpression& pSemExp,
    std::shared_ptr<ExecutorContext> pExecutorContext,
    const FutureVoid& pStopRequest,
    int pNbOfTimes)
{
  FutureVoid res;
  for (int i = 0; i < pNbOfTimes; ++i)
  {
    if (pStopRequest.isFinished())
      return FutureVoid();
    res = res.then([this, &pSemExp, pExecutorContext, pStopRequest]() mutable
    {
      if (pStopRequest.isFinished())
        return FutureVoid();
      return _runSemExp(pSemExp, pExecutorContext, pStopRequest);
    });
  }
  return res;
}


FutureVoid VirtualExecutor::_doExecutionUntil(
    AnnotatedExpression& pAnnExp,
    std::shared_ptr<ExecutorContext> pExecutorContext,
    const FutureVoid& pStopRequest,
    std::shared_ptr<int> pLimitOfRecursions)
{
  return _runSemExp(pAnnExp.semExp, pExecutorContext, pStopRequest)
      .then([=, &pAnnExp]() mutable
  {
    if (pStopRequest.isFinished() ||
        *pLimitOfRecursions <= 0)
      return FutureVoid();
    --*pLimitOfRecursions;
    return _doExecutionUntil(pAnnExp, pExecutorContext, pStopRequest, pLimitOfRecursions);
  });
}


FutureVoid VirtualExecutor::_runGrdExp(
    const UniqueSemanticExpression& pUSemExp,
    std::shared_ptr<ExecutorContext> pExecutorContext,
    const FutureVoid& pStopRequest)
{
  const GroundedExpression& grdExp = pUSemExp->getGrdExp();
  const SemanticResourceGrounding* resourceGrdPtr = grdExp->getResourceGroundingPtr();
  if (resourceGrdPtr != nullptr)
    return _exposeResource(resourceGrdPtr->resource, pStopRequest);
  return _sayWithAnnotations(pUSemExp, pExecutorContext, pStopRequest,
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


FutureVoid VirtualExecutor::runSemExp(
    UniqueSemanticExpression pUSemExp,
    std::shared_ptr<ExecutorContext>& pExecutorContext)
{
  auto keepSemExpAlive = std::make_shared<UniqueSemanticExpression>
      (std::move(pUSemExp));
  const FutureVoid stopRequest(_stopper);
  return _runSemExp(*keepSemExpAlive, pExecutorContext, stopRequest).thenVoid
      ([keepSemExpAlive] {});
}

void VirtualExecutor::stop()
{
  _stopper->setFinished();
}

bool VirtualExecutor::isStopped() const
{
  return _stopper->isFinished();
}


FutureVoid VirtualExecutor::_runSemExp(
    UniqueSemanticExpression& pUSemExp,
    std::shared_ptr<ExecutorContext> pExecutorContext,
    const FutureVoid& pStopRequest)
{
  switch (pUSemExp->type)
  {
  case SemanticExpressionType::GROUNDED:
    return _runGrdExp(pUSemExp, pExecutorContext, pStopRequest);
  case SemanticExpressionType::LIST:
  {
    if (pExecutorContext->sayOrExecute)
      return _sayWithAnnotations(pUSemExp, pExecutorContext, pStopRequest,
                                 _typeOfExecutor, pExecutorContext->contAnnotation);

    ListExpression& listExp = pUSemExp->getListExp();
    switch (listExp.listType)
    {
    case ListExpressionType::AND:
    {
      return _handleAndList(listExp, pExecutorContext, pStopRequest);
    }
    case ListExpressionType::THEN:
    case ListExpressionType::UNRELATED:
    {
      return _handleThenList(listExp, pExecutorContext, pStopRequest);
    }
    case ListExpressionType::OR:
    {
      auto itRandElt = Random::advanceIterator(listExp.elts);
      return _runSemExp(*itRandElt, pExecutorContext, pStopRequest);
    }
    }
    break;
  }
  case SemanticExpressionType::ANNOTATED:
  {
    AnnotatedExpression& annExp = pUSemExp->getAnnExp();

    auto subContext = std::make_shared<ExecutorContext>(*pExecutorContext);
    subContext->updateAnnotation(annExp.annotations);
    SemanticLanguageEnum subLanguage = SemExpGetter::getLanguage(annExp.annotations);
    if (subLanguage != SemanticLanguageEnum::UNKNOWN)
      subContext->textProcContext.langType = subLanguage;

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

    {
      bool isHandled = false;
      FutureVoid res = _handleDurationAnnotations(isHandled, annExp, subContext, pStopRequest);
      if (isHandled)
        return res;
    }

    int nbOfRepetitions = SemExpGetter::getNumberOfRepetitions(annExp.annotations);
    return _runSemExpNTimes(annExp.semExp, subContext, pStopRequest, nbOfRepetitions);
  }
  case SemanticExpressionType::INTERPRETATION:
  {
    auto& intExp = pUSemExp->getIntExp();
    return _runSemExp(intExp.interpretedExp, pExecutorContext, pStopRequest);
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
        pSemanticMemory.interactionContextContainer = std::move(metadataExp.interactionContextContainer);
        metadataExp.interactionContextContainer = std::unique_ptr<InteractionContextContainer>();
      });
    }
    return _runSemExp(metadataExp.semExp, subContext, pStopRequest);
  }
  case SemanticExpressionType::SETOFFORMS:
  {
    UniqueSemanticExpression* originalFrom = pUSemExp->getSetOfFormsExp().getOriginalForm();
    if (originalFrom != nullptr)
      return _runSemExp(*originalFrom, pExecutorContext, pStopRequest);
    break;
  }
  case SemanticExpressionType::CONDITION:
  {
    if (pExecutorContext->sayOrExecute)
    {
      return _sayWithAnnotations(pUSemExp, pExecutorContext, pStopRequest,
                                 _typeOfExecutor, pExecutorContext->contAnnotation);
    }

    auto& condExp = pUSemExp->getCondExp();
    return _runConditionExp(condExp, pExecutorContext, pStopRequest);
  }
  case SemanticExpressionType::FEEDBACK:
  case SemanticExpressionType::COMPARISON:
    return _sayWithAnnotations(pUSemExp, pExecutorContext, pStopRequest,
                               _typeOfExecutor, pExecutorContext->contAnnotation);
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
          return _runSemExp(cmdExp.semExp, subContext, pStopRequest);
        }
        auto semExpToInform = std::make_shared<UniqueSemanticExpression>
            (SemExpModifier::fromActionDescriptionToSentenceInPresentTense(grdExp));
        _assertPunctually(**semExpToInform);

        auto exposureTime = std::make_shared<std::unique_ptr<SemanticTimeGrounding>>
                                                                                     (SemanticTimeGrounding::nowInstance());
        return _runSemExp(cmdExp.semExp, pExecutorContext, pStopRequest).thenVoid
            ([this, exposureTime, semExpToInform]
        {
          (*exposureTime)->setEndOfThisTimeNow();

          SemExpModifier::putInPastWithTimeAnnotation(*semExpToInform, std::move(*exposureTime));
          _assertPermanently(std::move(*semExpToInform));
        });
      }
    }

    return _runSemExp(cmdExp.semExp, pExecutorContext, pStopRequest);
  }
  case SemanticExpressionType::FIXEDSYNTHESIS:
    return _sayAndAddDescriptionTree(pUSemExp, pExecutorContext, pStopRequest,
                                     _typeOfExecutor, pExecutorContext->contAnnotation);
  }
  assert("Bad semantic expression type");
  return _reportAnError("Bad semantic expression type");
}


} // End of namespace onsem
