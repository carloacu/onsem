#include <gtest/gtest.h>
#include <boost/scope_exit.hpp>
#include <onsem/common/utility/noresult.hpp>
#include <onsem/texttosemantic/dbtype/textprocessingcontext.hpp>
#include <onsem/semantictotext/semexpoperators.hpp>
#include <onsem/semantictotext/semanticconverter.hpp>
#include <onsem/semantictotext/semanticmemory/semantictracker.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemory.hpp>
#include "../../semanticreasonergtests.hpp"
#include "operator_answer.hpp"
#include "operator_check.hpp"
#include "operator_inform.hpp"


using namespace onsem;
namespace
{

struct TrackStateForTests
{
  TrackStateForTests(const std::string& pText,
                     SemanticMemory& pSemanticMemory,
                     const linguistics::LinguisticDatabase& pLingDb)
    : text(pText),
      semanticMemory(pSemanticMemory),
      lingDb(pLingDb),
      semTracker(std::make_shared<SemanticTracker>()),
      resultsStr(),
      connection()
  {
  }

  ~TrackStateForTests()
  {
    connection.disconnect();
  }

  std::string getResult()
  {
    if (resultsStr.empty())
      return constant::noResult;
    if (resultsStr.size() == 1)
      return resultsStr.back();
    std::stringstream ss;
    ss << "<" << resultsStr.size() << " results>";
    return ss.str();
  }

  std::string text;
  SemanticMemory& semanticMemory;
  const linguistics::LinguisticDatabase& lingDb;
  std::shared_ptr<SemanticTracker> semTracker;
  std::list<std::string> resultsStr;
  mystd::observable::Connection connection;
};


void operator_track(std::shared_ptr<TrackStateForTests>& pTrackState,
                    const TextProcessingContext& pContext)
{
  auto semExp = converter::textToSemExp
      (pTrackState->text, pContext, pTrackState->lingDb, true);
  memoryOperation::track(pTrackState->semanticMemory, std::move(semExp),
                         pTrackState->semTracker, pTrackState->lingDb);

  pTrackState->connection = pTrackState->semTracker->val.connect
      ([pTrackState](const UniqueSemanticExpression& pSemExp)
  {
    TextProcessingContext outContext(SemanticAgentGrounding::currentUser,
                                     SemanticAgentGrounding::me,
                                     SemanticLanguageEnum::ENGLISH);
    pTrackState->resultsStr.emplace_back();
    std::string& newResStr = pTrackState->resultsStr.back();
    converter::semExpToText(newResStr, pSemExp->clone(), outContext,
                            false, pTrackState->semanticMemory,
                            pTrackState->lingDb, nullptr);
  });
}


void operator_untrack(std::shared_ptr<TrackStateForTests>& pTrackState,
                      const linguistics::LinguisticDatabase& pLingDb)
{
  memoryOperation::untrack(pTrackState->semanticMemory.memBloc,
                           pTrackState->semTracker, pLingDb, nullptr);
}

}



