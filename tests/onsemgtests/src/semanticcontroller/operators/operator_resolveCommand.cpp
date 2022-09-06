#include "operator_resolveCommand.hpp"
#include "../../semanticreasonergtests.hpp"
#include "operator_inform.hpp"
#include <gtest/gtest.h>
#include <onsem/texttosemantic/languagedetector.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemory.hpp>
#include <onsem/semantictotext/semexpoperators.hpp>
#include <onsem/semantictotext/semanticconverter.hpp>


using namespace onsem;

namespace onsem
{
std::string operator_resolveCommand(const std::string& pText,
                                    SemanticMemory& pSemanticMemory,
                                    const linguistics::LinguisticDatabase& pLingDb,
                                    SemanticLanguageEnum pLanguage)
{
  if (pLanguage == SemanticLanguageEnum::UNKNOWN)
    pLanguage = linguistics::getLanguage(pText, pLingDb);
  TextProcessingContext textProc(SemanticAgentGrounding::currentUser,
                                 SemanticAgentGrounding::me,
                                 pLanguage);
  auto semExp =
      converter::textToContextualSemExp(pText, textProc,
                                        SemanticSourceEnum::WRITTENTEXT,
                                        pLingDb);
  memoryOperation::mergeWithContext(semExp, pSemanticMemory, pLingDb);
  auto resSemExp = memoryOperation::resolveCommandFromMemBlock(*semExp, pSemanticMemory.memBloc, textProc.author.userId, pLingDb);
  if (resSemExp)
    return semExpToTextExectionResult(std::move(*resSemExp), pLanguage, pSemanticMemory, pLingDb);
  return "";
}

} // End of namespace onsem




