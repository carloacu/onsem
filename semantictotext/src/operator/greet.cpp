#include <onsem/texttosemantic/dbtype/semanticexpressions.hpp>
#include <onsem/texttosemantic/dbtype/semanticgroundings.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/conceptset.hpp>
#include <onsem/semantictotext/semexpoperators.hpp>
#include <onsem/semantictotext/greet.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemory.hpp>
#include <onsem/texttosemantic/tool/semexpgetter.hpp>
#include "../utility/semexpcreator.hpp"
#include "../tool/userinfosfiller.hpp"
#include "../controller/steps/answertospecificquestions.hpp"

namespace onsem {

namespace {

struct UserInfosWithoutMemoryLinks : SemExpUserInfosFiller::UserInfosContainer {
    void addNames(const std::string& pUserId, const std::vector<std::string>& pNames) override {
        _userIdToNames[pUserId] = pNames;
    }

    void addGenders(const std::string&, const std::set<SemanticGenderType>&) override {}

    void addEquivalentUserIds(const std::string& pSubjectUserId, const std::string& pObjectUserId) override {
        _equivalentUserIds[pSubjectUserId] = pObjectUserId;
        _equivalentUserIds[pObjectUserId] = pSubjectUserId;
    }

    std::string hasEquivalentUserId(const std::string& pUserId) const {
        auto itEquUserId = _equivalentUserIds.find(pUserId);
        if (itEquUserId != _equivalentUserIds.end())
            return itEquUserId->second;
        return "";
    }

    std::string getName(const std::string& pUserId) const {
        auto it = _userIdToNames.find(pUserId);
        if (it != _userIdToNames.end())
            return SemanticNameGrounding::namesToStr(it->second);
        return "";
    }

