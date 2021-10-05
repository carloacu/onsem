#ifndef ONSEM_SEMANTICTOTEXT_LINGUISTICSYNTHESIZER_TOOL_SYNTHESIZERADDER_HPP
#define ONSEM_SEMANTICTOTEXT_LINGUISTICSYNTHESIZER_TOOL_SYNTHESIZERADDER_HPP

#include <string>
#include <list>
#include "synthesizerconditions.hpp"
#include "../synthesizertypes.hpp"
#include <onsem/common/enum/partofspeech.hpp>

namespace onsem
{
namespace synthTool
{


static inline void strToOut
(std::list<WordToSynthesize>& pOut,
 PartOfSpeech pPartOfSpeech,
 const std::string& pStr,
 SemanticLanguageEnum pLanguage,
 WordToSynthesizeTag pTag = WordToSynthesizeTag::ANY)
{
  pOut.emplace_back(SemanticWord(pLanguage, pStr, pPartOfSpeech),
                    InflectionToSynthesize(pStr, true, true, alwaysTrue), pTag);
}

static inline void strToOutCptsMove
(std::list<WordToSynthesize>& pOut,
 PartOfSpeech pPartOfSpeech,
 const std::string& pStr,
 SemanticLanguageEnum pLanguage,
 const std::map<std::string, char>&& pConcepts,
 WordToSynthesizeTag pTag = WordToSynthesizeTag::ANY)
{
  pOut.emplace_back(SemanticWord(pLanguage, pStr, pPartOfSpeech),
                    InflectionToSynthesize(pStr, true, true, alwaysTrue),
                    std::move(pConcepts), pTag);
}


static inline bool strToOutIfNotEmpty
(std::list<WordToSynthesize>& pOut,
 PartOfSpeech pPartOfSpeech,
 const std::string& pStr,
 SemanticLanguageEnum pLanguage)
{
  if (!pStr.empty())
  {
    pOut.emplace_back(SemanticWord(pLanguage, pStr, pPartOfSpeech),
                      InflectionToSynthesize(pStr, true, true, alwaysTrue));
    return true;
  }
  return false;
}


static inline void strWithApostropheToOut
(std::list<WordToSynthesize>& pOut,
 PartOfSpeech pPartOfSpeech,
 const std::string& pStrApos,
 const std::string& pStr,
 SemanticLanguageEnum pLanguage)
{
  pOut.emplace_back([&]
  {
    WordToSynthesize wordToToSynth(SemanticWord(pLanguage, pStr, pPartOfSpeech),
                                   InflectionToSynthesize(pStrApos, true, false, ifNextCharIsAVowelOrH));
    wordToToSynth.inflections.emplace_back(pStr, true, true, alwaysTrue);
    return wordToToSynth;
  }());
}


} // End of namespace synthTool
} // End of namespace onsem

#endif // ONSEM_SEMANTICTOTEXT_LINGUISTICSYNTHESIZER_TOOL_SYNTHESIZERADDER_HPP