TEST_F(SemanticReasonerGTests, operator_track_basic)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semanticMemory;
  std::map<const SemanticContextAxiom*, TruenessValue> axiomToConditionCurrentState;

  auto objectTracking = std::make_shared<TrackStateForTests>
      ("what do you eat ?", semanticMemory, lingDb);
  operator_track(objectTracking, fromUser);
  auto timeTracking = std::make_shared<TrackStateForTests>
      ("when do you eat ?", semanticMemory, lingDb);
  operator_track(timeTracking, fromUser);
  auto yesOrNoTracking = std::make_shared<TrackStateForTests>
      ("Do you eat chocolate ?", semanticMemory, lingDb);
  operator_track(yesOrNoTracking, fromUser);
  auto affirmationTracking = std::make_shared<TrackStateForTests>
      ("You eat chocolate", semanticMemory, lingDb);
  operator_track(affirmationTracking, fromUser);
  auto clearTrackersResults = [=]
  {
    objectTracking->resultsStr.clear();
    timeTracking->resultsStr.clear();
    yesOrNoTracking->resultsStr.clear();
    affirmationTracking->resultsStr.clear();
  };

  BOOST_SCOPE_EXIT_ALL(&)
  {
    objectTracking->connection.disconnect();
    timeTracking->connection.disconnect();
    yesOrNoTracking->connection.disconnect();
    affirmationTracking->connection.disconnect();
  };

  EXPECT_EQ(constant::noResult, objectTracking->getResult());
  EXPECT_EQ(constant::noResult, yesOrNoTracking->getResult());
  EXPECT_EQ(constant::noResult, affirmationTracking->getResult());

  operator_inform("you eat chocolate", semanticMemory, lingDb, {}, &axiomToConditionCurrentState);
  EXPECT_EQ("Chocolate", objectTracking->getResult());
  EXPECT_EQ(constant::noResult, timeTracking->getResult());
  EXPECT_EQ("True", yesOrNoTracking->getResult());
  EXPECT_EQ("", affirmationTracking->getResult());
  clearTrackersResults();

  operator_inform("you eat chocolate", semanticMemory, lingDb, {}, &axiomToConditionCurrentState);
  EXPECT_EQ("Chocolate", objectTracking->getResult());
  EXPECT_EQ(constant::noResult, timeTracking->getResult());
  EXPECT_EQ(constant::noResult, yesOrNoTracking->getResult());
  EXPECT_EQ(constant::noResult, affirmationTracking->getResult());
  clearTrackersResults();

  operator_inform("you don't eat chocolate", semanticMemory, lingDb, {}, &axiomToConditionCurrentState);
  EXPECT_EQ("", objectTracking->getResult());
  EXPECT_EQ(constant::noResult, timeTracking->getResult());
  EXPECT_EQ("False", yesOrNoTracking->getResult());
  EXPECT_EQ(constant::noResult, affirmationTracking->getResult());
  clearTrackersResults();

  operator_inform("you eat chocolate", semanticMemory, lingDb, {}, &axiomToConditionCurrentState);
  EXPECT_EQ("Chocolate", objectTracking->getResult());
  EXPECT_EQ(constant::noResult, timeTracking->getResult());
  EXPECT_EQ("True", yesOrNoTracking->getResult());
  EXPECT_EQ("", affirmationTracking->getResult());
  clearTrackersResults();

  operator_inform("you eat yesterday", semanticMemory, lingDb, {}, &axiomToConditionCurrentState);
  EXPECT_EQ(constant::noResult, objectTracking->getResult());
  EXPECT_EQ("Yesterday", timeTracking->getResult());
  EXPECT_EQ(constant::noResult, yesOrNoTracking->getResult());
  EXPECT_EQ(constant::noResult, affirmationTracking->getResult());
  clearTrackersResults();

  operator_inform("you eat chocolate", semanticMemory, lingDb, {}, &axiomToConditionCurrentState);
  EXPECT_EQ("Chocolate", objectTracking->getResult());
  EXPECT_EQ(constant::noResult, timeTracking->getResult());
  EXPECT_EQ(constant::noResult, yesOrNoTracking->getResult());
  EXPECT_EQ(constant::noResult, affirmationTracking->getResult());
  clearTrackersResults();

  operator_untrack(objectTracking, lingDb);
  operator_untrack(timeTracking, lingDb);
  operator_untrack(yesOrNoTracking, lingDb);
  operator_untrack(affirmationTracking, lingDb);
  operator_inform("you eat chocolate", semanticMemory, lingDb, {}, &axiomToConditionCurrentState);
  EXPECT_EQ(constant::noResult, objectTracking->getResult());
  EXPECT_EQ(constant::noResult, timeTracking->getResult());
  EXPECT_EQ(constant::noResult, yesOrNoTracking->getResult());
  EXPECT_EQ(constant::noResult, affirmationTracking->getResult());
}


