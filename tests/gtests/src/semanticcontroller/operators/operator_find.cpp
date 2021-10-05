#include <gtest/gtest.h>
#include "operator_inform.hpp"
#include <onsem/texttosemantic/languagedetector.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/groundedexpression.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemory.hpp>
#include <onsem/semantictotext/semanticconverter.hpp>
#include <onsem/semantictotext/semexpoperators.hpp>
#include "../../semanticreasonergtests.hpp"

using namespace onsem;

namespace
{
std::vector<std::string> operator_find(const std::string& pText,
                                       SemanticMemory& pSemanticMemory,
                                       const linguistics::LinguisticDatabase& pLingDb)
{
  SemanticLanguageEnum language = linguistics::getLanguage(pText, pLingDb);
  TextProcessingContext inTextProc(SemanticAgentGrounding::currentUser,
                                   SemanticAgentGrounding::me,
                                   language);
  auto semExp = converter::textToSemExp(pText, inTextProc, pLingDb);
  memoryOperation::resolveAgentAccordingToTheContext(semExp, pSemanticMemory, pLingDb);
  std::vector<std::unique_ptr<GroundedExpression>> answers;
  memoryOperation::find(answers, *semExp, pSemanticMemory.memBloc, pLingDb);

  std::vector<std::string> res(answers.size());
  std::size_t i = 0;
  for (auto& currAnswer : answers)
  {
    converter::semExpToText(res[i++], std::move(currAnswer),
        TextProcessingContext(SemanticAgentGrounding::me,
                              SemanticAgentGrounding::currentUser,
                              language),
        false, pSemanticMemory, pLingDb, nullptr);
  }
  return res;
}
}



TEST_F(SemanticReasonerGTests, operator_find_basic)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;
  operator_inform("Chocolate is a food", semMem, lingDb);
  operator_inform("Paul likes chocolate", semMem, lingDb);
  operator_inform("Paul likes eggs", semMem, lingDb);
  operator_inform("André a vu Paul", semMem, lingDb);
  operator_inform("André aime le café", semMem, lingDb);

  {
    auto res = operator_find("chocolate", semMem, lingDb);
    ASSERT_EQ(2u, res.size());
    EXPECT_EQ("Chocolate is a food.", res[0]);
    EXPECT_EQ("Paul likes chocolate.", res[1]);
  }

  {
    auto res = operator_find("eggs", semMem, lingDb);
    ASSERT_EQ(1u, res.size());
    EXPECT_EQ("Paul likes eggs.", res[0]);
  }

  {
    auto res = operator_find("Paul", semMem, lingDb);
    ASSERT_EQ(3u, res.size());
    EXPECT_EQ("Paul likes chocolate.", res[0]);
    EXPECT_EQ("Paul likes eggs.", res[1]);
    EXPECT_EQ("André saw Paul.", res[2]);
  }

  {
    auto res = operator_find("Paul and eggs", semMem, lingDb);
    ASSERT_EQ(1u, res.size());
    EXPECT_EQ("Paul likes eggs.", res[0]);
  }

  {
    auto res = operator_find("Paul and coffee", semMem, lingDb);
    ASSERT_EQ(0u, res.size());
  }

  {
    auto res = operator_find("Paul or coffee", semMem, lingDb);
    ASSERT_EQ(4u, res.size());
    EXPECT_EQ("Paul likes chocolate.", res[0]);
    EXPECT_EQ("Paul likes eggs.", res[1]);
    EXPECT_EQ("André saw Paul.", res[2]);
    EXPECT_EQ("André likes coffee.", res[3]);
  }

  {
    auto res = operator_find("André and coffee", semMem, lingDb);
    ASSERT_EQ(1u, res.size());
    EXPECT_EQ("André likes coffee.", res[0]);
  }

  {
    auto res = operator_find("André or coffee", semMem, lingDb);
    ASSERT_EQ(2u, res.size());
    EXPECT_EQ("André saw Paul.", res[0]);
    EXPECT_EQ("André likes coffee.", res[1]);
  }

  {
    auto res = operator_find("coffee", semMem, lingDb);
    ASSERT_EQ(1u, res.size());
    EXPECT_EQ("André likes coffee.", res[0]);
  }

  {
    auto res = operator_find("boat", semMem, lingDb);
    ASSERT_EQ(0u, res.size());
  }

  // when we inform a new infomration we remove the old conflicting knowledges about it
  // so here we remove the "André aime le café"
  operator_inform("André n'aime pas le café", semMem, lingDb);
  {
    auto res = operator_find("coffee", semMem, lingDb);
    ASSERT_EQ(1u, res.size());
    EXPECT_EQ("André doesn't like coffee.", res[0]);
  }

  {
    auto res = operator_find("to like", semMem, lingDb);
    ASSERT_EQ(7u, res.size());
    EXPECT_EQ("Paul likes chocolate.", res[0]);
    EXPECT_EQ("You told me Paul likes chocolate.", res[1]);
    EXPECT_EQ("Paul likes eggs.", res[2]);
    EXPECT_EQ("You told me Paul likes eggs.", res[3]);
    EXPECT_EQ("You told me André likes coffee.", res[4]);
    EXPECT_EQ("André doesn't like coffee.", res[5]);
    EXPECT_EQ("You told me André doesn't like coffee.", res[6]);
  }
  operator_inform("Paul likes nothing", semMem, lingDb);
  {
    auto res = operator_find("to like", semMem, lingDb);
    ASSERT_EQ(7u, res.size());
    EXPECT_EQ("You told me Paul likes chocolate.", res[0]);
    EXPECT_EQ("You told me Paul likes eggs.", res[1]);
    EXPECT_EQ("You told me André likes coffee.", res[2]);
    EXPECT_EQ("André doesn't like coffee.", res[3]);
    EXPECT_EQ("You told me André doesn't like coffee.", res[4]);
    EXPECT_EQ("Paul doesn't like anything.", res[5]);
    EXPECT_EQ("You told me Paul doesn't like anything.", res[6]);
  }
}
