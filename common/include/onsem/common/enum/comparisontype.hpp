#ifndef ONSEM_COMMON_TYPES_SEMANTICEXPRESSION_ENUM_COMPARISONTYPE_HPP
#define ONSEM_COMMON_TYPES_SEMANTICEXPRESSION_ENUM_COMPARISONTYPE_HPP

#include <string>
#include <map>
#include <ostream>

namespace onsem {

#define SEMANTIC_COMPARISONTYPE_TABLE                   \
    SEMANTIC_COMPARISONTYPE(COMPARATIVE, "comparative") \
    SEMANTIC_COMPARISONTYPE(SUPERLATIVE, "superlative") \
    SEMANTIC_COMPARISONTYPE(NONE, "none")

#define SEMANTIC_COMPARISONTYPE(a, b) a,
enum class ComparisonType { SEMANTIC_COMPARISONTYPE_TABLE };
#undef SEMANTIC_COMPARISONTYPE

#define SEMANTIC_COMPARISONTYPE(a, b) {ComparisonType::a, b},
static const std::map<ComparisonType, std::string> _comparisonType_toStr = {SEMANTIC_COMPARISONTYPE_TABLE};
#undef SEMANTIC_COMPARISONTYPE

#define SEMANTIC_COMPARISONTYPE(a, b) {b, ComparisonType::a},
static const std::map<std::string, ComparisonType> _comparisonType_fromStr = {SEMANTIC_COMPARISONTYPE_TABLE};
#undef SEMANTIC_COMPARISONTYPE

static inline std::string comparisonType_toStr(ComparisonType pComparisonType) {
    return _comparisonType_toStr.find(pComparisonType)->second;
}

static inline ComparisonType comparisonType_fromStr(const std::string& pComparisonTypeStr) {
    auto it = _comparisonType_fromStr.find(pComparisonTypeStr);
    if (it != _comparisonType_fromStr.end()) {
        return it->second;
    }
    return ComparisonType::NONE;
}

static inline void comparisonType_toConcisePrint(std::ostream& pOs, ComparisonType pComparisonType) {
    switch (pComparisonType) {
        case ComparisonType::COMPARATIVE: pOs << 'C'; return;
        case ComparisonType::SUPERLATIVE: pOs << 'S'; return;
        case ComparisonType::NONE: return;
    }
}

static inline bool comparisonType_fromConcisePrint(ComparisonType& pComparisonType, char pChar) {
    switch (pChar) {
        case 'C': pComparisonType = ComparisonType::COMPARATIVE; return true;
        case 'S': pComparisonType = ComparisonType::SUPERLATIVE; return true;
        default: return false;
    }
}

}    // End of namespace onsem

#endif    // ONSEM_COMMON_TYPES_SEMANTICEXPRESSION_ENUM_COMPARISONTYPE_HPP
