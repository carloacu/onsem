#include <gtest/gtest.h>
#include <boost/algorithm/string.hpp>
#include <onsem/semantictotext/semanticconverter.hpp>
#include <onsem/common/enum/semanticsourceenum.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase.hpp>
#include <onsem/texttosemantic/dbtype/semanticgroundings.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/groundedexpression.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/fixedsynthesisexpression.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemory.hpp>
#include <onsem/common/utility/make_unique.hpp>
#include "../semanticreasonergtests.hpp"
#include "../util/util.hpp"


using namespace onsem;

TEST_F(SemanticReasonerGTests, infinitiveVerb)
{
  auto statement = mystd::make_unique<SemanticStatementGrounding>();
  statement->word.setContent(SemanticLanguageEnum::ENGLISH, "greet", PartOfSpeech::VERB);
  statement->verbTense = SemanticVerbTense::UNKNOWN;
  auto expression = mystd::make_unique<GroundedExpression>(std::move(statement));

  TextProcessingContext textContext{
    SemanticAgentGrounding::me,
    SemanticAgentGrounding::currentUser,
    SemanticLanguageEnum::ENGLISH};

  std::string synthesized;
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory memory;
  converter::semExpToText(
        synthesized, std::move(expression), textContext, false, memory, lingDb, nullptr);

  ASSERT_TRUE(boost::iequals("to greet.", synthesized))
      << "Got \"" << synthesized << "\" instead";
}


TEST_F(SemanticReasonerGTests, simpleSentences)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;

  EXPECT_EQ("Robert eats.", reformulate("Robert eats", semMem, lingDb, SemanticLanguageEnum::ENGLISH));
  EXPECT_EQ("Robert walks on the street.", reformulate("Robert walks on the street.", semMem, lingDb, SemanticLanguageEnum::ENGLISH));
  EXPECT_EQ("Robert looks at you.", reformulate("Robert looks at me.", semMem, lingDb, SemanticLanguageEnum::ENGLISH));
  EXPECT_EQ("Eat the cucumber!", reformulate("eat the cucumber", semMem, lingDb, SemanticLanguageEnum::ENGLISH));
  EXPECT_EQ("Robert te regarde.", reformulate("Robert me regarde", semMem, lingDb, SemanticLanguageEnum::FRENCH));
  EXPECT_EQ("Eminem de son vrai nom Marshall Bruce Mathers III n\xC3\xA9 le 17 octobre 1972 \xC3\xA0 Saint Joseph dans l'\xC3\x89tat du Missouri stylis\xC3\xA9 souvent EMIN\xC6\x8EM est un auteur-compositeur-interpr\xC3\xA8te de rap am\xC3\xA9ricain, il est \xC3\xA9galement producteur et il est acteur. Il est aussi membre du groupe D12 dont il est le cofondateur.",
            reformulate("Eminem (souvent stylisé EMINƎM), de son vrai nom Marshall Bruce Mathers III, né le 17 octobre 1972 à Saint Joseph dans l'État du Missouri, est un auteur-compositeur-interprète de rap américain, également producteur et acteur. Il est aussi membre du groupe D12 dont il est le cofondateur", semMem, lingDb, SemanticLanguageEnum::FRENCH));
}

TEST_F(SemanticReasonerGTests, reformulateWhenNobodyIsFocused)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;

  EXPECT_EQ("You eat chocolate.", reformulate("I eat chocolate", semMem, lingDb, SemanticLanguageEnum::ENGLISH));
}


TEST_F(SemanticReasonerGTests, reformulateWithPresentContinuousOnlyWhenAppropriate)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;

  // appearance -> ko
  EXPECT_EQ("You seem.", reformulate("I am seeming", semMem, lingDb, SemanticLanguageEnum::ENGLISH));

  // emotion -> ko
  EXPECT_EQ("You like.", reformulate("I am liking", semMem, lingDb, SemanticLanguageEnum::ENGLISH));
  EXPECT_EQ("You love.", reformulate("I am loving", semMem, lingDb, SemanticLanguageEnum::ENGLISH));

  // mentalState -> ko
  EXPECT_EQ("You believe.", reformulate("I am believing", semMem, lingDb, SemanticLanguageEnum::ENGLISH));
  EXPECT_EQ("You doubt.", reformulate("I am doubting", semMem, lingDb, SemanticLanguageEnum::ENGLISH));
  EXPECT_EQ("You know.", reformulate("I am knowing", semMem, lingDb, SemanticLanguageEnum::ENGLISH));
  EXPECT_EQ("You think.", reformulate("I am thinking", semMem, lingDb, SemanticLanguageEnum::ENGLISH));
  EXPECT_EQ("You understand.", reformulate("I am understanding", semMem, lingDb, SemanticLanguageEnum::ENGLISH));

  // perception -> ko
  EXPECT_EQ("You feel.", reformulate("I am feeling", semMem, lingDb, SemanticLanguageEnum::ENGLISH));
  EXPECT_EQ("You hear.", reformulate("I am hearing", semMem, lingDb, SemanticLanguageEnum::ENGLISH));
  EXPECT_EQ("You see.", reformulate("I am seeing", semMem, lingDb, SemanticLanguageEnum::ENGLISH));
  EXPECT_EQ("You sound.", reformulate("I am sounding", semMem, lingDb, SemanticLanguageEnum::ENGLISH));
  EXPECT_EQ("You smell.", reformulate("I am smelling", semMem, lingDb, SemanticLanguageEnum::ENGLISH));
  EXPECT_EQ("You taste.", reformulate("I am tasting", semMem, lingDb, SemanticLanguageEnum::ENGLISH));

  // other-> ok
  EXPECT_EQ("You are smiling.", reformulate("I am smiling", semMem, lingDb, SemanticLanguageEnum::ENGLISH));
  EXPECT_EQ("You are looking.", reformulate("I am looking", semMem, lingDb, SemanticLanguageEnum::ENGLISH));
}


