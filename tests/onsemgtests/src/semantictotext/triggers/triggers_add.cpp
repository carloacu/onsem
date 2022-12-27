#include "triggers_add.hpp"
#include <onsem/semantictotext/semanticconverter.hpp>
#include <onsem/semantictotext/semexpoperators.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemory.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/groundedexpression.hpp>
#include <onsem/semantictotext/triggers.hpp>
#include "../../semanticreasonergtests.hpp"
#include "../../util/util.hpp"
#include "../operators/operator_answer.hpp"
#include "../operators/operator_check.hpp"
#include "../operators/operator_inform.hpp"
#include "triggers_match.hpp"
#include <onsem/tester/reactOnTexts.hpp>
#include <onsem/tester/detailedreactionanswer.hpp>


namespace onsem
{

void _triggers_addFromSemExp(UniqueSemanticExpression& pTriggerSemExp,
                             const std::string& pAnswerText,
                             SemanticMemory& pSemanticMemory,
                             const linguistics::LinguisticDatabase& pLingDb,
                             const std::list<std::string>& pReferences)
{
  TextProcessingContext answerProcContext(SemanticAgentGrounding::me,
                                          SemanticAgentGrounding::currentUser,
                                          SemanticLanguageEnum::UNKNOWN);
  answerProcContext.isTimeDependent = false;
  answerProcContext.cmdGrdExtractorPtr = std::make_shared<ResourceGroundingExtractor>(
        std::vector<std::string>{resourceLabelForTests_cmd, resourceLabelForTests_url});
  auto answerSemExp =
      converter::textToContextualSemExp(pAnswerText, answerProcContext, SemanticSourceEnum::WRITTENTEXT,
                                        pLingDb, &pReferences);

  triggers::add(std::move(pTriggerSemExp), std::move(answerSemExp), pSemanticMemory, pLingDb);
}


void triggers_add(const std::string& pTriggerText,
                  const std::string& pAnswerText,
                  SemanticMemory& pSemanticMemory,
                  const linguistics::LinguisticDatabase& pLingDb,
                  const std::list<std::string>& pReferences)
{
  TextProcessingContext triggerProcContext(SemanticAgentGrounding::currentUser,
                                           SemanticAgentGrounding::me,
                                           SemanticLanguageEnum::UNKNOWN);
  triggerProcContext.isTimeDependent = false;
  auto triggerSemExp = converter::textToContextualSemExp(pTriggerText, triggerProcContext,
                                                         SemanticSourceEnum::UNKNOWN, pLingDb);
  _triggers_addFromSemExp(triggerSemExp, pAnswerText, pSemanticMemory, pLingDb, pReferences);
}

void triggers_addToSemExpAnswer(
    const std::string& pTriggerText,
    UniqueSemanticExpression pAnswerSemExp,
    SemanticMemory& pSemanticMemory,
    const linguistics::LinguisticDatabase& pLingDb,
    SemanticLanguageEnum pLanguage)
{
  TextProcessingContext triggerProcContext(SemanticAgentGrounding::currentUser,
                                           SemanticAgentGrounding::me,
                                           pLanguage);
  triggerProcContext.isTimeDependent = false;
  auto triggerSemExp = converter::textToContextualSemExp(pTriggerText, triggerProcContext,
                                                         SemanticSourceEnum::UNKNOWN, pLingDb);
  triggers::add(std::move(triggerSemExp), std::move(pAnswerSemExp), pSemanticMemory, pLingDb);
}


void triggers_addAnswerWithOneParameter(
    const std::string& pTriggerText,
    const std::vector<std::string>& pParameterQuestions,
    SemanticMemory& pSemanticMemory,
    const linguistics::LinguisticDatabase& pLingDb,
    SemanticLanguageEnum pLanguage)
{
  TextProcessingContext paramQuestionProcContext(SemanticAgentGrounding::me,
                                                 SemanticAgentGrounding::currentUser,
                                                 pLanguage);
  paramQuestionProcContext.isTimeDependent = false;
  auto answer1Grd = std::make_unique<SemanticResourceGrounding>("label", pLanguage, pTriggerText);

  for (auto& currQuestion : pParameterQuestions)
  {
    auto paramSemExp = converter::textToContextualSemExp(currQuestion,
                                                         paramQuestionProcContext,
                                                         SemanticSourceEnum::UNKNOWN, pLingDb);
    answer1Grd->resource.parameterLabelsToQuestions["param1"].emplace_back(std::move(paramSemExp));
  }

  auto answer1SemExp = std::make_unique<GroundedExpression>(std::move(answer1Grd));

  triggers_addToSemExpAnswer(pTriggerText, std::move(answer1SemExp), pSemanticMemory, pLingDb, pLanguage);
}


void triggers_addOtherTriggerFormulations(const std::string& pTriggerText,
                                          const std::string& pAnswerText,
                                          SemanticMemory& pSemanticMemory,
                                          const linguistics::LinguisticDatabase& pLingDb,
                                          const std::list<std::string>& pReferences = std::list<std::string>())
{
  TextProcessingContext triggerProcContext(SemanticAgentGrounding::currentUser,
                                           SemanticAgentGrounding::me,
                                           SemanticLanguageEnum::UNKNOWN);
  triggerProcContext.isTimeDependent = false;
  auto triggerSemExp = converter::textToSemExp(pTriggerText, triggerProcContext, pLingDb);
  std::list<UniqueSemanticExpression> otherTriggerFormulations;
  converter::addOtherTriggerFormulations(otherTriggerFormulations, *triggerSemExp);
  for (auto& currOtherTriggerFormulation : otherTriggerFormulations)
    _triggers_addFromSemExp(currOtherTriggerFormulation, pAnswerText, pSemanticMemory, pLingDb, pReferences);
}


} // End of namespace onsem


