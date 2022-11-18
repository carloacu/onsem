#include <gtest/gtest.h>
#include <onsem/semantictotext/semanticintent.hpp>
#include "../semanticreasonergtests.hpp"
#include "../util/util.hpp"

using namespace onsem;

namespace
{


std::string _listOfIntentsToStr(const std::list<SemanticIntent>& pIntents)
{
  std::string res;

  bool firstIntent = true;
  for (const auto& currIntent : pIntents)
  {
    if (firstIntent)
      firstIntent = false;
    else
      res += "\n";
    res += currIntent.name();
    if (!currIntent.entities().empty())
    {
      res += "{";
      bool firstEntity = true;
      for (const auto& currEntity : currIntent.entities())
      {
        if (firstEntity)
          firstEntity = false;
        else
          res += ", ";
        res += "(" + currEntity.first + "->" + currEntity.second + ")";
      }
      res += "}";
    }
  }
  return res;
}


std::string _textToIntentsStr(const std::string& pText,
                              const linguistics::LinguisticDatabase& pLingDb,
                              SemanticLanguageEnum pLanguage)
{
  auto semExp = textToSemExp(pText, pLingDb, pLanguage);
  std::list<SemanticIntent> intents;
  extractIntents(intents, *semExp);
  return _listOfIntentsToStr(intents);
}

}



TEST_F(SemanticReasonerGTests, test_textIntents)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;

  // english
  SemanticLanguageEnum enLanguage = SemanticLanguageEnum::ENGLISH;
  EXPECT_EQ("", _textToIntentsStr("I am happy", lingDb, enLanguage));
  EXPECT_EQ("Farewell{(type->goodbye)}", _textToIntentsStr("Goodbye", lingDb, enLanguage));
  EXPECT_EQ("Farewell{(type->needToGo)}", _textToIntentsStr("I need to go", lingDb, enLanguage));

  // french
  SemanticLanguageEnum frLanguage = SemanticLanguageEnum::FRENCH;
  EXPECT_EQ("", _textToIntentsStr("Je suis content", lingDb, frLanguage));
  EXPECT_EQ("Greetings", _textToIntentsStr("Bonjour", lingDb, frLanguage));
  EXPECT_EQ("Farewell{(type->goodbye)}", _textToIntentsStr("Au revoir", lingDb, frLanguage));
  EXPECT_EQ("Farewell{(type->needToGo)}", _textToIntentsStr("Je dois y aller", lingDb, frLanguage));
}