TEST_F(SemanticReasonerGTests, operator_track_list)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semanticMemory;
  std::map<const SemanticContextAxiom*, TruenessValue> axiomToConditionCurrentState;

  auto objectTracking = std::make_shared<TrackStateForTests>
      ("what do you eat ?", semanticMemory, lingDb);
  operator_track(objectTracking, fromUser);
  auto yesOrNoTracking = std::make_shared<TrackStateForTests>
      ("you eat chocolate and lettuce?", semanticMemory, lingDb);
  operator_track(yesOrNoTracking, fromUser);
  auto affirmationTracking = std::make_shared<TrackStateForTests>
      ("you eat chocolate and lettuce", semanticMemory, lingDb);
  operator_track(affirmationTracking, fromUser);
  auto clearTrackersResults = [=]
  {
    objectTracking->resultsStr.clear();
    yesOrNoTracking->resultsStr.clear();
    affirmationTracking->resultsStr.clear();
  };

  BOOST_SCOPE_EXIT_ALL(&)
  {
    objectTracking->connection.disconnect();
    yesOrNoTracking->connection.disconnect();
    affirmationTracking->connection.disconnect();
  };


  EXPECT_EQ(constant::noResult, objectTracking->getResult());
  EXPECT_EQ(constant::noResult, yesOrNoTracking->getResult());
  EXPECT_EQ(constant::noResult, affirmationTracking->getResult());

  operator_inform("you eat chocolate", semanticMemory, lingDb, {}, &axiomToConditionCurrentState);
  EXPECT_EQ("Chocolate", objectTracking->getResult());
  EXPECT_EQ(constant::noResult, yesOrNoTracking->getResult());
  EXPECT_EQ(constant::noResult, affirmationTracking->getResult());
  clearTrackersResults();

  operator_inform("you eat chocolate", semanticMemory, lingDb, {}, &axiomToConditionCurrentState);
  EXPECT_EQ("Chocolate", objectTracking->getResult());
  EXPECT_EQ(constant::noResult, yesOrNoTracking->getResult());
  EXPECT_EQ(constant::noResult, affirmationTracking->getResult());
  clearTrackersResults();

  operator_inform("you eat lettuce", semanticMemory, lingDb, {}, &axiomToConditionCurrentState);
  EXPECT_EQ("Chocolate and lettuce", objectTracking->getResult());
  EXPECT_EQ("True", yesOrNoTracking->getResult());
  EXPECT_EQ("", affirmationTracking->getResult());
  clearTrackersResults();

  operator_inform("you don't eat chocolate", semanticMemory, lingDb, {}, &axiomToConditionCurrentState);
  EXPECT_EQ("Lettuce", objectTracking->getResult());
  EXPECT_EQ("False", yesOrNoTracking->getResult());
  EXPECT_EQ(constant::noResult, affirmationTracking->getResult());
  clearTrackersResults();

  operator_inform("you eat lettuce", semanticMemory, lingDb, {}, &axiomToConditionCurrentState);
  EXPECT_EQ("Lettuce", objectTracking->getResult());
  EXPECT_EQ(constant::noResult, yesOrNoTracking->getResult());
  EXPECT_EQ(constant::noResult, affirmationTracking->getResult());
  clearTrackersResults();

  operator_untrack(objectTracking, lingDb);
  operator_untrack(yesOrNoTracking, lingDb);
  operator_untrack(affirmationTracking, lingDb);
  operator_inform("you eat chocolate and lettuce", semanticMemory, lingDb, {}, &axiomToConditionCurrentState);
  EXPECT_EQ(constant::noResult, objectTracking->getResult());
  EXPECT_EQ(constant::noResult, yesOrNoTracking->getResult());
  EXPECT_EQ(constant::noResult, affirmationTracking->getResult());
}

