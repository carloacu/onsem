#ifndef ONSEM_TEXTTOSEMANTIC_TYPE_LINGUISTICMEANING_HPP
#define ONSEM_TEXTTOSEMANTIC_TYPE_LINGUISTICMEANING_HPP

#include <assert.h>
#include <onsem/texttosemantic/dbtype/staticlinguisticmeaning.hpp>
#include <onsem/texttosemantic/dbtype/semanticword.hpp>
#include "../api.hpp"

namespace onsem {

enum class LinguisticMeaningType { ID, WORD, EMPTY };

struct ONSEM_TEXTTOSEMANTIC_API LinguisticMeaning {
    LinguisticMeaning();
    LinguisticMeaning(const StaticLinguisticMeaning& pStaticMeaning);
    LinguisticMeaning(const SemanticWord& pWord);

    void emplace_id(SemanticLanguageEnum pLanguage, int pMeaningId);
    void emplace_word(const SemanticWord& pWord);
    bool getLanguageIfNotEmpty(SemanticLanguageEnum& pLanguage) const;

    LinguisticMeaningType getLinguisticMeaningType() const { return _linguisicMeaningType; }
    const StaticLinguisticMeaning& getStaticMeaning() const { return _staticMeaning; }
    const SemanticWord& getWord() const { return _word; }
    bool isEmpty() const { return _linguisicMeaningType == LinguisticMeaningType::EMPTY; }

private:
    LinguisticMeaningType _linguisicMeaningType;
    StaticLinguisticMeaning _staticMeaning;
    SemanticWord _word;
};

inline LinguisticMeaning::LinguisticMeaning()
    : _linguisicMeaningType(LinguisticMeaningType::EMPTY)
    , _staticMeaning()
    , _word() {}

inline LinguisticMeaning::LinguisticMeaning(const StaticLinguisticMeaning& pStaticMeaning)
    : _linguisicMeaningType(LinguisticMeaningType::ID)
    , _staticMeaning(pStaticMeaning)
    , _word() {
    assert(pStaticMeaning.meaningId != LinguisticMeaning_noMeaningId);
}

inline LinguisticMeaning::LinguisticMeaning(const SemanticWord& pWord)
    : _linguisicMeaningType(LinguisticMeaningType::WORD)
    , _staticMeaning()
    , _word(pWord) {}

inline void LinguisticMeaning::emplace_id(SemanticLanguageEnum pLanguage, int pMeaningId) {
    assert(pMeaningId != LinguisticMeaning_noMeaningId);
    _linguisicMeaningType = LinguisticMeaningType::ID;
    _staticMeaning.language = pLanguage;
    _staticMeaning.meaningId = pMeaningId;
}

inline void LinguisticMeaning::emplace_word(const SemanticWord& pWord) {
    _linguisicMeaningType = LinguisticMeaningType::WORD;
    _word = pWord;
}

inline bool LinguisticMeaning::getLanguageIfNotEmpty(SemanticLanguageEnum& pLanguage) const {
    switch (_linguisicMeaningType) {
        case LinguisticMeaningType::ID: {
            pLanguage = _staticMeaning.language;
            return true;
        }
        case LinguisticMeaningType::WORD: {
            pLanguage = _word.language;
            return true;
        }
        case LinguisticMeaningType::EMPTY: return false;
    }
    return false;
}

}    // End of namespace onsem

#endif    // ONSEM_TEXTTOSEMANTIC_TYPE_LINGUISTICMEANING_HPP
