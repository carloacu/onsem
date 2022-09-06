#include "operator_check.hpp"
#include <gtest/gtest.h>
#include "operator_inform.hpp"
#include <onsem/semantictotext/semanticmemory/semanticmemory.hpp>
#include <onsem/semantictotext/semanticconverter.hpp>
#include <onsem/semantictotext/semexpoperators.hpp>
#include "../../semanticreasonergtests.hpp"
#include "../../util/util.hpp"

using namespace onsem;

namespace onsem
{

TruenessValue operator_check(const std::string& pText,
                             const SemanticMemory& pSemanticMemory,
                             const linguistics::LinguisticDatabase& pLingDb,
                             const TextProcessingContext& pTextProcContext)
{
  auto semExp = converter::textToSemExp(pText, pTextProcContext, pLingDb);
  memoryOperation::resolveAgentAccordingToTheContext(semExp, pSemanticMemory, pLingDb);
  return memoryOperation::check(*semExp, pSemanticMemory.memBloc, pLingDb);
}


} // End of namespace onsem



TEST_F(SemanticReasonerGTests, operator_check_basic)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;
  ONSEM_TRUE(operator_check("true", semMem, lingDb));
  ONSEM_FALSE(operator_check("false", semMem, lingDb));
  ONSEM_UNKNOWN(operator_check("maybe", semMem, lingDb));
  ONSEM_TRUE(operator_check("1 is greater than 0", semMem, lingDb));
  ONSEM_FALSE(operator_check("0 is greater than 1", semMem, lingDb));
  ONSEM_FALSE(operator_check("0 is greater than 0", semMem, lingDb));
  ONSEM_TRUE(operator_check("0 is equal to 0", semMem, lingDb));
  ONSEM_TRUE(operator_check("1 equals 1", semMem, lingDb));
  ONSEM_FALSE(operator_check("1 equals 0", semMem, lingDb));
  ONSEM_FALSE(operator_check("toto equals \"toto\"", semMem, lingDb));

  ONSEM_UNKNOWN(operator_check("you like chocolate", semMem, lingDb));
  operator_inform("you like chocolate", semMem, lingDb);
  ONSEM_TRUE(operator_check("you like chocolate", semMem, lingDb));
  ONSEM_UNKNOWN(operator_check("what do you like?", semMem, lingDb));
  operator_inform("you don't like chocolate", semMem, lingDb);
  ONSEM_FALSE(operator_check("you like chocolate", semMem, lingDb));
  ONSEM_TRUE(operator_check("you dislike chocolate", semMem, lingDb));

  {
    operator_inform("si j'aime le chocolat, je suis gourmand", semMem, lingDb);
    ONSEM_UNKNOWN(operator_check("je suis gourmand", semMem, lingDb));
    operator_inform("j'aime le chocolat", semMem, lingDb);
    ONSEM_TRUE(operator_check("je suis gourmand", semMem, lingDb));
  }

  // check that the result is false if the object of the object is different
  {
    ONSEM_UNKNOWN(operator_check("Gustave aime manger du chocolat", semMem, lingDb));
    operator_inform("Gustave aime manger du chocolat", semMem, lingDb);
    ONSEM_TRUE(operator_check("Gustave aime manger du chocolat", semMem, lingDb));
    ONSEM_UNKNOWN(operator_check("Gustave aime manger des patates", semMem, lingDb));
  }

  // check with definite/indefinite pronouns
  {
    ONSEM_UNKNOWN(operator_check("Paul is on a table ?", semMem, lingDb));
    operator_inform("Paul is on the table", semMem, lingDb);
    ONSEM_TRUE(operator_check("Paul is on a table ?", semMem, lingDb));
  }

  // check on information merge with the context
  {
    SemanticMemory semMem2;
    ONSEM_UNKNOWN(operator_check("Paul is cool", semMem2, lingDb));
    operator_inform("Paul", semMem2, lingDb);
    operator_mergeAndInform("He is cool.", semMem2, lingDb);
    ONSEM_TRUE(operator_check("Paul is cool", semMem2, lingDb));
    ONSEM_UNKNOWN(operator_check("Pedro is cool", semMem2, lingDb));
    ONSEM_UNKNOWN(operator_check("Pierre is cool", semMem2, lingDb));
  }

  // check sub context difference
  {
    ONSEM_UNKNOWN(operator_check("I bought N5 robot", semMem, lingDb));
    operator_inform("I bought N5 robot", semMem, lingDb);
    ONSEM_TRUE(operator_check("I bought N5 robot", semMem, lingDb));
    ONSEM_TRUE(operator_check("I bought N5", semMem, lingDb));
    ONSEM_UNKNOWN(operator_check("I bought N7 robot", semMem, lingDb));
  }
}


