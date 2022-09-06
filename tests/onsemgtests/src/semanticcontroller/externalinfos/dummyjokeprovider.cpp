#include "dummyjokeprovider.hpp"
#include <onsem/texttosemantic/dbtype/semanticexpression/groundedexpression.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semantictextgrounding.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semantictimegrounding.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticagentgrounding.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase.hpp>
#include <onsem/semantictotext/semanticconverter.hpp>

namespace onsem
{
std::string DummyJokeProvider::idStrOfProv = "DummyJokeProvider";


DummyJokeProvider::DummyJokeProvider(const linguistics::LinguisticDatabase& pLingDb)
  : MemBlockAndExternalCallback(),
    _semExpsThatCanBeAnswered()
{
  _semExpsThatCanBeAnswered.emplace_back
      (converter::textToSemExp
       ("The joke of \\p_time=0\\ is \\p_meta=-1\\",
        TextProcessingContext(SemanticAgentGrounding::userNotIdentified,
                              SemanticAgentGrounding::userNotIdentified,
                              SemanticLanguageEnum::ENGLISH),
        pLingDb));
  _trigger.emplace_back
      (converter::textToSemExp
       ("I want to read the joke of \\p_time=0\\",
        TextProcessingContext(SemanticAgentGrounding::currentUser,
                              SemanticAgentGrounding::userNotIdentified,
                              SemanticLanguageEnum::ENGLISH),
        pLingDb));
}




mystd::unique_propagate_const<UniqueSemanticExpression> DummyJokeProvider::getAnswer
(const IndexToSubNameToParameterValue& pParams,
 SemanticLanguageEnum pLanguageType) const
{
  auto itParam = pParams.find(0);
  if (itParam != pParams.end())
  {
    auto itAttrName = itParam->second.find("");
    if (itAttrName != itParam->second.end())
    {
      const GroundedExpression* grdExp = itAttrName->second->getSemExp().getGrdExpPtr_SkipWrapperPtrs();
      if (grdExp != nullptr)
      {
        // filter only for the 1 january 2000
        const SemanticTimeGrounding* timeGrd = (*grdExp)->getTimeGroundingPtr();
        if (timeGrd != nullptr &&
            timeGrd->date.day == 1 && timeGrd->date.month == 1 && timeGrd->date.year == 2000)
        {
          if (pLanguageType == SemanticLanguageEnum::ENGLISH)
          {
            return mystd::unique_propagate_const<UniqueSemanticExpression>
                (mystd::make_unique<GroundedExpression>
                 (mystd::make_unique<SemanticTextGrounding>
                  (englishJoke(), SemanticLanguageEnum::ENGLISH)));
          }
          else if (pLanguageType == SemanticLanguageEnum::FRENCH)
          {
            return mystd::unique_propagate_const<UniqueSemanticExpression>
                (mystd::make_unique<GroundedExpression>
                 (mystd::make_unique<SemanticTextGrounding>
                  (frenchJoke(), SemanticLanguageEnum::FRENCH)));
          }
        }
      }
    }
  }
  return mystd::unique_propagate_const<UniqueSemanticExpression>();
}


std::string DummyJokeProvider::frenchJoke()
{
  return "C’est une conversation entre une maîtresse d’école et son élève :\n"
         "L’enfant :\n"
         "– Madame, madame, est-ce que je peux être puni pour quelque chose que je n’ai pas fait ?\n"
         "La maîtresse :\n"
         "– Mais bien sur que non, on ne va pas te punir pour quelque chose que tu n’as pas fait!!!\n"
         "L’enfant :\n"
         "– Eh bien, ça va alors… je n’ai pas fait mes devoirs hier !";
}


std::string DummyJokeProvider::englishJoke()
{
  return "A man goes to a brothel and says, \" I have £40 will you humiliate me please.\""
         "The Madam replies, \"Here put on this England shirt!\"";
}



}
