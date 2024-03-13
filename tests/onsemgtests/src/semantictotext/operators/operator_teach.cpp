#include "../../semanticreasonergtests.hpp"
#include "operator_answer.hpp"
#include "operator_inform.hpp"
#include "operator_resolveCommand.hpp"
#include "operator_executeFromCondition.hpp"
#include <gtest/gtest.h>
#include <onsem/common/utility/noresult.hpp>
#include <onsem/tester/reactOnTexts.hpp>
#include <onsem/semantictotext/type/naturallanguageexpression.hpp>
#include <onsem/texttosemantic/tool/semexpgetter.hpp>
#include <onsem/texttosemantic/languagedetector.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/groundedexpression.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemory.hpp>
#include <onsem/semantictotext/semanticconverter.hpp>
#include <onsem/semantictotext/semexpoperators.hpp>

using namespace onsem;
namespace {

DetailedReactionAnswer _operator_teach(const std::string& pText,
                                       SemanticMemory& pSemanticMemory,
                                       const linguistics::LinguisticDatabase& pLingDb,
                                       memoryOperation::SemanticActionOperatorEnum pActionOperator,
                                       SemanticLanguageEnum pTextLanguage = SemanticLanguageEnum::UNKNOWN) {
    SemanticLanguageEnum textLanguage =
        pTextLanguage == SemanticLanguageEnum::UNKNOWN ? linguistics::getLanguage(pText, pLingDb) : pTextLanguage;
    TextProcessingContext inContext(SemanticAgentGrounding::currentUser, SemanticAgentGrounding::me, textLanguage);
    auto semExp = converter::textToContextualSemExp(pText, inContext, SemanticSourceEnum::ASR, pLingDb);
    memoryOperation::mergeWithContext(semExp, pSemanticMemory, pLingDb);

    if (textLanguage == SemanticLanguageEnum::UNKNOWN)
        textLanguage = pSemanticMemory.defaultLanguage;
    mystd::unique_propagate_const<UniqueSemanticExpression> reaction;
    auto inputSemExpInMemory =
        memoryOperation::teach(reaction, pSemanticMemory, std::move(semExp), pLingDb, pActionOperator);
    return reactionToAnswer(reaction, pSemanticMemory, pLingDb, textLanguage, inputSemExpInMemory);
}

DetailedReactionAnswer _operator_teachAResourceWithParameters(
    const std::string& pAction,
    const std::map<std::string, std::vector<std::string>>& pParameterLabelToQuestionsStrs,
    SemanticMemory& pSemanticMemory,
    const linguistics::LinguisticDatabase& pLingDb,
    memoryOperation::SemanticActionOperatorEnum pActionOperator,
    SemanticLanguageEnum pLanguage = SemanticLanguageEnum::UNKNOWN) {
    SemanticLanguageEnum textLanguage =
        pLanguage == SemanticLanguageEnum::UNKNOWN ? linguistics::getLanguage(pAction, pLingDb) : pLanguage;

    TextProcessingContext triggerProcContext(
        SemanticAgentGrounding::currentUser, SemanticAgentGrounding::me, textLanguage);
    triggerProcContext.isTimeDependent = false;
    auto actionSemExp = converter::textToSemExp(pAction, triggerProcContext, pLingDb);

    auto outputResourceGrdExp = std::make_unique<GroundedExpression>(converter::createResourceWithParameters(
        resourceLabelForTests_cmd, pAction, pParameterLabelToQuestionsStrs, *actionSemExp, pLingDb, textLanguage));

    if (textLanguage == SemanticLanguageEnum::UNKNOWN)
        textLanguage = pSemanticMemory.defaultLanguage;
    mystd::unique_propagate_const<UniqueSemanticExpression> reaction;
    auto infinitiveActionSemExp = converter::imperativeToInfinitive(*actionSemExp);
    if (!infinitiveActionSemExp)
        return {};
    auto inputSemExpInMemory = memoryOperation::teachSplitted(reaction,
                                                              pSemanticMemory,
                                                              std::move(*infinitiveActionSemExp),
                                                              std::move(outputResourceGrdExp),
                                                              pLingDb,
                                                              pActionOperator);
    return reactionToAnswer(reaction, pSemanticMemory, pLingDb, textLanguage, inputSemExpInMemory);
}

DetailedReactionAnswer operator_teachBehavior(const std::string& pText,
                                              SemanticMemory& pSemanticMemory,
                                              const linguistics::LinguisticDatabase& pLingDb,
                                              SemanticLanguageEnum pTextLanguage = SemanticLanguageEnum::UNKNOWN) {
    return _operator_teach(
        pText, pSemanticMemory, pLingDb, memoryOperation::SemanticActionOperatorEnum::BEHAVIOR, pTextLanguage);
}

DetailedReactionAnswer operator_teachCondition(const std::string& pText,
                                               SemanticMemory& pSemanticMemory,
                                               const linguistics::LinguisticDatabase& pLingDb,
                                               SemanticLanguageEnum pTextLanguage = SemanticLanguageEnum::UNKNOWN) {
    return _operator_teach(
        pText, pSemanticMemory, pLingDb, memoryOperation::SemanticActionOperatorEnum::CONDITION, pTextLanguage);
}

DetailedReactionAnswer operator_teachInformation(const std::string& pText,
                                                 SemanticMemory& pSemanticMemory,
                                                 const linguistics::LinguisticDatabase& pLingDb,
                                                 SemanticLanguageEnum pTextLanguage = SemanticLanguageEnum::UNKNOWN) {
    return _operator_teach(
        pText, pSemanticMemory, pLingDb, memoryOperation::SemanticActionOperatorEnum::INFORMATION, pTextLanguage);
}

}

