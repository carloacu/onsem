#ifndef ONSEM_COMMON_TYPES_SEMANTICEXPRESSION_ENUM_COMPARISONOPERATOR_HPP
#define ONSEM_COMMON_TYPES_SEMANTICEXPRESSION_ENUM_COMPARISONOPERATOR_HPP

#include <string>
#include <map>
#include <vector>

namespace onsem {

#define SEMANTIC_COMPARISONOPERATOR_TABLE               \
    SEMANTIC_COMPARISONOPERATOR(EQUAL, "equal")         \
    SEMANTIC_COMPARISONOPERATOR(DIFFERENT, "different") \
    SEMANTIC_COMPARISONOPERATOR(MORE, "more")           \
    SEMANTIC_COMPARISONOPERATOR(LESS, "less")

#define SEMANTIC_COMPARISONOPERATOR(a, b) a,
enum class ComparisonOperator : char { SEMANTIC_COMPARISONOPERATOR_TABLE };
#undef SEMANTIC_COMPARISONOPERATOR

#define SEMANTIC_COMPARISONOPERATOR(a, b) b,
static const std::vector<std::string> _ComparisonOperator_toStr = {SEMANTIC_COMPARISONOPERATOR_TABLE};
#undef SEMANTIC_COMPARISONOPERATOR

#define SEMANTIC_COMPARISONOPERATOR(a, b) {b, ComparisonOperator::a},
static const std::map<std::string, ComparisonOperator> _ComparisonOperator_fromStr = {
    SEMANTIC_COMPARISONOPERATOR_TABLE};
#undef SEMANTIC_COMPARISONOPERATOR
#undef SEMANTIC_COMPARISONOPERATOR_TABLE

static inline char ComparisonOperator_toChar(ComparisonOperator pComparisonOperator) {
    return static_cast<char>(pComparisonOperator);
}

static inline ComparisonOperator ComparisonOperator_fromChar(unsigned char pComparisonOperator) {
    return static_cast<ComparisonOperator>(pComparisonOperator);
}

static inline std::string ComparisonOperator_toStr(ComparisonOperator pComparisonOperator) {
    return _ComparisonOperator_toStr[ComparisonOperator_toChar(pComparisonOperator)];
}

static inline ComparisonOperator ComparisonOperator_fromStr(const std::string& pComparisonOperatorStr) {
    auto it = _ComparisonOperator_fromStr.find(pComparisonOperatorStr);
    if (it != _ComparisonOperator_fromStr.end()) {
        return it->second;
    }
    return ComparisonOperator::EQUAL;
}

}    // End of namespace onsem

#endif    // ONSEM_COMMON_TYPES_SEMANTICEXPRESSION_ENUM_COMPARISONOPERATOR_HPP