    void addGrdExpToUserId(const GroundedExpression&, const std::string&) override {}

private:
    std::map<std::string, std::vector<std::string>> _userIdToNames;
    std::map<std::string, std::string> _equivalentUserIds;
};

std::string _extractNameFromGrdExp(const GroundedExpression& pGrdExp, const std::string& pAuthorUserId) {
    if (!pAuthorUserId.empty()) {
        UserInfosWithoutMemoryLinks userInfos;
        SemExpUserInfosFiller::tryToAddUserInfos(userInfos, pGrdExp);
        std::string equUserId = userInfos.hasEquivalentUserId(pAuthorUserId);
        if (!equUserId.empty()) {
            std::string name = userInfos.getName(equUserId);
            if (!name.empty())
                return name;
        }
    }
    return {};
}

mystd::unique_propagate_const<UniqueSemanticExpression> _answerNiceToMeetYouIfTheUserSaysHisName(
    const GroundedExpression& pGrdExp,
    const std::function<void(const std::string&)> onHumanHasGivenName,
    const std::string& pAuthorUserId) {
    std::string name = _extractNameFromGrdExp(pGrdExp, pAuthorUserId);

    if (!name.empty()) {
        if (onHumanHasGivenName)
            onHumanHasGivenName(name);
        // N5 replies "Nice to meet you <name>"
        auto res = std::make_unique<FeedbackExpression>(
            std::make_unique<GroundedExpression>(std::make_unique<SemanticConceptualGrounding>("niceToMeetYou")),
            std::make_unique<GroundedExpression>(std::make_unique<SemanticTextGrounding>(name)));
        return mystd::unique_propagate_const<UniqueSemanticExpression>(std::move(res));
    }
    return mystd::unique_propagate_const<UniqueSemanticExpression>();
}

/// Replies "hello/bye <interlocutor>"
/// If an interlocutor expression is specified, it is used.
/// Otherwise, the authorUserId is used to find a way to express the interlocutor.
/// If no interlocutor expression can be found, it is omitted.
/// i.e. it replies "hello / bye".
mystd::unique_propagate_const<UniqueSemanticExpression> _answerHelloBye(
    const SemanticExpression& pEngagementSemExp,
    const SemanticMemory& pSemanticMemory,
    const linguistics::LinguisticDatabase& pLingDb,
    const GreetCallbacks& callbacks,
    const std::string& pAuthorUserId,
    std::unique_ptr<SemanticExpression> pInterlocutor) {
    if (!pInterlocutor) {    // find name from semantic memory
        if (!pAuthorUserId.empty()) {
            std::vector<std::unique_ptr<GroundedExpression>> grdExpAnswers;
            memoryOperation::get(
                grdExpAnswers, SemExpCreator::askWhatIsYourName(pAuthorUserId), pSemanticMemory, pLingDb);
            if (!grdExpAnswers.empty()) {
                pInterlocutor = std::move(grdExpAnswers.front());
            }
        }
    }

    if (!pInterlocutor) {
        // No username found
        const auto genGrdPtr = pEngagementSemExp.getGrdExp().grounding().getGenericGroundingPtr();
        if (genGrdPtr != nullptr && ConceptSet::haveAConcept(genGrdPtr->concepts, "engagement_disengage")) {
            // N5 says "Bye"
            if (callbacks.onHumanLeaving)
                callbacks.onHumanLeaving();
            return UniqueSemanticExpression(pEngagementSemExp.clone());
        } else {
            if (callbacks.onHumanIntroduced)
                callbacks.onHumanIntroduced();
            // N5 replies "Hello, what is your name?"
            if (!pAuthorUserId.empty()) {
                return UniqueSemanticExpression(std::make_unique<FeedbackExpression>(
                    pEngagementSemExp.clone(), SemExpCreator::askWhatIsYourName(pAuthorUserId)));
            }
        }
    } else {
        // N5 replies "Hello/Bye <name>"
        return UniqueSemanticExpression(
            std::make_unique<FeedbackExpression>(pEngagementSemExp.clone(), std::move(pInterlocutor)));
    }

    return {};
}

bool isLookAtOtherOrder(const GroundedExpression& grdExp) {
    // TODO fix toPerson boolean
    const auto isLook = ConceptSet::haveAConceptThatBeginWith(grdExp->concepts, "verb_action_lookat");
    const auto fromMe = SemExpGetter::getUserIdOfSubject(grdExp) == SemanticAgentGrounding::me;
    auto toPerson(true);
    const auto grdExpObjectPtr = SemExpGetter::getGrdExpChild(grdExp.getGrdExp(), GrammaticalType::OBJECT);
    if (grdExpObjectPtr != nullptr) {
        auto* genGrdPtr = grdExpObjectPtr->grounding().getGenericGroundingPtr();
        if (genGrdPtr != nullptr) {
            toPerson = toPerson && ConceptSet::haveAConceptThatBeginWith(genGrdPtr->concepts, "reference_other")
                    && (genGrdPtr->entityType == SemanticEntityType::HUMAN);
            const auto grdExpObjectSpecifierPtr =
                SemExpGetter::getGrdExpChild(*grdExpObjectPtr, GrammaticalType::SPECIFIER);
            if (grdExpObjectSpecifierPtr != nullptr)
                toPerson = toPerson
                        && ConceptSet::haveAConceptThatBeginWith(grdExpObjectSpecifierPtr->grounding().concepts,
                                                                 "agent_human");
        }
    }
    return isLook && fromMe && toPerson;
}

mystd::unique_propagate_const<UniqueSemanticExpression> _greet(const SemanticExpression& pSemExp,
                                                               const SemanticMemory& pSemanticMemory,
                                                               const linguistics::LinguisticDatabase& pLingDb,
                                                               const GreetCallbacks& callbacks,
                                                               const std::string& pAuthorUserId,
                                                               std::unique_ptr<SemanticExpression> pInterlocutor) {
    switch (pSemExp.type) {
        case SemanticExpressionType::GROUNDED: {
            const auto& grdExp = pSemExp.getGrdExp();
            const auto genGrdPtr = grdExp.grounding().getGenericGroundingPtr();
            if (genGrdPtr != nullptr) {
                //  CASE no verb + engagement ==> Hello / Bye
                if (ConceptSet::haveAConceptThatBeginWith(genGrdPtr->concepts, "engagement_"))
                    return _answerHelloBye(
                        pSemExp, pSemanticMemory, pLingDb, callbacks, pAuthorUserId, std::move(pInterlocutor));
            } else {
                // CASE sentence with verb
                const auto statGrdPtr = grdExp.grounding().getStatementGroundingPtr();
                if (statGrdPtr != nullptr) {    // CASE statement

                    if (!statGrdPtr->requests.empty()) {    // CASE request (== question OR order)

                        if (statGrdPtr->requests.has(SemanticRequestType::ACTION)
                            && isLookAtOtherOrder(
                                grdExp)) {    // CASE human order an action AND action is "look at other"
                            if (callbacks.lookAtOtherPerson)
                                callbacks.lookAtOtherPerson(std::string());
                        } else {    // CASE might be a question
                            mystd::unique_propagate_const<UniqueSemanticExpression> exp;

                            // Nothing was specified to express the interlocutor
                            if (!pInterlocutor) {    // find an expression from the semantic memory
                                std::vector<std::unique_ptr<GroundedExpression>> grdExpAnswers;
                                memoryOperation::get(grdExpAnswers,
                                                     SemExpCreator::askWhatIsYourName(pAuthorUserId),
                                                     pSemanticMemory,
                                                     pLingDb);
                                if (!grdExpAnswers.empty())
                                    pInterlocutor = grdExpAnswers.front()->clone();
                            }

                            auto answerToNameQuestion = [&pInterlocutor, &exp](const std::string&,
                                                                               UniqueSemanticExpression pSubjectSemExp,
                                                                               const std::string& pVerbConcept) {
                                if (pInterlocutor && !pInterlocutor->isEmpty()) {
                                    exp = SemExpCreator::sentenceFromTriple(
                                        std::move(pSubjectSemExp),
                                        pVerbConcept,
                                        UniqueSemanticExpression(pInterlocutor.get()->clone()));
                                    return true;
                                }
                                return false;
                            };

                            SemanticRequestType requestType = statGrdPtr->requests.first();
                            answerToSpecificQuestions::handleNameQuestion(grdExp, requestType, answerToNameQuestion);
                            if (exp)
                                return exp;
                        }
                    } else {    // CASE is a simple statement, might be human giving his/her name
                        return _answerNiceToMeetYouIfTheUserSaysHisName(
                            grdExp, std::move(callbacks.onHumanHasGivenName), pAuthorUserId);
                    }
                }
            }
            break;
        }
        case SemanticExpressionType::LIST: {
            const auto& listExp = pSemExp.getListExp();
            for (const auto& currElt : listExp.elts) {
                auto res =
                    _greet(*currElt, pSemanticMemory, pLingDb, callbacks, pAuthorUserId, std::move(pInterlocutor));
                if (res)    // Do not return if res returns nothing
                    return res;
            }
            break;
        }
        case SemanticExpressionType::FEEDBACK:
            // Is a feedback, with an interjection, such as "Hi N5" or "Bye Paul"
            {
                const auto& fdkExp = pSemExp.getFdkExp();
                const GroundedExpression* fdkGrdExpPtr = fdkExp.feedbackExp->getGrdExpPtr_SkipWrapperPtrs();
                if (fdkGrdExpPtr != nullptr
                    && ConceptSet::haveAConceptThatBeginWith(fdkGrdExpPtr->grounding().concepts, "engagement_")) {
                    //  CASE sentence starts with interjection ("Hello, ...." or "Bye ...")
                    const GroundedExpression* concernedGrdExpPtr = fdkExp.concernedExp->getGrdExpPtr_SkipWrapperPtrs();
                    if (concernedGrdExpPtr != nullptr) {
                        const GroundedExpression& concernedGrdExp = *concernedGrdExpPtr;
                        const SemanticAgentGrounding* concernedAgentPtr = concernedGrdExp->getAgentGroundingPtr();
                        if (concernedAgentPtr != nullptr
                            && pSemanticMemory.memBloc.isItMe(
                                concernedAgentPtr->userId)) {    // Human speaks to the robot with interjection. Might
                                                                 // be "Hi N5", "Bye N5" ...
                            return _answerHelloBye(*fdkExp.feedbackExp,
                                                   pSemanticMemory,
                                                   pLingDb,
                                                   callbacks,
                                                   pAuthorUserId,
                                                   std::move(pInterlocutor));
                        }
                    }
                }
                return _greet(
                    *fdkExp.concernedExp, pSemanticMemory, pLingDb, callbacks, pAuthorUserId, std::move(pInterlocutor));
            }
        case SemanticExpressionType::METADATA: {
            const auto& metaExp = pSemExp.getMetadataExp();
            std::string authorUserId = metaExp.getAuthorId();
            return _greet(*metaExp.semExp, pSemanticMemory, pLingDb, callbacks, authorUserId, std::move(pInterlocutor));
        }
        case SemanticExpressionType::ANNOTATED:
            return _greet(*pSemExp.getAnnExp().semExp,
                          pSemanticMemory,
                          pLingDb,
                          callbacks,
                          pAuthorUserId,
                          std::move(pInterlocutor));
        case SemanticExpressionType::INTERPRETATION:
            return _greet(*pSemExp.getIntExp().interpretedExp,
                          pSemanticMemory,
                          pLingDb,
                          callbacks,
                          pAuthorUserId,
                          std::move(pInterlocutor));
        case SemanticExpressionType::FIXEDSYNTHESIS:
            return _greet(pSemExp.getFSynthExp().getSemExp(),
                          pSemanticMemory,
                          pLingDb,
                          callbacks,
                          pAuthorUserId,
                          std::move(pInterlocutor));
        case SemanticExpressionType::CONDITION:
        case SemanticExpressionType::COMPARISON:
        case SemanticExpressionType::SETOFFORMS:
        case SemanticExpressionType::COMMAND: break;
    }

    return mystd::unique_propagate_const<UniqueSemanticExpression>();
}
}    // End of anonymous namesapce

mystd::unique_propagate_const<UniqueSemanticExpression> greetInResponseOf(
    const SemanticExpression& pSemExp,
    const SemanticMemory& pSemanticMemory,
    const linguistics::LinguisticDatabase& pLingDb,
    const GreetCallbacks& pCallbacks,
    std::unique_ptr<SemanticExpression> pInterlocutor) {
    return _greet(pSemExp, pSemanticMemory, pLingDb, pCallbacks, "", std::move(pInterlocutor));
}

}    // End of namespace onsem
