#include "operator_show.hpp"
#include <gtest/gtest.h>
#include <onsem/texttosemantic/dbtype/linguisticdatabase.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/groundedexpression.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemory.hpp>
#include <onsem/semantictotext/semexpoperators.hpp>
#include <onsem/semantictotext/semanticconverter.hpp>
#include "operator_inform.hpp"
#include "../../semanticreasonergtests.hpp"

using namespace onsem;

namespace onsem
{

std::vector<std::string> operator_show(const std::string& pText,
                                       const SemanticMemory& pSemanticMemory,
                                       const linguistics::LinguisticDatabase& pLingDb)
{
  auto semExp = converter::textToSemExp
      (pText, TextProcessingContext(SemanticAgentGrounding::currentUser,
                                    SemanticAgentGrounding::me,
                                    SemanticLanguageEnum::UNKNOWN),
       pLingDb);
  std::vector<std::unique_ptr<GroundedExpression>> answers;
  memoryOperation::show(answers, *semExp, pSemanticMemory, pLingDb);

  TextProcessingContext outContext(SemanticAgentGrounding::currentUser,
                                   SemanticAgentGrounding::me,
                                   SemanticLanguageEnum::ENGLISH);
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

}


TEST_F(SemanticReasonerGTests, operator_show_basic)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;
  static const std::string talkUrlStr = "https://www.youtube.com/v/3MJJvXGuDag";
  operator_inform("\\" + resourceLabelForTests_url + "=" + talkUrlStr + "\\ is a man talking", semMem, lingDb);

  {
    auto res = operator_show("a man talking", semMem, lingDb);
    ASSERT_EQ(1u, res.size());
    EXPECT_EQ(talkUrlStr, res[0]);
  }

  {
    auto res = operator_show("Show me a man talking", semMem, lingDb);
    ASSERT_EQ(1u, res.size());
    EXPECT_EQ(talkUrlStr, res[0]);
  }

  EXPECT_TRUE(operator_show("a man walking", semMem, lingDb).empty());
  EXPECT_TRUE(operator_show("a girl talking", semMem, lingDb).empty());
  EXPECT_TRUE(operator_show("a woman singing", semMem, lingDb).empty());

  static const std::string singEnUrlStr = "https://www.youtube.com/watch?v=tQmEd_UeeIk";
  static const std::string singFrUrlStr = "https://www.youtube.com/watch?v=6uMS9_E-z4g";
  operator_inform("\\" + resourceLabelForTests_url + "=#en_US#" + singEnUrlStr + "\\ is a woman singing", semMem, lingDb);
  operator_inform("\\" + resourceLabelForTests_url + "=#fr_FR#" + singFrUrlStr + "\\ is a woman singing", semMem, lingDb);
  {
    auto res = operator_show("a woman singing", semMem, lingDb);
    ASSERT_EQ(2u, res.size());
    EXPECT_EQ(singEnUrlStr, res[0]);
    EXPECT_EQ(singFrUrlStr, res[1]);
  }
  {
    auto res = operator_show("une femme chantant", semMem, lingDb);
    ASSERT_EQ(2u, res.size());
    EXPECT_EQ(singEnUrlStr, res[0]);
    EXPECT_EQ(singFrUrlStr, res[1]);
  }
}
