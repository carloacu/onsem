#ifndef ONSEM_COMMON_SEMANTICGROUNDING_ENUM_SEMANTICREFERENCETYPE_H
#define ONSEM_COMMON_SEMANTICGROUNDING_ENUM_SEMANTICREFERENCETYPE_H

#include <string>
#include <vector>
#include <map>

namespace onsem {

#define SEMANTIC_GENERIC_REFERENCE_TABLE                  \
    ADD_SEMANTIC_REFERENCE_TYPE(DEFINITE, "definite")     \
    ADD_SEMANTIC_REFERENCE_TYPE(INDEFINITE, "indefinite") \
    ADD_SEMANTIC_REFERENCE_TYPE(UNDEFINED, "undefined")

#define ADD_SEMANTIC_REFERENCE_TYPE(a, b) a,
enum class SemanticReferenceType : char { SEMANTIC_GENERIC_REFERENCE_TABLE };
#undef ADD_SEMANTIC_REFERENCE_TYPE

#define ADD_SEMANTIC_REFERENCE_TYPE(a, b) b,
static const std::vector<std::string> _semanticReferenceType_toStr = {SEMANTIC_GENERIC_REFERENCE_TABLE};
#undef ADD_SEMANTIC_REFERENCE_TYPE

#define ADD_SEMANTIC_REFERENCE_TYPE(a, b) {b, SemanticReferenceType::a},
static const std::map<std::string, SemanticReferenceType> _semanticReferenceType_fromStr = {
    SEMANTIC_GENERIC_REFERENCE_TABLE};
#undef ADD_SEMANTIC_REFERENCE_TYPE

static inline char semanticReferenceType_tochar(SemanticReferenceType pReferenceType) {
    return static_cast<char>(pReferenceType);
}

static inline SemanticReferenceType semanticReferenceType_fromchar(unsigned char pReferenceType) {
    return static_cast<SemanticReferenceType>(pReferenceType);
}

static inline std::string semanticReferenceType_toStr(SemanticReferenceType pReferenceType) {
    return _semanticReferenceType_toStr[semanticReferenceType_tochar(pReferenceType)];
}

static inline SemanticReferenceType semanticReferenceType_fromStr(const std::string& pReferenceTypeStr) {
    auto it = _semanticReferenceType_fromStr.find(pReferenceTypeStr);
    if (it != _semanticReferenceType_fromStr.end()) {
        return it->second;
    }
    return SemanticReferenceType::UNDEFINED;
}

}    // End of namespace onsem

#endif    //  ONSEM_COMMON_SEMANTICGROUNDING_ENUM_SEMANTICREFERENCETYPE_H