TEST_F(SemanticReasonerGTests, operator_track_advanced)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semanticMemory;
  std::map<const SemanticContextAxiom*, TruenessValue> axiomToConditionCurrentState;
  operator_addFallback("I don't like chocolate", semanticMemory, lingDb);
  operator_addFallback("your left hand is not touched", semanticMemory, lingDb);

  auto affirmationTracking = std::make_shared<TrackStateForTests>
      ("I don't like chocolate", semanticMemory, lingDb);
  operator_track(affirmationTracking, fromUser);
  auto yesOrNoTracking = std::make_shared<TrackStateForTests>
      ("do I like chocolate", semanticMemory, lingDb);
  operator_track(yesOrNoTracking, fromUser);
  auto leftHandTracking = std::make_shared<TrackStateForTests>
      ("your left hand is not touched", semanticMemory, lingDb);
  operator_track(leftHandTracking, fromUser);
  auto clearTrackersResults = [=]
  {
    affirmationTracking->resultsStr.clear();
    yesOrNoTracking->resultsStr.clear();
    leftHandTracking->resultsStr.clear();
  };

  BOOST_SCOPE_EXIT_ALL(&)
  {
    affirmationTracking->connection.disconnect();
    yesOrNoTracking->connection.disconnect();
    leftHandTracking->connection.disconnect();
  };

  EXPECT_EQ(constant::noResult, affirmationTracking->getResult());
  EXPECT_EQ(constant::noResult, yesOrNoTracking->getResult());
  EXPECT_EQ(constant::noResult, leftHandTracking->getResult());

  operator_inform("I don't like chocolate", semanticMemory, lingDb, {}, &axiomToConditionCurrentState);
  EXPECT_EQ("", affirmationTracking->getResult());
  EXPECT_EQ("False", yesOrNoTracking->getResult());
  EXPECT_EQ(constant::noResult, leftHandTracking->getResult());
  clearTrackersResults();

  auto idExp = operator_inform("I like chocolate", semanticMemory, lingDb, {}, &axiomToConditionCurrentState);
  EXPECT_EQ(constant::noResult, affirmationTracking->getResult());
  EXPECT_EQ("True", yesOrNoTracking->getResult());
  EXPECT_EQ(constant::noResult, leftHandTracking->getResult());
  clearTrackersResults();

  semanticMemory.memBloc.removeExpression(*idExp, lingDb, &axiomToConditionCurrentState);
  EXPECT_EQ("", affirmationTracking->getResult());
  EXPECT_EQ("False", yesOrNoTracking->getResult());
  EXPECT_EQ(constant::noResult, leftHandTracking->getResult());
  clearTrackersResults();

  auto idExp2 = operator_inform("somebody touches your left hand", semanticMemory, lingDb, {}, &axiomToConditionCurrentState);
  EXPECT_EQ(constant::noResult, affirmationTracking->getResult());
  EXPECT_EQ(constant::noResult, yesOrNoTracking->getResult());
  EXPECT_EQ(constant::noResult, leftHandTracking->getResult());
  semanticMemory.memBloc.removeExpression(*idExp2, lingDb, &axiomToConditionCurrentState);
  EXPECT_EQ(constant::noResult, affirmationTracking->getResult());
  EXPECT_EQ(constant::noResult, yesOrNoTracking->getResult());
  EXPECT_EQ("", leftHandTracking->getResult());

  clearTrackersResults();
}


