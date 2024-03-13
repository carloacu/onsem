#ifndef ONSEM_COMMON_SEMANTICGROUNDING_ENUM_SEMANTICRELATIVELOCATIONTYPE_HPP
#define ONSEM_COMMON_SEMANTICGROUNDING_ENUM_SEMANTICRELATIVELOCATIONTYPE_HPP

#include <string>
#include <vector>
#include <map>

namespace onsem {

#define SEMANTIC_RELATIVELOCATIONTYPE_TABLE                     \
    ADD_SEMANTIC_RELATIVELOCATIONTYPE(L_ABOVE, "above")         \
    ADD_SEMANTIC_RELATIVELOCATIONTYPE(L_BEHIND, "behind")       \
    ADD_SEMANTIC_RELATIVELOCATIONTYPE(L_BELOW, "below")         \
    ADD_SEMANTIC_RELATIVELOCATIONTYPE(L_CLOSE, "close")         \
    ADD_SEMANTIC_RELATIVELOCATIONTYPE(L_EAST, "east")           \
    ADD_SEMANTIC_RELATIVELOCATIONTYPE(L_INFRONTOF, "inFrontOf") \
    ADD_SEMANTIC_RELATIVELOCATIONTYPE(L_INSIDE, "inside")       \
    ADD_SEMANTIC_RELATIVELOCATIONTYPE(L_FAR, "far")             \
    ADD_SEMANTIC_RELATIVELOCATIONTYPE(L_HIGH, "high")           \
    ADD_SEMANTIC_RELATIVELOCATIONTYPE(L_LEFT, "left")           \
    ADD_SEMANTIC_RELATIVELOCATIONTYPE(L_LOW, "low")             \
    ADD_SEMANTIC_RELATIVELOCATIONTYPE(L_NORTH, "north")         \
    ADD_SEMANTIC_RELATIVELOCATIONTYPE(L_ON, "on")               \
    ADD_SEMANTIC_RELATIVELOCATIONTYPE(L_OUTSIDE, "outside")     \
    ADD_SEMANTIC_RELATIVELOCATIONTYPE(L_RIGHT, "right")         \
    ADD_SEMANTIC_RELATIVELOCATIONTYPE(L_SOUTH, "south")         \
    ADD_SEMANTIC_RELATIVELOCATIONTYPE(L_UNDER, "under")         \
    ADD_SEMANTIC_RELATIVELOCATIONTYPE(L_WEST, "west")

#define ADD_SEMANTIC_RELATIVELOCATIONTYPE(a, b) a,
enum class SemanticRelativeLocationType : char { SEMANTIC_RELATIVELOCATIONTYPE_TABLE };
#undef ADD_SEMANTIC_RELATIVELOCATIONTYPE

#define ADD_SEMANTIC_RELATIVELOCATIONTYPE(a, b) {SemanticRelativeLocationType::a, b},
static const std::map<SemanticRelativeLocationType, std::string> _semanticRelativeLocationType_toStr = {
    SEMANTIC_RELATIVELOCATIONTYPE_TABLE};
#undef ADD_SEMANTIC_RELATIVELOCATIONTYPE

#define ADD_SEMANTIC_RELATIVELOCATIONTYPE(a, b) {b, SemanticRelativeLocationType::a},
static const std::map<std::string, SemanticRelativeLocationType> _semanticRelativeLocationType_fromStr = {
    SEMANTIC_RELATIVELOCATIONTYPE_TABLE};
#undef ADD_SEMANTIC_RELATIVELOCATIONTYPE

#define ADD_SEMANTIC_RELATIVELOCATIONTYPE(a, b) SemanticRelativeLocationType::a,
static const std::vector<SemanticRelativeLocationType> semanticRelativeLocationType_allValues = {
    SEMANTIC_RELATIVELOCATIONTYPE_TABLE};
#undef ADD_SEMANTIC_RELATIVELOCATIONTYPE

#define ADD_SEMANTIC_RELATIVELOCATIONTYPE(a, b) 1 +
static const std::size_t semanticRelativeLocationType_size = SEMANTIC_RELATIVELOCATIONTYPE_TABLE 0;
#undef ADD_SEMANTIC_RELATIVELOCATIONTYPE

#undef SEMANTIC_RELATIVELOCATIONTYPE_TABLE

static inline char semanticRelativeLocationType_toChar(SemanticRelativeLocationType pRelLocationType) {
    return static_cast<char>(pRelLocationType);
}

static inline SemanticRelativeLocationType semanticRelativeLocationType_fromChar(unsigned char pRelLocationType) {
    return static_cast<SemanticRelativeLocationType>(pRelLocationType);
}

static inline std::string semanticRelativeLocationType_toStr(SemanticRelativeLocationType pRelLocationType) {
    return _semanticRelativeLocationType_toStr.find(pRelLocationType)->second;
}

static inline SemanticRelativeLocationType semanticRelativeLocationType_fromStr(
    const std::string& pRelLocationTypeStr) {
    return _semanticRelativeLocationType_fromStr.find(pRelLocationTypeStr)->second;
}

static inline SemanticRelativeLocationType semanticRelativeLocationType_fromChar(char pRelLocationTypeChar) {
    return static_cast<SemanticRelativeLocationType>(pRelLocationTypeChar);
}

static inline bool canBeSemanticallyEqualToLocationWithoutRelativeInformation(
    SemanticRelativeLocationType pRelLocationType) {
    return pRelLocationType == SemanticRelativeLocationType::L_ABOVE
        || pRelLocationType == SemanticRelativeLocationType::L_ON;
}

}    // End of namespace onsem

#endif    //  ONSEM_COMMON_SEMANTICGROUNDING_ENUM_SEMANTICRELATIVELOCATIONTYPE_HPP
