#include "operator_answer.hpp"
#include "operator_inform.hpp"
#include <gtest/gtest.h>
#include <onsem/texttosemantic/tool/semexpgetter.hpp>
#include <onsem/texttosemantic/languagedetector.hpp>
#include <onsem/semantictotext/semanticconverter.hpp>
#include <onsem/semantictotext/semexpoperators.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemory.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/groundedexpression.hpp>
#include "../../semanticreasonergtests.hpp"
#include <onsem/tester/reactOnTexts.hpp>


namespace onsem
{

DetailedReactionAnswer operator_answer(const std::string& pText,
                                       const SemanticMemory& pSemanticMemory,
                                       const linguistics::LinguisticDatabase& lingDb,
                                       bool pCanAnswerIDontKnow)
{
  SemanticLanguageEnum language = linguistics::getLanguage(pText, lingDb);
  TextProcessingContext textProcContext(SemanticAgentGrounding::currentUser,
                                        SemanticAgentGrounding::me,
                                        language);
  auto semExp =
      converter::textToContextualSemExp(pText, textProcContext,
                                        SemanticSourceEnum::WRITTENTEXT, lingDb);
  memoryOperation::resolveAgentAccordingToTheContext(semExp, pSemanticMemory, lingDb);
  auto answerSemExp = memoryOperation::answer(std::move(semExp), pCanAnswerIDontKnow, pSemanticMemory, lingDb);
  DetailedReactionAnswer res;
  if (answerSemExp)
  {
    res.reactionType = SemExpGetter::extractContextualAnnotation(**answerSemExp);
    converter::semExpToText(res.answer, std::move(*answerSemExp),
                            TextProcessingContext(SemanticAgentGrounding::me,
                                                  SemanticAgentGrounding::currentUser,
                                                  language),
                            false, pSemanticMemory, lingDb, nullptr);
  }
  return res;
}

} // End of namespace onsem


using namespace onsem;


