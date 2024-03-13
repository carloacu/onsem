#ifndef ONSEM_COMMON_ENUM_VERBGOALENUM_HPP
#define ONSEM_COMMON_ENUM_VERBGOALENUM_HPP

#include <string>
#include <map>
#include <vector>

namespace onsem {

#define SEMANTIC_VERBGOAL_TABLE                         \
    ADD_SEMANTIC_VERBGOAL(ABILITY, "ability")           \
    ADD_SEMANTIC_VERBGOAL(ADVICE, "advice")             \
    ADD_SEMANTIC_VERBGOAL(CONDITIONAL, "conditional")   \
    ADD_SEMANTIC_VERBGOAL(MANDATORY, "mandatory")       \
    ADD_SEMANTIC_VERBGOAL(NOTIFICATION, "notification") \
    ADD_SEMANTIC_VERBGOAL(POSSIBILITY, "possibility")

#define ADD_SEMANTIC_VERBGOAL(a, b) a,
enum class VerbGoalEnum : char { SEMANTIC_VERBGOAL_TABLE };
#undef ADD_SEMANTIC_VERBGOAL

#define ADD_SEMANTIC_VERBGOAL(a, b) b,
static const std::vector<std::string> _alSemVerbGoalEnum_toStr = {SEMANTIC_VERBGOAL_TABLE};
#undef ADD_SEMANTIC_VERBGOAL

#define ADD_SEMANTIC_VERBGOAL(a, b) {b, VerbGoalEnum::a},
static const std::map<std::string, VerbGoalEnum> _alSemVerbGoalEnum_fromStr = {SEMANTIC_VERBGOAL_TABLE};
#undef ADD_SEMANTIC_VERBGOAL

static inline char semVerbGoalEnum_toChar(VerbGoalEnum pVerbGoal) {
    return static_cast<char>(pVerbGoal);
}

static inline VerbGoalEnum semVerbGoalEnum_fromChar(unsigned char pVerbGoal) {
    return static_cast<VerbGoalEnum>(pVerbGoal);
}

static inline std::string semVerbGoalEnum_toStr(VerbGoalEnum pVerbGoal) {
    return _alSemVerbGoalEnum_toStr[semVerbGoalEnum_toChar(pVerbGoal)];
}

static inline VerbGoalEnum semVerbGoalEnum_fromStr(const std::string& pVerbGoalStr) {
    auto it = _alSemVerbGoalEnum_fromStr.find(pVerbGoalStr);
    if (it != _alSemVerbGoalEnum_fromStr.end()) {
        return it->second;
    }
    return VerbGoalEnum::NOTIFICATION;
}

}    // End of namespace onsem

#endif    // ONSEM_COMMON_ENUM_VERBGOALENUM_HPP
