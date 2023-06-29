#include "triggers_match.hpp"
#include <gtest/gtest.h>
#include <onsem/texttosemantic/languagedetector.hpp>
#include <onsem/texttosemantic/tool/semexpgetter.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemory.hpp>
#include <onsem/semantictotext/semexpoperators.hpp>
#include <onsem/semantictotext/semanticconverter.hpp>
#include <onsem/semantictotext/triggers.hpp>
#include <onsem/tester/reactOnTexts.hpp>
#include "../operators/operator_inform.hpp"
#include "../../semanticreasonergtests.hpp"
#include "triggers_add.hpp"

using namespace onsem;

namespace
{

DetailedReactionAnswer _operator_reactFromTrigger_fromSemExp(UniqueSemanticExpression pSemExp,
                                                             SemanticMemory& pSemanticMemory,
                                                             const linguistics::LinguisticDatabase& pLingDb,
                                                             SemanticLanguageEnum pTextLanguage,
                                                             const ReactionOptions* pReactionOptions)
{
  if (pTextLanguage == SemanticLanguageEnum::UNKNOWN)
    pTextLanguage = pSemanticMemory.defaultLanguage;
  mystd::unique_propagate_const<UniqueSemanticExpression> reaction;
  memoryOperation::mergeWithContext(pSemExp, pSemanticMemory, pLingDb);
  auto inputSemExpInMemory = triggers::match(reaction, pSemanticMemory, std::move(pSemExp), pLingDb, pReactionOptions);
  return reactionToAnswer(reaction, pSemanticMemory, pLingDb, pTextLanguage, inputSemExpInMemory);
}

}

namespace onsem
{

DetailedReactionAnswer triggers_match(const std::string& pText,
    SemanticMemory& pSemanticMemory,
    const linguistics::LinguisticDatabase& pLingDb,
    SemanticLanguageEnum pTextLanguage,
    const ReactionOptions* pReactionOptions,
    bool pSetUsAsEverybody)
{
  SemanticLanguageEnum textLanguage = pTextLanguage == SemanticLanguageEnum::UNKNOWN ?
      linguistics::getLanguage(pText, pLingDb) : pTextLanguage;
  TextProcessingContext inContext(SemanticAgentGrounding::currentUser,
                                  SemanticAgentGrounding::me,
                                  textLanguage);
  if (pSetUsAsEverybody)
    inContext.setUsAsEverybody();
  auto semExp =
      converter::textToContextualSemExp(pText, inContext,
                                        SemanticSourceEnum::UNKNOWN, pLingDb);
  return _operator_reactFromTrigger_fromSemExp(std::move(semExp), pSemanticMemory, pLingDb,
                                               textLanguage, pReactionOptions);
}

} // End of namespace onsem



