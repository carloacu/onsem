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
    const ReactionOptions* pReactionOptions)
{
  SemanticLanguageEnum textLanguage = pTextLanguage == SemanticLanguageEnum::UNKNOWN ?
      linguistics::getLanguage(pText, pLingDb) : pTextLanguage;
  TextProcessingContext inContext(SemanticAgentGrounding::currentUser,
                                  SemanticAgentGrounding::me,
                                  textLanguage);
  auto semExp =
      converter::textToContextualSemExp(pText, inContext,
                                        SemanticSourceEnum::ASR, pLingDb);
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

  ONSEM_ANSWER_EQ(iAmYourFrined, triggers_match(whoAreYou, semMem, lingDb));
  ONSEM_BEHAVIOR_EQ(itIsStopped, triggers_match(stopApplication, semMem, lingDb));
  ONSEM_BEHAVIOR_EQ(itIsStopped, triggers_match("Ferme l'application", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ(itIsStopped, triggers_match("Quitte l'application", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ(itIsStopped, triggers_match("Stoppe l'application", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ(itIsStopped, triggers_match("Interromps l'application", semMem, lingDb));
  ONSEM_ANSWER_EQ(itIs15h, triggers_match(whatTimeItIs, semMem, lingDb));
  ONSEM_ANSWER_EQ(itIs15h, triggers_match("Quelle heure il est ?", semMem, lingDb));
  //ONSEM_ANSWER_EQ(itIs15h, operator_reactFromTrigger("C'est quoi l'heure", semMem, lingDb));
  //ONSEM_ANSWER_EQ(itIs15h, operator_reactFromTrigger("Il est quelle heure", semMem, lingDb));
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
  ONSEM_ANSWER_EQ(reaction9, triggers_match(trigger9, semMem, lingDb));
  ONSEM_BEHAVIOR_EQ(reaction10, triggers_match(trigger10, semMem, lingDb));
  ONSEM_ANSWER_EQ(reaction11, triggers_match(trigger11, semMem, lingDb));
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
  ONSEM_NOANSWER(triggers_match(trigger1, semMem, lingDb));
  ONSEM_NOANSWER(triggers_match(trigger2, semMem, lingDb));

  triggers_add(trigger1, reaction1, semMem, lingDb);
  triggers_add(trigger2, reaction2, semMem, lingDb);
  triggers_add(trigger3, reaction3, semMem, lingDb);
  triggers_add(trigger4, reaction4, semMem, lingDb);
  triggers_add(trigger5, reaction5, semMem, lingDb);
  triggers_add(trigger6, reaction6, semMem, lingDb, {}, SemanticLanguageEnum::ENGLISH);

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

  const std::vector<std::string> advanceParameterQuestions =
  {"De combien dois-je avancer en mètres ?",
   "De combien dois-je aller vers l'avant en mètres ?",
   "De combien dois-je aller tout droit en mètres ?"};
  triggers_addAnswerWithOneParameter("Avance", advanceParameterQuestions, semMem, lingDb, language);
  triggers_addAnswerWithOneParameter("Va vers l'avant", advanceParameterQuestions, semMem, lingDb, language);
  triggers_addAnswerWithOneParameter("Va tout droit", advanceParameterQuestions, semMem, lingDb, language);

  triggers_addAnswerWithOneParameter("Avance un peu", {}, semMem, lingDb, language);
  triggers_addAnswerWithOneParameter("Va un peu vers l'avant", {}, semMem, lingDb, language);
  triggers_addAnswerWithOneParameter("Va un peu tout droit", {}, semMem, lingDb, language);
  triggers_addAnswerWithOneParameter("Avance beaucoup", {}, semMem, lingDb, language);
  triggers_addAnswerWithOneParameter("Va très vers l'avant", {}, semMem, lingDb, language);

  const std::vector<std::string> moveBackParameterQuestions =
  {"De combien dois-je reculer en mètres ?",
   "De combien dois-je aller en arrière en mètres ?"};
  triggers_addAnswerWithOneParameter("Recule", moveBackParameterQuestions, semMem, lingDb, language);
  triggers_addAnswerWithOneParameter("Va en arrière", moveBackParameterQuestions, semMem, lingDb, language);

  triggers_addAnswerWithOneParameter("Recule un peu", {}, semMem, lingDb, language);
  triggers_addAnswerWithOneParameter("Va un peu en arrière", {}, semMem, lingDb, language);
  triggers_addAnswerWithOneParameter("Va un petit peu en arrière", {}, semMem, lingDb, language);
  triggers_addAnswerWithOneParameter("Recule beaucoup", {}, semMem, lingDb, language);
  triggers_addAnswerWithOneParameter("Va beaucoup en arrière", {}, semMem, lingDb, language);
  triggers_addAnswerWithOneParameter("Va très en arrière", {}, semMem, lingDb, language);

  const std::vector<std::string> turnParameterQuestions =
  {"De combien dois-je tourner en degré ?",
   "De combien dois-je faire une rotation en degré ?"};
  triggers_addAnswerWithOneParameter("Tourne à gauche", turnParameterQuestions, semMem, lingDb, language);
  triggers_addAnswerWithOneParameter("Va à gauche", turnParameterQuestions, semMem, lingDb, language);
  triggers_addAnswerWithOneParameter("Fais une rotation à gauche", turnParameterQuestions, semMem, lingDb, language);

  triggers_addAnswerWithOneParameter("Tourne à droite", turnParameterQuestions, semMem, lingDb, language);
  triggers_addAnswerWithOneParameter("Va à droite", turnParameterQuestions, semMem, lingDb, language);
  triggers_addAnswerWithOneParameter("Fais une rotation à droite", turnParameterQuestions, semMem, lingDb, language);

  const std::vector<std::string> turnHeadParameterQuestions =
  {"De combien dois-je tourner la tête en degré ?"};
  triggers_addAnswerWithOneParameter("Tourne la tête à gauche", turnHeadParameterQuestions, semMem, lingDb, language);
  triggers_addAnswerWithOneParameter("Tourne la tête à droite", turnHeadParameterQuestions, semMem, lingDb, language);
  triggers_addAnswerWithOneParameter("Regarde à gauche", turnHeadParameterQuestions, semMem, lingDb, language);

  const std::vector<std::string> riseHeadParameterQuestions =
  {"De combien dois-je lever la tête en degré ?"};
  triggers_addAnswerWithOneParameter("Lève la tête", riseHeadParameterQuestions, semMem, lingDb, language);
  triggers_addAnswerWithOneParameter("Regarde en haut", {}, semMem, lingDb, language);

  const std::vector<std::string> lowerDownHeadParameterQuestions =
  {"De combien dois-je baisser la tête en degré ?"};
  triggers_addAnswerWithOneParameter("Baisse la tête", lowerDownHeadParameterQuestions, semMem, lingDb, language);

  const std::vector<std::string> reduceVolumeParameterQuestions =
  {"à combien dois-je descendre le volume en pourcentage ?",
   "à combien dois-je diminuer le volume en pourcentage ?"};
  triggers_addAnswerWithOneParameter("Descends le volume", reduceVolumeParameterQuestions, semMem, lingDb, language);
  triggers_addAnswerWithOneParameter("Diminue le volume", reduceVolumeParameterQuestions, semMem, lingDb, language);

  triggers_addAnswerWithOneParameter("Mets le volume moins fort", {}, semMem, lingDb, language);
  triggers_addAnswerWithOneParameter("Parle moins fort", {}, semMem, lingDb, language);

  const std::vector<std::string> increaseVolumeParameterQuestions =
  {"à combien dois-je monter le volume en pourcentage ?",
   "à combien dois-je augmenter le volume en pourcentage ?"};
  triggers_addAnswerWithOneParameter("Monte le volume", increaseVolumeParameterQuestions, semMem, lingDb, language);
  triggers_addAnswerWithOneParameter("Augmente le volume", increaseVolumeParameterQuestions, semMem, lingDb, language);

  triggers_addAnswerWithOneParameter("Mets le volume plus fort", {}, semMem, lingDb, language);
  triggers_addAnswerWithOneParameter("Parle plus fort", {}, semMem, lingDb, language);

  const std::vector<std::string> setVolumeParameterQuestions =
  {"à combien dois-je mettre le volume en pourcentage ?"};
  triggers_addAnswerWithOneParameter("Mets le volume", setVolumeParameterQuestions, semMem, lingDb, language);



  triggers_addAnswerWithOneParameter("Lance", {"Qu'est-ce que je dois lancer ?"}, semMem, lingDb, language);

  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Avance(param1=3 mètres)\\", triggers_match("Avance 3 mètres", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Avance(param1=5 mètres)\\", triggers_match("Avance cinq mètres", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Avance(param1=4 mètres)\\", triggers_match("Avance de 4 mètres", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Avance(param1=1 mètre)\\", triggers_match("Avance d'un mètre", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Avance(param1=2 mètres)\\", triggers_match("Avance de deux mètres", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Avance(param1=0,7 mètre)\\", triggers_match("Avance de 0,7 m", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Avance(param1=0,3 mètre)\\", triggers_match("Avance de 30 centimètres", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Avance(param1=0,4 mètre)\\", triggers_match("Avance de 40 cm", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Va vers l'avant(param1=5 mètres)\\", triggers_match("Va vers l'avant 5 mètres", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Va vers l'avant(param1=6 mètres)\\", triggers_match("Va vers l'avant de 6 mètres", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Va tout droit(param1=7 mètres)\\", triggers_match("Va tout droit 7 mètres", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Va tout droit(param1=8 mètres)\\", triggers_match("Va tout droit de 8 mètres", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Avance(param1=7 mètres)\\", triggers_match("Je veux que tu avances de 7 mètres", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Avance\\", triggers_match("Avance", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Va vers l'avant\\", triggers_match("Va vers l'avant", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Avance un peu\\", triggers_match("Avance un peu", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Avance beaucoup\\", triggers_match("Avance beaucoup", semMem, lingDb));

  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Recule(param1=2 mètres)\\", triggers_match("Recule de 2 mètres", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Va en arrière\\", triggers_match("Va en arrière", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Va en arrière(param1=0,7 mètre)\\", triggers_match("Va en arrière 0,7 mètre", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Va en arrière(param1=0,35 mètre)\\", triggers_match("Va en arrière trente cinq centimètres", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Va en arrière(param1=0,5 mètre)\\", triggers_match("Va 50 centimètres en arrière", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Va un peu en arrière\\", triggers_match("Va un peu en arrière", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Va un petit peu en arrière\\", triggers_match("Va un petit peu en arrière", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Recule beaucoup\\", triggers_match("Recule beaucoup", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Va beaucoup en arrière\\", triggers_match("Va beaucoup en arrière", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Va très en arrière\\", triggers_match("Va très en arrière", semMem, lingDb));

  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Tourne à gauche(param1=34 degrés)\\", triggers_match("Tourne à gauche de 34 degrés", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Tourne à droite(param1=37 degrés)\\", triggers_match("Tourne à droite de 37 degrés", semMem, lingDb));
  //ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Va à droite(param1=42 degrés)\\", triggers_match("Va 42 degrés à droite", semMem, lingDb));
  //ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Va à droite(param1=45 degrés)\\", triggers_match("Va à droite de 45 degrés", semMem, lingDb));
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
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Lève la tête(param1=11 degrés)\\", triggers_match("Monte la tête de onze degrés", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Regarde en haut\\", triggers_match("Regarde en haut", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Regarde en haut\\", triggers_match("Regarde vers le haut", semMem, lingDb));

  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Baisse la tête\\", triggers_match("Descends la tête", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Baisse la tête(param1=29 degrés)\\", triggers_match("descends la tête de vingt neuf degrés", semMem, lingDb));

  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Descends le volume\\", triggers_match("Descends ton volume", semMem, lingDb, language));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Descends le volume(param1=30 pour cent)\\", triggers_match("Descends le volume de 30 %", semMem, lingDb, language));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Descends le volume(param1=31 pour cent)\\", triggers_match("Descends ton volume de 31 %", semMem, lingDb, language));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Diminue le volume(param1=32 pour cent)\\", triggers_match("Je veux que tu diminues le son de 32 %", semMem, lingDb, language));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Mets le volume moins fort\\", triggers_match("Mets le volume moins fort", semMem, lingDb, language));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Mets le volume moins fort\\", triggers_match(("Mets le volume encore moins fort"), semMem, lingDb, language));

  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Monte le volume(param1=12 pour cent)\\", triggers_match("Monte le volume de 12 %", semMem, lingDb, language));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Mets le volume(param1=90 pour cent)\\", triggers_match("Mets le volume à 90 %", semMem, lingDb, language));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Mets le volume(param1=91 pour cent)\\", triggers_match("Mets le volume à quatre vingt onze pour cent", semMem, lingDb, language));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Mets le volume plus fort\\", triggers_match("Mets le volume plus fort", semMem, lingDb, language));
  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Mets le volume plus fort\\", triggers_match("Mets le volume encore plus fort", semMem, lingDb, language));

  ONSEM_BEHAVIOR_EQ("\\label=#fr_FR#Lance(param1=Akinator)\\", triggers_match("Lance akinator", semMem, lingDb));
}


TEST_F(SemanticReasonerGTests, operator_reactFromTrigger_withParameters_en)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;
  auto language = SemanticLanguageEnum::ENGLISH;

  triggers_addAnswerWithOneParameter("Advance", {"How far should I advance in meters?"},
                                     semMem, lingDb, language);

  triggers_addAnswerWithOneParameter("Move forward a little", {""},
                                     semMem, lingDb, language);

  triggers_addAnswerWithOneParameter("Move forward a lot", {""},
                                     semMem, lingDb, language);

  triggers_addAnswerWithOneParameter("Move backward", {"How far should I move backward in meters?"},
                                     semMem, lingDb, language);

  triggers_addAnswerWithOneParameter("Move backward a little", {},
                                     semMem, lingDb, language);
  triggers_addAnswerWithOneParameter("Move backward a lot", {},
                                     semMem, lingDb, language);

  const std::vector<std::string> turnParameterQuestions = {"How far should I turn in degrees?"};
  triggers_addAnswerWithOneParameter("Turn left", {turnParameterQuestions},
                                     semMem, lingDb, language);
  triggers_addAnswerWithOneParameter("Turn on your left", {turnParameterQuestions},
                                     semMem, lingDb, language);
  triggers_addAnswerWithOneParameter("Turn right", {turnParameterQuestions},
                                     semMem, lingDb, language);
  triggers_addAnswerWithOneParameter("Turn on your right", {turnParameterQuestions},
                                     semMem, lingDb, language);
  triggers_addAnswerWithOneParameter("Launch", {"What should I launch?"},
                                     semMem, lingDb, language);

  triggers_addAnswerWithOneParameter("can you look up", {},
                                     semMem, lingDb, language);

  ONSEM_BEHAVIOR_EQ("\\label=#en_US#Advance(param1=3 meters)\\", triggers_match("Move forward 3 meters", semMem, lingDb));
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
  ONSEM_BEHAVIOR_EQ("\\label=#en_US#Launch(param1=Akinator)\\", triggers_match("Launch akinator", semMem, lingDb));
  ONSEM_ANSWER_EQ("\\label=#en_US#can you look up\\", triggers_match("Can you look up", semMem, lingDb));
  ONSEM_ANSWER_EQ("\\label=#en_US#can you look up\\", triggers_match("Can you please look up", semMem, lingDb));
  ONSEM_ANSWER_EQ("\\label=#en_US#can you look up\\", triggers_match("Could you look up", semMem, lingDb));
  ONSEM_ANSWER_EQ("\\label=#en_US#can you look up\\", triggers_match("Could you please look up", semMem, lingDb));
}

