#ifndef ONSEM_COMMON_TYPES_SEMANTICEXPRESSION_ENUM_SEMANTICVERBTENSE_H
#define ONSEM_COMMON_TYPES_SEMANTICEXPRESSION_ENUM_SEMANTICVERBTENSE_H

#include <string>
#include <map>
#include <vector>

namespace onsem {

#define SEMANTIC_VERBTENSE_TABLE                                \
    ADD_SEMANTIC_VERBTENSE(PRESENT, "present")                  \
    ADD_SEMANTIC_VERBTENSE(PUNCTUALPRESENT, "punctual_present") \
    ADD_SEMANTIC_VERBTENSE(PAST, "past")                        \
    ADD_SEMANTIC_VERBTENSE(PUNCTUALPAST, "punctual_past")       \
    ADD_SEMANTIC_VERBTENSE(FUTURE, "future")                    \
    ADD_SEMANTIC_VERBTENSE(UNKNOWN, "unknown")

#define ADD_SEMANTIC_VERBTENSE(a, b) a,
enum class SemanticVerbTense : char { SEMANTIC_VERBTENSE_TABLE };
#undef ADD_SEMANTIC_VERBTENSE

#define ADD_SEMANTIC_VERBTENSE(a, b) b,
static const std::vector<std::string> _semanticVerbTense_toStr = {SEMANTIC_VERBTENSE_TABLE};
#undef ADD_SEMANTIC_VERBTENSE

#define ADD_SEMANTIC_VERBTENSE(a, b) {b, SemanticVerbTense::a},
static const std::map<std::string, SemanticVerbTense> _semanticVerbTense_fromStr = {SEMANTIC_VERBTENSE_TABLE};
#undef ADD_SEMANTIC_VERBTENSE

static inline char semanticVerbTense_toChar(SemanticVerbTense pVerbTense) {
    return static_cast<char>(pVerbTense);
}

static inline SemanticVerbTense semanticVerbTense_fromChar(unsigned char pVerbTense) {
    return static_cast<SemanticVerbTense>(pVerbTense);
}

static inline std::string semanticVerbTense_toStr(SemanticVerbTense pVerbTense) {
    return _semanticVerbTense_toStr[semanticVerbTense_toChar(pVerbTense)];
}

static inline SemanticVerbTense semanticVerbTense_fromStr(const std::string& pVerbTenseStr) {
    auto it = _semanticVerbTense_fromStr.find(pVerbTenseStr);
    if (it != _semanticVerbTense_fromStr.end()) {
        return it->second;
    }
    return SemanticVerbTense::UNKNOWN;
}

static inline bool isAPastTense(SemanticVerbTense pVerbTense) {
    return pVerbTense == SemanticVerbTense::PUNCTUALPAST || pVerbTense == SemanticVerbTense::PAST;
}

static inline bool isAPresentTense(SemanticVerbTense pVerbTense) {
    return pVerbTense == SemanticVerbTense::PUNCTUALPRESENT || pVerbTense == SemanticVerbTense::PRESENT;
}

}    // End of namespace onsem

#endif    // ONSEM_COMMON_TYPES_SEMANTICEXPRESSION_ENUM_SEMANTICVERBTENSE_H
