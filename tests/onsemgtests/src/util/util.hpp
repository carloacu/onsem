#ifndef ONSEM_GTESTS_UTIL_UTIL_HPP
#define ONSEM_GTESTS_UTIL_UTIL_HPP

#include <string>
#include <memory>
#include <gtest/gtest.h>
#include <boost/filesystem/path.hpp>
#include <onsem/common/utility/observable/connection.hpp>
#include <onsem/common/enum/grammaticaltype.hpp>
#include <onsem/common/enum/semanticlanguagetype.hpp>
#include <onsem/tester/detailedreactionanswer.hpp>

#define ONSEM_ANNOTATIONTYPE_EQ(ann1, ann2) EXPECT_EQ(ann1, ann2) << "Actual: " << contextualAnnotation_toStr(ann2) << "\nExpected: " << contextualAnnotation_toStr(ann1);
#define ONSEM_ANSWER_EQ(pExpectedAnswerStr, pExp) \
  { const auto& res = pExp; EXPECT_EQ(pExpectedAnswerStr, res.answer); ONSEM_ANNOTATIONTYPE_EQ(ContextualAnnotation::ANSWER, res.reactionType); }
#define ONSEM_ANSWERNOTFOUND_EQ(pExpectedAnswerStr, pExp) \
  { const auto& res = pExp; EXPECT_EQ(pExpectedAnswerStr, res.answer); ONSEM_ANNOTATIONTYPE_EQ(ContextualAnnotation::ANSWERNOTFOUND, res.reactionType); }
#define ONSEM_BEHAVIOR_EQ(pExpectedAnswerStr, pExp) \
  { const auto& res = pExp; EXPECT_EQ(pExpectedAnswerStr, res.answer); ONSEM_ANNOTATIONTYPE_EQ(ContextualAnnotation::BEHAVIOR, res.reactionType); }
#define ONSEM_BEHAVIORNOTFOUND_EQ(pExpectedAnswerStr, pExp) \
  { const auto& res = pExp; EXPECT_EQ(pExpectedAnswerStr, res.answer); ONSEM_ANNOTATIONTYPE_EQ(ContextualAnnotation::BEHAVIORNOTFOUND, res.reactionType); }
#define ONSEM_FALSE(pValue) EXPECT_EQ(::TruenessValue::VAL_FALSE, pValue);
#define ONSEM_FEEDBACK_EQ(pExpectedAnswerStr, pExp) \
  { const auto& res = pExp; EXPECT_EQ(pExpectedAnswerStr, res.answer); ONSEM_ANNOTATIONTYPE_EQ(ContextualAnnotation::FEEDBACK, res.reactionType); }
#define ONSEM_TEACHINGFEEDBACK_EQ(pExpectedAnswerStr, pExp) \
  { const auto& res = pExp; EXPECT_EQ(pExpectedAnswerStr, res.answer); ONSEM_ANNOTATIONTYPE_EQ(ContextualAnnotation::TEACHINGFEEDBACK, res.reactionType); }
#define ONSEM_EXTERNALTEACHINGREQUEST_EQ(pExpectedAnswerStr, pExp) \
  { const auto& res = pExp; EXPECT_EQ(pExpectedAnswerStr, res.answer); ONSEM_ANNOTATIONTYPE_EQ(ContextualAnnotation::EXTERNALTEACHINGREQUEST, res.reactionType); }
#define ONSEM_NOANSWER(pResult) EXPECT_EQ("", pResult.answer);
#define ONSEM_NOTIFYSOMETHINGWILLBEDONE_EQ(pExpectedAnswerStr, pExp) \
  { const auto& res = pExp; EXPECT_EQ(pExpectedAnswerStr, res.answer); ONSEM_ANNOTATIONTYPE_EQ(ContextualAnnotation::NOTIFYSOMETHINGWILLBEDONE, res.reactionType); }
#define ONSEM_QUESTION_EQ(pExpectedAnswerStr, pExp) \
  { const auto& res = pExp; EXPECT_EQ(pExpectedAnswerStr, res.answer); ONSEM_ANNOTATIONTYPE_EQ(ContextualAnnotation::QUESTION, res.reactionType); }
#define ONSEM_REMOVEALLCONDITIONS_EQ(pExpectedAnswerStr, pExp) \
  { const auto& res = pExp; EXPECT_EQ(pExpectedAnswerStr, res.answer); ONSEM_ANNOTATIONTYPE_EQ(ContextualAnnotation::REMOVEALLCONDITIONS, res.reactionType); }
#define ONSEM_TRUE(pValue) EXPECT_EQ(::TruenessValue::VAL_TRUE, pValue);
#define ONSEM_UNKNOWN(pValue) EXPECT_EQ(::TruenessValue::UNKNOWN, pValue);


namespace onsem
{
namespace linguistics
{
  struct LinguisticDatabase;
}
struct SemanticMemory;
struct UniqueSemanticExpression;

static const std::string commonStr = "common";
static const std::string frenchStr = "french";
static const std::string englishStr = "english";
static const std::string japaneseStr = "japanese";


UniqueSemanticExpression textToSemExp
(const std::string& pText,
 const linguistics::LinguisticDatabase& pLingDb,
 SemanticLanguageEnum pLanguage = SemanticLanguageEnum::UNKNOWN);

UniqueSemanticExpression textToSemExpFromRobot
(const std::string& pText,
 const linguistics::LinguisticDatabase& pLingDb,
 SemanticLanguageEnum pLanguage = SemanticLanguageEnum::UNKNOWN);

UniqueSemanticExpression textToContextualSemExp
(const std::string& pText,
 const linguistics::LinguisticDatabase& pLingDb,
 SemanticLanguageEnum pLanguage);

std::string semExpToText
(UniqueSemanticExpression pSemExp,
 SemanticLanguageEnum pLanguage,
 const SemanticMemory& pSemanticMemory,
 const linguistics::LinguisticDatabase& pLingDb);

std::string semExpToTextExectionResult
(UniqueSemanticExpression pSemExp,
 SemanticLanguageEnum pLanguage,
 SemanticMemory& pSemanticMemory,
 const linguistics::LinguisticDatabase& pLingDb);

std::string reformulate
(const std::string& pText,
 const SemanticMemory& pSemanticMemory,
 const linguistics::LinguisticDatabase& pLingDb,
 SemanticLanguageEnum pLanguage);

void compareWithRef(const std::string& pExpectedAnswerStr,
                    const std::list<std::string>& pReferences,
                    const DetailedReactionAnswer& pResult);

void copyMemory(SemanticMemory& pNewSemMem,
                const SemanticMemory& pNewSemMemToCopy,
                const linguistics::LinguisticDatabase& pLingDb);

UniqueSemanticExpression invertPolarity(UniqueSemanticExpression pUSemExp);
UniqueSemanticExpression removeChild(UniqueSemanticExpression pUSemExp,
                                     GrammaticalType pChildType);

class TrackSemMemoryNotifications
{
public:
  TrackSemMemoryNotifications(SemanticMemory& pSemMem,
                              const linguistics::LinguisticDatabase& pLingDb);
  ~TrackSemMemoryNotifications();

  const std::string& getInfActions() const { return _infActions; }
  const std::string& getConditionToActions() const { return _conditionToActions; }

private:
  SemanticMemory& _semMem;
  const linguistics::LinguisticDatabase& _lingDb;
  mystd::observable::Connection _infActionChangedConnection;
  std::string _infActions;
  mystd::observable::Connection _conditionToActionChangedConnection;
  std::string _conditionToActions;
};


} // End of namespace onsem


#endif // ONSEM_GTESTS_UTIL_UTIL_HPP
