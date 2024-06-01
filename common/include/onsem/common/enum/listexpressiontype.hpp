#ifndef ONSEM_COMMON_TYPES_SEMANTICEXPRESSION_ENUM_LISTEXPRESSIONTYPE_HPP
#define ONSEM_COMMON_TYPES_SEMANTICEXPRESSION_ENUM_LISTEXPRESSIONTYPE_HPP

#include <string>
#include <map>
#include <vector>

namespace onsem {

#define ADD_SEMANTIC_LISTEXPTYPE_TABLE                       \
    ADD_SEMANTIC_LISTEXPTYPE(AND, "and")                     \
    ADD_SEMANTIC_LISTEXPTYPE(OR, "or")                       \
    ADD_SEMANTIC_LISTEXPTYPE(THEN, "then")                   \
    ADD_SEMANTIC_LISTEXPTYPE(THEN_REVERSED, "then_reversed") \
    ADD_SEMANTIC_LISTEXPTYPE(IN_BACKGROUND, "in_background") \
    ADD_SEMANTIC_LISTEXPTYPE(UNRELATED, "unrelated")

#define ADD_SEMANTIC_LISTEXPTYPE(a, b) a,
enum class ListExpressionType : char { ADD_SEMANTIC_LISTEXPTYPE_TABLE };
#undef ADD_SEMANTIC_LISTEXPTYPE

#define ADD_SEMANTIC_LISTEXPTYPE(a, b) b,
static const std::vector<std::string> _listExpressionType_toStr = {ADD_SEMANTIC_LISTEXPTYPE_TABLE};
#undef ADD_SEMANTIC_LISTEXPTYPE

#define ADD_SEMANTIC_LISTEXPTYPE(a, b) {b, ListExpressionType::a},
static const std::map<std::string, ListExpressionType> _listExpressionType_fromStr = {ADD_SEMANTIC_LISTEXPTYPE_TABLE};
#undef ADD_SEMANTIC_LISTEXPTYPE
#undef ADD_SEMANTIC_LISTEXPTYPE_TABLE

static inline char listExpressionType_toChar(ListExpressionType pListExpType) {
    return static_cast<char>(pListExpType);
}

static inline ListExpressionType listExpressionType_fromChar(unsigned char pListExpType) {
    return static_cast<ListExpressionType>(pListExpType);
}

static inline std::string listExpressionType_toStr(ListExpressionType pListExpType) {
    return _listExpressionType_toStr[listExpressionType_toChar(pListExpType)];
}

static inline ListExpressionType listExpressionType_fromStr(const std::string& pListExpTypeStr) {
    auto it = _listExpressionType_fromStr.find(pListExpTypeStr);
    if (it != _listExpressionType_fromStr.end()) {
        return it->second;
    }
    return ListExpressionType::UNRELATED;
}

}    // End of namespace onsem

#endif    // ONSEM_COMMON_TYPES_SEMANTICEXPRESSION_ENUM_LISTEXPRESSIONTYPE_HPP
