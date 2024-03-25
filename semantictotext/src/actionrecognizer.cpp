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


namespace onsem {

namespace {

std::optional<UniqueSemanticExpression> _extractConditionOnlySemExp(
        UniqueSemanticExpression& pSemExp) {
    switch (pSemExp->type) {
        case SemanticExpressionType::GROUNDED: {
            GroundedExpression& grdExp = pSemExp->getGrdExp();
            auto* statGrdPtr = grdExp->getStatementGroundingPtr();
            if (statGrdPtr != nullptr) {
                auto& statGrd = *statGrdPtr;
                if (statGrd.requests.has(SemanticRequestType::TIME)) {
                    statGrd.requests.erase(SemanticRequestType::TIME);
                    return pSemExp->clone();
                }

                if (SemExpGetter::isACoreferenceFromStatementGrounding(statGrd, CoreferenceDirectionEnum::BEFORE)) {
                    auto itTimeChild = grdExp.children.find(GrammaticalType::TIME);
                    if (itTimeChild != grdExp.children.end())
                        return itTimeChild->second->clone();
                }
            }
            break;
        }
        case SemanticExpressionType::LIST: {
            break;
        }
        case SemanticExpressionType::INTERPRETATION: {
            return _extractConditionOnlySemExp(pSemExp->getIntExp().interpretedExp);
        }
        case SemanticExpressionType::FEEDBACK: {
            return _extractConditionOnlySemExp(pSemExp->getFdkExp().concernedExp);
        }
        case SemanticExpressionType::ANNOTATED: {
            return _extractConditionOnlySemExp(pSemExp->getAnnExp().semExp);
        }
        case SemanticExpressionType::METADATA: {
            return _extractConditionOnlySemExp(pSemExp->getMetadataExp().semExp);
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
        UniqueSemanticExpression& pSemExp) {
    switch (pSemExp->type) {
        case SemanticExpressionType::GROUNDED: {
            GroundedExpression& grdExp = pSemExp->getGrdExp();
            for (auto itChild = grdExp.children.begin(); itChild != grdExp.children.end(); ) {
                GroundedExpression* childGrdExpPtr = itChild->second->getGrdExpPtr_SkipWrapperPtrs();
                if (childGrdExpPtr != nullptr) {
                    auto* metaGrdPtr = childGrdExpPtr->grounding().getMetaGroundingPtr();
                    if (metaGrdPtr != nullptr) {
                        parameterLabelToQuestions[metaGrdPtr->attibuteName].emplace_back(
                                    std::make_unique<GroundedExpression>([&] {
                            auto statementGrd = std::make_unique<SemanticStatementGrounding>();
                            statementGrd->requests.add(SemExpGetter::convertSemGramToRequestType(itChild->first));
                            statementGrd->coreference.emplace(Coreference(CoreferenceDirectionEnum::BEFORE));
                            return statementGrd;
                        }()));
                        itChild = grdExp.children.erase(itChild);
                        continue;
                    }
                }

                _extractParameters(parameterLabelToQuestions, itChild->second);
                ++itChild;
            }
            break;
        }
        case SemanticExpressionType::LIST: {
            ListExpression& listExp = pSemExp->getListExp();
            for (auto& currElt : listExp.elts) {
                _extractParameters(parameterLabelToQuestions, currElt);
            }
            break;
        }
        case SemanticExpressionType::INTERPRETATION: {
            _extractParameters(parameterLabelToQuestions, pSemExp->getIntExp().interpretedExp);
            break;
        }
        case SemanticExpressionType::FEEDBACK: {
            _extractParameters(parameterLabelToQuestions, pSemExp->getFdkExp().concernedExp);
            break;
        }
        case SemanticExpressionType::ANNOTATED: {
            _extractParameters(parameterLabelToQuestions, pSemExp->getAnnExp().semExp);
            break;
        }
        case SemanticExpressionType::METADATA: {
            _extractParameters(parameterLabelToQuestions, pSemExp->getMetadataExp().semExp);
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
                SemanticMemory& pSemMemory,
                const linguistics::LinguisticDatabase& pLingDb,
                SemanticLanguageEnum pLanguage) {
    TextProcessingContext triggerProcContext(
        SemanticAgentGrounding::currentUser, SemanticAgentGrounding::me, pLanguage);
    triggerProcContext.isTimeDependent = false;
    for (auto& currFormulation : pFormulations) {
        auto formulationSemExp = converter::textToSemExp(currFormulation, triggerProcContext, pLingDb);
        std::map<std::string, std::vector<UniqueSemanticExpression>> parameterLabelToQuestions;
        _extractParameters(parameterLabelToQuestions, formulationSemExp);

        auto outputResourceGrdExp = std::make_unique<GroundedExpression>(converter::createResourceWithParametersFromSemExp(
            "intent", pIntentName, parameterLabelToQuestions, *formulationSemExp, pLingDb, pLanguage));
        triggers::add(std::move(formulationSemExp), std::move(outputResourceGrdExp), pSemMemory, pLingDb);
    }
}

std::optional<ActionRecognizer::Intent> _reactionToIntent(const mystd::unique_propagate_const<UniqueSemanticExpression>& pReaction,
                                                          std::map<std::string, SemanticMemory>& pTypeToMemory,
                                                          const linguistics::LinguisticDatabase& pLingDb) {
    if (pReaction) {
         const GroundedExpression* grdExpPtr = pReaction->getSemExp().getGrdExpPtr_SkipWrapperPtrs();
         if (grdExpPtr != nullptr) {
             auto* resourceGrdPtr = grdExpPtr->grounding().getResourceGroundingPtr();
             if (resourceGrdPtr != nullptr) {
                 ActionRecognizer::Intent intent;
                 intent.name = resourceGrdPtr->resource.value;
                 for (auto& currParametersToSemExps : resourceGrdPtr->resource.parametersLabelsToValue) {
                     auto& semExps = currParametersToSemExps.second;
                     if (!semExps.empty()) {
                         std::vector<std::string> parameterSplitted;
                         mystd::split(parameterSplitted, currParametersToSemExps.first, ":");
                         if (!parameterSplitted.empty()) {
                             auto paramName = parameterSplitted[0];
                             auto& paramValues = intent.slotNameToValues[paramName];
                             TextProcessingContext outContext(
                                 SemanticAgentGrounding::currentUser, SemanticAgentGrounding::me, resourceGrdPtr->resource.language);
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

                                 if (!paramAdded)
                                     paramValues.push_back(newValue);
                             }
                         }
                     }
                 }

                 return intent;
             }
         }
    }
    return {};
}

}


std::string ActionRecognizer::Intent::toStr() const {
    std::string res = name;
    if (!slotNameToValues.empty()) {
        res += "(";
        bool firstSlot = true;
        for (const auto& currSlot : slotNameToValues) {
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


bool ActionRecognizer::ActionRecognized::empty() const {
    return !action && !condition;
}

std::string ActionRecognizer::ActionRecognized::toJson() const {
    std::string res;
    if (action)
        res = "\"action\": \"" + action->toStr() + "\"";
    if (condition) {
        if (!res.empty())
            res += ", ";
        res += "\"condition\": \"" + condition->toStr() + "\"";
    }
    return "{" + res + "}";
}




ActionRecognizer::ActionRecognizer(SemanticLanguageEnum pLanguage)
    : _language(pLanguage),
      _actionSemanticMemory(),
      _predicateSemanticMemory(),
      _typeToFormulations(),
      _typeToMemory() {
}


void ActionRecognizer::addType(const std::string& pType,
                               const std::vector<std::string>& pFormulations) {
    auto& formuationsForType = _typeToFormulations[pType];
    formuationsForType.insert(formuationsForType.end(), pFormulations.begin(), pFormulations.end());
}

void ActionRecognizer::addEntity(const std::string& pType,
                                 const std::string& pEntityId,
                                 const std::vector<std::string>& pEntityLabels,
                                 const linguistics::LinguisticDatabase& pLingDb) {
    auto& semMem = _typeToMemory[pType];
    _addIntent(pEntityId, pEntityLabels, semMem, pLingDb, _language);

    auto& formuations = _typeToFormulations[pType];
    std::vector<std::string> newEntityLabels;
    for (const auto& currFormulation : formuations) {
        for (const auto& currLabel : pEntityLabels) {
            newEntityLabels.emplace_back(currFormulation + " " + currLabel);
        }
    }
    _addIntent(pEntityId, newEntityLabels, semMem, pLingDb, _language);
}


void ActionRecognizer::addPredicate(const std::string& pPredicateName,
                                    const std::vector<std::string>& pPredicateFormulations,
                                    const linguistics::LinguisticDatabase& pLingDb) {
    _addIntent(pPredicateName, pPredicateFormulations, _predicateSemanticMemory, pLingDb, _language);
}


void ActionRecognizer::addAction(const std::string& pActionIntentName,
                                 const std::vector<std::string>& pIntentFormulations,
                                 const linguistics::LinguisticDatabase& pLingDb) {
    _addIntent(pActionIntentName, pIntentFormulations, _actionSemanticMemory, pLingDb, _language);
}


std::optional<ActionRecognizer::ActionRecognized> ActionRecognizer::recognize(UniqueSemanticExpression pUtteranceSemExp,
                                                                              const linguistics::LinguisticDatabase& pLingDb) {
    converter::addDifferentForms(pUtteranceSemExp, pLingDb);
    conditionsAdder::addConditonsForSomeTimedGrdExp(pUtteranceSemExp);

    std::optional<UniqueSemanticExpression> conditionSepExp;
    auto* condPtr = pUtteranceSemExp->getCondExpPtr_SkipWrapperPtrs();
    if (condPtr) {
        conditionSepExp.emplace(std::move(condPtr->conditionExp));
        pUtteranceSemExp = std::move(condPtr->thenExp);
    }

    // Function to extract the condition
    auto _extractCondtionIntent = [&]() -> std::optional<Intent> {
        if (conditionSepExp) {
            mystd::unique_propagate_const<UniqueSemanticExpression> conditionReaction;
            triggers::match(conditionReaction, _predicateSemanticMemory, std::move(*conditionSepExp), pLingDb);
            return _reactionToIntent(conditionReaction, _typeToMemory, pLingDb);
        };
        return {};
    };

    if (SemExpGetter::isACoreference(*pUtteranceSemExp, CoreferenceDirectionEnum::BEFORE)) {
        ActionRecognizer::ActionRecognized actionRecognized;
        actionRecognized.condition = _extractCondtionIntent();
        if (actionRecognized.empty())
            return {};
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
        auto actionIntentOpt = _reactionToIntent(reaction, _typeToMemory, pLingDb);
        if (actionIntentOpt) {
            ActionRecognizer::ActionRecognized actionRecognized;
            actionRecognized.action = std::move(*actionIntentOpt);
            actionRecognized.condition = _extractCondtionIntent();
            return actionRecognized;
        }
    }

    conditionSepExp = _extractConditionOnlySemExp(pUtteranceSemExp);
    if (conditionSepExp) {
        ActionRecognizer::ActionRecognized actionRecognized;
        actionRecognized.condition = _extractCondtionIntent();
        if (actionRecognized.empty())
            return {};
        return actionRecognized;
    }

    return {};
}



}    // End of namespace onsem
