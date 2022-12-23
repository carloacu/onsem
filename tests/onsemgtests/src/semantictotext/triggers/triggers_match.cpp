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
  memoryOperation::resolveAgentAccordingToTheContext(pSemExp, pSemanticMemory, pLingDb);
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



TEST_F(SemanticReasonerGTests, operator_reactFromTrigger_basic)
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
  const std::string trigger8 = "Avance";
  const std::string reaction8 = "Voilà, j'avance.";
  const std::string trigger9 = "Tourne de 30 dégrés";
  const std::string reaction9 = "Ma roue gauche est trop chaude.";
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
  triggers_add(trigger8, reaction8, semMem, lingDb);
  triggers_add(trigger9, reaction9, semMem, lingDb);

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
  ONSEM_BEHAVIOR_EQ(reaction8, triggers_match(trigger8, semMem, lingDb));
  ONSEM_ANSWER_EQ(reaction9, triggers_match(trigger9, semMem, lingDb));
}



TEST_F(SemanticReasonerGTests, operator_reactFromTrigger_withParameters_fr)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;

  triggers_addAnswerWithOneParameter("Avance", "De combien dois-je avancer en centimètres ?", semMem, lingDb);

  ONSEM_BEHAVIOR_EQ("\\l1=#fr_FR#v1(p1=300)\\", triggers_match("Avance 3 mètres", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("\\l1=#fr_FR#v1(p1=400)\\", triggers_match("Avance de 4 mètres", semMem, lingDb));
}


TEST_F(SemanticReasonerGTests, operator_reactFromTrigger_withParameters_en)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;

  triggers_addAnswerWithOneParameter("Advance", "How far should I advance in centimeters?", semMem, lingDb);

  ONSEM_BEHAVIOR_EQ("\\l1=#fr_FR#v1(p1=300)\\", triggers_match("Advance 3 meters", semMem, lingDb));
}