TEST_F(SemanticReasonerGTests, operator_answer_basic)
{
  SemanticMemory semMem;
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  std::map<const SemanticContextAxiom*, TruenessValue> axiomToConditionCurrentState;

  ONSEM_NOANSWER(operator_answer("You are on the floor", semMem, lingDb));
  ONSEM_ANSWERNOTFOUND_EQ("Je ne sais pas si j'aime le chocolat.", operator_answer("tu aimes le chocolat ?", semMem, lingDb));
  operator_inform("tu aimes le chocolat", semMem, lingDb);
  ONSEM_ANSWER_EQ("Oui, j'aime le chocolat.", operator_answer("tu aimes le chocolat ?", semMem, lingDb));
  ONSEM_ANSWER_EQ("Oui, j'aime le chocolat.", operator_answer("tu aimes le chocolat ou quoi", semMem, lingDb));
  ONSEM_ANSWERNOTFOUND_EQ("Je ne sais pas si je suis sympa.", operator_answer("tu es sympa ou pas", semMem, lingDb));
  ONSEM_ANSWERNOTFOUND_EQ("I don't know what my name is.", operator_answer("what is your name", semMem, lingDb));
  ONSEM_NOANSWER(operator_answer("tu aimes le chocolat", semMem, lingDb));

  // get answer from a subject (check some question transformations)
  {
    operator_inform("\\" + resourceLabelForTests_url +
                    "=https://www.youtube.com/v/4QpjSWndmqA\\ is the life of Gustave.", semMem, lingDb);
    ONSEM_ANSWER_EQ("Gustave's life is https://www.youtube.com/v/4QpjSWndmqA",
                    operator_answer("what is the life of Gustave?", semMem, lingDb));
    ONSEM_ANSWERNOTFOUND_EQ("I don't know what Gustavo's life is.", operator_answer("what is the life of Gustavo?", semMem, lingDb));
  }

  // get answer from a subject with merge with context
  {
    operator_inform("\\" + resourceLabelForTests_url + "=/path/to/raise_left_arm\\", semMem, lingDb);
    operator_mergeAndInform("It means to raise the left arm", semMem, lingDb);
    ONSEM_ANSWER_EQ("/path/to/raise_left_arm",
                    operator_answer("what does it mean to raise the left arm?", semMem, lingDb));
  }

  // answer agent info from a media
  {
    operator_inform("André Thunberg is \\" + resourceLabelForTests_url + "=https://www.youtube.com/v/5_ncIrrM8S0\\.",
                    semMem, lingDb);
    ONSEM_ANSWER_EQ("André Thunberg is https://www.youtube.com/v/5_ncIrrM8S0", operator_answer("Who is André Thunberg", semMem, lingDb));

    operator_inform("\\" + resourceLabelForTests_url + "=https://www.youtube.com/v/U51us-be7GQ\\ is Lauren.",
                    semMem, lingDb);
    ONSEM_ANSWER_EQ("Lauren is https://www.youtube.com/v/U51us-be7GQ", operator_answer("who is Lauren?", semMem, lingDb));
  }

  // common question answers
  {
    operator_inform("my taylor is rich", semMem, lingDb);
    operator_inform("Brian is in the kitchen", semMem, lingDb);
    ONSEM_ANSWER_EQ("Your Taylor", operator_answer("who is rich?", semMem, lingDb));
    ONSEM_ANSWER_EQ("Brian is in the kitchen.", operator_answer("where is Brian?", semMem, lingDb));
    ONSEM_ANSWER_EQ("Brian", operator_answer("who is in the kitchen?", semMem, lingDb));
  }

  // answer from the subject of a question
  {
    ONSEM_ANSWERNOTFOUND_EQ("I don't know what color your cat is.",
                            operator_answer("so what color is my cat", semMem, lingDb));
    operator_inform("my cat is blue", semMem, lingDb);
    ONSEM_ANSWER_EQ("Your cat is blue.", operator_answer("so what color is my cat", semMem, lingDb));
    ONSEM_ANSWER_EQ("La couleur de ton chat est bleue.", operator_answer("quel est la couleur de mon chat", semMem, lingDb));
  }

  // ask about negative stuffs
  {
    operator_inform("gustave eats chocolate", semMem, lingDb);
    ONSEM_ANSWER_EQ("Yes, Gustave eats something.", operator_answer("does gustave eat something?", semMem, lingDb));
    ONSEM_ANSWER_EQ("No, Gustave eats something.", operator_answer("does gustave eat nothing?", semMem, lingDb));
    operator_inform("gustave doesn't eat banana", semMem, lingDb);
    ONSEM_ANSWER_EQ("Gustave eats chocolate.", operator_answer("what does gustave eat?", semMem, lingDb));
    ONSEM_ANSWER_EQ("Gustave doesn't eat banana.", operator_answer("what does gustave not eat?", semMem, lingDb));
  }

  // take the verb tense into account
  {
    ONSEM_ANSWERNOTFOUND_EQ("I don't know what I do.", operator_answer("what do you do ?", semMem, lingDb));
    ONSEM_ANSWERNOTFOUND_EQ("I don't know what I did.", operator_answer("what did you do ?", semMem, lingDb));
    ONSEM_ANSWERNOTFOUND_EQ("I don't know what I will do.", operator_answer("what will you do ?", semMem, lingDb));

    operator_inform("you smile", semMem, lingDb);
    ONSEM_ANSWER_EQ("I smile.", operator_answer("what do you do ?", semMem, lingDb));
    ONSEM_ANSWERNOTFOUND_EQ("I don't know what I did.", operator_answer("what did you do ?", semMem, lingDb));
    ONSEM_ANSWERNOTFOUND_EQ("I don't know what I will do.", operator_answer("what will you do ?", semMem, lingDb));

    operator_inform("you looked at me", semMem, lingDb);
    ONSEM_ANSWER_EQ("I smile.", operator_answer("what do you do ?", semMem, lingDb));
    ONSEM_ANSWER_EQ("I looked at you.", operator_answer("what did you do ?", semMem, lingDb));
    ONSEM_ANSWERNOTFOUND_EQ("I don't know what I will do.", operator_answer("what will you do ?", semMem, lingDb));

    operator_inform("you will run", semMem, lingDb);
    ONSEM_ANSWER_EQ("I smile.", operator_answer("what do you do ?", semMem, lingDb));
    ONSEM_ANSWER_EQ("I looked at you.", operator_answer("what did you do ?", semMem, lingDb));
    ONSEM_ANSWER_EQ("I will run.", operator_answer("what will you do ?", semMem, lingDb));

    operator_inform("you will raise your left arm", semMem, lingDb);
    ONSEM_ANSWER_EQ("I smile.", operator_answer("what do you do ?", semMem, lingDb));
    ONSEM_ANSWER_EQ("I looked at you.", operator_answer("what did you do ?", semMem, lingDb));
    ONSEM_ANSWER_EQ("I will run and I will raise my left arm.", operator_answer("what will you do ?", semMem, lingDb));
  }

  // answer from a question with an apostrophe encoded on many bytes
  {
    operator_inform("Je vais faire une orgie", semMem, lingDb);
    ONSEM_ANSWER_EQ("Tu feras une orgie.", operator_answer("Qu’est-ce que je vais faire ?", semMem, lingDb));
  }

  // don't consider ability
  {
    operator_inform("Andrew can eat", semMem, lingDb);
    ONSEM_ANSWERNOTFOUND_EQ("I don't know if Andrew eats.", operator_answer("Does Andrew eat?", semMem, lingDb));
  }

  // answer from a purpose information
  {
    ONSEM_ANSWERNOTFOUND_EQ("Je ne sais pas pourquoi je suis sur terre.", operator_answer("pourquoi es-tu sur terre ?", semMem, lingDb));
    operator_inform("tu es sur terre pour aimer", semMem, lingDb);
    ONSEM_ANSWER_EQ("Je suis sur terre pour aimer.", operator_answer("pourquoi es-tu sur terre ?", semMem, lingDb));
    ONSEM_ANSWER_EQ("I am on earth in order to like.", operator_answer("why are you on earth ?", semMem, lingDb));
  }

  // yes or no answer with purpose difference
  {
    ONSEM_ANSWERNOTFOUND_EQ("Je ne sais pas si Paul est assis par terre pour manger.", operator_answer("Paul est assis par terre pour manger ?", semMem, lingDb));
    ONSEM_ANSWERNOTFOUND_EQ("Je ne sais pas si Paul est assis par terre pour jouer.", operator_answer("Paul est assis par terre pour jouer ?", semMem, lingDb));
    operator_inform("Paul est assis par terre pour jouer", semMem, lingDb);
    ONSEM_ANSWERNOTFOUND_EQ("Je ne sais pas si Paul est assis par terre pour manger.", operator_answer("Paul est assis par terre pour manger ?", semMem, lingDb));
    ONSEM_ANSWER_EQ("Oui, Paul est assis par terre pour jouer.", operator_answer("Paul est assis par terre pour jouer ?", semMem, lingDb));
  }

  // who answer
  {
    ONSEM_ANSWERNOTFOUND_EQ("Je ne sais pas qui habite à Lyon.", operator_answer("qui habite à Lyon ?", semMem, lingDb));
    operator_inform("Paul habite à Lyon", semMem, lingDb);
    ONSEM_ANSWER_EQ("Paul", operator_answer("qui habite à Lyon ?", semMem, lingDb));
    ONSEM_ANSWER_EQ("Paul", operator_answer("qui est-ce qui habite à Lyon ?", semMem, lingDb));
    ONSEM_ANSWER_EQ("Paul", operator_answer("qui est ce qui habite à Lyon ?", semMem, lingDb));
    ONSEM_ANSWER_EQ("Paul", operator_answer("who lives at Lyon ?", semMem, lingDb));
  }

  // what answer
  {
    ONSEM_ANSWERNOTFOUND_EQ("Je ne sais pas ce qui est blanc.", operator_answer("qu'est-ce qui est blanc ?", semMem, lingDb));
    operator_inform("Cet ordinateur est blanc.", semMem, lingDb);
    ONSEM_ANSWER_EQ("Cet ordinateur", operator_answer("qu'est-ce qui est blanc ?", semMem, lingDb));
    ONSEM_ANSWER_EQ("Cet ordinateur", operator_answer("qu'est ce qui est blanc ?", semMem, lingDb));
    ONSEM_ANSWER_EQ("This computer", operator_answer("what is white ?", semMem, lingDb));

    ONSEM_ANSWERNOTFOUND_EQ("Je ne sais pas ce que c'est une pomme.", operator_answer("qu'est-ce qu'une pomme ?", semMem, lingDb));
    static const std::string pommeEstUnFruit = "Une pomme est un fruit.";
    operator_inform(pommeEstUnFruit, semMem, lingDb);
    ONSEM_ANSWER_EQ(pommeEstUnFruit, operator_answer("qu'est-ce qu'une pomme ?", semMem, lingDb));
    ONSEM_ANSWER_EQ(pommeEstUnFruit, operator_answer("qu'est ce qu'une pomme ?", semMem, lingDb));
    ONSEM_ANSWER_EQ(pommeEstUnFruit, operator_answer("qu'est-ce que c'est une pomme ?", semMem, lingDb));
    ONSEM_ANSWER_EQ(pommeEstUnFruit, operator_answer("qu'est ce que c'est une pomme ?", semMem, lingDb));
    ONSEM_ANSWER_EQ(pommeEstUnFruit, operator_answer("une pomme c'est quoi ?", semMem, lingDb));
  }

  // cause answer
  {
    ONSEM_ANSWERNOTFOUND_EQ("Je ne sais pas pourquoi Paul est content.", operator_answer("Pourquoi Paul est content ?", semMem, lingDb));
    operator_inform("Paul est content parce que André lui a donné un cadeau", semMem, lingDb);
    ONSEM_ANSWER_EQ("Paul est content parce qu'André a donné un cadeau à Paul.",
                    operator_answer("Pourquoi Paul est content ?", semMem, lingDb));
    // check that have also been informed about the cause child
    ONSEM_ANSWER_EQ("André a donné un cadeau à Paul.",
                    operator_answer("Qu'est-ce qu'André a donné à Paul ?", semMem, lingDb));
  }

  // time answer
  {
    ONSEM_ANSWERNOTFOUND_EQ("Je ne sais pas quand tu es content.", operator_answer("quand je suis content ?", semMem, lingDb));
    operator_inform("je suis content quand tu souris", semMem, lingDb);
    ONSEM_ANSWER_EQ("Tu es content quand je souris.", operator_answer("quand je suis content ?", semMem, lingDb));
  }

  // duration answer
  {
    ONSEM_ANSWERNOTFOUND_EQ("I don't know how long the trip will last.",
                            operator_answer("How long will the trip last", semMem, lingDb));
    ONSEM_ANSWERNOTFOUND_EQ("Je ne sais pas combien de temps le voyage durera.",
                            operator_answer("Combien de temps va durer le voyage", semMem, lingDb));
    operator_inform("The trip will last 7 hours", semMem, lingDb);
    ONSEM_ANSWER_EQ("The trip will last 7 hours.",
                    operator_answer("How long will the trip last", semMem, lingDb));
    ONSEM_ANSWER_EQ("Le voyage durera 7 heures.",
                    operator_answer("Combien de temps va durer le voyage", semMem, lingDb));
  }

  // date time answer
  {
    operator_inform("En juin 1993, Paul a eu le bac.", semMem, lingDb);
    ONSEM_ANSWER_EQ("Paul a eu le bac en juin 1993.", operator_answer("Quand Paul a eu le bac ?", semMem, lingDb));
    operator_inform("Le 12 mai 1998, Stanislas a eu le bac.", semMem, lingDb);
    ONSEM_ANSWER_EQ("Stanislas a eu le bac le 12 mai 1998.", operator_answer("Quand Stanislas a eu le bac ?", semMem, lingDb));
    operator_inform("Le 42 mai 1990, Gustave a eu le bac.", semMem, lingDb); // test with a day out of range
    ONSEM_ANSWER_EQ("Gustave a eu le bac en mai 1990.", operator_answer("Quand Gustave a eu le bac ?", semMem, lingDb));
    operator_inform("Le 12 février, Théodore a eu le bac.", semMem, lingDb); // date without year
    ONSEM_ANSWER_EQ("Théodore a eu le bac le 12 février.", operator_answer("Quand Théodore a eu le bac ?", semMem, lingDb));
    operator_inform("Le 31 février 1998, Constantin a eu le bac.", semMem, lingDb); // day number between 1 and 31 but the date is wrong
    ONSEM_ANSWER_EQ("Constantin a eu le bac le 31 février 1998.", operator_answer("Quand Constantin a eu le bac ?", semMem, lingDb));
  }

  // location answer
  {
    ONSEM_ANSWERNOTFOUND_EQ("I don't know if you are on floor.", operator_answer("am I on the floor?", semMem, lingDb));
    operator_inform("je suis par terre", semMem, lingDb);
    ONSEM_ANSWER_EQ("Yes, you are on floor.", operator_answer("am I on the floor?", semMem, lingDb));
    operator_inform("I play football", semMem, lingDb);
    ONSEM_ANSWERNOTFOUND_EQ("I don't know where you play.", operator_answer("where do I play", semMem, lingDb));
    operator_inform("I go to Germany", semMem, lingDb);
    ONSEM_ANSWER_EQ("You go to Germany.", operator_answer("where do I go", semMem, lingDb));
    operator_inform("I eat in the kitchen", semMem, lingDb);
    ONSEM_ANSWER_EQ("You eat in the kitchen.", operator_answer("where do I eat", semMem, lingDb));
    operator_inform("Je mange à l'école", semMem, lingDb);
    ONSEM_ANSWER_EQ("Tu manges dans la cuisine et à l'école.", operator_answer("où est-ce que je mange", semMem, lingDb));
    operator_inform("Je travaille en France", semMem, lingDb);
    ONSEM_ANSWER_EQ("Tu travailles en France.", operator_answer("où est-ce que je travaille", semMem, lingDb));
    operator_inform("En Allemagne, Paul est connu", semMem, lingDb);
    ONSEM_ANSWER_EQ("Paul est connu en Allemagne.", operator_answer("où est-ce que Paul est connu", semMem, lingDb));
    // TODO: fix sentence "En France Guillaume est connu et Guillaume a gagné cette compétition"
    // TODO: fix sentence "En France, Guillaume est connu et Guillaume a gagné cette compétition"
  }

  // time and location answer
  {
    const std::string anneBirthCompleteInfosStr = "Anne est née en 1980 à Paris.";
    operator_inform(anneBirthCompleteInfosStr, semMem, lingDb);
    ONSEM_ANSWER_EQ("Anne est née en 1980.", operator_answer("Quand est née Anne ?", semMem, lingDb));
    ONSEM_ANSWER_EQ("Anne est née à Paris.", operator_answer("Où est née Anne ?", semMem, lingDb));
    ONSEM_ANSWER_EQ("Anne est née en 1980.", operator_answer("Quand Anne est née ?", semMem, lingDb));
    ONSEM_ANSWER_EQ("Anne est née à Paris.", operator_answer("Où Anne est née ?", semMem, lingDb));
    ONSEM_ANSWER_EQ(anneBirthCompleteInfosStr, operator_answer("Quand et où est née Anne ?", semMem, lingDb));
    ONSEM_ANSWER_EQ(anneBirthCompleteInfosStr, operator_answer("Quand et où Anne est née ?", semMem, lingDb));
  }

  // how answer + adverb as answer
  {
    ONSEM_ANSWERNOTFOUND_EQ("I don't know how your robot's battery is.", operator_answer("How is my robot's battery ?", semMem, lingDb));
    auto idSemExp = operator_inform("My robot's battery is nearly empty", semMem, lingDb);
    ONSEM_ANSWER_EQ("Your robot's battery is nearly empty.",
                    operator_answer("How is my robot's battery ?", semMem, lingDb));
    ONSEM_ANSWER_EQ("La batterie de ton robot est presque vide.",
                    operator_answer("Comment est la batterie de mon robot ?", semMem, lingDb));
    ONSEM_ANSWER_EQ("Your robot's battery is nearly empty.",
                    operator_answer("What is my robot's battery level ?", semMem, lingDb));
    semMem.memBloc.removeExpression(*idSemExp, lingDb, &axiomToConditionCurrentState);
    operator_inform("La batterie de mon robot est pleine", semMem, lingDb);
    ONSEM_ANSWER_EQ("Your robot's battery is full.",
                    operator_answer("What is my robot's battery level ?", semMem, lingDb));
  }

  // answer to question "do you know ..."
  {
    SemanticMemory semMem2;
    ONSEM_ANSWER_EQ("No, I don't know you.",
                    operator_answer("Do you know me ?", semMem2, lingDb));
    operator_inform("I am Renauld", semMem2, lingDb);
    ONSEM_ANSWER_EQ("Yes, I know you.",
                    operator_answer("Do you know me ?", semMem2, lingDb));
  }

  // answer to questions that the answer is inside the question
  {
    ONSEM_ANSWER_EQ("La couleur du cheval blanc d'Henri IV est blanche.",
                    operator_answer("quelle est la couleur du cheval blanc d'Henri IV", semMem, lingDb));
    ONSEM_ANSWER_EQ("The color of Henri IV's white horse is white.",
                    operator_answer("what is the color of the white horse of Henri IV", semMem, lingDb));
    ONSEM_ANSWERNOTFOUND_EQ("I don't know what the size of Henri IV's white horse is.",
                            operator_answer("what is the size of the white horse of Henri IV", semMem, lingDb));
  }

  // how many times quenstions
  {
    ONSEM_ANSWER_EQ("You didn't say you are touching an elephant.",
                    operator_answer("how many times did I say I am touching an elephant", semMem, lingDb));
    operator_inform("I am touching an elephant", semMem, lingDb);
    ONSEM_ANSWER_EQ("You said you are touching an elephant time.",
                    operator_answer("how many times did I say I am touching an elephant", semMem, lingDb));
    operator_inform("I am touching an elephant", semMem, lingDb);
    ONSEM_ANSWER_EQ("You said you are touching an elephant 2 times.",
                    operator_answer("how many times did I say I am touching an elephant", semMem, lingDb));
  }

  // answer even if the informed sentence is at passive form
  {
    operator_inform("your left hand is not touched.", semMem, lingDb);
    ONSEM_ANSWER_EQ("No, somebody doesn't touch my left hand.",
                    operator_answer("somebody touches your left hand ?", semMem, lingDb));
    ONSEM_ANSWER_EQ("Yes, somebody doesn't touch my left hand.",
                    operator_answer("somebody doesn't touch your left hand ?", semMem, lingDb));
  }

  // do you know question
  {
    ONSEM_ANSWER_EQ("No, I don't know any story.",
                    operator_answer("do you know a story", semMem, lingDb));
  }

  // infinitive question
  {
    ONSEM_ANSWERNOTFOUND_EQ("Je ne sais pas quelle position donner à cet objet.",
                            operator_answer("quelle position donner à cet objet", semMem, lingDb));
  }

  {
    operator_inform("I am 6", semMem, lingDb);
    ONSEM_ANSWER_EQ("You are 6.",
                    operator_answer("how old am I", semMem, lingDb));
    ONSEM_ANSWER_EQ("Your age is 6.",
                    operator_answer("what is my age", semMem, lingDb));
    ONSEM_ANSWER_EQ("Tu as 6 ans.",
                    operator_answer("quel est mon age", semMem, lingDb));
    ONSEM_ANSWERNOTFOUND_EQ("Je ne sais pas quel est mon age.",
                            operator_answer("quel est ton age", semMem, lingDb));
    operator_inform("if I am mistaken then I am 7", semMem, lingDb);
    ONSEM_ANSWER_EQ("You are 6.",
                    operator_answer("how old am I", semMem, lingDb));
    operator_inform("I am mistaken", semMem, lingDb);
    ONSEM_ANSWER_EQ("You are 7.",
                    operator_answer("how old am I", semMem, lingDb));
  }

  // owner synthesis
  {
    const std::string julietSoudWshHerHands = "Juliet should wash her hands.";
    operator_inform(julietSoudWshHerHands, semMem, lingDb);
    ONSEM_ANSWER_EQ(julietSoudWshHerHands,
                    operator_answer("What Juliet should do", semMem, lingDb));
    ONSEM_ANSWER_EQ("Juliet doit laver ses mains.",
                    operator_answer("Qu'est-ce que Juliet doit laver", semMem, lingDb));
  }

  // teach an action
  memoryOperation::allowToInformTheUserHowToTeach(semMem);
  ONSEM_ANSWER_EQ("For example, you can tell me to smile is to say I am smiling.",
                  operator_answer("how can I teach you to smile", semMem, lingDb));
  ONSEM_ANSWER_EQ("Par exemple, tu peux me dire que sourire c'est dire je souris.",
                  operator_answer("comment je peux t'apprendre à sourire", semMem, lingDb));
}


