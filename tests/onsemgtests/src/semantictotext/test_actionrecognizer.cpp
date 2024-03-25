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
    auto frLanguage = SemanticLanguageEnum::FRENCH;
    ActionRecognizer actionRecognizer(frLanguage);

    actionRecognizer.addType("rune", {"rune"});
    actionRecognizer.addEntity("rune", "rune1", {"Virginie"}, lingDb);
    actionRecognizer.addEntity("rune", "rune2", {"plateau"}, lingDb);
    actionRecognizer.addPredicate("is_pressed", {"[r:rune] est pressé", "[r:rune] est cliqué"}, lingDb);
    actionRecognizer.addAction("add", {"ajoute [number]"}, lingDb);
    actionRecognizer.addAction("go_to_rune", {"va à [r:rune]"}, lingDb);

    EXPECT_EQ("{\"action\": \"add(number=1)\"}",
              _recognize("Ajoute un", actionRecognizer, lingDb, frLanguage));

    EXPECT_EQ("{\"action\": \"add(number=1)\", "
              "\"condition\": \"is_pressed(r=rune2)\"}",
              _recognize("Quand la rune du plateau est pressée, ajoute un", actionRecognizer, lingDb, frLanguage));

    EXPECT_EQ("{\"action\": \"go_to_rune(r=rune1)\", "
              "\"condition\": \"is_pressed(r=rune1)\"}",
              _recognize("quand la rune Virginie est pressée, va à la rune Virginie", actionRecognizer, lingDb, frLanguage));

    EXPECT_EQ("{\"condition\": \"is_pressed(r=rune1)\"}",
              _recognize("si la rune Virginie est cliquée", actionRecognizer, lingDb, frLanguage));

    EXPECT_EQ("{\"condition\": \"is_pressed(r=rune1)\"}",
              _recognize("quand la rune Virginie est cliquée", actionRecognizer, lingDb, frLanguage));

    EXPECT_EQ("{\"condition\": \"is_pressed(r=rune1)\"}",
              _recognize("à chaque fois que la rune Virginie est cliquée", actionRecognizer, lingDb, frLanguage));
}


TEST_F(SemanticReasonerGTests, test_actionRecognizer_en) {
    const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
    auto enLanguage = SemanticLanguageEnum::ENGLISH;
    ActionRecognizer actionRecognizer(enLanguage);

    actionRecognizer.addPredicate("is_pressed", {"[r] is pressed", "[r] is clicked"}, lingDb);
    actionRecognizer.addAction("add", {"add [number]"}, lingDb);
    actionRecognizer.addAction("go_to", {"go to [location]"}, lingDb);

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