TEST_F(SemanticReasonerGTests, operator_teachBehavior_basic) {
    const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
    SemanticMemory semMem;
    memoryOperation::learnSayCommand(semMem, lingDb);

    ONSEM_NOANSWER(operator_teachBehavior("Hello", semMem, lingDb));
    ONSEM_NOANSWER(operator_teachBehavior("Look left", semMem, lingDb));
    ONSEM_NOANSWER(operator_teachBehavior("Paul likes chocolate", semMem, lingDb));
    ONSEM_NOANSWER(operator_teachBehavior("Gustave likes chocolate", semMem, lingDb));
    ONSEM_NOANSWER(operator_teachBehavior("I am Paul", semMem, lingDb));
    ONSEM_ANSWERNOTFOUND_EQ("I don't know who you are.", operator_answer("Who am I", semMem, lingDb));
    ONSEM_NOANSWER(operator_teachBehavior("say hi when you see me", semMem, lingDb));
    EXPECT_EQ("", operator_resolveCommand("smile", semMem, lingDb));
    ONSEM_TEACHINGFEEDBACK_EQ("Ok, to smile is to say I am smiling.",
                              operator_teachBehavior("to smile is to say I am smiling", semMem, lingDb));
    EXPECT_EQ("I am smiling.", operator_resolveCommand("smile", semMem, lingDb));
    EXPECT_EQ("I am smiling.", operator_execute("smile", semMem, lingDb));
    EXPECT_EQ("", operator_executeFromCondition("smile", semMem, lingDb));
}