TEST_F(SemanticReasonerGTests, reformulateNumbers)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;
  // en
  EXPECT_EQ("21", reformulate("twenty one", semMem, lingDb, SemanticLanguageEnum::ENGLISH));
  EXPECT_EQ("22", reformulate("twenty two", semMem, lingDb, SemanticLanguageEnum::ENGLISH));
  // fr
  EXPECT_EQ("33", reformulate("trente trois", semMem, lingDb, SemanticLanguageEnum::FRENCH));
  EXPECT_EQ("42", reformulate("quarante deux", semMem, lingDb, SemanticLanguageEnum::FRENCH));
}


TEST_F(SemanticReasonerGTests, reformulateTime)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;
  // en
  SemanticTimeGrounding::hardCodedCurrentTimeOfDay->timeInfos[SemanticTimeUnity::HOUR] = 7;
  EXPECT_EQ("8:30 am", reformulate("half past eight", semMem, lingDb, SemanticLanguageEnum::ENGLISH));
  EXPECT_EQ("8:15 am", reformulate("quarter past eight", semMem, lingDb, SemanticLanguageEnum::ENGLISH));
  EXPECT_EQ("7:45 am", reformulate("quarter to eight am", semMem, lingDb, SemanticLanguageEnum::ENGLISH));
  EXPECT_EQ("8:20 am", reformulate("twenty past eight", semMem, lingDb, SemanticLanguageEnum::ENGLISH));
  EXPECT_EQ("8:20 am", reformulate("twenty past eight a.m", semMem, lingDb, SemanticLanguageEnum::ENGLISH));
  EXPECT_EQ("8:20 pm", reformulate("twenty past eight p.m", semMem, lingDb, SemanticLanguageEnum::ENGLISH));
  EXPECT_EQ("7:40 pm", reformulate("twenty to eight p.m", semMem, lingDb, SemanticLanguageEnum::ENGLISH));

  // if we are already in the afternoon, we asume we are talking about the afternoon
  SemanticTimeGrounding::hardCodedCurrentTimeOfDay->timeInfos[SemanticTimeUnity::HOUR] = 14;
  EXPECT_EQ("8:20 pm", reformulate("twenty past eight", semMem, lingDb, SemanticLanguageEnum::ENGLISH));

  SemanticTimeGrounding::hardCodedCurrentTimeOfDay->timeInfos[SemanticTimeUnity::HOUR] = 22;
  EXPECT_EQ("Today", reformulate("today", semMem, lingDb, SemanticLanguageEnum::ENGLISH));
}


TEST_F(SemanticReasonerGTests, fixedSynthesis)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;

  SemanticLanguageEnum enLanguage = SemanticLanguageEnum::ENGLISH;
  auto semExp = textToSemExp("Who is Paul?", lingDb, enLanguage);
  const std::string fiexedSynthesisStr = "Whol is Paul ? (from fixed synthesis)";
  auto fSynthExp = mystd::make_unique<FixedSynthesisExpression>(std::move(semExp));
  fSynthExp->langToSynthesis.emplace(enLanguage, fiexedSynthesisStr);
  EXPECT_EQ(fiexedSynthesisStr, semExpToText(std::move(fSynthExp), enLanguage, semMem, lingDb));
}


TEST_F(SemanticReasonerGTests, vouvoiementInFrench)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;

  SemanticLanguageEnum enLanguage = SemanticLanguageEnum::FRENCH;
  auto semExp = textToSemExpFromRobot("Veux-tu voir des informations sur l'entreprise ?", lingDb, enLanguage);

  TextProcessingContext outTextProc(SemanticAgentGrounding::me,
                                    SemanticAgentGrounding::currentUser,
                                    enLanguage);
  outTextProc.vouvoiement = true;
  std::string reformulatedQuestion;
  converter::semExpToText(reformulatedQuestion, std::move(semExp), outTextProc,
                          false, semMem, lingDb, nullptr);

  EXPECT_EQ("Voulez-vous voir des informations sur l'entreprise ?", reformulatedQuestion);
}