using namespace onsem;



TEST_F(SemanticReasonerGTests, operator_addATrigger_basic)
{
  auto iStreams = linguistics::generateIStreams(lingDbPath, dynamicdictionaryPath);
  linguistics::LinguisticDatabase lingDb(iStreams.linguisticDatabaseStreams);
  iStreams.close();
  SemanticMemory semMem;
  ReactionOptions canReactToANoun;
  canReactToANoun.canReactToANoun = true;

  // simple scenario
  {
    const std::string howCanIBeHappy = "how can I be happy";
    ONSEM_ANSWERNOTFOUND_EQ("I don't know how you can be happy.",
                            operator_answer(howCanIBeHappy, semMem, lingDb));
    const std::string answerStr = "I suggest to sing.";
    triggers_add(howCanIBeHappy, answerStr, semMem, lingDb);
    ONSEM_ANSWER_EQ(answerStr, operator_answer(howCanIBeHappy, semMem, lingDb));
    ONSEM_ANSWER_EQ(answerStr, operator_react(howCanIBeHappy, semMem, lingDb));
    ONSEM_ANSWER_EQ(answerStr, triggers_match(howCanIBeHappy, semMem, lingDb));
  }

  // simple scenario in french
  {
    const std::string whatIsAMushroom = "qu'est-ce que c'est un champignon";
    ONSEM_ANSWERNOTFOUND_EQ("Je ne sais pas ce que c'est un champignon.", operator_react(whatIsAMushroom, semMem, lingDb));
    const std::string answerStr = "C'est un eucaryote pluricellulaire ou unicellulaire.";
    triggers_add(whatIsAMushroom, answerStr, semMem, lingDb);
    ONSEM_ANSWER_EQ(answerStr, operator_answer(whatIsAMushroom, semMem, lingDb));
    ONSEM_ANSWER_EQ(answerStr, operator_react(whatIsAMushroom, semMem, lingDb));
    ONSEM_ANSWER_EQ(answerStr, triggers_match(whatIsAMushroom, semMem, lingDb));
  }

  // a question about subject that is a list of elements
  {
    const std::string areWeHappy = "are we happy";
    ONSEM_ANSWERNOTFOUND_EQ("I don't know if we are happy.", operator_answer(areWeHappy, semMem, lingDb));
    const std::string answerStr = "It's probably sunday.";
    triggers_add(areWeHappy, answerStr, semMem, lingDb);
    ONSEM_ANSWER_EQ(answerStr, operator_answer(areWeHappy, semMem, lingDb));
    ONSEM_ANSWER_EQ(answerStr, triggers_match(areWeHappy, semMem, lingDb));
  }

  // a question about object that is a list of elements
  {
    const std::string questionStr = "Does Paul like banana and bear?";
    ONSEM_ANSWERNOTFOUND_EQ("I don't know if Paul likes banana and the bear.",
                            operator_answer(questionStr, semMem, lingDb));
    const std::string answerStr = "Paul is a nice guy.";
    triggers_add(questionStr, answerStr, semMem, lingDb);
    ONSEM_ANSWERNOTFOUND_EQ("I don't know if Paul likes banana.",
                            operator_answer("Does Paul like banana ?", semMem, lingDb));
    ONSEM_ANSWER_EQ(answerStr, operator_answer(questionStr, semMem, lingDb));
    ONSEM_ANSWER_EQ(answerStr, triggers_match(questionStr, semMem, lingDb));
  }

  // a question with an undefined subject
  {
    const std::string questionStr = "What does it mean to raise the left arm?";
    ONSEM_ANSWERNOTFOUND_EQ("I don't know what it means to raise the left arm.",
                            operator_answer(questionStr, semMem, lingDb));
    const std::string answerStr = "It means to run the animation called raise_the_left_arm.";
    triggers_add(questionStr, answerStr, semMem, lingDb);
    ONSEM_ANSWER_EQ(answerStr, operator_answer(questionStr, semMem, lingDb));
    ONSEM_ANSWER_EQ(answerStr, triggers_match(questionStr, semMem, lingDb));
  }

  // many trigger questions
  {
    const std::string question1Str = "What do you like?";
    const std::string question2Str = "Do you like soccer?";
    const std::string bothQuestionsStr = question1Str + " " + question2Str;
    ONSEM_ANSWERNOTFOUND_EQ("I don't know what I like.",
                            operator_answer(question1Str, semMem, lingDb));
    ONSEM_ANSWERNOTFOUND_EQ("I don't know if I like soccer.",
                            operator_answer(question2Str, semMem, lingDb));
    ONSEM_ANSWERNOTFOUND_EQ("I don't know what I like. I don't know if I like soccer.",
                            operator_answer(bothQuestionsStr, semMem, lingDb));
    const std::string answer1Str = "I like Soccer.";
    const std::string answer2Str = "Soccer is very nice.";
    const std::string bothAnswerStr = "Exactly. Yes. I like Soccer.";
    triggers_add(question1Str, answer1Str, semMem, lingDb);
    triggers_add(question2Str, answer2Str, semMem, lingDb);
    triggers_add(bothQuestionsStr, bothAnswerStr, semMem, lingDb);
    ONSEM_ANSWERNOTFOUND_EQ("I don't know if Paul likes banana.",
                            operator_answer("Does Paul like banana ?", semMem, lingDb));
    ONSEM_ANSWER_EQ(answer1Str, operator_answer(question1Str, semMem, lingDb));
    ONSEM_ANSWER_EQ(answer2Str, operator_answer(question2Str, semMem, lingDb));
    ONSEM_ANSWER_EQ(bothAnswerStr, operator_react(bothQuestionsStr, semMem, lingDb));
    ONSEM_ANSWER_EQ(bothAnswerStr, operator_answer(bothQuestionsStr, semMem, lingDb));
    ONSEM_ANSWER_EQ(bothAnswerStr, triggers_match(bothQuestionsStr, semMem, lingDb));
    ONSEM_ANSWERNOTFOUND_EQ("I don't know if I like tennis.",
                            operator_answer("Do you like tennis?", semMem, lingDb));
  }

  // a where question
  {
    const std::string questionsStr = "Où est la clé abîmée ?";
    ONSEM_ANSWERNOTFOUND_EQ("Je ne sais pas où est la clé abîmée.",
                            operator_react(questionsStr, semMem, lingDb));
    triggers_add(questionsStr, "Elle est dans la cuisine", semMem, lingDb);
    const std::string answerStr = "Elle est dans la cuisine.";
    ONSEM_ANSWER_EQ(answerStr, operator_react(questionsStr, semMem, lingDb));
    ONSEM_ANSWER_EQ(answerStr, triggers_match(questionsStr, semMem, lingDb));
  }

  // question about a sentence
  {
    const std::string questionsStr = "A quoi ça sert une tablette";
    ONSEM_ANSWERNOTFOUND_EQ("Je ne sais pas à quoi une tablette sert.",
                            operator_react(questionsStr, semMem, lingDb));
    const std::string answerStr = "Ça sert à afficher des choses.";
    triggers_add(questionsStr, answerStr, semMem, lingDb);
    ONSEM_ANSWER_EQ(answerStr, operator_react(questionsStr, semMem, lingDb));
    ONSEM_ANSWER_EQ(answerStr, triggers_match(questionsStr, semMem, lingDb));
  }

  // must question
  {
    const std::string questionsStr = "Que faut-il faire?";
    ONSEM_ANSWERNOTFOUND_EQ("Je ne sais pas ce qu'il faut faire.",
                            operator_answer(questionsStr, semMem, lingDb));
    const std::string answerStr = "Rien de spécial";
    triggers_add(questionsStr, answerStr, semMem, lingDb);
    ONSEM_ANSWER_EQ(answerStr, operator_answer(questionsStr, semMem, lingDb));
    ONSEM_ANSWER_EQ(answerStr, triggers_match(questionsStr, semMem, lingDb));
  }

  // infinitive question trigger
  {
    const std::string questionsStr = "comment formuler cette question";
    ONSEM_ANSWERNOTFOUND_EQ("Je ne sais pas comment formuler cette question.",
                            operator_answer(questionsStr, semMem, lingDb));
    const std::string answerStr = "Il faut regarder dans un livre de grammaire.";
    triggers_add(questionsStr, answerStr, semMem, lingDb);
    ONSEM_ANSWER_EQ(answerStr, operator_answer(questionsStr, semMem, lingDb));
    ONSEM_ANSWER_EQ(answerStr, triggers_match(questionsStr, semMem, lingDb));
  }

  // affirmation trigger
  {
    const std::string affirmationStr = "Paul is a nice guy";
    ONSEM_QUESTION_EQ("Why is Paul a nice guy?",
                      operator_react(affirmationStr, semMem, lingDb));
    const std::string answerStr = "Yes. Do you want to hear a fun story about him?";
    triggers_add(affirmationStr, answerStr, semMem, lingDb);
    ONSEM_ANSWER_EQ(answerStr, operator_react(affirmationStr, semMem, lingDb));
    ONSEM_FEEDBACK_EQ("Ok, you want to hear a fun story about him.", operator_react("yes", semMem, lingDb));
    ONSEM_NOANSWER(operator_answer(affirmationStr, semMem, lingDb));
    ONSEM_ANSWER_EQ(answerStr, triggers_match(affirmationStr, semMem, lingDb));
  }

  // sentence not completed
  {
    const std::string affirmationStr = "je dis que";
    ONSEM_FEEDBACK_EQ("Ok, tu dis.",
                      operator_react(affirmationStr, semMem, lingDb));
    const std::string answerStr = "Oui, que dis-tu ?";
    triggers_add(affirmationStr, answerStr, semMem, lingDb);
    ONSEM_ANSWER_EQ(answerStr, operator_react(affirmationStr, semMem, lingDb));
    ONSEM_ANSWER_EQ(answerStr, triggers_match(affirmationStr, semMem, lingDb));
  }

  // affirmation trigger + question trigger
  {
    const std::string partOfTriggerStr = "Dede is a nice guy.";
    const std::string triggerStr = "Do you like to jump? " + partOfTriggerStr;
    const std::string customAnswerStr = "I am a nice chatbot.";
    ONSEM_ANSWERNOTFOUND_EQ("I don't know if I like to jump.",
                            operator_answer("Do you like to jump?", semMem, lingDb));
    triggers_add(triggerStr, customAnswerStr, semMem, lingDb);
    ONSEM_ANSWER_EQ(customAnswerStr, operator_react(triggerStr, semMem, lingDb));
    ONSEM_ANSWER_EQ(customAnswerStr, operator_answer(triggerStr, semMem, lingDb));
    ONSEM_ANSWER_EQ(customAnswerStr, triggers_match(triggerStr, semMem, lingDb));
    ONSEM_NOANSWER(operator_answer(partOfTriggerStr, semMem, lingDb));
    ONSEM_NOANSWER(triggers_match(partOfTriggerStr, semMem, lingDb));
  }

  // order trigger
  {
    const std::string order1Str = "Donne-moi ta main";
    ONSEM_BEHAVIORNOTFOUND_EQ("Je ne sais pas te donner ma main.",
                              operator_react(order1Str, semMem, lingDb));
    triggers_add(order1Str, "Laquelle", semMem, lingDb);
    ONSEM_BEHAVIOR_EQ("Lequel ?", operator_react(order1Str, semMem, lingDb));
    const std::string order2Str = "Donne-moi ta main mouillée";
    ONSEM_BEHAVIORNOTFOUND_EQ("Je ne sais pas te donner ma main mouillée.",
                              operator_react(order2Str, semMem, lingDb));
    triggers_add(order2Str, "Ok la voilà", semMem, lingDb);
    ONSEM_BEHAVIOR_EQ("Ok. Le voilà", operator_react(order2Str, semMem, lingDb));
    ONSEM_BEHAVIOR_EQ("Ok. Le voilà", triggers_match(order2Str, semMem, lingDb));
  }

  // infinitive trigger
  {
    const std::string inStr = "Accéder à mon espace Carrefour dédié";
    ONSEM_NOANSWER(operator_react(inStr, semMem, lingDb));
    triggers_add(inStr, "Voilà votre espace personnel", semMem, lingDb);
    const std::string answerStr = "Voilà ton espace personnel";
    ONSEM_ANSWER_EQ(answerStr, operator_react(inStr, semMem, lingDb));
    ONSEM_ANSWER_EQ(answerStr, triggers_match(inStr, semMem, lingDb));
  }

  // nominal group trigger
  {
    const std::string inStr = "Mon espace Carrefour dédié";
    ONSEM_QUESTION_EQ("Quel est ton espace Carrefour dédié ?",
                      operator_react(inStr, semMem, lingDb, SemanticLanguageEnum::UNKNOWN,
                                     &canReactToANoun));
    ONSEM_NOANSWER(operator_react("je ne sais pas", semMem, lingDb));
    const std::string answerStr = "Le service est pour le moment inaccessible.";
    triggers_add(inStr, answerStr, semMem, lingDb);
    ONSEM_ANSWER_EQ(answerStr, operator_react(inStr, semMem, lingDb));
    ONSEM_ANSWER_EQ(answerStr, triggers_match(inStr, semMem, lingDb));
  }

  // nominal corss language trigger
  {
    const std::string inStr = "Je veux une assurance habitation";
    const std::string frAnswerStr = "C'est cher.";
    const std::string enAnswerStr = "It's dear.";
    triggers_add(inStr, frAnswerStr, semMem, lingDb);
    ONSEM_ANSWER_EQ(frAnswerStr, operator_react(inStr, semMem, lingDb));
    ONSEM_ANSWER_EQ(frAnswerStr, triggers_match(inStr, semMem, lingDb));
    const std::string enTrigger = "I want a home insurance";
    ONSEM_ANSWER_EQ(enAnswerStr, operator_react(enTrigger, semMem, lingDb));
    ONSEM_ANSWER_EQ(enAnswerStr, triggers_match(enTrigger, semMem, lingDb));
  }

  // condition trigger
  {
    const std::string triggerStr = "Si tu veux, regarde à droite";
    ONSEM_BEHAVIORNOTFOUND_EQ("Je ne sais pas regarder à droite.",
                              operator_react(triggerStr, semMem, lingDb));
    const std::string answerStr = "Je ne peux pas. J'ai mal au coup.";
    triggers_add(triggerStr, answerStr, semMem, lingDb);
    ONSEM_ANSWER_EQ(answerStr, operator_react(triggerStr, semMem, lingDb));
    ONSEM_ANSWER_EQ(answerStr, triggers_match(triggerStr, semMem, lingDb));
  }

  // present conditional trigger
  {
    const std::string triggerStr = "Je veux";
    const std::string answerStr = "Je suis content pour toi.";
    triggers_add(triggerStr, answerStr, semMem, lingDb);
    ONSEM_ANSWER_EQ(answerStr, operator_react("Je voudrais", semMem, lingDb));
    ONSEM_ANSWER_EQ(answerStr, triggers_match("Je voudrais", semMem, lingDb));
  }

  // trigger from context
  {
    const std::string triggerStr = "Je ne sais pas quelle est la différence entre un amant et un mari.";
    const std::string answerStr = "C'est le jour et la nuit.";
    triggers_add(triggerStr, answerStr, semMem, lingDb);
    memoryOperation::learnSayCommand(semMem, lingDb);
    ONSEM_BEHAVIOR_EQ("Quelle est la différence entre un amant et un mari ?",
                      operator_react("demande quelle est la différence entre un amant et un mari", semMem, lingDb));
    ONSEM_ANSWER_EQ(answerStr, operator_react("je ne sais pas", semMem, lingDb));
  }

  // action trigger
  {
    const std::string triggerStr = "lance akinator";
    const std::string answerStr = "Je lance akinatorApp.";
    triggers_add(triggerStr, answerStr, semMem, lingDb);
    memoryOperation::learnSayCommand(semMem, lingDb);
    ONSEM_BEHAVIOR_EQ(answerStr, operator_react(triggerStr, semMem, lingDb));
    ONSEM_BEHAVIOR_EQ(answerStr, triggers_match(triggerStr, semMem, lingDb));
    ONSEM_BEHAVIOR_EQ(answerStr, operator_react("démarre akinator", semMem, lingDb));
    ONSEM_BEHAVIOR_EQ(answerStr, operator_react("Je veux que tu lances akinator", semMem, lingDb));
    const auto toLaunchAkinator = "Lancer akinator";
    ONSEM_FEEDBACK_EQ("C'est pas faux.", operator_react(toLaunchAkinator, semMem, lingDb));
    triggers_addOtherTriggerFormulations(triggerStr, answerStr, semMem, lingDb);
    ONSEM_ANSWER_EQ(answerStr, operator_react(toLaunchAkinator, semMem, lingDb));
    {
      std::stringstream ss;
      ss << "<dictionary_modification language=\"french\">\n"
         << "  <word lemma=\"paul\" pos=\"interjection\">\n"
         << "    <inflectedWord />\n"
         << "  </word>\n"
         << "</dictionary_modification>";
      lingDb.addDynamicContent(ss);
    }
    ONSEM_BEHAVIOR_EQ(answerStr, operator_react("Paul lance akinator", semMem, lingDb));
    ONSEM_BEHAVIOR_EQ(answerStr, triggers_match("Paul lance akinator", semMem, lingDb));
  }

  // action trigger 2
  {
    const std::string triggerStr = "suis moi";
    const std::string answerStr = "Je te suis mon ami.";
    triggers_add(triggerStr, answerStr, semMem, lingDb);
    memoryOperation::learnSayCommand(semMem, lingDb);
    ONSEM_BEHAVIOR_EQ(answerStr, operator_react(triggerStr, semMem, lingDb));
    ONSEM_BEHAVIOR_EQ(answerStr, triggers_match(triggerStr, semMem, lingDb));
    ONSEM_BEHAVIOR_EQ(answerStr, operator_react("je veux que tu me suives", semMem, lingDb));
  }

  // negative action trigger
  {
    const std::string triggerStr = "ne bouge pas";
    const std::string answerStr = "Je suis une statue.";
    triggers_add(triggerStr, answerStr, semMem, lingDb);
    ONSEM_BEHAVIOR_EQ(answerStr, operator_react(triggerStr, semMem, lingDb));
    ONSEM_BEHAVIOR_EQ(answerStr, triggers_match(triggerStr, semMem, lingDb));
  }

  // other concept
  {
    const std::string triggerStr = "raconte   une autre blague";
    const std::string answerStr = "c'est une tomate qui traverse et ça fait paf la tomate";
    triggers_add("raconte une blague", answerStr, semMem, lingDb);
    triggers_add("raconte une autre blague", answerStr, semMem, lingDb);
    const std::string answerReformulatedStr = "C'est une tomate traverse et ça fait paf la tomate.";
    //ONSEM_BEHAVIOR_EQ(answerReformulatedStr, operator_react("raconte   une autre blague", semMem, lingDb));
    ONSEM_BEHAVIOR_EQ(answerReformulatedStr, operator_react("raconte   une blague", semMem, lingDb));
    ONSEM_BEHAVIOR_EQ(answerReformulatedStr, operator_react("raconte   une autre blague", semMem, lingDb));
    ONSEM_BEHAVIOR_EQ(answerReformulatedStr, triggers_match("raconte   une blague", semMem, lingDb));
    ONSEM_BEHAVIOR_EQ(answerReformulatedStr, triggers_match("raconte   une autre blague", semMem, lingDb));
  }

  {
    const std::string triggerStr = "Montre quand tu es heureux";
    const std::string answerStr = "Hiii";
    triggers_add(triggerStr, answerStr, semMem, lingDb);
    ONSEM_ANSWER_EQ(answerStr, operator_react(triggerStr, semMem, lingDb));
    ONSEM_ANSWER_EQ(answerStr, triggers_match(triggerStr, semMem, lingDb));
  }

  {
    const std::string triggerStr = "Sois énervé";
    const std::string answerStr = "Tu es chiant. J'ai envie d'être heureux.";
    triggers_add(triggerStr, answerStr, semMem, lingDb);
    ONSEM_BEHAVIOR_EQ(answerStr, operator_react(triggerStr, semMem, lingDb));
    ONSEM_BEHAVIOR_EQ(answerStr, triggers_match(triggerStr, semMem, lingDb));
  }

  {
    const std::string trigger1Str = "Mets le  son  plus fort";
    const std::string answer1Str = "J'augmente le volume.";
    triggers_add(trigger1Str, answer1Str, semMem, lingDb);
    const std::string trigger2Str = "Mets le  son  moins fort";
    const std::string answer2Str = "Je diminue le volume.";
    triggers_add(trigger2Str, answer2Str, semMem, lingDb);
    ONSEM_BEHAVIOR_EQ(answer1Str, operator_react(trigger1Str, semMem, lingDb));
    ONSEM_BEHAVIOR_EQ(answer1Str, triggers_match(trigger1Str, semMem, lingDb));
    ONSEM_BEHAVIOR_EQ(answer2Str, operator_react(trigger2Str, semMem, lingDb));
    ONSEM_BEHAVIOR_EQ(answer2Str, triggers_match(trigger2Str, semMem, lingDb));
  }

  {
    const std::string triggerStr = "Baisse la température";
    const std::string answerStr = "La température est basse maintenant.";
    triggers_add(triggerStr, answerStr, semMem, lingDb);
    const std::string triggerToTestStr = "Baisse encore la température";
    ONSEM_BEHAVIOR_EQ(answerStr, operator_react(triggerToTestStr, semMem, lingDb));
    ONSEM_BEHAVIOR_EQ(answerStr, triggers_match(triggerToTestStr, semMem, lingDb));
  }

  {
    const std::string triggerStr = "Peux-tu nous raconter une blague";
    const std::string answerStr = "Je n'ai pas beaucoup de blagues.";
    triggers_add(triggerStr, answerStr, semMem, lingDb);
    const std::string triggerToTestStr = "Peux-tu nous raconter une autre blague";
    ONSEM_ANSWER_EQ(answerStr, operator_react(triggerToTestStr, semMem, lingDb));
    ONSEM_ANSWER_EQ(answerStr, triggers_match(triggerToTestStr, semMem, lingDb));
  }

  // not undertood
  {
    const std::string triggerStr = "tu faim"; // we expect this sentence to be not understood
    const std::string answerStr = "Ok, tu as faim.";
    triggers_add(triggerStr, answerStr, semMem, lingDb);
    ONSEM_ANSWER_EQ(answerStr, operator_react(triggerStr, semMem, lingDb));
    ONSEM_ANSWER_EQ(answerStr, triggers_match(triggerStr, semMem, lingDb));
  }

  // Triggers are more important than feedback reactions
  {
    const std::string answerStr = "Salut. Comment vas-tu ?";
    triggers_add("salut Talala", answerStr, semMem, lingDb);
    operator_inform("tu es Talala", semMem, lingDb);
    ONSEM_ANSWER_EQ(answerStr, operator_react("bonjour Talala", semMem, lingDb));
  }

  // "I want you to" use case
  {
    const std::string triggerStr = "Je voudrais que tu danses";
    const std::string answerStr = "Ok, je danse.";
    triggers_add(triggerStr, answerStr, semMem, lingDb);
    ONSEM_ANSWER_EQ(answerStr, operator_react(triggerStr, semMem, lingDb));
    ONSEM_ANSWER_EQ(answerStr, triggers_match(triggerStr, semMem, lingDb));
  }
}


