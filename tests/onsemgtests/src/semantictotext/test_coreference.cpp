#include "../semanticreasonergtests.hpp"
#include <onsem/semantictotext/semanticconverter.hpp>
#include <onsem/semantictotext/semexpoperators.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemory.hpp>
#include <onsem/tester/reactOnTexts.hpp>

using namespace onsem;


namespace
{
TruenessValue _reactWitthNewMemoryAndWithCustomContext(const std::string& pInforamtion,
                                                       const std::string& pQuestion,
                                                       const TextProcessingContext& pContext,
                                                       const linguistics::LinguisticDatabase& pLingDb)
{
  SemanticMemory semMem;
  auto infoSemExp = converter::textToSemExp(pInforamtion, pContext, pLingDb);
  operator_react_fromSemExp(std::move(infoSemExp), semMem, pLingDb, pContext.langType);
  auto questionSemExp = converter::textToSemExp(pQuestion, pContext, pLingDb);
  return memoryOperation::check(std::move(*questionSemExp), semMem.memBloc, pLingDb);
}
}


TEST_F(SemanticReasonerGTests, test_coreference_us)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;

  TextProcessingContext contextEn(SemanticAgentGrounding::currentUser,
                                  SemanticAgentGrounding::me,
                                  SemanticLanguageEnum::ENGLISH);
  TextProcessingContext contextFr(SemanticAgentGrounding::currentUser,
                                  SemanticAgentGrounding::me,
                                  SemanticLanguageEnum::FRENCH);
  std::string isEverybodyHappyEn = "Is everybody happy?";
  std::string isEverybodyHappyFr = "Est-ce que tout le monde est content ?";

  contextEn.setUsAsYouAndMe();
  contextFr.setUsAsYouAndMe();

  ONSEM_UNKNOWN(_reactWitthNewMemoryAndWithCustomContext("we are happy", isEverybodyHappyEn, contextEn, lingDb));
  ONSEM_UNKNOWN(_reactWitthNewMemoryAndWithCustomContext("nous sommes content", isEverybodyHappyFr, contextFr, lingDb));
  ONSEM_UNKNOWN(_reactWitthNewMemoryAndWithCustomContext("on est content", isEverybodyHappyFr, contextFr, lingDb));

  contextEn.setUsAsEverybody();
  contextFr.setUsAsEverybody();

  ONSEM_TRUE(_reactWitthNewMemoryAndWithCustomContext("we are happy", isEverybodyHappyEn, contextEn, lingDb));
  ONSEM_TRUE(_reactWitthNewMemoryAndWithCustomContext("nous sommes content", isEverybodyHappyFr, contextFr, lingDb));
  ONSEM_TRUE(_reactWitthNewMemoryAndWithCustomContext("on est content", isEverybodyHappyFr, contextFr, lingDb));
}

