#include <gtest/gtest.h>
#include <onsem/texttosemantic/dbtype/linguisticdatabase.hpp>
#include <onsem/texttosemantic/linguisticanalyzer.hpp>
#include "../semanticreasonergtests.hpp"

using namespace onsem;

namespace
{
std::string _extractNewProperNounsInStr(
    const std::string& pText,
    SemanticLanguageEnum pLanguage,
    const linguistics::LinguisticDatabase& pLingDb)
{
  std::set<std::string> newProperNouns;
  linguistics::extractProperNounsThatDoesntHaveAnyOtherGrammaticalTypes(newProperNouns, pText, pLanguage, pLingDb);
  std::string res;
  bool firstIt = true;
  for (const auto& currProperNoun : newProperNouns)
  {
    if (firstIt)
    {
      res = currProperNoun;
      firstIt = false;
    }
    else
    {
      res += ", " + currProperNoun;

    }
  }
  return res;
}
}


TEST_F(SemanticReasonerGTests, check_extractNewProperNouns)
{
  const auto& lingDb = *lingDbPtr;
  const auto frLanguage = SemanticLanguageEnum::FRENCH;

  EXPECT_EQ("Ftrer", _extractNewProperNounsInStr("Paul Ftrer", frLanguage, lingDb));
  EXPECT_EQ("Barack, Obama", _extractNewProperNounsInStr("Barack Obama", frLanguage, lingDb));
  EXPECT_EQ("Nadar", _extractNewProperNounsInStr("Paul Nadar (photographe)", frLanguage, lingDb));
  EXPECT_EQ("Nadar", _extractNewProperNounsInStr("Paul Nadar photographe", frLanguage, lingDb));
  EXPECT_EQ("Nadar", _extractNewProperNounsInStr("Paul Né Nadar photographe", frLanguage, lingDb));
}


TEST_F(SemanticReasonerGTests, check_isAProperNoun)
{
  const auto& lingDb = *lingDbPtr;

  EXPECT_TRUE(linguistics::isAProperNoun("Sandrine", lingDb));
  EXPECT_TRUE(linguistics::isAProperNoun("Yasmine", lingDb));
  EXPECT_TRUE(linguistics::isAProperNoun("Yasmina", lingDb));
  EXPECT_TRUE(linguistics::isAProperNoun("Jasmine", lingDb));
  EXPECT_TRUE(linguistics::isAProperNoun("Victor", lingDb));
  EXPECT_TRUE(linguistics::isAProperNoun("Edna", lingDb));
  EXPECT_TRUE(linguistics::isAProperNoun("Brian", lingDb));
  EXPECT_TRUE(linguistics::isAProperNoun("Jessica", lingDb));
  EXPECT_TRUE(linguistics::isAProperNoun("Taylor", lingDb));
  EXPECT_TRUE(linguistics::isAProperNoun("Joël", lingDb));
  EXPECT_TRUE(linguistics::isAProperNoun("Timothée", lingDb));
  EXPECT_FALSE(linguistics::isAProperNoun("Sandrinea", lingDb));
  EXPECT_FALSE(linguistics::isAProperNoun("Porte", lingDb));
}
