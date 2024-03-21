#include <onsem/semantictotext/actionrecognizer.hpp>
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
                         auto& strs = intent.slotNameToValues[currParametersToSemExps.first];
                         TextProcessingContext outContext(
                             SemanticAgentGrounding::currentUser, SemanticAgentGrounding::me, resourceGrdPtr->resource.language);
                         SemanticMemory semMemory;

                         for (auto& currAnswer : semExps) {
                             std::string subRes;
                             converter::semExpToText(subRes, currAnswer->clone(), outContext, true, semMemory, pLingDb, nullptr);
                             strs.push_back(subRes);
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


std::string ActionRecognizer::ActionRecognized::toJson() const {
    auto res = "{\"action\": \"" + action.toStr() + "\"";
    if (condition)
      res += ", \"condition\": \"" + condition->toStr() + "\"";
    return res + "}";
}




ActionRecognizer::ActionRecognizer()
    : _actionSemanticMemory(),
      _predicateSemanticMemory() {
}



void ActionRecognizer::addPredicate(const std::string& pPredicateName,
                                    const std::vector<std::string>& pPredicateFormulations,
                                    const linguistics::LinguisticDatabase& pLingDb,
                                    SemanticLanguageEnum pLanguage) {
    _addIntent(pPredicateName, pPredicateFormulations, _predicateSemanticMemory, pLingDb, pLanguage);
}


void ActionRecognizer::addAction(const std::string& pActionIntentName,
                                 const std::vector<std::string>& pIntentFormulations,
                                 const linguistics::LinguisticDatabase& pLingDb,
                                 SemanticLanguageEnum pLanguage) {
    _addIntent(pActionIntentName, pIntentFormulations, _actionSemanticMemory, pLingDb, pLanguage);
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

    static const InformationType informationType = InformationType::INFORMATION;
    std::unique_ptr<CompositeSemAnswer> compSemAnswers;
    SemanticMemory localSemanticMemory;
    auto expForMem = localSemanticMemory.memBloc.addRootSemExp(std::move(pUtteranceSemExp), pLingDb);
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

        auto actionIntentOpt = _reactionToIntent(reaction, pLingDb);
        if (actionIntentOpt) {
            ActionRecognizer::ActionRecognized actionRecognized;
            actionRecognized.action = std::move(*actionIntentOpt);

            // Extract condition
            if (conditionSepExp) {
                mystd::unique_propagate_const<UniqueSemanticExpression> conditionReaction;
                triggers::match(conditionReaction, _predicateSemanticMemory, std::move(*conditionSepExp), pLingDb);
                actionRecognized.condition = _reactionToIntent(conditionReaction, pLingDb);
            }
            return actionRecognized;
        }
    }

    return {};
}



}    // End of namespace onsem
