#include "../../semanticreasonergtests.hpp"
#include <gtest/gtest.h>
#include <onsem/common/utility/make_unique.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemory.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase.hpp>
#include <onsem/tester/reactOnTexts.hpp>
#include "../operators/operator_answer.hpp"
#include "../operators/operator_get.hpp"
#include "../operators/operator_inform.hpp"
#include "../operators/operator_show.hpp"
#include "dummycommentaryprovider.hpp"
#include "dummyjokeprovider.hpp"

using namespace onsem;


TEST_F(SemanticReasonerGTests, test_externalInfosProvider)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;

  ASSERT_TRUE(operator_get("what is the joke of today ?", semMem, lingDb).empty());

  semMem.registerExternalInfosProvider(mystd::make_unique<DummyJokeProvider>(lingDb), lingDb);
  semMem.unregisterExternalInfosProvider(DummyJokeProvider::idStrOfProv);

  ASSERT_TRUE(operator_get("what is the joke of today ?", semMem, lingDb).empty());

  semMem.registerExternalInfosProvider(mystd::make_unique<DummyJokeProvider>(lingDb), lingDb);

  {
    auto res = operator_get("what is the joke of today ?", semMem, lingDb);
    ASSERT_EQ(1u, res.size());
    EXPECT_EQ(DummyJokeProvider::englishJoke(), res[0]);
  }
  ONSEM_ANSWER_EQ("The joke of today is " + DummyJokeProvider::englishJoke(),
                  operator_answer("what is the joke of today ?", semMem, lingDb));

  ASSERT_TRUE(operator_get("what is the joke of tomorrow ?", semMem, lingDb).empty());

  semMem.unregisterExternalInfosProvider(DummyJokeProvider::idStrOfProv);
  semMem.clear();
  semMem.registerExternalInfosProvider(mystd::make_unique<DummyJokeProvider>(lingDb), lingDb);

  {
    auto res = operator_show("the joke of today", semMem, lingDb);
    ASSERT_EQ(1u, res.size());
    EXPECT_EQ(DummyJokeProvider::englishJoke(), res[0]);
  }

  semMem.unregisterExternalInfosProvider(DummyJokeProvider::idStrOfProv);
  semMem.clear();
  semMem.registerExternalInfosProvider(mystd::make_unique<DummyJokeProvider>(lingDb), lingDb);

  {
    auto res = operator_get("quel est la blague du 1 janvier 2000 ?", semMem, lingDb);
    ASSERT_EQ(1u, res.size());
    EXPECT_EQ(DummyJokeProvider::frenchJoke(), res[0]);
  }
  ONSEM_ANSWER_EQ(DummyJokeProvider::frenchJoke(), operator_react("Je veux lire la blague du 1 janvier 2000", semMem, lingDb));
  operator_inform_fromRobot("Est-ce que tu veux lire la blague du 1 janvier 2000 ?", semMem, lingDb);
  ONSEM_ANSWER_EQ(DummyJokeProvider::frenchJoke(), operator_react("Oui je veux bien", semMem, lingDb));
  operator_inform_fromRobot("Est-ce que tu veux lire la blague du 1 janvier 2000 ?", semMem, lingDb);
  ONSEM_FEEDBACK_EQ("Je pensais le contraire.", operator_react("Non je ne veux pas", semMem, lingDb));

  semMem.registerExternalInfosProvider(mystd::make_unique<DummyCommentaryProvider>(lingDb), lingDb);
  {
    auto res = operator_get("quel est le commentaire du livre d'aujourd'hui", semMem, lingDb);
    ASSERT_EQ(1u, res.size());
    EXPECT_EQ(DummyCommentaryProvider::frenchCommentary(), res[0]);
  }

  semMem.unregisterExternalInfosProvider(DummyJokeProvider::idStrOfProv);
  semMem.unregisterExternalInfosProvider(DummyCommentaryProvider::idStrOfProv);
  semMem.clear();

  {
    auto res = operator_get("what is the joke of today ?", semMem, lingDb);
    ASSERT_EQ(0u, res.size());
  }
  {
    auto res = operator_get("quel est le commentaire du livre d'aujourd'hui", semMem, lingDb);
    ASSERT_EQ(0u, res.size());
  }
}
