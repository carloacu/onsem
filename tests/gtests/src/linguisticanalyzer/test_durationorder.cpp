#include <gtest/gtest.h>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semantictimegrounding.hpp>

using namespace onsem;


TEST(SemanticBase, test_durationOrder)
{
  {
    SemanticDuration durationGrd1;
    durationGrd1.sign = SemanticDurationSign::NEGATIVE;
    durationGrd1.timeInfos[SemanticTimeUnity::MINUTE] = 6;
    SemanticDuration durationGrd2;
    durationGrd2.sign = SemanticDurationSign::POSITIVE;
    durationGrd2.timeInfos[SemanticTimeUnity::MINUTE] = 6;
    EXPECT_TRUE(durationGrd1 < durationGrd2);
  }

  {
    SemanticDuration durationGrd1;
    durationGrd1.sign = SemanticDurationSign::POSITIVE;
    durationGrd1.timeInfos[SemanticTimeUnity::MINUTE] = 5;
    SemanticDuration durationGrd2;
    durationGrd2.sign = SemanticDurationSign::POSITIVE;
    durationGrd2.timeInfos[SemanticTimeUnity::MINUTE] = 6;
    EXPECT_TRUE(durationGrd1 < durationGrd2);
  }

  {
    SemanticDuration durationGrd1;
    durationGrd1.sign = SemanticDurationSign::NEGATIVE;
    durationGrd1.timeInfos[SemanticTimeUnity::MINUTE] = 6;
    SemanticDuration durationGrd2;
    durationGrd2.sign = SemanticDurationSign::NEGATIVE;
    durationGrd2.timeInfos[SemanticTimeUnity::MINUTE] = 5;
    EXPECT_TRUE(durationGrd1 < durationGrd2);
  }

  {
    SemanticDuration durationGrd1;
    durationGrd1.sign = SemanticDurationSign::POSITIVE;
    durationGrd1.timeInfos[SemanticTimeUnity::MINUTE] = 6;
    SemanticDuration durationGrd2;
    durationGrd2.sign = SemanticDurationSign::POSITIVE;
    durationGrd2.timeInfos[SemanticTimeUnity::MINUTE] = 6;
    durationGrd2.timeInfos[SemanticTimeUnity::SECOND] = 2;
    EXPECT_TRUE(durationGrd1 < durationGrd2);
  }

  {
    SemanticDuration durationGrd1;
    durationGrd1.sign = SemanticDurationSign::POSITIVE;
    durationGrd1.timeInfos[SemanticTimeUnity::MINUTE] = 6;
    durationGrd1.timeInfos[SemanticTimeUnity::SECOND] = 2;
    SemanticDuration durationGrd2;
    durationGrd2.sign = SemanticDurationSign::POSITIVE;
    durationGrd2.timeInfos[SemanticTimeUnity::MINUTE] = 6;
    EXPECT_FALSE(durationGrd1 < durationGrd2);
  }


  {
    SemanticDuration durationGrd1;
    durationGrd1.sign = SemanticDurationSign::POSITIVE;
    durationGrd1.timeInfos[SemanticTimeUnity::HOUR] = 0;
    durationGrd1.timeInfos[SemanticTimeUnity::MINUTE] = 6;
    durationGrd1.timeInfos[SemanticTimeUnity::SECOND] = 2;
    SemanticDuration durationGrd2;
    durationGrd2.sign = SemanticDurationSign::POSITIVE;
    durationGrd2.timeInfos[SemanticTimeUnity::MINUTE] = 7;
    EXPECT_TRUE(durationGrd1 < durationGrd2);
  }

  {
    SemanticDuration durationGrd1;
    durationGrd1.sign = SemanticDurationSign::POSITIVE;
    durationGrd1.timeInfos[SemanticTimeUnity::MINUTE] = 6;
    durationGrd1.timeInfos[SemanticTimeUnity::SECOND] = 2;
    SemanticDuration durationGrd2;
    durationGrd2.sign = SemanticDurationSign::POSITIVE;
    durationGrd2.timeInfos[SemanticTimeUnity::HOUR] = 0;
    durationGrd2.timeInfos[SemanticTimeUnity::MINUTE] = 3;
    EXPECT_FALSE(durationGrd1 < durationGrd2);
  }

  {
    SemanticDuration durationGrd1;
    durationGrd1.sign = SemanticDurationSign::NEGATIVE;
    durationGrd1.timeInfos[SemanticTimeUnity::MINUTE] = 6;
    durationGrd1.timeInfos[SemanticTimeUnity::LESS_THAN_A_MILLISECOND] = 1;
    SemanticDuration durationGrd2;
    durationGrd2.sign = SemanticDurationSign::NEGATIVE;
    durationGrd2.timeInfos[SemanticTimeUnity::MINUTE] = 6;
    EXPECT_TRUE(durationGrd1 < durationGrd2);
    EXPECT_FALSE(durationGrd2 < durationGrd1);
  }

  {
    SemanticDuration durationGrd1;
    durationGrd1.sign = SemanticDurationSign::NEGATIVE;
    durationGrd1.timeInfos[SemanticTimeUnity::MINUTE] = 6;
    SemanticDuration durationGrd2 = durationGrd1;
    EXPECT_FALSE(durationGrd1 < durationGrd2);
  }
}
