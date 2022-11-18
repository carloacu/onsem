#include "../../semanticreasonergtests.hpp"
#include "operator_inform.hpp"
#include "operator_check.hpp"
#include "operator_get.hpp"
#include <gtest/gtest.h>
#include <onsem/semantictotext/semanticmemory/semanticmemory.hpp>
#include <onsem/semantictotext/semexpoperators.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase.hpp>
#include <onsem/tester/memorybinarization.hpp>
#include <onsem/tester/reactOnTexts.hpp>
#include "../../util/util.hpp"

using namespace onsem;



TEST_F(SemanticReasonerGTests, operator_react_basic)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;
  TrackSemMemoryNotifications trackSemMemoryNotifications(semMem, lingDb);

  // Bad question detection
  {
    ONSEM_NOANSWER(operator_react("Salut que ça donne cette phrase", semMem, lingDb));
    ONSEM_NOANSWER(operator_react("Paul est grand quoi", semMem, lingDb));
    ONSEM_NOANSWER(operator_react("I don't know what you like", semMem, lingDb));
    ONSEM_NOANSWER(operator_react("Ok et N5", semMem, lingDb));
    ONSEM_NOANSWER(operator_react("C'est bien démocratie", semMem, lingDb));
    ONSEM_NOANSWER(operator_react("n5", semMem, lingDb));
    ONSEM_NOANSWER(operator_react("Parlant de politique", semMem, lingDb)); // don't react a sentence without subject
    ONSEM_NOANSWER(operator_react("Rien n'est impossible", semMem, lingDb));
  }
  ONSEM_FEEDBACK_EQ("Ok, tu regardes à droite.", operator_react("je regarde à droite", semMem, lingDb));

  // unknown answer
  ONSEM_ANSWERNOTFOUND_EQ("Je ne sais pas quelle langue je peux parler.",
                          operator_react("Quelle langue sais-tu parler", semMem, lingDb));
  ONSEM_ANSWERNOTFOUND_EQ("Je ne sais pas quelle langue je peux parler.",
                          operator_react("Quelle langue sais tu parler", semMem, lingDb));
  ONSEM_ANSWERNOTFOUND_EQ("Je ne sais pas quelle langue je peux parler.",
                          operator_react("Quelle langue tu sais parler", semMem, lingDb));
  ONSEM_ANSWERNOTFOUND_EQ("Je ne sais pas quelle est la consigne.",
                          operator_react("Quelle est la consigne ?", semMem, lingDb));
  ONSEM_QUESTION_EQ("(\tJe ne sais pas.\tTHEN\tQui es-tu ?\t)",
                    operator_react("qui suis je", semMem, lingDb));

  EXPECT_EQ("", trackSemMemoryNotifications.getInfActions());
  EXPECT_EQ("", trackSemMemoryNotifications.getConditionToActions());
  memoryOperation::defaultKnowledge(semMem, lingDb);
  operator_inform("to do nothing means \\" + resourceLabelForTests_cmd + "=Service.wait()\\", semMem, lingDb);
  EXPECT_EQ("To ask something.\nTo do nothing.\nTo repeat means to say the last thing that I said.\nTo say something.", trackSemMemoryNotifications.getInfActions());

  // condition to unknown actions
  ONSEM_BEHAVIORNOTFOUND_EQ("Je ne sais pas me lever.",
                            operator_react("lève-toi si tu aimes le chocolat", semMem, lingDb));
  EXPECT_EQ("", trackSemMemoryNotifications.getConditionToActions());
  const std::string standUpCommandStr = "\\" + resourceLabelForTests_cmd + "=ALRobotPosture.goToPosture(\"Stand\", 0.8)\\";
  operator_inform("to stand up means " + standUpCommandStr, semMem, lingDb);

  // command
  ONSEM_BEHAVIORNOTFOUND_EQ("Je ne sais pas te poser des questions.",
                            operator_react("Pose moi des questions", semMem, lingDb));

  // if condition (introduced by "si") to an action
  ONSEM_NOTIFYSOMETHINGWILLBEDONE_EQ("Ok, je me lèverai si j'aime le chocolat.",
                                     operator_react("lève-toi si tu aimes le chocolat", semMem, lingDb));
  EXPECT_EQ("Arise if I like chocolate!", trackSemMemoryNotifications.getConditionToActions());
  ONSEM_BEHAVIOR_EQ(standUpCommandStr, operator_react("tu aimes le chocolat", semMem, lingDb));
  EXPECT_EQ("Arise if I like chocolate!", trackSemMemoryNotifications.getConditionToActions());
  ONSEM_REMOVEALLCONDITIONS_EQ("Ok, je supprime toutes les conditions.", operator_react("Supprime toutes les conditions", semMem, lingDb));
  ONSEM_FEEDBACK_EQ("Oui, je sais que j'aime le chocolat.",
                    operator_react("tu aimes le chocolat", semMem, lingDb));

  // permanent condition (introduced by "à chaque fois que") to an action
  ONSEM_NOTIFYSOMETHINGWILLBEDONE_EQ("Ok, je me lèverai à chaque fois que Paul regarde à droite.",
                                     operator_react("lève-toi à chaque fois que Paul regarde à droite", semMem, lingDb));
  EXPECT_EQ("Arise whenever Paul watches right!", trackSemMemoryNotifications.getConditionToActions());
  ONSEM_BEHAVIOR_EQ(standUpCommandStr, operator_react("Paul regarde à droite", semMem, lingDb));
  ONSEM_FEEDBACK_EQ("Je pensais le contraire.", operator_react("Paul ne regarde pas à droite", semMem, lingDb));
  EXPECT_EQ("Arise whenever Paul watches right!", trackSemMemoryNotifications.getConditionToActions());
  ONSEM_BEHAVIOR_EQ(standUpCommandStr, operator_react("Paul regarde à droite", semMem, lingDb));
  ONSEM_FEEDBACK_EQ("Je pensais le contraire.", operator_react("Paul ne regarde pas à droite", semMem, lingDb));
  EXPECT_EQ("Arise whenever Paul watches right!", trackSemMemoryNotifications.getConditionToActions());
  ONSEM_NOANSWER(operator_react("regarder", semMem, lingDb));
  ONSEM_NOANSWER(operator_react("Paul regarde", semMem, lingDb)); // don't stand up, because "à droite" info is missing

  // condition when the trigger is contained in the condition
  ONSEM_NOTIFYSOMETHINGWILLBEDONE_EQ("Ok, je dirai j'entends quelqu'un à chaque fois que j'entends quelqu'un.",
                                     operator_react("à chaque fois que tu entends quelqu'un dis j'entends quelqu'un", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("J'entends quelqu'un.", operator_react("tu m'entends", semMem, lingDb));

  // condition when the trigger is more detailed than in the condition
  ONSEM_NOTIFYSOMETHINGWILLBEDONE_EQ("Ok, I will say my left hand is touched whenever it's touched.",
                                     operator_react("whenever your left hand is touched, say my left hand is touched", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("My left hand is touched.", operator_react("I am touching your left hand", semMem, lingDb));

  // learn a composed command
  const std::string raiseRightHandCommandStr = "\\" + resourceLabelForTests_cmd + "=semantic/animations/raise_right_hand.qianim\\";
  operator_inform("to raise your right hand means " + raiseRightHandCommandStr, semMem, lingDb);
  operator_inform("to check hands means to raise your right hand", semMem, lingDb);
  ONSEM_BEHAVIOR_EQ(raiseRightHandCommandStr, operator_react("check hands", semMem, lingDb));
  EXPECT_EQ("To ask something.\nTo check hands means to raise my right hand.\nTo do nothing.\nTo raise my right hand.\n"
            "To repeat means to say the last thing that I said.\nTo say something.\nTo stand up.", trackSemMemoryNotifications.getInfActions());

  // condition on user input
  ONSEM_TEACHINGFEEDBACK_EQ("Ok, to scratch my head means to say I am scratching my head.",
                            operator_react("to scratch your head means to say I am scratching my head", semMem, lingDb));
  ONSEM_BEHAVIORNOTFOUND_EQ("I can't rub my head.", operator_react("rub your head", semMem, lingDb));
  ONSEM_NOTIFYSOMETHINGWILLBEDONE_EQ("Ok, I will scratch my head whenever you say rub my head.",
                                     operator_react("When I say rub your head scratch your head", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("I am scratching my head.", operator_react("rub your head", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("I am scratching my head.", operator_react("rub your head", semMem, lingDb));

  // conflict condition vs action definition
  ONSEM_TEACHINGFEEDBACK_EQ("Ok, to rub my head means to say I am rubbing my head.",
                            operator_react("to rub your head means to say I am rubbing my head", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("I am scratching my head.", operator_react("rub your head", semMem, lingDb));
  ONSEM_REMOVEALLCONDITIONS_EQ("Ok, I am removing all conditions.", operator_react("Remove all conditions", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("I am rubbing my head.", operator_react("rub your head", semMem, lingDb));

  // condition to a permanent actions
  ONSEM_NOTIFYSOMETHINGWILLBEDONE_EQ("Ok, I will stand up and I will raise my right hand whenever I see you.",
                                     operator_react("Whenever you see me stand up and raise your right hand", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("(\t" + standUpCommandStr + "\tAND\t" + raiseRightHandCommandStr + "\t)",
                    operator_react("You see me", semMem, lingDb));
  ONSEM_REMOVEALLCONDITIONS_EQ("Ok, je supprime toutes les conditions.", operator_react("Supprime toutes les conditions", semMem, lingDb));
  ONSEM_FEEDBACK_EQ("Yes, I know I see you.", operator_react("You see me", semMem, lingDb));

  // answer hello
  {
    ONSEM_ANSWER_EQ("Bonjour", operator_react("bonjour", semMem, lingDb));
    ONSEM_ANSWER_EQ("Bonjour", operator_react("salut", semMem, lingDb));
    ONSEM_ANSWER_EQ("Bonjour", operator_react("hi", semMem, lingDb));
    ONSEM_ANSWER_EQ("Hello", operator_react("hello", semMem, lingDb));
  }

  // answer bye-bye
  {
    ONSEM_ANSWER_EQ("Au revoir", operator_react("au revoir", semMem, lingDb));
    ONSEM_ANSWER_EQ("Bye-bye", operator_react("bye", semMem, lingDb));
    ONSEM_ANSWER_EQ("Bye-bye", operator_react("bye-bye", semMem, lingDb));
  }

  // an inform cannot contradict the source (because it's an assert)
  {
    ONSEM_ANSWER_EQ("Yes, I said hello.", operator_react("did you say hello ?", semMem, lingDb));
    ONSEM_FEEDBACK_EQ("No, I said hello.", operator_react("you didn't say hello", semMem, lingDb));
    ONSEM_ANSWER_EQ("Yes, I said hello.", operator_react("did you say hello ?", semMem, lingDb));
  }

  // unknown answer
  {
    ONSEM_ANSWERNOTFOUND_EQ("I don't know what I am talking about.",
                            operator_react("what are you talking about?", semMem, lingDb));
  }

  // say action
  {
    ONSEM_BEHAVIOR_EQ("Hello",
                      operator_react("say hello", semMem, lingDb));
    ONSEM_BEHAVIOR_EQ("Bonjour",
                      operator_react("dis bonjour", semMem, lingDb));
    ONSEM_BEHAVIOR_EQ("I don't know what the joke of today is.",
                      operator_react("say the joke of the day", semMem, lingDb));
    ONSEM_BEHAVIOR_EQ("Je ne sais pas quelle est la blague d'aujourd'hui.",
                      operator_react("dis la blague du jour", semMem, lingDb));
    const std::string jokeStr = "\"j'ai une blague sur les magasins mais elle n'a pas super marchée\"";
    ONSEM_NOANSWER(operator_react("la blague d'aujourd'hui c'est " + jokeStr, semMem, lingDb));
    ONSEM_ANSWER_EQ(jokeStr, operator_react("say the joke of today", semMem, lingDb));
    ONSEM_ANSWER_EQ(jokeStr, operator_react("say the joke of the day", semMem, lingDb));
    ONSEM_ANSWER_EQ(jokeStr, operator_react("raconte la blague d'aujourd'hui", semMem, lingDb));
    ONSEM_ANSWER_EQ(jokeStr, operator_react("raconte la blague du jour", semMem, lingDb));
    ONSEM_ANSWER_EQ(jokeStr, operator_react("raconte-moi la blague du jour", semMem, lingDb));
    ONSEM_ANSWER_EQ(jokeStr, operator_react("dis la blague du jour", semMem, lingDb));
    ONSEM_ANSWER_EQ("La blague d'aujourd'hui est " + jokeStr + ".", operator_react("quel est la blague du jour", semMem, lingDb));
    ONSEM_BEHAVIOR_EQ("(\t\"a1\"\tTHEN\t(\t\"a2\"\tAND\t\"a3\"\t)\tTHEN\t\"a4\"\t)",
                      operator_react("Say \"a1\" then say \"a2\" and say \"a3\" then say \"a4\"", semMem, lingDb));
  }

  // action labeling after execution of simple action
  {
    ONSEM_BEHAVIOR_EQ("Thanks",
                      operator_react("say thanks", semMem, lingDb));
    ONSEM_TEACHINGFEEDBACK_EQ("Ok, to be nice is to say thanks.",
                              operator_react("that is to be nice", semMem, lingDb));
    ONSEM_BEHAVIOR_EQ("Thanks",
                      operator_react("be nice", semMem, lingDb));
    ONSEM_BEHAVIOR_EQ("Thanks",
                      operator_react("I want you to be nice", semMem, lingDb));
  }

  // action labeling after execution of a sequence of actions
  {
    ONSEM_BEHAVIOR_EQ("(\tI like you.\tTHEN\tWhat is your name?\t)",
                      operator_react("say that you like me then ask what is my name", semMem, lingDb));
    ONSEM_TEACHINGFEEDBACK_EQ("Ok, to welcome means to say I like you and then to ask what your name is.",
                              operator_react("it means to welcome", semMem, lingDb));
    ONSEM_BEHAVIOR_EQ("(\tI like you.\tTHEN\tWhat is your name?\t)",
                      operator_react("welcome", semMem, lingDb));
  }

  // action labeling after execution of a sequence of actions even more complex
  {
    ONSEM_BEHAVIOR_EQ("(\t\"a1\"\tTHEN\t(\t\"a2\"\tAND\t\"a3\"\t)\tTHEN\t\"a4\"\t)",
                      operator_react("Say \"a1\" then say \"a2\" and say \"a3\" then say \"a4\"", semMem, lingDb));
    ONSEM_TEACHINGFEEDBACK_EQ("Ok, to become crazy is to say \"a1\", then to say \"a2\" and to say \"a3\" and then to say \"a4\".",
                              operator_react("that's to become crazy", semMem, lingDb));
    ONSEM_BEHAVIOR_EQ("(\t(\t\"a1\"\tTHEN\t(\t\"a2\"\tAND\t\"a3\"\t)\tTHEN\t\"a4\"\t)\tTHEN\tI finished to become crazy.\t)",
                      operator_react("become crazy then say I have finished to become crazy", semMem, lingDb));
    ONSEM_TEACHINGFEEDBACK_EQ("Ok, to do a big test is to become crazy and then to say I finished to become crazy.",
                              operator_react("it's to do a big test", semMem, lingDb));
    ONSEM_BEHAVIOR_EQ("(\t(\t\"a1\"\tTHEN\t(\t\"a2\"\tAND\t\"a3\"\t)\tTHEN\t\"a4\"\t)\tTHEN\tI finished to become crazy.\t)",
                      operator_react("do a big test", semMem, lingDb));
  }

  //EXPECT_EQ("Ok, I eat chocolate whenever my body is hot.",
  //          operator_react("whenever your body is hot, you eat chocolate", semMem, lingDb));
  ONSEM_NOANSWER(operator_react("whenever your body is hot, you eat chocolate", semMem, lingDb));
  ONSEM_ANSWERNOTFOUND_EQ("I don't know what I eat.",
                          operator_react("what do you eat ?", semMem, lingDb));
  operator_inform("your arm is hot", semMem, lingDb);
  ONSEM_ANSWER_EQ("I eat chocolate.", operator_react("what do you eat ?", semMem, lingDb));
  operator_inform("your body is not hot", semMem, lingDb);
  ONSEM_ANSWERNOTFOUND_EQ("I don't know what I eat.",
                          operator_react("what do you eat ?", semMem, lingDb));

  ONSEM_ANSWER_EQ("Bonjour", operator_react("bonjour", semMem, lingDb));
  ONSEM_NOTIFYSOMETHINGWILLBEDONE_EQ("Ok, je dirai salut à chaque fois que tu dis bonjour.", operator_react("quand je dis bonjour, dis salut", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("Salut", operator_react("bonjour", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("Salut", operator_react("bonjour", semMem, lingDb));
  ONSEM_NOTIFYSOMETHINGWILLBEDONE_EQ("Ok, je ne ferai rien à chaque fois que tu dis bonjour.", operator_react("quand je dis bonjour, ne fais rien", semMem, lingDb));
  ONSEM_ANSWER_EQ("Bonjour", operator_react("bonjour", semMem, lingDb));
  ONSEM_NOTIFYSOMETHINGWILLBEDONE_EQ("Ok, I will ask what you can do for me whenever you look at me.", operator_react("when I look at you ask what can I do for you", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("What can you do for me?", operator_react("I look at you", semMem, lingDb));

  {
    const std::string manTalkingCommandStr1 =
        "\\" + resourceLabelForTests_url + "=https://www.youtube.com/watch?v=q_5QpYJuGa0\\";
    const std::string manTalkingCommandStr2 =
        "\\" + resourceLabelForTests_url + "=https://www.youtube.com/watch?v=XTgsRn0_ng4\\";
    operator_inform(manTalkingCommandStr1 + " is a man talking", semMem, lingDb);
    operator_inform(manTalkingCommandStr2 + " is a man talking", semMem, lingDb);
    std::string firstAnswer = operator_react("show me a man talking", semMem, lingDb).answer;
    std::string secondAnswer = operator_react("show me a man talking", semMem, lingDb).answer;
    std::string thirdAnswer = operator_react("show me a man talking", semMem, lingDb).answer;

    EXPECT_TRUE(firstAnswer != secondAnswer);
    EXPECT_TRUE(firstAnswer == thirdAnswer);
    EXPECT_TRUE(firstAnswer == manTalkingCommandStr1 || firstAnswer == manTalkingCommandStr2);
    EXPECT_TRUE(secondAnswer == manTalkingCommandStr1 || secondAnswer == manTalkingCommandStr2);
  }

  {
    ONSEM_ANSWERNOTFOUND_EQ("I don't know where Paul is.",
                            operator_react("Where is Paul ?", semMem, lingDb));
    ONSEM_ANSWER_EQ("Paul is in the bedroom.", operator_react("Paul is in the bedroom. Where is Paul ?", semMem, lingDb));
  }


  {
    operator_inform("je suis un avocat", semMem, lingDb);
    operator_inform("mon métier est journaliste", semMem, lingDb);
    ONSEM_ANSWER_EQ("Ton métier est journaliste et un avocat.",
                    operator_react("Quel est mon métier ?", semMem, lingDb));
  }

  // filter answers acording to a concept
  {
    ONSEM_ANSWERNOTFOUND_EQ("Je ne sais pas de quelle couleur sont tes cheveux.",
                            operator_react("de quelle couleur sont mes cheveux", semMem, lingDb));
    operator_inform("mes cheveux sont longs", semMem, lingDb);
    ONSEM_ANSWERNOTFOUND_EQ("Je ne sais pas de quelle couleur sont tes cheveux.",
                            operator_react("de quelle couleur sont mes cheveux", semMem, lingDb));
    operator_inform("mes cheveux sont roux", semMem, lingDb);
    ONSEM_ANSWER_EQ("Tes cheveux sont roux.",
                    operator_react("de quelle couleur sont mes cheveux", semMem, lingDb));
    ONSEM_ANSWER_EQ("Your hairs are auburn.",
                    operator_react("what color are my hairs", semMem, lingDb));
  }

  {
    const std::string lookRightCommandStr = "\\" + resourceLabelForTests_cmd + "=semantic/animations/look_right.qianim\\";
    operator_inform("to look right means " + lookRightCommandStr, semMem, lingDb);
    ONSEM_NOTIFYSOMETHINGWILLBEDONE_EQ("Ok, je regarderai à droite à chaque fois que Paul est content.",
                                       operator_react("A chaque fois que Paul est content regarde à droite", semMem, lingDb));
    ONSEM_BEHAVIOR_EQ(lookRightCommandStr, operator_react("Paul est content", semMem, lingDb));
    ONSEM_FEEDBACK_EQ("Je pensais le contraire.", operator_react("Paul n'est pas content", semMem, lingDb));
    ONSEM_BEHAVIOR_EQ(lookRightCommandStr, operator_react("Paul est content", semMem, lingDb));
  }

  // condition of knowing something
  {
    ONSEM_NOTIFYSOMETHINGWILLBEDONE_EQ("Ok, I will say your name if I know your name.",
                                       operator_react("If you know my name say it", semMem, lingDb));
    operator_inform("My name is Christophe.", semMem, lingDb);
    ONSEM_BEHAVIOR_EQ("(\tOk, I will say your name if I know your name.\tTHEN\tChristophe\t)",
                      operator_react("If you know my name say it", semMem, lingDb));
    ONSEM_NOTIFYSOMETHINGWILLBEDONE_EQ("Ok, I will say your name whenever you say hello.",
                                       operator_react("say my name when I say hello", semMem, lingDb));
    ONSEM_BEHAVIOR_EQ("Christophe",
                      operator_react("hello", semMem, lingDb));
    ONSEM_REMOVEALLCONDITIONS_EQ("Ok, I am removing all conditions.", operator_react("Remove all conditions", semMem, lingDb));
  }

  // time answer
  {
    ONSEM_QUESTION_EQ("(\tJe ne sais pas.\tTHEN\tQuand es-tu content ?\t)",
                      operator_react("quand je suis content ?", semMem, lingDb));
    operator_inform("je suis content quand tu souris", semMem, lingDb);
    ONSEM_ANSWER_EQ("Tu es content quand je souris.", operator_react("quand je suis content ?", semMem, lingDb));
    ONSEM_QUESTION_EQ("(\tJe ne sais pas.\tTHEN\tEs-tu content ?\t)",
                      operator_react("est-ce que je suis content ?", semMem, lingDb));
    operator_inform("Tu souris", semMem, lingDb);
    ONSEM_ANSWER_EQ("Oui, tu es content.",
                    operator_react("est-ce que je suis content ?", semMem, lingDb));
  }

  // object answer
  {
    ONSEM_QUESTION_EQ("(\tJe ne sais pas.\tTHEN\tQu'aimes-tu ?\t)",
                      operator_react("qu'est-ce que j'aime ?", semMem, lingDb));
    // it should not be considered has an answer to the previous question
    ONSEM_NOANSWER(operator_react("c'est du chocolat", semMem, lingDb));
    ONSEM_QUESTION_EQ("(\tJe ne sais pas.\tTHEN\tQu'aimes-tu ?\t)",
                      operator_react("qu'est-ce que j'aime ?", semMem, lingDb));
  }

  // learn a command
  {
    ONSEM_BEHAVIORNOTFOUND_EQ("I can't salute.",
                              operator_react("salute", semMem, lingDb));
    ONSEM_TEACHINGFEEDBACK_EQ("Ok, to salute means to say hello.",
                              operator_react("to salute means to say hello", semMem, lingDb));
    ONSEM_BEHAVIOR_EQ("Hello",
                      operator_react("salute", semMem, lingDb));
    ONSEM_TEACHINGFEEDBACK_EQ("Ok, to salute is to raise the right arm.",
                              operator_react("Actually, to salute is to raise the right arm", semMem, lingDb));
    operator_inform("to raise my right hand means " + raiseRightHandCommandStr, semMem, lingDb);
    ONSEM_BEHAVIOR_EQ(raiseRightHandCommandStr, operator_react("salute", semMem, lingDb));
    ONSEM_TEACHINGFEEDBACK_EQ("Ok, to salute means to say hello and to raise the right arm.",
                              operator_react("to salute means to say hello and raise the right arm", semMem, lingDb));
    ONSEM_BEHAVIOR_EQ("(\tHello\tAND\t" + raiseRightHandCommandStr + "\t)",
                      operator_react("salute", semMem, lingDb));
    ONSEM_TEACHINGFEEDBACK_EQ("Ok, to salute you means to say hi man.",
                              operator_react("to salute me means to say hi man", semMem, lingDb));
    ONSEM_ANSWER_EQ("Pour saluer il faut dire salut homme",
                    operator_react("Comment saluer ?", semMem, lingDb));
    ONSEM_BEHAVIOR_EQ("Pour saluer il faut dire salut homme",
                      operator_react("Dis comment saluer", semMem, lingDb));
    ONSEM_BEHAVIOR_EQ("Hi man",
                      operator_react("salute me", semMem, lingDb));
    ONSEM_BEHAVIOR_EQ("Paul smiled.",
                      operator_react("Say Paul smiled", semMem, lingDb));
    ONSEM_TEACHINGFEEDBACK_EQ("Ok, to do a test means to say Paul smiled.",
                              operator_react("to do a test means to say Paul smiled", semMem, lingDb));
    ONSEM_BEHAVIOR_EQ("Paul smiled",
                      operator_react("Do a test", semMem, lingDb));
  }

  // recall past experiences
  {
    SemanticTimeGrounding::setAnHardCodedTimeElts(true, false);
    SemanticTimeGrounding::hardCodedCurrentTimeOfDay->add(SemanticTimeUnity::SECOND, 1);
    operator_inform("Remy is happy", semMem, lingDb);
    SemanticTimeGrounding::hardCodedCurrentTimeOfDay->add(SemanticTimeUnity::SECOND, 10);
    ONSEM_ANSWER_EQ("Yes, you said Remy is happy.",
                    operator_react("did I say that Remy is happy", semMem, lingDb));
    SemanticTimeGrounding::hardCodedCurrentTimeOfDay->add(SemanticTimeUnity::SECOND, 11);
    ONSEM_ANSWER_EQ("You asked me if you said Remy is happy.",
                    operator_react("what happened just after that", semMem, lingDb));
    // ask about the time
    ONSEM_ANSWER_EQ("It was 11 seconds ago.",
                    operator_react("when was it", semMem, lingDb));

    operator_mergeAndInform("Paul found a pen", semMem, lingDb);
    operator_mergeAndInform("Paul is a great man", semMem, lingDb); // the past memory to skip
    ONSEM_ANSWER_EQ("Paul found a pen.",
                    operator_react("what did Paul find", semMem, lingDb));
    ONSEM_ANSWER_EQ("Yes, you said Paul found a pen.",
                    operator_react("did I say that Paul found a pen", semMem, lingDb));
    // recall past experiences with filtering
    ONSEM_ANSWER_EQ("I answered Paul found a pen.",
                    operator_react("what did you answered just after that", semMem, lingDb));
    // recall with in other direction
    ONSEM_ANSWER_EQ("You asked me what Paul found.",
                    operator_react("what happened just before that", semMem, lingDb));
    // in french
    ONSEM_ANSWER_EQ("J'ai répondu que Paul a trouvé un stylo.",
                    operator_react("et puis", semMem, lingDb));

    // "and then", "and after that", ...
    ONSEM_ANSWER_EQ("You asked me if you said Paul found a pen.",
                    operator_react("and then", semMem, lingDb));
    ONSEM_ANSWER_EQ("I answered yes you said Paul found a pen.",
                    operator_react("and after that", semMem, lingDb));

    // check that any other sentence with "and then" inside is not considered has a request to go on
    ONSEM_NOANSWER(operator_react("marcher et puis regarder", semMem, lingDb));
  }

  // answer filtered by an occurence rank
  {
    ONSEM_QUESTION_EQ("(\tI don't know.\tTHEN\tWhen did you say occurenceRankTest for the first time?\t)",
                      operator_react("When did I say occurenceRankTest for the first time", semMem, lingDb));
    SemanticTimeGrounding::hardCodedCurrentTimeOfDay->add(SemanticTimeUnity::SECOND, 1);
    operator_inform("occurenceRankTest", semMem, lingDb);
    SemanticTimeGrounding::hardCodedCurrentTimeOfDay->add(SemanticTimeUnity::SECOND, 13);
    ONSEM_ANSWER_EQ("You said occurenceRankTest for the first time 13 seconds ago.",
                    operator_react("When did I say occurenceRankTest for the first time", semMem, lingDb));
    ONSEM_QUESTION_EQ("(\tI don't know.\tTHEN\tWhen did you say occurenceRankTest for the second time?\t)",
                      operator_react("When did I say occurenceRankTest for the second time", semMem, lingDb));
    SemanticTimeGrounding::hardCodedCurrentTimeOfDay->add(SemanticTimeUnity::SECOND, 2);
    operator_inform("occurenceRankTest", semMem, lingDb);
    SemanticTimeGrounding::hardCodedCurrentTimeOfDay->add(SemanticTimeUnity::SECOND, 10);
    ONSEM_ANSWER_EQ("You said occurenceRankTest for the first time 25 seconds ago.",
                    operator_react("When did I say occurenceRankTest for the first time", semMem, lingDb));
    ONSEM_ANSWER_EQ("You said occurenceRankTest for the last time 10 seconds ago.",
                    operator_react("When did I say occurenceRankTest for the last time", semMem, lingDb));
    ONSEM_ANSWER_EQ("The second time that you said occurenceRankTest was 10 seconds ago.",
                    operator_react("when was the second time I said occurenceRankTest", semMem, lingDb));
    ONSEM_ANSWER_EQ("Tu as dit occurenceRankTest pour la première fois il y a 25 secondes.",
                    operator_react("quand ai-je dit occurenceRankTest pour la première fois", semMem, lingDb));
    ONSEM_ANSWER_EQ("La dernière fois que tu as dit occurenceRankTest est il y a 10 secondes.",
                    operator_react("quelle est la dernière fois que j'ai dit occurenceRankTest", semMem, lingDb));
  }

  // merge with context
  {
    ONSEM_QUESTION_EQ("(\tI don't know.\tTHEN\tWhat do you want?\t)",
                      operator_react("What do I want ?", semMem, lingDb));
    operator_inform_fromRobot("What do you want ?", semMem, lingDb);
    operator_mergeAndInform("to play soccer", semMem, lingDb);
    ONSEM_ANSWER_EQ("You want to play soccer.",
                    operator_react("What do I want ?", semMem, lingDb));
  }

  // merge with context - incomplete sentences
  {
    // ok for nominal group
    ONSEM_NOANSWER(operator_react("I went to the", semMem, lingDb));
    ONSEM_QUESTION_EQ("When did you go to the cinema?",
                      operator_react("cinema", semMem, lingDb));

    // not ok for sentences
    ONSEM_NOANSWER(operator_react("I went to the", semMem, lingDb));
    ONSEM_NOANSWER(operator_react("Jean is an old man", semMem, lingDb));
    static const std::string spinRightCommandStr = "\\" + resourceLabelForTests_cmd + "=semantic/animations/spin_right.qianim\\";
    ONSEM_NOANSWER(operator_react("I went to the", semMem, lingDb));
    ONSEM_TEACHINGFEEDBACK_EQ("Ok, to spin right means " + spinRightCommandStr,
                              operator_react("to spin right means " + spinRightCommandStr, semMem, lingDb));
    ONSEM_NOANSWER(operator_react("I went to the", semMem, lingDb));
    ONSEM_BEHAVIOR_EQ(spinRightCommandStr,
                      operator_react("spin right", semMem, lingDb));
  }

  // positive sentiment feedbacks
  {
    ONSEM_QUESTION_EQ("(\tJe suis content de l'entendre.\tTHEN\tPourquoi es-tu content ?\t)",
                      operator_react("je suis content", semMem, lingDb));
    ONSEM_QUESTION_EQ("(\tI am happy to hear that.\tTHEN\tWhy are you happy?\t)",
                      operator_react("I am happy", semMem, lingDb));
    // why answer understanding
    ONSEM_QUESTION_EQ("Why was it a beautiful day yesterday?",
                      operator_react("because it was a beautiful day yesterday", semMem, lingDb));
    ONSEM_ANSWER_EQ("You are happy because it was a beautiful day yesterday.",
                    operator_react("why am I happy", semMem, lingDb));
    // don't reask why if it's already
    ONSEM_FEEDBACK_EQ("I am happy to hear that.",
                      operator_react("I am happy", semMem, lingDb));
  }

  // purpose answer
  {
    ONSEM_QUESTION_EQ("When did Andrew climb in order to see what is on the top of the mountain?",
                      operator_react("Andrew climbed in order to see what is on the top of the mountain", semMem, lingDb));
    ONSEM_ANSWER_EQ("Andrew climbed in order to see what is on the top of the mountain.",
                    operator_react("For what purpose Andrew climbed?", semMem, lingDb));
  }

  {
    ONSEM_QUESTION_EQ("When did you work at Paris?",
                      operator_react("I worked at Paris.", semMem, lingDb));
    ONSEM_FEEDBACK_EQ("I thought the opposite.",
                      operator_react("I didn't worked at Paris.", semMem, lingDb));
    operator_mergeAndInform("Claire worked at Paris.", semMem, lingDb);
    ONSEM_FEEDBACK_EQ("Claire also worked at Paris.",
                      operator_react("My brother worked at Paris.", semMem, lingDb));
    ONSEM_FEEDBACK_EQ("Ok, you work at Issy.",
                      operator_react("I work at Issy.", semMem, lingDb));
    ONSEM_ANSWER_EQ("Claire and your brother",
                    operator_react("Who worked at Paris?", semMem, lingDb));
    ONSEM_FEEDBACK_EQ("Claire and your brother also worked at Paris.",
                      operator_react("I worked at Paris.", semMem, lingDb));
  }

  // location proactive question
  {
    ONSEM_QUESTION_EQ("Où as-tu marché hier ?",
                      operator_react("J’ai marché hier.", semMem, lingDb));
    ONSEM_FEEDBACK_EQ("Ok, tu as marché à Paris hier.",
                      operator_react("à Paris", semMem, lingDb));
    ONSEM_ANSWER_EQ("Tu as marché à Paris.",
                    operator_react("Où j'ai marché hier ?", semMem, lingDb));
    ONSEM_FEEDBACK_EQ("Oui, je sais que tu as marché hier.",
                      operator_react("J’ai marché hier.", semMem, lingDb));
  }

  // time proactive question
  {
    ONSEM_QUESTION_EQ("Quand as-tu fait les courses dans le 10ème ?",
                      operator_react("J’ai fait les courses dans le 10ème.", semMem, lingDb));
    ONSEM_FEEDBACK_EQ("Ok, tu as fait les courses dans le 10ème hier.",
                      operator_react("hier", semMem, lingDb));
    ONSEM_ANSWER_EQ("Tu as fait les courses dans le 10\xC3\xA8me hier.",
                    operator_react("Quand j'ai fait les courses dans le 10ème ?", semMem, lingDb));
    ONSEM_FEEDBACK_EQ("Oui, je sais que tu as fait les courses dans le 10ème.",
                      operator_react("J’ai fait les courses dans le 10ème.", semMem, lingDb));
  }

  // location then time proactive question
  {
    ONSEM_QUESTION_EQ("Quand as-tu couru ?",
                      operator_react("J’ai couru.", semMem, lingDb));
    ONSEM_QUESTION_EQ("Où as-tu couru avant-hier ?",
                      operator_react("avant-hier", semMem, lingDb));
    ONSEM_FEEDBACK_EQ("Ok, tu as couru à Paris avant-hier.",
                      operator_react("à Paris", semMem, lingDb));
    ONSEM_FEEDBACK_EQ("Oui, je sais que tu as couru.",
                      operator_react("J’ai couru.", semMem, lingDb));
  }

  // quantity question when the number is explicitly said
  {
    ONSEM_QUESTION_EQ("(\tI don't know.\tTHEN\tHow many children do you have?\t)",
                      operator_react("How many children do I have?", semMem, lingDb));
    operator_inform("I have many children.", semMem, lingDb);
    ONSEM_QUESTION_EQ("(\tI don't know.\tTHEN\tHow many children do you have?\t)",
                      operator_react("How many children do I have?", semMem, lingDb));
    operator_inform("I have 2 children.", semMem, lingDb);
    ONSEM_ANSWER_EQ("You have 2 children.",
                    operator_react("How many children do I have?", semMem, lingDb));
    ONSEM_ANSWER_EQ("No, you have 2 children.",
                    operator_react("Do I have 3 children?", semMem, lingDb));
    ONSEM_ANSWER_EQ("Yes, you have 2 children.",
                    operator_react("Do I have 2 children?", semMem, lingDb));
  }

  // quantity question when the number is not explicitly said
  {
    ONSEM_ANSWERNOTFOUND_EQ("I don't know how many fruits Donna bought.",
                            operator_react("How many fruits did Donna buy?", semMem, lingDb));
    operator_mergeAndInform("Donna bought a banana", semMem, lingDb);
    operator_mergeAndInform("Donna bought 3 lemons", semMem, lingDb);
    operator_mergeAndInform("Donna bought four oranges", semMem, lingDb);
    operator_mergeAndInform("Donna bought a cake", semMem, lingDb);
    ONSEM_ANSWER_EQ("Donna bought 8 fruits.",
                    operator_react("How many fruits did Donna buy?", semMem, lingDb));
    ONSEM_ANSWER_EQ("No, Donna bought 8 fruits.",
                    operator_react("Did Donna buy 7 fruits?", semMem, lingDb));
    ONSEM_ANSWER_EQ("Yes, Donna bought 8 fruits.",
                    operator_react("Did Donna buy 8 fruits?", semMem, lingDb));
  }

  // "ça va" question
  {
    ONSEM_ANSWER_EQ("Oui, je vais bien. Merci",
                    operator_react("ça va ?", semMem, lingDb));
    ONSEM_ANSWER_EQ("Oui, je vais bien. Merci",
                    operator_react("ca va ?", semMem, lingDb));
    ONSEM_ANSWER_EQ("Je vais bien. Merci",
                    operator_react("comment ça va ?", semMem, lingDb));
    ONSEM_ANSWER_EQ("Je vais bien. Merci",
                    operator_react("comment ça va", semMem, lingDb));
    ONSEM_QUESTION_EQ("(\tJe suis content de l'entendre.\tTHEN\tPourquoi vas-tu bien ?\t)",
                      operator_react("ça va", semMem, lingDb));
    ONSEM_QUESTION_EQ("(\tJe suis content de l'entendre.\tTHEN\tPourquoi vas-tu bien ?\t)",
                      operator_react("ça va bien", semMem, lingDb));
  }

  // condition -> information -> action
  {
    ONSEM_NOTIFYSOMETHINGWILLBEDONE_EQ("Ok, je dirai youpis si Innocent est content.",
                                       operator_react("si Innocent est content dis youpis", semMem, lingDb));
    ONSEM_NOTIFYSOMETHINGWILLBEDONE_EQ("Ok, Innocent est content si il sourit.",
                                       operator_react("si Innocent sourit, Innocent est content", semMem, lingDb)); // fix with "quand"
    ONSEM_BEHAVIOR_EQ("Youpis",
                      operator_react("Innocent sourit", semMem, lingDb));
  }

  // do an action from the answer of a question
  {
    ONSEM_TEACHINGFEEDBACK_EQ("Ok, to do a ballet spin is to say I do a ballet spin.",
                              operator_react("To do a ballet spin is to say I do a ballet spin", semMem, lingDb));
    ONSEM_BEHAVIOR_EQ("I do a ballet spin.",
                      operator_react("Do a ballet spin", semMem, lingDb));
    ONSEM_QUESTION_EQ("What must I do?",
                      operator_react("Do", semMem, lingDb));
    ONSEM_BEHAVIOR_EQ("I do a ballet spin.",
                      operator_react("A ballet spin", semMem, lingDb));
  }

  // "Qui c'est" question management
  {
    ONSEM_NOANSWER(operator_react("Delphine est mon ami", semMem, lingDb));
    ONSEM_QUESTION_EQ("Quand est-ce que Delphine travaillait les maths ?",
                      operator_react("Delphine travaillait les maths", semMem, lingDb));
    ONSEM_FEEDBACK_EQ("Delphine a aussi travaillé les maths.",
                      operator_react("Je travaille les maths", semMem, lingDb));
    ONSEM_ANSWER_EQ("C'est ton ami.",
                    operator_react("Qui c'est", semMem, lingDb));
    ONSEM_NOANSWER(operator_react("C'est", semMem, lingDb));
  }

  // reaction on yes/no answers
  {
    ONSEM_BEHAVIOR_EQ("Veux-tu manger ?",
                      operator_react("Demande si je veux manger", semMem, lingDb));
    ONSEM_FEEDBACK_EQ("Ok, tu veux manger.",
                      operator_react("Oui", semMem, lingDb));
  }

  // don't associate a person to anybody
  {
    ONSEM_FEEDBACK_EQ("Oui, je sais que j'aime le chocolat.",
                      operator_react("tu aimes le chocolat", semMem, lingDb));
    ONSEM_NOANSWER(operator_react("Elles aiment le chocolat.", semMem, lingDb));
    ONSEM_NOANSWER(operator_react("quelques-unes aiment les licornes", semMem, lingDb));
    ONSEM_ANSWERNOTFOUND_EQ("Je ne sais pas si j'aime les licornes.",
                            operator_react("Tu aimes les licornes ?", semMem, lingDb));
  }

  // test old weird bug with everybody concept
  {
    ONSEM_QUESTION_EQ("Why do you like everybody?",
                   operator_react("I like everybody", semMem, lingDb));
    ONSEM_FEEDBACK_EQ("You also look at me.",
                      operator_react("Alexis looks at you", semMem, lingDb));
    ONSEM_FEEDBACK_EQ("Ok, you look at chocolate.",
                      operator_react("I look at chocolate", semMem, lingDb));
    ONSEM_QUESTION_EQ("(\tJe ne sais pas.\tTHEN\tAimes-tu le chocolat ?\t)",
                      operator_react("J'aime le chocolat ?", semMem, lingDb));
  }

  // test question to wikipedia
  {
    ONSEM_NOANSWER(operator_react("Adrien-Marie Legendre, né le 18 septembre 1752 à Paris et mort le 10 janvier 1833 à Auteuil, est un mathématicien français.", semMem, lingDb));
    ONSEM_ANSWER_EQ("Adrien-Marie Legendre est un mathématicien français.",
                    operator_react("qui est Adrien-Marie Legendre", semMem, lingDb));
    // this sentences caused a bug
    ONSEM_FEEDBACK_EQ("Oui, je sais que tu deviens.",
                      operator_react("je t'aime et devient", semMem, lingDb));
    ONSEM_FEEDBACK_EQ("Oui, je sais qu'Adrien-Marie Legendre est.",
                      operator_react("Adrien-Marie Legendre est", semMem, lingDb));
    ONSEM_ANSWER_EQ("Adrien-Marie Legendre est un mathématicien français.",
                    operator_react("qui est Adrien-Marie Legendre", semMem, lingDb));
    ONSEM_BEHAVIORNOTFOUND_EQ("Je ne sais pas te montrer Adrien-Marie Legendre.",
                              operator_react("Montre-moi Adrien-Marie Legendre", semMem, lingDb));
    ONSEM_ANSWER_EQ("Adrien-Marie Legendre",
                    operator_react("qui est un mathématicien", semMem, lingDb));
    ONSEM_FEEDBACK_EQ("Adrien-Marie Legendre est aussi un mathématicien.",
                      operator_react("je suis un mathématicien", semMem, lingDb));
    ONSEM_FEEDBACK_EQ("Oui, je sais qu'Adrien-Marie Legendre né à Paris est un mathématicien français.",
                      operator_react("Adrien-Marie Legendre, né à Paris, est un mathématicien français.", semMem, lingDb));
    ONSEM_ANSWER_EQ("Toi et Adrien-Marie Legendre",
                    operator_react("qui est un mathématicien", semMem, lingDb));
    ONSEM_FEEDBACK_EQ("Toi et Adrien-Marie Legendre êtes aussi des mathématiciens.",
                      operator_react("Toto est un mathématicien", semMem, lingDb));
    ONSEM_NOANSWER(operator_react("Deee est une actrice de cinéma et de théâtre, chanteuse et compositrice. "
                                  "Elle est la seule comédienne allemande à avoir tenu un rôle en vedette à Broadway.", semMem, lingDb));
    ONSEM_NOANSWER(operator_react("Initialement cuisinier, il est devenu chanteur", semMem, lingDb));
    ONSEM_ANSWER_EQ("Deee",
                    operator_react("Who is a singer", semMem, lingDb));
    ONSEM_NOANSWER(operator_react("Paul Der est un informaticien", semMem, lingDb));
    ONSEM_NOANSWER(operator_react("Paul Fee est un chercheur", semMem, lingDb));
    ONSEM_ANSWERNOTFOUND_EQ("Je ne sais pas qui est Paul Io.",
                            operator_react("Qui est Paul Io ?", semMem, lingDb));
    ONSEM_FEEDBACK_EQ("Deee is also a singer.",
                      operator_react("Malena Ernman is a singer", semMem, lingDb));
    ONSEM_ANSWER_EQ("Malena Ernman is a singer.",
                    operator_react("What do you know about Malena Ernman?", semMem, lingDb));
    ONSEM_ANSWER_EQ("Paul Der is a computer scientist.",
                    operator_react("What do you know about Paul Der?", semMem, lingDb));
    // TODO: Fix "What do you know about Malena Ernman and Paul Der?"
    ONSEM_NOANSWER(operator_react("Barack Obama, né le 4 août 1961 à Honolulu (Hawaï), est un homme d'État américain.", semMem, lingDb));
    ONSEM_NOANSWER(operator_react("Francky Vandendriessche est devenu un entraîneur des gardiens à KV Courtrai.", semMem, lingDb));
    ONSEM_NOANSWER(operator_react("Reese Witherspoon, Michelle Obama, Hillary Clinton et Andrea Jung Andrea Jung est une femme d'affaires américaine", semMem, lingDb));
    ONSEM_ANSWER_EQ("Barack Obama est un homme d'État américain.",
                    operator_react("qui est Barack Obama", semMem, lingDb));
    ONSEM_FEEDBACK_EQ("Deee et Malena Ernman sont aussi des chanteuses.",
                      operator_react("Lady Gaga est une chanteuse", semMem, lingDb));
    ONSEM_FEEDBACK_EQ("Oui, je connais Lady Gaga.",
                      operator_react("tu connais Lady Gaga", semMem, lingDb));
    ONSEM_ANSWER_EQ("C'est une chanteuse.",
                    operator_react("qui c'est", semMem, lingDb));
    ONSEM_ANSWER_EQ("Barack Obama",
                    operator_react("Qui est né le 4 août 1961 ?", semMem, lingDb));
  }


  EXPECT_TRUE(checkMemBlocBinarization(semMem.memBloc, lingDb));
  static const std::string reactMemoryFilename("reactmemory.smb");
  semMem.memBloc.writeInBinaryFile(lingDb, reactMemoryFilename, 3000000000);
  semMem.memBloc.clearKnowledgeByKnowledgeOnlyForTests(lingDb, nullptr);

  SemanticMemory semMemBinary;
  semMemBinary.memBloc.loadBinaryFile(reactMemoryFilename);
  ONSEM_ANSWER_EQ("Barack Obama",
                  operator_react("Qui est né le 4 août 1961 ?", semMemBinary, lingDb));
}


TEST_F(SemanticReasonerGTests, operator_react_useActionAsVariables)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;
  memoryOperation::learnSayCommand(semMem, lingDb);

  ONSEM_TEACHINGFEEDBACK_EQ("Ok, to welcome is to say nice to meet you.",
                            operator_react("to welcome is to say nice to meet you", semMem, lingDb));

  ONSEM_ANSWER_EQ("To welcome is to say nice to meet you.",
                  operator_react("what is to welcome", semMem, lingDb));

  EXPECT_EQ(std::vector<std::string>{"To say nice to meet you."},
            operator_get("to welcome", semMem, lingDb));

  ONSEM_BEHAVIOR_EQ("Nice to meet you",
                    operator_react("welcome", semMem, lingDb));

  ONSEM_TEACHINGFEEDBACK_EQ("Ok, to welcome is to say nice to meet you.",
                            operator_react("that is to welcome", semMem, lingDb));

  ONSEM_BEHAVIOR_EQ("Nice to meet you",
                    operator_react("welcome", semMem, lingDb));

  ONSEM_BEHAVIOR_EQ("(\tNice to meet you\tTHEN\tHello\t)",
                    operator_react("welcome then say hello", semMem, lingDb));

  ONSEM_TEACHINGFEEDBACK_EQ("Ok, to welcome is to say nice to meet you and then to say hello.",
                            operator_react("that is to welcome", semMem, lingDb));

  ONSEM_BEHAVIOR_EQ("(\tNice to meet you\tTHEN\tHello\t)",
                    operator_react("welcome", semMem, lingDb));
}


TEST_F(SemanticReasonerGTests, operator_react_fromNominalGroups)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;
  ReactionOptions canReactToANoun;
  canReactToANoun.canReactToANoun = true;

  ONSEM_QUESTION_EQ("Qui est la femme dont je parle ?",
                    operator_react("La femme dont tu parles.", semMem, lingDb, SemanticLanguageEnum::UNKNOWN,
                                   &canReactToANoun));
  // TODO: handle sentence "The woman I am speaking about"
  ONSEM_NOANSWER(operator_react("Eva", semMem, lingDb));
  ONSEM_ANSWER_EQ("La femme dont je parle est Eva.",
                  operator_react("Qui est la femme dont tu parles ?", semMem, lingDb));
  ONSEM_ANSWER_EQ("La femme dont je parle est Eva.",
                  operator_react("La femme dont tu parles.", semMem, lingDb, SemanticLanguageEnum::UNKNOWN,
                                 &canReactToANoun));

  ONSEM_QUESTION_EQ("Qu'est-ce que pizza ?",
                    operator_react("pizza", semMem, lingDb, SemanticLanguageEnum::UNKNOWN,
                                   &canReactToANoun));
  ONSEM_NOANSWER(operator_react("un plat", semMem, lingDb));
  ONSEM_ANSWER_EQ("Pizza est un plat.",
                  operator_react("pizza", semMem, lingDb, SemanticLanguageEnum::UNKNOWN,
                                 &canReactToANoun));
  ONSEM_ANSWER_EQ("Une pizza est un plat.",
                  operator_react("une pizza", semMem, lingDb, SemanticLanguageEnum::UNKNOWN,
                                 &canReactToANoun));
  ONSEM_ANSWER_EQ("La pizza est un plat.",
                  operator_react("la pizza", semMem, lingDb, SemanticLanguageEnum::UNKNOWN,
                                 &canReactToANoun));
  ONSEM_ANSWER_EQ("Une pizza est un plat.",
                  operator_react("qu'est-ce qu'une pizza", semMem, lingDb));

  ONSEM_QUESTION_EQ("Qu'est-ce qu'une table ?",
                    operator_react("une table", semMem, lingDb, SemanticLanguageEnum::UNKNOWN,
                                   &canReactToANoun));
  ONSEM_NOANSWER(operator_react("c'est un meuble", semMem, lingDb));
  ONSEM_ANSWER_EQ("Une table est un meuble.",
                  operator_react("une table", semMem, lingDb, SemanticLanguageEnum::UNKNOWN,
                                 &canReactToANoun));
}


TEST_F(SemanticReasonerGTests, operator_react_saveAnswerToObjectQuestions)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;

  operator_inform_fromRobot("qu'est-ce que tu aimes ?", semMem, lingDb);
  operator_mergeAndInform("le chocolat", semMem, lingDb);
  ONSEM_ANSWER_EQ("Tu aimes le chocolat.",
                  operator_react("qu'est-ce que j'aime ?", semMem, lingDb));
}

TEST_F(SemanticReasonerGTests, operator_react_saveAnswerToYesOrNoQuestions)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;

  operator_inform_fromRobot("aimes-tu le chocolat ?", semMem, lingDb);
  operator_mergeAndInform("oui", semMem, lingDb);
  ONSEM_ANSWER_EQ("Tu aimes le chocolat.",
                  operator_react("qu'est-ce que j'aime ?", semMem, lingDb));
}


TEST_F(SemanticReasonerGTests, operator_react_checkReactionType)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;

  memoryOperation::defaultKnowledge(semMem, lingDb);
  const std::string raiseHeadCommandStr = "\\" + resourceLabelForTests_cmd + "=semantic/animations/look_up.qianim\\";
  operator_inform("to raise my head means " + raiseHeadCommandStr, semMem, lingDb);
  operator_inform("this mistake is nothing", semMem, lingDb);
  operator_inform("Noé est mon frère", semMem, lingDb);
  operator_inform("Gustave aime lire", semMem, lingDb);
  operator_inform("être content c'est dire super", semMem, lingDb);

  ONSEM_NOTIFYSOMETHINGWILLBEDONE_EQ("Ok, je dirai miaouh si quelqu'un touche ma tête.",
                                     operator_react("si quelqu'un touche ta tête, dis miaouh", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ(raiseHeadCommandStr, operator_react("lève la tête", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("Super", operator_react("sois content", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("Super", operator_react("sois content s'il te plaît", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("Super", operator_react("Je veux que tu sois content", semMem, lingDb));
  ONSEM_ANSWER_EQ("Noé est ton frère.", operator_react("Qui est Noé ?", semMem, lingDb));
  ONSEM_QUESTION_EQ("Qui est Paul ?", operator_react("Dis à Paul que je suis content pour lui", semMem, lingDb));
  ONSEM_QUESTION_EQ("Qui est Jacques ?", operator_react("Si tu es content dis bonjour à Jacques", semMem, lingDb));
  ONSEM_FEEDBACK_EQ("Gustave aime aussi lire.", operator_react("Laurent aime lire.", semMem, lingDb));
  ONSEM_BEHAVIORNOTFOUND_EQ("Je ne sais pas chanter.", operator_react("chante", semMem, lingDb));
  ONSEM_BEHAVIORNOTFOUND_EQ("Je ne sais pas ouvrir la bouche.", operator_react("si on touche ta tête, ouvre la bouche", semMem, lingDb));
  ONSEM_ANSWERNOTFOUND_EQ("Je ne sais pas quel est le sens de la vie.", operator_react("quel est le sens de la vie ?", semMem, lingDb));
  ONSEM_NOANSWER(operator_react("en fait", semMem, lingDb));
}


TEST_F(SemanticReasonerGTests, operator_react_checkReferences)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;

  operator_informAxiom_fromRobot("I am a robot.", semMem, lingDb, {"Own body"});
  // TODO: Fix with sentence "Spiderman is Paul."
  operator_informAxiom_fromRobot("Paul is Spiderman.", semMem, lingDb, {"Spiderman movie"});
  operator_mergeAndInform("Cesar likes Egypt.", semMem, lingDb, {"History book", "Asterix"});
  operator_mergeAndInform("Cesar has a big empire because he wins a lot of battles.", semMem, lingDb, {"History book of Cesar"});

  compareWithRef("Yes, Cesar likes Egypt.", {"History book", "Asterix"},
                 operator_react("Does Cesar like Egypt?", semMem, lingDb));
  compareWithRef("Cesar likes Egypt.", {"History book", "Asterix"},
                 operator_react("What does Cesar like ?", semMem, lingDb));
  compareWithRef("Cesar has a big empire because he wins a battles lot.", {"History book of Cesar"},
                 operator_react("Why Cesar has a big empire ?", semMem, lingDb));
  compareWithRef("Cesar", {"History book", "Asterix"},
                 operator_react("Who likes Egypt ?", semMem, lingDb));
  compareWithRef("Yes, I know I am a robot.", {"Own body"},
                 operator_react("You are a robot", semMem, lingDb));
  compareWithRef("No, I am a robot.", {"Own body"},
                 operator_react("You are not a robot", semMem, lingDb));
  compareWithRef("Cesar also has a big empire.", {"History book of Cesar"},
                 operator_react("Dede has a big empire", semMem, lingDb));
  compareWithRef("Spiderman is Paul.", {"Spiderman movie"},
                 operator_react("Who is Spiderman?", semMem, lingDb));
  compareWithRef("Oui, Spiderman est Paul.", {"Spiderman movie"},
                 operator_react("Est-ce que Spiderman est Paul?", semMem, lingDb));

  SemanticMemory semMem2;
  semMem2.memBloc.subBlockPtr = &semMem.memBloc;
  compareWithRef("Cesar likes Egypt.", {"History book", "Asterix"},
                 operator_react("What does Cesar like ?", semMem2, lingDb));
  // 2 levels of depth
  {
    SemanticMemory semMemTopLevel;
    semMemTopLevel.memBloc.subBlockPtr = &semMem2.memBloc;
    compareWithRef("Cesar likes Egypt.", {"History book", "Asterix"},
                   operator_react("What does Cesar like ?", semMemTopLevel, lingDb));
  }
}


TEST_F(SemanticReasonerGTests, operator_react_occurrenceOrderOfPastMemories)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;

  operator_inform("I like you", semMem, lingDb);
  operator_inform("to say \\p_meta=0\\ means \\p_meta=0\\", semMem, lingDb);

  // ask about last thing that was written
  ONSEM_ANSWER_EQ("The last thing that you said is to say something means something.",
                  operator_react("what is the last thing that I said ?", semMem, lingDb));
  ONSEM_ANSWER_EQ("The first thing that you said is you like me.",
                  operator_react("what is the first thing that I said ?", semMem, lingDb));
  ONSEM_ANSWER_EQ("The first thing that you said is you like me.",
                  operator_react("what is the first thing I said ?", semMem, lingDb));
  ONSEM_ANSWER_EQ("You said you like me.",
                  operator_react("what thing did I say for the first time?", semMem, lingDb));
  ONSEM_ANSWER_EQ("You said you like me.",
                  operator_react("what did I say for the first time", semMem, lingDb));

  // repeat learning
  ONSEM_BEHAVIOR_EQ("You said you like me.",
                    operator_react("say the last thing that you said", semMem, lingDb));
  operator_inform("to repeat means to say the last thing that you said", semMem, lingDb);
  ONSEM_BEHAVIOR_EQ("You said you like me.",
                    operator_react("repeat", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("Hi",
                    operator_react("say hi", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("Hi",
                    operator_react("repeat", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("Hi",
                    operator_react("repeat", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("Hi",
                    operator_react("repeat please", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("Hi",
                    operator_react("please repeat", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("Hi",
                    operator_react("alright repeat", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("Hi",
                    operator_react("Alright repeat", semMem, lingDb));
}


TEST_F(SemanticReasonerGTests, operator_react_resourceWithSpecificLanguage)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;

  const std::string womandSiningCommandStr_en =
      "\\" + resourceLabelForTests_url + "=#en_US#https://www.youtube.com/watch?v=tQmEd_UeeIk\\";
  operator_inform(womandSiningCommandStr_en + " is a woman singing", semMem, lingDb);
  const std::string womandSiningCommandStr_fr =
      "\\" + resourceLabelForTests_url + "=#fr_FR#https://www.youtube.com/watch?v=6uMS9_E-z4g\\";
  operator_inform(womandSiningCommandStr_fr + " is a woman singing", semMem, lingDb);
  ONSEM_ANSWER_EQ(womandSiningCommandStr_en, operator_react("show me a woman singing", semMem, lingDb));
  ONSEM_ANSWER_EQ(womandSiningCommandStr_en, operator_react("show me a woman who is singing", semMem, lingDb));
  ONSEM_ANSWER_EQ(womandSiningCommandStr_en, operator_react("show me a woman who sings", semMem, lingDb));
  ONSEM_ANSWER_EQ(womandSiningCommandStr_fr, operator_react("montre-moi une femme qui chante", semMem, lingDb));
  ONSEM_ANSWER_EQ(womandSiningCommandStr_fr, operator_react("montre-moi une femme chantant", semMem, lingDb));
  ONSEM_BEHAVIORNOTFOUND_EQ("Je ne sais pas te montrer une femme qui court.",
                            operator_react("montre-moi une femme qui court", semMem, lingDb));
}


TEST_F(SemanticReasonerGTests, operator_react_cause)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;

  const std::string urlStr1 =
      "\\" + resourceLabelForTests_url + "=https://www.youtube.com/watch?v=4yQil_1ZA98\\";
  operator_inform("il y a une année de joie parce que " + urlStr1, semMem, lingDb);
  ONSEM_ANSWER_EQ("Il y a une année de joie parce que " + urlStr1,
                  operator_react("pourquoi il y a une année de joie", semMem, lingDb));
  const std::string urlStr2 =
      "\\" + resourceLabelForTests_url + "=https://www.youtube.com/watch?v=4GZNoxrKZUQ\\";
  operator_inform(urlStr2 + " est pour cela qu'il y a de l'espérance.", semMem, lingDb);
  ONSEM_ANSWER_EQ("Il y a de l'espérance parce que " + urlStr2,
                  operator_react("pourquoi il y a de l'espérance", semMem, lingDb));
  const std::string urlStr3 =
      "\\" + resourceLabelForTests_url + "=https://www.youtube.com/watch?v=ZYCV4DmWbkw\\";
  operator_inform(urlStr3 + ". C'est pourquoi Paul est content.", semMem, lingDb);
  ONSEM_ANSWER_EQ("Paul est content parce que " + urlStr3,
                  operator_react("pourquoi Paul est content", semMem, lingDb));
  const std::string urlStr4 =
      "\\" + resourceLabelForTests_url + "=https://www.youtube.com/watch?v=zMyaKqtu0TE\\";
  operator_inform(urlStr4 + ". c'est pour cela que je suis content", semMem, lingDb);
  ONSEM_ANSWER_EQ("Tu es content parce que " + urlStr4,
                  operator_react("pourquoi je suis content ?", semMem, lingDb));
  ONSEM_ANSWER_EQ("Oui, tu es content.",
                  operator_react("je suis content ?", semMem, lingDb));
  ONSEM_ANSWER_EQ("Tu es content parce que " + urlStr4,
                  operator_react("pourquoi ?", semMem, lingDb));
}


TEST_F(SemanticReasonerGTests, operator_react_sayThanks)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;

  ONSEM_FEEDBACK_EQ("Thanks, you are kind.",
                    operator_react("I like you", semMem, lingDb));
  ONSEM_FEEDBACK_EQ("Merci, tu es gentil.",
                    operator_react("Je t'aime", semMem, lingDb));
  ONSEM_FEEDBACK_EQ("Merci, tu es gentil.",
                    operator_react("Je t'aime", semMem, lingDb));

  operator_inform("You like everybody", semMem, lingDb);
  ONSEM_FEEDBACK_EQ("Thanks, I also like you.",
                    operator_react("I like you", semMem, lingDb));
  ONSEM_FEEDBACK_EQ("Merci, je t'aime aussi.",
                    operator_react("Je t'aime", semMem, lingDb));

  ONSEM_FEEDBACK_EQ("Thanks, you are kind.",
                    operator_react("You are cool", semMem, lingDb));
  ONSEM_FEEDBACK_EQ("Merci, tu es gentil.",
                    operator_react("tu es cool", semMem, lingDb));
}


TEST_F(SemanticReasonerGTests, operator_react_sayYouAreWelcome)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;

  ONSEM_FEEDBACK_EQ("You're welcome",
                    operator_react("Thanks", semMem, lingDb));
  ONSEM_FEEDBACK_EQ("You're welcome",
                    operator_react("thanks.", semMem, lingDb));
  ONSEM_FEEDBACK_EQ("You're welcome",
                    operator_react("Thank you", semMem, lingDb));

  ONSEM_FEEDBACK_EQ("De rien",
                    operator_react("Merci", semMem, lingDb));
  ONSEM_FEEDBACK_EQ("De rien",
                    operator_react("merci.", semMem, lingDb));
}


TEST_F(SemanticReasonerGTests, operator_react_onInterjections)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;
  memoryOperation::learnSayCommand(semMem, lingDb);
  static const std::string sorryIWillTryToImproveMyslefEnglish = "Sorry, I will try to improve myself.";

  ONSEM_FEEDBACK_EQ("Nice, you like that.",
                    operator_react("It's nice", semMem, lingDb));
  ONSEM_FEEDBACK_EQ("Cool, you like that.",
                    operator_react("it's cool", semMem, lingDb));
  ONSEM_FEEDBACK_EQ("Awesome, you like that.",
                    operator_react("It's awesome", semMem, lingDb));
  ONSEM_FEEDBACK_EQ(sorryIWillTryToImproveMyslefEnglish,
                    operator_react("It's not nice", semMem, lingDb));
  ONSEM_FEEDBACK_EQ(sorryIWillTryToImproveMyslefEnglish,
                    operator_react("it's not cool", semMem, lingDb));
  ONSEM_FEEDBACK_EQ(sorryIWillTryToImproveMyslefEnglish,
                    operator_react("It's not awesome", semMem, lingDb));
  ONSEM_FEEDBACK_EQ("Super, tu l'aimes.",
                    operator_react("C'est super", semMem, lingDb));
  ONSEM_FEEDBACK_EQ("Désolé, j'essaierai de m'améliorer.",
                    operator_react("C'est pas super", semMem, lingDb));

  ONSEM_FEEDBACK_EQ("Nice, you like that.",
                    operator_react("Nice", semMem, lingDb));
  ONSEM_FEEDBACK_EQ("Cool, you like that.",
                    operator_react("cool", semMem, lingDb));
  ONSEM_FEEDBACK_EQ("Awesome, you like that.",
                    operator_react("Awesome", semMem, lingDb));
  ONSEM_FEEDBACK_EQ(sorryIWillTryToImproveMyslefEnglish,
                    operator_react("Not nice", semMem, lingDb));
  ONSEM_FEEDBACK_EQ(sorryIWillTryToImproveMyslefEnglish,
                    operator_react("not cool", semMem, lingDb));
  ONSEM_FEEDBACK_EQ(sorryIWillTryToImproveMyslefEnglish,
                    operator_react("Not awesome", semMem, lingDb));
  ONSEM_FEEDBACK_EQ(sorryIWillTryToImproveMyslefEnglish,
                    operator_react("F*cking jackass", semMem, lingDb));

  // check that it can be overwrited
  ONSEM_NOTIFYSOMETHINGWILLBEDONE_EQ("Ok, I will say you are the kind with me whenever you say awesome.",
                                     operator_react("when I say awesome say you are kind with me", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("You are the kind with me.",
                    operator_react("Awesome", semMem, lingDb));
}


TEST_F(SemanticReasonerGTests, operator_react_objectsMergeAndRemoval)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;

  operator_inform("I see a moving object in my personal zone", semMem, lingDb);
  ONSEM_ANSWER_EQ("You see a moving object.",
                  operator_react("what do I see ?", semMem, lingDb));
  operator_inform("I see many moving objects next to me", semMem, lingDb);
  ONSEM_ANSWER_EQ("You see a moving object and many moving objects.",
                  operator_react("what do I see ?", semMem, lingDb));
  ONSEM_FEEDBACK_EQ("I thought you saw many moving objects close to you.",
                    operator_react("I don't see any moving objects next to me", semMem, lingDb));
  ONSEM_ANSWER_EQ("You see a moving object.",
                  operator_react("what do I see ?", semMem, lingDb));
  operator_inform("I see many moving objects far away from me", semMem, lingDb);
  ONSEM_ANSWER_EQ("You see a moving object and many moving objects.",
                  operator_react("what do I see ?", semMem, lingDb));
  operator_inform("I see many people far away from me", semMem, lingDb);
  ONSEM_ANSWER_EQ("You see a moving object, many moving objects and many people.",
                  operator_react("what do I see ?", semMem, lingDb));
}


TEST_F(SemanticReasonerGTests, operator_react_evenIfTheIsASpellingMistake)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;

  operator_mergeAndInform("Alexis looks at a chair", semMem, lingDb);
  ONSEM_ANSWER_EQ("Alexis looks at a chair.",
                  operator_react("what Alexis look at?", semMem, lingDb));
  ONSEM_ANSWER_EQ("Alexis looks at a chair.",
                  operator_react("what Alexis looks at?", semMem, lingDb));
  ONSEM_ANSWER_EQ("Alexis looks at a chair.",
                  operator_react("what does Alexis look at?", semMem, lingDb)); // this one doesn't have a spelling mistake ;)
}


TEST_F(SemanticReasonerGTests, operator_react_choice)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;

  const std::string question = "Do I like chocolate or banana?";
  ONSEM_QUESTION_EQ("(\tI don't know.\tTHEN\tDo you like chocolate or banana?\t)", operator_react(question, semMem, lingDb));
  ONSEM_QUESTION_EQ("(\tJe ne sais pas.\tTHEN\tAimes-tu le chocolat ou la banane ?\t)",
                    operator_react("J'aime le chocolat ou la banane ?", semMem, lingDb));
  operator_inform("I don't like chocolate", semMem, lingDb);
  ONSEM_ANSWER_EQ("You don't like chocolate.", operator_react(question, semMem, lingDb));
  operator_inform("I like banana", semMem, lingDb);
  ONSEM_ANSWER_EQ("You like banana.", operator_react(question, semMem, lingDb));
}


TEST_F(SemanticReasonerGTests, operator_react_abilityQuestion)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;

  ONSEM_ANSWERNOTFOUND_EQ("I don't know if you can say something.",
                          operator_react("can I say something?", semMem, lingDb));
  ONSEM_ANSWER_EQ("No, I can't.", operator_react("can you say something?", semMem, lingDb));
  memoryOperation::learnSayCommand(semMem, lingDb);
  ONSEM_QUESTION_EQ("(\tYes, I can.\tTHEN\tDo you want me to say something now?\t)", operator_react("can you say something?", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("Something", operator_react("yes", semMem, lingDb));
  ONSEM_QUESTION_EQ("(\tOui, je peux.\tTHEN\tVeux-tu que je dise bonjour maintenant ?\t)", operator_react("sais-tu dire bonjour ?", semMem, lingDb));
  ONSEM_NOANSWER(operator_react("je ne sais pas", semMem, lingDb));
  ONSEM_QUESTION_EQ("Veux-tu que je dise bonjour maintenant ?", operator_react("pas du tout", semMem, lingDb));
  ONSEM_FEEDBACK_EQ("Ok, tu ne veux pas que je dise bonjour maintenant.", operator_react("non", semMem, lingDb));
  ONSEM_QUESTION_EQ("(\tOui, je peux.\tTHEN\tVeux-tu que je dise bonjour maintenant ?\t)", operator_react("sais-tu dire bonjour ?", semMem, lingDb));
  ONSEM_NOANSWER(operator_react("je ne sais pas", semMem, lingDb));
  ONSEM_QUESTION_EQ("Veux-tu que je dise bonjour maintenant ?", operator_react("Allez oui!", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("Bonjour", operator_react("ouep", semMem, lingDb));

  memoryOperation::allowToInformTheUserHowToTeach(semMem);
  ONSEM_QUESTION_EQ("(\tNo, I can't but you can teach me.\tTHEN\tDo you want to know how?\t)", operator_react("can you sing?", semMem, lingDb));
  ONSEM_ANSWER_EQ("For example, you can tell me to sing is to say I am singing.", operator_react("yes", semMem, lingDb));
  ONSEM_TEACHINGFEEDBACK_EQ("Ok, to sing is to say I am singing.", operator_react("to sing is to say I am singing", semMem, lingDb));
  ONSEM_QUESTION_EQ("(\tYes, I can.\tTHEN\tDo you want me to sing now?\t)", operator_react("can you sing", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("I am singing.", operator_react("yes", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("I am singing.", operator_react("sing", semMem, lingDb));

  ONSEM_QUESTION_EQ("(\tNon, je ne peux pas mais tu peux m'apprendre.\tTHEN\tVeux-tu savoir comment ?\t)", operator_react("peux-tu courir ?", semMem, lingDb));
  ONSEM_ANSWER_EQ("Par exemple, tu peux me dire que courir c'est dire je cours.", operator_react("oui", semMem, lingDb));
  ONSEM_TEACHINGFEEDBACK_EQ("Ok, courir c'est dire je cours.", operator_react("courir c'est dire je cours.", semMem, lingDb));
  ONSEM_QUESTION_EQ("(\tOui, je peux.\tTHEN\tVeux-tu que je coure maintenant ?\t)", operator_react("peux-tu courir ?", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("Je cours.", operator_react("oui", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("Je cours.", operator_react("cours", semMem, lingDb));

  operator_inform("to crouch is to say I am crouched", semMem, lingDb);
  ONSEM_ANSWER_EQ("I can say something, ask something, repeat, sing, run and crouch.", operator_react("what can you do?", semMem, lingDb));
}


TEST_F(SemanticReasonerGTests, operator_react_onEngageDisengage)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;

  ONSEM_NOANSWER(operator_react("hello buddy", semMem, lingDb));
  operator_mergeAndInform("your name is buddy", semMem, lingDb);
  ONSEM_FEEDBACK_EQ("Hello human", operator_react("hello buddy", semMem, lingDb));
  ONSEM_FEEDBACK_EQ("Hello human", operator_react("hello Buddy", semMem, lingDb));
  ONSEM_FEEDBACK_EQ("Hello human", operator_react("Hello Buddy", semMem, lingDb));
  ONSEM_FEEDBACK_EQ("Hi humain", operator_react("Hi Buddy", semMem, lingDb));
  ONSEM_FEEDBACK_EQ("Bonjour humain", operator_react("bonjour buddy", semMem, lingDb));
  ONSEM_FEEDBACK_EQ("Bonjour humain", operator_react("bonjour Buddy", semMem, lingDb));
  ONSEM_FEEDBACK_EQ("Bonjour humain", operator_react("Bonjour Buddy", semMem, lingDb));
  ONSEM_FEEDBACK_EQ("Salut humain", operator_react("Salut Buddy", semMem, lingDb));
  operator_mergeAndInform("je suis Yves", semMem, lingDb);
  ONSEM_FEEDBACK_EQ("Hello Yves", operator_react("Hello Buddy", semMem, lingDb));
  ONSEM_FEEDBACK_EQ("Bonjour Yves", operator_react("Bonjour Buddy", semMem, lingDb));
  ONSEM_FEEDBACK_EQ("Bye Yves", operator_react("Bye Buddy", semMem, lingDb));
  ONSEM_FEEDBACK_EQ("Bye-bye Yves", operator_react("Bye-bye Buddy", semMem, lingDb));
  ONSEM_FEEDBACK_EQ("Au revoir Yves", operator_react("Au revoir Buddy", semMem, lingDb));
}



TEST_F(SemanticReasonerGTests, operator_react_testNameAssignement)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;
  memoryOperation::learnSayCommand(semMem, lingDb);

  ONSEM_FEEDBACK_EQ("Nice to meet you Paul", operator_react("I am Paul", semMem, lingDb));
  EXPECT_TRUE(test_knowTheNameOf("Paul", semMem, lingDb));
  ONSEM_FEEDBACK_EQ("Enchanté André", operator_react("je suis André", semMem, lingDb));
  EXPECT_TRUE(test_knowTheNameOf("André", semMem, lingDb));
  semMem.setCurrUserId("42");
  ONSEM_FEEDBACK_EQ("Enchanté Aurélia", operator_react("Aurélia c'est moi", semMem, lingDb));
  EXPECT_TRUE(test_knowTheNameOf("Aurélia", semMem, lingDb));
  semMem.setCurrUserId("43");
  ONSEM_FEEDBACK_EQ("Enchanté Sandrine", operator_react("Sandrine c'est mon nom", semMem, lingDb));
  EXPECT_TRUE(test_knowTheNameOf("Sandrine", semMem, lingDb));
}


TEST_F(SemanticReasonerGTests, operator_react_linkUserToAnAlreadyKnowAgent)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;

  // add information about Donna
  ONSEM_NOANSWER(operator_react("Donna is a goalkeeper", semMem, lingDb));

  // add information about the current user
  ONSEM_QUESTION_EQ("(\tI don't know.\tTHEN\tDo you like to swim?\t)", operator_react("do I like to swim?", semMem, lingDb));
  ONSEM_QUESTION_EQ("Why do you like to swim?", operator_react("yeah", semMem, lingDb));
  ONSEM_QUESTION_EQ("Why is it funny?", operator_react("because it's funny", semMem, lingDb));

  // add Donna information to the current user
  ONSEM_QUESTION_EQ("(\tI don't know.\tTHEN\tAre you a goalkeeper?\t)", operator_react("am I a goalkeeper?", semMem, lingDb));
  ONSEM_FEEDBACK_EQ("Nice to meet you Donna", operator_react("I am Donna", semMem, lingDb));
  ONSEM_ANSWER_EQ("Yes, you are a goalkeeper.", operator_react("am I a goalkeeper?", semMem, lingDb));

  // remove Donna information from current user
  ONSEM_FEEDBACK_EQ("I thought the opposite.", operator_react("I am not Donna", semMem, lingDb));
  ONSEM_QUESTION_EQ("(\tI don't know.\tTHEN\tAre you a goalkeeper?\t)", operator_react("am I a goalkeeper?", semMem, lingDb));

  // check that other information about the current user are still there
  ONSEM_ANSWER_EQ("Yes, you like to swim.", operator_react("do I like to swim?", semMem, lingDb));
  ONSEM_ANSWER_EQ("You like to swim because it's funny.", operator_react("why", semMem, lingDb));
}



TEST_F(SemanticReasonerGTests, operator_react_allowToHaveManyNames)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;

  ONSEM_NOANSWER(operator_react("Anne likes chocolate", semMem, lingDb));
  ONSEM_FEEDBACK_EQ("Nice to meet you Anne", operator_react("I am Anne", semMem, lingDb));
  ONSEM_FEEDBACK_EQ("Nice to meet you Capat", operator_react("I am Capat", semMem, lingDb));
  ONSEM_ANSWER_EQ("You like chocolate.", operator_react("what do I like", semMem, lingDb));
  ONSEM_NOANSWER(operator_react("Capat likes beer", semMem, lingDb));
  ONSEM_ANSWER_EQ("You like chocolate and beer.", operator_react("what do I like", semMem, lingDb));
  ONSEM_ANSWER_EQ("You are Capat.", operator_react("who am I", semMem, lingDb));
  ONSEM_ANSWER_EQ("Yes, you are Anne.", operator_react("Am I Anne", semMem, lingDb));
  ONSEM_ANSWER_EQ("Yes, you are Capat.", operator_react("Am I Capat", semMem, lingDb));
  ONSEM_ANSWER_EQ("No, you are Capat.", operator_react("Am I Dede", semMem, lingDb));
  ONSEM_FEEDBACK_EQ("I thought the opposite.", operator_react("I am not Capat", semMem, lingDb));
  ONSEM_ANSWER_EQ("No, you are Anne.", operator_react("Am I Capat", semMem, lingDb));
  ONSEM_ANSWER_EQ("Yes, you are Anne.", operator_react("Am I Anne", semMem, lingDb));
  // TODO: fix question "Am I Dede" here
  ONSEM_ANSWER_EQ("You like chocolate.", operator_react("what do I like", semMem, lingDb));
}


TEST_F(SemanticReasonerGTests, operator_react_whenTheRobotHearHisName)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;

  ONSEM_NOANSWER(operator_react("Buddy", semMem, lingDb, SemanticLanguageEnum::ENGLISH));
  operator_mergeAndInform("your name is buddy", semMem, lingDb);
  ONSEM_FEEDBACK_EQ("It's me.", operator_react("Buddy", semMem, lingDb, SemanticLanguageEnum::ENGLISH));
  ONSEM_FEEDBACK_EQ("C'est moi.", operator_react("Buddy", semMem, lingDb, SemanticLanguageEnum::FRENCH));
}


TEST_F(SemanticReasonerGTests, operator_react_learnAnAction)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;

  semMem.proativeSpecifications.canLearnANewAxiomaticnAction = true;
  ONSEM_EXTERNALTEACHINGREQUEST_EQ("\\learn=#en_US#To salute.\\", operator_react("I will teach you how to salute", semMem, lingDb));
  ONSEM_EXTERNALTEACHINGREQUEST_EQ("\\learn=#en_US#To swap my ass.\\", operator_react("I'll teach you how to swap your ass", semMem, lingDb));
  ONSEM_EXTERNALTEACHINGREQUEST_EQ("\\learn=#en_US#To jump.\\", operator_react("I am going to teach you how to jump", semMem, lingDb));
  ONSEM_EXTERNALTEACHINGREQUEST_EQ("\\learn=#en_US#To go forward.\\", operator_react("I am going to teach you how to go forward", semMem, lingDb));
  ONSEM_EXTERNALTEACHINGREQUEST_EQ("\\learn=#fr_FR#Saluer.\\", operator_react("je vais t'apprendre à saluer", semMem, lingDb));
}


TEST_F(SemanticReasonerGTests, operator_react_conditionalActions)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;
  memoryOperation::learnSayCommand(semMem, lingDb);

  ONSEM_BEHAVIORNOTFOUND_EQ("Je ne sais pas chanter.",
                            operator_react("chante", semMem, lingDb));
  ONSEM_BEHAVIORNOTFOUND_EQ("Je ne sais pas chanter.",
                            operator_react("chante si tu me vois", semMem, lingDb));
  ONSEM_NOTIFYSOMETHINGWILLBEDONE_EQ("Ok, je dirai bonjour si je te vois.",
                                     operator_react("dis bonjour si tu me vois", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("Bonjour",
                    operator_react("tu me vois", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("You are glad and happy.",
                    operator_react("dis en anglais que je suis content et heureux", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("You will be glad if I see you.",
                    operator_react("dis en anglais que je serai content si tu me vois", semMem, lingDb));
}


TEST_F(SemanticReasonerGTests, operator_react_answerOfGeneratedQuestions)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;
  memoryOperation::learnSayCommand(semMem, lingDb);

  ONSEM_BEHAVIOR_EQ("Es-tu sympa ?",
                    operator_react("demande-moi si je suis sympa", semMem, lingDb));
  ONSEM_QUESTION_EQ("(\tJe suis content de l'entendre.\tTHEN\tPourquoi es-tu sympa ?\t)",
                    operator_react("oui", semMem, lingDb));

  // + translation
  ONSEM_BEHAVIOR_EQ("Es-tu content ?",
                    operator_react("ask me in French if I am happy", semMem, lingDb));
  ONSEM_QUESTION_EQ("(\tJe suis content de l'entendre.\tTHEN\tPourquoi es-tu content ?\t)",
                    operator_react("oui", semMem, lingDb));
}

TEST_F(SemanticReasonerGTests, operator_react_similarities)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;

  ONSEM_QUESTION_EQ("Où est né Alexandre le 5 janvier 1979 ?",
                    operator_react("Alexandre est né le 5 janvier 1979.", semMem, lingDb));
  ONSEM_NOANSWER(operator_react("Damien est né le 3 avril 1978 à Paris.", semMem, lingDb));
  ONSEM_FEEDBACK_EQ("Damien est aussi né à Paris.",
                    operator_react("je suis né le 20 septembre 1986 à Paris.", semMem, lingDb));
  ONSEM_FEEDBACK_EQ("Damien est aussi né le 3 avril 1978.",
                    operator_react("je suis né le 3 avril 1978.", semMem, lingDb));
  ONSEM_FEEDBACK_EQ("Je pensais le contraire.",
                    operator_react("je suis né le 3 mars 1995.", semMem, lingDb));
  ONSEM_FEEDBACK_EQ("Damien est aussi né le 3 avril.",
                    operator_react("je suis né le 3 avril 1995.", semMem, lingDb));
  operator_inform_fromRobot("Quand es-tu né ?", semMem, lingDb);
  ONSEM_FEEDBACK_EQ("Damien est aussi né en avril 1978.",
                    operator_react("5 avril 1978.", semMem, lingDb));
  operator_inform_fromRobot("Quand es-tu né ?", semMem, lingDb);
  ONSEM_FEEDBACK_EQ("Damien est aussi né en avril.",
                    operator_react("le 5 avril 1995.", semMem, lingDb));
}

TEST_F(SemanticReasonerGTests, operator_react_changeTheUser_namesSaving)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;

  semMem.setCurrUserId("1");
  ONSEM_FEEDBACK_EQ("Enchanté Paul",
                    operator_react("je m'appelle Paul", semMem, lingDb));
  ONSEM_ANSWER_EQ("Tu t'appelles Paul.",
                  operator_react("comment je m'appelle ?", semMem, lingDb));
  ONSEM_FEEDBACK_EQ("Enchanté Pierre",
                    operator_react("je m'appelle Pierre", semMem, lingDb));
  ONSEM_ANSWER_EQ("Tu t'appelles Pierre.",
                  operator_react("comment je m'appelle ?", semMem, lingDb));
  semMem.setCurrUserId("2");
  ONSEM_QUESTION_EQ("(\tJe ne sais pas.\tTHEN\tComment t'appelles-tu ?\t)",
                    operator_react("comment je m'appelle ?", semMem, lingDb));
  ONSEM_FEEDBACK_EQ("Nice to meet you André",
                    operator_react("André", semMem, lingDb));
  ONSEM_ANSWER_EQ("Tu t'appelles André.",
                  operator_react("comment je m'appelle ?", semMem, lingDb));
  semMem.setCurrUserId("1");
  ONSEM_ANSWER_EQ("Tu t'appelles Pierre.",
                  operator_react("comment je m'appelle ?", semMem, lingDb));
  ONSEM_ANSWERNOTFOUND_EQ("Je ne sais pas comment ton père s'appelle.",
                          operator_react("Comment s'appelle mon père ?", semMem, lingDb));
  ONSEM_NOANSWER(operator_react("Philippe c'est mon père.", semMem, lingDb));
  ONSEM_ANSWER_EQ("Ton père s'appelle Philippe.",
                  operator_react("Comment s'appelle mon père ?", semMem, lingDb));
  // check that it still work after a serialization
  SemanticMemory semMem2;
  copyMemory(semMem2, semMem, lingDb);
  ONSEM_ANSWER_EQ("Ton père s'appelle Philippe.",
                  operator_react("Comment s'appelle mon père ?", semMem2, lingDb));
  ONSEM_TRUE(operator_check("Mon père s'appelle Philippe.", semMem2, lingDb));
  EXPECT_EQ(std::vector<std::string>{"Philippe"}, operator_get("Comment s'appelle mon père ?", semMem2, lingDb));
  ONSEM_FEEDBACK_EQ("Je pensais que Philippe était ton père.",
                    operator_react("Non, mon père c'est pas Philippe", semMem2, lingDb));
  ONSEM_ANSWERNOTFOUND_EQ("Je ne sais pas comment ton père s'appelle.",
                          operator_react("Comment s'appelle mon père ?", semMem2, lingDb));
  ONSEM_UNKNOWN(operator_check("Mon père s'appelle Philippe.", semMem2, lingDb));
  EXPECT_TRUE(operator_get("Comment s'appelle mon père ?", semMem2, lingDb).empty());
}


TEST_F(SemanticReasonerGTests, operator_react_changeTheUser_genderHandling)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;
  memoryOperation::learnSayCommand(semMem, lingDb);

  semMem.setCurrUserId("1");
  operator_mergeAndInform("I am Aaa", semMem, lingDb);
  semMem.setCurrUserId("2");
  ONSEM_BEHAVIOR_EQ("Aaa est un chanteur.",
                    operator_react("say in French that Aaa is a singer", semMem, lingDb));
  operator_mergeAndInform("Aaa is a girl", semMem, lingDb);
  ONSEM_BEHAVIOR_EQ("Aaa est une chanteuse.",
                    operator_react("say in French that Aaa is a singer", semMem, lingDb));
  semMem.setCurrUserId("3");
  operator_mergeAndInform("I am Bb", semMem, lingDb);
  semMem.setCurrUserId("2");
  ONSEM_BEHAVIOR_EQ("Aaa et Bb sont des chanteurs.",
                    operator_react("say in French that Aaa and Bb are singers", semMem, lingDb));
  ONSEM_FEEDBACK_EQ("Aaa is also a girl.",
                    operator_react("Bb is a girl", semMem, lingDb));
  ONSEM_BEHAVIOR_EQ("Aaa et Bb sont des chanteuses.",
                    operator_react("say in French that Aaa and Bb are singers", semMem, lingDb));

  ONSEM_BEHAVIOR_EQ("Aaa est une artiste.",
                    operator_react("say in French that Aaa is an artist", semMem, lingDb));
  operator_mergeAndInform("Aaa is a boy", semMem, lingDb);
  ONSEM_BEHAVIOR_EQ("Aaa est un artiste.",
                    operator_react("say in French that Aaa is an artist", semMem, lingDb));
}


TEST_F(SemanticReasonerGTests, operator_react_changeTheUser_auestionQboutUser)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;

  semMem.setCurrUserId("1");
  operator_mergeAndInform("I am Aaa", semMem, lingDb);
  semMem.setCurrUserId("2");
  operator_mergeAndInform("Aaa likes chocolate", semMem, lingDb);
  ONSEM_ANSWER_EQ("Aaa likes chocolate.",
                  operator_react("what does Aaa like?", semMem, lingDb));
  ONSEM_ANSWER_EQ("Aaa likes chocolate.",
                  operator_react("what Aaa likes?", semMem, lingDb)); // not correct english but it's understandable...
}


TEST_F(SemanticReasonerGTests, operator_react_changeTheUser_askAboutWhatAnotherUserWillDo)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;
  memoryOperation::learnSayCommand(semMem, lingDb);

  semMem.setCurrUserId("1");
  operator_mergeAndInform("I am Erwan", semMem, lingDb);
  semMem.setCurrUserId("2");
  operator_mergeAndInform("je suis Paul", semMem, lingDb);
  ONSEM_NOTIFYSOMETHINGWILLBEDONE_EQ("Ok, je demanderai à Erwan en anglais ce qu'Erwan fera ce soir.",
                                     operator_react("demande à Erwan en anglais ce qu'il fera ce soir", semMem, lingDb));
  {
    semMem.setCurrUserId("1");
    std::list<UniqueSemanticExpression> semExpsForThisSpecificUser;
    semMem.extractSemExpsForASpecificUser(semExpsForThisSpecificUser, "1");

    ASSERT_EQ(1u, semExpsForThisSpecificUser.size());
    ONSEM_BEHAVIOR_EQ("What will you do this evening?",
                      operator_react_fromSemExp(std::move(semExpsForThisSpecificUser.front()), semMem, lingDb,
                                                semMem.defaultLanguage));
  }
  ONSEM_FEEDBACK_EQ("Ok, you will go this evening to the cinema.",
                    operator_react("I will go to cinema", semMem, lingDb));
  semMem.setCurrUserId("2");
  ONSEM_ANSWER_EQ("Erwan ira ce soir au cinéma.",
                  operator_react("qu'est ce que Erwan fera ce soir ?", semMem, lingDb));
  ONSEM_ANSWER_EQ("Erwan ira ce soir au cinéma.",
                  operator_react("qu'est ce que Erwan va faire ce soir ?", semMem, lingDb));
}



TEST_F(SemanticReasonerGTests, operator_react_handleEverythingConcept)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;

  operator_inform("I like everybody", semMem, lingDb);
  ONSEM_ANSWER_EQ("You",
                  operator_react("who likes Paul?", semMem, lingDb));
}



TEST_F(SemanticReasonerGTests, operator_react_subBlockPtr)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory subSemMem;
  SemanticMemory semMem;

  operator_mergeAndInform("Paul likes banana", subSemMem, lingDb);
  ONSEM_ANSWERNOTFOUND_EQ("I don't know what Paul likes.",
                          operator_react("what does Paul like?", semMem, lingDb));
  semMem.memBloc.subBlockPtr = &subSemMem.memBloc;

  // 2 levels of depth
  {
    SemanticMemory semMemTopLevel;
    semMemTopLevel.memBloc.subBlockPtr = &semMem.memBloc;
    ONSEM_ANSWER_EQ("Yes, Paul likes banana.",
                    operator_react("Does Paul like banana?", semMemTopLevel, lingDb));
  }

  ONSEM_ANSWER_EQ("Paul likes banana.",
                  operator_react("what does Paul like?", semMem, lingDb));
  operator_mergeAndInform("Paul likes bear", semMem, lingDb);
  ONSEM_ANSWER_EQ("Paul likes banana and the bear.",
                  operator_react("what does Paul like?", semMem, lingDb));
  ONSEM_QUESTION_EQ("(\tJe ne sais pas.\tTHEN\tLe savais-tu ?\t)",
                    operator_react("Est-ce que je le savais ?", semMem, lingDb));
  ONSEM_ANSWERNOTFOUND_EQ("Je ne sais pas comment tu peux plus en savoir sur Paul.",
                          operator_react("Comment puis-je en savoir plus sur Paul ?", semMem, lingDb));
}
