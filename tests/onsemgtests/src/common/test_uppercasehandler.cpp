#include <gtest/gtest.h>
#include <onsem/common/utility/uppercasehandler.hpp>
#include "../semanticreasonergtests.hpp"


using namespace onsem;

namespace
{

std::string _lowerCaseText(const std::string& pStr)
{
  auto res = pStr;
  if (lowerCaseText(res))
    EXPECT_NE(pStr, res);
  else
    EXPECT_EQ(pStr, res);
  return res;
}

}



TEST_F(SemanticReasonerGTests, common_uppercaseHandler_lowerCaseText)
{
  EXPECT_EQ("toussaint", _lowerCaseText("toussaint"));
  EXPECT_EQ("toussaint", _lowerCaseText("Toussaint"));
  EXPECT_EQ("youtube", _lowerCaseText("YouTube"));
  EXPECT_EQ("élégance", _lowerCaseText("Élégance"));
  EXPECT_EQ("élégance", _lowerCaseText("ÉLÉGANCE"));
}
