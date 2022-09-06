#include <gtest/gtest.h>
#include "../semanticreasonergtests.hpp"

using namespace onsem;


TEST_F(SemanticReasonerGTests, conceptSet_conceptToChildConcepts)
{
  const ConceptSet& conceptSet = lingDbPtr->conceptSet;

  {
    std::vector<std::string> concepts;
    conceptSet.conceptToChildConcepts(concepts, "age");
    ASSERT_EQ(1u, concepts.size());
    EXPECT_EQ("age_*", concepts[0]);
  }
  {
    std::vector<std::string> concepts;
    conceptSet.conceptToChildConcepts(concepts, "resource");
    ASSERT_EQ(1u, concepts.size());
    EXPECT_EQ("resource_*", concepts[0]);
  }
  {
    std::vector<std::string> concepts;
    conceptSet.conceptToChildConcepts(concepts, "duration");
    ASSERT_EQ(12u, concepts.size());
    EXPECT_EQ("duration_hour", concepts[0]);
    EXPECT_EQ("duration_hour_*", concepts[1]);
    EXPECT_EQ("duration_millisecond", concepts[2]);
    EXPECT_EQ("duration_millisecond_*", concepts[3]);
    EXPECT_EQ("duration_minute", concepts[4]);
    EXPECT_EQ("duration_minute_*", concepts[5]);
    EXPECT_EQ("duration_relative_delayedStart", concepts[6]);
    EXPECT_EQ("duration_relative_delayedStart_*", concepts[7]);
    EXPECT_EQ("duration_relative_until", concepts[8]);
    EXPECT_EQ("duration_relative_until_*", concepts[9]);
    EXPECT_EQ("duration_second", concepts[10]);
    EXPECT_EQ("duration_second_*", concepts[11]);
  }
  {
    std::vector<std::string> concepts;
    conceptSet.conceptToChildConcepts(concepts, "unless");
    ASSERT_EQ(1u, concepts.size());
    EXPECT_EQ("unless_*", concepts[0]);
  }
  {
    std::vector<std::string> concepts;
    conceptSet.conceptToChildConcepts(concepts, "unlea");
    EXPECT_TRUE(concepts.empty());
  }
}


TEST_F(SemanticReasonerGTests, conceptSet_conceptToParentConcepts)
{
  {
    std::vector<std::string> concepts;
    ConceptSet::conceptToParentConcepts(concepts, "cpt");
    EXPECT_EQ(0u, concepts.size());
  }

  {
    std::vector<std::string> concepts;
    ConceptSet::conceptToParentConcepts(concepts, "cpt_subCpt");
    ASSERT_EQ(1u, concepts.size());
    EXPECT_EQ("cpt", concepts[0]);
  }

  {
    std::vector<std::string> concepts;
    ConceptSet::conceptToParentConcepts(concepts, "cpt_subCpt_subSubCpt");
    ASSERT_EQ(2u, concepts.size());
    EXPECT_EQ("cpt", concepts[0]);
    EXPECT_EQ("cpt_subCpt", concepts[1]);
  }
}
