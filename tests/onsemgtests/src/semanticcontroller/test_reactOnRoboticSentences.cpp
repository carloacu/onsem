#include "../semanticreasonergtests.hpp"
#include <gtest/gtest.h>
#include <onsem/semantictotext/semanticmemory/semanticmemory.hpp>
#include <onsem/semantictotext/semexpoperators.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase.hpp>
#include <onsem/tester/reactOnTexts.hpp>
#include "operators/operator_inform.hpp"

using namespace onsem;


TEST_F(SemanticReasonerGTests, operator_reactOnRoboticSentences)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;
  std::map<const SemanticContextAxiom*, TruenessValue> axiomToConditionCurrentState;
  memoryOperation::learnSayCommand(semMem, lingDb);

  // current activity
  ONSEM_ANSWERNOTFOUND_EQ("I don't know what I am executing.", operator_react("what are you executing?", semMem, lingDb));
  operator_inform("you are executing \"thriller\"", semMem, lingDb);
  ONSEM_ANSWER_EQ("I am executing \"thriller\".", operator_react("what are you executing?", semMem, lingDb));
  ONSEM_ANSWER_EQ("I am running \"thriller\".", operator_react("what are you running", semMem, lingDb));
  ONSEM_ANSWER_EQ("I am executing \"thriller\".", operator_react("what are you doing", semMem, lingDb));
  ONSEM_ANSWER_EQ("Yes, I am executing something.", operator_react("are you executing something", semMem, lingDb));
  ONSEM_ANSWER_EQ("J'exécute \"thriller\".", operator_react("que fais-tu", semMem, lingDb));
  operator_inform("you are executing nothing", semMem, lingDb);
  ONSEM_ANSWER_EQ("I am not executing anything.", operator_react("what are you executing?", semMem, lingDb));
  ONSEM_ANSWER_EQ("No, I am not executing something.", operator_react("are you executing something", semMem, lingDb));

  // the robot is interacting
  ONSEM_ANSWERNOTFOUND_EQ("I don't know if I am interacting.", operator_react("Are you interacting", semMem, lingDb));
  operator_inform("you are interacting", semMem, lingDb);
  ONSEM_ANSWER_EQ("Yes, I am interacting.", operator_react("Are you interacting", semMem, lingDb));
  operator_inform("you are not interacting", semMem, lingDb);
  ONSEM_ANSWER_EQ("No, I am not interacting.", operator_react("Are you interacting", semMem, lingDb));

  // the robot is alone
  ONSEM_ANSWERNOTFOUND_EQ("I don't know if I am alone.", operator_react("Are you alone", semMem, lingDb));
  operator_inform("you are alone", semMem, lingDb);
  ONSEM_ANSWER_EQ("Yes, I am alone.", operator_react("Are you alone", semMem, lingDb));
  operator_inform("you are not alone", semMem, lingDb);
  ONSEM_ANSWER_EQ("No, I am not alone.", operator_react("Are you alone", semMem, lingDb));

  // the robot battery status
  ONSEM_ANSWERNOTFOUND_EQ("I don't know how my battery is.", operator_react("How is your battery", semMem, lingDb));
  operator_inform("Your battery is empty.", semMem, lingDb);
  ONSEM_ANSWER_EQ("My battery is empty.", operator_react("How is your battery", semMem, lingDb));
  operator_inform("Your battery is nearly empty.", semMem, lingDb);
  ONSEM_ANSWER_EQ("My battery is nearly empty.", operator_react("How is your battery", semMem, lingDb));
  operator_inform("Your battery is low.", semMem, lingDb);
  ONSEM_ANSWER_EQ("My battery is low.", operator_react("How is your battery", semMem, lingDb));
  operator_inform("Your battery is half full.", semMem, lingDb);
  ONSEM_ANSWER_EQ("My battery is half full.", operator_react("How is your battery", semMem, lingDb));
  operator_inform("Your battery is full.", semMem, lingDb);
  ONSEM_ANSWER_EQ("My battery is full.", operator_react("How is your battery", semMem, lingDb));
  operator_inform("My glass is empty.", semMem, lingDb);
  ONSEM_ANSWER_EQ("My battery is full.", operator_react("How is your battery", semMem, lingDb));

  // the robot is pushed
  ONSEM_ANSWERNOTFOUND_EQ("I don't know if I am pushed.", operator_react("Are you pushed", semMem, lingDb));
  operator_inform_fromRobot("I am pushed.", semMem, lingDb);
  ONSEM_ANSWER_EQ("Yes, I am pushed.", operator_react("Are you pushed", semMem, lingDb));
  operator_inform_fromRobot("I am not pushed.", semMem, lingDb);
  ONSEM_ANSWER_EQ("No, I am not pushed.", operator_react("Are you pushed", semMem, lingDb));

  // the robot hear a speech
  ONSEM_ANSWERNOTFOUND_EQ("I don't know if I hear a speech.", operator_react("Are you hearing a speech", semMem, lingDb));
  operator_inform_fromRobot("I am hearing a speech.", semMem, lingDb);
  ONSEM_ANSWER_EQ("Yes, I hear a speech.", operator_react("Are you hearing a speech", semMem, lingDb));
  operator_inform_fromRobot("I am not hearing a speech.", semMem, lingDb);
  ONSEM_ANSWER_EQ("No, I don't hear any speech.", operator_react("Are you hearing a speech", semMem, lingDb));

  // the user is coming
  ONSEM_QUESTION_EQ("(\tI don't know.\tTHEN\tDo you come?\t)", operator_react("am I coming", semMem, lingDb));
  operator_inform_fromRobot("You are coming", semMem, lingDb);
  ONSEM_ANSWER_EQ("Yes, you are coming.", operator_react("am I coming", semMem, lingDb));
  operator_inform_fromRobot("You are not coming", semMem, lingDb);
  ONSEM_ANSWER_EQ("No, you are not coming.", operator_react("am I coming", semMem, lingDb));

  // the user is leaving
  ONSEM_QUESTION_EQ("(\tI don't know.\tTHEN\tDo you leave?\t)", operator_react("am I leaving", semMem, lingDb));
  operator_inform_fromRobot("You are leaving", semMem, lingDb);
  ONSEM_ANSWER_EQ("Yes, you are leaving.", operator_react("am I leaving", semMem, lingDb));
  operator_inform_fromRobot("You are not leaving", semMem, lingDb);
  ONSEM_ANSWER_EQ("No, you are not leaving.", operator_react("am I leaving", semMem, lingDb));

  // start activity reasons
  ONSEM_ANSWERNOTFOUND_EQ("I don't know why I started \"thriller\".", operator_react("Why did you start \"thriller\"?", semMem, lingDb));
  operator_inform_fromRobot("I started \"thriller\" because I was turned on", semMem, lingDb);
  ONSEM_ANSWER_EQ("I started \"thriller\" because I was turned on.", operator_react("Why did you start \"thriller\"?", semMem, lingDb));
  operator_inform_fromRobot("I started \"thriller2\" because the trigger condition was true", semMem, lingDb);
  ONSEM_ANSWER_EQ("I started \"thriller2\" because the trigger condition was true.", operator_react("Why did you start \"thriller2\"?", semMem, lingDb));
  operator_inform_fromRobot("I started \"thriller3\" because you asked me orally", semMem, lingDb);
  ONSEM_ANSWER_EQ("I started \"thriller3\" because you asked me.", operator_react("Why did you start \"thriller3\"?", semMem, lingDb));
  operator_inform_fromRobot("I started \"thriller4\" because you trigger the application by the tablet", semMem, lingDb);
  ONSEM_ANSWER_EQ("I started \"thriller4\" because you trigger the application by the tablet.", operator_react("Why did you start \"thriller4\"?", semMem, lingDb));
  operator_inform_fromRobot("I started \"thriller5\" because it was pending", semMem, lingDb);
  ONSEM_ANSWER_EQ("I started \"thriller5\" because it pended.", operator_react("Why did you start \"thriller5\"?", semMem, lingDb));
  operator_inform_fromRobot("I started \"thriller6\" because an external program asked me that", semMem, lingDb);
  ONSEM_ANSWER_EQ("I started \"thriller6\" because an external program asked me that.", operator_react("Why did you start \"thriller6\"?", semMem, lingDb));

  // stop activity reasons
  ONSEM_ANSWERNOTFOUND_EQ("I don't know why I stopped \"thriller\".", operator_react("Why did you stop \"thriller\"?", semMem, lingDb));
  operator_inform_fromRobot("I stopped \"thriller\" because it exited by itself", semMem, lingDb);
  ONSEM_ANSWER_EQ("I stopped \"thriller\" because it exited by itself.", operator_react("Why did you stop \"thriller\"?", semMem, lingDb));
  operator_inform_fromRobot("I stopped \"thriller2\" because you stopped it by a head tactile gesture", semMem, lingDb);
  ONSEM_ANSWER_EQ("I stopped \"thriller2\" because you stopped that by a head tactile gesture.", operator_react("Why did you stop \"thriller2\"?", semMem, lingDb));
  operator_inform_fromRobot("I stopped \"thriller3\" because no users were present", semMem, lingDb);
  ONSEM_ANSWER_EQ("I stopped \"thriller3\" because no user was present.", operator_react("Why did you stop \"thriller3\"?", semMem, lingDb));
  operator_inform_fromRobot("I stopped \"thriller4\" because I stopped to live", semMem, lingDb);
  ONSEM_ANSWER_EQ("I stopped \"thriller4\" because I stopped to live.", operator_react("Why did you stop \"thriller4\"?", semMem, lingDb));
  operator_inform_fromRobot("I stopped \"thriller5\" because I was injured", semMem, lingDb);
  ONSEM_ANSWER_EQ("I stopped \"thriller5\" because I was injured.", operator_react("Why did you stop \"thriller5\"?", semMem, lingDb));
  operator_inform_fromRobot("I stopped \"thriller6\" because the context was incompatible", semMem, lingDb);
  ONSEM_ANSWER_EQ("I stopped \"thriller6\" because the context was incompatible.", operator_react("Why did you stop \"thriller6\"?", semMem, lingDb));
  operator_inform_fromRobot("I stopped \"thriller7\" because an external program started another activity", semMem, lingDb);
  ONSEM_ANSWER_EQ("I stopped \"thriller7\" because an external program started another activity.", operator_react("Why did you stop \"thriller7\"?", semMem, lingDb));
  operator_inform_fromRobot("I stopped \"thriller8\" because an external program stopped the application", semMem, lingDb);
  ONSEM_ANSWER_EQ("I stopped \"thriller8\" because an external program stopped the application.", operator_react("Why did you stop \"thriller8\"?", semMem, lingDb));

  // number of humans seen
  const std::string howManyHumanDoYouSeeInEnglish = "How many humans do you see?";
  const std::string howManyHumanDoYouSeeInFrench = "Combien tu vois d'humains ?";
  ONSEM_ANSWERNOTFOUND_EQ("I don't know how many humans I see.", operator_react(howManyHumanDoYouSeeInEnglish, semMem, lingDb));
  ONSEM_ANSWERNOTFOUND_EQ("Je ne sais pas combien de gens il y a.", operator_react("Combien il y a de gens ?", semMem, lingDb));
  auto seeSentence = operator_inform_fromRobot("I see you. There is one person.", semMem, lingDb);
  ONSEM_ANSWER_EQ("I see one human.", operator_react(howManyHumanDoYouSeeInEnglish, semMem, lingDb));
  ONSEM_ANSWER_EQ("Je vois un humain.", operator_react(howManyHumanDoYouSeeInFrench, semMem, lingDb));
  ONSEM_ANSWER_EQ("Je vois une personne.", operator_react("Combien de personnes vois-tu ?", semMem, lingDb));
  ONSEM_ANSWER_EQ("There is one person.", operator_react("How many people are there?", semMem, lingDb));
  semMem.memBloc.removeExpression(*seeSentence, lingDb, &axiomToConditionCurrentState);
  seeSentence = operator_inform_fromRobot("I don't see you and I see a person", semMem, lingDb);
  ONSEM_ANSWER_EQ("I see one human.", operator_react(howManyHumanDoYouSeeInEnglish, semMem, lingDb));
  ONSEM_ANSWER_EQ("Je vois un humain.", operator_react(howManyHumanDoYouSeeInFrench, semMem, lingDb));
  semMem.memBloc.removeExpression(*seeSentence, lingDb, &axiomToConditionCurrentState);
  seeSentence = operator_inform_fromRobot("I don't see you and I see 2 people", semMem, lingDb);
  ONSEM_ANSWER_EQ("I see 2 humans.", operator_react(howManyHumanDoYouSeeInEnglish, semMem, lingDb));
  ONSEM_ANSWER_EQ("Je vois 2 humains.", operator_react(howManyHumanDoYouSeeInFrench, semMem, lingDb));
  semMem.memBloc.removeExpression(*seeSentence, lingDb, &axiomToConditionCurrentState);
  seeSentence = operator_inform_fromRobot("I see you and I see another person", semMem, lingDb);
  ONSEM_ANSWER_EQ("I see 2 humans.", operator_react(howManyHumanDoYouSeeInEnglish, semMem, lingDb));
  ONSEM_ANSWER_EQ("Je vois 2 humains.", operator_react(howManyHumanDoYouSeeInFrench, semMem, lingDb));
  semMem.memBloc.removeExpression(*seeSentence, lingDb, &axiomToConditionCurrentState);
  seeSentence = operator_informAxiom_fromRobot("There are 3 people. I see you and I see 2 other people.", semMem, lingDb);
  ONSEM_FEEDBACK_EQ("No, I see you.", operator_react("you don't see me", semMem, lingDb));
  ONSEM_ANSWER_EQ("I see 3 humans.", operator_react(howManyHumanDoYouSeeInEnglish, semMem, lingDb));
  ONSEM_ANSWER_EQ("Je vois 3 humains.", operator_react(howManyHumanDoYouSeeInFrench, semMem, lingDb));
  ONSEM_ANSWER_EQ("I see 3 people.", operator_react("How many people do you see?", semMem, lingDb));
  ONSEM_ANSWER_EQ("I see 3 people.", operator_react("How many peoples do you see?", semMem, lingDb));
  ONSEM_ANSWER_EQ("I see 3 people.", operator_react("How many persons do you see?", semMem, lingDb));
  ONSEM_ANSWER_EQ("There are 3 humans.", operator_react("How many humans are there?", semMem, lingDb));
  ONSEM_ANSWER_EQ("There are 3 people.", operator_react("How many people are there?", semMem, lingDb));
  ONSEM_ANSWER_EQ("There are 3 people.", operator_react("How many persons are there?", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("I see 3 people.", operator_react("Say how many people you see", semMem, lingDb));
  ONSEM_ANSWER_EQ("Je vois 3 personnes.", operator_react("Combien de personnes vois-tu ?", semMem, lingDb));
  ONSEM_ANSWER_EQ("Je vois 3 gens.", operator_react("Combien de gens vois-tu ?", semMem, lingDb));
  ONSEM_ANSWER_EQ("Il y a 3 gens.", operator_react("Combien il y a de gens ?", semMem, lingDb));
  ONSEM_ANSWER_EQ("Il y a 3 personnes.", operator_react("Combien y a-t-il de personnes ?", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("Je vois 3 personnes.", operator_react("Dis combien tu vois de personnes", semMem, lingDb));
  semMem.memBloc.removeExpression(*seeSentence, lingDb, &axiomToConditionCurrentState);
  operator_mergeAndInform("you see me", semMem, lingDb);
  operator_mergeAndInform("you see another human", semMem, lingDb);
  ONSEM_ANSWER_EQ("I see you and another human.", operator_react("what do you see?", semMem, lingDb));
}