TEST_F(SemanticReasonerGTests, operator_answer_choice)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;

  {
    const std::string question = "I am a man or a woman?";
    ONSEM_ANSWERNOTFOUND_EQ("I don't know if you are a man or a woman.", operator_answer(question, semMem, lingDb));
    operator_inform("I am a man", semMem, lingDb);
    ONSEM_ANSWER_EQ("You are a man.", operator_answer(question, semMem, lingDb));
  }

  {
    const std::string question = "Are you a man or a woman?";
    ONSEM_ANSWERNOTFOUND_EQ("I don't know if I am a man or a woman.", operator_answer(question, semMem, lingDb));
    operator_inform("You are a robot", semMem, lingDb);
    ONSEM_ANSWER_EQ("Neither, I am a robot.", operator_answer(question, semMem, lingDb));
    ONSEM_ANSWER_EQ("Ni l'un ni l'autre, je suis un robot.", operator_answer("Es-tu un homme ou une femme", semMem, lingDb));
  }

  // question atnegative form
  {
    const std::string question = "Are you not sad?";
    ONSEM_ANSWERNOTFOUND_EQ("I don't know if I am not sad.", operator_answer(question, semMem, lingDb));
    operator_inform("You are sad", semMem, lingDb);
    ONSEM_ANSWER_EQ("No, I am sad.", operator_answer(question, semMem, lingDb));
  }

  {
    const std::string question = "Do I like chocolate or banana?";
    ONSEM_ANSWERNOTFOUND_EQ("I don't know if you like chocolate or banana.", operator_answer(question, semMem, lingDb));
    operator_inform("I don't like chocolate", semMem, lingDb);
    ONSEM_ANSWER_EQ("You don't like chocolate.", operator_answer(question, semMem, lingDb));
    operator_inform("I don't like banana", semMem, lingDb);
    ONSEM_ANSWER_EQ("Neither", operator_answer(question, semMem, lingDb));
    ONSEM_ANSWER_EQ("Ni l'un ni l'autre",
                    operator_answer("j'aime le chocolat ou la banane ?", semMem, lingDb));
    ONSEM_ANSWER_EQ("You don't like chocolate and banana.",
                    operator_answer("Do I like chocolate, butter or banana?", semMem, lingDb));
    operator_inform("I like butter", semMem, lingDb);
    ONSEM_ANSWER_EQ("You like butter.",
                    operator_answer("Do I like chocolate, butter or banana?", semMem, lingDb));
    operator_inform("I like banana", semMem, lingDb);
    ONSEM_ANSWER_EQ("You like butter and banana.",
                    operator_answer("Do I like chocolate, butter or banana?", semMem, lingDb));
    ONSEM_ANSWER_EQ("Both, you like butter and banana.",
                    operator_answer("Do I like butter or banana?", semMem, lingDb));
    operator_inform("I like chocolate", semMem, lingDb);
    ONSEM_ANSWER_EQ("All of them, you like chocolate, butter and banana.",
                    operator_answer("Do I like chocolate, butter or banana?", semMem, lingDb));
    ONSEM_ANSWER_EQ("Tous, tu aimes le chocolat, le beurre et la banane.",
                    operator_answer("j'aime le chocolat, le beurre ou la banane ?", semMem, lingDb));
    operator_inform("I don't like chocolate and banana", semMem, lingDb);
    operator_inform("I don't like butter", semMem, lingDb);
    ONSEM_ANSWER_EQ("None of them",
                    operator_answer("Do I like chocolate, butter or banana?", semMem, lingDb));
    ONSEM_ANSWER_EQ("Aucun d'entre eux",
                    operator_answer("j'aime le chocolat, le beurre ou la banane ?", semMem, lingDb));
  }
}


