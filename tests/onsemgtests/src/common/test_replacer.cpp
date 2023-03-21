#include <gtest/gtest.h>
#include <onsem/common/utility/string.hpp>
#include "../semanticreasonergtests.hpp"


using namespace onsem;



TEST(StringUtil, replacer)
{
  mystd::Replacer replacerWithSeparators(true, true);
  mystd::Replacer replacerWithoutSeparators(true, false);

  replacerWithSeparators.addReplacementPattern("^a", "salut");
  replacerWithoutSeparators.addReplacementPattern("^a", "salut");
  replacerWithSeparators.addReplacementPattern("toto", "titi");
  replacerWithoutSeparators.addReplacementPattern("toto", "titi");
  replacerWithSeparators.addReplacementPattern("^comment tu vas ?$", "ça va ?");
  replacerWithoutSeparators.addReplacementPattern("^comment tu vas ?$", "ça va ?");

  EXPECT_EQ("titi", replacerWithSeparators.doReplacements("toto"));
  EXPECT_EQ("titi", replacerWithoutSeparators.doReplacements("toto"));
  EXPECT_EQ("atoto", replacerWithSeparators.doReplacements("atoto"));
  EXPECT_EQ("saluttiti", replacerWithoutSeparators.doReplacements("atoto"));
  EXPECT_EQ("totob", replacerWithSeparators.doReplacements("totob"));
  EXPECT_EQ("titib", replacerWithoutSeparators.doReplacements("totob"));
  EXPECT_EQ("titi b", replacerWithSeparators.doReplacements("toto b"));
  EXPECT_EQ("titi b", replacerWithoutSeparators.doReplacements("toto b"));

  EXPECT_EQ("ça va ?", replacerWithSeparators.doReplacements("comment tu vas ?"));
  EXPECT_EQ("ça va ?", replacerWithoutSeparators.doReplacements("comment tu vas ?"));
  EXPECT_EQ(" comment tu vas ?", replacerWithSeparators.doReplacements(" comment tu vas ?"));
  EXPECT_EQ(" comment tu vas ?", replacerWithoutSeparators.doReplacements(" comment tu vas ?"));
  EXPECT_EQ("comment tu vas ? ", replacerWithSeparators.doReplacements("comment tu vas ? "));
  EXPECT_EQ("comment tu vas ? ", replacerWithoutSeparators.doReplacements("comment tu vas ? "));

  EXPECT_EQ("abc", replacerWithSeparators.doReplacements("abc"));
  EXPECT_EQ("salutbc", replacerWithoutSeparators.doReplacements("abc"));

  EXPECT_EQ("salut bc", replacerWithSeparators.doReplacements("a bc"));
  EXPECT_EQ("salut bc", replacerWithoutSeparators.doReplacements("a bc"));


  mystd::Replacer replacerWithSeparatorsNotCaseSensitive(false, true);
  replacerWithSeparatorsNotCaseSensitive.addReplacementPattern("^a", "salut");
  replacerWithSeparatorsNotCaseSensitive.addReplacementPattern("Toto", "titi");
  replacerWithSeparatorsNotCaseSensitive.addReplacementPattern("^comment tu vas ?$", "ça va ?");

  EXPECT_EQ("A bc", replacerWithSeparators.doReplacements("A bc"));
  EXPECT_EQ("salut bc", replacerWithSeparatorsNotCaseSensitive.doReplacements("A bc"));
  EXPECT_EQ("titi", replacerWithSeparatorsNotCaseSensitive.doReplacements("toto"));
  EXPECT_EQ("titi", replacerWithSeparatorsNotCaseSensitive.doReplacements("Toto"));
  EXPECT_EQ("Toto", replacerWithSeparators.doReplacements("Toto"));
}