TEST_F(SemanticReasonerGTests, operator_teachCondition_basic) {
    const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
    SemanticMemory semMem;
    memoryOperation::learnSayCommand(semMem, lingDb);

    ONSEM_NOANSWER(operator_teachCondition("Hello", semMem, lingDb));
    ONSEM_NOANSWER(operator_teachCondition("Look left", semMem, lingDb));
    ONSEM_NOANSWER(operator_teachCondition("Paul likes chocolate", semMem, lingDb));
    ONSEM_NOANSWER(operator_teachCondition("Gustave likes chocolate", semMem, lingDb));
    ONSEM_NOANSWER(operator_teachCondition("I am Paul", semMem, lingDb));
    ONSEM_ANSWERNOTFOUND_EQ("I don't know who you are.", operator_answer("Who am I", semMem, lingDb));
    ONSEM_NOTIFYSOMETHINGWILLBEDONE_EQ("Ok, I will say hi whenever I see you.",
                                       operator_teachCondition("say hi when you see me", semMem, lingDb));
    static const std::string helloPaulQuote = "\"Bonjour Paul!!\"";
    ONSEM_NOTIFYSOMETHINGWILLBEDONE_EQ(
        "Ok, I will say " + helloPaulQuote + " whenever I see Paul.",
        operator_teachCondition("Say " + helloPaulQuote + " when you see Paul", semMem, lingDb));
    EXPECT_EQ(helloPaulQuote, operator_executeFromCondition("You see Paul", semMem, lingDb));
    EXPECT_EQ("Hi", operator_executeFromCondition("You see me", semMem, lingDb));
    EXPECT_EQ("", operator_executeFromCondition("You don't see me", semMem, lingDb));
    EXPECT_EQ("Hi", operator_execute("You see me", semMem, lingDb));
    EXPECT_EQ("", operator_executeFromCondition("You don't see me", semMem, lingDb));
    EXPECT_EQ("", operator_resolveCommand("You see me", semMem, lingDb));
    ONSEM_NOANSWER(operator_teachCondition("to smile is to say I am smiling", semMem, lingDb));
    EXPECT_EQ("", operator_resolveCommand("smile", semMem, lingDb));
}

TEST_F(SemanticReasonerGTests, operator_teachCondition_fromInform) {
    const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
    SemanticMemory semMem;
    const std::string commandResource = "\\" + resourceLabelForTests_cmd + "=presentCompany()\\";
    operator_inform("to present the company means " + commandResource
                        + ". "
                          "Present the company when somebody is interested",
                    semMem,
                    lingDb);
    // trigger it from a text
    EXPECT_EQ(commandResource, operator_executeFromCondition("Somebody is interested", semMem, lingDb));
    // trigger it from a natural language expression
    NaturalLanguageExpression nle;
    nle.word.text = "interest";
    nle.word.type = NaturalLanguageTypeOfText::VERB;
    nle.word.language = SemanticLanguageEnum::ENGLISH;
    nle.verbTense = SemanticVerbTense::PRESENT;
    NaturalLanguageExpression totoNle;
    totoNle.word.text = "toto";
    totoNle.word.type = NaturalLanguageTypeOfText::AGENT;
    totoNle.word.language = SemanticLanguageEnum::ENGLISH;
    nle.children.emplace(GrammaticalType::OBJECT, std::move(totoNle));
    auto totoInterestedSemExp = converter::naturalLanguageExpressionToSemanticExpression(nle, lingDb);
    EXPECT_EQ(
        commandResource,
        operator_executeFromSemExpCondition(*totoInterestedSemExp, SemanticLanguageEnum::ENGLISH, semMem, lingDb));
}

TEST_F(SemanticReasonerGTests, operator_teachInformation_basic) {
    const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
    SemanticMemory semMem;
    memoryOperation::learnSayCommand(semMem, lingDb);

    ONSEM_NOANSWER(operator_teachInformation("Hello", semMem, lingDb));
    ONSEM_NOANSWER(operator_teachInformation("Look left", semMem, lingDb));
    ONSEM_NOANSWER(operator_teachInformation("Paul likes chocolate", semMem, lingDb));
    ONSEM_NOANSWER(operator_teachInformation("Gustave likes chocolate", semMem, lingDb));
    ONSEM_NOANSWER(operator_teachInformation("I am Paul", semMem, lingDb));
    ONSEM_ANSWER_EQ("You are Paul.", operator_answer("Who am I", semMem, lingDb));
    ONSEM_NOANSWER(operator_teachInformation("say hi when you see me", semMem, lingDb));
    ONSEM_NOANSWER(operator_teachInformation("to smile is to say I am smiling", semMem, lingDb));
    EXPECT_EQ("", operator_resolveCommand("smile", semMem, lingDb));
}