TEST_F(SemanticReasonerGTests, operator_resolveCommand_basic)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;

  memoryOperation::learnSayCommand(semMem, lingDb);
  static const std::string resource_goToPostureStr = "\\" + resourceLabelForTests_cmd + "=ALRobotPosture.goToPosture(\"Stand\", 0.8)\\";
  operator_inform("to stand up means " + resource_goToPostureStr, semMem, lingDb);
  static const std::string resource_movebackStr = "\\" + resourceLabelForTests_cmd +"=semantic/animations/move_back.pmt\\";
  operator_inform("to move back means " + resource_movebackStr, semMem, lingDb);
  static const std::string resource_raiseLeftHandStr = "\\" + resourceLabelForTests_cmd + "=semantic/animations/raise_left_hand.qianim\\";
  operator_inform("to raise your left hand means " + resource_raiseLeftHandStr, semMem, lingDb);
  static const std::string resource_whiteEyesStr = "\\" + resourceLabelForTests_cmd + "=ALLeds.fadeRGB(\"FaceLeds\", \"white\", 1)\\";
  operator_inform("to have white eyes means " + resource_whiteEyesStr, semMem, lingDb);
  static const std::string resource_yellowEyesStr = "\\" + resourceLabelForTests_cmd + "=ALLeds.fadeRGB(\"FaceLeds\", \"yellow\", 1)\\";
  operator_inform("to have yellow eyes means " + resource_yellowEyesStr, semMem, lingDb);


  EXPECT_EQ("Yes",
            operator_resolveCommand("say yes", semMem, lingDb));
  EXPECT_EQ("Yes",
            operator_resolveCommand("please say yes", semMem, lingDb));
  EXPECT_EQ("No",
            operator_resolveCommand("say no", semMem, lingDb));
  static const std::string iAmVeryHappyToSeeYouInFrench = "\"Je suis très content dete voir!!\"";
  EXPECT_EQ(iAmVeryHappyToSeeYouInFrench,
            operator_resolveCommand("say " + iAmVeryHappyToSeeYouInFrench, semMem, lingDb, SemanticLanguageEnum::ENGLISH));
  EXPECT_EQ("Quelque chose",
            operator_resolveCommand("s'il te plait dis quelque chose", semMem, lingDb));
  EXPECT_EQ("Quelque chose",
            operator_resolveCommand("s'il te plaît dis quelque chose", semMem, lingDb));
  EXPECT_EQ("Salut tout le monde",
            operator_resolveCommand("dis salut tout le monde", semMem, lingDb));
  EXPECT_EQ("Bonjour",
            operator_resolveCommand("dis-moi bonjour", semMem, lingDb));
  EXPECT_EQ(resource_whiteEyesStr,
            operator_resolveCommand("have white eyes", semMem, lingDb));
  EXPECT_EQ(resource_goToPostureStr,
            operator_resolveCommand("stand up", semMem, lingDb));
  EXPECT_EQ(resource_movebackStr,
            operator_resolveCommand("move backward", semMem, lingDb));
  EXPECT_EQ(resource_movebackStr,
            operator_resolveCommand("move backwards", semMem, lingDb));
  EXPECT_EQ(resource_raiseLeftHandStr,
            operator_resolveCommand("lève la main gauche", semMem, lingDb));
  EXPECT_EQ(resource_raiseLeftHandStr,
            operator_resolveCommand("raise your left hand", semMem, lingDb));
  // operator_do("lève ta main gauche", semMem, lingDb);
  EXPECT_EQ("", operator_resolveCommand("you like chocolate", semMem, lingDb));
  EXPECT_EQ("", operator_resolveCommand("do goodbye", semMem, lingDb));

  // TODO: write this in a separate test case
  EXPECT_EQ("(\t" + resource_goToPostureStr + "\tAND\t" + resource_goToPostureStr + "\t)",
            operator_resolveCommand("stand up and stand up", semMem, lingDb));

  EXPECT_EQ("(\t" + resource_goToPostureStr + "\tTHEN\t" + resource_goToPostureStr + "\t)",
            operator_resolveCommand("stand up and then stand up", semMem, lingDb));

  EXPECT_EQ("(\t" + resource_goToPostureStr + "\tTHEN\t" + resource_movebackStr + "\t)",
            operator_resolveCommand("stand up and then move back", semMem, lingDb));

  EXPECT_EQ("(\t" + resource_goToPostureStr + "\tTHEN\t" + resource_raiseLeftHandStr + "\tTHEN\t" + resource_movebackStr + "\t)",
            operator_resolveCommand("stand up, then raise your left hand and then move back", semMem, lingDb));


  // replace expression by its value
  operator_inform("I am Paul.", semMem, lingDb);
  EXPECT_EQ("Paul",
            operator_resolveCommand("say my name", semMem, lingDb));
  operator_inform("I want to fly.", semMem, lingDb);
  EXPECT_EQ("You want to fly.",
            operator_resolveCommand("say what I want to do", semMem, lingDb));


  // learn dynamically some actions
  static const std::string resource_doGoodbyeStr = "\\" + resourceLabelForTests_cmd + "=GoobyeService.run()\\";
  operator_inform("to do goodbye means " + resource_doGoodbyeStr, semMem, lingDb);
  EXPECT_EQ(resource_doGoodbyeStr,
            operator_resolveCommand("do goodbye", semMem, lingDb));
  operator_inform("to do goodbye means to salute", semMem, lingDb);
  EXPECT_EQ("", operator_resolveCommand("do goodbye", semMem, lingDb));
  operator_inform("to salute means to say you are a nice man", semMem, lingDb);
  EXPECT_EQ("You are a nice man.",
            operator_resolveCommand("do goodbye", semMem, lingDb));
  operator_inform("to compliment means to do goodbye in french", semMem, lingDb);
  EXPECT_EQ("Tu es un homme agréable.",
            operator_resolveCommand("compliment", semMem, lingDb));

  // check understanding of some other teachings
  EXPECT_EQ("", operator_resolveCommand("Do a totem pole", semMem, lingDb));
  operator_inform("To do a totem pole is to say I am doing a totem pole", semMem, lingDb);
  EXPECT_EQ("I am doing a totem pole.", operator_resolveCommand("Do a totem pole", semMem, lingDb));

  // translation
  EXPECT_EQ("Bonjour",
            operator_resolveCommand("say hello in French", semMem, lingDb));
  // repetition
  EXPECT_EQ("Hello\tHello\tHello\tHello",
            operator_resolveCommand("say hello four times", semMem, lingDb));
  // repetition + translation
  EXPECT_EQ("Bonjour\tBonjour\tBonjour\tBonjour",
            operator_resolveCommand("say hello four times in French", semMem, lingDb));
  // repetition in list of commands
  EXPECT_EQ("(\tYou are a nice man.\tAND\tPoop!\tPoop!\t)",
            operator_resolveCommand("do goodbye and say poop 2 times", semMem, lingDb));
  EXPECT_EQ("(\tNice\tNice\tNice\tAND\tYou are a nice man.\t)",
            operator_resolveCommand("say nice 3 times and do goodbye", semMem, lingDb));

  // AND has a higher priority than THEN
  EXPECT_EQ("(\t(\t\"a1\"\tAND\t\"a2\"\t)\tTHEN\t(\t\"a3\"\tAND\t\"a4\"\t)\t)",
            operator_resolveCommand("Say \"a1\" and say \"a2\" then say \"a3\" and say \"a4\"", semMem, lingDb));
  EXPECT_EQ("(\t\"a1\"\tTHEN\t(\t\"a2\"\tAND\t\"a3\"\t)\tTHEN\t\"a4\"\t)",
            operator_resolveCommand("Say \"a1\" then say \"a2\" and say \"a3\" then say \"a4\"", semMem, lingDb));
  EXPECT_EQ("(\t(\t\"a1\"\tAND\t\"a2\"\t)\tTHEN\t(\t\"a3\"\tAND\t\"a4\"\t)\tTHEN\t(\t\"a5\"\tAND\t\"a6\"\t)\t)",
            operator_resolveCommand("Say \"a1\" and say \"a2\" then say \"a3\" and say \"a4\" then say \"a5\" and say \"a6\"", semMem, lingDb));
}


