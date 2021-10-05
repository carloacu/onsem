#include <gtest/gtest.h>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semantictextgrounding.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/groundedexpression.hpp>
#include <onsem/texttosemantic/dbtype/semanticgroundings.hpp>
#include <onsem/semantictotext/semanticmemory/expressionhandleinmemory.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemorybloc.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemory.hpp>
#include "../semanticreasonergtests.hpp"

using namespace onsem;

/// A small tool to track how boost signals are emitted.
struct MyStdSignalSpy
{
  template<typename... Args>
  MyStdSignalSpy(mystd::observable::ObservableUnsafe<void(Args...)>& signal)
    : count(0u)
  {
    signal.connectUnsafe([this](Args...){ ++count; });
  }

  unsigned count;
};

UniqueSemanticExpression makeSemExpFromText(const std::string& text)
{
  return mystd::make_unique<GroundedExpression>(mystd::make_unique<SemanticTextGrounding>(text));
}


TEST(SemanticMemory, defaultConstructedBlockIsEmpty)
{
  SemanticMemoryBlock block;
  ASSERT_TRUE(block.empty());
}

TEST_F(SemanticReasonerGTests, addKnowledgePreservesKnowledgeEmitsSignal)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;
  MyStdSignalSpy addedSpy{semMem.memBloc.semExpAdded};
  MyStdSignalSpy removedSpy{semMem.memBloc.expressionRemoved};

  std::string text = "grand-mère sait faire un bon café";
  auto semExp = makeSemExpFromText(text);
  auto expHandleInMem = semMem.memBloc.addRootSemExp(semExp->clone(), lingDb);
  ASSERT_EQ(*semExp, *expHandleInMem->semExp);

  EXPECT_EQ(1u, addedSpy.count);
  EXPECT_EQ(0u, removedSpy.count);
  EXPECT_FALSE(semMem.memBloc.empty());
}

TEST_F(SemanticReasonerGTests, removeKnowledgeFromInsertedOneEmitsSignal)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;
  std::string text = "grand-mère sait faire un bon café";
  auto semExp = makeSemExpFromText(text);
  auto expHandleInMem = semMem.memBloc.addRootSemExp(std::move(semExp), lingDb);

  MyStdSignalSpy addedSpy{semMem.memBloc.semExpAdded};
  MyStdSignalSpy removedSpy{semMem.memBloc.expressionRemoved};
  semMem.memBloc.removeExpression(*expHandleInMem, lingDb, nullptr);

  EXPECT_EQ(0u, addedSpy.count);
  EXPECT_EQ(1u, removedSpy.count);
  EXPECT_TRUE(semMem.memBloc.empty());
}


TEST_F(SemanticReasonerGTests, copyKnowledges)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  auto getGrdExp = [](const std::string& pText)
  {
    return mystd::make_unique<GroundedExpression>
        (mystd::make_unique<SemanticTextGrounding>(pText));
  };

  UniqueSemanticExpression semExp1 = getGrdExp("knowledge1");
  UniqueSemanticExpression semExp2 = getGrdExp("knowledge2");

  // copyKnowledges of a memory block
  {
    SemanticMemory semMem;
    std::list<UniqueSemanticExpression> copiedSemExps;
    semMem.memBloc.copySemExps(copiedSemExps);
    EXPECT_EQ(0u, copiedSemExps.size());

    semMem.memBloc.addRootSemExp(semExp1->clone(), lingDb);
    semMem.memBloc.copySemExps(copiedSemExps);
    EXPECT_EQ(1u, copiedSemExps.size());
    EXPECT_EQ(*semExp1, *copiedSemExps.front());
    EXPECT_NE(*semExp2, *copiedSemExps.front());
  }

  // copyKnowledges of a user memory
  {
    SemanticMemory semMem;

    std::list<UniqueSemanticExpression> copiedSemExps;
    semMem.copySemExps(copiedSemExps);
    EXPECT_EQ(0u, copiedSemExps.size());

    semMem.memBloc.addRootSemExp(semExp1->clone(), lingDb);
    semMem.memBloc.addRootSemExp(semExp2->clone(), lingDb);
    copiedSemExps.clear();
    semMem.copySemExps(copiedSemExps);
    EXPECT_EQ(2u, copiedSemExps.size());
    auto itCpKnowledge = copiedSemExps.begin();
    EXPECT_EQ(*semExp1, **itCpKnowledge);
    ++itCpKnowledge;
    EXPECT_EQ(*semExp2, **itCpKnowledge);
    EXPECT_NE(*semExp1, **itCpKnowledge);
  }
}


