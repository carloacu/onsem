#include "dummycommentaryprovider.hpp"
#include <onsem/texttosemantic/dbtype/semanticexpression/groundedexpression.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semantictextgrounding.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semantictimegrounding.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemory.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase.hpp>
#include <onsem/semantictotext/semanticconverter.hpp>

namespace onsem
{
std::string DummyCommentaryProvider::idStrOfProv = "DummyCommentaryProvider";


DummyCommentaryProvider::DummyCommentaryProvider(const linguistics::LinguisticDatabase& pLingDb)
  : MemBlockAndExternalCallback(),
    _semExpsThatCanBeAnswered()
{
  _semExpsThatCanBeAnswered.emplace_back
      (converter::textToSemExp
       ("the commentary of the book of \\p_time=0\\ is \\p_meta=-1\\",
        TextProcessingContext(SemanticAgentGrounding::userNotIdentified,
                              SemanticAgentGrounding::userNotIdentified,
                              SemanticLanguageEnum::ENGLISH),
        pLingDb));
}




mystd::unique_propagate_const<UniqueSemanticExpression> DummyCommentaryProvider::getAnswer
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
        const SemanticTimeGrounding* timeGrd = (*grdExp)->getTimeGroundingPtr();
        if (timeGrd != nullptr &&
            timeGrd->date.day == 1 && timeGrd->date.month == 1 && timeGrd->date.year == 2000)
        {
          if (pLanguageType == SemanticLanguageEnum::ENGLISH)
          {
            return mystd::unique_propagate_const<UniqueSemanticExpression>
                (mystd::make_unique<GroundedExpression>
                 (mystd::make_unique<SemanticTextGrounding>
                  (englishCommentary(), SemanticLanguageEnum::ENGLISH)));
          }
          else if (pLanguageType == SemanticLanguageEnum::FRENCH)
          {
            return mystd::unique_propagate_const<UniqueSemanticExpression>
                (mystd::make_unique<GroundedExpression>
                 (mystd::make_unique<SemanticTextGrounding>
                  (frenchCommentary(), SemanticLanguageEnum::FRENCH)));
          }
        }
      }
    }
  }
  return mystd::unique_propagate_const<UniqueSemanticExpression>();
}


std::string DummyCommentaryProvider::frenchCommentary()
{
  return "C’est un livre long mais intéressant!";
}


std::string DummyCommentaryProvider::englishCommentary()
{
  return "It's a long book but intersting!";
}


}
