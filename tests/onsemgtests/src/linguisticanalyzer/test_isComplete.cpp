#include "../semanticreasonergtests.hpp"
#include <onsem/texttosemantic/dbtype/linguisticdatabase.hpp>
#include <onsem/texttosemantic/tool/iscomplete.hpp>
using namespace onsem;


TEST_F(SemanticReasonerGTests, isComplete)
{
  linguistics::LinguisticDatabase& lingdb = *lingDbPtr;
  SemanticLanguageEnum enLang = SemanticLanguageEnum::ENGLISH;

  EXPECT_FALSE(linguistics::isComplete("when you are happy", lingdb, enLang));
  EXPECT_TRUE(linguistics::isComplete("when are you happy", lingdb, enLang));

  EXPECT_FALSE(linguistics::isComplete("if you are happy", lingdb, enLang));
  EXPECT_TRUE(linguistics::isComplete("if you are happy say I am happy", lingdb, enLang));

  EXPECT_TRUE(linguistics::isComplete("I think", lingdb, enLang));
  EXPECT_FALSE(linguistics::isComplete("I think for", lingdb, enLang));
  EXPECT_TRUE(linguistics::isComplete("I think for that", lingdb, enLang));
  EXPECT_FALSE(linguistics::isComplete("I went to the", lingdb, enLang));

  EXPECT_FALSE(linguistics::isComplete("raise your right hand, say hello", lingdb, enLang));
  EXPECT_TRUE(linguistics::isComplete("raise your right hand say hello and look right", lingdb, enLang));

  EXPECT_TRUE(linguistics::isComplete("a boat", lingdb, enLang));
  EXPECT_FALSE(linguistics::isComplete("a boat a sail", lingdb, enLang));
  EXPECT_FALSE(linguistics::isComplete("a boat, a sail", lingdb, enLang));
  EXPECT_FALSE(linguistics::isComplete("a boat a sail and", lingdb, enLang));
  EXPECT_FALSE(linguistics::isComplete("a boat, a sail and", lingdb, enLang));
  EXPECT_TRUE(linguistics::isComplete("a boat, a sail and a sailor", lingdb, enLang));



  SemanticLanguageEnum frLang = SemanticLanguageEnum::FRENCH;

  EXPECT_FALSE(linguistics::isComplete("quand tu es content", lingdb, frLang));
  EXPECT_TRUE(linguistics::isComplete("quand es tu content", lingdb, frLang));

  EXPECT_FALSE(linguistics::isComplete("si tu es content", lingdb, frLang));
  EXPECT_TRUE(linguistics::isComplete("si tu es content dis je suis content", lingdb, frLang));

  EXPECT_TRUE(linguistics::isComplete("je pense", lingdb, frLang));
  EXPECT_FALSE(linguistics::isComplete("je pense à", lingdb, frLang));
  EXPECT_TRUE(linguistics::isComplete("je pense à toi", lingdb, frLang));

  EXPECT_FALSE(linguistics::isComplete("lève la main droite dis bonjour", lingdb, frLang));
  EXPECT_FALSE(linguistics::isComplete("lève la main droite, dis bonjour", lingdb, frLang));
  EXPECT_TRUE(linguistics::isComplete("lève la main droite dis bonjour et regarde à droite", lingdb, frLang));

  EXPECT_TRUE(linguistics::isComplete("un bateau", lingdb, frLang));
  EXPECT_FALSE(linguistics::isComplete("un bateau une voile", lingdb, frLang));
  EXPECT_FALSE(linguistics::isComplete("un bateau, une voile", lingdb, frLang));
  EXPECT_FALSE(linguistics::isComplete("un bateau une voile et", lingdb, frLang));
  EXPECT_FALSE(linguistics::isComplete("un bateau, une voile et", lingdb, frLang));
  EXPECT_TRUE(linguistics::isComplete("un bateau une voile et un matelot", lingdb, frLang));
}