TEST_F(SemanticReasonerGTests, operator_addATrigger_priorityOverSpecifications)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;

  {
    const std::string trigger1Str = "Raconte une histoire";
    const std::string answer1Str = "Je te dis un truc.";
    const std::string trigger2Str = "Raconte une histoire triste";
    const std::string answer2Str = "J'ai perdu mon vélo.";
    triggers_add(trigger1Str, answer1Str, semMem, lingDb);
    ONSEM_BEHAVIOR_EQ(answer1Str, operator_react(trigger1Str, semMem, lingDb));
    ONSEM_BEHAVIOR_EQ(answer1Str, triggers_match(trigger1Str, semMem, lingDb));
    ONSEM_BEHAVIOR_EQ(answer1Str, operator_react(trigger2Str, semMem, lingDb));
    ONSEM_BEHAVIOR_EQ(answer1Str, triggers_match(trigger2Str, semMem, lingDb));
    ONSEM_BEHAVIOR_EQ(answer1Str, triggers_match("Raconte une autre histoire", semMem, lingDb));
    ONSEM_BEHAVIOR_EQ(answer1Str, triggers_match("Raconte moi une histoire", semMem, lingDb));
    triggers_add(trigger2Str, answer2Str, semMem, lingDb);
    ONSEM_BEHAVIOR_EQ(answer1Str, operator_react(trigger1Str, semMem, lingDb));
    ONSEM_BEHAVIOR_EQ(answer1Str, triggers_match(trigger1Str, semMem, lingDb));
    ONSEM_BEHAVIOR_EQ(answer2Str, operator_react(trigger2Str, semMem, lingDb));
    ONSEM_BEHAVIOR_EQ(answer2Str, triggers_match(trigger2Str, semMem, lingDb));
    ONSEM_BEHAVIOR_EQ(answer1Str, triggers_match("Raconte une autre histoire", semMem, lingDb));
    ONSEM_BEHAVIOR_EQ(answer1Str, triggers_match("Raconte moi une histoire", semMem, lingDb));
    ONSEM_BEHAVIOR_EQ(answer2Str, triggers_match("Raconte moi une histoire triste", semMem, lingDb));
  }

  {
    const std::string trigger1Str = "Parle plus fort";
    const std::string answer1Str = "J'augmente le volume.";
    const std::string trigger2Str = "Parle moins fort";
    const std::string answer2Str = "Je diminue le volume.";
    triggers_add(trigger1Str, answer1Str, semMem, lingDb);
    triggers_add(trigger2Str, answer2Str, semMem, lingDb);
    ONSEM_BEHAVIOR_EQ(answer1Str, operator_react(trigger1Str, semMem, lingDb));
    ONSEM_BEHAVIOR_EQ(answer1Str, triggers_match(trigger1Str, semMem, lingDb));
    ONSEM_BEHAVIOR_EQ(answer2Str, operator_react(trigger2Str, semMem, lingDb));
    ONSEM_BEHAVIOR_EQ(answer2Str, triggers_match(trigger2Str, semMem, lingDb));
    ONSEM_BEHAVIOR_EQ(answer1Str, triggers_match("Parle encore plus fort", semMem, lingDb));
    ONSEM_BEHAVIOR_EQ(answer2Str, triggers_match("Parle encore moins fort", semMem, lingDb));
  }
}


