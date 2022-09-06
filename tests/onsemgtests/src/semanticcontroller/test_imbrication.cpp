#include <gtest/gtest.h>
#include <onsem/semantictotext/tool/semexpcomparator.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/groundedexpression.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticconceptualgrounding.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticgenericgrouding.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemory.hpp>
#include "../semanticreasonergtests.hpp"
#include "../util/util.hpp"


using namespace onsem;

namespace
{
ImbricationType _getImbrication(const std::string& pStr1,
                                const std::string& pStr2,
                                const SemanticMemory& pSemanticMemory,
                                const linguistics::LinguisticDatabase& pLingDb,
                                SemanticLanguageEnum pLanguage = SemanticLanguageEnum::UNKNOWN,
                                SemExpComparator::ComparisonErrorReporting* pComparisonErrorReportingPtr = nullptr)
{
  auto uSemExp1 = textToSemExp(pStr1, pLingDb, pLanguage);
  auto uSemExp2 = textToSemExp(pStr2, pLingDb, pLanguage);
  auto& semExp1 = *uSemExp1;
  auto& semExp2 = *uSemExp2;
  auto res = SemExpComparator::getSemExpsImbrications(semExp1, semExp2, pSemanticMemory.memBloc, pLingDb,
                                                      nullptr, pComparisonErrorReportingPtr);
  auto resOtherSide = SemExpComparator::getSemExpsImbrications(semExp2, semExp1, pSemanticMemory.memBloc, pLingDb,
                                                               nullptr);
  auto resOtherSideExpected = SemExpComparator::switchOrderOfEltsImbrication(res);
  EXPECT_EQ(resOtherSideExpected, resOtherSide);
  return res;
}
}


