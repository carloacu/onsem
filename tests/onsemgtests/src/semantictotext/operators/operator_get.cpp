#include "operator_get.hpp"
#include <gtest/gtest.h>
#include "operator_inform.hpp"
#include <onsem/texttosemantic/dbtype/linguisticdatabase.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/groundedexpression.hpp>
#include <onsem/texttosemantic/languagedetector.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemory.hpp>
#include <onsem/semantictotext/semexpoperators.hpp>
#include <onsem/semantictotext/semanticconverter.hpp>
#include "../../semanticreasonergtests.hpp"

using namespace onsem;

namespace onsem
{
std::vector<std::string> operator_get(const std::string& pText,
                                      const SemanticMemory& pSemanticMemory,
                                      const linguistics::LinguisticDatabase& pLingDb)
{
  SemanticLanguageEnum language = linguistics::getLanguage(pText, pLingDb);
  TextProcessingContext inTextProc(SemanticAgentGrounding::currentUser,
                                   SemanticAgentGrounding::me,
                                   language);
  auto semExp = converter::textToContextualSemExp
      (pText, inTextProc, SemanticSourceEnum::ASR, pLingDb);
  memoryOperation::resolveAgentAccordingToTheContext(semExp, pSemanticMemory, pLingDb);
  std::vector<std::unique_ptr<GroundedExpression>> answers;
  memoryOperation::get(answers, std::move(semExp), pSemanticMemory, pLingDb);

  TextProcessingContext outContext(SemanticAgentGrounding::currentUser,
                                   SemanticAgentGrounding::me,
                                   language);
  std::vector<std::string> res(answers.size());
  std::size_t i = 0;
  for (auto& currAnswer : answers)
  {
    std::string subRes;
    converter::semExpToText(subRes, std::move(currAnswer), outContext,
                            true, pSemanticMemory, pLingDb, nullptr);
    res[i++] = subRes;
  }
  return res;
}

} // End of namespace onsem



TEST_F(SemanticReasonerGTests, operator_get_basic)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;
  operator_inform("Paul likes chocolate", semMem, lingDb);

  {
    auto res = operator_get("what does Paul like ?", semMem, lingDb);
    ASSERT_EQ(1u, res.size());
    ASSERT_EQ("Chocolate", res[0]);
  }
  {
    auto res = operator_get("what Paul likes ?", semMem, lingDb);
    ASSERT_EQ(1u, res.size());
    ASSERT_EQ("Chocolate", res[0]);
  }
  {
    auto res = operator_get("who likes chocolate ?", semMem, lingDb);
    ASSERT_EQ(1u, res.size());
    ASSERT_EQ("Paul", res[0]);
  }
  {
    auto res = operator_get("who likes ?", semMem, lingDb);
    ASSERT_EQ(1u, res.size());
    ASSERT_EQ("Paul", res[0]);
  }
  {
    auto res = operator_get("Does Paul like chocolate ?", semMem, lingDb);
    ASSERT_EQ(1u, res.size());
    ASSERT_EQ("True", res[0]);
  }
  {
    auto res = operator_get("Doesn't' Paul like chocolate ?", semMem, lingDb);
    ASSERT_EQ(1u, res.size());
    ASSERT_EQ("False", res[0]);
  }
  {
    auto res = operator_get("Does Paul like beer ?", semMem, lingDb);
    ASSERT_EQ(0u, res.size());
  }

  {
    operator_inform("Paul est un chanteur", semMem, lingDb);
    auto res = operator_get("quel est le métier de Paul", semMem, lingDb);
    ASSERT_EQ(1u, res.size());
    ASSERT_EQ("Un chanteur", res[0]);
  }
  {
    operator_inform("Pierre est un chanteur et un homme dévoué", semMem, lingDb);
    auto res = operator_get("quel est le métier de Pierre", semMem, lingDb);
    ASSERT_EQ(1u, res.size());
    ASSERT_EQ("Un chanteur", res[0]);
  }

  {
    operator_inform("si j'aime le chocolat, je suis gourmand", semMem, lingDb);
    ASSERT_TRUE(operator_get("je suis gourmand", semMem, lingDb).empty());
    operator_inform("j'aime le chocolat", semMem, lingDb);
    auto res = operator_get("pourquoi je suis gourmand ?", semMem, lingDb);
    ASSERT_EQ(1u, res.size());
    ASSERT_EQ("J'aime le chocolat.", res[0]);
  }

  // time information
  {
    ASSERT_TRUE(operator_get("quand je suis content ?", semMem, lingDb).empty());
    operator_inform("je suis content quand tu souris", semMem, lingDb);
    auto res = operator_get("quand je suis content ?", semMem, lingDb);
    ASSERT_EQ(1u, res.size());
    ASSERT_EQ("Tu souris.", res[0]);
  }
}

