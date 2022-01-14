#include <gtest/gtest.h>
#include <onsem/semantictotext/bnfparser.hpp>
#include "../semanticreasonergtests.hpp"

using namespace onsem;


TEST_F(SemanticReasonerGTests, bnfparser)
{

  {
    std::string inputText = "je suis content";
    auto res = flattenBnfRegex(inputText);
    ASSERT_EQ(1, res.size());
    EXPECT_EQ(inputText, res[0]);
  }

  {
    std::string inputText = "je suis ( content";
    auto res = flattenBnfRegex(inputText);
    ASSERT_EQ(1, res.size());
    EXPECT_EQ("je suis  content", res[0]);
  }

  {
    std::string inputText = "je suis ( content )";
    auto res = flattenBnfRegex(inputText);
    ASSERT_EQ(1, res.size());
    EXPECT_EQ("je suis  content ", res[0]);
  }

  {
    std::string inputText = "je suis ( content | heureux )";
    auto res = flattenBnfRegex(inputText);
    ASSERT_EQ(2, res.size());
    EXPECT_EQ("je suis  content ", res[0]);
    EXPECT_EQ("je suis  heureux ", res[1]);
  }

  {
    std::string inputText = "Raconte une [autre] blague";
    auto res = flattenBnfRegex(inputText);
    ASSERT_EQ(2, res.size());
    EXPECT_EQ("Raconte une  blague", res[0]);
    EXPECT_EQ("Raconte une autre blague", res[1]);
  }
}

