#include <gtest/gtest.h>
#include "../semanticreasonergtests.hpp"
#include "../util/util.hpp"

using namespace onsem;

namespace {

UniqueSemanticExpression _textToSemExpWithoutOriginalText(
        const std::string& pText,
        const linguistics::LinguisticDatabase& pLingDb) {
    return textToSemExp(pText, pLingDb, SemanticLanguageEnum::UNKNOWN, false);

}

}


TEST_F(SemanticReasonerGTests, checkThatTwoTextsHaveTheSameSemanticRepresentation) {
    const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;

    EXPECT_EQ(*_textToSemExpWithoutOriginalText("Est-ce que tu connais N5 ?", lingDb),
              *_textToSemExpWithoutOriginalText("Connais-tu N5 ?", lingDb));

    EXPECT_NE(*_textToSemExpWithoutOriginalText("Est-ce que tu connais N5 ?", lingDb),
              *_textToSemExpWithoutOriginalText("Connais-tu bien N5 ?", lingDb));

    EXPECT_EQ(*_textToSemExpWithoutOriginalText("Est-ce que tu connais N5 ?", lingDb),
              *removeChild(_textToSemExpWithoutOriginalText("Connais-tu bien N5 ?", lingDb), GrammaticalType::MANNER));

    EXPECT_EQ(*_textToSemExpWithoutOriginalText("Est-ce que tu connais déjà bien N5 ?", lingDb),
              *_textToSemExpWithoutOriginalText("Connais-tu déjà bien N5 ?", lingDb));

    EXPECT_EQ(*_textToSemExpWithoutOriginalText("Are you not sad?", lingDb),
              *_textToSemExpWithoutOriginalText("You are not sad?", lingDb));

    EXPECT_NE(*_textToSemExpWithoutOriginalText("Are you not sad?", lingDb),
              *_textToSemExpWithoutOriginalText("Are you sad?", lingDb));

    EXPECT_EQ(*_textToSemExpWithoutOriginalText("Are you not sad?", lingDb),
              *invertPolarity(_textToSemExpWithoutOriginalText("Are you sad?", lingDb)));
}