TEST_F(SemanticReasonerGTests, operator_answer_whatIsAction)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;

  // Say is not a composed action so instead of enumerating what compose this action
  // we run it and the execution of "say something" is "something"
  memoryOperation::learnSayCommand(semMem, lingDb);
  ONSEM_ANSWER_EQ("To say something is something.",
                  operator_answer("what is to say something?", semMem, lingDb));
  ONSEM_ANSWER_EQ("To repeat is to say the last thing that I said.",
                  operator_answer("what is to repeat?", semMem, lingDb));
  ONSEM_ANSWER_EQ("To repeat is to say the last thing that I said.",
                  operator_answer("what does it mean to repeat?", semMem, lingDb));
  ONSEM_ANSWER_EQ("To repeat is to say the last thing that I said.",
                  operator_answer("what means to repeat?", semMem, lingDb));
  ONSEM_ANSWER_EQ("Répéter c'est dire la dernière chose que j'ai dit.",
                  operator_answer("qu'est-ce que c'est répéter ?", semMem, lingDb));
  ONSEM_ANSWER_EQ("Répéter c'est dire la dernière chose que j'ai dit.",
                  operator_answer("qu'est-ce que ça veut dire répéter ?", semMem, lingDb));
  ONSEM_ANSWERNOTFOUND_EQ("I don't know what is to smile.",
                          operator_answer("what is to smile?", semMem, lingDb));
}