TEST_F(SemanticReasonerGTests, operator_resolveCommand_nonImperativeSentences)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;
  memoryOperation::learnSayCommand(semMem, lingDb);

  EXPECT_EQ("Hello",
            operator_resolveCommand("you have to say hello", semMem, lingDb));
  EXPECT_EQ("Hello",
            operator_resolveCommand("I want you to say hello", semMem, lingDb));
  EXPECT_EQ("",
            operator_resolveCommand("I want chocolate", semMem, lingDb));
}


TEST_F(SemanticReasonerGTests, operator_resolveCommand_say)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;
  memoryOperation::learnSayCommand(semMem, lingDb);

  EXPECT_EQ("Hello",
            operator_resolveCommand("say hello", semMem, lingDb));
  EXPECT_EQ("I am happy.",
            operator_resolveCommand("say I am happy", semMem, lingDb));
  EXPECT_EQ("You are happy.",
            operator_resolveCommand("say that I am happy", semMem, lingDb));
  EXPECT_EQ("You are cool.",
            operator_resolveCommand("say that I am cool", semMem, lingDb));
  EXPECT_EQ("I love you.",
            operator_resolveCommand("say I love you", semMem, lingDb));
  EXPECT_EQ("You love me.",
            operator_resolveCommand("say that I love you", semMem, lingDb));

  EXPECT_EQ("Bonjour",
            operator_resolveCommand("dis bonjour", semMem, lingDb));
  EXPECT_EQ("Bonjour",
            operator_resolveCommand("Vas-y dis bonjour", semMem, lingDb));
  EXPECT_EQ("Je suis content.",
            operator_resolveCommand("dis je suis content", semMem, lingDb));
  EXPECT_EQ("Je suis sympa.",
            operator_resolveCommand("dis je suis sympa", semMem, lingDb));
  EXPECT_EQ("Tu es content.",
            operator_resolveCommand("dis que je suis content", semMem, lingDb));
  EXPECT_EQ("Je t'aime.",
            operator_resolveCommand("dis que tu m'aimes", semMem, lingDb));
}


TEST_F(SemanticReasonerGTests, operator_resolveCommand_ask)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;
  memoryOperation::learnSayCommand(semMem, lingDb);

  EXPECT_EQ("Qu'aimes-tu ?",
            operator_resolveCommand("ask me in French what I like", semMem, lingDb));
}



TEST_F(SemanticReasonerGTests, operator_resolveCommand_startWithRobotName)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;
  memoryOperation::learnSayCommand(semMem, lingDb);

  EXPECT_EQ("", operator_resolveCommand("asimo say hello", semMem, lingDb));
  EXPECT_EQ("", operator_resolveCommand("asimo demande quel est mon nom", semMem, lingDb));
  operator_mergeAndInform("you are Asimo", semMem, lingDb);
  EXPECT_EQ("Hello", operator_resolveCommand("asimo say hello", semMem, lingDb));
  EXPECT_EQ("Hello", operator_resolveCommand("Asimo say hello", semMem, lingDb));
  EXPECT_EQ("", operator_resolveCommand("n5 say hello", semMem, lingDb));
  EXPECT_EQ("Quel est ton nom ?", operator_resolveCommand("asimo demande quel est mon nom", semMem, lingDb));
}



TEST_F(SemanticReasonerGTests, operator_resolveCommand_ofSubMemoryBlock)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;
  memoryOperation::learnSayCommand(semMem, lingDb);

  SemanticMemory semMem2;
  semMem2.memBloc.subBlockPtr = &semMem.memBloc;

  EXPECT_EQ("Hello", operator_resolveCommand("say hello", semMem, lingDb));
  EXPECT_EQ("Hello", operator_resolveCommand("say hello", semMem2, lingDb));
  // 2 levels of depth
  {
    SemanticMemory semMemTopLevel;
    semMemTopLevel.memBloc.subBlockPtr = &semMem2.memBloc;
    EXPECT_EQ("Hello", operator_resolveCommand("say hello", semMemTopLevel, lingDb));
  }
}