TEST_F(SemanticReasonerGTests, operator_teachBehavior_frenchMainFormulation) {
    const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
    SemanticMemory semMem;
    memoryOperation::learnSayCommand(semMem, lingDb);

    EXPECT_EQ("", operator_resolveCommand("marche", semMem, lingDb));
    ONSEM_TEACHINGFEEDBACK_EQ("Ok pour marcher il faut dire je marche. Et puis ?",
                              operator_teachBehavior("pour marcher il faut dire je marche", semMem, lingDb));
    ONSEM_TEACHINGFEEDBACK_EQ("Ok pour marcher il faut dire je marche et puis il faut dire c'est tout. Et puis ?",
                              operator_teachBehavior("il faut dire c'est tout", semMem, lingDb));
    EXPECT_EQ("Je marche.\tTHEN\tC'est tout.", operator_resolveCommand("marche", semMem, lingDb));

    EXPECT_EQ("", operator_resolveCommand("cours", semMem, lingDb));
    ONSEM_TEACHINGFEEDBACK_EQ("Ok pour courir il faut dire j'utilise mes jambes. Et puis ?",
                              operator_react("pour courir il faut dire j'utilise mes jambes", semMem, lingDb));
    ONSEM_BEHAVIOR_EQ("J'utilise mes jambes.", operator_react("cours", semMem, lingDb));
    ONSEM_BEHAVIOR_EQ("(\t(\tJe marche.\tTHEN\tC'est tout.\t)\tIN_BACKGROUND: J'utilise mes jambes.\t)",
                      operator_react("marche en courant", semMem, lingDb));

    ONSEM_BEHAVIORNOTFOUND_EQ("Je ne sais pas grimper.", operator_react("grimpe", semMem, lingDb));
    ONSEM_TEACHINGFEEDBACK_EQ("Ok pour grimper il faut dire je marche et il faut sauter. Et puis ?",
                              operator_react("pour grimper il faut dire je marche et sauter", semMem, lingDb));
    ONSEM_BEHAVIORNOTFOUND_EQ("Je ne sais pas sauter.", operator_react("grimpe", semMem, lingDb));
    EXPECT_EQ("", operator_resolveCommand("grimpe", semMem, lingDb));
    ONSEM_ANSWER_EQ("Il y a 2 étapes pour grimper. Veux-tu que je les dise une par une ?",
                    operator_react("Comment grimper ?", semMem, lingDb));
    ONSEM_ANSWER_EQ("Il faut dire je marche et il faut sauter.", operator_react("non", semMem, lingDb));
    ONSEM_ANSWER_EQ("Il y a 2 étapes pour grimper. Veux-tu que je les dise une par une ?",
                    operator_react("Comment grimper ?", semMem, lingDb));
    ONSEM_ANSWER_EQ("Il faut dire je marche. Dis et après pour continuer !", operator_react("oui", semMem, lingDb));
    ONSEM_ANSWER_EQ("Il faut sauter.", operator_react("et après", semMem, lingDb));
    ONSEM_ANSWER_EQ("Il faut dire je marche. Dis et apr\xC3\xA8s pour continuer !",
                    operator_react("et avant", semMem, lingDb));

    ONSEM_BEHAVIOR_EQ("Il y a 2 étapes pour grimper. Veux-tu que je les dise une par une ?",
                      operator_react("Dis comment grimper", semMem, lingDb));
    ONSEM_ANSWER_EQ("Il faut dire je marche. Dis et après pour continuer !", operator_react("oui", semMem, lingDb));
    ONSEM_ANSWER_EQ("Il faut sauter.", operator_react("et après", semMem, lingDb));
    ONSEM_ANSWER_EQ("C'est fini.", operator_react("et après", semMem, lingDb));
    ONSEM_ANSWERNOTFOUND_EQ("Je ne sais pas ce qui s'est passé juste après ça.",
                            operator_react("et après", semMem, lingDb));
    ONSEM_TEACHINGFEEDBACK_EQ("Ok pour faire le beau il faut dire je fais le beau. Et puis ?",
                              operator_react("pour faire le beau il faut dire je fais le beau", semMem, lingDb));
    ONSEM_TEACHINGFEEDBACK_EQ(
        "Ok pour faire le beau il faut dire je fais le beau et puis il faut lever la tête. Et puis ?",
        operator_react("puis il faut lever la tête", semMem, lingDb));
    ONSEM_TEACHINGFEEDBACK_EQ("Ok pour faire le beau il faut dire je fais le beau, puis il faut lever la tête, puis il "
                              "faut danser et puis il faut dire houhou. Et puis ?",
                              operator_react("puis il faut danser et il faut dire houhou", semMem, lingDb));
    ONSEM_NOANSWER(operator_react("C'est tout", semMem, lingDb));
}

