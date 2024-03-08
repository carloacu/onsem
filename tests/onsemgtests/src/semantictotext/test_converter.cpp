#include "../semanticreasonergtests.hpp"
#include <gtest/gtest.h>
#include <onsem/semantictotext/semanticmemory/semanticmemory.hpp>
#include <onsem/semantictotext/semanticconverter.hpp>


using namespace onsem;

namespace {

std::string _geFutureFrom(const std::string& pInputText,
                          SemanticLanguageEnum pLanguage,
                          const SemanticMemory& pSemMem,
                          const linguistics::LinguisticDatabase& pLingDb)
{
  auto semExpFuture = textToSemExp(pInputText, pLingDb, pLanguage);
  auto semExpFutureForm = converter::getFutureIndicativeAssociatedForm(std::move(semExpFuture));
  return semExpToText(std::move(semExpFutureForm), pLanguage, pSemMem, pLingDb);
}

std::string _getWayToAskForIt(const std::string& pInputText,
                              SemanticLanguageEnum pLanguage,
                              const SemanticMemory& pSemMem,
                              const linguistics::LinguisticDatabase& pLingDb)
{
  auto semExpInfinitive = textToSemExp(pInputText, pLingDb, pLanguage);
  UniqueSemanticExpression imperativeSemExp;
  UniqueSemanticExpression wouldLikeSemExp;
  converter::getInfinitiveToTwoDifferentPossibleWayToAskForIt(
        imperativeSemExp, wouldLikeSemExp, std::move(semExpInfinitive));
  return semExpToTextFromUser(std::move(imperativeSemExp), pLanguage, pSemMem, pLingDb) + " | " +
      semExpToTextFromUser(std::move(wouldLikeSemExp), pLanguage, pSemMem, pLingDb);
}



}


TEST_F(SemanticReasonerGTests, operator_converter_getFutureIndicativeAssociatedForm)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;

  EXPECT_EQ("Je marcherai.", _geFutureFrom("Marcher", SemanticLanguageEnum::FRENCH, semMem, lingDb));
  EXPECT_EQ("Je marcherai.", _geFutureFrom("Il faut marcher", SemanticLanguageEnum::FRENCH, semMem, lingDb));
  EXPECT_EQ("Je marcherai et puis je sauterai.", _geFutureFrom("Il faut marcher puis il faut sauter", SemanticLanguageEnum::FRENCH, semMem, lingDb));
}




TEST_F(SemanticReasonerGTests, operator_converter_getWayToAskForIt)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;

  EXPECT_EQ("Marche ! | Je voudrais que tu marches.",
            _getWayToAskForIt("Marcher", SemanticLanguageEnum::FRENCH, semMem, lingDb));

  EXPECT_EQ("Avance rapidement vers la droite ! | Je voudrais que tu avances rapidement vers la droite.",
            _getWayToAskForIt("Avancer rapidement vers la droite", SemanticLanguageEnum::FRENCH, semMem, lingDb));


  EXPECT_EQ("Grab the bottle! | I would like you are grabbing the bottle.",
            _getWayToAskForIt("To grab the bottle", SemanticLanguageEnum::ENGLISH, semMem, lingDb));
}