TEST_F(SemanticReasonerGTests, operator_answer_whitoutIdontKnowAnswer)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;

  ONSEM_NOANSWER(operator_answer("You are on the floor", semMem, lingDb, false));
  ONSEM_NOANSWER(operator_answer("Are you happy?", semMem, lingDb, false));
  ONSEM_ANSWERNOTFOUND_EQ("I don't know if I am happy.",
                          operator_answer("Are you happy?", semMem, lingDb, true));
  operator_inform("You are happy.", semMem, lingDb);
  ONSEM_ANSWER_EQ("Yes, I am happy.",
                  operator_answer("Are you happy?", semMem, lingDb, false));
  ONSEM_ANSWER_EQ("Yes, I am happy.",
                  operator_answer("Are you happy?", semMem, lingDb, true));
}



TEST_F(SemanticReasonerGTests, operator_answer_from_sub_part_of_a_date)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;
  operator_inform("Bart est né le 7 septembre 1988", semMem, lingDb);
  ONSEM_ANSWER_EQ("Bart",
                  operator_answer("qui est né le 7 septembre 1988", semMem, lingDb, true));
  ONSEM_ANSWER_EQ("Bart",
                  operator_answer("Qui est né en 1988", semMem, lingDb, true));
  ONSEM_ANSWER_EQ("Bart",
                  operator_answer("Qui est né en mille neuf cents quatre-vingts huit", semMem, lingDb, true));
  ONSEM_ANSWER_EQ("Bart",
                  operator_answer("Qui est né en septembre", semMem, lingDb, true));
  ONSEM_ANSWER_EQ("Bart",
                  operator_answer("Qui est né en septembre 1988", semMem, lingDb, true));
  ONSEM_ANSWERNOTFOUND_EQ("Je ne sais pas qui est né en 1987.",
                          operator_answer("Qui est né en 1987", semMem, lingDb, true));
  ONSEM_ANSWERNOTFOUND_EQ("Je ne sais pas qui est né en 1987.",
                          operator_answer("Qui est né en mille neuf cents quatre-vingts sept", semMem, lingDb, true));
  ONSEM_ANSWERNOTFOUND_EQ("Je ne sais pas qui est né en octobre.",
                          operator_answer("Qui est né en octobre", semMem, lingDb, true));
  ONSEM_ANSWERNOTFOUND_EQ("Je ne sais pas qui est né en septembre 1987.",
                          operator_answer("Qui est né en septembre 1987", semMem, lingDb, true));
  ONSEM_ANSWERNOTFOUND_EQ("Je ne sais pas qui est né en octobre 1988.",
                          operator_answer("Qui est né en octobre 1988", semMem, lingDb, true));

}


