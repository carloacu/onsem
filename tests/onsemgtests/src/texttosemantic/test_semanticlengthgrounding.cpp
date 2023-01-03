#include "../semanticreasonergtests.hpp"
#include <gtest/gtest.h>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticlengthgrounding.hpp>

using namespace onsem;


std::string _printLength(const SemanticLength& pLength)
{
  std::list<std::string> elts;
  pLength.printLength(elts, "");
  std::string res;
  for (const auto& currElt : elts)
    res += currElt;
  return res;
}


TEST_F(SemanticReasonerGTests, convert_lengths)
{
  SemanticLength semLength;
  semLength.lengthInfos[SemanticLengthUnity::KILOMETER] = 3;

  EXPECT_EQ("(3km)", _printLength(semLength));
  semLength.convertToUnity(SemanticLengthUnity::METER);
  EXPECT_EQ("(3000m)", _printLength(semLength));

  semLength.lengthInfos.clear();
  semLength.lengthInfos[SemanticLengthUnity::CENTIMETER] = 270;
  EXPECT_EQ("(270cm)", _printLength(semLength));
  semLength.convertToUnity(SemanticLengthUnity::METER);
  EXPECT_EQ("(2.7m)", _printLength(semLength));
}