TEST_F(SemanticReasonerGTests, operator_reactFromTrigger_basic_fr)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;

  const std::string whoAreYou = "Qui es tu ?";
  const std::string iAmYourFrined = "Je suis ton ami.";
  const std::string stopApplication = "Arrête l'application";
  const std::string itIsStopped = "Voilà, c'est arrêté.";
  const std::string whatTimeItIs = "Quelle heure est-il ?";
  const std::string itIs15h = "Il est 15 heures.";
  const std::string whatAboutWellBeingOfAnimals = "Qu'en est-il du bien-être des animaux ?";
  const std::string itIsNotFamous = "Ce n'est pas fameux.";
  const std::string whatIsToussaint = "Qu'est-ce que la Toussaint ?";
  const std::string itIsAHoliday = "C'est une fête.";
  const std::string whatIsAscension = "Qu'est-ce que l'ascension ?";
  const std::string itIsTheLeavingOfJesusOnHeaven = "C'est le départ de Jésus au ciel.";
  const std::string itLastOneHour = "ça dure 1 heure";
  const std::string itLastTwoHours = "ça dure 2 heures";
  const std::string itIsLong = "C'est long.";
  const std::string itIsShort = "C'est court.";
  const std::string whatIs115 = "C'est quoi le 115 ?";
  const std::string itIsSamusocial = "C'est le samusocial.";
  const std::string longQuestion = "Comment l'Écriture sainte peut-elle être « vérité » alors que tout ce qu'elle contient n'est pas exact ?";
  const std::string itIsHardToSay = "C'est difficile à dire.";
  const std::string trigger1 = "Comment l'Église une ?";
  const std::string reaction1 = "Cette question est bizarre.";
  const std::string trigger2 = "Comment savoir si on fait le bon choix ?";
  const std::string reaction2 = "Ça donne la paix.";
  const std::string trigger3 = "Pour marcher";
  const std::string reaction3 = "Pour faire du sport.";
  const std::string trigger4 = "maintenant";
  const std::string reaction4 = "Paul";
  const std::string trigger5 = "à quoi ça sert de prier ?";
  const std::string reaction5 = "Ça sert à être heureux.";
  const std::string trigger6 = "les 10 commandements";
  const std::string reaction6 = "C'est la première alliance.";
  const std::string trigger7 = "mets l'application Aa";
  const std::string reaction7 = "C'est une application sympa.";
  const std::string trigger8 = "Descends le volume";
  const std::string reaction8 = "Voilà, je parle moins fort.";
  const std::string trigger9 = "Tourne de 30 dégrés";
  const std::string reaction9 = "Ma roue gauche est trop chaude.";
  const std::string trigger10 = "Avance";
  const std::string reaction10 = "Voilà, j'avance.";
  const std::string trigger11 = "Avant se un petit peu";
  const std::string reaction11 = "La phrase est bizarre.";
  const std::string trigger12 = "Montrez-moi une journée avec Olivier Giroud !";
  const std::string reaction12 = "J'ai une vidéo sur le sujet.";
  const std::string trigger13 = "J'ai vu Paul";
  const std::string reaction13 = "Il est sympa.";
  const std::string trigger14 = "Pourquoi Marie ?";
  const std::string reaction14 = "C'est connu.";
  const std::string trigger15 = "C'est quoi la chanson";
  const std::string reaction15 = "Je ne la connais pas.";
  const std::string trigger16 = "Fais-tu le café";
  const std::string reaction16 = "Tu me charries.";
  const std::string trigger17 = "ça va ?";
  const std::string reaction17 = "J'ai mal à la tête.";
  const std::string trigger18 = "Tourner sur toi même";
  const std::string reaction18 = "J'ai le tourni.";
  const std::string trigger19 = "Fais une sieste";
  const std::string reaction19 = "Ok, je dors.";
  const std::string trigger20 = "Au revoir";
  const std::string reaction20 = "Au revoir";

  ONSEM_NOANSWER(triggers_match(whoAreYou, semMem, lingDb));
  ONSEM_NOANSWER(triggers_match(stopApplication, semMem, lingDb));
  ONSEM_NOANSWER(triggers_match(whatTimeItIs, semMem, lingDb));
  ONSEM_NOANSWER(triggers_match(whatAboutWellBeingOfAnimals, semMem, lingDb));
  ONSEM_NOANSWER(triggers_match(whatIsToussaint, semMem, lingDb));
  ONSEM_NOANSWER(triggers_match(whatIsAscension, semMem, lingDb));
  ONSEM_NOANSWER(triggers_match(itLastOneHour, semMem, lingDb));
  ONSEM_NOANSWER(triggers_match(longQuestion, semMem, lingDb));

  triggers_add(whoAreYou, iAmYourFrined, semMem, lingDb);
  triggers_add(stopApplication, itIsStopped, semMem, lingDb);
  triggers_add(whatTimeItIs, itIs15h, semMem, lingDb);
  triggers_add(whatAboutWellBeingOfAnimals, itIsNotFamous, semMem, lingDb);
  triggers_add(whatIsToussaint, itIsAHoliday, semMem, lingDb);
  triggers_add(whatIsAscension, itIsTheLeavingOfJesusOnHeaven, semMem, lingDb);
  triggers_add(itLastOneHour, itIsLong, semMem, lingDb);
  triggers_add(itLastTwoHours, itIsShort, semMem, lingDb);
  triggers_add(whatIs115, itIsSamusocial, semMem, lingDb);
  triggers_add(longQuestion, itIsHardToSay, semMem, lingDb);
  triggers_add(trigger1, reaction1, semMem, lingDb);
  triggers_add(trigger2, reaction2, semMem, lingDb);
  triggers_add(trigger3, reaction3, semMem, lingDb);
  triggers_add(trigger4, reaction4, semMem, lingDb);
  triggers_add(trigger5, reaction5, semMem, lingDb);
  triggers_add(trigger6, reaction6, semMem, lingDb);
  triggers_add(trigger7, reaction7, semMem, lingDb);
  triggers_add(trigger8, reaction8, semMem, lingDb, {}, SemanticLanguageEnum::FRENCH);
  triggers_add(trigger9, reaction9, semMem, lingDb);
  triggers_add(trigger10, reaction10, semMem, lingDb);
  triggers_add(trigger11, reaction11, semMem, lingDb);
  triggers_add(trigger12, reaction12, semMem, lingDb);
  triggers_add(trigger13, reaction13, semMem, lingDb);
  triggers_add(trigger14, reaction14, semMem, lingDb);
  triggers_add(trigger15, reaction15, semMem, lingDb);
  triggers_add(trigger16, reaction16, semMem, lingDb);
  triggers_add(trigger17, reaction17, semMem, lingDb);
  triggers_add(trigger18, reaction18, semMem, lingDb);
  triggers_add(trigger19, reaction19, semMem, lingDb);
  triggers_add(trigger20, reaction20, semMem, lingDb);


  ONSEM_ANSWER_EQ(iAmYourFrined, triggers_match(whoAreYou, semMem, lingDb));
  ONSEM_ANSWER_EQ(iAmYourFrined, triggers_match("peux-tu me dire qui tu es", semMem, lingDb));
  ONSEM_ANSWER_EQ(iAmYourFrined, triggers_match("Pourriez-vous me dire qui vous êtes ?", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ(itIsStopped, triggers_match(stopApplication, semMem, lingDb));
  ONSEM_BEHAVIOR_EQ(itIsStopped, triggers_match("Ferme l'application", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ(itIsStopped, triggers_match("Quitte l'application", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ(itIsStopped, triggers_match("Stoppe l'application", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ(itIsStopped, triggers_match("Interromps l'application", semMem, lingDb));
  ONSEM_ANSWER_EQ(itIs15h, triggers_match(whatTimeItIs, semMem, lingDb));
  ONSEM_ANSWER_EQ(itIs15h, triggers_match("Quelle heure il est ?", semMem, lingDb));
  //ONSEM_ANSWER_EQ(itIs15h, operator_reactFromTrigger("C'est quoi l'heure", semMem, lingDb));
  ONSEM_ANSWER_EQ(itIs15h, triggers_match("Il est quelle heure", semMem, lingDb));
  ONSEM_NOANSWER(triggers_match("C'est tout", semMem, lingDb));
  ONSEM_ANSWER_EQ(itIsNotFamous, triggers_match(whatAboutWellBeingOfAnimals, semMem, lingDb));
  ONSEM_ANSWER_EQ(itIsAHoliday, triggers_match(whatIsToussaint, semMem, lingDb));
  ONSEM_ANSWER_EQ(itIsAHoliday, triggers_match("Qu'est-ce que la toussaint ?", semMem, lingDb));
  ONSEM_ANSWER_EQ(itIsTheLeavingOfJesusOnHeaven, triggers_match(whatIsAscension, semMem, lingDb));
  ONSEM_ANSWER_EQ(itIsTheLeavingOfJesusOnHeaven, triggers_match("Qu'est-ce que l'Ascension ?", semMem, lingDb));
  ONSEM_ANSWER_EQ(itIsLong, triggers_match(itLastOneHour, semMem, lingDb));
  ONSEM_ANSWER_EQ(itIsShort, triggers_match(itLastTwoHours, semMem, lingDb));
  ONSEM_ANSWER_EQ(itIsSamusocial, triggers_match(whatIs115, semMem, lingDb));
  ONSEM_ANSWER_EQ(itIsHardToSay, triggers_match(longQuestion, semMem, lingDb));
  ONSEM_ANSWER_EQ(reaction1, triggers_match(trigger1, semMem, lingDb));
  ONSEM_ANSWER_EQ(reaction2, triggers_match(trigger2, semMem, lingDb));
  ONSEM_ANSWER_EQ(reaction3, triggers_match(trigger3, semMem, lingDb));
  ONSEM_ANSWER_EQ(reaction4, triggers_match(trigger4, semMem, lingDb));
  ONSEM_ANSWER_EQ(reaction5, triggers_match(trigger5, semMem, lingDb));
  ONSEM_ANSWER_EQ(reaction6, triggers_match("les 10 Commandements", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ(reaction7, triggers_match(trigger7, semMem, lingDb));
  ONSEM_BEHAVIOR_EQ(reaction7, triggers_match("remets l'application Aa", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ(reaction8, triggers_match(trigger8, semMem, lingDb, SemanticLanguageEnum::FRENCH));
  ONSEM_BEHAVIOR_EQ(reaction8, triggers_match("Descends ton volume", semMem, lingDb, SemanticLanguageEnum::FRENCH));
  ONSEM_ANSWER_EQ(reaction8, triggers_match("Descends le volume tout de suite", semMem, lingDb, SemanticLanguageEnum::FRENCH));
  ONSEM_ANSWER_EQ(reaction9, triggers_match(trigger9, semMem, lingDb));
  ONSEM_BEHAVIOR_EQ(reaction10, triggers_match(trigger10, semMem, lingDb));
  ONSEM_ANSWER_EQ(reaction11, triggers_match(trigger11, semMem, lingDb));
  ONSEM_BEHAVIOR_EQ(reaction12, triggers_match(trigger12, semMem, lingDb));
  ONSEM_ANSWER_EQ(reaction13, triggers_match(trigger13, semMem, lingDb));
  ONSEM_ANSWER_EQ(reaction14, triggers_match(trigger14, semMem, lingDb));
  ONSEM_ANSWER_EQ(reaction15, triggers_match(trigger15, semMem, lingDb));
  ONSEM_ANSWER_EQ(reaction15, triggers_match("C'est quoi la chanson maintenant", semMem, lingDb));
  ONSEM_ANSWER_EQ(reaction16, triggers_match(trigger16, semMem, lingDb));
  ONSEM_NOANSWER(triggers_match("Comment tu vas faire ?", semMem, lingDb));
  ONSEM_ANSWER_EQ(reaction17, triggers_match(trigger17, semMem, lingDb));
  ONSEM_NOANSWER(triggers_match("Vais-je bien", semMem, lingDb));
  ONSEM_ANSWER_EQ(reaction18, triggers_match(trigger18, semMem, lingDb));
  ONSEM_NOANSWER(triggers_match("toi", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ(reaction19, triggers_match(trigger19, semMem, lingDb));
  ONSEM_BEHAVIOR_EQ(reaction19, triggers_match("Fais dodo", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ(reaction19, triggers_match("Va faire dodo", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ(reaction19, triggers_match("Va faire une sieste", semMem, lingDb));
  ONSEM_ANSWER_EQ(reaction20, triggers_match(trigger20, semMem, lingDb));
  ONSEM_ANSWER_EQ(reaction20, triggers_match("à plus", semMem, lingDb));
}

TEST_F(SemanticReasonerGTests, operator_reactFromTrigger_basic_en)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;

  const std::string trigger1 = "Who are you";
  const std::string reaction1 = "I am your friend.";
  const std::string trigger2 = "Move forward";
  const std::string reaction2 = "I need legs for that.";
  const std::string trigger3 = "Start robotbehav application";
  const std::string reaction3 = "You can make me move now.";
  const std::string trigger4 = "Start the akinator application";
  const std::string reaction4 = "It's a nice mental game.";
  const std::string trigger5 = "open freeze dance";
  const std::string reaction5 = "Why do you want to do that?";
  const std::string trigger6 = "start parle et carte";
  const std::string reaction6 = "It's a nice application.";
  const std::string trigger7 = "Hello robot";
  const std::string reaction7 = "Hello";
  const std::string trigger8 = "What is the volume level?";
  const std::string reaction8 = "My volume level is 80 percents.";
  ONSEM_NOANSWER(triggers_match(trigger1, semMem, lingDb));
  ONSEM_NOANSWER(triggers_match(trigger2, semMem, lingDb));

  triggers_add(trigger1, reaction1, semMem, lingDb);
  triggers_add(trigger2, reaction2, semMem, lingDb);
  triggers_add(trigger3, reaction3, semMem, lingDb);
  triggers_add(trigger4, reaction4, semMem, lingDb);
  triggers_add(trigger5, reaction5, semMem, lingDb);
  triggers_add(trigger6, reaction6, semMem, lingDb, {}, SemanticLanguageEnum::ENGLISH);
  triggers_add(trigger7, reaction7, semMem, lingDb);
  triggers_add(trigger8, reaction8, semMem, lingDb);

  ONSEM_ANSWER_EQ(reaction1, triggers_match(trigger1, semMem, lingDb));
  ONSEM_ANSWER_EQ(reaction1, triggers_match("tell me who you are", semMem, lingDb));
  ONSEM_ANSWER_EQ(reaction1, triggers_match("tell us who you are", semMem, lingDb));
  ONSEM_ANSWER_EQ(reaction1, triggers_match("please tell me who you are", semMem, lingDb));
  ONSEM_ANSWER_EQ(reaction1, triggers_match("can you tell me who you are", semMem, lingDb));
  ONSEM_ANSWER_EQ(reaction1, triggers_match("can you please tell me who you are", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ(reaction2, triggers_match(trigger2, semMem, lingDb));
  ONSEM_BEHAVIOR_EQ(reaction2, triggers_match("Can you move forward", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ(reaction2, triggers_match("could you go forward", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ(reaction3, triggers_match("Start robotbehav", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ(reaction3, triggers_match("Start Robotbehav", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ(reaction3, triggers_match("Start robotbehav application", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ(reaction3, triggers_match("Start Robotbehav application", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ(reaction3, triggers_match("Start the robotbehav", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ(reaction3, triggers_match("Start the Robotbehav", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ(reaction3, triggers_match("Start the robotbehav application", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ(reaction3, triggers_match("Start the Robotbehav application", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ(reaction4, triggers_match(trigger4, semMem, lingDb));
  ONSEM_BEHAVIOR_EQ(reaction4, triggers_match("Start akinator application", semMem, lingDb));
  ONSEM_ANSWER_EQ(reaction5, triggers_match(trigger5, semMem, lingDb));
  ONSEM_BEHAVIOR_EQ(reaction6, triggers_match(trigger6, semMem, lingDb));
  ONSEM_ANSWER_EQ(reaction7, triggers_match(trigger7, semMem, lingDb));
  ONSEM_NOANSWER(triggers_match("Designing robots with emotional understanding requires advanced algorithms.", semMem, lingDb));
  ONSEM_ANSWER_EQ(reaction8, triggers_match(trigger8, semMem, lingDb));
  ONSEM_NOANSWER(triggers_match("What is the level of the radio?", semMem, lingDb));
}


TEST_F(SemanticReasonerGTests, operator_reactFromTrigger_otherLanguage)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;
  auto language = SemanticLanguageEnum::OTHER;

  const std::string trigger1 = "Trigger1 text";
  const std::string reaction1 = "Reaction1 answer";
  triggers_add(trigger1, reaction1, semMem, lingDb, {}, language);
  ONSEM_ANSWER_EQ(reaction1, triggers_match(trigger1, semMem, lingDb, language));
  ONSEM_ANSWER_EQ(reaction1, triggers_match("trigger1    text. ", semMem, lingDb, language));
  ONSEM_ANSWERNOTFOUND_EQ("", triggers_match("Another trigger", semMem, lingDb, language));
}



TEST_F(SemanticReasonerGTests, operator_reactFromTrigger_withParameters_fr)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;
  auto language = SemanticLanguageEnum::FRENCH;

  const std::vector<std::string> howManyMetersParameterQuestion = {"combien de mètres"};
  triggers_addAnswerWithOneParameter("Avance", howManyMetersParameterQuestion, semMem, lingDb, language);
  triggers_addAnswerWithOneParameter("Va vers l'avant", howManyMetersParameterQuestion, semMem, lingDb, language);
  triggers_addAnswerWithOneParameter("Va tout droit", howManyMetersParameterQuestion, semMem, lingDb, language);

  triggers_addAnswerWithOneParameter("Avance un peu", {}, semMem, lingDb, language);
  triggers_addAnswerWithOneParameter("Va un peu vers l'avant", {}, semMem, lingDb, language);
  triggers_addAnswerWithOneParameter("Va un peu tout droit", {}, semMem, lingDb, language);
  triggers_addAnswerWithOneParameter("Avance beaucoup", {}, semMem, lingDb, language);
  triggers_addAnswerWithOneParameter("Va très vers l'avant", {}, semMem, lingDb, language);

  triggers_addAnswerWithOneParameter("Recule", howManyMetersParameterQuestion, semMem, lingDb, language);
  triggers_addAnswerWithOneParameter("Va en arrière", howManyMetersParameterQuestion, semMem, lingDb, language);

  triggers_addAnswerWithOneParameter("Recule un peu", {}, semMem, lingDb, language);
  triggers_addAnswerWithOneParameter("Va un peu en arrière", {}, semMem, lingDb, language);
  triggers_addAnswerWithOneParameter("Recule beaucoup", {}, semMem, lingDb, language);
  triggers_addAnswerWithOneParameter("Va beaucoup en arrière", {}, semMem, lingDb, language);

  const std::vector<std::string> howManyDegreesParameterQuestion = {"combien de degrés"};
  triggers_addAnswerWithOneParameter("Tourne à gauche", howManyDegreesParameterQuestion, semMem, lingDb, language);
  triggers_addAnswerWithOneParameter("Va à gauche", howManyDegreesParameterQuestion, semMem, lingDb, language);
  triggers_addAnswerWithOneParameter("Fais une rotation à gauche", howManyDegreesParameterQuestion, semMem, lingDb, language);

  triggers_addAnswerWithOneParameter("Tourne à droite", howManyDegreesParameterQuestion, semMem, lingDb, language);
  triggers_addAnswerWithOneParameter("Va à droite", howManyDegreesParameterQuestion, semMem, lingDb, language);
  triggers_addAnswerWithOneParameter("Fais une rotation à droite", howManyDegreesParameterQuestion, semMem, lingDb, language);

  triggers_addAnswerWithOneParameter("Tourne la tête à gauche", howManyDegreesParameterQuestion, semMem, lingDb, language);
  triggers_addAnswerWithOneParameter("Tourne la tête à droite", howManyDegreesParameterQuestion, semMem, lingDb, language);
  triggers_addAnswerWithOneParameter("Regarde à gauche", howManyDegreesParameterQuestion, semMem, lingDb, language);
  triggers_addAnswerWithOneParameter("Regarde à droite", howManyDegreesParameterQuestion, semMem, lingDb, language);

  triggers_addAnswerWithOneParameter("Lève la tête", howManyDegreesParameterQuestion, semMem, lingDb, language);
  triggers_addAnswerWithOneParameter("Regarde", howManyDegreesParameterQuestion, semMem, lingDb, language);
  triggers_addAnswerWithOneParameter("Regarde en haut", howManyDegreesParameterQuestion, semMem, lingDb, language);
  triggers_addAnswerWithOneParameter("Regarde en l'air", howManyDegreesParameterQuestion, semMem, lingDb, language);

  triggers_addAnswerWithOneParameter("Baisse la tête", howManyDegreesParameterQuestion, semMem, lingDb, language);

  const std::vector<std::string> howMuchInPercentageParameterQuestions = {"combien en pourcentage", "combien"};
  triggers_addAnswerWithOneParameter("Descends le volume", howMuchInPercentageParameterQuestions, semMem, lingDb, language);
  triggers_addAnswerWithOneParameter("Diminue le volume", howMuchInPercentageParameterQuestions, semMem, lingDb, language);

  triggers_addAnswerWithOneParameter("Mets le volume moins fort", howMuchInPercentageParameterQuestions, semMem, lingDb, language);
  triggers_addAnswerWithOneParameter("Parle moins fort", howMuchInPercentageParameterQuestions, semMem, lingDb, language);

  triggers_addAnswerWithOneParameter("Monte le volume", howMuchInPercentageParameterQuestions, semMem, lingDb, language);
  triggers_addAnswerWithOneParameter("Augmente le volume", howMuchInPercentageParameterQuestions, semMem, lingDb, language);

  triggers_addAnswerWithOneParameter("Mets le volume plus fort", howMuchInPercentageParameterQuestions, semMem, lingDb, language);
  triggers_addAnswerWithOneParameter("Parle plus fort", howMuchInPercentageParameterQuestions, semMem, lingDb, language);

  triggers_addAnswerWithOneParameter("Mets le volume", howMuchInPercentageParameterQuestions, semMem, lingDb, language);

  const std::vector<std::string> whatQuestion = {"quoi"};
  triggers_addAnswerWithOneParameter("Lance", whatQuestion, semMem, lingDb, language);

  const std::vector<std::string> whereQuestion = {"où"};
  triggers_addAnswerWithOneParameter("Tourne sans t'arrêter", whereQuestion, semMem, lingDb, language);
  triggers_addAnswerWithOneParameter("Tourne en continu", whereQuestion, semMem, lingDb, language);
  triggers_addAnswerWithOneParameter("Tourne sans arrêt", whereQuestion, semMem, lingDb, language);
  triggers_addAnswerWithOneParameter("fais la toupie", whereQuestion, semMem, lingDb, language);

  ONSEM_ANSWERNOTFOUND_EQ("", triggers_match("fais un saut périeux", semMem, lingDb));
  ONSEM_ANSWERNOTFOUND_EQ("", triggers_match("fous de ma gueule", semMem, lingDb));

  const std::vector<std::string> howQuestion = {"comment"};
  triggers_addAnswerWithOneParameter("Fais demi tour", howQuestion, semMem, lingDb, language);
  triggers_addAnswerWithOneParameter("Demi-tour", howQuestion, semMem, lingDb, language);
  triggers_addAnswerWithOneParameter("Tourne toi", howQuestion, semMem, lingDb, language);

  std::map<std::string, std::vector<std::string>> turnParameters {
    {"location", {"où"}}, {"speed", {"comment"}}, {"nbOfTimes", {"combien de fois"}}
  };
  triggers_addAnswerWithManyParameters("Fais un tour complet", turnParameters, semMem, lingDb, language);
  triggers_addAnswerWithManyParameters("Fais une pirouette", turnParameters, semMem, lingDb, language);
  triggers_addAnswerWithManyParameters("Tourne sur toi même", turnParameters, semMem, lingDb, language);
  triggers_addAnswerWithManyParameters("Fais un tour sur toi même", turnParameters, semMem, lingDb, language);
  triggers_addAnswerWithManyParameters("un tour sur toi même", turnParameters, semMem, lingDb, language);
  triggers_addAnswerWithManyParameters("Fais un 360", turnParameters, semMem, lingDb, language);

  const std::vector<std::string> howLongInMinutesQuestion = {"pendant combien de minutes"};
  triggers_addAnswerWithOneParameter("Éteins-toi", howLongInMinutesQuestion, semMem, lingDb, language);
  triggers_addAnswerWithOneParameter("Tu peux t'éteindre", howLongInMinutesQuestion, semMem, lingDb, language);
  triggers_addAnswerWithOneParameter("Tu t'éteins", howLongInMinutesQuestion, semMem, lingDb, language);
  triggers_addAnswerWithOneParameter("Ferme-toi", howLongInMinutesQuestion, semMem, lingDb, language);

  triggers_addAnswerWithOneParameter("Arrête de bouger", howLongInMinutesQuestion, semMem, lingDb, language);
  triggers_addAnswerWithOneParameter("Ne bouge pas", howLongInMinutesQuestion, semMem, lingDb, language);
  triggers_addAnswerWithOneParameter("Arrête-toi ici", howLongInMinutesQuestion, semMem, lingDb, language);
  triggers_addAnswerWithOneParameter("Reste ici", howLongInMinutesQuestion, semMem, lingDb, language);
  triggers_addAnswerWithOneParameter("Arrête-toi où tu es", howLongInMinutesQuestion, semMem, lingDb, language);
  triggers_addAnswerWithOneParameter("Reste où tu es", howLongInMinutesQuestion, semMem, lingDb, language);
  triggers_addAnswerWithOneParameter("Ferme-toi", howLongInMinutesQuestion, semMem, lingDb, language);
  triggers_addAnswerWithOneParameter("Reste immobile", howLongInMinutesQuestion, semMem, lingDb, language);

  triggers_addAnswerWithOneParameter("Raconte une blague", {}, semMem, lingDb, language);
  triggers_addAnswerWithOneParameter("Qui es tu ?", {}, semMem, lingDb, language);
  triggers_addAnswerWithOneParameter("C'est quoi ton nom ?", {}, semMem, lingDb, language);
  triggers_addAnswerWithOneParameter("Sais-tu faire le café ?", {}, semMem, lingDb, language);
  triggers_addAnswerWithOneParameter("Peux-tu donner l'heure", {}, semMem, lingDb, language);
  triggers_addAnswerWithOneParameter("une balade", {}, semMem, lingDb, language);


  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Avance(param1=3 mètres)\\", triggers_match("Avance 3 mètres", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Avance(param1=5 mètres)\\", triggers_match("avance cinq mètres", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Avance(param1=4 mètres)\\", triggers_match("Avance de 4 mètres", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Avance(param1=1 mètre)\\", triggers_match("Avance d'un mètre", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Avance(param1=2 mètres)\\", triggers_match("Avance de deux mètres", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Avance(param1=0,7 mètre)\\", triggers_match("Avance de 0,7 m", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Avance(param1=0,1 mètre)\\", triggers_match("Je veux que tu avances de 10 cm", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Avance(param1=0,1 mètre)\\", triggers_match("J'aimerais que tu avances de 10 cm", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Avance(param1=0,3 mètre)\\", triggers_match("Avance de 30 centimètres", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Avance(param1=0,4 mètre)\\", triggers_match("Avance de 40 cm", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Va vers l'avant(param1=5 mètres)\\", triggers_match("Va vers l'avant 5 mètres", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Va vers l'avant(param1=6 mètres)\\", triggers_match("Va vers l'avant de 6 mètres", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Va tout droit\\", triggers_match("Va tout droit", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Va tout droit(param1=7 mètres)\\", triggers_match("Va tout droit 7 mètres", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Va tout droit(param1=8 mètres)\\", triggers_match("Va tout droit de 8 mètres", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Avance(param1=7 mètres)\\", triggers_match("Je veux que tu avances de 7 mètres", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Avance\\", triggers_match("Avance", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Va vers l'avant\\", triggers_match("Va vers l'avant", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Avance un peu\\", triggers_match("Avance un peu", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Avance beaucoup\\", triggers_match("Avance beaucoup", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Avance beaucoup\\", triggers_match("Avance énormément", semMem, lingDb));

  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Recule(param1=2 mètres)\\", triggers_match("Recule de 2 mètres", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Va en arrière\\", triggers_match("Va en arrière", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Va en arrière(param1=0,7 mètre)\\", triggers_match("Va en arrière 0,7 mètre", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Va en arrière(param1=0,35 mètre)\\", triggers_match("Va en arrière trente cinq centimètres", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Va en arrière(param1=0,5 mètre)\\", triggers_match("Va 50 centimètres en arrière", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Va un peu en arrière\\", triggers_match("Va un peu en arrière", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Va un peu en arrière\\", triggers_match("Va un petit peu en arrière", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Recule beaucoup\\", triggers_match("Recule beaucoup", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Va beaucoup en arrière\\", triggers_match("Va beaucoup en arrière", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Va beaucoup en arrière\\", triggers_match("Va très en arrière", semMem, lingDb));

  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Tourne à gauche(param1=34 degrés)\\", triggers_match("Tourne à gauche de 34 degrés", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Tourne à droite(param1=37 degrés)\\", triggers_match("Tourne à droite de 37 degrés", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Va à droite(param1=42 degrés)\\", triggers_match("Va 42 degrés à droite", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Va à droite(param1=45 degrés)\\", triggers_match("Va à droite de 45 degrés", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Fais une rotation à droite(param1=23 degrés)\\", triggers_match("Fais une rotation à droite de 23 degrés", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Tourne à gauche(param1=25 degrés)\\", triggers_match("Tourne à gauche de vingt cinq degrés", semMem, lingDb));

  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Tourne la tête à gauche\\", triggers_match("Tourne la tête à gauche", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Tourne la tête à gauche(param1=38 degrés)\\", triggers_match("Tourne la tête à gauche de trente huit degrés", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Regarde à gauche\\", triggers_match("Regarde à gauche", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Regarde à gauche\\", triggers_match("Regarde vers la gauche", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Regarde à gauche\\", triggers_match("Regarde sur la gauche", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Tourne la tête à droite\\", triggers_match("Tourne la tête à droite", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Tourne la tête à droite(param1=25 degrés)\\", triggers_match("Tourne la tête à droite de vingt cinq degrés", semMem, lingDb));

  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Lève la tête\\", triggers_match("Lève la tête", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Lève la tête\\", triggers_match("Monte la tête", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Lève la tête(param1=11 degrés)\\", triggers_match("Monte la tête de onze degrés", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Regarde\\", triggers_match("regarde", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Regarde en haut\\", triggers_match("Regarde en haut", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Regarde en l'air\\", triggers_match("Regarde vers le haut", semMem, lingDb));

  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Baisse la tête\\", triggers_match("Descends la tête", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Baisse la tête(param1=29 degrés)\\", triggers_match("descends la tête de vingt neuf degrés", semMem, lingDb));

  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Descends le volume\\", triggers_match("Descends ton volume", semMem, lingDb, language));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Descends le volume(param1=30 pour cent)\\", triggers_match("Descends le volume de 30 %", semMem, lingDb, language));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Descends le volume(param1=31 pour cent)\\", triggers_match("Descends ton volume de 31 %", semMem, lingDb, language));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Diminue le volume(param1=32 pour cent)\\", triggers_match("Je veux que tu diminues le son de 32 %", semMem, lingDb, language));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Mets le volume moins fort\\", triggers_match("Mets le volume moins fort", semMem, lingDb, language));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Mets le volume moins fort\\", triggers_match(("Mets le volume encore moins fort"), semMem, lingDb, language));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Parle plus fort\\", triggers_match(("parle encore plus fort"), semMem, lingDb, language));

  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Monte le volume(param1=12 pour cent)\\", triggers_match("Monte le volume de 12 %", semMem, lingDb, language));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Mets le volume(param1=90 pour cent)\\", triggers_match("Mets le volume à 90 %", semMem, lingDb, language));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Mets le volume(param1=80)\\", triggers_match("Mets le volume à 80", semMem, lingDb, language));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Mets le volume(param1=91 pour cent)\\", triggers_match("Mets le volume à quatre vingt onze pour cent", semMem, lingDb, language));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Mets le volume plus fort\\", triggers_match("Mets le volume plus fort", semMem, lingDb, language));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Mets le volume plus fort\\", triggers_match("Mets le volume encore plus fort", semMem, lingDb, language));

  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Lance(param1=Akinator)\\", triggers_match("Lance akinator", semMem, lingDb));

  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Tourne sans t'arrêter\\", triggers_match("Tourne sans t'arrêter", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Tourne sans t'arrêter(param1=Gauche)\\", triggers_match("Tourne sans t'arrêter à gauche", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Tourne en continu\\", triggers_match("Tourne en continu", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Tourne en continu(param1=Droite)\\", triggers_match("tourne en continu à droite", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Tourne sans arrêt\\", triggers_match("tourne sans arrêt", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Tourne sans arrêt(param1=Gauche)\\", triggers_match("Tourne sans arrêt à gauche", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#fais la toupie\\", triggers_match("fais la toupie", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#fais la toupie(param1=Droite)\\", triggers_match("Fais la toupie à droite", semMem, lingDb));

  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Tourne toi\\", triggers_match("Tourne toi", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Tourne toi(param1=Rapidement)\\", triggers_match("Tourne toi rapidement", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Tourne toi\\", triggers_match("Retourne toi", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Tourne toi(param1=Lentement)\\", triggers_match("Retourne toi lentement", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Tourne toi(param1=Rapidement)\\", triggers_match("Retourne toi rapidement", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Fais demi tour(param1=Lentement)\\", triggers_match("Fais lentement demi tour", semMem, lingDb));

  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Fais un tour complet(nbOfTimes=1)\\", triggers_match("Fais un tour complet", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Fais un tour complet(nbOfTimes=2)\\", triggers_match("Fais un tour complet 2 fois", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Fais un tour complet(nbOfTimes=1, speed=Vite)\\", triggers_match("Fais un tour complet vite", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Fais un tour complet(nbOfTimes=1, speed=Rapidement)\\", triggers_match("Fais un tour complet rapidement", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Fais un tour complet(location=Droite, nbOfTimes=1)\\", triggers_match("Fais un tour complet à droite", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Tourne sur toi même(location=Sur toi, nbOfTimes=1)\\", triggers_match("Tourne sur toi même", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Fais une pirouette(nbOfTimes=1)\\", triggers_match("Fais une pirouette", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Fais un tour sur toi même(location=Sur toi, nbOfTimes=1)\\", triggers_match("Fais un tour sur toi même", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Fais un 360(nbOfTimes=1)\\", triggers_match("Fais un 360", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Fais un 360(nbOfTimes=1, speed=Lentement)\\", triggers_match("Fais un 360 lentement", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Fais un 360(nbOfTimes=3, speed=Lentement)\\", triggers_match("Fais un 360 lentement 3 fois", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Fais un 360(location=Droite, nbOfTimes=4, speed=Vite)\\", triggers_match("Fais un 360 à droite vite 4 fois", semMem, lingDb));

  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Éteins-toi\\", triggers_match("éteins toi", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Éteins-toi\\", triggers_match("Éteins-toi", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Éteins-toi(param1=3 minutes)\\", triggers_match("Éteins-toi pendant 3 minutes", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Éteins-toi(param1=120 minutes)\\", triggers_match("Éteins-toi pendant 2 heures", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Éteins-toi(param1=60 minutes)\\", triggers_match("éteins toi une heure", semMem, lingDb));
  ONSEM_ANSWER_EQ("\\label=#fr_FR#Tu peux t'éteindre\\", triggers_match("Tu peux t'éteindre", semMem, lingDb));
  ONSEM_ANSWER_EQ("\\label=#fr_FR#Tu t'éteins\\", triggers_match("Tu t'éteins", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Ferme-toi\\", triggers_match("Ferme-toi", semMem, lingDb));

  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Arrête de bouger\\", triggers_match("arrête de bouger", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Ne bouge pas\\", triggers_match("Ne bouge pas", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Ne bouge pas\\", triggers_match("Ne bouge plus", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Reste ici\\", triggers_match("reste ici", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Reste ici(param1=120 minutes)\\", triggers_match("reste là pendant 2 heures", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Reste où tu es\\", triggers_match("reste où tu es", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Arrête-toi ici\\", triggers_match("arrête toi ici", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Arrête-toi ici\\", triggers_match("arrête toi là", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Arrête-toi où tu es\\", triggers_match("arrête toi où tu es", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Reste immobile(param1=5 minutes)\\", triggers_match("reste immobile 5 minutes", semMem, lingDb));

  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Raconte une blague\\", triggers_match("raconte    une autre histoire drôle", semMem, lingDb));
  ONSEM_NOANSWER(triggers_match("Dis moi quel robot es-tu ?", semMem, lingDb));
  ONSEM_ANSWER_EQ("\\label=#fr_FR#Qui es tu ?\\", triggers_match("Pourriez-vous me dire qui vous êtes ?", semMem, lingDb));
  ONSEM_NOANSWER(triggers_match("Que ne sais-tu pas faire ?", semMem, lingDb));
  ONSEM_NOANSWER(triggers_match("Quel est mon nom ?", semMem, lingDb));
  ONSEM_NOANSWER(triggers_match("Ne me dis rien", semMem, lingDb));
  ONSEM_NOANSWER(triggers_match("Pouvez-vous vous présenter ?", semMem, lingDb));
  ONSEM_ANSWER_EQ("\\label=#fr_FR#une balade\\",  triggers_match("En avant pour la balade", semMem, lingDb));
  ONSEM_NOANSWER(triggers_match("Baisse ta lampe.", semMem, lingDb));
  ONSEM_NOANSWER(triggers_match("tu peux   nous donner   l'heure", semMem, lingDb));
  ONSEM_NOANSWER(triggers_match("mets", semMem, lingDb));
}


TEST_F(SemanticReasonerGTests, operator_reactFromTrigger_withParameters_en)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;
  auto language = SemanticLanguageEnum::ENGLISH;

  const std::vector<std::string> howFarInMetersParameterQuestion = {"how far in meters"};
  triggers_addAnswerWithOneParameter("Advance", howFarInMetersParameterQuestion,
                                     semMem, lingDb, language);

  triggers_addAnswerWithOneParameter("Move forward a little", {""},
                                     semMem, lingDb, language);

  triggers_addAnswerWithOneParameter("Move forward a lot", {""},
                                     semMem, lingDb, language);

  triggers_addAnswerWithOneParameter("Move backward", howFarInMetersParameterQuestion,
                                     semMem, lingDb, language);

  triggers_addAnswerWithOneParameter("Move backward a little", {},
                                     semMem, lingDb, language);
  triggers_addAnswerWithOneParameter("Move backward a lot", {},
                                     semMem, lingDb, language);

  const std::vector<std::string> howFarInDegreesParameterQuestion = {"how far in degrees"};
  triggers_addAnswerWithOneParameter("Turn left", {howFarInDegreesParameterQuestion},
                                     semMem, lingDb, language);
  triggers_addAnswerWithOneParameter("Turn on your left", {howFarInDegreesParameterQuestion},
                                     semMem, lingDb, language);
  triggers_addAnswerWithOneParameter("Turn right", {howFarInDegreesParameterQuestion},
                                     semMem, lingDb, language);
  triggers_addAnswerWithOneParameter("Turn on your right", {howFarInDegreesParameterQuestion},
                                     semMem, lingDb, language);

  triggers_addAnswerWithOneParameter("Turn the head to the right", {howFarInDegreesParameterQuestion},
                                     semMem, lingDb, language);
  triggers_addAnswerWithOneParameter("Turn the head to the left", {howFarInDegreesParameterQuestion},
                                     semMem, lingDb, language);

  triggers_addAnswerWithOneParameter("Head up", howFarInDegreesParameterQuestion, semMem, lingDb, language);
  triggers_addAnswerWithOneParameter("Head down", howFarInDegreesParameterQuestion, semMem, lingDb, language);

  const std::vector<std::string> howMuchInPercentageParameterQuestion = {"how many in percentage", "how many"};
  triggers_addAnswerWithOneParameter("Lower the volume", howMuchInPercentageParameterQuestion, semMem, lingDb, language);
  triggers_addAnswerWithOneParameter("Lower the voice", howMuchInPercentageParameterQuestion, semMem, lingDb, language);

  triggers_addAnswerWithOneParameter("Turn up the volume", howMuchInPercentageParameterQuestion, semMem, lingDb, language);
  triggers_addAnswerWithOneParameter("Set the volume", howMuchInPercentageParameterQuestion, semMem, lingDb, language);

  const std::vector<std::string> whatQuestion = {"what"};
  triggers_addAnswerWithOneParameter("Launch", whatQuestion, semMem, lingDb, language);

  triggers_addAnswerWithOneParameter("can you look up", {}, semMem, lingDb, language);

  const std::vector<std::string> whereQuestion = {"where"};
  triggers_addAnswerWithOneParameter("Turn without stopping", whereQuestion, semMem, lingDb, language);
  triggers_addAnswerWithOneParameter("Turn continuously", whereQuestion, semMem, lingDb, language);

  const std::vector<std::string> howQuestion = {"how"};
  triggers_addAnswerWithOneParameter("Rotate", howQuestion, semMem, lingDb, language);
  triggers_addAnswerWithOneParameter("Turn around", howQuestion, semMem, lingDb, language);

  std::map<std::string, std::vector<std::string>> turnParameters {
    {"location", {"where"}}, {"speed", {"how"}}, {"nbOfTimes", {"how many times"}}
  };
  triggers_addAnswerWithManyParameters("Make a full turn", turnParameters, semMem, lingDb, language);
  triggers_addAnswerWithManyParameters("Do a 360", turnParameters, semMem, lingDb, language);
  triggers_addAnswerWithManyParameters("Pirouette", turnParameters, semMem, lingDb, language);
  triggers_addAnswerWithManyParameters("Take a spin on yourself", turnParameters, semMem, lingDb, language);

  const std::vector<std::string> howManyMinutesQuestion = {"how many minutes"};
  triggers_addAnswerWithOneParameter("Shut down", howManyMinutesQuestion, semMem, lingDb, language);
  triggers_addAnswerWithOneParameter("You can turn off", howManyMinutesQuestion, semMem, lingDb, language);
  triggers_addAnswerWithOneParameter("You turn off", howManyMinutesQuestion, semMem, lingDb, language);
  triggers_addAnswerWithOneParameter("Turn off", howManyMinutesQuestion, semMem, lingDb, language);

  triggers_addAnswerWithOneParameter("Do not move", howManyMinutesQuestion, semMem, lingDb, language);
  triggers_addAnswerWithOneParameter("Stop moving", howManyMinutesQuestion, semMem, lingDb, language);
  triggers_addAnswerWithOneParameter("fine thank you", {}, semMem, lingDb, language);

  ONSEM_BEHAVIOR_EQ("\\label=#en_US#Advance(param1=3 meters)\\", triggers_match("Move forward 3 meters", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#en_US#Advance(param1=0.2 meter)\\", triggers_match("move forward 20 cm", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#en_US#Advance(param1=0.2 meter)\\", triggers_match("move forward 20 centimeters", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#en_US#Advance(param1=2 meters)\\", triggers_match("Go forward two meters", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#en_US#Advance(param1=3 meters)\\", triggers_match("Advance 3 meters", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#en_US#Move forward a little\\", triggers_match("can you go forward a little", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#en_US#Move forward a little\\", triggers_match("could you go forward a little", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#en_US#Move backward(param1=3 meters)\\", triggers_match("Move backward 3 meters", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#en_US#Move backward(param1=0.4 meter)\\", triggers_match("move backward 40 centimeters", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#en_US#Move backward(param1=1 meter)\\", triggers_match("Go backward one meter", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#en_US#Move backward a little\\", triggers_match("Move backward a little", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#en_US#Move backward a little\\", triggers_match("Move backward a bit", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#en_US#Move backward a lot\\", triggers_match("Move backward a lot", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#en_US#Turn left(param1=32 degrees)\\", triggers_match("Turn left 32 degrees", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#en_US#Turn right(param1=33 degrees)\\", triggers_match("turn right 33 degrees", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#en_US#Turn the head to the right(param1=34 degrees)\\", triggers_match("Turn the head to the right 34 degrees", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#en_US#Turn the head to the left(param1=41 degrees)\\", triggers_match("Turn the head to the left 41 degrees", semMem, lingDb));

  ONSEM_BEHAVIOR_EQ("\\label=#en_US#Head up\\", triggers_match("Head up", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#en_US#Head up\\", triggers_match("Raise the head", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#en_US#Head up\\", triggers_match("Raise your head", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#en_US#Head up(param1=24 degrees)\\", triggers_match("Head up 24 degrees", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#en_US#Head up(param1=43 degrees)\\", triggers_match("Raise the head 43 degrees", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#en_US#Head up(param1=43 degrees)\\", triggers_match("Raise your head 43 degrees", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#en_US#Head down\\", triggers_match("Head down", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#en_US#Head down\\", triggers_match("Lower your head", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#en_US#Head down(param1=25 degrees)\\", triggers_match("Head down 25 degrees", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#en_US#Head down(param1=23 degrees)\\", triggers_match("Lower your head 23 degrees", semMem, lingDb));

  ONSEM_BEHAVIOR_EQ("\\label=#en_US#Lower the volume\\", triggers_match("Lower the volume", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#en_US#Lower the voice\\", triggers_match("Lower your voice", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#en_US#Lower the volume\\", triggers_match("Turn down the volume", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#en_US#Lower the volume\\", triggers_match("Turn the volume down", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#en_US#Lower the volume\\", triggers_match("Reduce the volume level.", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#en_US#Lower the volume\\", triggers_match("Decrease the sound level", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#en_US#Lower the volume(param1=29 percents)\\", triggers_match("Turn down the volume by 29 %", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#en_US#Lower the volume(param1=28)\\", triggers_match("Turn down the volume by 28", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#en_US#Lower the volume(param1=30 percents)\\", triggers_match("Lower the volume by 30%", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#en_US#Lower the volume(param1=31 percents)\\", triggers_match("Lower your volume by 31%", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#en_US#Lower the voice(param1=28 percents)\\", triggers_match("Lower the voice by 28%", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#en_US#Lower the voice(param1=29 percents)\\", triggers_match("Lower your voice by 29%", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#en_US#Turn up the volume\\", triggers_match("Turn up the volume", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#en_US#Turn up the volume(param1=31 percents)\\", triggers_match("Turn up the volume by 31%", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#en_US#Set the volume\\", triggers_match("Set the volume", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#en_US#Set the volume(param1=54 percents)\\", triggers_match("Set the volume to 54%", semMem, lingDb));

  ONSEM_BEHAVIOR_EQ("\\label=#en_US#Launch(param1=Akinator)\\", triggers_match("Launch akinator", semMem, lingDb));
  ONSEM_ANSWER_EQ("\\label=#en_US#can you look up\\", triggers_match("Can you look up", semMem, lingDb));
  ONSEM_ANSWER_EQ("\\label=#en_US#can you look up\\", triggers_match("Can you please look up", semMem, lingDb));
  ONSEM_ANSWER_EQ("\\label=#en_US#can you look up\\", triggers_match("Could you look up", semMem, lingDb));
  ONSEM_ANSWER_EQ("\\label=#en_US#can you look up\\", triggers_match("Could you please look up", semMem, lingDb));

  ONSEM_BEHAVIOR_EQ("\\label=#en_US#Turn without stopping\\", triggers_match("Turn without stopping", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#en_US#Turn without stopping(param1=To the left)\\", triggers_match("Turn without stopping to the left", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#en_US#Turn continuously\\", triggers_match("Turn continuously", semMem, lingDb));

  ONSEM_BEHAVIOR_EQ("\\label=#en_US#Rotate\\", triggers_match("Rotate", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#en_US#Rotate(param1=Slowly)\\", triggers_match("Rotate slowly", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#en_US#Turn around\\", triggers_match("Turn around", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#en_US#Turn around(param1=Quickly)\\", triggers_match("Turn around quickly", semMem, lingDb));

  ONSEM_BEHAVIOR_EQ("\\label=#en_US#Make a full turn(nbOfTimes=1)\\", triggers_match("Make a full turn", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#en_US#Make a full turn(nbOfTimes=1)\\", triggers_match("Make a complete turn", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#en_US#Make a full turn(nbOfTimes=1, speed=Quickly)\\", triggers_match("Make a full turn quickly", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#en_US#Make a full turn(nbOfTimes=2, speed=Quickly)\\", triggers_match("Make a full turn quickly 2 times", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#en_US#Make a full turn(location=The left, nbOfTimes=3, speed=Slowly)\\", triggers_match("Make a full turn to the left slowly 3 times", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#en_US#Make a full turn(nbOfTimes=3)\\", triggers_match("Make a full turn 3 times", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#en_US#Do a 360(nbOfTimes=1)\\", triggers_match("Do a 360", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#en_US#Pirouette(nbOfTimes=1)\\", triggers_match("Pirouette", semMem, lingDb, language));
  ONSEM_BEHAVIOR_EQ("\\label=#en_US#Pirouette(location=The left, nbOfTimes=1)\\", triggers_match("Pirouette to the left", semMem, lingDb, language));
  ONSEM_BEHAVIOR_EQ("\\label=#en_US#Take a spin on yourself(location=On you, nbOfTimes=1)\\", triggers_match("Take a spin on yourself", semMem, lingDb));

  ONSEM_BEHAVIOR_EQ("\\label=#en_US#Shut down\\", triggers_match("Shut down", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#en_US#Shut down(param1=4 minutes)\\", triggers_match("Shut down for 4 minutes", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#en_US#Shut down(param1=5 minutes)\\", triggers_match("Shut down during 5 minutes", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#en_US#Shut down(param1=3 minutes)\\", triggers_match("Shut down 3 minutes", semMem, lingDb));
  ONSEM_ANSWER_EQ("\\label=#en_US#You can turn off\\", triggers_match("You can turn off", semMem, lingDb));
  ONSEM_ANSWER_EQ("\\label=#en_US#You turn off\\", triggers_match("You turn off", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#en_US#Turn off\\", triggers_match("Turn off", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#en_US#Turn off(param1=0.5 minute)\\", triggers_match("Turn off 30 seconds", semMem, lingDb));

  ONSEM_BEHAVIOR_EQ("\\label=#en_US#Do not move\\", triggers_match("Do not move", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#en_US#Do not move(param1=5 minutes)\\", triggers_match("Do not move 5 minutes", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#en_US#Stop moving\\", triggers_match("Stop moving", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#en_US#Stop moving(param1=240 minutes)\\", triggers_match("Stop moving during 4 hours", semMem, lingDb));
  ONSEM_NOANSWER(triggers_match("Understanding emotions in robotics", semMem, lingDb));
  ONSEM_NOANSWER(triggers_match("Hi there!", semMem, lingDb));
}


TEST_F(SemanticReasonerGTests, operator_reactFromTrigger_mergeWithContext_fr)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;
  auto language = SemanticLanguageEnum::FRENCH;

  auto superAnswer = "Super";
  triggers_add("Comment je peux t'apprendre un comportement ?", superAnswer, semMem, lingDb);
  triggers_add("Je ne veux pas savoir comment je peux t'apprendre un comportement", "D'accord je ne t'embêterai plus avec ça", semMem, lingDb);

  operator_inform_fromRobot("Est-ce que tu veux savoir comment tu peux m'apprendre un comportement ?", semMem, lingDb);
  ONSEM_ANSWER_EQ(superAnswer, operator_react("oui", semMem, lingDb));
}

