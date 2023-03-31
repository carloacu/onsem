#include "../semanticreasonergtests.hpp"
#include <onsem/tester/reactOnTexts.hpp>

using namespace onsem;


TEST_F(SemanticReasonerGTests, test_confidence)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;

  EXPECT_GT(100, textToConfidence("qui peuvent entre gars non", lingDb));

  EXPECT_EQ(100, textToConfidence("je suis content", lingDb));
}

