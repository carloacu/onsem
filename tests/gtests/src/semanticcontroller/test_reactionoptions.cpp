#include "../semanticreasonergtests.hpp"
#include <gtest/gtest.h>
#include <onsem/semantictotext/semanticmemory/semanticmemory.hpp>
#include <onsem/semantictotext/type/reactionoptions.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase.hpp>
#include <onsem/tester/reactOnTexts.hpp>
#include "operators/operator_addATrigger.hpp"
#include "operators/operator_inform.hpp"
#include "externalinfos/dummyjokeprovider.hpp"

using namespace onsem;


TEST_F(SemanticReasonerGTests, operator_reactionOptions_canAnswerIDontKnow)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;

  ReactionOptions canAnswerIDontKnowReaction;
  canAnswerIDontKnowReaction.canAnswerIDontKnow = true;
  ReactionOptions cannnotAnswerIDontKnowReaction;
  cannnotAnswerIDontKnowReaction.canAnswerIDontKnow = false;

  const std::string questionStr = "what does Paul like?";
  ONSEM_ANSWERNOTFOUND_EQ("I don't know what Paul likes.",
                          operator_react(questionStr, semMem, lingDb,
                                         SemanticLanguageEnum::ENGLISH, &canAnswerIDontKnowReaction));
  ONSEM_NOANSWER(operator_react(questionStr, semMem, lingDb,
                                SemanticLanguageEnum::ENGLISH, &cannnotAnswerIDontKnowReaction));

  const std::string orderFrStr = "saute";
  ONSEM_BEHAVIORNOTFOUND_EQ("Je ne sais pas sauter.",
                            operator_react(orderFrStr, semMem, lingDb,
                                           SemanticLanguageEnum::FRENCH, &canAnswerIDontKnowReaction));
  ONSEM_NOANSWER(operator_react(orderFrStr, semMem, lingDb,
                                SemanticLanguageEnum::FRENCH, &cannnotAnswerIDontKnowReaction));
}


TEST_F(SemanticReasonerGTests, operator_reactionOptions_canDoProactivity)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;

  ReactionOptions canDoProactivityReaction;
  canDoProactivityReaction.canDoAProactivity = true;
  ReactionOptions cannotDoProactivityReaction;
  cannotDoProactivityReaction.canDoAProactivity = false;

  const std::string sentenceToTriggerProactivityFrStr = "j'aime cette voiture";
  ONSEM_QUESTION_EQ("Pourquoi aimes-tu cette voiture ?",
                    operator_react(sentenceToTriggerProactivityFrStr, semMem, lingDb,
                                   SemanticLanguageEnum::FRENCH, &canDoProactivityReaction));
  ONSEM_NOANSWER(operator_react(sentenceToTriggerProactivityFrStr, semMem, lingDb,
                                SemanticLanguageEnum::FRENCH, &cannotDoProactivityReaction));

  const std::string garbageSentenceStr = "dfgdfg";
  ONSEM_NOANSWER(operator_react(garbageSentenceStr, semMem, lingDb,
                                SemanticLanguageEnum::FRENCH, &cannotDoProactivityReaction));
  ONSEM_NOANSWER(operator_react(garbageSentenceStr, semMem, lingDb,
                                SemanticLanguageEnum::FRENCH, &canDoProactivityReaction));
  canDoProactivityReaction.canReactToANoun = true;
  ONSEM_QUESTION_EQ("Qu'est-ce que dfgdfg ?",
                    operator_react(garbageSentenceStr, semMem, lingDb,
                                   SemanticLanguageEnum::FRENCH, &canDoProactivityReaction));
  canDoProactivityReaction.canReactToANoun = false;

  ONSEM_NOANSWER(operator_react("twingo est une voiture", semMem, lingDb,
                                SemanticLanguageEnum::FRENCH, &cannotDoProactivityReaction));
  ONSEM_NOANSWER(operator_react("twingo", semMem, lingDb,
                                SemanticLanguageEnum::FRENCH, &canDoProactivityReaction));
  canDoProactivityReaction.canReactToANoun = true;
  ONSEM_ANSWER_EQ("Twingo est une voiture.",
                  operator_react("twingo", semMem, lingDb,
                                 SemanticLanguageEnum::FRENCH, &canDoProactivityReaction));
  ONSEM_NOANSWER(operator_react("twingo", semMem, lingDb,
                                SemanticLanguageEnum::FRENCH, &cannotDoProactivityReaction));
}



