#ifndef ONSEM_TEXTTOSEMANTIC_SRC_TOOL_PARTOFSPEECH_PARTOFSPEECHDELBIGRAMIMPOSSIBILITIES_HPP
#define ONSEM_TEXTTOSEMANTIC_SRC_TOOL_PARTOFSPEECH_PARTOFSPEECHDELBIGRAMIMPOSSIBILITIES_HPP

#include <map>
#include <vector>
#include <list>
#include <memory>
#include <onsem/common/enum/partofspeech.hpp>
#include <onsem/texttosemantic/tool/partofspeech/partofspeechcontextfilter.hpp>


namespace onsem
{
namespace linguistics
{
class InflectionsChecker;
struct InflectedWord;
using TokIt = std::vector<Token>::iterator;


class PartOfSpeechDelBigramImpossibilities : public PartOfSpeechContextFilter
{
public:
  PartOfSpeechDelBigramImpossibilities
  (const std::string& pName,
   const InflectionsChecker& pFls,
   const std::vector<std::pair<PartOfSpeech, PartOfSpeech> >& pImpSuccessions,
   const std::vector<std::pair<PartOfSpeech, PartOfSpeech> >& pCheckCompatibility);

  PartOfSpeechDelBigramImpossibilities
  (const InflectionsChecker& pFls,
   unsigned char const** pChar);

  bool process(std::vector<Token>& pTokens) const override;


private:
  struct GramCompatibility
  {
    GramCompatibility
    (PartOfSpeech pPartOfSpeech,
     bool pCheckCompabiblity)
      : partOfSpeech(pPartOfSpeech),
        checkCompabiblity(pCheckCompabiblity)
    {
    }

    PartOfSpeech partOfSpeech;
    bool checkCompabiblity;
  };
  const InflectionsChecker& fFls;
  std::vector<PartOfSpeech> fCantBeAtTheBeginning;
  std::vector<PartOfSpeech> fCheckCompatibilityAtTheBeginning;
  std::map<PartOfSpeech, std::vector<GramCompatibility> > fPrevSuccs;
  std::map<PartOfSpeech, std::vector<GramCompatibility> > fNextSuccs;
  std::vector<PartOfSpeech> fCantBeAtTheEnding;
  std::vector<PartOfSpeech> fCheckCompatibilityAtTheEnding;

  void xInit
  (const std::vector<std::pair<PartOfSpeech, PartOfSpeech> >& pImpSuccessions,
   const std::vector<std::pair<PartOfSpeech, PartOfSpeech> >& pCheckCompatibility);


  bool xDelBigramImpossibilitiesForAToken
  (TokIt& pTokenItToClean,
   TokIt& pTokenItNeighbor,
   const std::map<PartOfSpeech, std::vector<GramCompatibility> >& pImpSuccs,
   bool pTokToCleanBeforeTokNeighbor) const;

};

} // End of namespace linguistics
} // End of namespace onsem


#endif // ONSEM_TEXTTOSEMANTIC_SRC_TOOL_PARTOFSPEECH_PARTOFSPEECHDELBIGRAMIMPOSSIBILITIES_HPP