TEST_F(SemanticReasonerGTests, operator_addATrigger_checkReferences)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;

  const std::string howCanITeachYou = "how can I teach you";
  const std::list<std::string> debugReferences{"/path/file.txt", "line 12"};
  triggers_add(howCanITeachYou,
                       "for example, you can say \"to salute is to say hello\"", semMem, lingDb,
                       debugReferences);
  compareWithRef("For example, you can say \"to salute is to say hello\".", debugReferences,
                 operator_react(howCanITeachYou, semMem, lingDb));
}


TEST_F(SemanticReasonerGTests, operator_addATrigger_subMemory_question)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  static const std::string questionsStr = "Comment t'apprendre quelque chose?";
  static const std::string customAnswerStr = "Essaie de me dire quelque chose !";
  SemanticMemory subSemMem;
  triggers_add(questionsStr, customAnswerStr, subSemMem, lingDb);

  SemanticMemory semMem;
  ONSEM_ANSWERNOTFOUND_EQ("Je ne sais pas comment m'apprendre quelque chose.",
                          operator_react(questionsStr, semMem, lingDb));
  semMem.memBloc.subBlockPtr = &subSemMem.memBloc;
  ONSEM_ANSWER_EQ(customAnswerStr, operator_react(questionsStr, semMem, lingDb));
  // 2 levels of depth
  {
    SemanticMemory semMemTopLevel;
    semMemTopLevel.memBloc.subBlockPtr = &semMem.memBloc;
    ONSEM_ANSWER_EQ(customAnswerStr, operator_react(questionsStr, semMemTopLevel, lingDb));
  }
}


