#ifndef ONSEM_COMMON_SEMANTICGROUNDING_ENUM_SEMANTICQUANTITYTYPE_H
#define ONSEM_COMMON_SEMANTICGROUNDING_ENUM_SEMANTICQUANTITYTYPE_H

#include <string>
#include <vector>
#include <map>

namespace onsem {

// max 16 for the binary encoding

#define SEMANTIC_QUANTITY_TABLE                                           \
    ADD_SEMANTIC_QUANTITY(ANYTHING, "anything")                           \
    ADD_SEMANTIC_QUANTITY(EVERYTHING, "everything")                       \
    ADD_SEMANTIC_QUANTITY(NUMBER, "number")                               \
    ADD_SEMANTIC_QUANTITY(NUMBERTOFILL, "numberToFill")                   \
    ADD_SEMANTIC_QUANTITY(MOREOREQUALTHANNUMBER, "moreOrEqualThanNumber") \
    ADD_SEMANTIC_QUANTITY(MAXNUMBER, "maxNumber")                         \
    ADD_SEMANTIC_QUANTITY(UNKNOWN, "unknown")

#define ADD_SEMANTIC_QUANTITY(a, b) a,
enum class SemanticQuantityType : char { SEMANTIC_QUANTITY_TABLE };
#undef ADD_SEMANTIC_QUANTITY

#define ADD_SEMANTIC_QUANTITY(a, b) b,
static const std::vector<std::string> _semanticQuantityType_toStr = {SEMANTIC_QUANTITY_TABLE};
#undef ADD_SEMANTIC_QUANTITY

#define ADD_SEMANTIC_QUANTITY(a, b) {b, SemanticQuantityType::a},
static const std::map<std::string, SemanticQuantityType> _semanticQuantityType_fromStr = {SEMANTIC_QUANTITY_TABLE};
#undef ADD_SEMANTIC_QUANTITY

static inline char semanticQuantityType_toChar(SemanticQuantityType pQuantityType) {
    return static_cast<char>(pQuantityType);
}

static inline SemanticQuantityType semanticQuantityType_fromChar(unsigned char pQuantityType) {
    return static_cast<SemanticQuantityType>(pQuantityType);
}

static inline std::string semanticQuantityType_toStr(SemanticQuantityType pQuantityType) {
    return _semanticQuantityType_toStr[semanticQuantityType_toChar(pQuantityType)];
}

static inline SemanticQuantityType semanticQuantityType_fromStr(const std::string& pQuantityTypeStr) {
    auto it = _semanticQuantityType_fromStr.find(pQuantityTypeStr);
    if (it != _semanticQuantityType_fromStr.end()) {
        return it->second;
    }
    return SemanticQuantityType::UNKNOWN;
}

}    // End of namespace onsem

#endif    //  ONSEM_COMMON_SEMANTICGROUNDING_ENUM_SEMANTICQUANTITYTYPE_H
