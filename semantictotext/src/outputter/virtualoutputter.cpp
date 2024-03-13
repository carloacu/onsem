#include <onsem/semantictotext/outputter/virtualoutputter.hpp>
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
#include <onsem/semantictotext/semanticconverter.hpp>
#include "../linguisticsynthesizer/linguisticsynthesizer.hpp"
#include "grdexptooutputterinformation.hpp"

namespace onsem {

VirtualOutputter::VirtualOutputter(SemanticMemory& pSemanticMemory,
                                   const linguistics::LinguisticDatabase& pLingDb,
                                   SemanticSourceEnum pHowTheTextWillBeExposed)
    : _lingDb(pLingDb)
    , _semanticMemory(pSemanticMemory)
    , _typeOfOutputter(pHowTheTextWillBeExposed) {}

void VirtualOutputter::_sayAndAddDescriptionTree(const SemanticExpression& pSemExp,
                                                 const OutputterContext& pOutputterContext,
                                                 SemanticSourceEnum pFrom,
                                                 ContextualAnnotation pContextualAnnotation) {
    IndexToSubNameToParameterValue params;
    params[0][""] = std::make_unique<ReferenceOfSemanticExpressionContainer>(pSemExp);
    auto descExp = MetadataExpression::constructSourceFromSourceEnumInPresent(
        SemanticAgentGrounding::getRobotAgentPtr(), pFrom, pContextualAnnotation);
    static const std::set<SemanticExpressionType> expressionTypesToSkip = {SemanticExpressionType::ANNOTATED,
                                                                           SemanticExpressionType::SETOFFORMS};
    descExp = descExp->clone(&params, true, &expressionTypesToSkip);
    _assertPunctually(*descExp);
    auto descExpWithMetadata = std::make_shared<std::unique_ptr<MetadataExpression>>(
        std::make_unique<MetadataExpression>(SemanticSourceEnum::SEMREACTION, std::move(descExp), pSemExp.clone()));
    (*descExpWithMetadata)->contextualAnnotation = pContextualAnnotation;

    auto exposureTime = std::make_shared<std::unique_ptr<SemanticTimeGrounding>>(SemanticTimeGrounding::nowInstance());

    std::list<std::unique_ptr<SynthesizerResult>> synthesizerResults;
    _convertToText(synthesizerResults, pSemExp, pOutputterContext.textProcContext);

    for (const auto& currResult : synthesizerResults) {
        switch (currResult->type) {
            case SynthesizerResultEnum::TEXT: {
                const auto& syntText = *dynamic_cast<const SynthesizerText*>(&*currResult);
                _exposeText(syntText.text, pOutputterContext.textProcContext.langType);
                break;
            }
            case SynthesizerResultEnum::TASK: {
                const auto& syntTask = *dynamic_cast<const SynthesizerTask*>(&*currResult);
                _processResource(syntTask.resource, pOutputterContext.inputSemExpPtr);
                break;
            }
        }
    }

    (*exposureTime)->setEndOfThisTimeNow();
    if ((*descExpWithMetadata)->source)
        SemExpModifier::putInPastWithTimeAnnotation(*(*descExpWithMetadata)->source, std::move(*exposureTime));
    _teachInformation(std::move(*descExpWithMetadata));
}

void VirtualOutputter::_sayWithAnnotations(const SemanticExpression& pSemExp,
                                           const OutputterContext& pOutputterContext,
                                           SemanticSourceEnum pFrom,
                                           ContextualAnnotation pContextualAnnotation) {
    if (pOutputterContext.annotations->empty()) {
        _sayAndAddDescriptionTree(pSemExp, pOutputterContext, pFrom, pContextualAnnotation);
        return;
    }

    auto annExp = std::make_unique<AnnotatedExpression>(pSemExp.clone());
    for (const auto& currAnnotation : *pOutputterContext.annotations)
        annExp->annotations.emplace(currAnnotation.first, currAnnotation.second->clone());
    _sayAndAddDescriptionTree(*annExp, pOutputterContext, pFrom, pContextualAnnotation);
}

void VirtualOutputter::_handleList(const ListExpression& pListExp,
                                   Link pLink,
                                   const OutputterContext& pOutputterContext) {
    _beginOfScope(pLink);
    for (auto& currElt : pListExp.elts)
        processSemExp(*currElt, pOutputterContext);
    _endOfScope();
}

void VirtualOutputter::_handleThenReversedList(const ListExpression& pListExp,
                                               const OutputterContext& pOutputterContext) {
    _beginOfScope(Link::THEN_REVERSED);
    for (auto it = pListExp.elts.rbegin(); it != pListExp.elts.rend(); ++it) {
        auto& currElt = **it;
        processSemExp(currElt, pOutputterContext);
    }
    _endOfScope();
}

void VirtualOutputter::_runConditionExp(const ConditionExpression& pCondExp,
                                        const OutputterContext& pOutputterContext) {
    /*
    return _waitUntil(*pCondExp.conditionExp).then
        ([this, &pCondExp, pOutputterContext]() mutable
    {
      return _runSemExp(pCondExp.thenExp, pOutputterContext);
    });
    */
}

void VirtualOutputter::_processResource(const SemanticResource& pResource, const SemanticExpression* pInputSemExpPtr) {
    std::map<std::string, std::vector<std::string>> parameters;
    for (auto& currParametersToSemExps : pResource.parametersLabelsToValue) {
        auto& semExps = currParametersToSemExps.second;
        if (!semExps.empty()) {
            auto& strs = parameters[currParametersToSemExps.first];
            TextProcessingContext outContext(
                SemanticAgentGrounding::currentUser, SemanticAgentGrounding::me, pResource.language);
            SemanticMemory semMemory;

            for (auto& currAnswer : semExps) {
                std::string subRes;
                converter::semExpToText(subRes, currAnswer->clone(), outContext, true, semMemory, _lingDb, nullptr);
                strs.push_back(subRes);
            }
        }
    }
    _exposeResource(pResource, parameters);
}

void VirtualOutputter::_handleDurationAnnotations(bool&,
                                                  const AnnotatedExpression&,
                                                  const OutputterContext& pOutputterContext) {}

void VirtualOutputter::_doUntil(const AnnotatedExpression& pAnnExp,
                                std::shared_ptr<OutputterContext> pOutputterContext,
                                std::shared_ptr<int> pLimitOfRecursions) {
    /*
    _runSemExp(pAnnExp.semExp, pOutputterContext)
        .then([=, &pAnnExp]() mutable
    {
      if (*pLimitOfRecursions <= 0)
        return;
      --*pLimitOfRecursions;
      return _doUntil(pAnnExp, pOutputterContext, pLimitOfRecursions);
    });
    */
}

void VirtualOutputter::_processGrdExp(const SemanticExpression& pSemExp, const OutputterContext& pOutputterContext) {
    const GroundedExpression& grdExp = pSemExp.getGrdExp();
    const SemanticResourceGrounding* resourceGrdPtr = grdExp->getResourceGroundingPtr();
    if (resourceGrdPtr != nullptr) {
        _processResource(resourceGrdPtr->resource, pOutputterContext.inputSemExpPtr);
        return;
    }

    OutputInformation outputInformation;
    if (grdExpToOutputInformation(outputInformation, grdExp)) {
        if (outputInformation.nbOfTimes)
            _insideScopeNbOfTimes(*outputInformation.nbOfTimes);

        if (outputInformation.toDoInBackground) {
            _beginOfScope(Link::IN_BACKGROUND);
            processSemExp(**outputInformation.toDoInBackground, pOutputterContext);
            _endOfScope();
        }

        _endOfScope();
        _beginOfScope(Link::THEN);
        return;
    }

    _sayWithAnnotations(pSemExp, pOutputterContext, _typeOfOutputter, pOutputterContext.contAnnotation);
}

void VirtualOutputter::_reportAnError(const std::string&) {
    return;
}

void VirtualOutputter::_convertToText(std::list<std::unique_ptr<SynthesizerResult>>& pRes,
                                      const SemanticExpression& pSemExp,
                                      const TextProcessingContext& pTextProcContext) {
    auto userId = _semanticMemory.getCurrUserId();
    synthesize(pRes, pSemExp.clone(), false, _semanticMemory.memBloc, userId, pTextProcContext, _lingDb, nullptr);
}

/*
FutureVoid VirtualOutputter::_waitUntil(const SemanticExpression& pSemExp,
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

void VirtualOutputter::processSemExp(const SemanticExpression& pSemExp, const OutputterContext& pOutputterContext) {
    switch (pSemExp.type) {
        case SemanticExpressionType::GROUNDED: {
            _processGrdExp(pSemExp, pOutputterContext);
            return;
        }
        case SemanticExpressionType::LIST: {
            if (pOutputterContext.sayOrExecute) {
                _sayWithAnnotations(pSemExp, pOutputterContext, _typeOfOutputter, pOutputterContext.contAnnotation);
                return;
            }

            const ListExpression& listExp = pSemExp.getListExp();
            switch (listExp.listType) {
                case ListExpressionType::AND: {
                    _handleList(listExp, Link::AND, pOutputterContext);
                    return;
                }
                case ListExpressionType::THEN:
                case ListExpressionType::UNRELATED: {
                    _handleList(listExp, Link::THEN, pOutputterContext);
                    return;
                }
                case ListExpressionType::THEN_REVERSED: {
                    _handleThenReversedList(listExp, pOutputterContext);
                    return;
                }
                case ListExpressionType::OR: {
                    auto itRandElt = Random::advanceConstIterator(listExp.elts);
                    processSemExp(**itRandElt, pOutputterContext);
                    return;
                }
            }
            break;
        }
        case SemanticExpressionType::ANNOTATED: {
            const AnnotatedExpression& annExp = pSemExp.getAnnExp();

            auto subContext = pOutputterContext;
            subContext.updateAnnotation(annExp.annotations);
            SemanticLanguageEnum subLanguage = SemExpGetter::getLanguage(annExp.annotations);
            if (subLanguage != SemanticLanguageEnum::UNKNOWN)
                subContext.textProcContext.langType = subLanguage;

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
                _handleDurationAnnotations(isHandled, annExp, subContext);
                if (isHandled)
                    return;
            }

            int nbOfRepetitions = SemExpGetter::getNumberOfRepetitions(annExp.annotations);
            if (nbOfRepetitions > 1)
                _beginOfScope(Link::THEN);
            processSemExp(*annExp.semExp, subContext);

            if (nbOfRepetitions > 1) {
                _resourceNbOfTimes(nbOfRepetitions);
                _endOfScope();
            }

            const UniqueSemanticExpression* backgroundSemExpPtr = nullptr;
            auto itBackground = annExp.annotations.find(GrammaticalType::IN_BACKGROUND);
            if (itBackground != annExp.annotations.end())
                backgroundSemExpPtr = &itBackground->second;
            if (backgroundSemExpPtr != nullptr) {
                _beginOfScope(Link::IN_BACKGROUND);
                processSemExp(**backgroundSemExpPtr, pOutputterContext);
                _endOfScope();
            }
            return;
        }
        case SemanticExpressionType::INTERPRETATION: {
            auto& intExp = pSemExp.getIntExp();
            processSemExp(*intExp.interpretedExp, pOutputterContext);
            return;
        }
        case SemanticExpressionType::METADATA: {
            auto& metadataExp = pSemExp.getMetadataExp();
            auto subContext = pOutputterContext;
            subContext.contAnnotation = metadataExp.contextualAnnotation;
            subContext.sayOrExecute = metadataExp.contextualAnnotation != ContextualAnnotation::BEHAVIOR;
            if (metadataExp.interactionContextContainer)
                _semanticMemory.interactionContextContainer = metadataExp.interactionContextContainer->clone();
            processSemExp(*metadataExp.semExp, subContext);
            return;
        }
        case SemanticExpressionType::SETOFFORMS: {
            UniqueSemanticExpression* originalFrom = pSemExp.getSetOfFormsExp().getOriginalForm();
            if (originalFrom != nullptr) {
                processSemExp(**originalFrom, pOutputterContext);
                return;
            }
            break;
        }
        case SemanticExpressionType::CONDITION: {
            if (pOutputterContext.sayOrExecute) {
                _sayWithAnnotations(pSemExp, pOutputterContext, _typeOfOutputter, pOutputterContext.contAnnotation);
                return;
            }

            auto& condExp = pSemExp.getCondExp();
            _runConditionExp(condExp, pOutputterContext);
            return;
        }
        case SemanticExpressionType::FEEDBACK:
        case SemanticExpressionType::COMPARISON: {
            _sayWithAnnotations(pSemExp, pOutputterContext, _typeOfOutputter, pOutputterContext.contAnnotation);
            return;
        }
        case SemanticExpressionType::COMMAND: {
            auto& cmdExp = pSemExp.getCmdExp();
            if (cmdExp.description) {
                const GroundedExpression* grdExpPtr = (*cmdExp.description)->getGrdExpPtr_SkipWrapperPtrs();
                if (grdExpPtr != nullptr) {
                    const GroundedExpression& grdExp = *grdExpPtr;
                    const SemanticStatementGrounding* statementPtr = grdExp->getStatementGroundingPtr();
                    if (statementPtr != nullptr && statementPtr->concepts.count("verb_action_say")) {
                        auto subContext = pOutputterContext;
                        subContext.sayOrExecute = true;
                        return processSemExp(*cmdExp.semExp, subContext);
                    }
                    auto semExpToInform = std::make_shared<UniqueSemanticExpression>(
                        SemExpModifier::fromActionDescriptionToSentenceInPresentTense(grdExp));
                    _assertPunctually(semExpToInform->getSemExp());

                    auto exposureTime =
                        std::make_shared<std::unique_ptr<SemanticTimeGrounding>>(SemanticTimeGrounding::nowInstance());
                    processSemExp(*cmdExp.semExp, pOutputterContext);
                    (*exposureTime)->setEndOfThisTimeNow();
                    SemExpModifier::putInPastWithTimeAnnotation(*semExpToInform, std::move(*exposureTime));
                    _assertPermanently(std::move(*semExpToInform));
                    return;
                }
            }

            processSemExp(*cmdExp.semExp, pOutputterContext);
            return;
        }
        case SemanticExpressionType::FIXEDSYNTHESIS: {
            _sayAndAddDescriptionTree(pSemExp, pOutputterContext, _typeOfOutputter, pOutputterContext.contAnnotation);
            return;
        }
    }
    assert("Bad semantic expression type");
    _reportAnError("Bad semantic expression type");
}

}    // End of namespace onsem