TEST_F(SemanticReasonerGTests, memoryPrunning)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;
  semMem.memBloc.maxNbOfExpressionsInAMemoryBlock = 2;
  auto semExp1 = makeSemExpFromText("1");
  auto keepSemExp1InMemory = semMem.memBloc.addRootSemExp(semExp1->clone(), lingDb);
  ASSERT_EQ(1u, semMem.memBloc.nbOfKnowledges());

  auto semExp2 = makeSemExpFromText("2");
  semMem.memBloc.addRootSemExp(semExp2->clone(), lingDb);
  ASSERT_EQ(2u, semMem.memBloc.nbOfKnowledges());
  {
    std::list<UniqueSemanticExpression> copiedKnowledges;
    semMem.memBloc.copySemExps(copiedKnowledges);
    ASSERT_EQ(2u, copiedKnowledges.size());
    auto itCpKnowledge = copiedKnowledges.begin();
    EXPECT_EQ(semExp1, *itCpKnowledge);
    ++itCpKnowledge;
    EXPECT_EQ(*semExp2, **itCpKnowledge);
  }

  auto semExp3 = makeSemExpFromText("3");
  auto keepSemExp3InMemory = semMem.memBloc.addRootSemExp(semExp3->clone(), lingDb);
  ASSERT_EQ(2u, semMem.memBloc.nbOfKnowledges());
  {
    std::list<UniqueSemanticExpression> copiedKnowledges;
    semMem.memBloc.copySemExps(copiedKnowledges);
    ASSERT_EQ(2u, copiedKnowledges.size());
    auto itCpKnowledge = copiedKnowledges.begin();
    EXPECT_EQ(*semExp1, **itCpKnowledge);
    ++itCpKnowledge;
    EXPECT_EQ(*semExp3, **itCpKnowledge);
  }

  auto semExp4 = makeSemExpFromText("4");
  auto keepSemExp4InMemory = semMem.memBloc.addRootSemExp(semExp4->clone(), lingDb);
  ASSERT_EQ(3u, semMem.memBloc.nbOfKnowledges());
  {
    std::list<UniqueSemanticExpression> copiedKnowledges;
    semMem.memBloc.copySemExps(copiedKnowledges);
    ASSERT_EQ(3u, copiedKnowledges.size());
    auto itCpKnowledge = copiedKnowledges.begin();
    EXPECT_EQ(*semExp1, **itCpKnowledge);
    ++itCpKnowledge;
    EXPECT_EQ(*semExp3, **itCpKnowledge);
    ++itCpKnowledge;
    EXPECT_EQ(*semExp4, **itCpKnowledge);
  }

  keepSemExp1InMemory.reset();
  keepSemExp4InMemory.reset();

  auto semExp5 = makeSemExpFromText("5");
  auto keepSemExp5InMemory = semMem.memBloc.addRootSemExp(semExp5->clone(), lingDb);
  ASSERT_EQ(2u, semMem.memBloc.nbOfKnowledges());
  {
    std::list<UniqueSemanticExpression> copiedKnowledges;
    semMem.memBloc.copySemExps(copiedKnowledges);
    ASSERT_EQ(2u, copiedKnowledges.size());
    auto itCpKnowledge = copiedKnowledges.begin();
    EXPECT_EQ(*semExp3, **itCpKnowledge);
    ++itCpKnowledge;
    EXPECT_EQ(*semExp5, **itCpKnowledge);
  }

  auto semExp6 = makeSemExpFromText("6");
  semMem.memBloc.addRootSemExp(semExp6->clone(), lingDb);
  ASSERT_EQ(3u, semMem.memBloc.nbOfKnowledges());
  {
    std::list<UniqueSemanticExpression> copiedKnowledges;
    semMem.memBloc.copySemExps(copiedKnowledges);
    ASSERT_EQ(3u, copiedKnowledges.size());
    auto itCpKnowledge = copiedKnowledges.begin();
    EXPECT_EQ(*semExp3, **itCpKnowledge);
    ++itCpKnowledge;
    EXPECT_EQ(*semExp5, **itCpKnowledge);
    ++itCpKnowledge;
    EXPECT_EQ(*semExp6, **itCpKnowledge);
  }

  keepSemExp5InMemory.reset();

  auto semExp7 = makeSemExpFromText("7");
  semMem.memBloc.addRootSemExp(semExp7->clone(), lingDb);
  ASSERT_EQ(2u, semMem.memBloc.nbOfKnowledges());
  {
    std::list<UniqueSemanticExpression> copiedKnowledges;
    semMem.memBloc.copySemExps(copiedKnowledges);
    ASSERT_EQ(2u, copiedKnowledges.size());
    auto itCpKnowledge = copiedKnowledges.begin();
    EXPECT_EQ(*semExp3, **itCpKnowledge);
    ++itCpKnowledge;
    EXPECT_EQ(*semExp7, **itCpKnowledge);
  }

  keepSemExp3InMemory.reset();
}