TEST_F(SemanticReasonerGTests, test_imbrication_basic)
{
  SemanticMemory semanticMemory;
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;

  EXPECT_EQ(ImbricationType::EQUALS, _getImbrication("I like banana", "I like banana", semanticMemory, lingDb));
  EXPECT_EQ(ImbricationType::EQUALS, _getImbrication("I don't like banana", "I dislike banana", semanticMemory, lingDb));
  EXPECT_EQ(ImbricationType::EQUALS, _getImbrication("I like banana", "I am fond of banana", semanticMemory, lingDb));
  EXPECT_EQ(ImbricationType::EQUALS, _getImbrication("a man and a woman", "a man and a woman", semanticMemory, lingDb));
  EXPECT_EQ(ImbricationType::EQUALS, _getImbrication("tu es par terre", "tu es sur le sol", semanticMemory, lingDb));
  EXPECT_EQ(ImbricationType::EQUALS, _getImbrication("no", "absolutely not", semanticMemory, lingDb));
  EXPECT_EQ(ImbricationType::EQUALS, _getImbrication("je m'aime", "j'aime moi", semanticMemory, lingDb));
  EXPECT_EQ(ImbricationType::EQUALS, _getImbrication("un chocolat", "le chocolat", semanticMemory, lingDb));
  EXPECT_EQ(ImbricationType::EQUALS, _getImbrication("chocolat", "le chocolat", semanticMemory, lingDb));
  EXPECT_EQ(ImbricationType::EQUALS, _getImbrication("chocolat", "un chocolat", semanticMemory, lingDb));
  EXPECT_EQ(ImbricationType::EQUALS, _getImbrication("le 3 septembre 1986", "le 3 septembre 1986", semanticMemory, lingDb));
  EXPECT_EQ(ImbricationType::EQUALS, _getImbrication("si tu es content dis bonjour", "si tu es content dis bonjour", semanticMemory, lingDb));
  EXPECT_EQ(ImbricationType::EQUALS, _getImbrication("a N5", "a N5 robot", semanticMemory, lingDb, SemanticLanguageEnum::ENGLISH));
  EXPECT_EQ(ImbricationType::EQUALS, _getImbrication("I bought a N5", "I bought a N5 robot", semanticMemory, lingDb));
  EXPECT_EQ(ImbricationType::EQUALS, _getImbrication("I bought N5", "I bought N5 robot", semanticMemory, lingDb));
  EXPECT_EQ(ImbricationType::EQUALS, _getImbrication("Je veux une assurance habitation", "I want a home insurance", semanticMemory, lingDb));
  EXPECT_EQ(ImbricationType::EQUALS, _getImbrication("Je veux une assurance habitation", "Je voudrais une assurance habitation", semanticMemory, lingDb));
  EXPECT_EQ(ImbricationType::EQUALS, _getImbrication("et puis", "Et après", semanticMemory, lingDb));

  EXPECT_EQ(ImbricationType::OPPOSES, _getImbrication("I like banana", "I don't like banana", semanticMemory, lingDb));
  EXPECT_EQ(ImbricationType::OPPOSES, _getImbrication("I like banana", "I dislike banana", semanticMemory, lingDb));

  EXPECT_EQ(ImbricationType::CONTAINS, _getImbrication("everybody", "me", semanticMemory, lingDb));
  EXPECT_EQ(ImbricationType::CONTAINS, _getImbrication("si tout le monde est content dis bonjour", "si tu es content dis bonjour", semanticMemory, lingDb));

  EXPECT_EQ(ImbricationType::ISCONTAINED, _getImbrication("Paul", "everybody", semanticMemory, lingDb));

  EXPECT_EQ(ImbricationType::HYPONYM, _getImbrication("moi", "quelqu'un", semanticMemory, lingDb));
  EXPECT_EQ(ImbricationType::HYPONYM, _getImbrication("I ask walk", "I say walk", semanticMemory, lingDb));
  EXPECT_EQ(ImbricationType::HYPONYM, _getImbrication("je m'aime", "quelqu'un aime quelqu'un", semanticMemory, lingDb));

  EXPECT_EQ(ImbricationType::HYPERNYM, _getImbrication("I say rub your head", "I ask rub your head", semanticMemory, lingDb));
  EXPECT_EQ(ImbricationType::HYPERNYM, _getImbrication("I call \\p_agent=1\\", "I call you", semanticMemory, lingDb));
  EXPECT_EQ(ImbricationType::HYPERNYM, _getImbrication("I call \\p_agent=1\\ by phone", "I call you by phone", semanticMemory, lingDb));
  EXPECT_EQ(ImbricationType::HYPERNYM, _getImbrication("a person", "me", semanticMemory, lingDb));

  EXPECT_EQ(ImbricationType::MORE_DETAILED, _getImbrication("le 3 septembre 1986", "septembre 1986", semanticMemory, lingDb));
  EXPECT_EQ(ImbricationType::MORE_DETAILED, _getImbrication("I look left", "I look", semanticMemory, lingDb));
  EXPECT_EQ(ImbricationType::MORE_DETAILED, _getImbrication("somebody touches your left hand", "your left hand is touched", semanticMemory, lingDb));
  EXPECT_EQ(ImbricationType::MORE_DETAILED, _getImbrication("your right elbow", "your body", semanticMemory, lingDb));
  EXPECT_EQ(ImbricationType::MORE_DETAILED, _getImbrication("si tu es content dis bonjour sinon dis au revoir", "si tu es content dis bonjour", semanticMemory, lingDb));
  EXPECT_EQ(ImbricationType::MORE_DETAILED, _getImbrication("the dance Macarena", "a dance", semanticMemory, lingDb));

  EXPECT_EQ(ImbricationType::LESS_DETAILED, _getImbrication("septembre", "le 3 septembre 1986", semanticMemory, lingDb));
  EXPECT_EQ(ImbricationType::LESS_DETAILED, _getImbrication("Paul ira à Paris", "Paul ira à Paris. Pierre aime les fleurs", semanticMemory, lingDb));
  EXPECT_EQ(ImbricationType::LESS_DETAILED, _getImbrication("Paul ira à Paris", "Paul ira à Paris en janvier", semanticMemory, lingDb));
  EXPECT_EQ(ImbricationType::LESS_DETAILED, _getImbrication("I asked walk", "I said walk yesterday", semanticMemory, lingDb));
  EXPECT_EQ(ImbricationType::LESS_DETAILED, _getImbrication("oui", "Oui, allons-y !", semanticMemory, lingDb));

  EXPECT_EQ(ImbricationType::DIFFERS, _getImbrication("et puis", "et avant", semanticMemory, lingDb));
  EXPECT_EQ(ImbricationType::DIFFERS, _getImbrication("Il est né en 1988", "Il est né le 5 mai 1986", semanticMemory, lingDb));
  EXPECT_EQ(ImbricationType::DIFFERS, _getImbrication("le 3 septembre 1986", "septembre 1987", semanticMemory, lingDb));
  EXPECT_EQ(ImbricationType::DIFFERS, _getImbrication("le 3 septembre 1986", "le 4 septembre 1986", semanticMemory, lingDb));
  EXPECT_EQ(ImbricationType::DIFFERS, _getImbrication("@karim-boudjema", "deux joueurs de tennis Vijay Amritraj", semanticMemory, lingDb));
  EXPECT_EQ(ImbricationType::DIFFERS, _getImbrication("Abderrahim", "un conseiller", semanticMemory, lingDb));
  EXPECT_EQ(ImbricationType::DIFFERS, _getImbrication("j'aime quelqu'un", "quelqu'un m'aime", semanticMemory, lingDb));
  EXPECT_EQ(ImbricationType::DIFFERS, _getImbrication("I say rub your head", "I don't ask rub your head", semanticMemory, lingDb));
  EXPECT_EQ(ImbricationType::DIFFERS, _getImbrication("I look left", "I don't look", semanticMemory, lingDb));
  EXPECT_EQ(ImbricationType::DIFFERS, _getImbrication("I like a person", "I don't like you", semanticMemory, lingDb));
  EXPECT_EQ(ImbricationType::DIFFERS, _getImbrication("I am speaking", "I am smiling", semanticMemory, lingDb));
  EXPECT_EQ(ImbricationType::DIFFERS, _getImbrication("a man and a woman", "a man or a woman", semanticMemory, lingDb));
}