TEST_F(SemanticReasonerGTests, operator_check_lists)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;
  operator_inform("you like chocolate", semMem, lingDb);
  operator_inform("you don't like beer", semMem, lingDb);

  // T && T => T
  ONSEM_TRUE(operator_check("you like chocolate and you don't like beer", semMem, lingDb));
  // T && F => F
  ONSEM_FALSE(operator_check("you like chocolate and you like beer", semMem, lingDb));
  // T && U => U
  ONSEM_UNKNOWN(operator_check("you like chocolate and you like water", semMem, lingDb));
  // F && T => F
  ONSEM_FALSE(operator_check("you don't like chocolate and you don't like beer", semMem, lingDb));
  // F && F => F
  ONSEM_FALSE(operator_check("you don't like chocolate and you like beer", semMem, lingDb));
  // F && U => F
  ONSEM_FALSE(operator_check("you don't like chocolate and you like water", semMem, lingDb));
  // U && T => U
  ONSEM_UNKNOWN(operator_check("you like water and you don't like beer", semMem, lingDb));
  // U && F => F
  ONSEM_FALSE(operator_check("you like water and you like beer", semMem, lingDb));
  // U && U => U
  ONSEM_UNKNOWN(operator_check("you like water and you like water", semMem, lingDb));
  // T && T && T => T
  ONSEM_TRUE(operator_check("you like chocolate, you don't like beer and you don't like beer", semMem, lingDb));
  // F && F && F => F
  ONSEM_FALSE(operator_check("you don't like chocolate, you like beer and you like beer", semMem, lingDb));
  // T && T && U => U
  ONSEM_UNKNOWN(operator_check("you like chocolate, you don't like beer and you like water", semMem, lingDb));
  // T && F && T => F
  ONSEM_FALSE(operator_check("you like chocolate, you like beer and you don't like beer", semMem, lingDb));
  // T && F && U => F
  ONSEM_FALSE(operator_check("you like chocolate, you like beer and you like water", semMem, lingDb));
  // T && U && F => F
  ONSEM_FALSE(operator_check("you like chocolate, you like water and you like beer", semMem, lingDb));
  // U && T && F => F
  ONSEM_FALSE(operator_check("you like water, you like chocolate and you like beer", semMem, lingDb));
  // U && F && T => F
  ONSEM_FALSE(operator_check("you like water, you like beer and you like chocolate", semMem, lingDb));


  // T || T => T
  ONSEM_TRUE(operator_check("you like chocolate or you don't like beer", semMem, lingDb));
  // T || F => T
  ONSEM_TRUE(operator_check("you like chocolate or you like beer", semMem, lingDb));
  // T || U => T
  ONSEM_TRUE(operator_check("you like chocolate or you like water", semMem, lingDb));
  // F || T => T
  ONSEM_TRUE(operator_check("you don't like chocolate or you don't like beer", semMem, lingDb));
  // F || F => F
  ONSEM_FALSE(operator_check("you don't like chocolate or you like beer", semMem, lingDb));
  // F || U => U
  ONSEM_UNKNOWN(operator_check("you don't like chocolate or you like water", semMem, lingDb));
  // U || T => T
  ONSEM_TRUE(operator_check("you like water or you don't like beer", semMem, lingDb));
  // U || F => U
  ONSEM_UNKNOWN(operator_check("you like water or you like beer", semMem, lingDb));
  // U || U => U
  ONSEM_UNKNOWN(operator_check("you like water or you like water", semMem, lingDb));
  // T || T || T => T
  ONSEM_TRUE(operator_check("you like chocolate, you don't like beer or you don't like beer", semMem, lingDb));
  // F || F || F => F
  ONSEM_FALSE(operator_check("you don't like chocolate, you like beer or you like beer", semMem, lingDb));
  // T || T || U => T
  ONSEM_TRUE(operator_check("you like chocolate, you don't like beer or you like water", semMem, lingDb));
  // T || F || T => T
  ONSEM_TRUE(operator_check("you like chocolate, you like beer or you don't like beer", semMem, lingDb));
  // T || F || U => T
  ONSEM_TRUE(operator_check("you like chocolate, you like beer or you like water", semMem, lingDb));
  // T || U || F => T
  ONSEM_TRUE(operator_check("you like chocolate, you like water or you like beer", semMem, lingDb));
  // U || T || F => T
  ONSEM_TRUE(operator_check("you like water, you like chocolate or you like beer", semMem, lingDb));
  // U || F || T => T
  ONSEM_TRUE(operator_check("you like water, you like beer or you like chocolate", semMem, lingDb));
}


