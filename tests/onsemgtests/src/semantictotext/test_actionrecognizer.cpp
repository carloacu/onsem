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

    actionRecognizer.addType("object", {"objet"});
    actionRecognizer.addType("checkpoint", {"checkpoint"});
    actionRecognizer.addEntity("checkpoint", "checkpoint1", {"Virginie"}, lingDb);
    actionRecognizer.addEntity("checkpoint", "checkpoint2", {"plateau"}, lingDb);
    actionRecognizer.addEntity("object", "patate", {"patate"}, lingDb);
    actionRecognizer.addPredicate("clicked", {"[c:checkpoint] est pressé", "[c:checkpoint] est cliqué"}, lingDb);
    std::map<std::string, ActionRecognizer::ParamInfo> whatNbParameter{
        {"nb", ActionRecognizer::ParamInfo("int", {"what"})}};
    actionRecognizer.addAction("add", {"ajouter"}, whatNbParameter, lingDb);
    std::map<std::string, ActionRecognizer::ParamInfo> whereParameter{
        {"loc", ActionRecognizer::ParamInfo("checkpoint", {"where"})}};
    actionRecognizer.addAction("go_to_loc", {"aller"}, whereParameter, lingDb);
    std::map<std::string, ActionRecognizer::ParamInfo> whatObjParameter{
        {"obj", ActionRecognizer::ParamInfo("object", {"what"})}};
    actionRecognizer.addAction("bring", {"apporter"}, whatObjParameter, lingDb);
    actionRecognizer.addAction("arms_down", {"relâcher ses bras", "baisser ses bras"}, {}, lingDb);
    actionRecognizer.addAction("unfreeze", {"se défiger"}, {}, lingDb);

    EXPECT_EQ("{\"action\": \"bring(obj=patate)\"}",
              _recognize("apporte une patate", actionRecognizer, lingDb, frLanguage));

    EXPECT_EQ("{\"action\": \"add(nb=1)\"}",
              _recognize("Ajoute un", actionRecognizer, lingDb, frLanguage));

    EXPECT_EQ("{\"action\": \"add(nb=1)\", "
              "\"condition\": \"clicked(c=checkpoint2)\"}",
              _recognize("Quand le checkpoint du plateau est pressée, ajoute un", actionRecognizer, lingDb, frLanguage));

    EXPECT_EQ("{\"action\": \"go_to_loc(loc=checkpoint1)\", "
              "\"condition\": \"clicked(c=checkpoint1)\"}",
              _recognize("quand le checkpoint Virginie est pressée, va au checkpoint Virginie", actionRecognizer, lingDb, frLanguage));

    EXPECT_EQ("{\"condition\": \"clicked(c=checkpoint1)\"}",
              _recognize("si le checkpoint Virginie est cliquée", actionRecognizer, lingDb, frLanguage));

    EXPECT_EQ("{\"condition\": \"clicked(c=checkpoint1)\"}",
              _recognize("quand le checkpoint Virginie est cliquée", actionRecognizer, lingDb, frLanguage));

    EXPECT_EQ("{\"condition\": \"clicked(c=checkpoint1)\"}",
              _recognize("à chaque fois que le checkpoint Virginie est cliquée", actionRecognizer, lingDb, frLanguage));

    EXPECT_EQ("{\"action\": \"arms_down\"}",
              _recognize("relâche tes bras", actionRecognizer, lingDb, frLanguage));
    EXPECT_EQ("{\"action\": \"arms_down\"}",
              _recognize("j'aimerais que tu baisses tes bras", actionRecognizer, lingDb, frLanguage));
    EXPECT_EQ("{\"action\": \"arms_down\"}",
              _recognize("Veux-tu bien baisser tes bras ?", actionRecognizer, lingDb, frLanguage));
    EXPECT_EQ("{\"action\": \"arms_down\"}",
              _recognize("Voudrais-tu bien baisser tes bras ?", actionRecognizer, lingDb, frLanguage));
    EXPECT_EQ("{\"action\": \"arms_down\"}",
              _recognize("baisser tes bras", actionRecognizer, lingDb, frLanguage));
    EXPECT_EQ("{\"action\": \"arms_down\"}",
              _recognize("tu dois baisser tes bras", actionRecognizer, lingDb, frLanguage));
    EXPECT_EQ("{\"action\": \"arms_down\"}",
              _recognize("tu aurais l'amabilité de baisser tes bras", actionRecognizer, lingDb, frLanguage));
    EXPECT_EQ("{\"action\": \"arms_down\"}",
              _recognize("aurais-tu la gentillesse de baisser tes bras", actionRecognizer, lingDb, frLanguage));
    EXPECT_EQ("{\"action\": \"unfreeze\"}",
              _recognize("défige-toi", actionRecognizer, lingDb, frLanguage));
}


