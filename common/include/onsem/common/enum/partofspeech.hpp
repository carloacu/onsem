#ifndef ONSEM_COMMON_TYPE_PARTOSPEECH_HPP
#define ONSEM_COMMON_TYPE_PARTOSPEECH_HPP

#include <string>
#include <map>
#include <vector>
#include <assert.h>

namespace onsem {

/**
 * Every grammatical possibilities for a token.
 *
 * Be careful, for optimisation purpose, the order is important:
 * "<= PartOfSpeech::BOOKMARK"   == an annotation
 * "<= PartOfSpeech::INTERSPACE" == we can insert a new bookmark just after it
 * ">= PartOfSpeech::UNKNOWN"    == a word
 */
#define PARTOFSPEECH_TYPE_TABLE                                                     \
    ADD_PARTOFSPEECH_TYPE(BOOKMARK, "bookmark", "bookmark")                         \
    ADD_PARTOFSPEECH_TYPE(INTERSPACE, "interspace", "interspace")                   \
    ADD_PARTOFSPEECH_TYPE(LINKBETWEENWORDS, "linkbetweenwords", "linkBetweenWords") \
    ADD_PARTOFSPEECH_TYPE(PUNCTUATION, "punctuation", "punctuation")                \
    ADD_PARTOFSPEECH_TYPE(UNKNOWN, "unknown", "unknown")                            \
    ADD_PARTOFSPEECH_TYPE(ADJECTIVE, "adjective", "adjective")                      \
    ADD_PARTOFSPEECH_TYPE(ADVERB, "adverb", "adverb")                               \
    ADD_PARTOFSPEECH_TYPE(CONJUNCTIVE, "conjunctive", "conjunctive")                \
    ADD_PARTOFSPEECH_TYPE(SUBORDINATING_CONJONCTION, "sub_conj", "sub-conj")        \
    ADD_PARTOFSPEECH_TYPE(DETERMINER, "determiner", "determiner")                   \
    ADD_PARTOFSPEECH_TYPE(PARTITIVE, "partitive", "partitive")                      \
    ADD_PARTOFSPEECH_TYPE(INTERJECTION, "interjection", "interjection")             \
    ADD_PARTOFSPEECH_TYPE(NOUN, "noun", "noun")                                     \
    ADD_PARTOFSPEECH_TYPE(PREPOSITION, "preposition", "preposition")                \
    ADD_PARTOFSPEECH_TYPE(PRONOUN, "pronoun", "pronoun")                            \
    ADD_PARTOFSPEECH_TYPE(VERB, "verb", "verb")                                     \
    ADD_PARTOFSPEECH_TYPE(AUX, "auxiliary", "auxiliary")                            \
    ADD_PARTOFSPEECH_TYPE(PRONOUN_COMPLEMENT, "pronoun_comp", "pronounComp")        \
    ADD_PARTOFSPEECH_TYPE(PRONOUN_SUBJECT, "pronoun_subject", "pronounSubject")     \
    ADD_PARTOFSPEECH_TYPE(PROPER_NOUN, "proper_noun", "properNoun")

/// It is a grammatical information refering of how a word is used in a sentence.
#define ADD_PARTOFSPEECH_TYPE(a, b, c) a,
enum class PartOfSpeech : char { PARTOFSPEECH_TYPE_TABLE };
#undef ADD_PARTOFSPEECH_TYPE

#define ADD_PARTOFSPEECH_TYPE(a, b, c) {b, PartOfSpeech::a},
static const std::map<std::string, PartOfSpeech> _partOfSpeech_fromStr = {PARTOFSPEECH_TYPE_TABLE};
#undef ADD_PARTOFSPEECH_TYPE

#define ADD_PARTOFSPEECH_TYPE(a, b, c) b,
static const std::vector<std::string> _partOfSpeech_toStr = {PARTOFSPEECH_TYPE_TABLE};
#undef ADD_PARTOFSPEECH_TYPE

#define ADD_PARTOFSPEECH_TYPE(a, b, c) c,
static const std::vector<std::string> _partOfSpeech_toCptName = {PARTOFSPEECH_TYPE_TABLE};
#undef ADD_PARTOFSPEECH_TYPE
#undef PARTOFSPEECH_TYPE_TABLE

static inline char partOfSpeech_toChar(PartOfSpeech pPartOfSpeech) {
    return static_cast<char>(pPartOfSpeech);
}

static inline PartOfSpeech partOfSpeech_fromChar(unsigned char pPartOfSpeech) {
    return static_cast<PartOfSpeech>(pPartOfSpeech);
}

static inline std::string partOfSpeech_toStr(PartOfSpeech pPartOfSpeech) {
    return _partOfSpeech_toStr[partOfSpeech_toChar(pPartOfSpeech)];
}

static inline std::string partOfSpeech_toCptName(PartOfSpeech pPartOfSpeech) {
    return _partOfSpeech_toCptName[partOfSpeech_toChar(pPartOfSpeech)];
}

inline static PartOfSpeech partOfSpeech_fromStr(const std::string& pPartOfSpeechStr) {
    auto it = _partOfSpeech_fromStr.find(pPartOfSpeechStr);
    if (it != _partOfSpeech_fromStr.end())
        return it->second;
    assert(false);
    return PartOfSpeech::UNKNOWN;
}

static inline bool partOfSpeech_isAWord(PartOfSpeech pPartOfSpeech) {
    return pPartOfSpeech >= PartOfSpeech::UNKNOWN;
}

static inline bool partOfSpeech_isVerbal(PartOfSpeech pPartOfSpeech) {
    return pPartOfSpeech == PartOfSpeech::VERB || pPartOfSpeech == PartOfSpeech::AUX;
}

static inline bool partOfSpeech_isNominal(PartOfSpeech pPartOfSpeech) {
    return pPartOfSpeech == PartOfSpeech::NOUN || pPartOfSpeech == PartOfSpeech::PROPER_NOUN
        || pPartOfSpeech == PartOfSpeech::UNKNOWN;
}

static inline bool partOfSpeech_isPronominal(PartOfSpeech pPartOfSpeech) {
    return pPartOfSpeech == PartOfSpeech::PRONOUN || pPartOfSpeech == PartOfSpeech::PRONOUN_COMPLEMENT
        || pPartOfSpeech == PartOfSpeech::PRONOUN_SUBJECT;
}

}    // End of namespace onsem

#endif    // ONSEM_COMMON_TYPE_PARTOSPEECH_HPP