TEST_F(SemanticReasonerGTests, operator_answer_reply_composed_of_many_sentence_1)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;
  const std::string questionStr = "Bruno aime un informaticien ?";
  operator_inform("Bruno aime Nicolas", semMem, lingDb);
  ONSEM_NOANSWER(operator_answer(questionStr, semMem, lingDb, false));
  operator_inform("Nicolas est un informaticien", semMem, lingDb);
  ONSEM_ANSWER_EQ("Oui, Bruno aime un informaticien.",
                  operator_answer(questionStr, semMem, lingDb, true));
}


TEST_F(SemanticReasonerGTests, operator_answer_reply_composed_of_many_sentence_2)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;
  const std::string questionStr = "Bruno aime un informaticien ?";
  operator_inform("Nicolas est un informaticien", semMem, lingDb);
  ONSEM_NOANSWER(operator_answer(questionStr, semMem, lingDb, false));
  operator_inform("Bruno aime Nicolas", semMem, lingDb);
  ONSEM_ANSWER_EQ("Oui, Bruno aime un informaticien.",
                  operator_answer(questionStr, semMem, lingDb, true));
}


TEST_F(SemanticReasonerGTests, operator_answer_withAQuestionWithSubordinates)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;

  operator_inform("Antoine Griezmann, né le 21 mars 1991, est un footballeur.", semMem, lingDb);
  operator_inform("Danilo Luiz da Silva, dit Danilo, né le 15 juillet 1991 à Bicas, dans l'État de Minas Gerais au Brésil, est un footballeur international brésilien, évoluant au poste de latéral ou milieu droit à la Juventus.", semMem, lingDb);
  operator_inform("Jason Young, né le 21 mars 1991, est un athlète jamaïcain", semMem, lingDb);
  operator_inform("José Javier del Aguila (né le 7 mars 1991 au Guatemala) est un joueur de football international guatémaltèque, qui évolue au poste de milieu de terrain.", semMem, lingDb);
  operator_inform("Usain Bolt, né le 21 août 1986 dans la paroisse de Trelawny, est un athlète jamaïcain, spécialiste des épreuves de sprint.", semMem, lingDb);
  operator_inform("Zinédine Zidane, né le 23 juin 1972 à Marseille, est un footballeur international français", semMem, lingDb);
  ONSEM_ANSWER_EQ("Zinédine Zidane",
                  operator_answer("qui est un footballeur né le 23 juin 1972", semMem, lingDb, true));
  ONSEM_ANSWER_EQ("Antoine Griezmann, Danilo Luiz Da Silva, José Javier del Aguila et Zinédine Zidane",
                  operator_answer("qui est un footballeur", semMem, lingDb, true));
  ONSEM_ANSWER_EQ("Zinédine Zidane",
                  operator_answer("qui est né le 23 juin 1972", semMem, lingDb, true));
  ONSEM_ANSWER_EQ("Antoine Griezmann et Jason Young",
                  operator_answer("qui est né le 21 mars 1991", semMem, lingDb, true));
  ONSEM_ANSWER_EQ("Antoine Griezmann, Jason Young et José Javier del Aguila",
                  operator_answer("qui est né en mars 1991", semMem, lingDb, true));
  ONSEM_ANSWER_EQ("Antoine Griezmann et José Javier del Aguila",
                  operator_answer("qui est un footballeur né en mars 1991", semMem, lingDb, true));
  ONSEM_ANSWER_EQ("Antoine Griezmann",
                  operator_answer("qui est un footballeur né le 21 mars 1991", semMem, lingDb, true));
  ONSEM_ANSWER_EQ("Jason Young et Usain Bolt",
                  operator_answer("qui est un athlète", semMem, lingDb, true));
  ONSEM_ANSWER_EQ("Jason Young et Usain Bolt",
                  operator_answer("qui est un athlète jamaïcain", semMem, lingDb, true));
  ONSEM_ANSWER_EQ("Jason Young",
                  operator_answer("qui est un athlète né le 21 mars 1991", semMem, lingDb, true));
  ONSEM_ANSWER_EQ("Jason Young",
                  operator_answer("qui est un athlète jamaïcain né le 21 mars 1991", semMem, lingDb, true));
  ONSEM_ANSWER_EQ("Usain Bolt",
                  operator_answer("qui est un athlète jamaïcain né le 21 août 1986", semMem, lingDb, true));
  ONSEM_ANSWERNOTFOUND_EQ("Je ne sais pas qui est un athlète français.",
                          operator_answer("qui est un athlète français", semMem, lingDb, true));
  ONSEM_ANSWERNOTFOUND_EQ("Je ne sais pas qui est un athlète français né le 21 mars 1991.",
                          operator_answer("qui est un athlète français né le 21 mars 1991", semMem, lingDb, true));
  ONSEM_ANSWERNOTFOUND_EQ("Je ne sais pas qui est un footballeur né le 21 mars 1992.",
                          operator_answer("qui est un footballeur né le 21 mars 1992", semMem, lingDb, true));
  ONSEM_ANSWERNOTFOUND_EQ("Je ne sais pas si Toto aime un footballeur.",
                          operator_answer("Toto aime un footballeur ?", semMem, lingDb, true));
  ONSEM_ANSWERNOTFOUND_EQ("Je ne sais pas qui aime un footballeur.",
                          operator_answer("qui aime un footballeur", semMem, lingDb, true));
  ONSEM_ANSWERNOTFOUND_EQ("Je ne sais pas qui aime un footballeur né en mars 1991.",
                          operator_answer("qui aime un footballeur né en mars 1991", semMem, lingDb, true));
  operator_inform("Toto aime Antoine Griezmann", semMem, lingDb);
  ONSEM_ANSWER_EQ("Oui, Toto aime un footballeur.",
                  operator_answer("Toto aime un footballeur ?", semMem, lingDb, true));
  ONSEM_ANSWER_EQ("Toto",
                  operator_answer("qui aime un footballeur", semMem, lingDb, true));
  ONSEM_ANSWER_EQ("Toto",
                  operator_answer("qui aime un footballeur né en mars 1991", semMem, lingDb, true));

  operator_inform("Raphael est né le 7 novembre 1975.", semMem, lingDb);
  ONSEM_ANSWER_EQ("Raphael",
                  operator_answer("qui est né en novembre 1975", semMem, lingDb, true));
  const std::string xhoIsASingerBornInNomber1975 = "Qui est un chanteur né en novembre 1975";
  ONSEM_ANSWERNOTFOUND_EQ("Je ne sais pas qui est un chanteur né en novembre 1975.",
                          operator_answer(xhoIsASingerBornInNomber1975, semMem, lingDb, true));
  operator_inform("Raphael est un chanteur.", semMem, lingDb);
  ONSEM_ANSWER_EQ("Raphael",
                  operator_answer(xhoIsASingerBornInNomber1975, semMem, lingDb, true));
  //ONSEM_ANSWER_EQ("Raphael",
  //                operator_answer("Quel chanteur est né en novembre 1975", semMem, lingDb, true));
}

