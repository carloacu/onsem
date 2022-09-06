#include <gtest/gtest.h>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semantictimegrounding.hpp>


using namespace onsem;


TEST(SemanticBase, test_durationSubstraction)
{
  {
    SemanticDuration durationGrd1;
    durationGrd1.sign = SemanticDurationSign::POSITIVE;
    durationGrd1.timeInfos[SemanticTimeUnity::MINUTE] = 4;
    durationGrd1.timeInfos[SemanticTimeUnity::SECOND] = 4;
    SemanticDuration durationGrd2;
    durationGrd2.sign = SemanticDurationSign::POSITIVE;
    durationGrd2.timeInfos[SemanticTimeUnity::MINUTE] = 3;
    durationGrd2.timeInfos[SemanticTimeUnity::SECOND] = 2;
    SemanticDuration durationGrd3;
    durationGrd3.sign = SemanticDurationSign::POSITIVE;
    durationGrd3.timeInfos[SemanticTimeUnity::MINUTE] = 1;
    durationGrd3.timeInfos[SemanticTimeUnity::SECOND] = 2;
    EXPECT_EQ(durationGrd3,  durationGrd1 - durationGrd2);
    EXPECT_EQ(durationGrd1,  durationGrd2 + durationGrd3);
  }

  {
    SemanticDuration durationGrd1;
    durationGrd1.sign = SemanticDurationSign::POSITIVE;
    durationGrd1.timeInfos[SemanticTimeUnity::MINUTE] = 4;
    durationGrd1.timeInfos[SemanticTimeUnity::SECOND] = 4;
    SemanticDuration durationGrd2;
    durationGrd2.sign = SemanticDurationSign::POSITIVE;
    durationGrd2.timeInfos[SemanticTimeUnity::MINUTE] = 3;
    durationGrd2.timeInfos[SemanticTimeUnity::SECOND] = 15;
    SemanticDuration durationGrd3;
    durationGrd3.sign = SemanticDurationSign::POSITIVE;
    durationGrd3.timeInfos[SemanticTimeUnity::SECOND] = 49;
    EXPECT_EQ(durationGrd3,  durationGrd1 - durationGrd2);
    EXPECT_EQ(durationGrd1,  durationGrd2 + durationGrd3);
    durationGrd3.sign = SemanticDurationSign::NEGATIVE;
    EXPECT_EQ(durationGrd3,  durationGrd2 - durationGrd1);
    EXPECT_EQ(durationGrd2,  durationGrd1 + durationGrd3);
  }
}