TEST_F(SemanticReasonerGTests, test_actionRecognizer_en) {
    const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
    auto enLanguage = SemanticLanguageEnum::ENGLISH;
    ActionRecognizer actionRecognizer(enLanguage);

    actionRecognizer.addEntity("checkpoint", "checkpoint1", {"Virginie"}, lingDb);
    actionRecognizer.addEntity("checkpoint", "checkpoint2", {"tray"}, lingDb);
    actionRecognizer.addPredicate("clicked", {"[c:checkpoint] is pressed", "[c:checkpoint] is clicked"}, lingDb);
    std::map<std::string, ActionRecognizer::ParamInfo> whatNbParameter{
        {"nb", ActionRecognizer::ParamInfo("int", {"quoi"})}};
    actionRecognizer.addAction("add", {"add"}, whatNbParameter, lingDb);
    std::map<std::string, ActionRecognizer::ParamInfo> whereParameter{
        {"loc", ActionRecognizer::ParamInfo("checkpoint", {"where"})}};
    actionRecognizer.addAction("go_to_loc", {"go"}, whereParameter, lingDb);
    actionRecognizer.addAction("arms_down", {"relax his arms",
                                             "drop his arms",
                                             "lower his arms"}, {}, lingDb);

    EXPECT_EQ("{\"action\": \"go_to_loc(loc=The Virginie checkpoint)\"}",
              _recognize("go to  the Virginie checkpoint", actionRecognizer, lingDb, enLanguage));

    EXPECT_EQ("{\"action\": \"go_to_loc(loc=The Virginie checkpoint)\", "
              "\"condition\": \"clicked(c=The Virginie checkpoint)\"}",
              _recognize("if the Virginie checkpoint is clicked go to  the Virginie checkpoint", actionRecognizer, lingDb, enLanguage));

    EXPECT_EQ("{\"condition\": \"clicked(c=The Virginie checkpoint)\"}",
              _recognize("if the Virginie checkpoint is clicked", actionRecognizer, lingDb, enLanguage));

    EXPECT_EQ("{\"condition\": \"clicked(c=The Virginie checkpoint)\"}",
              _recognize("when the Virginie checkpoint is clicked", actionRecognizer, lingDb, enLanguage));

    EXPECT_EQ("{\"condition\": \"clicked(c=The Virginie checkpoint)\"}",
              _recognize("whenever the Virginie checkpoint is clicked", actionRecognizer, lingDb, enLanguage));

    EXPECT_EQ("{\"action\": \"arms_down\"}",
              _recognize("relax your arms", actionRecognizer, lingDb, enLanguage));
    EXPECT_EQ("{\"action\": \"arms_down\"}",
              _recognize("Do you want to lower your arms?", actionRecognizer, lingDb, enLanguage));
    EXPECT_EQ("{\"action\": \"arms_down\"}",
              _recognize("Would you like to lower your arms?", actionRecognizer, lingDb, enLanguage));
    EXPECT_EQ("{\"action\": \"arms_down\"}",
              _recognize("Would you be kind enough to relax your arms?", actionRecognizer, lingDb, enLanguage));
    EXPECT_EQ("{\"action\": \"arms_down\"}",
              _recognize("Would you be so kind as to lower your arms?", actionRecognizer, lingDb, enLanguage));
    EXPECT_EQ("{\"action\": \"arms_down\"}",
              _recognize("I would like you to lower your arms", actionRecognizer, lingDb, enLanguage));
    EXPECT_EQ("{\"action\": \"arms_down\"}",
              _recognize("I'd like you to drop your arms", actionRecognizer, lingDb, enLanguage));

}
