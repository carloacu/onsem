#ifndef ONSEM_SEMANTICTOTEXT_LINGUISTICSYNTHESIZER_TOOL_SYNTHESIZERADDER_HPP
#define ONSEM_SEMANTICTOTEXT_LINGUISTICSYNTHESIZER_TOOL_SYNTHESIZERADDER_HPP

#include <string>
#include <list>
#include "synthesizerconditions.hpp"
#include "../synthesizertypes.hpp"
#include <onsem/common/enum/partofspeech.hpp>

namespace onsem {
namespace synthTool {

static inline void strToOut(std::list<WordToSynthesize>& pOut,
                            PartOfSpeech pPartOfSpeech,
                            const std::string& pStr,
                            SemanticLanguageEnum pLanguage,
                            WordToSynthesizeTag pTag = WordToSynthesizeTag::ANY,
                            int pPriorityOffset = 0) {
    pOut.emplace_back(SemanticWord(pLanguage, pStr, pPartOfSpeech),
                      InflectionToSynthesize(pStr, true, true, alwaysTrue),
                      pTag,
                      nullptr,
                      pPriorityOffset);
}

static inline void strToOutCptsMove(std::list<WordToSynthesize>& pOut,
                                    PartOfSpeech pPartOfSpeech,
                                    const std::string& pStr,
                                    SemanticLanguageEnum pLanguage,
                                    const std::map<std::string, char>&& pConcepts,
                                    WordToSynthesizeTag pTag = WordToSynthesizeTag::ANY) {
    pOut.emplace_back(SemanticWord(pLanguage, pStr, pPartOfSpeech),
                      InflectionToSynthesize(pStr, true, true, alwaysTrue),
                      std::move(pConcepts),
                      pTag);
}

static inline bool strToOutIfNotEmpty(std::list<WordToSynthesize>& pOut,
                                      PartOfSpeech pPartOfSpeech,
                                      const std::string& pStr,
                                      SemanticLanguageEnum pLanguage,
                                      const std::set<WordContextualInfos>* pContextualInfosPtr = nullptr) {
    if (!pStr.empty()) {
        pOut.emplace_back(SemanticWord(pLanguage, pStr, pPartOfSpeech),
                          InflectionToSynthesize(pStr, true, true, alwaysTrue),
                          WordToSynthesizeTag::ANY,
                          pContextualInfosPtr);
        return true;
    }
    return false;
}

static inline void strWithApostropheToOut(
    std::list<WordToSynthesize>& pOut,
    PartOfSpeech pPartOfSpeech,
    const std::string& pStrApos,
    const std::string& pStr,
    SemanticLanguageEnum pLanguage,
    bool (*pContextCondition)(const OutSentence*) = [](const OutSentence*) { return true; }) {
    pOut.emplace_back([&] {
        WordToSynthesize wordToToSynth(
            SemanticWord(pLanguage, pStr, pPartOfSpeech),
            InflectionToSynthesize(pStrApos, true, false, ifNeedAnApostropheBefore, pContextCondition));
        wordToToSynth.inflections.emplace_back(pStr, true, true, alwaysTrue, pContextCondition);
        return wordToToSynth;
    }());
}

static inline void addInflectionToLastOut(std::list<WordToSynthesize>& pOut, const std::string& pStr) {
    if (!pOut.empty())
        pOut.back().inflections.emplace_back(InflectionToSynthesize(pStr, true, true, alwaysTrue));
}

}    // End of namespace synthTool
}    // End of namespace onsem

#endif    // ONSEM_SEMANTICTOTEXT_LINGUISTICSYNTHESIZER_TOOL_SYNTHESIZERADDER_HPP