TEST_F(SemanticReasonerGTests, operator_reactionOptions_canAnswerWithATrigger)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;

  ReactionOptions canAnswerWithATriggerReaction;
  canAnswerWithATriggerReaction.canAnswerWithATrigger = true;
  ReactionOptions cannnotAnswerWithATriggerReaction;
  cannnotAnswerWithATriggerReaction.canAnswerWithATrigger = false;

  // from a nominal group
  {
    const std::string nominalGroupTrigger = "Gad";
    const std::string reactionOfNominalGroup = "Gad is a humorist";
    operator_addATrigger(nominalGroupTrigger, reactionOfNominalGroup, semMem, lingDb);
    ONSEM_NOANSWER(operator_react(nominalGroupTrigger, semMem, lingDb,
                                  SemanticLanguageEnum::ENGLISH, &cannnotAnswerWithATriggerReaction));
    ONSEM_ANSWER_EQ("Gad is a humorist.",
                    operator_react(nominalGroupTrigger, semMem, lingDb,
                                   SemanticLanguageEnum::ENGLISH, &canAnswerWithATriggerReaction));
  }

  // from a question
  {
    const std::string questionTrigger = "What does Gad like?";
    const std::string answerOfQuestion = "Potatoes";
    operator_addATrigger(questionTrigger, answerOfQuestion, semMem, lingDb);
    ONSEM_ANSWERNOTFOUND_EQ("I don't know what Gad likes.",
                            operator_react(questionTrigger, semMem, lingDb,
                                           SemanticLanguageEnum::ENGLISH, &cannnotAnswerWithATriggerReaction));
    ONSEM_ANSWER_EQ(answerOfQuestion,
                    operator_react(questionTrigger, semMem, lingDb,
                                   SemanticLanguageEnum::ENGLISH, &canAnswerWithATriggerReaction));
  }

  // from a affirmation
  {
    const std::string affirmationTrigger = "Gad is a nice man";
    const std::string reactionOfAffirmation = "I like Gad too";
    operator_addATrigger(affirmationTrigger, reactionOfAffirmation, semMem, lingDb);
    ONSEM_QUESTION_EQ("Why is Gad a nice man?",
                      operator_react(affirmationTrigger, semMem, lingDb,
                                     SemanticLanguageEnum::ENGLISH, &cannnotAnswerWithATriggerReaction));
    ONSEM_ANSWER_EQ("I also like Gad.",
                    operator_react(affirmationTrigger, semMem, lingDb,
                                   SemanticLanguageEnum::ENGLISH, &canAnswerWithATriggerReaction));
  }
}


TEST_F(SemanticReasonerGTests, operator_reactionOptions_canReactToANoun)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;

  ReactionOptions canReactToANoun;
  canReactToANoun.canReactToANoun = true;
  ReactionOptions cannotReactToANoun;
  cannotReactToANoun.canReactToANoun = false;

  const std::string garbageSentenceStr = "dfgdfg";
  ONSEM_NOANSWER(operator_react(garbageSentenceStr, semMem, lingDb,
                                SemanticLanguageEnum::FRENCH, &cannotReactToANoun));
  ONSEM_QUESTION_EQ("Qu'est-ce que dfgdfg ?",
                    operator_react(garbageSentenceStr, semMem, lingDb,
                                   SemanticLanguageEnum::FRENCH, &canReactToANoun));

  ONSEM_FEEDBACK_EQ("C'est pas faux.",
                    operator_react("twingo est une voiture", semMem, lingDb));
  ONSEM_NOANSWER(operator_react("twingo", semMem, lingDb,
                                SemanticLanguageEnum::FRENCH, &cannotReactToANoun));
  ONSEM_ANSWER_EQ("Twingo est une voiture.",
                  operator_react("twingo", semMem, lingDb,
                                 SemanticLanguageEnum::FRENCH, &canReactToANoun));
}


