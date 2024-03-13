#ifndef ONSEM_COMMON_SEMANTICGROUNDING_ENUM_SEMANTICENTITYTYPE_HPP
#define ONSEM_COMMON_SEMANTICGROUNDING_ENUM_SEMANTICENTITYTYPE_HPP

#include <string>
#include <vector>
#include <map>

namespace onsem {

#define SEMANTIC_ENTITY_TYPE_TABLE                         \
    ADD_SEMANTIC_ENTITY_TYPE(HUMAN, "human")               \
    ADD_SEMANTIC_ENTITY_TYPE(ROBOT, "robot")               \
    ADD_SEMANTIC_ENTITY_TYPE(ANIMAL, "animal")             \
    ADD_SEMANTIC_ENTITY_TYPE(THING, "thing")               \
    ADD_SEMANTIC_ENTITY_TYPE(MODIFIER, "modifier")         \
    ADD_SEMANTIC_ENTITY_TYPE(SENTENCE, "sentence")         \
    ADD_SEMANTIC_ENTITY_TYPE(NUMBER, "number")             \
    ADD_SEMANTIC_ENTITY_TYPE(AGENTORTHING, "agentOrThing") \
    ADD_SEMANTIC_ENTITY_TYPE(UNKNOWN, "unknown")

#define ADD_SEMANTIC_ENTITY_TYPE(a, b) a,
enum class SemanticEntityType : char { SEMANTIC_ENTITY_TYPE_TABLE };
#undef ADD_SEMANTIC_ENTITY_TYPE

#define ADD_SEMANTIC_ENTITY_TYPE(a, b) b,
static const std::vector<std::string> _semanticEntityType_toStr = {SEMANTIC_ENTITY_TYPE_TABLE};
#undef ADD_SEMANTIC_ENTITY_TYPE

#define ADD_SEMANTIC_ENTITY_TYPE(a, b) {b, SemanticEntityType::a},
static const std::map<std::string, SemanticEntityType> _semanticEntityType_fromStr = {SEMANTIC_ENTITY_TYPE_TABLE};
#undef ADD_SEMANTIC_ENTITY_TYPE

#define ADD_SEMANTIC_ENTITY_TYPE(a, b) SemanticEntityType::a,
static const std::vector<SemanticEntityType> semanticEntityType_allValues = {SEMANTIC_ENTITY_TYPE_TABLE};
#undef ADD_SEMANTIC_ENTITY_TYPE

#define ADD_SEMANTIC_ENTITY_TYPE(a, b) 1 +
static const std::size_t semanticEntityType_size = SEMANTIC_ENTITY_TYPE_TABLE 0;
#undef ADD_SEMANTIC_ENTITY_TYPE

#undef SEMANTIC_ENTITY_TYPE_TABLE

static inline char semanticEntityType_toChar(SemanticEntityType pAgentType) {
    return static_cast<char>(pAgentType);
}

static inline SemanticEntityType semanticEntityType_fromChar(unsigned char pAgentType) {
    return static_cast<SemanticEntityType>(pAgentType);
}

static inline std::string semanticEntityType_toStr(SemanticEntityType pAgentType) {
    return _semanticEntityType_toStr[semanticEntityType_toChar(pAgentType)];
}

static inline SemanticEntityType semanticEntityType_fromStr(const std::string& pAgentTypeStr) {
    auto it = _semanticEntityType_fromStr.find(pAgentTypeStr);
    if (it != _semanticEntityType_fromStr.end()) {
        return it->second;
    }
    return SemanticEntityType::UNKNOWN;
}

static inline bool doesSemanticEntityTypeCanBeCompatible(SemanticEntityType pAgentType1,
                                                         SemanticEntityType pAgentType2) {
    return pAgentType1 == pAgentType2 || pAgentType1 == SemanticEntityType::UNKNOWN
        || pAgentType2 == SemanticEntityType::UNKNOWN || pAgentType1 == SemanticEntityType::SENTENCE
        || pAgentType2 == SemanticEntityType::SENTENCE
        || (pAgentType1 == SemanticEntityType::AGENTORTHING
            && (pAgentType2 == SemanticEntityType::HUMAN || pAgentType2 == SemanticEntityType::THING))
        || (pAgentType2 == SemanticEntityType::AGENTORTHING
            && (pAgentType1 == SemanticEntityType::HUMAN || pAgentType1 == SemanticEntityType::THING));
}

static inline bool doesSemanticEntityTypeAreStronglyCompatible(SemanticEntityType pAgentType1,
                                                               SemanticEntityType pAgentType2) {
    return pAgentType1 == pAgentType2
        || (pAgentType1 == SemanticEntityType::AGENTORTHING
            && (pAgentType2 == SemanticEntityType::HUMAN || pAgentType2 == SemanticEntityType::THING))
        || (pAgentType2 == SemanticEntityType::AGENTORTHING
            && (pAgentType1 == SemanticEntityType::HUMAN || pAgentType1 == SemanticEntityType::THING));
}

}    // End of namespace onsem

#endif    // ONSEM_COMMON_SEMANTICGROUNDING_ENUM_SEMANTICENTITYTYPE_HPP