TEST_F(SemanticReasonerGTests, operator_get_lists)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;
  operator_inform("Paul likes chocolate", semMem, lingDb);
  operator_inform("Paul eats beaf", semMem, lingDb);
  operator_inform("Paul eats chocolate", semMem, lingDb);

  // (a) && (ba) => a
  {
    auto res = operator_get("what does Paul like and what does Paul eat ?", semMem, lingDb);
    ASSERT_EQ(1u, res.size());
    ASSERT_EQ("Chocolate", res[0]);
  }
  // (ba) && (a) => a
  {
    auto res = operator_get("what does Paul eat and what does Paul like ?", semMem, lingDb);
    ASSERT_EQ(1u, res.size());
    ASSERT_EQ("Chocolate", res[0]);
  }

  // (a) || (ba) => a
  {
    auto res = operator_get("what does Paul like or what does Paul eat ?", semMem, lingDb);
    ASSERT_EQ(1u, res.size());
    ASSERT_EQ("Chocolate", res[0]);
  }
  // (ba) || (a) => a
  {
    auto res = operator_get("what does Paul eat or what does Paul like ?", semMem, lingDb);
    ASSERT_EQ(2u, res.size());
    ASSERT_EQ("Beaf", res[0]);
    ASSERT_EQ("Chocolate", res[1]);
  }

  // (a) , (ba) => ab
  {
    auto res = operator_get("what does Paul like ? what does Paul eat ?", semMem, lingDb);
    ASSERT_EQ(2u, res.size());
    ASSERT_EQ("Chocolate", res[0]);
    ASSERT_EQ("Beaf", res[1]);
  }
  // (ba) , (a) => ba
  {
    auto res = operator_get("what does Paul eat ? what does Paul like ?", semMem, lingDb);
    ASSERT_EQ(2u, res.size());
    ASSERT_EQ("Beaf", res[0]);
    ASSERT_EQ("Chocolate", res[1]);
  }
}

TEST_F(SemanticReasonerGTests, operator_get_choice)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;

  const std::string question = "Do I like chocolate or banana?";
  EXPECT_TRUE(operator_get(question, semMem, lingDb).empty());
  operator_inform("I don't like chocolate", semMem, lingDb);
  EXPECT_TRUE(operator_get(question, semMem, lingDb).empty());
  operator_inform("I like banana", semMem, lingDb);
  {
    auto res = operator_get(question, semMem, lingDb);
    ASSERT_EQ(1u, res.size());
    EXPECT_EQ("Banana", res[0]);
  }
  operator_inform("I like chocolate", semMem, lingDb);
  {
    auto res = operator_get(question, semMem, lingDb);
    ASSERT_EQ(2u, res.size());
    EXPECT_EQ("Chocolate", res[0]);
    EXPECT_EQ("Banana", res[1]);
  }
  operator_inform("I like butter", semMem, lingDb);
  {
    auto res = operator_get(question, semMem, lingDb);
    ASSERT_EQ(2u, res.size());
    EXPECT_EQ("Chocolate", res[0]);
    EXPECT_EQ("Banana", res[1]);
  }
}

