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


TEST_F(SemanticReasonerGTests, test_actionRecognizer_fr) {
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

    EXPECT_EQ("{\"action\": \"go_to(location=La rune Virginie)\", "
              "\"condition\": \"is_pressed(r=La rune Virginie)\"}",
              _recognize("quand la rune Virginie est pressée, va à la rune Virginie", actionRecognizer, lingDb, frLanguage));

    EXPECT_EQ("{\"condition\": \"is_pressed(r=La rune Virginie)\"}",
              _recognize("si la rune Virginie est cliquée", actionRecognizer, lingDb, frLanguage));

    EXPECT_EQ("{\"condition\": \"is_pressed(r=La rune Virginie)\"}",
              _recognize("quand la rune Virginie est cliquée", actionRecognizer, lingDb, frLanguage));

    EXPECT_EQ("{\"condition\": \"is_pressed(r=La rune Virginie)\"}",
              _recognize("à chaque fois que la rune Virginie est cliquée", actionRecognizer, lingDb, frLanguage));
}


TEST_F(SemanticReasonerGTests, test_actionRecognizer_en) {
    const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
    ActionRecognizer actionRecognizer;
    auto enLanguage = SemanticLanguageEnum::ENGLISH;

    actionRecognizer.addPredicate("is_pressed", {"[r] is pressed", "[r] is clicked"}, lingDb, enLanguage);
    actionRecognizer.addAction("add", {"add [number]"}, lingDb, enLanguage);
    actionRecognizer.addAction("go_to", {"go to [location]"}, lingDb, enLanguage);

    EXPECT_EQ("{\"action\": \"go_to(location=The Virginie rune)\"}",
              _recognize("go to  the Virginie rune", actionRecognizer, lingDb, enLanguage));

    EXPECT_EQ("{\"action\": \"go_to(location=The Virginie rune)\", "
              "\"condition\": \"is_pressed(r=The Virginie rune)\"}",
              _recognize("if the Virginie rune is clicked go to  the Virginie rune", actionRecognizer, lingDb, enLanguage));

    EXPECT_EQ("{\"condition\": \"is_pressed(r=The Virginie rune)\"}",
              _recognize("if the Virginie rune is clicked", actionRecognizer, lingDb, enLanguage));

    EXPECT_EQ("{\"condition\": \"is_pressed(r=The Virginie rune)\"}",
              _recognize("when the Virginie rune is clicked", actionRecognizer, lingDb, enLanguage));

    EXPECT_EQ("{\"condition\": \"is_pressed(r=The Virginie rune)\"}",
              _recognize("whenever the Virginie rune is clicked", actionRecognizer, lingDb, enLanguage));
}