TEST_F(SemanticReasonerGTests, operator_teachBehavior_cookingRecipe) {
    const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
    SemanticMemory semMem;
    memoryOperation::learnSayCommand(semMem, lingDb);

    ONSEM_TEACHINGFEEDBACK_EQ(
        "Ok pour faire des pâtes il faut de l'eau et des pâtes et puis il faut faire bouillir les pâtes. Et puis ?",
        operator_react("pour faire des pâtes il faut de l'eau et des pâtes puis il faut faire bouillir les pâtes",
                       semMem,
                       lingDb));
    ONSEM_TEACHINGFEEDBACK_EQ(
        "Je ne peux pas faire des pâtes mais je sais comment faire. Veux-tu que je te dise comment faire des pâtes ?",
        operator_react("fais des pâtes", semMem, lingDb));
    ONSEM_BEHAVIOR_EQ("Il y a 2 étapes pour faire des pâtes. Veux-tu que je les dise une par une ?",
                      operator_react("Oui", semMem, lingDb));
    ONSEM_ANSWER_EQ("Il faut de l'eau et des pâtes. Dis et après pour continuer !",
                    operator_react("Oui", semMem, lingDb));
    ONSEM_ANSWER_EQ("Il faut faire bouillir les pâtes.", operator_react("Et après", semMem, lingDb));
}

TEST_F(SemanticReasonerGTests, operator_teachBehavior_from_constructTeachSemExp) {
    const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
    SemanticMemory semMem;
    SemanticLanguageEnum language = SemanticLanguageEnum::FRENCH;

    EXPECT_EQ("", operator_resolveCommand("saute", semMem, lingDb));

    TextProcessingContext textProc(SemanticAgentGrounding::currentUser, SemanticAgentGrounding::me, language);
    auto labelSemExp = converter::textToContextualSemExp("saute", textProc, SemanticSourceEnum::WRITTENTEXT, lingDb);
    auto infLabelSemExp = converter::imperativeToInfinitive(*labelSemExp);
    ASSERT_TRUE(infLabelSemExp);

    static const std::string cmdValue = "cmd_value";
    auto answerSemExp = std::make_unique<GroundedExpression>(
        std::make_unique<SemanticResourceGrounding>(resourceLabelForTests_cmd, language, cmdValue));

    auto teachSemExp = converter::constructTeachSemExp(std::move(*infLabelSemExp), std::move(answerSemExp));
    mystd::unique_propagate_const<UniqueSemanticExpression> reaction;
    auto inputSemExpInMemory = memoryOperation::teach(
        reaction, semMem, std::move(teachSemExp), lingDb, memoryOperation::SemanticActionOperatorEnum::BEHAVIOR);
    ONSEM_TEACHINGFEEDBACK_EQ(
        "Ok pour sauter \tTHEN\t\\" + resourceLabelForTests_cmd + "=#fr_FR#" + cmdValue + "\\\tTHEN\t et puis ?",
        reactionToAnswer(reaction, semMem, lingDb, language, inputSemExpInMemory));

    EXPECT_EQ("\\" + resourceLabelForTests_cmd + "=#fr_FR#" + cmdValue + "\\",
              operator_resolveCommand("saute", semMem, lingDb));
}

