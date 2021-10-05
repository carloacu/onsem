#include "../../semanticreasonergtests.hpp"
#include <gtest/gtest.h>
#include <onsem/texttosemantic/languagedetector.hpp>
#include <onsem/semantictotext/semanticconverter.hpp>
#include <onsem/semantictotext/semexpoperators.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemory.hpp>

using namespace onsem;

namespace
{
bool _operator_isASubPart(const std::string& pInputText,
                          const std::string& pTextToFind,
                          const SemanticMemory& pSemanticMemory,
                          const linguistics::LinguisticDatabase& pLingDb)
{
  SemanticLanguageEnum language = linguistics::getLanguage(pInputText, pLingDb);
  TextProcessingContext textProcToRobot(SemanticAgentGrounding::currentUser,
                                        SemanticAgentGrounding::me,
                                        language);
  auto inputSemExp = converter::textToSemExp(pInputText, textProcToRobot, pLingDb);
  auto semExpToMatch = converter::textToSemExp(pTextToFind, textProcToRobot, pLingDb);
  return memoryOperation::isASubpart(*inputSemExp, *semExpToMatch, pSemanticMemory, pLingDb);
}
}


TEST_F(SemanticReasonerGTests, operator_isASubpart_basic)
{
  SemanticMemory semanticMemory;
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  EXPECT_TRUE(_operator_isASubPart("", "", semanticMemory, lingDb));
  EXPECT_TRUE(_operator_isASubPart("aaa", "", semanticMemory, lingDb));
  EXPECT_FALSE(_operator_isASubPart("", "aaa", semanticMemory, lingDb));
  EXPECT_FALSE(_operator_isASubPart("bbb", "aaa", semanticMemory, lingDb));

  // english
  EXPECT_TRUE(_operator_isASubPart("hello", "", semanticMemory, lingDb));
  EXPECT_TRUE(_operator_isASubPart("hello", "hello", semanticMemory, lingDb));
  EXPECT_TRUE(_operator_isASubPart("hello", "Hello", semanticMemory, lingDb));
  EXPECT_TRUE(_operator_isASubPart("Hello", "hello", semanticMemory, lingDb));
  EXPECT_FALSE(_operator_isASubPart("hello", "banana", semanticMemory, lingDb));
  EXPECT_FALSE(_operator_isASubPart("hello", "man", semanticMemory, lingDb));
  EXPECT_TRUE(_operator_isASubPart("hello, I am Paul", "hello", semanticMemory, lingDb));
  EXPECT_FALSE(_operator_isASubPart("hello, I am Paul", "banana", semanticMemory, lingDb));
  EXPECT_TRUE(_operator_isASubPart("I like you. look left", "look left", semanticMemory, lingDb));
  EXPECT_TRUE(_operator_isASubPart("look left and say hello", "look left", semanticMemory, lingDb));
  EXPECT_TRUE(_operator_isASubPart("say hello and look left", "look left", semanticMemory, lingDb));
  EXPECT_TRUE(_operator_isASubPart("I look left", "I look", semanticMemory, lingDb));
  EXPECT_TRUE(_operator_isASubPart("true", "yes", semanticMemory, lingDb));
  EXPECT_TRUE(_operator_isASubPart("yes", "true", semanticMemory, lingDb));
  EXPECT_TRUE(_operator_isASubPart("no", "absolutely not", semanticMemory, lingDb));
  EXPECT_TRUE(_operator_isASubPart("speak english", "speak in English", semanticMemory, lingDb));
  EXPECT_FALSE(_operator_isASubPart("you are not a nice robot", "you are a nice robot", semanticMemory, lingDb));
  EXPECT_TRUE(_operator_isASubPart("you are not a nice robot", "you are not a nice robot", semanticMemory, lingDb));

  // french
  EXPECT_TRUE(_operator_isASubPart("Ça va", "ça va", semanticMemory, lingDb));
  EXPECT_TRUE(_operator_isASubPart("Je m'appelle David", "je m'appelle David affectation", semanticMemory, lingDb));
  EXPECT_TRUE(_operator_isASubPart("regarde à droite et dis bonjour", "regarde vers la droite", semanticMemory, lingDb));
  EXPECT_TRUE(_operator_isASubPart("regarde à droite et dis bonjour", "regarde à droite", semanticMemory, lingDb));
  EXPECT_TRUE(_operator_isASubPart("dis bonjour et regarde à droite", "regarde vers la droite", semanticMemory, lingDb));
  EXPECT_TRUE(_operator_isASubPart("dis bonjour et regarde à droite", "regarde à droite", semanticMemory, lingDb));
  EXPECT_TRUE(_operator_isASubPart("Oui, Paul est content", "Paul est content", semanticMemory, lingDb));
  EXPECT_TRUE(_operator_isASubPart("Paul ira à Paris en janvier", "Paul ira à Paris", semanticMemory, lingDb));
  EXPECT_FALSE(_operator_isASubPart("Paul est content ?", "Paul est content", semanticMemory, lingDb));
  EXPECT_TRUE(_operator_isASubPart("Paul est content !", "Paul est content", semanticMemory, lingDb));
  EXPECT_FALSE(_operator_isASubPart("Paul était content", "Paul est content", semanticMemory, lingDb));
  EXPECT_TRUE(_operator_isASubPart("qu'est-ce que tu sais faire", "que sais - tu faire", semanticMemory, lingDb));
}
