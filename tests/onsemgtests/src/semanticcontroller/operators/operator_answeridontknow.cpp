#include "../../semanticreasonergtests.hpp"
#include <onsem/common/utility/noresult.hpp>
#include <onsem/texttosemantic/tool/semexpgetter.hpp>
#include <onsem/texttosemantic/languagedetector.hpp>
#include <onsem/semantictotext/semanticconverter.hpp>
#include <onsem/semantictotext/semexpoperators.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemory.hpp>


namespace onsem
{
namespace
{

std::string _operator_answerIDontKnow(const std::string& pText,
                                      bool pForQuestion,
                                      const linguistics::LinguisticDatabase& lingDb)
{
  SemanticLanguageEnum language = linguistics::getLanguage(pText, lingDb);
  auto semExp =
      converter::textToSemExp(pText,
                              TextProcessingContext(SemanticAgentGrounding::currentUser,
                                                    SemanticAgentGrounding::me,
                                                    language),
                              lingDb);
  auto answerSemExp = pForQuestion ? memoryOperation::answerIDontKnow(*semExp) :
                                     memoryOperation::answerICannotDo(*semExp);
  if (answerSemExp)
  {
    std::string res;
    SemanticMemory semMem;
    converter::semExpToText(res, std::move(*answerSemExp),
                            TextProcessingContext(SemanticAgentGrounding::me,
                                                  SemanticAgentGrounding::currentUser,
                                                  language),
                            false, semMem, lingDb, nullptr);
    return res;
  }
  return constant::noResult;
}

std::string operator_answerIDontKnow(const std::string& pText,
                                                 const linguistics::LinguisticDatabase& lingDb)
{
  return _operator_answerIDontKnow(pText, true, lingDb);
}

std::string operator_answerICannotDo(const std::string& pText,
                                                const linguistics::LinguisticDatabase& lingDb)
{
  return _operator_answerIDontKnow(pText, false, lingDb);
}

}
} // End of namespace onsem


using namespace onsem;


TEST_F(SemanticReasonerGTests, operator_answerIDontKnow_basic)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;

  // english
  EXPECT_EQ(constant::noResult, operator_answerIDontKnow("You are on the floor", lingDb));
  EXPECT_EQ(constant::noResult, operator_answerIDontKnow("look left", lingDb));
  EXPECT_EQ("I don't know if I am on floor.", operator_answerIDontKnow("Are you on the floor?", lingDb));
  EXPECT_EQ("I don't know if I like chocolate.", operator_answerIDontKnow("I like chocolate. Do you like chocolate?", lingDb));
  EXPECT_EQ("I don't know what I like.", operator_answerIDontKnow("What do you like? Me, I like chocolate.", lingDb));
  EXPECT_EQ("I don't know if I am happy. I don't know why I am shy.", operator_answerIDontKnow("Are you happy? Why are you shy?", lingDb));
  EXPECT_EQ("I don't know if I want to see a video that describes Carrefour.", operator_answerIDontKnow("Do you want to see a video that describes Carrefour", lingDb));
  EXPECT_EQ("I don't know what you like.", operator_answerIDontKnow("What do I like?", lingDb));
  EXPECT_EQ("I don't know who plays football.", operator_answerIDontKnow("Who plays football", lingDb));


  // french
  EXPECT_EQ(constant::noResult, operator_answerIDontKnow("Je suis content", lingDb));
  EXPECT_EQ(constant::noResult, operator_answerIDontKnow("regarde à droite", lingDb));
  EXPECT_EQ("Je ne sais pas qui est Paul.", operator_answerIDontKnow("Qui est Paul ?", lingDb));
  EXPECT_EQ("Je ne sais pas ce que j'en pense.", operator_answerIDontKnow("Je ne sais pas. Qu'est-ce que tu en penses ?", lingDb));
  EXPECT_EQ("Je ne sais pas comment je vais.", operator_answerIDontKnow("Comment ça va ? Moi ça va bien.", lingDb));
  EXPECT_EQ("Je ne sais pas comment Paul s'est senti.", operator_answerIDontKnow("Comment Paul s’est-il senti ?", lingDb));
  EXPECT_EQ("Je ne sais pas si je vais au Japon.", operator_answerIDontKnow("Est-ce que tu vas au Japon ?", lingDb));
  EXPECT_EQ("Je ne sais pas si je veux voir une vidéo qui décrit Carrefour.", operator_answerIDontKnow("Veux tu voir une vidéo qui décrit Carrefour", lingDb));
  EXPECT_EQ("Je ne sais pas qui est le président.", operator_answerIDontKnow("c’est qui le président ?", lingDb));
  EXPECT_EQ("Je ne sais pas ce que c'est une tomate.", operator_answerIDontKnow("C’est quoi une tomate ?", lingDb));
  EXPECT_EQ("Je ne sais pas quelle est la différence entre un bébé et un crocodile.", operator_answerIDontKnow("Quelle est la différence entre un bébé et un crocodile?", lingDb));
  EXPECT_EQ("Je ne sais pas quand et où Anne est née.", operator_answerIDontKnow("Quand et où est née Anne", lingDb));
  EXPECT_EQ("Je ne sais pas où est mon père.", operator_answerIDontKnow("Il est où ton père", lingDb));
}


TEST_F(SemanticReasonerGTests, operator_answerICannotDo_basic)
{
  const linguistics::LinguisticDatabase& lingDb = *lingDbPtr;

  // english
  EXPECT_EQ(constant::noResult, operator_answerICannotDo("You are on the floor", lingDb));
  EXPECT_EQ(constant::noResult, operator_answerICannotDo("Are you are on the floor?", lingDb));
  EXPECT_EQ("I can't look left.", operator_answerICannotDo("look left", lingDb));
  EXPECT_EQ("I can't look left. I can't salute.", operator_answerICannotDo("look left and salute", lingDb));
  EXPECT_EQ("I can't look left. I can't say hello.", operator_answerICannotDo("look left or say hello", lingDb));
  EXPECT_EQ("I can't show you a video that describes N5.", operator_answerICannotDo("Show me a video that describes N5", lingDb));
  EXPECT_EQ("I can't jump.", operator_answerICannotDo("Jump", lingDb));

  // french
  EXPECT_EQ(constant::noResult, operator_answerICannotDo("Je suis content", lingDb));
  EXPECT_EQ(constant::noResult, operator_answerICannotDo("Qui est Paul ?", lingDb));
  EXPECT_EQ("Je ne sais pas regarder à droite.", operator_answerICannotDo("regarde à droite", lingDb));
  EXPECT_EQ("Je ne sais pas te montrer une vidéo qui décrit N5.", operator_answerICannotDo("Montre moi une vidéo qui décrit N5", lingDb));
}
