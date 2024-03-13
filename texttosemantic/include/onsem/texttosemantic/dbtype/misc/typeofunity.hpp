#ifndef ONSEM_TEXTTOSEMANTIC_DBTYPE_MISC_TYPEOFUNITY_HPP
#define ONSEM_TEXTTOSEMANTIC_DBTYPE_MISC_TYPEOFUNITY_HPP

#include <string>
#include <vector>
#include <map>
#include <assert.h>

namespace onsem {

#define SEMANTIC_TYPEOFUNITY_UNITY_TABLE                 \
    SEMANTIC_TYPEOFUNITY_UNITY(ANGLE, "angle")           \
    SEMANTIC_TYPEOFUNITY_UNITY(LENGTH, "length")         \
    SEMANTIC_TYPEOFUNITY_UNITY(PERCENTAGE, "percentage") \
    SEMANTIC_TYPEOFUNITY_UNITY(TIME, "time")

#define SEMANTIC_TYPEOFUNITY_UNITY(a, b) a,
enum class TypeOfUnity { SEMANTIC_TYPEOFUNITY_UNITY_TABLE };
#undef SEMANTIC_TYPEOFUNITY_UNITY

#define SEMANTIC_TYPEOFUNITY_UNITY(a, b) b,
static const std::vector<std::string> _typeOfUnity_toStr = {SEMANTIC_TYPEOFUNITY_UNITY_TABLE};
#undef SEMANTIC_TYPEOFUNITY_UNITY

#define SEMANTIC_TYPEOFUNITY_UNITY(a, b) {b, TypeOfUnity::a},
static const std::map<std::string, TypeOfUnity> _typeOfUnity_fromStr = {SEMANTIC_TYPEOFUNITY_UNITY_TABLE};
#undef SEMANTIC_TYPEOFUNITY_UNITY

static inline char typeOfUnity_toChar(TypeOfUnity pTypeOfUnity) {
    return static_cast<char>(pTypeOfUnity);
}

static inline TypeOfUnity typeOfUnity_fromChar(unsigned char pTypeOfUnity) {
    return static_cast<TypeOfUnity>(pTypeOfUnity);
}

static inline std::string typeOfUnity_toStr(TypeOfUnity pTypeOfUnity) {
    return _typeOfUnity_toStr[typeOfUnity_toChar(pTypeOfUnity)];
}

static inline TypeOfUnity typeOfUnity_fromStr(const std::string& pTypeOfUnityStr) {
    auto it = _typeOfUnity_fromStr.find(pTypeOfUnityStr);
    if (it != _typeOfUnity_fromStr.end())
        return it->second;
    assert(false);
    return TypeOfUnity::LENGTH;
}

}    // End of namespace onsem

#endif    // ONSEM_TEXTTOSEMANTIC_DBTYPE_MISC_TYPEOFUNITY_HPP
