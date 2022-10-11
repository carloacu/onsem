#include "partofspeechdelbigramimpossibilities.hpp"
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticgenericgrounding.hpp>
#include <onsem/texttosemantic/tool/inflectionschecker.hpp>
#include <onsem/texttosemantic/tool/syntacticanalyzertokenshandler.hpp>
#include <onsem/texttosemantic/dbtype/linguistic/lingtypetoken.hpp>
#include "../chunkshandler.hpp"

namespace onsem
{
namespace linguistics
{


PartOfSpeechDelBigramImpossibilities::PartOfSpeechDelBigramImpossibilities
(const std::string& pName,
 const InflectionsChecker& pFls,
 const std::vector<std::pair<PartOfSpeech, PartOfSpeech> >& pImpSuccessions,
 const std::vector<std::pair<PartOfSpeech, PartOfSpeech> >& pCheckCompatibility)
  : PartOfSpeechContextFilter(pName),
    fFls(pFls),
    fCantBeAtTheBeginning(),
    fCheckCompatibilityAtTheBeginning(),
    fPrevSuccs(),
    fNextSuccs(),
    fCantBeAtTheEnding(),
    fCheckCompatibilityAtTheEnding()
{
  xInit(pImpSuccessions, pCheckCompatibility);
}


void PartOfSpeechDelBigramImpossibilities::xInit
(const std::vector<std::pair<PartOfSpeech, PartOfSpeech> >& pImpSuccessions,
 const std::vector<std::pair<PartOfSpeech, PartOfSpeech> >& pCheckCompatibility)
{
  for (std::size_t i = 0; i < pImpSuccessions.size(); ++i)
  {
    if (pImpSuccessions[i].first == PartOfSpeech::PUNCTUATION &&
        pImpSuccessions[i].second != PartOfSpeech::PUNCTUATION)
    {
      fCantBeAtTheBeginning.push_back(pImpSuccessions[i].second);
    }
    else if (pImpSuccessions[i].second == PartOfSpeech::PUNCTUATION &&
             pImpSuccessions[i].first != PartOfSpeech::PUNCTUATION)
    {
      fCantBeAtTheEnding.push_back(pImpSuccessions[i].first);
    }
    fNextSuccs[pImpSuccessions[i].first].emplace_back(pImpSuccessions[i].second, false);
    fPrevSuccs[pImpSuccessions[i].second].emplace_back(pImpSuccessions[i].first, false);
  }
  for (std::size_t i = 0; i < pCheckCompatibility.size(); ++i)
  {
    if (pCheckCompatibility[i].first == PartOfSpeech::PUNCTUATION &&
        pCheckCompatibility[i].second != PartOfSpeech::PUNCTUATION)
    {
      fCheckCompatibilityAtTheBeginning.push_back(pCheckCompatibility[i].second);
    }
    else if (pCheckCompatibility[i].second == PartOfSpeech::PUNCTUATION &&
             pCheckCompatibility[i].first != PartOfSpeech::PUNCTUATION)
    {
      fCheckCompatibilityAtTheEnding.push_back(pCheckCompatibility[i].first);
    }
    fNextSuccs[pCheckCompatibility[i].first].emplace_back(pCheckCompatibility[i].second, true);
    fPrevSuccs[pCheckCompatibility[i].second].emplace_back(pCheckCompatibility[i].first, true);
  }
}


bool PartOfSpeechDelBigramImpossibilities::process
(std::vector<Token>& pTokens) const
{
  TokIt prevIt = getTheNextestToken(pTokens.begin(), pTokens.end());
  if (prevIt == pTokens.end())
  {
    return false;
  }
  bool ifDel = false;
  // Delete impossible grammatical possibilities at the beginning
  ifDel |= delPartOfSpeechs(prevIt->inflWords, fCantBeAtTheBeginning);

  // Check the compatibility of the first word
  for (std::size_t i = 0; i < fCheckCompatibilityAtTheBeginning.size(); ++i)
  {
    if (prevIt->inflWords.front().word.partOfSpeech == fCheckCompatibilityAtTheBeginning[i] &&
        !fFls.areCompatibles(InflectedWord::getPuntuationIGram(),
                             prevIt->inflWords.front()))
    {
      ifDel |= delAPartOfSpeech(prevIt->inflWords, fCheckCompatibilityAtTheBeginning[i]);
    }
  }

  // iterate over all the tokens of the sentence
  for (auto it = getNextToken(prevIt, pTokens.end());
       it != pTokens.end(); it = getNextToken(it, pTokens.end()))
  {
    // Have more than 1 grammatical possiblility for the current token
    if (it->inflWords.size() > 1)
    {
      ifDel |= xDelBigramImpossibilitiesForAToken(it, prevIt, fPrevSuccs, false);
    }

    // Have more than 1 grammatical possiblility for the previous token
    if (prevIt->inflWords.size() > 1)
    {
      ifDel |= xDelBigramImpossibilitiesForAToken(prevIt, it, fNextSuccs, true);
    }
    prevIt = it;
  }

  // Delete impossible grammatical possibilities at the ending
  ifDel |= delPartOfSpeechs(prevIt->inflWords, fCantBeAtTheEnding);

  // Check the compatibility of the last word
  for (std::size_t i = 0; i < fCheckCompatibilityAtTheEnding.size(); ++i)
  {
    if (prevIt->inflWords.front().word.partOfSpeech == fCheckCompatibilityAtTheEnding[i] &&
        !fFls.areCompatibles(prevIt->inflWords.front(),
                             InflectedWord::getPuntuationIGram()))
    {
      ifDel |= delAPartOfSpeech(prevIt->inflWords, fCheckCompatibilityAtTheEnding[i]);
    }
  }

  bool ifRemoveFlexions = filterIncompatibleInflectionsInTokenList(pTokens, fFls);
  return ifDel || ifRemoveFlexions;
}


bool PartOfSpeechDelBigramImpossibilities::xDelBigramImpossibilitiesForAToken
(TokIt& pTokenItToClean,
 TokIt& pTokenItNeighbor,
 const std::map<PartOfSpeech, std::vector<GramCompatibility> >& pImpSuccs,
 bool pTokToCleanBeforeTokNeighbor) const
{
  bool ifDel = false;
  // Iterate over all grammatical possibilities that we will maybe remove
  for (auto itGram = pTokenItToClean->inflWords.begin();
       itGram != pTokenItToClean->inflWords.end(); )
  {
    // if "itGram" is impossible with the given neighbor
    bool ifItGramImp = false;

    // "itGram" has to be in the map of possible impossibilities
    auto itImpSuccsForAGram = pImpSuccs.find(itGram->word.partOfSpeech);
    if (itImpSuccsForAGram != pImpSuccs.end())
    {
      // Iterate over all grammatical possibilities of the neighbor token
      for (const auto& currGramNeighbor : pTokenItNeighbor->inflWords)
      {
        ifItGramImp = false;
        for (std::size_t i = 0; i < itImpSuccsForAGram->second.size(); ++i)
        {
          if (currGramNeighbor.word.partOfSpeech == itImpSuccsForAGram->second[i].partOfSpeech &&
              (!itImpSuccsForAGram->second[i].checkCompabiblity ||
               (pTokToCleanBeforeTokNeighbor && !fFls.areCompatibles(*itGram, currGramNeighbor)) ||
               (!pTokToCleanBeforeTokNeighbor && !fFls.areCompatibles(currGramNeighbor, *itGram))))
          {
            ifItGramImp = true;
            break;
          }
        }
        // if "itGram" and "itGramNeighbor" are compatibles we don't have to delete "itGram"
        if (!ifItGramImp)
        {
          break;
        }
      }
    }
    // if "itGram" is incompatible with the given neighbor
    if (ifItGramImp)
    {
      delAPartOfSpeech(pTokenItToClean->inflWords, itGram);
      ifDel = true;
      if (pTokenItToClean->inflWords.size() > 1)
      {
        itGram = pTokenItToClean->inflWords.begin();
      }
      else
      {
        itGram = pTokenItToClean->inflWords.end();
      }
    }
    else
    {
      ++itGram;
    }
  }
  return ifDel;
}


} // End of namespace linguistics
} // End of namespace onsem