TEST_F(SemanticReasonerGTests, test_imbrication_for_opposite_concepts)
{
  SemanticMemory semanticMemory;
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;

  const std::string goodConcept = "health_good";
  const std::string badConcept = "health_bad";
  EXPECT_TRUE(lingDb.conceptSet.areOppositeConcepts(goodConcept, badConcept));

  EXPECT_EQ(ImbricationType::OPPOSES,
            SemExpComparator::getSemExpsImbrications(
              *mystd::make_unique<GroundedExpression>(mystd::make_unique<SemanticConceptualGrounding>(goodConcept)),
              *mystd::make_unique<GroundedExpression>(mystd::make_unique<SemanticConceptualGrounding>(badConcept)),
              semanticMemory.memBloc, lingDb, nullptr));

  auto cptToGenGrdWithThisConcept = [](const std::string& pConcept)
  {
    auto res = mystd::make_unique<SemanticGenericGrounding>();
    res->concepts.emplace(pConcept, 4);
    return res;
  };
  EXPECT_EQ(ImbricationType::OPPOSES,
            SemExpComparator::getSemExpsImbrications(
              *mystd::make_unique<GroundedExpression>(cptToGenGrdWithThisConcept(goodConcept)),
              *mystd::make_unique<GroundedExpression>(cptToGenGrdWithThisConcept(badConcept)),
              semanticMemory.memBloc, lingDb, nullptr));
}




