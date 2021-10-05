#include "englishthendisambiguator.hpp"
#include <onsem/texttosemantic/dbtype/inflectedword.hpp>
#include <onsem/texttosemantic/dbtype/linguistic/lingtypetoken.hpp>
#include <onsem/texttosemantic/tool/syntacticanalyzertokenshandler.hpp>

namespace onsem
{
namespace linguistics
{

void englishThenDisambiguator(std::vector<Token>& pTokens)
{
  for (TokIt itTok = getNextToken(pTokens.begin(), pTokens.end()); itTok != pTokens.end();
       itTok = getNextToken(itTok, pTokens.end()))
  {
    InflectedWord& currIGram = itTok->inflWords.front();
    if (currIGram.word.partOfSpeech == PartOfSpeech::CONJUNCTIVE &&
        currIGram.word.lemma == "then")
    {
      bool isACondition = false;
      for (TokIt itPrevTok = getPrevToken(itTok, pTokens.begin(), pTokens.end()); itPrevTok != pTokens.end();
           itPrevTok = getPrevToken(itPrevTok, pTokens.begin(), pTokens.end()))
      {
        const InflectedWord& prevIGram = itPrevTok->inflWords.front();
        if (prevIGram.word.partOfSpeech == PartOfSpeech::PUNCTUATION)
          break;
        if (prevIGram.word.partOfSpeech == PartOfSpeech::CONJUNCTIVE &&
            (prevIGram.infos.hasContextualInfo(WordContextualInfos::THEN) ||
             prevIGram.infos.hasContextualInfo(WordContextualInfos::ELSE)))
          break;
        else if (prevIGram.word.partOfSpeech == PartOfSpeech::SUBORDINATING_CONJONCTION &&
                 prevIGram.infos.hasContextualInfo(WordContextualInfos::CONDITION))
        {
          isACondition = true;
          break;
        }
      }

      if (!isACondition)
        currIGram.infos.concepts.emplace("list_then", 4);
    }
  }
}

} // End of namespace linguistics
} // End of namespace onsem

