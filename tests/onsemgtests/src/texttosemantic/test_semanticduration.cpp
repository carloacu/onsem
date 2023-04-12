#include "../semanticreasonergtests.hpp"
#include <gtest/gtest.h>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticdurationgrounding.hpp>

using namespace onsem;


std::string _printDuration(const SemanticDuration& pLength)
{
  std::list<std::string> elts;
  pLength.printDuration(elts, "");
  std::string res;
  for (const auto& currElt : elts)
    res += currElt;
  return res;
}


TEST_F(SemanticReasonerGTests, convert_durations)
{
  SemanticDuration semDuration;
  semDuration.timeInfos[SemanticTimeUnity::MINUTE] = 3;

  EXPECT_EQ("(3min)", _printDuration(semDuration));
  semDuration.convertToUnity(SemanticTimeUnity::MINUTE);
  EXPECT_EQ("(3min)", _printDuration(semDuration));

  semDuration.timeInfos.clear();
  semDuration.timeInfos[SemanticTimeUnity::SECOND] = 50;
  EXPECT_EQ("(50s)", _printDuration(semDuration));
  semDuration.convertToUnity(SemanticTimeUnity::MINUTE);
  EXPECT_EQ("(0.833333min)", _printDuration(semDuration));

  semDuration.timeInfos.clear();
  semDuration.timeInfos[SemanticTimeUnity::SECOND] = 60;
  EXPECT_EQ("(60s)", _printDuration(semDuration));
  semDuration.convertToUnity(SemanticTimeUnity::MINUTE);
  EXPECT_EQ("(1min)", _printDuration(semDuration));

  semDuration.timeInfos.clear();
  semDuration.timeInfos[SemanticTimeUnity::HOUR] = 2;
  EXPECT_EQ("(2h)", _printDuration(semDuration));
  semDuration.convertToUnity(SemanticTimeUnity::MINUTE);
  EXPECT_EQ("(120min)", _printDuration(semDuration));
}