TEST_F(SemanticReasonerGTests, operator_track_seePeople)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semanticMemory;
  std::map<const SemanticContextAxiom*, TruenessValue> axiomToConditionCurrentState;

  auto nobodyTracking = std::make_shared<TrackStateForTests>
      ("I don't see anybody", semanticMemory, lingDb);
  auto onePersonTracking = std::make_shared<TrackStateForTests>
      ("I see one person", semanticMemory, lingDb);
  auto twoPeopleTracking = std::make_shared<TrackStateForTests>
      ("I see 2 people", semanticMemory, lingDb);
  auto threePeopleTracking = std::make_shared<TrackStateForTests>
      ("I see 3 people", semanticMemory, lingDb);
  operator_track(nobodyTracking, fromRobot);
  operator_track(onePersonTracking, fromRobot);
  operator_track(twoPeopleTracking, fromRobot);
  operator_track(threePeopleTracking, fromRobot);

  auto clearTrackersResults = [=]
  {
    nobodyTracking->resultsStr.clear();
    onePersonTracking->resultsStr.clear();
    twoPeopleTracking->resultsStr.clear();
    threePeopleTracking->resultsStr.clear();
  };
  BOOST_SCOPE_EXIT_ALL(&)
  {
    nobodyTracking->connection.disconnect();
    onePersonTracking->connection.disconnect();
    twoPeopleTracking->connection.disconnect();
    threePeopleTracking->connection.disconnect();
  };

  EXPECT_EQ(constant::noResult, nobodyTracking->getResult());
  EXPECT_EQ(constant::noResult, onePersonTracking->getResult());
  EXPECT_EQ(constant::noResult, twoPeopleTracking->getResult());
  EXPECT_EQ(constant::noResult, threePeopleTracking->getResult());
  auto idExp1 = operator_inform_fromRobot("I see no person", semanticMemory, lingDb, {}, &axiomToConditionCurrentState);
  EXPECT_EQ("", nobodyTracking->getResult());
  EXPECT_EQ(constant::noResult, onePersonTracking->getResult());
  EXPECT_EQ(constant::noResult, twoPeopleTracking->getResult());
  EXPECT_EQ(constant::noResult, threePeopleTracking->getResult());
  clearTrackersResults();
  auto idExp2 = operator_inform_fromRobot("I see one person", semanticMemory, lingDb, {}, &axiomToConditionCurrentState);
  semanticMemory.memBloc.removeExpression(*idExp1, lingDb, &axiomToConditionCurrentState);
  EXPECT_EQ(constant::noResult, nobodyTracking->getResult());
  EXPECT_EQ("", onePersonTracking->getResult());
  EXPECT_EQ(constant::noResult, twoPeopleTracking->getResult());
  EXPECT_EQ(constant::noResult, threePeopleTracking->getResult());
  clearTrackersResults();
  idExp1 = operator_inform_fromRobot("I don't see you and I see 2 people", semanticMemory, lingDb, {}, &axiomToConditionCurrentState);
  semanticMemory.memBloc.removeExpression(*idExp2, lingDb, &axiomToConditionCurrentState);
  ONSEM_ANSWER_EQ("I see 2 people.",
                  operator_answer("How many people do you see?", semanticMemory, lingDb));
  EXPECT_EQ(constant::noResult, nobodyTracking->getResult());
  EXPECT_EQ(constant::noResult, onePersonTracking->getResult());
  EXPECT_EQ("", twoPeopleTracking->getResult());
  EXPECT_EQ(constant::noResult, threePeopleTracking->getResult());
  clearTrackersResults();
  idExp2 = operator_inform_fromRobot("I see you and I see 0 other people", semanticMemory, lingDb, {}, &axiomToConditionCurrentState);
  semanticMemory.memBloc.removeExpression(*idExp1, lingDb, &axiomToConditionCurrentState);
  ONSEM_ANSWER_EQ("I see one person.",
                  operator_answer("How many people do you see?", semanticMemory, lingDb));
  ONSEM_FALSE(operator_check(nobodyTracking->text, semanticMemory, lingDb, fromRobot));
  ONSEM_TRUE(operator_check(onePersonTracking->text, semanticMemory, lingDb, fromRobot));
  ONSEM_FALSE(operator_check(twoPeopleTracking->text, semanticMemory, lingDb, fromRobot));
  EXPECT_EQ(constant::noResult, nobodyTracking->getResult());
  EXPECT_EQ("", onePersonTracking->getResult());
  EXPECT_EQ(constant::noResult, twoPeopleTracking->getResult());
  EXPECT_EQ("", threePeopleTracking->getResult());
  clearTrackersResults();
  idExp1 = operator_informAxiom_fromRobot("I see you and I see another person", semanticMemory, lingDb, {}, &axiomToConditionCurrentState);
  semanticMemory.memBloc.removeExpression(*idExp2, lingDb, &axiomToConditionCurrentState);
  ONSEM_ANSWER_EQ("I see 2 people.",
                  operator_answer("How many people do you see?", semanticMemory, lingDb));
  ONSEM_FALSE(operator_check(nobodyTracking->text, semanticMemory, lingDb, fromRobot));
  ONSEM_FALSE(operator_check(onePersonTracking->text, semanticMemory, lingDb, fromRobot));
  ONSEM_TRUE(operator_check(twoPeopleTracking->text, semanticMemory, lingDb, fromRobot));
  ONSEM_FALSE(operator_check(threePeopleTracking->text, semanticMemory, lingDb, fromRobot));
  EXPECT_EQ(constant::noResult, nobodyTracking->getResult());
  EXPECT_EQ(constant::noResult, onePersonTracking->getResult());
  EXPECT_EQ("", twoPeopleTracking->getResult());
  EXPECT_EQ(constant::noResult, threePeopleTracking->getResult());
  clearTrackersResults();
  idExp2 = operator_informAxiom_fromRobot("I see you and I see 2 other people", semanticMemory, lingDb, {}, &axiomToConditionCurrentState);
  semanticMemory.memBloc.removeExpression(*idExp1, lingDb, &axiomToConditionCurrentState);
  EXPECT_EQ(constant::noResult, nobodyTracking->getResult());
  EXPECT_EQ(constant::noResult, onePersonTracking->getResult());
  EXPECT_EQ(constant::noResult, twoPeopleTracking->getResult());
  EXPECT_EQ("", threePeopleTracking->getResult());
  clearTrackersResults();
  operator_inform_fromRobot("I don't see you and I see 0 people", semanticMemory, lingDb, {}, &axiomToConditionCurrentState); // contrary to an assert so nothing is raised
  EXPECT_EQ(constant::noResult, nobodyTracking->getResult());
  EXPECT_EQ(constant::noResult, onePersonTracking->getResult());
  EXPECT_EQ(constant::noResult, twoPeopleTracking->getResult());
  EXPECT_EQ(constant::noResult, threePeopleTracking->getResult());
  idExp1 = operator_informAxiom_fromRobot("I don't see you and I see 0 people", semanticMemory, lingDb, {}, &axiomToConditionCurrentState);
  semanticMemory.memBloc.removeExpression(*idExp2, lingDb, &axiomToConditionCurrentState);
  EXPECT_EQ("", nobodyTracking->getResult());
  EXPECT_EQ(constant::noResult, onePersonTracking->getResult());
  EXPECT_EQ("", twoPeopleTracking->getResult());
  EXPECT_EQ(constant::noResult, threePeopleTracking->getResult());
  clearTrackersResults();
  idExp2 = operator_informAxiom_fromRobot("I don't see you and I see one person", semanticMemory, lingDb, {}, &axiomToConditionCurrentState);
  semanticMemory.memBloc.removeExpression(*idExp1, lingDb, &axiomToConditionCurrentState);
  EXPECT_EQ(constant::noResult, nobodyTracking->getResult());
  EXPECT_EQ("", onePersonTracking->getResult());
  EXPECT_EQ(constant::noResult, twoPeopleTracking->getResult());
  EXPECT_EQ(constant::noResult, threePeopleTracking->getResult());
  clearTrackersResults();
  idExp1 = operator_informAxiom_fromRobot("I don't see you and I see one person", semanticMemory, lingDb, {}, &axiomToConditionCurrentState);
  semanticMemory.memBloc.removeExpression(*idExp2, lingDb, &axiomToConditionCurrentState);
  EXPECT_EQ(constant::noResult, nobodyTracking->getResult());
  EXPECT_EQ(constant::noResult, onePersonTracking->getResult());
  EXPECT_EQ(constant::noResult, twoPeopleTracking->getResult());
  EXPECT_EQ(constant::noResult, threePeopleTracking->getResult());
  clearTrackersResults();
  idExp2 = operator_informAxiom_fromRobot("I see you and I see 0 other people", semanticMemory, lingDb, {}, &axiomToConditionCurrentState);
  semanticMemory.memBloc.removeExpression(*idExp1, lingDb, &axiomToConditionCurrentState);
  EXPECT_EQ(constant::noResult, nobodyTracking->getResult());
  EXPECT_EQ(constant::noResult, onePersonTracking->getResult());
  EXPECT_EQ(constant::noResult, twoPeopleTracking->getResult());
  EXPECT_EQ(constant::noResult, threePeopleTracking->getResult());
  clearTrackersResults();
  idExp1 = operator_informAxiom_fromRobot("I see you and I see 1 other people", semanticMemory, lingDb, {}, &axiomToConditionCurrentState);
  semanticMemory.memBloc.removeExpression(*idExp2, lingDb, &axiomToConditionCurrentState);
  EXPECT_EQ(constant::noResult, nobodyTracking->getResult());
  EXPECT_EQ(constant::noResult, onePersonTracking->getResult());
  EXPECT_EQ("", twoPeopleTracking->getResult());
  EXPECT_EQ(constant::noResult, threePeopleTracking->getResult());
}