TEST_F(SemanticReasonerGTests, operator_teachBehavior_repetitions) {
    const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
    SemanticMemory semMem;
    SemanticLanguageEnum language = SemanticLanguageEnum::FRENCH;
    memoryOperation::learnSayCommand(semMem, lingDb);

    const std::map<std::string, std::vector<std::string>> distanceParameter{{"distance", {"combien de mètres"}}};
    const std::string moveForwardStr = "avance";
    _operator_teachAResourceWithParameters(moveForwardStr,
                                           distanceParameter,
                                           semMem,
                                           lingDb,
                                           memoryOperation::SemanticActionOperatorEnum::BEHAVIOR,
                                           language);

    const std::map<std::string, std::vector<std::string>> objectParameter{{"object", {"quoi"}}};
    const std::string putMusicStr = "mets une musique";
    _operator_teachAResourceWithParameters(
        putMusicStr, objectParameter, semMem, lingDb, memoryOperation::SemanticActionOperatorEnum::BEHAVIOR, language);

    EXPECT_EQ("\\" + resourceLabelForTests_cmd + "=#fr_FR#" + moveForwardStr + "\\",
              operator_resolveCommand("avance", semMem, lingDb));

    EXPECT_EQ("\\" + resourceLabelForTests_cmd + "=#fr_FR#" + moveForwardStr + "(distance=3 mètres)\\",
              operator_resolveCommand("avance 3 mètres", semMem, lingDb));

    const std::map<std::string, std::vector<std::string>> howManyDegreesParameterQuestion = {
        {"angle", {"combien de degrés"}}};
    const std::string turnRightStr = "Tourne à droite";
    const std::string turnLeftStr = "Tourne à gauche";
    const std::vector<std::string> turnsStr = {/* "Tourne", */ turnRightStr, turnLeftStr};
    for (const auto& currTurnStr : turnsStr)
        _operator_teachAResourceWithParameters(currTurnStr,
                                               howManyDegreesParameterQuestion,
                                               semMem,
                                               lingDb,
                                               memoryOperation::SemanticActionOperatorEnum::BEHAVIOR,
                                               language);

    ONSEM_TEACHINGFEEDBACK_EQ(
        "Ok pour faire un carré il faut avancer de 30 centimètres. Et puis ?",
        operator_teachBehavior("pour faire un carré il faut avancer de 30 centimètres", semMem, lingDb));
    ONSEM_TEACHINGFEEDBACK_EQ("Ok pour faire un carré il faut avancer de 30 centimètres et puis il faut tourner de 90 "
                              "degrés à droite. Et puis ?",
                              operator_teachBehavior("il faut tourner à droite de 90 degrés", semMem, lingDb));
    ONSEM_TEACHINGFEEDBACK_EQ("Ok pour faire un carré il faut avancer de 30 centimètres, puis il faut tourner de 90 "
                              "degrés à droite et puis il faut répéter tout 3 fois. Et puis ?",
                              operator_teachBehavior("il faut répéter tout ça 3 fois", semMem, lingDb));
    ONSEM_TEACHINGFEEDBACK_EQ(
        "Ok pour faire un carré il faut avancer de 30 centimètres, puis il faut tourner de 90 degrés à droite, puis il "
        "faut répéter tout 3 fois et puis il faut tourner à gauche. Et puis ?",
        operator_teachBehavior("il faut tourner à gauche", semMem, lingDb));

    std::string doSquareOutput = "(\t(\t\\" + resourceLabelForTests_cmd + "=#fr_FR#" + moveForwardStr
                               + "(distance=0,3 mètre)\\\tTHEN\t\\" + resourceLabelForTests_cmd + "=#fr_FR#"
                               + turnRightStr + "(angle=90 degrés)\\\t)" + "\tNUMBER_OF_TIMES: 4\t)" + "\tTHEN\t\\"
                               + resourceLabelForTests_cmd + "=#fr_FR#" + turnLeftStr + "\\";

    EXPECT_EQ(doSquareOutput, operator_resolveCommand("fais un carré", semMem, lingDb));

    ONSEM_TEACHINGFEEDBACK_EQ("Ok pour faire un truc cool il faut faire un carré. Et puis ?",
                              operator_teachBehavior("pour faire un truc cool il faut faire un carré", semMem, lingDb));
    ONSEM_TEACHINGFEEDBACK_EQ(
        "Ok pour faire un truc cool il faut faire un carré et puis il faut le faire en mettant une musique de Michael "
        "Jackson. Et puis ?",
        operator_teachBehavior("il faut faire ça en mettant une musique de Michael Jackson", semMem, lingDb));

    EXPECT_EQ("(\t(\t" + doSquareOutput + "\t)\tIN_BACKGROUND: \\" + resourceLabelForTests_cmd
                  + "=#fr_FR#mets une musique(object=Une musique de Michael Jackson)\\\t)",
              operator_resolveCommand("fais un truc cool", semMem, lingDb));
}
