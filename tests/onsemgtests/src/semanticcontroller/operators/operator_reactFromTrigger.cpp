#include "operator_reactFromTrigger.hpp"
#include "../../semanticreasonergtests.hpp"
#include <gtest/gtest.h>
#include <onsem/texttosemantic/languagedetector.hpp>
#include <onsem/texttosemantic/tool/semexpgetter.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemory.hpp>
#include <onsem/semantictotext/semexpoperators.hpp>
#include <onsem/semantictotext/semanticconverter.hpp>
#include <onsem/tester/reactOnTexts.hpp>
#include "operator_addATrigger.hpp"

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
  memoryOperation::reactFromTrigger(reaction, pSemanticMemory, std::move(pSemExp), pLingDb,
                                    pReactionOptions);
  return reactionToAnswer(reaction, pSemanticMemory, pLingDb, pTextLanguage);
}

}

namespace onsem
{

DetailedReactionAnswer operator_reactFromTrigger(const std::string& pText,
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
  ONSEM_NOANSWER(operator_reactFromTrigger(whoAreYou, semMem, lingDb));
  ONSEM_NOANSWER(operator_reactFromTrigger(stopApplication, semMem, lingDb));
  ONSEM_NOANSWER(operator_reactFromTrigger(whatTimeItIs, semMem, lingDb));
  ONSEM_NOANSWER(operator_reactFromTrigger(whatAboutWellBeingOfAnimals, semMem, lingDb));
  ONSEM_NOANSWER(operator_reactFromTrigger(whatIsToussaint, semMem, lingDb));

  operator_addATrigger(whoAreYou, iAmYourFrined, semMem, lingDb);
  operator_addATrigger(stopApplication, itIsStopped, semMem, lingDb);
  operator_addATrigger(whatTimeItIs, itIs15h, semMem, lingDb);
  operator_addATrigger(whatAboutWellBeingOfAnimals, itIsNotFamous, semMem, lingDb);
  operator_addATrigger(whatIsToussaint, itIsAHoliday, semMem, lingDb);

  ONSEM_ANSWER_EQ(iAmYourFrined, operator_reactFromTrigger(whoAreYou, semMem, lingDb));
  ONSEM_BEHAVIOR_EQ(itIsStopped, operator_reactFromTrigger(stopApplication, semMem, lingDb));
  ONSEM_BEHAVIOR_EQ(itIsStopped, operator_reactFromTrigger("Ferme l'application", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ(itIsStopped, operator_reactFromTrigger("Quitte l'application", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ(itIsStopped, operator_reactFromTrigger("Stoppe l'application", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ(itIsStopped, operator_reactFromTrigger("Interromps l'application", semMem, lingDb));
  ONSEM_ANSWER_EQ(itIs15h, operator_reactFromTrigger(whatTimeItIs, semMem, lingDb));
  ONSEM_ANSWER_EQ(itIs15h, operator_reactFromTrigger("Quelle heure il est ?", semMem, lingDb));
  //ONSEM_ANSWER_EQ(itIs15h, operator_reactFromTrigger("C'est quoi l'heure", semMem, lingDb));
  //ONSEM_ANSWER_EQ(itIs15h, operator_reactFromTrigger("Il est quelle heure", semMem, lingDb));
  ONSEM_ANSWER_EQ(itIsNotFamous, operator_reactFromTrigger(whatAboutWellBeingOfAnimals, semMem, lingDb));
  ONSEM_ANSWER_EQ(itIsAHoliday, operator_reactFromTrigger(whatIsToussaint, semMem, lingDb));
  ONSEM_ANSWER_EQ(itIsAHoliday, operator_reactFromTrigger("Qu'est-ce que la toussaint ?", semMem, lingDb));
}
