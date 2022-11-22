#include <gtest/gtest.h>
#include <onsem/semantictotext/semanticconverter.hpp>
#include <onsem/semantictotext/semexpoperators.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticstatementgrounding.hpp>
#include <onsem/texttosemantic/tool/semexpmodifier.hpp>
#include "../semanticreasonergtests.hpp"

using namespace onsem;

struct SemanticModifierTestWithRequestType:
    public SemanticReasonerGTests,
    public testing::WithParamInterface<SemanticRequestType>
{};



TEST_F(SemanticReasonerGTests, listGroundedExpressions)
{
  auto semExp = textToSemExp(
        "rajouter une fonctionnalité et rajouter un test automatique",
        *lingDbPtr, SemanticLanguageEnum::FRENCH);
  auto grdExps = SemExpModifier::listTopGroundedExpressionsPtr(*semExp);
  ASSERT_EQ(2u, grdExps.size());
}

TEST_P(SemanticModifierTestWithRequestType, addRequestToGroundedExpression)
{
  auto semExp = textToSemExp(
        "grand-mère sait faire un bon café",
        *lingDbPtr, SemanticLanguageEnum::FRENCH);
  auto* grdExpPtr = semExp->getGrdExpPtr_SkipWrapperPtrs();
  ASSERT_NE(nullptr, grdExpPtr);
  auto& grdExp = *grdExpPtr;
  auto& grounding = grdExp.grounding();
  auto& stmtGrd = grounding.getStatementGrounding();
  ASSERT_TRUE(stmtGrd.requests.empty());

  SemExpModifier::addRequest(grdExp, GetParam());

  ASSERT_EQ(1u, stmtGrd.requests.types.size());
  ASSERT_TRUE(stmtGrd.requests.has(GetParam()));
}

TEST_P(SemanticModifierTestWithRequestType, addRequestToSemanticExpression)
{
  auto semExp = textToSemExp(
        "grand-mère sait faire un bon café",
        *lingDbPtr, SemanticLanguageEnum::FRENCH);
  auto* grdExpPtr = semExp->getGrdExpPtr_SkipWrapperPtrs();
  ASSERT_NE(nullptr, grdExpPtr);
  auto& grdExp = *grdExpPtr;
  auto& grounding = grdExp.grounding();
  auto& stmtGrd = grounding.getStatementGrounding();
  ASSERT_TRUE(stmtGrd.requests.empty());

  SemExpModifier::addRequest(*semExp, GetParam());
  ASSERT_EQ(1u, stmtGrd.requests.types.size());
  ASSERT_TRUE(stmtGrd.requests.has(GetParam()));
}

INSTANTIATE_TEST_CASE_P(
    SemanticRequestTypeBased, SemanticModifierTestWithRequestType,
    ::testing::Values(
      SemanticRequestType::ACTION, SemanticRequestType::CAUSE, SemanticRequestType::YESORNO,
      SemanticRequestType::CHOICE, SemanticRequestType::LOCATION, SemanticRequestType::MANNER,
      SemanticRequestType::NOTHING, SemanticRequestType::OBJECT, SemanticRequestType::PURPOSE,
      SemanticRequestType::QUANTITY, SemanticRequestType::SUBJECT, SemanticRequestType::TIME,
      SemanticRequestType::VERB));
