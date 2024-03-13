#ifndef ONSEM_COMMON_ENUM_SEMANTICGENDERTYPE_HPP
#define ONSEM_COMMON_ENUM_SEMANTICGENDERTYPE_HPP

#include <string>
#include <map>
#include <vector>
#include <ostream>

namespace onsem {

#define SEMANTIC_GENDER_TYPE_TABLE               \
    ADD_SEMANTIC_GENDER_TYPE(MASCULINE, "male")  \
    ADD_SEMANTIC_GENDER_TYPE(FEMININE, "female") \
    ADD_SEMANTIC_GENDER_TYPE(NEUTRAL, "neutral") \
    ADD_SEMANTIC_GENDER_TYPE(UNKNOWN, "unknown")

#define ADD_SEMANTIC_GENDER_TYPE(a, b) a,
enum class SemanticGenderType : char { SEMANTIC_GENDER_TYPE_TABLE };
#undef ADD_SEMANTIC_GENDER_TYPE

#define ADD_SEMANTIC_GENDER_TYPE(a, b) b,
static const std::vector<std::string> _semanticGenderType_toStr = {SEMANTIC_GENDER_TYPE_TABLE};
#undef ADD_SEMANTIC_GENDER_TYPE

#define ADD_SEMANTIC_GENDER_TYPE(a, b) {b, SemanticGenderType::a},
static const std::map<std::string, SemanticGenderType> _semanticGenderType_fromStr = {SEMANTIC_GENDER_TYPE_TABLE};
#undef ADD_SEMANTIC_GENDER_TYPE
#undef SEMANTIC_GENDER_TYPE_TABLE

static inline unsigned char semanticGenderType_toChar(SemanticGenderType pGenderType) {
    return static_cast<unsigned char>(pGenderType);
}

static inline SemanticGenderType semanticGenderType_fromChar(unsigned char pGenderType) {
    return static_cast<SemanticGenderType>(pGenderType);
}

static inline std::string semanticGenderType_toStr(SemanticGenderType pGenderType) {
    return _semanticGenderType_toStr[semanticGenderType_toChar(pGenderType)];
}

static inline SemanticGenderType semanticGenderType_fromStr(const std::string& pGenderTypeStr) {
    auto it = _semanticGenderType_fromStr.find(pGenderTypeStr);
    if (it != _semanticGenderType_fromStr.end()) {
        return it->second;
    }
    return SemanticGenderType::UNKNOWN;
}

static inline bool gendersAreWeaklyEqual(SemanticGenderType pGender1, SemanticGenderType pGender2) {
    return pGender1 == pGender2 || pGender1 == SemanticGenderType::UNKNOWN || pGender2 == SemanticGenderType::UNKNOWN;
}

static inline void gender_toConcisePrint(std::ostream& pOs, SemanticGenderType pGender) {
    switch (pGender) {
        case SemanticGenderType::MASCULINE: pOs << 'm'; return;
        case SemanticGenderType::FEMININE: pOs << 'f'; return;
        case SemanticGenderType::NEUTRAL: pOs << 'n'; return;
        case SemanticGenderType::UNKNOWN: return;
    }
}

static inline bool gender_fromConcisePrint(SemanticGenderType& pGender, char pChar) {
    switch (pChar) {
        case 'm': pGender = SemanticGenderType::MASCULINE; return true;
        case 'f': pGender = SemanticGenderType::FEMININE; return true;
        case 'n': pGender = SemanticGenderType::NEUTRAL; return true;
        default: return false;
    }
}

}    // End of namespace onsem

#endif    // ONSEM_COMMON_ENUM_SEMANTICGENDERTYPE_HPP