TEST_F(SemanticReasonerGTests, test_imbrication_errorReporting)
{
  SemanticMemory semanticMemory;
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;

  {
    SemExpComparator::ComparisonErrorReporting comparisonErrorReporting;
    EXPECT_EQ(ImbricationType::DIFFERS, _getImbrication("Paul is happy",
                                                        "I am happy",
                                                        semanticMemory, lingDb, SemanticLanguageEnum::UNKNOWN,
                                                        &comparisonErrorReporting));
    ASSERT_EQ(1, comparisonErrorReporting.childrenThatAreNotEqual.size());
    auto it = comparisonErrorReporting.childrenThatAreNotEqual.begin();
    EXPECT_EQ(GrammaticalType::SUBJECT, it->first);
    ASSERT_EQ(1, it->second.size());
    auto it2 = it->second.begin();
    EXPECT_EQ(ImbricationType::DIFFERS, it2->first);
    EXPECT_EQ(1, it2->second);
  }

  {
    SemExpComparator::ComparisonErrorReporting comparisonErrorReporting;
    EXPECT_EQ(ImbricationType::DIFFERS, _getImbrication("Paul is happy",
                                                        "I am sad",
                                                        semanticMemory, lingDb, SemanticLanguageEnum::UNKNOWN,
                                                        &comparisonErrorReporting));
    ASSERT_EQ(2, comparisonErrorReporting.childrenThatAreNotEqual.size());
    auto it = comparisonErrorReporting.childrenThatAreNotEqual.begin();
    EXPECT_EQ(GrammaticalType::SUBJECT, it->first);
    ++it;
    EXPECT_EQ(GrammaticalType::OBJECT, it->first);
  }

  {
    SemExpComparator::ComparisonErrorReporting comparisonErrorReporting;
    EXPECT_EQ(ImbricationType::MORE_DETAILED, _getImbrication("Does Paul like banana and bear?",
                                                              "Does Paul like banana?",
                                                              semanticMemory, lingDb, SemanticLanguageEnum::UNKNOWN,
                                                              &comparisonErrorReporting));
    ASSERT_EQ(1, comparisonErrorReporting.childrenThatAreNotEqual.size());
    auto it = comparisonErrorReporting.childrenThatAreNotEqual.begin();
    EXPECT_EQ(GrammaticalType::OBJECT, it->first);
  }

  {
    SemExpComparator::ComparisonErrorReporting comparisonErrorReporting;
    EXPECT_EQ(ImbricationType::LESS_DETAILED, _getImbrication("Baisse la température",
                                                              "Baisse encore la température",
                                                              semanticMemory, lingDb, SemanticLanguageEnum::UNKNOWN,
                                                              &comparisonErrorReporting));
    ASSERT_EQ(1, comparisonErrorReporting.childrenThatAreNotEqual.size());
    auto it = comparisonErrorReporting.childrenThatAreNotEqual.begin();
    EXPECT_EQ(GrammaticalType::SPECIFIER, it->first);
    ASSERT_EQ(1, it->second.size());
    auto it2 = it->second.begin();
    EXPECT_EQ(ImbricationType::LESS_DETAILED, it2->first);
    EXPECT_EQ(1, it2->second);
  }

  {
    SemExpComparator::ComparisonErrorReporting comparisonErrorReporting;
    EXPECT_EQ(ImbricationType::LESS_DETAILED, _getImbrication("Baisse la température",
                                                              "Baisse encore la température de la pièce",
                                                              semanticMemory, lingDb, SemanticLanguageEnum::UNKNOWN,
                                                              &comparisonErrorReporting));
    ASSERT_EQ(1, comparisonErrorReporting.childrenThatAreNotEqual.size());
    auto it = comparisonErrorReporting.childrenThatAreNotEqual.begin();
    EXPECT_EQ(GrammaticalType::SPECIFIER, it->first);
    ASSERT_EQ(1, it->second.size());
    auto it2 = it->second.begin();
    EXPECT_EQ(ImbricationType::LESS_DETAILED, it2->first);
    EXPECT_EQ(2, it2->second);
  }

  {
    SemExpComparator::ComparisonErrorReporting comparisonErrorReporting;
    EXPECT_EQ(ImbricationType::MORE_DETAILED, _getImbrication("raconte moi quelque chose de joyeux",
                                                              "Dis quelque chose",
                                                              semanticMemory, lingDb, SemanticLanguageEnum::UNKNOWN,
                                                              &comparisonErrorReporting));
    EXPECT_EQ(2, comparisonErrorReporting.nbOfErrors());
    ASSERT_EQ(2, comparisonErrorReporting.childrenThatAreNotEqual.size());
    auto it = comparisonErrorReporting.childrenThatAreNotEqual.begin();
    EXPECT_EQ(GrammaticalType::RECEIVER, it->first);
    ASSERT_EQ(1, it->second.size());
    auto it2 = it->second.begin();
    EXPECT_EQ(ImbricationType::MORE_DETAILED, it2->first);
    EXPECT_EQ(1, it2->second);
    ++it;
    EXPECT_EQ(GrammaticalType::SPECIFIER, it->first);
    ASSERT_EQ(1, it->second.size());
    auto it3 = it->second.begin();
    EXPECT_EQ(ImbricationType::MORE_DETAILED, it3->first);
    EXPECT_EQ(1, it3->second);
  }

  {
    SemExpComparator::ComparisonErrorReporting comparisonErrorReporting;
    EXPECT_EQ(ImbricationType::LESS_DETAILED, _getImbrication("Raconte une histoire",
                                                              "Raconte une histoire triste",
                                                              semanticMemory, lingDb, SemanticLanguageEnum::UNKNOWN,
                                                              &comparisonErrorReporting));
    ASSERT_EQ(1, comparisonErrorReporting.childrenThatAreNotEqual.size());
    EXPECT_EQ(GrammaticalType::SPECIFIER, comparisonErrorReporting.childrenThatAreNotEqual.begin()->first);
  }

  {
    SemExpComparator::ComparisonErrorReporting comparisonErrorReporting;
    EXPECT_EQ(ImbricationType::DIFFERS, _getImbrication("Raconte une histoire joyeuse",
                                                        "Raconte une histoire triste",
                                                        semanticMemory, lingDb, SemanticLanguageEnum::UNKNOWN,
                                                        &comparisonErrorReporting));
    ASSERT_EQ(1, comparisonErrorReporting.childrenThatAreNotEqual.size());
    EXPECT_EQ(GrammaticalType::SPECIFIER, comparisonErrorReporting.childrenThatAreNotEqual.begin()->first);
  }

  {
    SemExpComparator::ComparisonErrorReporting comparisonErrorReporting;
    EXPECT_EQ(ImbricationType::LESS_DETAILED, _getImbrication("Peux-tu nous raconter une blague",
                                                              "Peux-tu nous raconter une autre blague",
                                                              semanticMemory, lingDb, SemanticLanguageEnum::UNKNOWN,
                                                              &comparisonErrorReporting));
    ASSERT_EQ(1, comparisonErrorReporting.childrenThatAreNotEqual.size());
    EXPECT_EQ(GrammaticalType::OTHER_THAN, comparisonErrorReporting.childrenThatAreNotEqual.begin()->first);
  }
}

