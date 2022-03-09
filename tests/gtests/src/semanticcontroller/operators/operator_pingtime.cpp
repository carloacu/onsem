#include "../../semanticreasonergtests.hpp"
#include <gtest/gtest.h>
#include <onsem/texttosemantic/dbtype/linguisticdatabase.hpp>
#include <onsem/texttosemantic/tool/semexpgetter.hpp>
#include <onsem/texttosemantic/languagedetector.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemory.hpp>
#include <onsem/semantictotext/semanticconverter.hpp>
#include <onsem/semantictotext/semexpoperators.hpp>
#include <onsem/tester/reactOnTexts.hpp>


using namespace onsem;


DetailedReactionAnswer operator_pingtime(
    const SemanticDuration& pSemDuration,
    SemanticMemory& pSemanticMemory,
    const linguistics::LinguisticDatabase& pLingDb,
    SemanticLanguageEnum pOutLanguage)
{
  mystd::unique_propagate_const<UniqueSemanticExpression> reaction;
  memoryOperation::pingTime(reaction, pSemanticMemory, pSemDuration, pLingDb);
  return reactionToAnswer(reaction, pSemanticMemory, pLingDb, pOutLanguage);
}



TEST_F(SemanticReasonerGTests, operator_pingtime_basic)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;
  memoryOperation::learnSayCommand(semMem, lingDb);
  std::list<SemanticDuration> semanticBeginOfTimes;
  auto timeConditionConnection = semMem.memBloc.semanticTimesWithConditionsLinked.connectUnsafe([&](const SemanticDuration& pSemanticBeginOfTime)
  {
    semanticBeginOfTimes.emplace_back(pSemanticBeginOfTime);
  });

  ONSEM_BEHAVIOR_EQ("Hi", operator_react("say hi", semMem, lingDb));
  EXPECT_EQ(0u, semanticBeginOfTimes.size());
  ONSEM_NOTIFYSOMETHINGWILLBEDONE_EQ("Ok, I will say hi tomorrow.", operator_react("say hi tomorrow", semMem, lingDb));
  ASSERT_EQ(1u, semanticBeginOfTimes.size());
  ONSEM_BEHAVIOR_EQ("Hi", operator_pingtime(semanticBeginOfTimes.front(), semMem, lingDb, SemanticLanguageEnum::ENGLISH));
  semanticBeginOfTimes.pop_front();

  ONSEM_BEHAVIOR_EQ("Something", operator_react("say something now", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("Quelque chose", operator_react("dis quelque chose maintenant", semMem, lingDb));

  {
    ONSEM_NOTIFYSOMETHINGWILLBEDONE_EQ("Ok, I will say hi in 2 minutes.", operator_react("say hi in 2 minutes", semMem, lingDb));
    ASSERT_EQ(1u, semanticBeginOfTimes.size());
    const SemanticDuration& absoluteDuration = semanticBeginOfTimes.front();
    SemanticDuration relativeDuration =
        SemanticTimeGrounding::absoluteToRelative(absoluteDuration);
    EXPECT_EQ(SemanticDurationSign::POSITIVE, relativeDuration.sign);
    EXPECT_EQ(1u, relativeDuration.timeInfos.size());
    auto itTimeInfos = relativeDuration.timeInfos.begin();
    EXPECT_EQ(SemanticTimeUnity::MINUTE, itTimeInfos->first);
    EXPECT_EQ(2, itTimeInfos->second);

    ONSEM_BEHAVIOR_EQ("Hi", operator_pingtime(absoluteDuration, semMem, lingDb, SemanticLanguageEnum::ENGLISH));
    semanticBeginOfTimes.pop_front();
  }

  {
    ONSEM_NOTIFYSOMETHINGWILLBEDONE_EQ("Ok, I will say toto in 500 milliseconds.", operator_react("say toto in 500 milliseconds", semMem, lingDb));
    ASSERT_EQ(1u, semanticBeginOfTimes.size());
    const SemanticDuration& absoluteDuration = semanticBeginOfTimes.front();
    SemanticDuration relativeDuration =
        SemanticTimeGrounding::absoluteToRelative(absoluteDuration);
    EXPECT_EQ(SemanticDurationSign::POSITIVE, relativeDuration.sign);
    EXPECT_EQ(1u, relativeDuration.timeInfos.size());
    auto itTimeInfos = relativeDuration.timeInfos.begin();
    EXPECT_EQ(SemanticTimeUnity::MILLISECOND, itTimeInfos->first);
    EXPECT_EQ(500, itTimeInfos->second);

    ONSEM_BEHAVIOR_EQ("Toto", operator_pingtime(absoluteDuration, semMem, lingDb, SemanticLanguageEnum::ENGLISH));
    semanticBeginOfTimes.pop_front();
  }

  semMem.memBloc.semanticTimesWithConditionsLinked.disconnectUnsafe(timeConditionConnection);
}