TEST_F(SemanticReasonerGTests, operator_reactionOptions_canSayOkToAnAffirmation)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;

  ReactionOptions canSayOkToAnAffirmation;
  canSayOkToAnAffirmation.canSayOkToAnAffirmation = true;
  ReactionOptions cannotSayOkToAnAffirmation;
  cannotSayOkToAnAffirmation.canSayOkToAnAffirmation = false;

  const std::string sentenceStr = "Je suis grand";
  ONSEM_NOANSWER(operator_react(sentenceStr, semMem, lingDb,
                                SemanticLanguageEnum::FRENCH, &cannotSayOkToAnAffirmation));
  semMem.clear();
  ONSEM_FEEDBACK_EQ("Ok, tu es grand.",
                    operator_react(sentenceStr, semMem, lingDb,
                                   SemanticLanguageEnum::FRENCH, &canSayOkToAnAffirmation));
}


TEST_F(SemanticReasonerGTests, operator_reactionOptions_canAnswerWithExternalEngines)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;

  ReactionOptions canAnswerWithExternalEnginesReaction;
  canAnswerWithExternalEnginesReaction.canAnswerWithExternalEngines = true;
  ReactionOptions cannnotAnswerWithExternalEnginesReaction;
  cannnotAnswerWithExternalEnginesReaction.canAnswerWithExternalEngines = false;

  semMem.registerExternalInfosProvider(mystd::make_unique<DummyJokeProvider>(lingDb), lingDb);

  const std::string question = "what is the joke of today ?";
  ONSEM_ANSWERNOTFOUND_EQ("I don't know what the joke of today is.",
                          operator_react(question, semMem, lingDb,
                                         SemanticLanguageEnum::ENGLISH, &cannnotAnswerWithExternalEnginesReaction));
  ONSEM_ANSWER_EQ("The joke of today is " + DummyJokeProvider::englishJoke(),
                  operator_react(question, semMem, lingDb,
                                 SemanticLanguageEnum::ENGLISH, &canAnswerWithExternalEnginesReaction));

  semMem.unregisterExternalInfosProvider(DummyJokeProvider::idStrOfProv);
}



