#include "../semanticreasonergtests.hpp"
#include <gtest/gtest.h>
#include <onsem/semantictotext/semanticmemory/semanticmemory.hpp>
#include <onsem/semantictotext/semanticconverter.hpp>

using namespace onsem;

namespace {

std::string _geFutureFrom(const std::string& pInputText,
                          SemanticLanguageEnum pLanguage,
                          const SemanticMemory& pSemMem,
                          const linguistics::LinguisticDatabase& pLingDb) {
    auto semExpFuture = textToSemExp(pInputText, pLingDb, pLanguage);
    auto semExpFutureForm = converter::getFutureIndicativeAssociatedForm(std::move(semExpFuture));
    return semExpToText(std::move(semExpFutureForm), pLanguage, pSemMem, pLingDb);
}

std::string _infinitiveToRequestVariationsStr(const std::string& pInputText,
                                              SemanticLanguageEnum pLanguage,
                                              const SemanticMemory& pSemMem,
                                              const linguistics::LinguisticDatabase& pLingDb) {
    auto semExpInfinitive = textToSemExp(pInputText, pLingDb, pLanguage);
    std::list<UniqueSemanticExpression> outSemExps;
    converter::infinitiveToRequestVariations(
                outSemExps, std::move(semExpInfinitive));
    std::string res;
    for (auto& currElt : outSemExps) {
        if (!res.empty())
            res += " | ";
        res += semExpToTextFromUser(std::move(currElt), pLanguage, pSemMem, pLingDb, true);
    }
    return res;
}

}

TEST_F(SemanticReasonerGTests, operator_converter_getFutureIndicativeAssociatedForm) {
    const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
    SemanticMemory semMem;

    EXPECT_EQ("Je marcherai.", _geFutureFrom("Marcher", SemanticLanguageEnum::FRENCH, semMem, lingDb));
    EXPECT_EQ("Je marcherai.", _geFutureFrom("Il faut marcher", SemanticLanguageEnum::FRENCH, semMem, lingDb));
    EXPECT_EQ("Je marcherai et puis je sauterai.",
              _geFutureFrom("Il faut marcher puis il faut sauter", SemanticLanguageEnum::FRENCH, semMem, lingDb));
}

TEST_F(SemanticReasonerGTests, operator_converter_infinitiveToRequestVariations) {
    const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
    SemanticMemory semMem;

    EXPECT_EQ("Marche ! | Je voudrais que tu marches. | Veux-tu marcher ?",
              _infinitiveToRequestVariationsStr("Marcher", SemanticLanguageEnum::FRENCH, semMem, lingDb));

    EXPECT_EQ("Dis [text](pouet pouet) ! | Je voudrais que tu dises [text](pouet pouet). | Veux-tu dire [text](pouet pouet) ?",
              _infinitiveToRequestVariationsStr("dire [text](pouet pouet)", SemanticLanguageEnum::FRENCH, semMem, lingDb));

    EXPECT_EQ("Avance rapidement vers la droite ! | Je voudrais que tu avances rapidement vers la droite. | Veux-tu avancer rapidement vers la droite ?",
              _infinitiveToRequestVariationsStr("Avancer rapidement vers la droite", SemanticLanguageEnum::FRENCH, semMem, lingDb));

    EXPECT_EQ("Grab the bottle! | I would like you to grab the bottle. | Do you want to grab the bottle?",
              _infinitiveToRequestVariationsStr("To grab the bottle", SemanticLanguageEnum::ENGLISH, semMem, lingDb));
}
