#include "../semanticreasonergtests.hpp"
#include <onsem/texttosemantic/languagedetector.hpp>
#include <onsem/common/enum/semanticsourceenum.hpp>
#include <onsem/semantictotext/actionrecognizer.hpp>
#include <onsem/semantictotext/semanticconverter.hpp>

using namespace onsem;


namespace {

std::string _recognize(const std::string& pText,
                       ActionRecognizer& pActionRecognizer,
                       const linguistics::LinguisticDatabase& pLingDb,
                       SemanticLanguageEnum pTextLanguage) {
    SemanticLanguageEnum textLanguage =
            pTextLanguage == SemanticLanguageEnum::UNKNOWN ? linguistics::getLanguage(pText, pLingDb) : pTextLanguage;
    TextProcessingContext inContext(SemanticAgentGrounding::currentUser, SemanticAgentGrounding::me, textLanguage);
    auto semExp = converter::textToContextualSemExp(pText, inContext, SemanticSourceEnum::UNKNOWN, pLingDb);
    auto actionRecognizedOpt = pActionRecognizer.recognize(std::move(semExp), pLingDb);
    if (actionRecognizedOpt)
        return actionRecognizedOpt->toJson();
    return "";
}

}


TEST_F(SemanticReasonerGTests, test_actionRecognizer) {
    const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
    ActionRecognizer actionRecognizer;
    auto frLanguage = SemanticLanguageEnum::FRENCH;

    actionRecognizer.addPredicate("is_pressed", {"[r] est pressé", "[r] est cliqué"}, lingDb, frLanguage);
    actionRecognizer.addAction("add", {"ajoute [number]"}, lingDb, frLanguage);
    actionRecognizer.addAction("go_to", {"va à [location]"}, lingDb, frLanguage);

    EXPECT_EQ("{\"action\": \"add(number=1)\"}",
              _recognize("Ajoute un", actionRecognizer, lingDb, frLanguage));

    EXPECT_EQ("{\"action\": \"add(number=1)\", "
              "\"condition\": \"is_pressed(r=La rune du plateau)\"}",
              _recognize("Quand la rune du plateau est pressée, ajoute un", actionRecognizer, lingDb, frLanguage));

    EXPECT_EQ("{\"action\": \"go_to(location=La rune Alex)\", "
              "\"condition\": \"is_pressed(r=La rune Alex)\"}",
              _recognize("quand la rune Alex est pressée, va à la rune Alex", actionRecognizer, lingDb, frLanguage));
}
