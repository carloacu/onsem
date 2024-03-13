#include <gtest/gtest.h>
#include <onsem/semantictotext/tool/semexpcomparator.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemory.hpp>
#include <onsem/texttosemantic/tool/semexpmodifier.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/conceptset.hpp>
#include <onsem/tester/reactOnTexts.hpp>
#include "../semanticreasonergtests.hpp"

using namespace onsem;

namespace {

bool _checkPolarity(const std::string& pText1,
                    const std::string& pText2,
                    const linguistics::LinguisticDatabase& pLingDb) {
    auto semExp1 = textToSemExp(pText1, pLingDb);
    auto semExp2 = textToSemExp(pText2, pLingDb);
    auto* grdExp1Ptr = semExp1->getGrdExpPtr_SkipWrapperPtrs();
    assert(grdExp1Ptr != nullptr);
    auto* grdExp2Ptr = semExp2->getGrdExpPtr_SkipWrapperPtrs();
    assert(grdExp2Ptr != nullptr);
    return SemExpComparator::haveSamePolarity(*grdExp1Ptr, *grdExp2Ptr, pLingDb.conceptSet, true);
}

std::string _invertPolarity(const std::string& pText,
                            const SemanticMemory& pSemanticMemory,
                            const linguistics::LinguisticDatabase& pLingDb) {
    auto semExp1 = textToSemExp(pText, pLingDb);
    SemExpModifier::invertPolarity(*semExp1);
    return semExpToText(std::move(semExp1), SemanticLanguageEnum::ENGLISH, pSemanticMemory, pLingDb);
}

}

TEST_F(SemanticReasonerGTests, test_checkPolarity) {
    const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
    EXPECT_FALSE(_checkPolarity("ne me dis rien", "parle", lingDb));
}

TEST_F(SemanticReasonerGTests, test_invertPolarity) {
    const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
    SemanticMemory semanticMemory;

    EXPECT_EQ("I am not sad.", _invertPolarity("you are sad", semanticMemory, lingDb));
    EXPECT_EQ("I am sad.", _invertPolarity("you are not sad", semanticMemory, lingDb));
    EXPECT_EQ("Am I not sad?", _invertPolarity("are you sad", semanticMemory, lingDb));
    EXPECT_EQ("Am I sad?", _invertPolarity("are you not sad", semanticMemory, lingDb));
}
