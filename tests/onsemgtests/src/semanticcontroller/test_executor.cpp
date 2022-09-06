#include <gtest/gtest.h>
#include <onsem/texttosemantic/tool/semexpmodifier.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemory.hpp>
#include "../semanticreasonergtests.hpp"
#include "../util/util.hpp"

using namespace onsem;


TEST_F(SemanticReasonerGTests, test_textExecutor)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;
  SemanticLanguageEnum enLanguage = SemanticLanguageEnum::ENGLISH;
  SemanticLanguageEnum frLanguage = SemanticLanguageEnum::FRENCH;

  // any text
  {
    auto semExpToExecute = textToSemExp("you are cool", lingDb, enLanguage);
    EXPECT_EQ("I am cool.", semExpToTextExectionResult(std::move(semExpToExecute),
                                                       enLanguage, semMem, lingDb));
  }


  auto lookLeftResource = mystd::make_unique<GroundedExpression>
      (mystd::make_unique<SemanticResourceGrounding>(resourceLabelForTests_cmd,
                                                     SemanticLanguageEnum::UNKNOWN,
                                                     "methodToLookLeft()"));

  // resource alone
  EXPECT_EQ("\\" + resourceLabelForTests_cmd + "=methodToLookLeft()\\",
            semExpToTextExectionResult(lookLeftResource->clone(),
                                       enLanguage, semMem, lingDb));

  // resource inside an object
  {
    auto semExpToExecute = textToSemExp("To look left is", lingDb, enLanguage);
    SemExpModifier::addChildFromSemExp(*semExpToExecute, GrammaticalType::OBJECT,
                                       lookLeftResource->clone(), ListExpressionType::UNRELATED);
    EXPECT_EQ("To look left is \\" + resourceLabelForTests_cmd + "=methodToLookLeft()\\",
              semExpToTextExectionResult(std::move(semExpToExecute),
                                         enLanguage, semMem, lingDb));
  }

  // resource inside a location
  {
    auto semExpToExecute = textToSemExp("You can look at bugs", lingDb, enLanguage);
    auto redmineResource = mystd::make_unique<GroundedExpression>
        (mystd::make_unique<SemanticResourceGrounding>(resourceLabelForTests_url,
                                                       SemanticLanguageEnum::UNKNOWN,
                                                       "https://www.redmine.org/"));
    SemExpModifier::addChildFromSemExp(*semExpToExecute, GrammaticalType::LOCATION,
                                       redmineResource->clone(), ListExpressionType::UNRELATED);
    EXPECT_EQ("I can look at bugs here: \\" + resourceLabelForTests_url + "=https://www.redmine.org/\\",
              semExpToTextExectionResult(semExpToExecute->clone(), enLanguage, semMem, lingDb));
    EXPECT_EQ("Je peux regarder des bogues ici : \\" + resourceLabelForTests_url + "=https://www.redmine.org/\\",
              semExpToTextExectionResult(std::move(semExpToExecute), frLanguage, semMem, lingDb));
  }
}

