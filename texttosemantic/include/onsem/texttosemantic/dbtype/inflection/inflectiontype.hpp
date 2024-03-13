#ifndef ONSEM_TEXTTOSEMANTIC_TYPE_INFLECTION_INFLECTIONTYPE_HPP
#define ONSEM_TEXTTOSEMANTIC_TYPE_INFLECTION_INFLECTIONTYPE_HPP

#include <string>
#include <map>
#include <assert.h>
#include <onsem/common/enum/partofspeech.hpp>

namespace onsem {

#define LINGUISTIC_INFLECTIONTYPE_TABLE                     \
    ADD_LINGUISTIC_INFLECTIONTYPE(ADJECTIVAL, "adjectival") \
    ADD_LINGUISTIC_INFLECTIONTYPE(NOMINAL, "nominal")       \
    ADD_LINGUISTIC_INFLECTIONTYPE(PRONOMINAL, "pronominal") \
    ADD_LINGUISTIC_INFLECTIONTYPE(VERBAL, "verbal")         \
    ADD_LINGUISTIC_INFLECTIONTYPE(EMPTY, "empty")

#define ADD_LINGUISTIC_INFLECTIONTYPE(a, b) a,
enum class InflectionType { LINGUISTIC_INFLECTIONTYPE_TABLE };
#undef ADD_LINGUISTIC_INFLECTIONTYPE

#define ADD_LINGUISTIC_INFLECTIONTYPE(a, b) {InflectionType::a, b},
static const std::map<InflectionType, std::string> _inflectionType_toStr = {LINGUISTIC_INFLECTIONTYPE_TABLE};
#undef ADD_LINGUISTIC_INFLECTIONTYPE

#define ADD_LINGUISTIC_INFLECTIONTYPE(a, b) {b, InflectionType::a},
static const std::map<std::string, InflectionType> _inflectionType_fromStr = {LINGUISTIC_INFLECTIONTYPE_TABLE};
#undef ADD_LINGUISTIC_INFLECTIONTYPE

static inline std::string inflectionType_toStr(InflectionType pVerbTense) {
    return _inflectionType_toStr.find(pVerbTense)->second;
}

static inline InflectionType inflectionType_fromStr(const std::string& pVerbTenseStr) {
    auto it = _inflectionType_fromStr.find(pVerbTenseStr);
    if (it != _inflectionType_fromStr.end()) {
        return it->second;
    }
    assert(false);
    return InflectionType::EMPTY;
}

static inline InflectionType inflectionType_fromPartOfSpeech(PartOfSpeech pPartOfSpeech) {
    switch (pPartOfSpeech) {
        case PartOfSpeech::ADJECTIVE: return InflectionType::ADJECTIVAL;
        case PartOfSpeech::NOUN:
        case PartOfSpeech::PROPER_NOUN:
        case PartOfSpeech::DETERMINER:
        case PartOfSpeech::PARTITIVE:
        case PartOfSpeech::PREPOSITION: return InflectionType::NOMINAL;
        case PartOfSpeech::VERB:
        case PartOfSpeech::AUX: return InflectionType::VERBAL;
        case PartOfSpeech::PRONOUN:
        case PartOfSpeech::PRONOUN_COMPLEMENT:
        case PartOfSpeech::PRONOUN_SUBJECT: return InflectionType::PRONOMINAL;
        case PartOfSpeech::BOOKMARK:
        case PartOfSpeech::INTERSPACE:
        case PartOfSpeech::LINKBETWEENWORDS:
        case PartOfSpeech::PUNCTUATION:
        case PartOfSpeech::ADVERB:
        case PartOfSpeech::CONJUNCTIVE:
        case PartOfSpeech::SUBORDINATING_CONJONCTION:
        case PartOfSpeech::INTERJECTION:
        case PartOfSpeech::UNKNOWN: return InflectionType::EMPTY;
    }
    return InflectionType::EMPTY;
}

}    // End of namespace onsem

#endif    // ONSEM_TEXTTOSEMANTIC_TYPE_INFLECTION_INFLECTIONTYPE_HPP
