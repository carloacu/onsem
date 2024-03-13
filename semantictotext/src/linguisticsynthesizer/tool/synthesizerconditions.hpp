#ifndef ONSEM_SEMANTICTOTEXT_LINGUISTICSYNTHESIZER_TOOL_SYNTHESIZERCONDITIONS_HPP
#define ONSEM_SEMANTICTOTEXT_LINGUISTICSYNTHESIZER_TOOL_SYNTHESIZERCONDITIONS_HPP

#include <string>
#include <onsem/common/utility/uppercasehandler.hpp>
#include "../synthesizertypes.hpp"

namespace onsem {

inline bool alwaysTrue(const WordToSynthesize&) {
    return true;
}

inline bool isNextIsNotAPunctuation(const WordToSynthesize& pNext) {
    return pNext.word.partOfSpeech != PartOfSpeech::PUNCTUATION;
}

inline bool ifNextCharIsAVowel_forABloc(const InflectionToSynthesize& pNext) {
    return !pNext.str.empty() && isFirstLetterAVowel(pNext.str);
}

inline bool ifNextCharIsAVowel(const WordToSynthesize& pNext) {
    assert(!pNext.inflections.empty());
    const InflectionToSynthesize& inflNext = pNext.inflections.front();
    if (inflNext.fromResourcePtr != nullptr)
        return false;
    return ifNextCharIsAVowel_forABloc(inflNext);
}

inline bool ifNeedAnApostropheBefore(const WordToSynthesize& pNext) {
    assert(!pNext.inflections.empty());
    const InflectionToSynthesize& inflNext = pNext.inflections.front();
    if (inflNext.fromResourcePtr != nullptr)
        return false;
    return ifNextCharIsAVowel_forABloc(inflNext)
        || (!inflNext.str.empty() && (inflNext.str[0] == 'h' || inflNext.str[0] == 'H')
            && pNext.contextualInfos.count(WordContextualInfos::FR_ASPIREDH) == 0);
}

inline bool ifNextWordIsNotAnAdjective(const WordToSynthesize& pNext) {
    return pNext.word.partOfSpeech != PartOfSpeech::ADJECTIVE;
}

inline bool ifNextWordIsNotAPrepostion(const WordToSynthesize& pNext) {
    return pNext.word.partOfSpeech != PartOfSpeech::PREPOSITION;
}

}    // End of namespace onsem

#endif    // ONSEM_SEMANTICTOTEXT_LINGUISTICSYNTHESIZER_TOOL_SYNTHESIZERCONDITIONS_HPP