TEST_F(SemanticReasonerGTests, operator_addATrigger_subMemory_affirmation)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  static const std::string affirmationStr = "Paul est content";
  SemanticMemory subSemMem;
  triggers_add(affirmationStr, "Effectivement, demain c'est son anniversaire", subSemMem, lingDb);

  SemanticMemory semMem;
  ONSEM_NOANSWER(operator_react(affirmationStr, semMem, lingDb));
  semMem.memBloc.subBlockPtr = &subSemMem.memBloc;
  ONSEM_ANSWER_EQ("Effectivement. Demain. C'est son anniversaire.",
                  operator_react(affirmationStr, semMem, lingDb));
  // 2 levels of depth
  {
    SemanticMemory semMemTopLevel;
    semMemTopLevel.memBloc.subBlockPtr = &semMem.memBloc;
    ONSEM_ANSWER_EQ("Effectivement. Demain. C'est son anniversaire.",
                    operator_react(affirmationStr, semMemTopLevel, lingDb));
  }
}


TEST_F(SemanticReasonerGTests, operator_addATrigger_subMemory_nominalGroup)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  ReactionOptions canReactToANoun;
  canReactToANoun.canReactToANoun = true;
  static const std::string propernounStr = "Paul";
  static const std::string nounStr = "la Box du mois";
  SemanticMemory subSemMem;
  triggers_add(propernounStr, "Demain c'est son anniversaire", subSemMem, lingDb);
  triggers_add("c'est quoi " + nounStr, "C'est des nems", subSemMem, lingDb);

  SemanticMemory semMem;
  semMem.defaultLanguage = SemanticLanguageEnum::FRENCH;
  ONSEM_QUESTION_EQ("Qui est Paul ?", operator_react(propernounStr, semMem, lingDb, SemanticLanguageEnum::UNKNOWN,
                                                     &canReactToANoun));
  ONSEM_NOANSWER(operator_react("Je ne sais pas", semMem, lingDb));
  semMem.memBloc.subBlockPtr = &subSemMem.memBloc;
  ONSEM_ANSWER_EQ("Demain. C'est son anniversaire.",
                  operator_react(propernounStr, semMem, lingDb, SemanticLanguageEnum::UNKNOWN,
                                 &canReactToANoun));
  ONSEM_ANSWER_EQ("C'est des nems.",
                  operator_react(nounStr, semMem, lingDb, SemanticLanguageEnum::UNKNOWN,
                                 &canReactToANoun));

  // 2 levels of depth
  {
    SemanticMemory semMemTopLevel;
    semMem.defaultLanguage = SemanticLanguageEnum::ENGLISH;
    semMemTopLevel.memBloc.subBlockPtr = &semMem.memBloc;
    ONSEM_ANSWER_EQ("Tomorrow. It's his birthday.",
                    operator_react(propernounStr, semMemTopLevel, lingDb, SemanticLanguageEnum::UNKNOWN,
                                   &canReactToANoun));
  }
}



