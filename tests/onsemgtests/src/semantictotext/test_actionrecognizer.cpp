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
        return actionRecognizedOpt->toJsonWithIntentInStr();
    return "";
}

bool _isObviouslyWrong(const std::string& pIntent,
                       const std::string& pText,
                       const ActionRecognizer& pActionRecognizer,
                       const linguistics::LinguisticDatabase& pLingDb,
                       SemanticLanguageEnum pTextLanguage) {
    SemanticLanguageEnum textLanguage =
            pTextLanguage == SemanticLanguageEnum::UNKNOWN ? linguistics::getLanguage(pText, pLingDb) : pTextLanguage;
    TextProcessingContext inContext(SemanticAgentGrounding::currentUser, SemanticAgentGrounding::me, textLanguage);
    auto semExp = converter::textToContextualSemExp(pText, inContext, SemanticSourceEnum::UNKNOWN, pLingDb);
    return pActionRecognizer.isObviouslyWrong(pIntent, *semExp, pLingDb);
}



}


TEST_F(SemanticReasonerGTests, test_actionRecognizer_fr) {
    const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
    auto frLanguage = SemanticLanguageEnum::FRENCH;
    ActionRecognizer actionRecognizer(frLanguage);

    actionRecognizer.addType("object", {"objet"}, false);
    actionRecognizer.addType("checkpoint", {"checkpoint"}, true);
    actionRecognizer.addEntity("checkpoint", "checkpoint1", {"Virginie"}, lingDb);
    actionRecognizer.addEntity("checkpoint", "checkpoint2", {"plateau"}, lingDb);
    actionRecognizer.addEntity("checkpoint", "checkpoint3", {"Charles"}, lingDb);
    actionRecognizer.addEntity("object", "patate", {"patate"}, lingDb);
    actionRecognizer.addPredicate("clicked", {"[c:checkpoint] est pressé", "[c:checkpoint] est cliqué"}, lingDb);
    actionRecognizer.addPredicate("same_location", {"[self] est proche de [c:checkpoint]", "[self] est pas loin de [c:checkpoint]"}, lingDb);
    actionRecognizer.addPredicate("sees_someone", {"[self] voit quelqu'un"}, lingDb);

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
    actionRecognizer.addAction("what_i_know", {"dire ce que [self] sait"}, {}, lingDb);
    std::map<std::string, ActionRecognizer::ParamInfo> howFarInMetersParameter{
        {"distance", ActionRecognizer::ParamInfo("checkpoint", {"how far in meters"})}};
    actionRecognizer.addAction("move", {"avance"}, howFarInMetersParameter, lingDb);
    std::map<std::string, ActionRecognizer::ParamInfo> howFarInDegreesParameter{
        {"angle", ActionRecognizer::ParamInfo("angle", {"how far in degrees"})}};
    actionRecognizer.addAction("turn_left", {"tourne à gauche"}, howFarInDegreesParameter, lingDb);
    actionRecognizer.addAction("turn_right", {"tourne à droite"}, howFarInDegreesParameter, lingDb);
    actionRecognizer.addAction("low_battery_level_reaction", {"avertir de la batterie faible"}, howFarInDegreesParameter, lingDb);
    actionRecognizer.addAction("salute", {"saluer de la main et de la voix"}, howFarInDegreesParameter, lingDb);
    actionRecognizer.addAction("look_at_somebody", {"regarder un utilisateur"}, howFarInDegreesParameter, lingDb);
    std::map<std::string, ActionRecognizer::ParamInfo> whatParameter{
        {"something", ActionRecognizer::ParamInfo("str", {"what"})}};
    actionRecognizer.addAction("say", {"dire"}, whatParameter, lingDb);
    actionRecognizer.addAction("put", {"poser quelque chose"}, {}, lingDb);
    actionRecognizer.addAction("raise_arms", {"lever les bras"}, {}, lingDb);

    EXPECT_EQ("{\"intent\": \"move\", \"to_run_in_background\": [{\"intent\": \"raise_arms\"}]}",
              _recognize("avance en levant les bras", actionRecognizer, lingDb, frLanguage));

    EXPECT_EQ("{\"intent\": \"move\", \"to_run_sequentially\": [{\"intent\": \"raise_arms\"}]}",
              _recognize("avance puis lève les bras", actionRecognizer, lingDb, frLanguage));

    EXPECT_EQ("{\"intent\": \"bring(obj=patate)\"}",
              _recognize("apporte une patate", actionRecognizer, lingDb, frLanguage));

    EXPECT_EQ("{\"intent\": \"add(nb=1)\"}",
              _recognize("Ajoute un", actionRecognizer, lingDb, frLanguage));

    EXPECT_EQ("{\"intent\": \"add(nb=1)\", "
              "\"condition\": {\"intent\": \"clicked(c=checkpoint2)\"}}",
              _recognize("Quand le checkpoint du plateau est pressée, ajoute un", actionRecognizer, lingDb, frLanguage));

    EXPECT_EQ("{\"intent\": \"add(nb=1)\", "
              "\"condition\": {\"intent\": \"clicked(c=checkpoint3)\"}}",
              _recognize("Si le checkpoint de Charles est pressé, ajoute un", actionRecognizer, lingDb, frLanguage));

    EXPECT_EQ("{\"intent\": \"go_to_loc(loc=checkpoint1)\", "
              "\"condition\": {\"intent\": \"clicked(c=checkpoint1)\"}}",
              _recognize("quand le checkpoint Virginie est pressée, va au checkpoint Virginie", actionRecognizer, lingDb, frLanguage));

    EXPECT_EQ("{\"condition\": {\"intent\": \"clicked(c=checkpoint1)\"}}",
              _recognize("si le checkpoint Virginie est cliquée", actionRecognizer, lingDb, frLanguage));

    EXPECT_EQ("{\"condition\": {\"intent\": \"clicked(c=Le checkpoint NomQuiExistePas)\"}}",
              _recognize("si le checkpoint NomQuiExistePas est cliquée", actionRecognizer, lingDb, frLanguage));

    EXPECT_EQ("{\"intent\": \"add(nb=1)\", \"condition\": {\"intent\": \"clicked(c=Le checkpoint de NomQuiExistePas)\"}}",
              _recognize("si le checkpoint de NomQuiExistePas est cliquée, ajoute un", actionRecognizer, lingDb, frLanguage));

    EXPECT_EQ("{\"condition\": {\"intent\": \"clicked(c=checkpoint1)\"}}",
              _recognize("quand le checkpoint Virginie est cliquée", actionRecognizer, lingDb, frLanguage));

    EXPECT_EQ("{\"condition\": {\"intent\": \"clicked(c=checkpoint1)\"}}",
              _recognize("à chaque fois que le checkpoint Virginie est cliquée", actionRecognizer, lingDb, frLanguage));

    EXPECT_EQ("{\"intent\": \"__unknown__(from_text=saute)\", "
              "\"condition\": {\"intent\": \"clicked(c=checkpoint1)\"}}",
              _recognize("quand le checkpoint Virginie est cliquée saute", actionRecognizer, lingDb, frLanguage));

    EXPECT_EQ("{\"intent\": \"__unknown__(from_text=ça sera une belle journée)\", "
              "\"condition\": {\"intent\": \"clicked(c=checkpoint1)\"}}",
              _recognize("quand le checkpoint Virginie est cliquée ça sera une belle journée", actionRecognizer, lingDb, frLanguage));

    EXPECT_EQ("{\"intent\": \"add(nb=1)\", \"condition\": {\"intent\": \"sees_someone\"}}",
              _recognize("quand tu vois quelqu'un, ajoute un", actionRecognizer, lingDb, frLanguage));

    EXPECT_EQ("{\"intent\": \"arms_down\"}",
              _recognize("relâche tes bras", actionRecognizer, lingDb, frLanguage));
    EXPECT_EQ("{\"intent\": \"arms_down\"}",
              _recognize("baisse tes bras", actionRecognizer, lingDb, frLanguage));
    EXPECT_EQ("{\"intent\": \"arms_down\"}",
              _recognize("baisse tes mains", actionRecognizer, lingDb, frLanguage));
    EXPECT_EQ("{\"intent\": \"arms_down\"}",
              _recognize("j'aimerais que tu baisses tes bras", actionRecognizer, lingDb, frLanguage));
    EXPECT_EQ("{\"intent\": \"arms_down\"}",
              _recognize("Veux-tu bien baisser tes bras ?", actionRecognizer, lingDb, frLanguage));
    EXPECT_EQ("{\"intent\": \"arms_down\"}",
              _recognize("Voudrais-tu bien baisser tes bras ?", actionRecognizer, lingDb, frLanguage));
    EXPECT_EQ("{\"intent\": \"arms_down\"}",
              _recognize("baisser tes bras", actionRecognizer, lingDb, frLanguage));
    EXPECT_EQ("{\"intent\": \"arms_down\"}",
              _recognize("tu dois baisser tes bras", actionRecognizer, lingDb, frLanguage));
    EXPECT_EQ("{\"intent\": \"arms_down\"}",
              _recognize("tu aurais l'amabilité de baisser tes bras", actionRecognizer, lingDb, frLanguage));
    EXPECT_EQ("{\"intent\": \"arms_down\"}",
              _recognize("aurais-tu la gentillesse de baisser tes bras", actionRecognizer, lingDb, frLanguage));

    EXPECT_EQ("", _recognize("ne relâche pas tes bras", actionRecognizer, lingDb, frLanguage));
    EXPECT_TRUE(_isObviouslyWrong("arms_down", "ne relâche pas tes bras", actionRecognizer, lingDb, frLanguage));
    EXPECT_FALSE(_isObviouslyWrong("arms_down", "ne monte pas tes bras", actionRecognizer, lingDb, frLanguage));

    EXPECT_EQ("{\"intent\": \"unfreeze\"}",
              _recognize("défige-toi", actionRecognizer, lingDb, frLanguage));
    EXPECT_EQ("{\"intent\": \"what_i_know\"}",
              _recognize("dis ce que tu sais", actionRecognizer, lingDb, frLanguage));

    EXPECT_EQ("{\"intent\": \"what_i_know\", "
              "\"condition\": {\"intent\": \"same_location(c=checkpoint1)\"}}",
              _recognize("si tu es proche du checkpoint Virginie dis ce que tu sais", actionRecognizer, lingDb, frLanguage));
    EXPECT_EQ("{\"intent\": \"what_i_know\", "
              "\"condition\": {\"intent\": \"same_location(c=checkpoint1)\"}}",
              _recognize("si tu es pas loin du checkpoint Virginie dis ce que tu sais", actionRecognizer, lingDb, frLanguage));
    EXPECT_EQ("{\"intent\": \"what_i_know\"}",
              _recognize("si tu es loin du checkpoint Virginie dis ce que tu sais", actionRecognizer, lingDb, frLanguage));

    EXPECT_EQ("{\"intent\": \"move(distance=10 mètres)\"}",
              _recognize("avance de 10 mètres", actionRecognizer, lingDb, frLanguage));
    EXPECT_EQ("{\"intent\": \"move(distance=0,2 mètre)\"}",
              _recognize("avance de 20 centimètres", actionRecognizer, lingDb, frLanguage));
    EXPECT_EQ("{\"intent\": \"turn_left(angle=34 degrés)\"}",
              _recognize("tourne à gauche de 34 degrés", actionRecognizer, lingDb, frLanguage));
    EXPECT_EQ("{\"intent\": \"turn_right(angle=12 degrés)\"}",
              _recognize("tourne à droite de 12 degrés", actionRecognizer, lingDb, frLanguage));
    EXPECT_EQ("{\"intent\": \"low_battery_level_reaction\"}",
              _recognize("Avertis de la batterie faible", actionRecognizer, lingDb, frLanguage));
    EXPECT_EQ("{\"intent\": \"salute\"}",
              _recognize("salue de la main et de la voix", actionRecognizer, lingDb, frLanguage));
    EXPECT_EQ("{\"intent\": \"look_at_somebody\"}",
              _recognize("regarde un humain", actionRecognizer, lingDb, frLanguage));
    EXPECT_EQ("{\"intent\": \"look_at_somebody\"}",
              _recognize("regarde un utilisateur", actionRecognizer, lingDb, frLanguage));

    EXPECT_EQ("{\"intent\": \"say(something=Je suis content.)\"}",
              _recognize("dis que tu es content", actionRecognizer, lingDb, frLanguage));
    EXPECT_EQ("{\"intent\": \"put\"}",
              _recognize("pose le panier", actionRecognizer, lingDb, frLanguage));
    EXPECT_EQ("{\"intent\": \"put\"}",
              _recognize("pose les bonbons", actionRecognizer, lingDb, frLanguage));

    EXPECT_EQ("{\"intent\": \"put\", \"to_run_in_parallel\": [{\"intent\": \"look_at_somebody\"}]}",
              _recognize("pose les bonbons et regarde un utilisateur", actionRecognizer, lingDb, frLanguage));
    EXPECT_EQ("{\"intent\": \"put\", \"to_run_sequentially\": [{\"intent\": \"look_at_somebody\"}]}",
              _recognize("pose les bonbons puis regarde un utilisateur", actionRecognizer, lingDb, frLanguage));
    EXPECT_EQ("{\"intent\": \"put\", \"to_run_in_background\": [{\"intent\": \"look_at_somebody\"}]}",
              _recognize("pose les bonbons en regardant un utilisateur", actionRecognizer, lingDb, frLanguage));


    EXPECT_EQ("", _recognize("avance puis fais un saut périlleux", actionRecognizer, lingDb, frLanguage));

    EXPECT_EQ("{\"intent\": \"__unknown__(from_text=fais un saut périlleux)\", \"condition\": {\"intent\": \"clicked(c=checkpoint1)\"}}",
              _recognize("si le checkpoint Virginie est cliquée, fais un saut périlleux", actionRecognizer, lingDb, frLanguage));
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
    std::map<std::string, ActionRecognizer::ParamInfo> howFarInMetersParameter{
        {"distance", ActionRecognizer::ParamInfo("checkpoint", {"how far in meters"})}};
    actionRecognizer.addAction("move", {"move forward"}, howFarInMetersParameter, lingDb);
    std::map<std::string, ActionRecognizer::ParamInfo> howFarInDegreesParameter{
        {"angle", ActionRecognizer::ParamInfo("angle", {"how far in degrees"})}};
    actionRecognizer.addAction("turn_left", {"turn left"}, howFarInDegreesParameter, lingDb);
    actionRecognizer.addAction("not_move", {"to not move"}, howFarInDegreesParameter, lingDb);
    actionRecognizer.addAction("look_at_oneself", {"to look at oneself"}, howFarInDegreesParameter, lingDb);
    actionRecognizer.addAction("put", {"put something"}, {}, lingDb);

    EXPECT_EQ("{\"intent\": \"go_to_loc(loc=The Virginie checkpoint)\"}",
              _recognize("go to  the Virginie checkpoint", actionRecognizer, lingDb, enLanguage));

    EXPECT_EQ("{\"intent\": \"go_to_loc(loc=The Virginie checkpoint)\", "
              "\"condition\": {\"intent\": \"clicked(c=The Virginie checkpoint)\"}}",
              _recognize("if the Virginie checkpoint is clicked go to  the Virginie checkpoint", actionRecognizer, lingDb, enLanguage));

    EXPECT_EQ("{\"condition\": {\"intent\": \"clicked(c=The Virginie checkpoint)\"}}",
              _recognize("if the Virginie checkpoint is clicked", actionRecognizer, lingDb, enLanguage));

    EXPECT_EQ("{\"condition\": {\"intent\": \"clicked(c=The Virginie checkpoint)\"}}",
              _recognize("when the Virginie checkpoint is clicked", actionRecognizer, lingDb, enLanguage));

    EXPECT_EQ("{\"condition\": {\"intent\": \"clicked(c=The Virginie checkpoint)\"}}",
              _recognize("whenever the Virginie checkpoint is clicked", actionRecognizer, lingDb, enLanguage));

    EXPECT_EQ("{\"intent\": \"arms_down\"}",
              _recognize("relax your arms", actionRecognizer, lingDb, enLanguage));
    EXPECT_EQ("{\"intent\": \"arms_down\"}",
              _recognize("Do you want to lower your arms?", actionRecognizer, lingDb, enLanguage));
    EXPECT_EQ("{\"intent\": \"arms_down\"}",
              _recognize("Would you like to lower your arms?", actionRecognizer, lingDb, enLanguage));
    EXPECT_EQ("{\"intent\": \"arms_down\"}",
              _recognize("Would you be kind enough to relax your arms?", actionRecognizer, lingDb, enLanguage));
    EXPECT_EQ("{\"intent\": \"arms_down\"}",
              _recognize("Would you be so kind as to lower your arms?", actionRecognizer, lingDb, enLanguage));
    EXPECT_EQ("{\"intent\": \"arms_down\"}",
              _recognize("I would like you to lower your arms", actionRecognizer, lingDb, enLanguage));
    EXPECT_EQ("{\"intent\": \"arms_down\"}",
              _recognize("I'd like you to drop your arms", actionRecognizer, lingDb, enLanguage));

    EXPECT_EQ("{\"intent\": \"move(distance=10 meters)\"}",
              _recognize("move forward 10 meters", actionRecognizer, lingDb, enLanguage));
    EXPECT_EQ("{\"intent\": \"turn_left(angle=32 degrees)\"}",
              _recognize("Turn left 32 degrees", actionRecognizer, lingDb, enLanguage));
    EXPECT_EQ("{\"intent\": \"not_move\"}",
              _recognize("Don't move", actionRecognizer, lingDb, enLanguage));

    EXPECT_EQ("{\"intent\": \"look_at_oneself\"}",
              _recognize("look at oneself", actionRecognizer, lingDb, enLanguage));
    EXPECT_EQ("{\"intent\": \"look_at_oneself\"}",
              _recognize("look at yourself", actionRecognizer, lingDb, enLanguage));
    EXPECT_EQ("{\"intent\": \"put\"}",
              _recognize("put a tray", actionRecognizer, lingDb, enLanguage));
}