TEST_F(SemanticReasonerGTests, operator_reactionOptions_mergeTextAnResource)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;

  ReactionOptions canMergeTextAndResourceReaction;
  canMergeTextAndResourceReaction.reactWithTextAndResource = true;
  ReactionOptions cannnotMergeTextAndResourceReaction;
  cannnotMergeTextAndResourceReaction.reactWithTextAndResource = false;

  const std::string whoIsN5Str = "What N5 likes ?";
  operator_inform_fromRobot("N5 likes to talk with you", semMem, lingDb);
  ONSEM_ANSWER_EQ("N5 likes to talk with you.",
                  operator_react(whoIsN5Str, semMem, lingDb,
                                 SemanticLanguageEnum::ENGLISH, &canMergeTextAndResourceReaction));
  operator_addATrigger(whoIsN5Str, "You should ask him", semMem, lingDb);
  ONSEM_ANSWER_EQ("You should ask him.",
                  operator_react(whoIsN5Str, semMem, lingDb,
                                 SemanticLanguageEnum::ENGLISH, &canMergeTextAndResourceReaction));

  const std::string questionTriggerStr = "What does Gad like?";
  const std::string videoResource1Str =
      "\\" + resourceLabelForTests_url + "=https://www.youtube.com/watch?v=aD12ChGw-ww\\";
  operator_addATrigger(questionTriggerStr, videoResource1Str, semMem, lingDb);
  operator_inform("Gad likes well-being of humanity", semMem, lingDb);

  ONSEM_ANSWER_EQ(videoResource1Str,
                  operator_react(questionTriggerStr, semMem, lingDb,
                                 SemanticLanguageEnum::ENGLISH, &cannnotMergeTextAndResourceReaction));
  ONSEM_ANSWER_EQ("(\t" + videoResource1Str + "\tTHEN\tGad likes the humanity well-being.\t)",
                  operator_react(questionTriggerStr, semMem, lingDb,
                                 SemanticLanguageEnum::ENGLISH, &canMergeTextAndResourceReaction));

  operator_addATrigger(questionTriggerStr, "Gad likes me", semMem, lingDb);

  ONSEM_ANSWER_EQ(videoResource1Str,
                  operator_react(questionTriggerStr, semMem, lingDb,
                                 SemanticLanguageEnum::ENGLISH, &cannnotMergeTextAndResourceReaction));
  ONSEM_ANSWER_EQ("(\t" + videoResource1Str + "\tTHEN\tGad likes me.\t)",
                  operator_react(questionTriggerStr, semMem, lingDb,
                                 SemanticLanguageEnum::ENGLISH, &canMergeTextAndResourceReaction));


  const std::string questionTrigger2Str = "Qui est Gad ?";
  operator_inform("Gad est un humoriste", semMem, lingDb);

  ONSEM_ANSWER_EQ("Gad est un humoriste.",
                  operator_react(questionTrigger2Str, semMem, lingDb,
                                 SemanticLanguageEnum::FRENCH, &cannnotMergeTextAndResourceReaction));
  ONSEM_ANSWER_EQ("Gad est un humoriste.",
                  operator_react(questionTrigger2Str, semMem, lingDb,
                                 SemanticLanguageEnum::FRENCH, &canMergeTextAndResourceReaction));

  const std::string videoResource2Str =
      "\\" + resourceLabelForTests_url + "=https://www.youtube.com/watch?v=XgQVUNTYgr4\\";
  operator_addATrigger(questionTrigger2Str, videoResource2Str, semMem, lingDb);

  ONSEM_ANSWER_EQ(videoResource2Str,
                  operator_react(questionTrigger2Str, semMem, lingDb,
                                 SemanticLanguageEnum::FRENCH, &cannnotMergeTextAndResourceReaction));
  ONSEM_ANSWER_EQ("(\t" + videoResource2Str + "\tTHEN\tGad est un humoriste.\t)",
                  operator_react(questionTrigger2Str, semMem, lingDb,
                                 SemanticLanguageEnum::FRENCH, &canMergeTextAndResourceReaction));
  canMergeTextAndResourceReaction.canReactToANoun = true;
  operator_addATrigger("Gad", videoResource2Str, semMem, lingDb);
  ONSEM_ANSWER_EQ("(\t" + videoResource2Str + "\tTHEN\tGad est un humoriste.\t)",
                  operator_react("Gad", semMem, lingDb,
                                 SemanticLanguageEnum::FRENCH, &canMergeTextAndResourceReaction));
  canMergeTextAndResourceReaction.canReactToANoun = false;


  const std::string questionTrigger3Str = "Montre-moi Gad";
  const std::string videoResource3Str =
      "\\" + resourceLabelForTests_url + "=https://www.youtube.com/watch?v=aDpywB8pHJ4\\";
  operator_addATrigger(questionTrigger3Str, videoResource3Str, semMem, lingDb);

  ONSEM_BEHAVIOR_EQ(videoResource3Str,
                    operator_react(questionTrigger3Str, semMem, lingDb,
                                   SemanticLanguageEnum::FRENCH, &cannnotMergeTextAndResourceReaction));
  ONSEM_BEHAVIOR_EQ(videoResource3Str,
                    operator_react(questionTrigger3Str, semMem, lingDb,
                                   SemanticLanguageEnum::FRENCH, &canMergeTextAndResourceReaction));


  const std::string questionTrigger4Str = "Je veux voir une vidéo de Gad";
  const std::string videoResource4Str =
      "\\" + resourceLabelForTests_url + "=https://www.youtube.com/watch?v=FS6P3A4_1gQ\\";
  operator_addATrigger(questionTrigger4Str, videoResource4Str, semMem, lingDb);

  ONSEM_ANSWER_EQ(videoResource4Str,
                  operator_react(questionTrigger4Str, semMem, lingDb,
                                 SemanticLanguageEnum::FRENCH, &cannnotMergeTextAndResourceReaction));
  ONSEM_ANSWER_EQ(videoResource4Str,
                  operator_react(questionTrigger4Str, semMem, lingDb,
                                 SemanticLanguageEnum::FRENCH, &canMergeTextAndResourceReaction));


  // Don't do a proactive reaction after a question
  const std::string questionTrigger5Str = "J'ai trouvé ma brosse à dents";
  const std::string answer5Str = "Cool, elle était où ?";
  operator_addATrigger(questionTrigger5Str, answer5Str, semMem, lingDb);
  ONSEM_ANSWER_EQ("Cool, où était-elle ?",
                  operator_react(questionTrigger5Str, semMem, lingDb,
                                 SemanticLanguageEnum::FRENCH, &canMergeTextAndResourceReaction));
}