TEST_F(SemanticReasonerGTests, operator_reply_by_the_trigger_the_closer_to_the_input)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  static const std::string affirmation1Str = "Paul1 est content";
  static const std::string affirmation2Str = "Paul2 est content";
  static const std::string questionStr = "qui est Paul1";
  SemanticMemory semMem;
  operator_mergeAndInform("Paul1 est Paul2", semMem, lingDb);
  triggers_add(affirmation1Str, "Réponse1", semMem, lingDb);
  triggers_add(affirmation2Str, "Réponse2", semMem, lingDb);
  triggers_add(questionStr, "Réponse3", semMem, lingDb);

  ONSEM_UNKNOWN(operator_check(affirmation2Str, semMem, lingDb));
  ONSEM_ANSWER_EQ("Réponse1", operator_react(affirmation1Str, semMem, lingDb));
  ONSEM_TRUE(operator_check(affirmation2Str, semMem, lingDb));
  ONSEM_ANSWER_EQ("Réponse2", operator_react(affirmation2Str, semMem, lingDb));
  ONSEM_ANSWER_EQ("Réponse3", operator_react(questionStr, semMem, lingDb));
}



TEST_F(SemanticReasonerGTests, operator_trigger_with_a_coreference)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  static const std::string affirmation1Str = "Paul1 est content";
  SemanticMemory semMem;
  triggers_add("Ils portent un bracelet", "Réponse1", semMem, lingDb);

  ONSEM_ANSWER_EQ("Réponse1", operator_react("Ils portent un bracelet", semMem, lingDb));
  triggers_add("Paul porte un masque", "Réponse2", semMem, lingDb);
  ONSEM_ANSWER_EQ("Réponse1", operator_react("Ils portent un bracelet", semMem, lingDb));
  ONSEM_ANSWER_EQ("Réponse2", operator_react("Paul porte un masque", semMem, lingDb));
  ONSEM_NOANSWER(operator_react("Elles portent un masque", semMem, lingDb));
  semMem.clear();
  triggers_add("Paul porte un masque", "Réponse2", semMem, lingDb);
  ONSEM_NOANSWER(operator_react("Elles portent un masque", semMem, lingDb));
}