TEST_F(SemanticReasonerGTests, operator_check_listsOfObjects)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;
  operator_inform("you like chocolate", semMem, lingDb);
  operator_inform("you like coffee", semMem, lingDb);
  operator_inform("you don't like beer", semMem, lingDb);

  // From positive verb
  // T && T => T
  ONSEM_TRUE(operator_check("you like chocolate and coffee", semMem, lingDb));
  // T && F => F
  ONSEM_FALSE(operator_check("you like chocolate and beer", semMem, lingDb));
  // T && U => U
  ONSEM_UNKNOWN(operator_check("you like chocolate and water", semMem, lingDb));


  // From negative verb
  // F && F => F
  ONSEM_FALSE(operator_check("you don't like chocolate and coffee", semMem, lingDb));
  // F && T => F
  ONSEM_FALSE(operator_check("you don't like chocolate and beer", semMem, lingDb));
  // T && T => T
  ONSEM_TRUE(operator_check("you don't like beer and beer", semMem, lingDb));
}


TEST_F(SemanticReasonerGTests, operator_check_listsOfVerbs)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;
  operator_inform("you like chocolate", semMem, lingDb);
  operator_inform("you like bread", semMem, lingDb);
  operator_inform("you like coffee", semMem, lingDb);
  operator_inform("you see chocolate", semMem, lingDb);
  operator_inform("you see milk", semMem, lingDb);
  operator_inform("you don't see bread", semMem, lingDb);

  // T & T => T
  ONSEM_TRUE(operator_check("you like chocolate and see chocolate", semMem, lingDb));
  // T & F => F
  ONSEM_FALSE(operator_check("you like bread and see bread", semMem, lingDb));
  // U & T => U
  ONSEM_UNKNOWN(operator_check("you like milk and see milk", semMem, lingDb));
}


TEST_F(SemanticReasonerGTests, operator_check_choice)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;
  const std::string question = "Do I like chocolate or banana?";
  ONSEM_UNKNOWN(operator_check(question, semMem, lingDb));
  operator_inform("I don't like chocolate", semMem, lingDb);
  ONSEM_UNKNOWN(operator_check(question, semMem, lingDb));
  operator_inform("I don't like banana", semMem, lingDb);
  ONSEM_FALSE(operator_check(question, semMem, lingDb));
  operator_inform("I like banana", semMem, lingDb);
  ONSEM_TRUE(operator_check(question, semMem, lingDb));
}



TEST_F(SemanticReasonerGTests, operator_check_numberOfElements)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;

  operator_mergeAndInform("You don't see me", semMem, lingDb);
  ONSEM_UNKNOWN(operator_check("Do you see a person?", semMem, lingDb));
  ONSEM_UNKNOWN(operator_check("Don't you see a person?", semMem, lingDb));
  operator_mergeAndInform("You see one person and you see me", semMem, lingDb);
  ONSEM_TRUE(operator_check("Do you see 1 person?", semMem, lingDb));
  ONSEM_FALSE(operator_check("Do you see 2 people?", semMem, lingDb));
  operator_mergeAndInform("You see me and you see one person", semMem, lingDb);
  ONSEM_TRUE(operator_check("Do you see 1 person?", semMem, lingDb));
  ONSEM_FALSE(operator_check("Do you see 2 people?", semMem, lingDb));
  operator_mergeAndInform("You see me and you see another person", semMem, lingDb);
  ONSEM_FALSE(operator_check("Do you see 1 person?", semMem, lingDb));
  ONSEM_TRUE(operator_check("Do you see 2 people?", semMem, lingDb));
  operator_mergeAndInform("You don't see me and you see one person", semMem, lingDb);
  ONSEM_TRUE(operator_check("Do you see 1 person?", semMem, lingDb));
  ONSEM_FALSE(operator_check("Do you see 2 people?", semMem, lingDb));
  operator_mergeAndInform("You don't see me", semMem, lingDb);
  ONSEM_TRUE(operator_check("Do you see 1 person?", semMem, lingDb));
  ONSEM_FALSE(operator_check("Do you see 2 people?", semMem, lingDb));
}
