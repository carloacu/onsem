#ifndef ONSEM_COMMON_SEMANTICGROUNDING_ENUM_SEMANTICSUBJECTIVEQUANTITY_HPP
#define ONSEM_COMMON_SEMANTICGROUNDING_ENUM_SEMANTICSUBJECTIVEQUANTITY_HPP

#include <string>
#include <vector>
#include <map>

namespace onsem {

// max 16 for the binary encoding

#define SEMANTIC_SUBJECTIVEQUANTITY_TABLE         \
    ADD_SEMANTIC_SUBJECTIVEQUANTITY(MANY, "many") \
    ADD_SEMANTIC_SUBJECTIVEQUANTITY(SOME, "some") \
    ADD_SEMANTIC_SUBJECTIVEQUANTITY(MORE, "more") \
    ADD_SEMANTIC_SUBJECTIVEQUANTITY(LESS, "less") \
    ADD_SEMANTIC_SUBJECTIVEQUANTITY(UNKNOWN, "unknown")

#define ADD_SEMANTIC_SUBJECTIVEQUANTITY(a, b) a,
enum class SemanticSubjectiveQuantity : char { SEMANTIC_SUBJECTIVEQUANTITY_TABLE };
#undef ADD_SEMANTIC_SUBJECTIVEQUANTITY

#define ADD_SEMANTIC_SUBJECTIVEQUANTITY(a, b) b,
static const std::vector<std::string> _semanticSubjectiveQuantity_toStr = {SEMANTIC_SUBJECTIVEQUANTITY_TABLE};
#undef ADD_SEMANTIC_SUBJECTIVEQUANTITY

#define ADD_SEMANTIC_SUBJECTIVEQUANTITY(a, b) {b, SemanticSubjectiveQuantity::a},
static const std::map<std::string, SemanticSubjectiveQuantity> _semanticSubjectiveQuantity_fromStr = {
    SEMANTIC_SUBJECTIVEQUANTITY_TABLE};
#undef ADD_SEMANTIC_SUBJECTIVEQUANTITY

static inline char semanticSubjectiveQuantity_toChar(SemanticSubjectiveQuantity pSubjectiveQuantity) {
    return static_cast<char>(pSubjectiveQuantity);
}

static inline SemanticSubjectiveQuantity semanticSubjectiveQuantity_fromChar(unsigned char pQuantityType) {
    return static_cast<SemanticSubjectiveQuantity>(pQuantityType);
}

static inline std::string semanticSubjectiveQuantity_toStr(SemanticSubjectiveQuantity pSubjectiveQuantity) {
    return _semanticSubjectiveQuantity_toStr[semanticSubjectiveQuantity_toChar(pSubjectiveQuantity)];
}

static inline SemanticSubjectiveQuantity semanticSubjectiveQuantity_fromStr(const std::string& pSubjectiveQuantityStr) {
    auto it = _semanticSubjectiveQuantity_fromStr.find(pSubjectiveQuantityStr);
    if (it != _semanticSubjectiveQuantity_fromStr.end()) {
        return it->second;
    }
    return SemanticSubjectiveQuantity::UNKNOWN;
}

}    // End of namespace onsem

#endif    //  ONSEM_COMMON_SEMANTICGROUNDING_ENUM_SEMANTICSUBJECTIVEQUANTITY_HPP
