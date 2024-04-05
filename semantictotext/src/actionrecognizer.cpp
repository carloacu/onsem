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


void _correferenceToRobot(UniqueSemanticExpression& pSemExp,
                          const linguistics::LinguisticDatabase& pLingDb) {
    switch (pSemExp->type) {
    case SemanticExpressionType::ANNOTATED: {
        AnnotatedExpression& annExp = pSemExp->getAnnExp();
        _correferenceToRobot(annExp.semExp, pLingDb);
        for (auto& currAnn : annExp.annotations)
            _correferenceToRobot(currAnn.second, pLingDb);
        break;
    }
    case SemanticExpressionType::COMMAND: {
        CommandExpression& cmdExp = pSemExp->getCmdExp();
        _correferenceToRobot(cmdExp.semExp, pLingDb);
        break;
    }
    case SemanticExpressionType::FEEDBACK: {
        FeedbackExpression& fdkExp = pSemExp->getFdkExp();
        _correferenceToRobot(fdkExp.concernedExp, pLingDb);
        break;
    }
    case SemanticExpressionType::GROUNDED: {
        GroundedExpression& grdExp = pSemExp->getGrdExp();
        if (grdExp.grounding().type == SemanticGroundingType::GENERIC) {
            SemanticGenericGrounding& genGrd = grdExp->getGenericGrounding();
            if (SemExpGetter::isASpecificHuman(genGrd) &&
                SemExpGetter::isACoreferenceFromGenericGrounding(genGrd, CoreferenceDirectionEnum::BEFORE)) {
                grdExp.moveGrounding(SemanticAgentGrounding::getRobotAgentPtr());
            }
        }
        if (grdExp.grounding().type == SemanticGroundingType::META) {
            SemanticMetaGrounding& metaGrd = grdExp->getMetaGrounding();
            if (metaGrd.attibuteName == "self") {
                grdExp.moveGrounding(SemanticAgentGrounding::getRobotAgentPtr());
            }
        }

        for (auto& child : grdExp.children) {
            _correferenceToRobot(child.second, pLingDb);
        }
        break;
    }
    case SemanticExpressionType::INTERPRETATION: {
        InterpretationExpression& intExp = pSemExp->getIntExp();
        _correferenceToRobot(intExp.interpretedExp, pLingDb);
        if (intExp.source == InterpretationSource::STATEMENTCOREFERENCE)
            _correferenceToRobot(intExp.originalExp, pLingDb);
        break;
    }
    case SemanticExpressionType::LIST: {
        ListExpression& listExp = pSemExp->getListExp();
        for (auto& elt : listExp.elts) {
            _correferenceToRobot(elt, pLingDb);
        }
        break;
    }
    case SemanticExpressionType::METADATA: {
        MetadataExpression& metaExp = pSemExp->getMetadataExp();
        if (metaExp.source) {
            _correferenceToRobot(*metaExp.source, pLingDb);
        }
        _correferenceToRobot(metaExp.semExp, pLingDb);
        break;
    }
    case SemanticExpressionType::CONDITION: {
        ConditionExpression& condExp = pSemExp->getCondExp();
        _correferenceToRobot(condExp.conditionExp, pLingDb);
        _correferenceToRobot(condExp.thenExp, pLingDb);
        if (condExp.elseExp) {
            _correferenceToRobot(*condExp.elseExp, pLingDb);
        }
        break;
    }
    case SemanticExpressionType::FIXEDSYNTHESIS: {
        FixedSynthesisExpression& fSynthExp = pSemExp->getFSynthExp();
        _correferenceToRobot(fSynthExp.getUSemExp(), pLingDb);
        break;
    }
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
        _correferenceToRobot(formulationSemExp, pLingDb);

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
                       SemanticMemory& pSemanticMemory,
                       const std::map<std::string, std::vector<UniqueSemanticExpression>>& pParameterLabelToQuestions,
                       const linguistics::LinguisticDatabase& pLingDb,
                       SemanticLanguageEnum pLanguage) {
    auto outputResourceGrdExp = std::make_unique<GroundedExpression>(converter::createResourceWithParametersFromSemExp(
                                                                         "intent", pActionIntentName, pParameterLabelToQuestions, *pFormulationSemExp, pLingDb, pLanguage));
    triggers::add(std::move(pFormulationSemExp), std::move(outputResourceGrdExp), pSemanticMemory, pLingDb);
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
      _typeToMemory() {
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
        _correferenceToRobot(formulationSemExp, pLingDb);

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

        _addSemExpTrigger(pActionIntentName, std::move(formulationSemExp), _actionSemanticMemory,
                          parameterLabelToQuestions, pLingDb, _language);
        for (auto& currOtherFormulaation : otherFormulationsSemExps)
            _addSemExpTrigger(pActionIntentName, std::move(currOtherFormulaation), _actionSemanticMemory,
                              parameterLabelToQuestions, pLingDb, _language);
    }
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
        }
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
