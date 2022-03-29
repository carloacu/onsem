#ifndef ONSEM_SEMANTICTOTEXT_SRC_UTILITY_SEMEXPCREATOR_HPP
#define ONSEM_SEMANTICTOTEXT_SRC_UTILITY_SEMEXPCREATOR_HPP

#include <map>
#include <vector>
#include <list>
#include <onsem/common/enum/semanticrequesttype.hpp>
#include <onsem/common/enum/semanticsourceenum.hpp>
#include <onsem/texttosemantic/dbtype/misc/conditionspecification.hpp>
#include "../type/semanticdetailledanswer.hpp"

namespace onsem
{
namespace linguistics
{
struct LinguisticDatabase;
}
struct InterpretationExpression;
struct SemanticExpression;
struct ConditionExpression;
struct GroundedExpression;
struct AnswerExp;
struct SemanticAgentGrounding;
struct UniqueSemanticExpression;
struct SemanticMemory;

namespace SemExpCreator
{

std::unique_ptr<ListExpression> mergeInAList(UniqueSemanticExpression pSemExp1,
                                             UniqueSemanticExpression pSemExp2);

std::unique_ptr<ListExpression> generateAndThen();

std::unique_ptr<GroundedExpression> copyAndReformateGrdExpToPutItInAnAnswer(const GroundedExpression& pGrdExp);

std::unique_ptr<SemanticExpression> sayThat();

std::unique_ptr<GroundedExpression> sayIKnow(bool pPolarity);

std::unique_ptr<GroundedExpression> thereIsXStepsFor(int pNbOfSteps, UniqueSemanticExpression pPurposeSemExp);

std::unique_ptr<GroundedExpression> doYouWantMeToSayThemOneByOne(const SemanticAgentGrounding& pSubjectGrounding);

std::unique_ptr<GroundedExpression> sayAndThenToContinue();

UniqueSemanticExpression formulateConditionToAction(
    const GroundedExpression& pCondition,
    const SemanticExpression& pStuffToDo,
    const SemanticExpression* pStuffToDoOtherwisePtr,
    bool pSemExpToDoIsAlwaysActive);

UniqueSemanticExpression saySemxExp1IsSemExp2(UniqueSemanticExpression pSemExpSubject,
                                              UniqueSemanticExpression pSemExpObject);

UniqueSemanticExpression sentenceFromTriple(UniqueSemanticExpression pSemExpSubject,
                                            const std::string& pVerbConcept,
                                            UniqueSemanticExpression pSemExpObject);

UniqueSemanticExpression getImperativeAssociateFrom(const GroundedExpression& pGrdExp);
UniqueSemanticExpression getFutureIndicativeFromInfinitive(const GroundedExpression& pGrdExp);
UniqueSemanticExpression getIndicativeFromImperative(const GroundedExpression& pGrdExp);
UniqueSemanticExpression getInfinitiveFromImperativeForm(const GroundedExpression& pGrdExp);

UniqueSemanticExpression askWhatIs(const SemanticExpression& pSubjectSemExp);

UniqueSemanticExpression formulateActionDefinition(const GroundedExpression& pLabel,
                                                   const SemanticStatementGrounding& pEqualityStatement,
                                                   UniqueSemanticExpression pDefinition);

UniqueSemanticExpression whoSemExp();

UniqueSemanticExpression askWhoIs(const SemanticExpression& pSubjectSemExp);

UniqueSemanticExpression askIfTrue(const GroundedExpression& pOriginalGrdExp,
                                   const linguistics::LinguisticDatabase& pLingDb);

UniqueSemanticExpression askDoYouWantMeToDoItNow(
    const SemanticAgentGrounding& pSubjectGrounding,
    const GroundedExpression& pActionGrdExp);

UniqueSemanticExpression iWantThatYou(
    const std::string& pSubjectId,
    UniqueSemanticExpression pObject);

UniqueSemanticExpression sayYesOrNo(bool pAnswerPolarity);

UniqueSemanticExpression formulateAge(
    UniqueSemanticExpression&& pSubject,
    std::unique_ptr<SemanticExpression> pAgeSemExp);

UniqueSemanticExpression sayThatTheAssertionIsTrueOrFalse(
    const GroundedExpression& pGrdExp,
    bool pTrueOrFalse);

std::unique_ptr<GroundedExpression> sayICan(bool pPolarity);
UniqueSemanticExpression askDoYouWantToKnowHow(const SemanticAgentGrounding& pSubjectGrounding,
                                               UniqueSemanticExpression pActionSemExp);
/**
 * @brief askWhatIsYourName returns a SemanticExpression used to ask a SemanticMemory about the name
 *  of the user, based on its user Id.
 * @param pSubjectId is the current user Id.
 * @return a UniqueSemanticExpression
 */
UniqueSemanticExpression askWhatIsYourName(const std::string& pSubjectId);
void addButYouCanTeachMe(GroundedExpression& pRootGrdExp,
                         const SemanticAgentGrounding& pSubjectGrounding);
UniqueSemanticExpression sayIThoughtThat(UniqueSemanticExpression pObjectSemExp);
UniqueSemanticExpression sayThatWeThoughtTheContrary();

UniqueSemanticExpression formulateWeekDay(
    const std::string& pWeekDayConcept);

UniqueSemanticExpression okIRemoveAllConditions();

UniqueSemanticExpression formulateHowWeKnowSomething(
    const SemanticExpression& pWhatWeKnow,
    const SemanticExpression& pHowWeKnowThat);

UniqueSemanticExpression forExampleSayToDoMeansToSayIDo(
    const SemanticAgentGrounding& pAuthor,
    const GroundedExpression& pActionGrdExp);

std::unique_ptr<SemanticExpression> getSemExpThatSomebodyToldMeThat(
    const SemanticAgentGrounding& pAuthor);

std::unique_ptr<SemanticExpression> getSemExpOfEventValue(
    const std::string& pEventName,
    const std::string& pEventValue);

std::unique_ptr<SemanticExpression> sayThatOpNotifyInformedMeThat();

UniqueSemanticExpression sayThanksThatsCool(
    const SemanticAgentGrounding& pSubjectGrounding);

UniqueSemanticExpression sayIAmHappyToHearThat();

UniqueSemanticExpression itsMe();

UniqueSemanticExpression itIsNotKind();

UniqueSemanticExpression itIsABadNews();

UniqueSemanticExpression iAmSorryToHearThat();

UniqueSemanticExpression niceYouLikeIt(UniqueSemanticExpression pNiceSemExp,
                                       const SemanticAgentGrounding& pSubjectGrounding);

UniqueSemanticExpression sorryIWillTryToImproveMyself();

UniqueSemanticExpression getWhoIsSomebodyQuestion(
    const GroundedExpression& pPersonToAsk);

UniqueSemanticExpression generateNumberOfTimesAnswer(
    const GroundedExpression& pGrdExpQuestion,
    std::size_t pNbOfTimes);

mystd::unique_propagate_const<UniqueSemanticExpression> generateAnswer(std::map<SemanticRequestType, AllAnswerElts>& pAllAnswers,
                                                                       std::list<std::string>& pReferences,
                                                                       const GroundedExpression& pGrdExpQuestion,
                                                                       const SemanticRequests& pRequests,
                                                                       const SemanticMemoryBlock& pMemBlock,
                                                                       const linguistics::LinguisticDatabase& pLingDb);

UniqueSemanticExpression sayThatWeDontKnowTheAnswer(const SemanticExpression& pSemExp);
UniqueSemanticExpression sayThatWeDontKnowAnInstanceOf(const SemanticExpression& pSemExp);

UniqueSemanticExpression sayThatWeAreNotAbleToDoIt(
    const SemanticExpression& pSemExp);

UniqueSemanticExpression askForPrecision(
    const GroundedExpression& pGrdExp,
    SemanticRequestType pRequestType);

UniqueSemanticExpression sayWeWillDoIt_fromGrdExp(
    const GroundedExpression& pGrdExp);

UniqueSemanticExpression sayWeWillDoIt(const ConditionSpecification& pCondSpec);

UniqueSemanticExpression sayAlso();
UniqueSemanticExpression sayThanks();
UniqueSemanticExpression sayOk();
UniqueSemanticExpression sayTrue();
UniqueSemanticExpression sayFalse();

UniqueSemanticExpression confirmInformation(
    const GroundedExpression& pGrdExp);

UniqueSemanticExpression confirmInfoCondition(const ConditionSpecification& pCondSpec);

UniqueSemanticExpression generateYesOrNo(bool pSamePolarity);

UniqueSemanticExpression generateYesOrNoAnswer(UniqueSemanticExpression pSemExp,
                                               bool pSamePolarity);

UniqueSemanticExpression generateYesOrNoAnswerFromQuestion(
    const GroundedExpression& pGrdExpQuestion,
    bool pSamePolarity,
    const std::map<GrammaticalType, const SemanticExpression*>& pAnnotationsOfTheAnswer);

UniqueSemanticExpression generateYesOrNoAnswerFromMemory(
    UniqueSemanticExpression pSemExp,
    bool pSamePolarity,
    const std::map<GrammaticalType, const SemanticExpression*>& pAnnotationsOfTheAnswer);


} // End of namespace SemExpCreator
} // End of namespace onsem


#endif // ONSEM_SEMANTICTOTEXT_SRC_UTILITY_SEMEXPCREATOR_HPP
