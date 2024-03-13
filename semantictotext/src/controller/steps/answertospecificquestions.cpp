#include "answertospecificquestions.hpp"
#include <onsem/texttosemantic/dbtype/semanticexpression/groundedexpression.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/interpretationexpression.hpp>
#include <onsem/texttosemantic/dbtype/semanticgroundings.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/conceptset.hpp>
#include <onsem/texttosemantic/tool/semexpmodifier.hpp>
#include <onsem/texttosemantic/tool/semexpgetter.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemory.hpp>
#include <onsem/semantictotext/semanticmemory/links/groundedexpwithlinks.hpp>
#include <onsem/semantictotext/semexpsimplifer.hpp>
#include <onsem/semantictotext/semanticconverter.hpp>
#include "../../semanticmemory/semanticmemoryblockviewer.hpp"
#include "../../semanticmemory/semanticmemoryblockprivate.hpp"
#include "../../utility/semexpcreator.hpp"
#include "../../type/referencesfiller.hpp"
#include "../../type/semanticdetailledanswer.hpp"
#include "../semexpcontroller.hpp"
#include "semanticmemorylinker.hpp"

namespace onsem {
namespace answerToSpecificQuestions {
namespace {

bool _tryToAnswerToDoYouKnow(SemControllerWorkingStruct& pWorkStruct,
                             SemanticMemoryBlockViewer& pMemViewer,
                             const GroundedExpression& pGrdExp) {
    bool askForMoreInformation = pWorkStruct.reactOperator == SemanticOperatorEnum::FEEDBACK
                              && pWorkStruct.typeOfFeedback == SemanticTypeOfFeedback::ASK_FOR_ADDITIONAL_INFORMATION;
    if (pWorkStruct.reactOperator != SemanticOperatorEnum::REACT
        && pWorkStruct.reactOperator != SemanticOperatorEnum::ANSWER
        && pWorkStruct.reactOperator != SemanticOperatorEnum::CHECK
        && pWorkStruct.reactOperator != SemanticOperatorEnum::GET && !askForMoreInformation)
        return false;

    const SemanticStatementGrounding* statGrdPtr = pGrdExp->getStatementGroundingPtr();
    if (statGrdPtr == nullptr || statGrdPtr->concepts.find("mentalState_know") == statGrdPtr->concepts.end())
        return false;

    const GroundedExpression* grdExpSubjectPtr = SemExpGetter::getGrdExpChild(pGrdExp, GrammaticalType::SUBJECT);
    if (grdExpSubjectPtr == nullptr)
        return false;
    const SemanticAgentGrounding* subjAgentGrdPtr = (*grdExpSubjectPtr)->getAgentGroundingPtr();
    if (subjAgentGrdPtr == nullptr || subjAgentGrdPtr->userId != SemanticAgentGrounding::me)
        return false;

    auto itObject = pGrdExp.children.find(GrammaticalType::OBJECT);
    if (itObject == pGrdExp.children.end())
        return false;

    SemControllerWorkingStruct subWorkStruct(pWorkStruct);
    if (subWorkStruct.askForNewRecursion()) {
        subWorkStruct.reactOperator = SemanticOperatorEnum::GET;
        controller::applyOperatorOnSemExp(subWorkStruct, pMemViewer, *itObject->second);

        std::map<GrammaticalType, const SemanticExpression*> annotationsOfTheAnswer;
        RelatedContextAxiom relatedContextAxiom;
        subWorkStruct.compositeSemAnswers->getSourceContextAxiom(relatedContextAxiom);
        bool answerPolarity = !subWorkStruct.compositeSemAnswers->semAnswers.empty();
        if (pWorkStruct.reactOperator == SemanticOperatorEnum::REACT
            || pWorkStruct.reactOperator == SemanticOperatorEnum::ANSWER) {
            pWorkStruct.addAnswer(
                ContextualAnnotation::ANSWER,
                SemExpCreator::generateYesOrNoAnswerFromQuestion(pGrdExp, answerPolarity, annotationsOfTheAnswer),
                ReferencesFiller(relatedContextAxiom));
        } else {
            pWorkStruct.compositeSemAnswers->semAnswers.emplace_back([&] {
                auto newAns = std::make_unique<LeafSemAnswer>(ContextualAnnotation::ANSWER);
                newAns->answerElts[SemanticRequestType::YESORNO].answersGenerated.emplace_back(
                    SemExpCreator::sayYesOrNo(answerPolarity), nullptr);
                return newAns;
            }());
        }
        if (!answerPolarity && (pWorkStruct.reactOperator == SemanticOperatorEnum::REACT || askForMoreInformation))
            pWorkStruct.addQuestion(SemExpCreator::askWhatIs(*itObject->second));
        return true;
    }

    return false;
}

bool _tryToAnswerAboutAbilities(SemControllerWorkingStruct& pWorkStruct,
                                SemanticMemoryBlockViewer& pMemViewer,
                                const GroundedExpression& pGrdExp) {
    if (pWorkStruct.reactOperator != SemanticOperatorEnum::REACT
        && pWorkStruct.reactOperator != SemanticOperatorEnum::ANSWER
        && pWorkStruct.reactOperator != SemanticOperatorEnum::GET)
        return false;

    const SemanticStatementGrounding* statGrdPtr = pGrdExp->getStatementGroundingPtr();
    if (statGrdPtr == nullptr || statGrdPtr->verbGoal != VerbGoalEnum::ABILITY
        || SemExpGetter::getUserIdOfSubject(pGrdExp) != SemanticAgentGrounding::me)
        return false;

    auto relatedCmdExp = pGrdExp.clone();
    GroundedExpression& cmdGrdExp = relatedCmdExp->getGrdExp();
    SemanticStatementGrounding& cmdStadGrd = cmdGrdExp->getStatementGrounding();
    cmdStadGrd.verbGoal = VerbGoalEnum::NOTIFICATION;
    cmdStadGrd.requests.set(SemanticRequestType::ACTION);

    SemControllerWorkingStruct subWorkStruct(pWorkStruct);
    if (subWorkStruct.askForNewRecursion()) {
        subWorkStruct.reactOperator = SemanticOperatorEnum::RESOLVECOMMAND;
        controller::applyOperatorOnSemExp(subWorkStruct, pMemViewer, *relatedCmdExp);
        bool doTheRobotCanDoTheAction = subWorkStruct.haveAnAnswer();

        std::unique_ptr<GroundedExpression> IcanSemExp = SemExpCreator::sayICan(doTheRobotCanDoTheAction);
        if (pWorkStruct.reactOperator == SemanticOperatorEnum::REACT) {
            RelatedContextAxiom relatedContextAxiom;
            subWorkStruct.getSourceContextAxiom(relatedContextAxiom);
            if (doTheRobotCanDoTheAction) {
                pWorkStruct.addAnswer(
                    ContextualAnnotation::ANSWER,
                    SemExpCreator::generateYesOrNoAnswer(std::move(IcanSemExp), doTheRobotCanDoTheAction),
                    ReferencesFiller(relatedContextAxiom));
                pWorkStruct.addQuestion(SemExpCreator::askDoYouWantMeToDoItNow(*pWorkStruct.author, cmdGrdExp));
            } else if (pWorkStruct.proativeSpecificationsPtr != nullptr
                       && pWorkStruct.proativeSpecificationsPtr->informTheUserHowToTeachMe) {
                SemExpCreator::addButYouCanTeachMe(*IcanSemExp, *pWorkStruct.author);
                pWorkStruct.addAnswer(
                    ContextualAnnotation::ANSWER,
                    SemExpCreator::generateYesOrNoAnswer(std::move(IcanSemExp), doTheRobotCanDoTheAction),
                    ReferencesFiller(relatedContextAxiom));
                cmdStadGrd.requests.clear();
                cmdStadGrd.verbTense = SemanticVerbTense::UNKNOWN;
                SemExpModifier::removeChild(cmdGrdExp, GrammaticalType::SUBJECT);
                pWorkStruct.addQuestion(
                    SemExpCreator::askDoYouWantToKnowHow(*pWorkStruct.author, std::move(relatedCmdExp)));
            } else {
                pWorkStruct.addAnswer(
                    ContextualAnnotation::ANSWER,
                    SemExpCreator::generateYesOrNoAnswer(std::move(IcanSemExp), doTheRobotCanDoTheAction),
                    ReferencesFiller(relatedContextAxiom));
            }
        }
    }
    return true;
}

bool _tryToAnswerToBeVerbNameQuestion(
    const GroundedExpression& pGrdExp,
    const std::function<bool(const std::string&, UniqueSemanticExpression, const std::string&)>& pAnswerNameQuestion) {
    const GroundedExpression* grdExpSubjectPtr = SemExpGetter::getGrdExpChild(pGrdExp, GrammaticalType::SUBJECT);
    if (grdExpSubjectPtr == nullptr || !ConceptSet::haveAConcept(grdExpSubjectPtr->grounding().concepts, "name"))
        return false;

    auto itOwner = grdExpSubjectPtr->children.find(GrammaticalType::OWNER);
    if (itOwner == grdExpSubjectPtr->children.end())
        return false;
    const auto* ownerGrdExpPtr = SemExpGetter::getUnnamedGrdExpPtr(*itOwner->second);
    if (ownerGrdExpPtr != nullptr) {
        const auto* ownerAgentGrdPtr = ownerGrdExpPtr->grounding().getAgentGroundingPtr();
        if (ownerAgentGrdPtr != nullptr)
            return pAnswerNameQuestion(
                ownerAgentGrdPtr->userId, grdExpSubjectPtr->clone(), ConceptSet::conceptVerbEquality);
    }
    return false;
}

bool _tryToAnswerToHasNameVerbNameQuestion(
    const GroundedExpression& pGrdExp,
    const std::function<bool(const std::string&, UniqueSemanticExpression, const std::string&)>& pAnswerNameQuestion) {
    auto itSubject = pGrdExp.children.find(GrammaticalType::SUBJECT);
    if (itSubject == pGrdExp.children.end())
        return false;
    const auto* grdExpSubjectPtr = SemExpGetter::getUnnamedGrdExpPtr(*itSubject->second);
    if (grdExpSubjectPtr == nullptr)
        return false;

    const auto* subjectAgentGrdPtr = grdExpSubjectPtr->grounding().getAgentGroundingPtr();
    if (subjectAgentGrdPtr != nullptr)
        return pAnswerNameQuestion(subjectAgentGrdPtr->userId, grdExpSubjectPtr->clone(), "predicate_hasName");
    return false;
}

bool _tryToAnswerToNameQuestion(
    const GroundedExpression& pGrdExp,
    const std::function<bool(const std::string&, UniqueSemanticExpression, const std::string&)>& pAnswerNameQuestion) {
    const SemanticStatementGrounding* statGrdPtr = pGrdExp->getStatementGroundingPtr();
    if (statGrdPtr == nullptr)
        return false;

    if (ConceptSet::haveAConcept(statGrdPtr->concepts, ConceptSet::conceptVerbEquality))
        return _tryToAnswerToBeVerbNameQuestion(pGrdExp, pAnswerNameQuestion);
    if (ConceptSet::haveAConcept(statGrdPtr->concepts, "predicate_hasName"))
        return _tryToAnswerToHasNameVerbNameQuestion(pGrdExp, pAnswerNameQuestion);
    return false;
}

bool _checkNameBeVerb(SemControllerWorkingStruct& pWorkStruct,
                      SemanticMemoryBlockViewer& pMemViewer,
                      const GroundedExpression& pGrdExp,
                      bool pStatementIsPositive,
                      const SemanticExpression& pSubjectSemExp,
                      const SemanticExpression& pObjectSemExp) {
    const GroundedExpression* grdExpSubjectPtr = pSubjectSemExp.getGrdExpPtr_SkipWrapperPtrs();
    if (grdExpSubjectPtr == nullptr)
        return false;

    const auto& subjectGrd = grdExpSubjectPtr->grounding();
    const SemanticAgentGrounding* authorAgentGrdPtr = nullptr;
    if (ConceptSet::haveAConcept(subjectGrd.concepts, "name")) {
        auto itOwner = grdExpSubjectPtr->children.find(GrammaticalType::OWNER);
        if (itOwner == grdExpSubjectPtr->children.end())
            return false;
        const auto* ownerGrdExpPtr = SemExpGetter::getUnnamedGrdExpPtr(*itOwner->second);
        if (ownerGrdExpPtr == nullptr)
            return false;
        authorAgentGrdPtr = ownerGrdExpPtr->grounding().getAgentGroundingPtr();
    } else {
        authorAgentGrdPtr = subjectGrd.getAgentGroundingPtr();
    }

    if (authorAgentGrdPtr == nullptr)
        return false;
    auto& authorAgentGrd = *authorAgentGrdPtr;

    const GroundedExpression* grdExpObjectPtr = pObjectSemExp.getGrdExpPtr_SkipWrapperPtrs();
    if (grdExpObjectPtr == nullptr)
        return false;
    auto* objectAgentGrdPtr = grdExpObjectPtr->grounding().getAgentGroundingPtr();
    if (objectAgentGrdPtr != nullptr) {
        RelatedContextAxiom relContextAxiom;
        if (pMemViewer.areSameUser(objectAgentGrdPtr->userId, authorAgentGrd.userId, relContextAxiom)) {
            if (pWorkStruct.reactOperator == SemanticOperatorEnum::ANSWER
                || pWorkStruct.reactOperator == SemanticOperatorEnum::REACT) {
                pWorkStruct.addAnswer(
                    ContextualAnnotation::ANSWER,
                    SemExpCreator::generateYesOrNoAnswerFromQuestion(pGrdExp, pStatementIsPositive, {}),
                    ReferencesFiller(relContextAxiom));
            } else {
                pWorkStruct.compositeSemAnswers->semAnswers.emplace_back([&] {
                    auto newAns = std::make_unique<LeafSemAnswer>(ContextualAnnotation::ANSWER);
                    newAns->answerElts[SemanticRequestType::YESORNO].answersGenerated.emplace_back(
                        SemExpCreator::sayYesOrNo(pStatementIsPositive), &relContextAxiom);
                    return newAns;
                }());
            }
            return true;
        }
    } else if (grdExpObjectPtr->grounding().getNameGroundingPtr() == nullptr) {
        return false;
    }

    const SemanticMemoryGrdExp* semMemoryGrdExpPtr = nullptr;
    auto nameGrd = pMemViewer.getConstViewPrivate().getNameGrd(authorAgentGrd.userId, semMemoryGrdExpPtr);
    if (!nameGrd)
        return false;
    UniqueSemanticExpression nameSemExp = std::make_unique<GroundedExpression>(std::move(nameGrd));
    RelatedContextAxiom relContextAxiom;
    if (semMemoryGrdExpPtr != nullptr)
        relContextAxiom.add(*semMemoryGrdExpPtr);
    if (pWorkStruct.reactOperator == SemanticOperatorEnum::ANSWER
        || pWorkStruct.reactOperator == SemanticOperatorEnum::REACT) {
        UniqueSemanticExpression answerSemExp = SemExpCreator::generateYesOrNoAnswer(
            SemExpCreator::sentenceFromTriple(
                pSubjectSemExp.clone(), ConceptSet::conceptVerbEquality, std::move(nameSemExp)),
            !pStatementIsPositive);
        pWorkStruct.addAnswer(ContextualAnnotation::ANSWER, std::move(answerSemExp), ReferencesFiller(relContextAxiom));
    } else {
        pWorkStruct.compositeSemAnswers->semAnswers.emplace_back([&] {
            auto newAns = std::make_unique<LeafSemAnswer>(ContextualAnnotation::ANSWER);
            newAns->answerElts[SemanticRequestType::OBJECT].answersGenerated.emplace_back(
                SemExpCreator::sayYesOrNo(!pStatementIsPositive), &relContextAxiom);
            return newAns;
        }());
    }
    return true;
}

bool _checkName(SemControllerWorkingStruct& pWorkStruct,
                SemanticMemoryBlockViewer& pMemViewer,
                const GroundedExpression& pGrdExp) {
    if (pWorkStruct.reactOperator != SemanticOperatorEnum::REACT
        && pWorkStruct.reactOperator != SemanticOperatorEnum::ANSWER
        && pWorkStruct.reactOperator != SemanticOperatorEnum::GET
        && pWorkStruct.reactOperator != SemanticOperatorEnum::CHECK)
        return false;

    const SemanticStatementGrounding* statGrdPtr = pGrdExp->getStatementGroundingPtr();
    if (statGrdPtr == nullptr)
        return false;

    if (ConceptSet::haveAConcept(statGrdPtr->concepts, ConceptSet::conceptVerbEquality)) {
        const SemanticExpression* subjectSemExpPtr =
            SemExpGetter::getChildFromGrdExp(pGrdExp, GrammaticalType::SUBJECT);
        if (subjectSemExpPtr == nullptr)
            return false;
        const SemanticExpression* objectSemExpPtr = SemExpGetter::getChildFromGrdExp(pGrdExp, GrammaticalType::OBJECT);
        if (objectSemExpPtr == nullptr)
            return false;
        return _checkNameBeVerb(
                   pWorkStruct, pMemViewer, pGrdExp, statGrdPtr->polarity, *subjectSemExpPtr, *objectSemExpPtr)
            || _checkNameBeVerb(
                   pWorkStruct, pMemViewer, pGrdExp, statGrdPtr->polarity, *objectSemExpPtr, *subjectSemExpPtr);
    }
    if (ConceptSet::haveAConcept(statGrdPtr->concepts, "predicate_hasName")) {
        const SemanticExpression* subjectSemExpPtr =
            SemExpGetter::getChildFromGrdExp(pGrdExp, GrammaticalType::SUBJECT);
        if (subjectSemExpPtr == nullptr)
            return false;
        const SemanticExpression* objectSemExpPtr = SemExpGetter::getChildFromGrdExp(pGrdExp, GrammaticalType::OBJECT);
        if (objectSemExpPtr == nullptr)
            return false;
        return _checkNameBeVerb(
            pWorkStruct, pMemViewer, pGrdExp, statGrdPtr->polarity, *subjectSemExpPtr, *objectSemExpPtr);
    }
    return false;
}

bool _canYouQuestions(SemControllerWorkingStruct& pWorkStruct,
                      SemanticMemoryBlockViewer& pMemViewer,
                      const GroundedExpression& pGrdExp) {
    if (pWorkStruct.reactOperator != SemanticOperatorEnum::REACT
        && pWorkStruct.reactOperator != SemanticOperatorEnum::REACTFROMTRIGGER)
        return false;

    const auto* statGrdPtr = pGrdExp->getStatementGroundingPtr();
    if (statGrdPtr != nullptr && statGrdPtr->verbGoal == VerbGoalEnum::ABILITY
        && statGrdPtr->requests.has(SemanticRequestType::YESORNO)
        && SemExpGetter::getUserIdOfSubject(pGrdExp) == SemanticAgentGrounding::me) {
        SemControllerWorkingStruct subWorkStruct(pWorkStruct);
        if (subWorkStruct.askForNewRecursion()) {
            subWorkStruct.reactOperator = SemanticOperatorEnum::REACTFROMTRIGGER;
            auto grdExp = pGrdExp.clone();
            auto* clonedStatementPtr = grdExp->grounding().getStatementGroundingPtr();
            if (clonedStatementPtr == nullptr)
                return false;
            auto& clonedStatement = *clonedStatementPtr;
            clonedStatement.requests.set(SemanticRequestType::ACTION);
            clonedStatement.verbGoal = VerbGoalEnum::NOTIFICATION;

            controller::applyOperatorOnGrdExp(subWorkStruct, pMemViewer, *grdExp, {}, *grdExp);
            return pWorkStruct.addAnswers(subWorkStruct);
        }
    }
    return false;
}

bool _tryToAnswerToWhoIsQuestion(
    const GroundedExpression& pGrdExp,
    const SemanticExpression& pObjectSemExp,
    const std::function<bool(const std::string&, UniqueSemanticExpression, const std::string&)>& pAnswerNameQuestion) {
    const auto* objectGrdExpPtr = pObjectSemExp.getGrdExpPtr_SkipWrapperPtrs();
    if (objectGrdExpPtr == nullptr)
        return false;
    const auto* objectGenGrdPtr = objectGrdExpPtr->grounding().getGenericGroundingPtr();
    if (objectGenGrdPtr == nullptr || !ConceptSet::haveAnyOfConcepts(objectGenGrdPtr->concepts, {"agent", "name"}))
        return false;

    const SemanticStatementGrounding* statGrdPtr = pGrdExp->getStatementGroundingPtr();
    if (statGrdPtr == nullptr || !ConceptSet::haveAConcept(statGrdPtr->concepts, ConceptSet::conceptVerbEquality))
        return false;

    auto itSubject = pGrdExp.children.find(GrammaticalType::SUBJECT);
    if (itSubject == pGrdExp.children.end())
        return false;
    const auto* grdExpSubjectPtr = SemExpGetter::getUnnamedGrdExpPtr(*itSubject->second);
    if (grdExpSubjectPtr == nullptr)
        return false;
    const auto* subjectAgentGrdPtr = grdExpSubjectPtr->grounding().getAgentGroundingPtr();
    if (subjectAgentGrdPtr != nullptr)
        return pAnswerNameQuestion(
            subjectAgentGrdPtr->userId, grdExpSubjectPtr->clone(), ConceptSet::conceptVerbEquality);
    return false;
}

bool _iterateOnBornTimes(
    SemControllerWorkingStruct& pWorkStruct,
    SemanticMemoryBlockViewer& pMemViewer,
    UniqueSemanticExpression pUserSemExp,
    const std::function<
        bool(const SemanticTimeGrounding&, SemControllerWorkingStruct&, const AnswerExp&, UniqueSemanticExpression&)>&
        pOnEachTimeGrd) {
    SemControllerWorkingStruct subWorkStruct(pWorkStruct);
    if (subWorkStruct.askForNewRecursion()) {
        // search for birthdate of the owner
        auto statementGrd = std::make_unique<SemanticStatementGrounding>();
        statementGrd->verbTense = SemanticVerbTense::PUNCTUALPAST;
        statementGrd->concepts.emplace("verb_born", 4);
        statementGrd->requests.set(SemanticRequestType::TIME);
        auto newRootExp = std::make_unique<GroundedExpression>(std::move(statementGrd));
        GrammaticalType agentBornedGrammType = GrammaticalType::OBJECT;
        newRootExp->children.emplace(agentBornedGrammType, std::move(pUserSemExp));

        static const SemanticRequests timeRequest(SemanticRequestType::TIME);
        subWorkStruct.reactOperator = SemanticOperatorEnum::GET;
        semanticMemoryLinker::satisfyAQuestion(subWorkStruct, pMemViewer, *newRootExp, {}, *newRootExp, timeRequest);

        std::list<const AnswerExp*> timeAnswers;
        subWorkStruct.getAnswersForRequest(timeAnswers, timeRequest);
        if (!timeAnswers.empty()) {
            pUserSemExp = std::move(newRootExp->children.find(agentBornedGrammType)->second);
            for (const auto& currTimeAnsw : timeAnswers) {
                const SemanticTimeGrounding* timeGrd = currTimeAnsw->getGrdExp()->getTimeGroundingPtr();
                if (timeGrd != nullptr && pOnEachTimeGrd(*timeGrd, subWorkStruct, *currTimeAnsw, pUserSemExp))
                    return true;
            }
        }
    }
    return false;
}

bool _tryToAnswerToAnAgeQuestion(SemControllerWorkingStruct& pWorkStruct,
                                 SemanticMemoryBlockViewer& pMemViewer,
                                 const GroundedExpression& pGrdExp) {
    if (pWorkStruct.reactOperator != SemanticOperatorEnum::REACT
        && pWorkStruct.reactOperator != SemanticOperatorEnum::ANSWER
        && pWorkStruct.reactOperator != SemanticOperatorEnum::GET)
        return false;

    const GroundedExpression* grdExpSubject = SemExpGetter::getGrdExpChild(pGrdExp, GrammaticalType::SUBJECT);
    if (grdExpSubject == nullptr || !ConceptSet::haveAConcept(grdExpSubject->grounding().concepts, "age"))
        return false;
    auto itOwner = grdExpSubject->children.find(GrammaticalType::OWNER);
    if (itOwner == grdExpSubject->children.end())
        return false;

    return _iterateOnBornTimes(
        pWorkStruct,
        pMemViewer,
        itOwner->second->clone(),
        [&](const SemanticTimeGrounding& pTimeGrd,
            SemControllerWorkingStruct&,
            const AnswerExp& pAnswExp,
            UniqueSemanticExpression& pUserSemExp) {
            auto ageGrd = std::make_unique<SemanticGenericGrounding>();
            ageGrd->concepts.emplace("age_*", 4);
            ageGrd->quantity.setNumber(pTimeGrd.date.getAge());
            auto ageSemExp = std::make_unique<GroundedExpression>(std::move(ageGrd));

            if (pWorkStruct.reactOperator == SemanticOperatorEnum::ANSWER
                || pWorkStruct.reactOperator == SemanticOperatorEnum::REACT) {
                pWorkStruct.addAnswer(ContextualAnnotation::ANSWER,
                                      SemExpCreator::formulateAge(std::move(pUserSemExp), std::move(ageSemExp)),
                                      ReferencesFiller(pAnswExp.relatedContextAxioms));
            } else {
                pWorkStruct.compositeSemAnswers->semAnswers.emplace_back([&ageSemExp] {
                    auto newAns = std::make_unique<LeafSemAnswer>(ContextualAnnotation::ANSWER);
                    newAns->answerElts[SemanticRequestType::OBJECT].answersGenerated.emplace_back(std::move(ageSemExp));
                    return newAns;
                }());
            }
            return true;
        });
}

bool _tryToAnswerToBirthdateQuestion(SemControllerWorkingStruct& pWorkStruct,
                                     SemanticMemoryBlockViewer& pMemViewer,
                                     const GroundedExpression& pGrdExp) {
    if (pWorkStruct.reactOperator != SemanticOperatorEnum::REACT
        && pWorkStruct.reactOperator != SemanticOperatorEnum::ANSWER
        && pWorkStruct.reactOperator != SemanticOperatorEnum::GET)
        return false;

    auto itSubject = pGrdExp.children.find(GrammaticalType::SUBJECT);
    if (itSubject == pGrdExp.children.end())
        return false;
    const auto& semExpSubject = *itSubject->second;
    const auto* grdExpSubjectPtr = semExpSubject.getGrdExpPtr();
    if (grdExpSubjectPtr == nullptr)
        return false;
    const auto& grdExpSubject = *grdExpSubjectPtr;
    if (!ConceptSet::haveAConceptThatBeginWith(grdExpSubject->concepts, "time_birthda"))
        return false;
    bool askForBirthday = false;
    if (ConceptSet::haveAConcept(grdExpSubject->concepts, "time_birthday"))
        askForBirthday = true;
    else if (!ConceptSet::haveAConcept(grdExpSubject->concepts, "time_birthdate"))
        return false;
    auto itOwner = grdExpSubject.children.find(GrammaticalType::OWNER);
    if (itOwner == grdExpSubject.children.end())
        return false;

    return _iterateOnBornTimes(
        pWorkStruct,
        pMemViewer,
        itOwner->second->clone(),
        [&](const SemanticTimeGrounding& pTimeGrd,
            SemControllerWorkingStruct&,
            const AnswerExp& pAnswExp,
            UniqueSemanticExpression&) {
            auto timeGrd = std::make_unique<SemanticTimeGrounding>();
            timeGrd->date = pTimeGrd.date;
            if (askForBirthday)
                timeGrd->date.year.reset();
            if (!timeGrd->date.empty()) {
                auto timeGrdExp = std::make_unique<GroundedExpression>(std::move(timeGrd));

                if (pWorkStruct.reactOperator == SemanticOperatorEnum::ANSWER
                    || pWorkStruct.reactOperator == SemanticOperatorEnum::REACT) {
                    pWorkStruct.addAnswer(
                        ContextualAnnotation::ANSWER,
                        SemExpCreator::saySemxExp1IsSemExp2(semExpSubject.clone(), std::move(timeGrdExp)),
                        ReferencesFiller(pAnswExp.relatedContextAxioms));
                } else {
                    pWorkStruct.compositeSemAnswers->semAnswers.emplace_back([&] {
                        auto newAns = std::make_unique<LeafSemAnswer>(ContextualAnnotation::ANSWER);
                        newAns->answerElts[SemanticRequestType::OBJECT].answersGenerated.emplace_back(
                            std::move(timeGrdExp), &pAnswExp.relatedContextAxioms);
                        return newAns;
                    }());
                }
                return true;
            }
            return false;
        });

    return false;
}

bool _tryToAnswerToWhatCanYouDo(SemControllerWorkingStruct& pWorkStruct,
                                const SemanticMemoryBlock& pMemBlock,
                                const GroundedExpression& pGrdExp) {
    if (pWorkStruct.reactOperator != SemanticOperatorEnum::REACT
        && pWorkStruct.reactOperator != SemanticOperatorEnum::ANSWER)
        return false;

    const SemanticStatementGrounding* statGrdPtr = pGrdExp->getStatementGroundingPtr();
    if (statGrdPtr == nullptr || statGrdPtr->verbGoal != VerbGoalEnum::ABILITY
        || statGrdPtr->concepts.count("verb_action") == 0
        || SemExpGetter::getUserIdOfSubject(pGrdExp) != SemanticAgentGrounding::me)
        return false;

    const std::map<intSemId, const GroundedExpWithLinks*>& infActions = pMemBlock.getInfActions();
    UniqueSemanticExpression res;
    for (const auto& currInfAction : infActions) {
        auto actionGrdExp = currInfAction.second->grdExp.clone();
        SemanticStatementGrounding* statActionPtr = actionGrdExp->grounding().getStatementGroundingPtr();
        if (statActionPtr != nullptr) {
            statActionPtr->verbTense = SemanticVerbTense::PRESENT;
            statActionPtr->verbGoal = VerbGoalEnum::ABILITY;
        }
        actionGrdExp->children.emplace(
            GrammaticalType::SUBJECT, std::make_unique<GroundedExpression>(SemanticAgentGrounding::getRobotAgentPtr()));
        if (res->isEmpty())
            res = std::move(actionGrdExp);
        else
            SemExpModifier::addNewSemExp(res, std::move(actionGrdExp), ListExpressionType::AND);
    }
    simplifier::processFromMemBlock(res, pMemBlock, pWorkStruct.lingDb);
    pWorkStruct.addAnswerWithoutReferences(ContextualAnnotation::ANSWER, std::move(res));
    return !infActions.empty();
}

bool _tryToAnswerToWhatDoYouKnowAbout(SemControllerWorkingStruct& pWorkStruct,
                                      SemanticMemoryBlockViewer& pMemViewer,
                                      const GroundedExpression& pGrdExp) {
    if (pWorkStruct.reactOperator != SemanticOperatorEnum::REACT
        && pWorkStruct.reactOperator != SemanticOperatorEnum::ANSWER
        && pWorkStruct.reactOperator != SemanticOperatorEnum::GET)
        return false;

    const SemanticStatementGrounding* statGrdPtr = pGrdExp->getStatementGroundingPtr();
    if (statGrdPtr == nullptr || statGrdPtr->verbGoal != VerbGoalEnum::NOTIFICATION
        || statGrdPtr->concepts.count("mentalState_know") == 0
        || SemExpGetter::getUserIdOfSubject(pGrdExp) != SemanticAgentGrounding::me)
        return false;

    bool res = false;
    auto itTopic = pGrdExp.children.find(GrammaticalType::TOPIC);
    if (itTopic != pGrdExp.children.end()) {
        std::list<const GroundedExpression*> topicGrdExpsPtr;
        itTopic->second->getGrdExpPtrs_SkipWrapperLists(topicGrdExpsPtr);
        for (const auto& currTopic : topicGrdExpsPtr) {
            if (currTopic != nullptr) {
                auto* agentGrdPtr = currTopic->grounding().getAgentGroundingPtr();
                if (agentGrdPtr != nullptr && agentGrdPtr->isSpecificUser()) {
                    SemControllerWorkingStruct subWorkStruct(pWorkStruct);
                    if (subWorkStruct.askForNewRecursion()) {
                        auto semExpWhoIs = SemExpCreator::askWhoIs(*currTopic);
                        converter::addDifferentForms(semExpWhoIs, subWorkStruct.lingDb);
                        controller::applyOperatorOnSemExp(subWorkStruct, pMemViewer, *semExpWhoIs);
                        if (subWorkStruct.haveAnAnswer()) {
                            pWorkStruct.addAnswers(subWorkStruct);
                            res = true;
                        }
                    }
                }
            }
        }
    }
    return res;
}

bool _tryToAnswerToWhatIsAction(SemControllerWorkingStruct& pWorkStruct,
                                SemanticMemoryBlockViewer& pMemViewer,
                                const GroundedExpression& pGrdExp) {
    if (pWorkStruct.reactOperator != SemanticOperatorEnum::REACT
        && pWorkStruct.reactOperator != SemanticOperatorEnum::ANSWER)
        return false;

    const SemanticStatementGrounding* statGrdPtr = pGrdExp->getStatementGroundingPtr();
    if (statGrdPtr == nullptr || !ConceptSet::haveAConceptThatBeginWith(statGrdPtr->concepts, "verb_equal_"))
        return false;

    if (pGrdExp.children.count(GrammaticalType::SUBJECT) != 0)
        return false;

    auto itObject = pGrdExp.children.find(GrammaticalType::OBJECT);
    if (itObject == pGrdExp.children.end())
        return false;
    const SemanticExpression& objectSemExp = *itObject->second;
    const GroundedExpression* grdExpObjectPtr = objectSemExp.getGrdExpPtr_SkipWrapperPtrs();
    if (grdExpObjectPtr == nullptr)
        return false;
    const GroundedExpression& grdExpObject = *grdExpObjectPtr;
    const SemanticStatementGrounding* objectStatPtr = grdExpObject->getStatementGroundingPtr();
    if (objectStatPtr == nullptr || objectStatPtr->verbTense != SemanticVerbTense::UNKNOWN)
        return false;

    const SemanticExpression* infCommandToDo =
        semanticMemoryLinker::getActionComposition(pWorkStruct, pMemViewer, grdExpObject);
    if (infCommandToDo == nullptr)
        return false;
    pWorkStruct.addAnswerWithoutReferences(
        ContextualAnnotation::ANSWER,
        SemExpCreator::saySemxExp1IsSemExp2(objectSemExp.clone(), infCommandToDo->clone()));
    return true;
}

bool _tryToAnswerToHowWeKnowSomething(SemControllerWorkingStruct& pWorkStruct,
                                      SemanticMemoryBlockViewer& pMemViewer,
                                      const GroundedExpression& pGrdExp) {
    if (pWorkStruct.reactOperator != SemanticOperatorEnum::REACT
        && pWorkStruct.reactOperator != SemanticOperatorEnum::ANSWER
        && pWorkStruct.reactOperator != SemanticOperatorEnum::GET)
        return false;

    // an object has to exist in the question
    auto itObject = pGrdExp.children.find(GrammaticalType::OBJECT);
    if (itObject == pGrdExp.children.end())
        return false;

    // the subject has to be the robot himself
    const GroundedExpression* grdExpSubjectPtr = SemExpGetter::getGrdExpChild(pGrdExp, GrammaticalType::SUBJECT);
    if (grdExpSubjectPtr != nullptr) {
        const SemanticAgentGrounding* agentSubject = (*grdExpSubjectPtr)->getAgentGroundingPtr();
        if (agentSubject != nullptr && agentSubject->userId == SemanticAgentGrounding::me) {
            // the verb has to have the concept "mentalState_know"
            const auto& grdExpConcepts = pGrdExp->concepts;
            if (grdExpConcepts.find("mentalState_know") != grdExpConcepts.end()) {
                const GroundedExpression* objectGrdExp = itObject->second->getGrdExpPtr_SkipWrapperPtrs();
                if (objectGrdExp != nullptr) {
                    const SemanticStatementGrounding* objectStatementGrd = (*objectGrdExp)->getStatementGroundingPtr();
                    if (objectStatementGrd != nullptr) {
                        SemControllerWorkingStruct subWorkStruct(pWorkStruct);
                        if (subWorkStruct.askForNewRecursion()) {
                            subWorkStruct.reactOperator = SemanticOperatorEnum::HOWYOUKNOW;
                            controller::applyOperatorOnSemExp(subWorkStruct, pMemViewer, *itObject->second);

                            auto& subDetAnswers = subWorkStruct.compositeSemAnswers->semAnswers;
                            if (!subDetAnswers.empty()) {
                                if (pWorkStruct.reactOperator == SemanticOperatorEnum::REACT
                                    || pWorkStruct.reactOperator == SemanticOperatorEnum::ANSWER) {
                                    for (const auto& currDetAnsw : subDetAnswers) {
                                        const LeafSemAnswer* leafPtr = currDetAnsw->getLeafPtr();
                                        if (leafPtr != nullptr) {
                                            for (const auto& currAnswerElts : leafPtr->answerElts) {
                                                const AllAnswerElts& answerElt = currAnswerElts.second;
                                                if (!answerElt.answersGenerated.empty()) {
                                                    pWorkStruct.addAnswer(
                                                        ContextualAnnotation::ANSWER,
                                                        SemExpCreator::formulateHowWeKnowSomething(
                                                            *itObject->second,
                                                            *answerElt.answersGenerated.front().genSemExp),
                                                        ReferencesFiller(answerElt));
                                                    return true;
                                                }
                                            }
                                        }
                                    }
                                } else {
                                    pWorkStruct.addAnswers(subWorkStruct);
                                    return true;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return false;
}

bool _tryToAnswerToHowCanITeachYouSomething(SemControllerWorkingStruct& pWorkStruct,
                                            const GroundedExpression& pGrdExp) {
    if (pWorkStruct.reactOperator != SemanticOperatorEnum::REACT
        && pWorkStruct.reactOperator != SemanticOperatorEnum::ANSWER)
        return false;

    const SemanticStatementGrounding* statGrdPtr = pGrdExp->getStatementGroundingPtr();
    if (statGrdPtr == nullptr || statGrdPtr->verbGoal != VerbGoalEnum::ABILITY
        || statGrdPtr->verbTense != SemanticVerbTense::PRESENT || statGrdPtr->concepts.count("verb_action_teach") == 0)
        return false;

    auto itSubject = pGrdExp.children.find(GrammaticalType::SUBJECT);
    if (itSubject == pGrdExp.children.end() || pWorkStruct.author == nullptr
        || !SemExpGetter::isSemExpEqualToAnAgent(*itSubject->second, *pWorkStruct.author))
        return false;

    auto itReceiver = pGrdExp.children.find(GrammaticalType::RECEIVER);
    if (itReceiver == pGrdExp.children.end()
        || !SemExpGetter::doSemExpHoldUserId(*itReceiver->second, SemanticAgentGrounding::me))
        return false;

    auto itObject = pGrdExp.children.find(GrammaticalType::OBJECT);
    if (itObject != pGrdExp.children.end()) {
        const GroundedExpression* objectGrdExpPtr = itObject->second->getGrdExpPtr_SkipWrapperPtrs();
        if (objectGrdExpPtr != nullptr) {
            const GroundedExpression& objectGrdExp = *objectGrdExpPtr;
            const SemanticStatementGrounding* objectStatGrdPtr = objectGrdExp->getStatementGroundingPtr();
            if (objectStatGrdPtr != nullptr && objectStatGrdPtr->verbTense == SemanticVerbTense::UNKNOWN) {
                pWorkStruct.addAnswerWithoutReferences(
                    ContextualAnnotation::ANSWER,
                    SemExpCreator::forExampleSayToDoMeansToSayIDo(*pWorkStruct.author, objectGrdExp));
                return true;
            }
        }
    }
    return false;
}

bool _tryToAnswerToHowToDoAnAction(SemControllerWorkingStruct& pWorkStruct,
                                   SemanticMemoryBlockViewer& pMemViewer,
                                   const GroundedExpression& pGrdExp) {
    if (pWorkStruct.reactOperator != SemanticOperatorEnum::REACT
        && pWorkStruct.reactOperator != SemanticOperatorEnum::ANSWER)
        return false;

    const SemanticStatementGrounding* statGrdPtr = pGrdExp->getStatementGroundingPtr();
    if (statGrdPtr == nullptr || statGrdPtr->verbTense != SemanticVerbTense::UNKNOWN)
        return false;

    if (pGrdExp.children.count(GrammaticalType::SUBJECT) > 0)
        return false;

    auto* actionDefPtr = semanticMemoryLinker::getActionActionDefinition(pWorkStruct, pMemViewer, pGrdExp);
    if (actionDefPtr != nullptr) {
        auto& actionDef = *actionDefPtr;
        auto actionDefCloned = actionDef.clone();
        auto* actionDefClonedGrdExpPtr = actionDefCloned->getGrdExpPtr_SkipWrapperPtrs();
        if (actionDefClonedGrdExpPtr != nullptr) {
            SemExpModifier::infGrdExpToMandatoryForm(*actionDefClonedGrdExpPtr);
            pWorkStruct.addAnswerWithoutReferences(
                ContextualAnnotation::ANSWER,
                converter::constructTeachSemExp(pGrdExp.clone(), std::move(actionDefCloned)));
        } else {
            auto* listExpPtr = actionDefCloned->getListExpPtr_SkipWrapperPtrs();
            if (listExpPtr != nullptr) {
                auto& listExp = *listExpPtr;
                int nbOfElts = listExp.elts.size();
                if (nbOfElts > 1) {
                    auto answerListExp = std::make_unique<ListExpression>();
                    answerListExp->elts.emplace_back(SemExpCreator::thereIsXStepsFor(nbOfElts, pGrdExp.clone()));
                    if (pWorkStruct.author != nullptr)
                        answerListExp->elts.emplace_back(
                            SemExpCreator::doYouWantMeToSayThemOneByOne(*pWorkStruct.author));
                    pWorkStruct.addAnswerWithoutReferences(ContextualAnnotation::ANSWER, std::move(answerListExp));
                    auto* leafPtr = pWorkStruct.compositeSemAnswers->semAnswers.back()->getLeafPtr();
                    if (leafPtr != nullptr) {
                        auto& leaf = *leafPtr;
                        leaf.interactionContextContainer = std::make_unique<InteractionContextContainer>();
                        int icAllDescriptionId = [&] {
                            InteractionContext icAllDescription;
                            icAllDescription.textToSay = listExp.clone();
                            return leaf.interactionContextContainer->addInteractionContext(std::move(icAllDescription));
                        }();

                        std::list<int> icListIds;
                        auto nbOfElts = listExp.elts.size();
                        std::size_t i = 0;
                        for (auto& currElt : listExp.elts) {
                            ++i;
                            bool isLastElt = i == nbOfElts;
                            icListIds.push_back([&] {
                                InteractionContext icEltDescription;
                                if (isLastElt)
                                    icEltDescription.textToSay = std::move(currElt);
                                else
                                    icEltDescription.textToSay = SemExpCreator::mergeInAList(
                                        std::move(currElt), SemExpCreator::sayAndThenToContinue());
                                return leaf.interactionContextContainer->addInteractionContext(
                                    std::move(icEltDescription));
                            }());
                        }

                        mystd::optional<int> prevId;
                        for (auto it = icListIds.begin(); it != icListIds.end();) {
                            auto nextIt = it;
                            ++nextIt;
                            InteractionContext* icPtr = leaf.interactionContextContainer->getInteractionContextPtr(*it);
                            if (icPtr != nullptr) {
                                auto& ic = *icPtr;
                                if (nextIt != icListIds.end())
                                    ic.answerPossibilities.emplace_back(
                                        std::make_unique<ListExpression>(ListExpressionType::THEN), *nextIt);
                                if (prevId)
                                    ic.answerPossibilities.emplace_back(
                                        std::make_unique<ListExpression>(ListExpressionType::THEN_REVERSED), *prevId);
                            }
                            prevId.emplace(*it);
                            it = nextIt;
                        }

                        InteractionContext icMain;
                        icMain.answerPossibilities.emplace_back(SemExpCreator::generateYesOrNo(false),
                                                                icAllDescriptionId);
                        if (!icListIds.empty()) {
                            icMain.answerPossibilities.emplace_back(SemExpCreator::generateYesOrNo(true),
                                                                    icListIds.front());

                            int lastEltId = icListIds.back();
                            InteractionContext icEltFinished;
                            icEltFinished.textToSay = SemExpCreator::itIsFinished();
                            icEltFinished.answerPossibilities.emplace_back(
                                std::make_unique<ListExpression>(ListExpressionType::THEN_REVERSED), lastEltId);
                            int finishedEltId =
                                leaf.interactionContextContainer->addInteractionContext(std::move(icEltFinished));

                            InteractionContext* lastIcPtr =
                                leaf.interactionContextContainer->getInteractionContextPtr(lastEltId);
                            if (lastIcPtr != nullptr)
                                lastIcPtr->answerPossibilities.emplace_back(
                                    std::make_unique<ListExpression>(ListExpressionType::THEN), finishedEltId);
                        } else {
                            assert(false);
                        }
                        leaf.interactionContextContainer->currentPosition.emplace(
                            leaf.interactionContextContainer->addInteractionContext(std::move(icMain)));
                    }
                }
            }
        }
        return true;
    }

    return false;
}

bool _tryToAnswerTheNameOfAUserId(SemControllerWorkingStruct& pWorkStruct,
                                  const SemanticMemoryBlockPrivate& pMemBlocPrivate,
                                  const std::string& pUserId,
                                  UniqueSemanticExpression pSubjectSemExp,
                                  const std::string& pVerbConcept) {
    const SemanticMemoryGrdExp* semMemoryGrdExpPtr = nullptr;
    auto nameGrd = pMemBlocPrivate.getNameGrd(pUserId, semMemoryGrdExpPtr);
    if (!nameGrd)
        return false;
    RelatedContextAxiom relContextAxiom;
    if (semMemoryGrdExpPtr != nullptr)
        relContextAxiom.add(*semMemoryGrdExpPtr);
    UniqueSemanticExpression nameSemExp = std::make_unique<GroundedExpression>(std::move(nameGrd));
    if (pWorkStruct.reactOperator == SemanticOperatorEnum::ANSWER
        || pWorkStruct.reactOperator == SemanticOperatorEnum::REACT) {
        pWorkStruct.addAnswer(
            ContextualAnnotation::ANSWER,
            SemExpCreator::sentenceFromTriple(std::move(pSubjectSemExp), pVerbConcept, std::move(nameSemExp)),
            ReferencesFiller(relContextAxiom));
    } else {
        pWorkStruct.compositeSemAnswers->semAnswers.emplace_back([&] {
            auto newAns = std::make_unique<LeafSemAnswer>(ContextualAnnotation::ANSWER);
            newAns->answerElts[SemanticRequestType::OBJECT].answersGenerated.emplace_back(std::move(nameSemExp),
                                                                                          &relContextAxiom);
            return newAns;
        }());
    }
    return true;
}

}

bool handleNameQuestion(
    const GroundedExpression& pGrdExp,
    SemanticRequestType pRequestType,
    const std::function<bool(const std::string&, UniqueSemanticExpression, const std::string&)>& pAnswerNameQuestion) {
    switch (pRequestType) {
        case SemanticRequestType::OBJECT: {
            auto itObject = pGrdExp.children.find(GrammaticalType::OBJECT);
            if (itObject != pGrdExp.children.end())
                return _tryToAnswerToWhoIsQuestion(pGrdExp, *itObject->second, pAnswerNameQuestion);
            return _tryToAnswerToNameQuestion(pGrdExp, pAnswerNameQuestion);
        }
        default: break;
    }
    return false;
}

bool process(SemControllerWorkingStruct& pWorkStruct,
             SemanticMemoryBlockViewer& pMemViewer,
             const GroundedExpression& pGrdExp,
             SemanticRequestType pRequestType) {
    if (pWorkStruct.reactOperator == SemanticOperatorEnum::REACT
        || pWorkStruct.reactOperator == SemanticOperatorEnum::ANSWER
        || pWorkStruct.reactOperator == SemanticOperatorEnum::GET) {
        auto answerToNameQuestion =
            [&](const std::string& pUserId, UniqueSemanticExpression pSubjectSemExp, const std::string& pVerbConcept) {
                auto& memBlockPrivate = pMemViewer.getConstViewPrivate();
                return _tryToAnswerTheNameOfAUserId(
                    pWorkStruct, memBlockPrivate, pUserId, std::move(pSubjectSemExp), pVerbConcept);
            };
        if (handleNameQuestion(pGrdExp, pRequestType, answerToNameQuestion))
            return true;
    }

    switch (pRequestType) {
        case SemanticRequestType::YESORNO:
            return _canYouQuestions(pWorkStruct, pMemViewer, pGrdExp) || _checkName(pWorkStruct, pMemViewer, pGrdExp)
                || _tryToAnswerToDoYouKnow(pWorkStruct, pMemViewer, pGrdExp)
                || _tryToAnswerAboutAbilities(pWorkStruct, pMemViewer, pGrdExp);
        case SemanticRequestType::OBJECT: {
            return _tryToAnswerToBirthdateQuestion(pWorkStruct, pMemViewer, pGrdExp)
                || _tryToAnswerToAnAgeQuestion(pWorkStruct, pMemViewer, pGrdExp)
                || _tryToAnswerToWhatCanYouDo(pWorkStruct, pMemViewer.constView, pGrdExp)
                || _tryToAnswerToWhatDoYouKnowAbout(pWorkStruct, pMemViewer, pGrdExp);
        }
        case SemanticRequestType::TIME: return _tryToAnswerToBirthdateQuestion(pWorkStruct, pMemViewer, pGrdExp);
        case SemanticRequestType::MANNER:
            return _tryToAnswerToHowWeKnowSomething(pWorkStruct, pMemViewer, pGrdExp)
                || _tryToAnswerToHowCanITeachYouSomething(pWorkStruct, pGrdExp)
                || _tryToAnswerToHowToDoAnAction(pWorkStruct, pMemViewer, pGrdExp);
        case SemanticRequestType::SUBJECT: return _tryToAnswerToWhatIsAction(pWorkStruct, pMemViewer, pGrdExp);
        default: break;
    }
    return false;
}

}    // End of namespace answerToSpecificQuestions
}    // End of namespace onsem