TEST_F(SemanticReasonerGTests, operator_react_only_one_time_if_2_similar_trigger)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  static const std::string affirmation1Str = "Paul1 est content";
  static const std::string affirmation2Str = "Paul2 est content";
  static const std::string questionStr = "qui est Paul1";
  SemanticMemory semMem;
  operator_mergeAndInform("Paul1 est Paul2", semMem, lingDb);
  triggers_add(affirmation1Str, "Réponse1", semMem, lingDb);
  triggers_add(affirmation2Str, "Réponse2", semMem, lingDb);
  triggers_add(questionStr, "Réponse3", semMem, lingDb);

  ONSEM_UNKNOWN(operator_check(affirmation2Str, semMem, lingDb));
  ONSEM_ANSWER_EQ("Réponse1", operator_react(affirmation1Str, semMem, lingDb));
  ONSEM_TRUE(operator_check(affirmation2Str, semMem, lingDb));
  ONSEM_ANSWER_EQ("Réponse2", operator_react(affirmation2Str, semMem, lingDb));
  ONSEM_ANSWER_EQ("Réponse3", operator_react(questionStr, semMem, lingDb));
}


TEST_F(SemanticReasonerGTests, operator_trigger_with_relative_time)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  onsem::SemanticTimeGrounding::setAnHardCodedTimeElts(true, false);
  static const std::string questionStr = "Quelle est la blague d'aujourd'hui ?";
  static const std::string answerStr = "Blague1";
  SemanticMemory semMem;
  triggers_add(questionStr, answerStr, semMem, lingDb);
  onsem::SemanticTimeGrounding::setAnHardCodedTimeElts(false, false);
  ONSEM_ANSWER_EQ(answerStr, operator_react(questionStr, semMem, lingDb));
  onsem::SemanticTimeGrounding::setAnHardCodedTimeElts(true, false);

  static const std::string question2Str = "Y a-t-il encore des moulins aujourd'hui ?";
  static const std::string answer2Str = "Answer2";
  triggers_add(question2Str, answer2Str, semMem, lingDb);
  onsem::SemanticTimeGrounding::setAnHardCodedTimeElts(false, false);
  ONSEM_ANSWER_EQ(answer2Str, operator_react(question2Str, semMem, lingDb));
}
