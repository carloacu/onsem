#ifndef ONSEM_TEXTTOSEMANTIC_TOOL_PARTOFSPEECH_PARTOFSPEECHCUSTOMFILTER_HPP
#define ONSEM_TEXTTOSEMANTIC_TOOL_PARTOFSPEECH_PARTOFSPEECHCUSTOMFILTER_HPP

#include <map>
#include <list>
#include <vector>
#include <onsem/common/enum/partofspeech.hpp>
#include <onsem/common/enum/lingenumlinkedmeaningdirection.hpp>
#include <onsem/texttosemantic/dbtype/linguisticmeaning.hpp>
#include <onsem/texttosemantic/dbtype/inflectedword.hpp>
#include <onsem/texttosemantic/tool/partofspeech/partofspeechcontextfilter.hpp>
#include "../../api.hpp"

namespace onsem
{
struct SemanticWord;
namespace linguistics
{
struct SpecificLinguisticDatabase;
struct InflectedWord;
class InflectionsChecker;
using TokIt = std::vector<Token>::iterator;


class ONSEM_TEXTTOSEMANTIC_API PartOfSpeechCustomFilter : public PartOfSpeechContextFilter
{
public:
  PartOfSpeechCustomFilter(const std::string& pName,
                           const InflectionsChecker& pFls,
                           const SpecificLinguisticDatabase& pSpecLingDb,
                           const std::string& pLabel);

  bool process(std::vector<Token>& pTokens) const override;


private:
  const InflectionsChecker& fFls;
  const SpecificLinguisticDatabase& fSpecLingDb;
  const std::string fLabel;
  std::vector<PartOfSpeech> fGramTypesWeCanSkipBefore;

  bool xTryToLinkAToken
  (std::list<std::pair<TokIt, std::list<InflectedWord>::iterator> >& pLinkedTokens,
   std::vector<Token>& pTokens,
   std::vector<Token>::iterator pRootTok,
   std::list<InflectedWord>::iterator pRootItIGram,
   const onsem::linguistics::LingWordsGroup& pMeanGroup) const;

  std::vector<Token>::iterator xSearchWordAfter
  (std::vector<Token>& pTokens,
   std::list<InflectedWord>::iterator& pOnlyItIGramToKeep,
   std::vector<Token>::iterator pCurrTok,
   const SemanticWord& pWord,
   const InflectedWord& pMainIGram) const;

  std::vector<Token>::iterator xSearchWordBefore
  (std::vector<Token>& pTokens,
   std::list<InflectedWord>::iterator& pOnlyItIGramToKeep,
   std::vector<Token>::iterator pCurrTok,
   const SemanticWord& pWord,
   const InflectedWord& pMainIGram) const;

  bool xCanBeLinked
  (const InflectedWord& pMainIGram,
   const InflectedWord& pSubIGram) const;


  void xTestOfPuttingNounAtBottomOfListIfOccurTwoTimesInARow
  (std::vector<Token>& pTokens,
   bool pExceptForVerb) const;

  void xPutNounAtBottomIfNecessary
  (std::list<InflectedWord>& pIGram1,
   std::list<InflectedWord>& pIGram2,
   bool pExceptForVerb) const;
};


} // End of namespace linguistics
} // End of namespace onsem


#endif // ONSEM_TEXTTOSEMANTIC_TOOL_PARTOFSPEECH_PARTOFSPEECHCUSTOMFILTER_HPP
