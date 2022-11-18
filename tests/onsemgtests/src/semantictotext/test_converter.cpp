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

}


TEST_F(SemanticReasonerGTests, operator_converter_getFutureIndicativeAssociatedForm)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;
  SemanticMemory semMem;

  EXPECT_EQ("Je marcherai.", _geFutureFrom("Marcher", SemanticLanguageEnum::FRENCH, semMem, lingDb));
  EXPECT_EQ("Je marcherai.", _geFutureFrom("Il faut marcher", SemanticLanguageEnum::FRENCH, semMem, lingDb));
  EXPECT_EQ("Je marcherai et puis je sauterai.", _geFutureFrom("Il faut marcher puis il faut sauter", SemanticLanguageEnum::FRENCH, semMem, lingDb));
}

