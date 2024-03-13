#include <gtest/gtest.h>
#include <onsem/texttosemantic/tool/semexpmodifier.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemory.hpp>
#include "../semanticreasonergtests.hpp"
#include "../util/util.hpp"

using namespace onsem;

TEST_F(SemanticReasonerGTests, test_outputter) {
    const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
    SemanticMemory semMem;
    SemanticLanguageEnum enLanguage = SemanticLanguageEnum::ENGLISH;
    SemanticLanguageEnum frLanguage = SemanticLanguageEnum::FRENCH;

    // any text
    {
        auto semExpToOutput = textToSemExp("you are cool", lingDb, enLanguage);
        EXPECT_EQ("I am cool.", semExpToOutputStr(*semExpToOutput, enLanguage, semMem, lingDb));
    }

    auto lookLeftResource = std::make_unique<GroundedExpression>(std::make_unique<SemanticResourceGrounding>(
        resourceLabelForTests_cmd, SemanticLanguageEnum::UNKNOWN, "methodToLookLeft()"));

    // resource alone
    EXPECT_EQ("\\" + resourceLabelForTests_cmd + "=methodToLookLeft()\\",
              semExpToOutputStr(*lookLeftResource, enLanguage, semMem, lingDb));

    // resource inside an object
    {
        auto semExpToOutput = textToSemExp("To look left is", lingDb, enLanguage);
        SemExpModifier::addChildFromSemExp(
            *semExpToOutput, GrammaticalType::OBJECT, lookLeftResource->clone(), ListExpressionType::UNRELATED);
        EXPECT_EQ("To look left is \tTHEN\t\\" + resourceLabelForTests_cmd + "=methodToLookLeft()\\",
                  semExpToOutputStr(*semExpToOutput, enLanguage, semMem, lingDb));
    }

    // resource inside a location
    {
        auto semExpToOutput = textToSemExp("You can look at bugs", lingDb, enLanguage);
        auto redmineResource = std::make_unique<GroundedExpression>(std::make_unique<SemanticResourceGrounding>(
            resourceLabelForTests_url, SemanticLanguageEnum::UNKNOWN, "https://www.redmine.org/"));
        SemExpModifier::addChildFromSemExp(
            *semExpToOutput, GrammaticalType::LOCATION, redmineResource->clone(), ListExpressionType::UNRELATED);
        EXPECT_EQ("I can look at bugs here: \tTHEN\t\\" + resourceLabelForTests_url + "=https://www.redmine.org/\\",
                  semExpToOutputStr(*semExpToOutput, enLanguage, semMem, lingDb));
        EXPECT_EQ(
            "Je peux regarder des bogues ici : \tTHEN\t\\" + resourceLabelForTests_url + "=https://www.redmine.org/\\",
            semExpToOutputStr(*semExpToOutput, frLanguage, semMem, lingDb));
    }
}
