#include <gtest/gtest.h>
#include "../semanticreasonergtests.hpp"
#include "../util/util.hpp"


using namespace onsem;


TEST_F(SemanticReasonerGTests, checkThatTwoTextsHaveTheSameSemanticRepresentation)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;

  EXPECT_EQ(*textToSemExp("Est-ce que tu connais N5 ?", lingDb),
            *textToSemExp("Connais-tu N5 ?", lingDb));

  EXPECT_NE(*textToSemExp("Est-ce que tu connais N5 ?", lingDb),
            *textToSemExp("Connais-tu bien N5 ?", lingDb));

  EXPECT_EQ(*textToSemExp("Est-ce que tu connais N5 ?", lingDb),
            *removeChild(textToSemExp("Connais-tu bien N5 ?", lingDb), GrammaticalType::MANNER));

  EXPECT_EQ(*textToSemExp("Est-ce que tu connais déjà bien N5 ?", lingDb),
            *textToSemExp("Connais-tu déjà bien N5 ?", lingDb));

  EXPECT_EQ(*textToSemExp("Are you not sad?", lingDb),
            *textToSemExp("You are not sad?", lingDb));

  EXPECT_NE(*textToSemExp("Are you not sad?", lingDb),
            *textToSemExp("Are you sad?", lingDb));

  EXPECT_EQ(*textToSemExp("Are you not sad?", lingDb),
            *invertPolarity(textToSemExp("Are you sad?", lingDb)));
}
