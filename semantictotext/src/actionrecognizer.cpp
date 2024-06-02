#include <onsem/semantictotext/actionrecognizer.hpp>
#include <onsem/common/utility/string.hpp>
#include <onsem/texttosemantic/dbtype/textprocessingcontext.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpressions.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticmetagrounding.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticstatementgrounding.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticagentgrounding.hpp>
#include <onsem/texttosemantic/tool/semexpgetter.hpp>
#include <onsem/semantictotext/semanticmemory/links/expressionwithlinks.hpp>
#include <onsem/semantictotext/semanticconverter.hpp>
#include <onsem/semantictotext/triggers.hpp>
#include "controller/semexpcontroller.hpp"
#include "conversion/conditionsadder.hpp"
#include "utility/utility.hpp"
#include "utility/semexpcreator.hpp"


namespace onsem {

namespace {

std::optional<UniqueSemanticExpression> _extractConditionOnlySemExp(
        UniqueSemanticExpression& pSemExp,
        bool& pIsTimeOnly) {
    switch (pSemExp->type) {
    case SemanticExpressionType::GROUNDED: {
        GroundedExpression& grdExp = pSemExp->getGrdExp();
        auto* statGrdPtr = grdExp->getStatementGroundingPtr();
        if (statGrdPtr != nullptr) {
            auto& statGrd = *statGrdPtr;
            if (statGrd.requests.has(SemanticRequestType::TIME)) {
                statGrd.requests.erase(SemanticRequestType::TIME);
                pIsTimeOnly = true;
                return pSemExp->clone();
            }

            auto itTimeChild = grdExp.children.find(GrammaticalType::TIME);
            if (itTimeChild != grdExp.children.end()) {
                pIsTimeOnly = SemExpGetter::isACoreferenceFromStatementGrounding(statGrd, CoreferenceDirectionEnum::BEFORE);
                return itTimeChild->second->clone();
            }
        }
        break;
    }
    case SemanticExpressionType::LIST: {
        break;
    }
    case SemanticExpressionType::INTERPRETATION: {
        return _extractConditionOnlySemExp(pSemExp->getIntExp().interpretedExp, pIsTimeOnly);
    }
    case SemanticExpressionType::FEEDBACK: {
        return _extractConditionOnlySemExp(pSemExp->getFdkExp().concernedExp, pIsTimeOnly);
    }
    case SemanticExpressionType::ANNOTATED: {
        return _extractConditionOnlySemExp(pSemExp->getAnnExp().semExp, pIsTimeOnly);
    }
    case SemanticExpressionType::METADATA: {
        return _extractConditionOnlySemExp(pSemExp->getMetadataExp().semExp, pIsTimeOnly);
    }
    case SemanticExpressionType::FIXEDSYNTHESIS:
    case SemanticExpressionType::COMMAND:
    case SemanticExpressionType::CONDITION:
    case SemanticExpressionType::COMPARISON:
    case SemanticExpressionType::SETOFFORMS: break;
    }
    return {};
}


void _extractParameters(
        std::map<std::string, std::vector<UniqueSemanticExpression>>& parameterLabelToQuestions,
        UniqueSemanticExpression& pSemExp,
        GrammaticalType pFirstChildGramType = GrammaticalType::UNKNOWN) {
    switch (pSemExp->type) {
    case SemanticExpressionType::GROUNDED: {
        GroundedExpression& grdExp = pSemExp->getGrdExp();
        for (auto itChild = grdExp.children.begin(); itChild != grdExp.children.end(); ) {
            auto newRequest = SemanticRequestType::NOTHING;
            if (pFirstChildGramType == GrammaticalType::UNKNOWN)
                newRequest = SemExpGetter::convertSemGramToRequestType(itChild->first);
            else
                newRequest = SemExpGetter::convertSemGramToRequestType(pFirstChildGramType);

            GroundedExpression* childGrdExpPtr = itChild->second->getGrdExpPtr_SkipWrapperPtrs();
            if (childGrdExpPtr != nullptr) {
                auto* metaGrdPtr = childGrdExpPtr->grounding().getMetaGroundingPtr();
                if (metaGrdPtr != nullptr) {
                    parameterLabelToQuestions[metaGrdPtr->attibuteName].emplace_back([&] {
                        auto grdExp = std::make_unique<GroundedExpression>([&] {
                            auto statementGrd = std::make_unique<SemanticStatementGrounding>();
                            statementGrd->requests.add(newRequest);
                            statementGrd->coreference.emplace(Coreference(CoreferenceDirectionEnum::BEFORE));
                            return statementGrd;
                        }());
                        return grdExp;
                    }());
                    itChild = grdExp.children.erase(itChild);
                    continue;
                }
            }

            if (pFirstChildGramType == GrammaticalType::UNKNOWN)
                _extractParameters(parameterLabelToQuestions, itChild->second, itChild->first);
            else
                _extractParameters(parameterLabelToQuestions, itChild->second, pFirstChildGramType);
            ++itChild;
        }
        break;
    }
    case SemanticExpressionType::LIST: {
        ListExpression& listExp = pSemExp->getListExp();
        for (auto& currElt : listExp.elts) {
            _extractParameters(parameterLabelToQuestions, currElt, pFirstChildGramType);
        }
        break;
    }
    case SemanticExpressionType::INTERPRETATION: {
        _extractParameters(parameterLabelToQuestions, pSemExp->getIntExp().interpretedExp, pFirstChildGramType);
        break;
    }
    case SemanticExpressionType::FEEDBACK: {
        _extractParameters(parameterLabelToQuestions, pSemExp->getFdkExp().concernedExp, pFirstChildGramType);
        break;
    }
    case SemanticExpressionType::ANNOTATED: {
        _extractParameters(parameterLabelToQuestions, pSemExp->getAnnExp().semExp, pFirstChildGramType);
        break;
    }
    case SemanticExpressionType::METADATA: {
        _extractParameters(parameterLabelToQuestions, pSemExp->getMetadataExp().semExp, pFirstChildGramType);
        break;
    }
    case SemanticExpressionType::FIXEDSYNTHESIS:
    case SemanticExpressionType::COMMAND:
    case SemanticExpressionType::CONDITION:
    case SemanticExpressionType::COMPARISON:
    case SemanticExpressionType::SETOFFORMS: break;
    }
}


void _addIntent(const std::string& pIntentName,
                const std::vector<std::string>& pFormulations,
                const std::map<std::string, ActionRecognizer::ParamInfo>& pParameterLabelToInfos,
                SemanticMemory& pSemMemory,
                const linguistics::LinguisticDatabase& pLingDb,
                SemanticLanguageEnum pLanguage) {
    TextProcessingContext triggerProcContext(
                SemanticAgentGrounding::currentUser, SemanticAgentGrounding::me, pLanguage);
    triggerProcContext.isTimeDependent = false;
    for (auto& currFormulation : pFormulations) {
        auto formulationSemExp = converter::textToSemExp(currFormulation, triggerProcContext, pLingDb);
        converter::correferenceToRobot(formulationSemExp, pLingDb);

        std::map<std::string, std::vector<std::string>> parameterLabelToQuestionsStrs;
        for (auto& currParam : pParameterLabelToInfos)
            parameterLabelToQuestionsStrs.emplace(currParam.first + ":" + currParam.second.type, currParam.second.questions);

        std::map<std::string, std::vector<UniqueSemanticExpression>> parameterLabelToQuestions;
        converter::createParameterSemanticexpressions(parameterLabelToQuestions, parameterLabelToQuestionsStrs, pLingDb, SemanticLanguageEnum::ENGLISH);
        _extractParameters(parameterLabelToQuestions, formulationSemExp);

        auto outputResourceGrdExp = std::make_unique<GroundedExpression>(converter::createResourceWithParametersFromSemExp(
                                                                             "intent", pIntentName, parameterLabelToQuestions, *formulationSemExp, pLingDb, pLanguage));
        triggers::add(std::move(formulationSemExp), std::move(outputResourceGrdExp), pSemMemory, pLingDb);
    }
}

void _addSemExpTrigger(const std::string& pActionIntentName,
                       UniqueSemanticExpression pFormulationSemExp,
                       std::map<std::string, std::list<UniqueSemanticExpression>>& pActionToSemExps,
                       SemanticMemory& pSemanticMemory,
                       const std::map<std::string, std::vector<UniqueSemanticExpression>>& pParameterLabelToQuestions,
                       const linguistics::LinguisticDatabase& pLingDb,
                       SemanticLanguageEnum pLanguage) {
    pActionToSemExps[pActionIntentName].emplace_back(pFormulationSemExp->clone());
    auto outputResourceGrdExp = std::make_unique<GroundedExpression>(converter::createResourceWithParametersFromSemExp(
                                                                         "intent", pActionIntentName, pParameterLabelToQuestions, *pFormulationSemExp, pLingDb, pLanguage));
    triggers::add(std::move(pFormulationSemExp), std::move(outputResourceGrdExp), pSemanticMemory, pLingDb);
}


ActionRecognizer::Intent _unknownIntent() {
    ActionRecognizer::Intent intent;
    intent.name = "UNKNOWN";
    return intent;
}

std::optional<ActionRecognizer::ActionRecognized> _reactionToIntent(const SemanticExpression& pReaction,
                                                                    std::map<std::string, SemanticMemory>& pTypeToMemory,
                                                                    const linguistics::LinguisticDatabase& pLingDb) {
    const GroundedExpression* grdExpPtr = pReaction.getGrdExpPtr_SkipWrapperPtrs();
    if (grdExpPtr != nullptr) {
        auto* resourceGrdPtr = grdExpPtr->grounding().getResourceGroundingPtr();
        if (resourceGrdPtr != nullptr) {
            ActionRecognizer::ActionRecognized res;
            res.intent.emplace(ActionRecognizer::Intent());
            auto& intent = *res.intent;
            intent.name = resourceGrdPtr->resource.value;
            for (auto& currParametersToSemExps : resourceGrdPtr->resource.parametersLabelsToValue) {
                auto& semExps = currParametersToSemExps.second;
                if (!semExps.empty()) {
                    std::vector<std::string> parameterSplitted;
                    mystd::split(parameterSplitted, currParametersToSemExps.first, ":");
                    if (!parameterSplitted.empty()) {
                        auto paramName = parameterSplitted[0];
                        auto& paramValues = intent.params[paramName];
                        TextProcessingContext outContext(
                                    SemanticAgentGrounding::me, SemanticAgentGrounding::currentUser, resourceGrdPtr->resource.language);
                        SemanticMemory semMemory;

                        for (auto& currAnswer : semExps) {
                            std::string newValue;
                            converter::semExpToText(newValue, currAnswer->clone(), outContext, true, semMemory, pLingDb, nullptr);

                            bool paramAdded = false;
                            if (parameterSplitted.size() > 1) {
                                auto paramType = parameterSplitted[1];
                                auto itParamMemory = pTypeToMemory.find(paramType);
                                if (itParamMemory != pTypeToMemory.end()) {
                                    mystd::unique_propagate_const<UniqueSemanticExpression> entityReaction;
                                    triggers::match(entityReaction, itParamMemory->second, currAnswer->clone(), pLingDb);

                                    if (entityReaction) {
                                        const GroundedExpression* entityGrdExpPtr = entityReaction->getSemExp().getGrdExpPtr_SkipWrapperPtrs();
                                        if (entityGrdExpPtr != nullptr) {
                                            auto* entityResourceGrdPtr = entityGrdExpPtr->grounding().getResourceGroundingPtr();
                                            if (entityResourceGrdPtr != nullptr) {
                                                paramValues.push_back(entityResourceGrdPtr->resource.value);
                                                paramAdded = true;
                                            }
                                        }
                                    }
                                }
                            }

                            if (!paramAdded)
                                paramValues.push_back(newValue);
                        }
                    }
                }
            }
            if (!res.empty())
                return res;
        }
    } else {
        auto listExpPtr = pReaction.getListExpPtr_SkipWrapperPtrs();
        if (listExpPtr != nullptr) {
            auto& listExp = *listExpPtr;
            ActionRecognizer::ActionRecognized res;
            auto& actions = [&]() -> std::list<ActionRecognizer::ActionRecognized>& {
                if (listExp.listType == ListExpressionType::THEN)
                    return res.toRunSequentially;
                if (listExp.listType == ListExpressionType::IN_BACKGROUND)
                    return res.toRunInBackground;
                return res.toRunInParallel;
            }();
            for (auto& currElt : listExp.elts) {
                auto subActioon = _reactionToIntent(*currElt, pTypeToMemory, pLingDb);
                if (subActioon)
                    actions.push_back(std::move(*subActioon));
            }
            if (!res.empty())
                return res;
        }
    }
    return {};
}

}


std::string ActionRecognizer::Intent::toStr() const {
    std::string res = name;
    if (!params.empty()) {
        res += "(";
        bool firstSlot = true;
        for (const auto& currSlot : params) {
            if (firstSlot)
                firstSlot = false;
            else
                res += ", ";
            res += currSlot.first + "=";

            bool firstIterationOnSlot = true;
            for (const auto& currValue : currSlot.second) {
                if (firstIterationOnSlot)
                    firstIterationOnSlot = false;
                else
                    res += "|";
                res += currValue;
            }
        }
        res += ")";
    }
    return res;
}

std::string ActionRecognizer::Intent::toJson() const {
    std::string res = "\"name\": \"" + name + "\"";
    if (!params.empty()) {
        std::string paramRes;
        for (const auto& currSlot : params) {
            if (currSlot.second.empty())
                continue;
            std::string valuesRes;
            for (auto& currValue : currSlot.second) {
                if (!valuesRes.empty())
                    valuesRes += ", ";
                valuesRes += "\"" + currValue + "\"";
            }
            if (!paramRes.empty())
                paramRes += ", ";
            paramRes += "\"" + currSlot.first + "\": [" + valuesRes + "]";
        }
        res += ", \"params\": {" + paramRes + "}";
    }
    return "{" + res + "}";
}


bool ActionRecognizer::ActionRecognized::isOnlyAnIntent() const {
    return intent && !condition && toRunSequentially.empty() && toRunInParallel.empty() &&
            toRunInBackground.empty();
}

bool ActionRecognizer::ActionRecognized::empty() const {
    return !intent && !condition && toRunSequentially.empty() && toRunInParallel.empty() &&
            toRunInBackground.empty();
}

std::string ActionRecognizer::ActionRecognized::toJson() const {
    std::string res;
    if (intent)
        res = "\"intent\": \"" + intent->toJson() + "\"";
    if (condition) {
        if (!res.empty())
            res += ", ";
        res += "\"condition\": " + condition->toJson();
    }

    auto tryToWriteList = [&](const std::list<ActionRecognized>& pList, const std::string& pLabel) {
        if (!pList.empty()) {
            std::string listRes;
            for (auto& currElt : pList) {
                if (!listRes.empty())
                    listRes += ", ";
                listRes += currElt.toJson();
            }
            if (!res.empty())
                res += ", ";
            res += "\"" + pLabel + "\": [" + listRes + "]";
        }
    };

    tryToWriteList(toRunSequentially, "to_run_sequentially");
    tryToWriteList(toRunInParallel, "to_run_in_parallel");
    tryToWriteList(toRunInBackground, "to_run_in_background");
    return "{" + res + "}";
}

std::string ActionRecognizer::ActionRecognized::toJsonWithIntentInStr() const {
    std::string res;
    if (intent)
        res = "\"intent\": \"" + intent->toStr() + "\"";
    if (condition) {
        if (!res.empty())
            res += ", ";
        res += "\"condition\": " + condition->toJsonWithIntentInStr();
    }

    auto tryToWriteList = [&](const std::list<ActionRecognized>& pList, const std::string& pLabel) {
        if (!pList.empty()) {
            std::string listRes;
            for (auto& currElt : pList) {
                if (!listRes.empty())
                    listRes += ", ";
                listRes += currElt.toJsonWithIntentInStr();
            }
            if (!res.empty())
                res += ", ";
            res += "\"" + pLabel + "\": [" + listRes + "]";
        }
    };

    tryToWriteList(toRunSequentially, "to_run_sequentially");
    tryToWriteList(toRunInParallel, "to_run_in_parallel");
    tryToWriteList(toRunInBackground, "to_run_in_background");
    return "{" + res + "}";
}

ActionRecognizer::ParamInfo::ParamInfo(const std::string& pType,
                                       const std::vector<std::string>& pQuestions)
    : type(pType),
      questions(pQuestions)
{
}


ActionRecognizer::ActionRecognizer(SemanticLanguageEnum pLanguage)
    : _language(pLanguage),
      _actionSemanticMemory(),
      _predicateSemanticMemory(),
      _typeToFormulations(),
      _typeWithValueConsideredAsOwner(),
      _typeToMemory(),
      _actionToSemExps() {
}


void ActionRecognizer::addType(const std::string& pType,
                               const std::vector<std::string>& pFormulations,
                               bool pIsValueConsideredAsOwner) {
    auto& formuationsForType = _typeToFormulations[pType];
    formuationsForType.insert(formuationsForType.end(), pFormulations.begin(), pFormulations.end());
    if (pIsValueConsideredAsOwner)
        _typeWithValueConsideredAsOwner.insert(pType);
}

void ActionRecognizer::addEntity(const std::string& pType,
                                 const std::string& pEntityId,
                                 const std::vector<std::string>& pEntityLabels,
                                 const linguistics::LinguisticDatabase& pLingDb) {
    auto& semMem = _typeToMemory[pType];
    const std::map<std::string, ParamInfo> parameterLabelToQuestions;
    _addIntent(pEntityId, pEntityLabels, parameterLabelToQuestions, semMem, pLingDb, _language);

    bool isValueConsideredAsOwner = _typeWithValueConsideredAsOwner.count(pType) > 0;
    auto& formuations = _typeToFormulations[pType];
    std::vector<std::string> newEntityLabels;
    for (const auto& currFormulation : formuations) {
        for (const auto& currLabel : pEntityLabels) {
            newEntityLabels.emplace_back(currFormulation + " " + currLabel);
            if (isValueConsideredAsOwner) {
                if (_language == SemanticLanguageEnum::FRENCH)
                    newEntityLabels.emplace_back(currFormulation + " de " + currLabel);
                else
                    newEntityLabels.emplace_back(currFormulation + " of " + currLabel);
            }
        }
    }
    _addIntent(pEntityId, newEntityLabels, parameterLabelToQuestions, semMem, pLingDb, _language);
}


void ActionRecognizer::addPredicate(const std::string& pPredicateName,
                                    const std::vector<std::string>& pPredicateFormulations,
                                    const linguistics::LinguisticDatabase& pLingDb) {
    const std::map<std::string, ParamInfo> parameterLabelToQuestions;
    _addIntent(pPredicateName, pPredicateFormulations, parameterLabelToQuestions,
               _predicateSemanticMemory, pLingDb, _language);
}




void ActionRecognizer::addAction(const std::string& pActionIntentName,
                                 const std::vector<std::string>& pIntentFormulations,
                                 const std::map<std::string, ParamInfo>& pParameterLabelToInfos,
                                 const linguistics::LinguisticDatabase& pLingDb) {
    TextProcessingContext triggerProcContext(
                SemanticAgentGrounding::currentUser, SemanticAgentGrounding::me, _language);
    triggerProcContext.isTimeDependent = false;
    for (auto& currFormulation : pIntentFormulations) {
        auto formulationSemExp = converter::textToSemExp(currFormulation, triggerProcContext, pLingDb);
        converter::correferenceToRobot(formulationSemExp, pLingDb);

        std::list<UniqueSemanticExpression> otherFormulationsSemExps;
        {
            auto* grdExpPtr = formulationSemExp->getGrdExpPtr_SkipWrapperPtrs();
            if (grdExpPtr != nullptr && SemExpGetter::isAnInfinitiveGrdExp(*grdExpPtr))
                otherFormulationsSemExps.emplace_back(SemExpCreator::getImperativeAssociateFrom(*grdExpPtr));
        }

        std::map<std::string, std::vector<std::string>> parameterLabelToQuestionsStrs;
        for (auto& currParam : pParameterLabelToInfos)
            parameterLabelToQuestionsStrs.emplace(currParam.first + ":" + currParam.second.type, currParam.second.questions);

        std::map<std::string, std::vector<UniqueSemanticExpression>> parameterLabelToQuestions;
        converter::createParameterSemanticexpressions(parameterLabelToQuestions, parameterLabelToQuestionsStrs, pLingDb, SemanticLanguageEnum::ENGLISH);

        _addSemExpTrigger(pActionIntentName, std::move(formulationSemExp), _actionToSemExps,
                          _actionSemanticMemory, parameterLabelToQuestions, pLingDb, _language);
        for (auto& currOtherFormulaation : otherFormulationsSemExps)
            _addSemExpTrigger(pActionIntentName, std::move(currOtherFormulaation), _actionToSemExps,
                              _actionSemanticMemory, parameterLabelToQuestions, pLingDb, _language);
    }
}


bool ActionRecognizer::isObviouslyWrong(const std::string& pActionIntentName,
                                        const SemanticExpression& pUtteranceSemExp,
                                        const linguistics::LinguisticDatabase& pLingDb) const {
  auto it = _actionToSemExps.find(pActionIntentName);
  if (it != _actionToSemExps.end()) {
    for (auto& currSemExpToMatch : it->second) {
      auto imbrication = SemExpComparator::getSemExpsImbrications(pUtteranceSemExp, *currSemExpToMatch, _actionSemanticMemory.memBloc, pLingDb, nullptr);
      if (imbrication == ImbricationType::OPPOSES)
        return true;
    }
  }
  return false;
}


std::optional<ActionRecognizer::ActionRecognized> ActionRecognizer::recognize(UniqueSemanticExpression pUtteranceSemExp,
                                                                              const linguistics::LinguisticDatabase& pLingDb) {
    converter::addDifferentForms(pUtteranceSemExp, pLingDb);
    conditionsAdder::addConditonsForSomeTimedGrdExp(pUtteranceSemExp);

    std::optional<UniqueSemanticExpression> conditionSemExp;
    auto* condPtr = pUtteranceSemExp->getCondExpPtr_SkipWrapperPtrs();
    if (condPtr) {
        conditionSemExp.emplace(std::move(condPtr->conditionExp));
        pUtteranceSemExp = std::move(condPtr->thenExp);
    }

    // Function to extract the condition
    auto _extractCondtion = [&]() -> std::optional<ActionRecognized> {
        if (conditionSemExp) {
            mystd::unique_propagate_const<UniqueSemanticExpression> conditionReaction;
            triggers::match(conditionReaction, _predicateSemanticMemory, std::move(*conditionSemExp), pLingDb);
            if (conditionReaction) {
              const auto& conditionReactionSemExp = conditionReaction->getSemExp();
              return _reactionToIntent(conditionReactionSemExp, _typeToMemory, pLingDb);
            }
        }
        return {};
    };

    if (SemExpGetter::isACoreference(*pUtteranceSemExp, CoreferenceDirectionEnum::BEFORE)) {
        auto conditionOpt = _extractCondtion();
        if (!conditionOpt)
            return {};
        ActionRecognizer::ActionRecognized actionRecognized;
        actionRecognized.condition = std::make_unique<ActionRecognizer::ActionRecognized>(std::move(*conditionOpt));
        return actionRecognized;
    }

    static const InformationType informationType = InformationType::INFORMATION;
    std::unique_ptr<CompositeSemAnswer> compSemAnswers;
    SemanticMemory localSemanticMemory;
    auto expForMem = localSemanticMemory.memBloc.addRootSemExp(pUtteranceSemExp->clone(), pLingDb);
    ExpressionWithLinks& expForMemRef = *expForMem;
    controller::applyOperatorOnExpHandleInMemory(compSemAnswers,
                                                 expForMemRef,
                                                 SemanticOperatorEnum::REACTFROMTRIGGER,
                                                 informationType,
                                                 _actionSemanticMemory,
                                                 nullptr,
                                                 pLingDb,
                                                 nullptr);

    mystd::unique_propagate_const<UniqueSemanticExpression> reaction;
    if (compSemAnswers) {
        controller::compAnswerToSemExp(reaction, *compSemAnswers);

        if (reaction) {
          const auto& reactionSemExp = reaction->getSemExp();
          auto actionRecognizedOpt = _reactionToIntent(reactionSemExp, _typeToMemory, pLingDb);
          if (actionRecognizedOpt) {
              auto conditionOpt = _extractCondtion();
              if (conditionOpt) {
                  ActionRecognizer::ActionRecognized actionRecognized;
                  actionRecognized.condition = std::make_unique<ActionRecognizer::ActionRecognized>(std::move(*conditionOpt));
                  if (actionRecognizedOpt->isOnlyAnIntent())
                      actionRecognized.intent = std::move(actionRecognizedOpt->intent);
                  else
                      actionRecognized.toRunSequentially.push_back(std::move(*actionRecognizedOpt));
                  return actionRecognized;
              }
              return actionRecognizedOpt;
          }
        }
    }

    bool isConditionOnly = false;
    if (!conditionSemExp) {
        conditionSemExp = _extractConditionOnlySemExp(pUtteranceSemExp, isConditionOnly);
    }

    if (conditionSemExp) {
        auto conditionOpt = _extractCondtion();
        if (conditionOpt) {
            ActionRecognizer::ActionRecognized actionRecognized;
            actionRecognized.condition = std::make_unique<ActionRecognizer::ActionRecognized>(std::move(*conditionOpt));
            if (!isConditionOnly)
                actionRecognized.intent = _unknownIntent();
            return actionRecognized;
        }
    }

    return {};
}



}    // End of namespace onsem
