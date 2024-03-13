#ifndef ONSEM_COMMON_TYPE_ENUM_LINGENUMLINKEDMEANINGDIRECTION_HPP
#define ONSEM_COMMON_TYPE_ENUM_LINGENUMLINKEDMEANINGDIRECTION_HPP

#include <string>
#include <map>

namespace onsem {

#define ADD_LINKEDMEANINGDIRECTION_TABLE             \
    ADD_LINKEDMEANINGDIRECTION(FORWARD, "forward")   \
    ADD_LINKEDMEANINGDIRECTION(BACKWARD, "backward") \
    ADD_LINKEDMEANINGDIRECTION(BOTH, "both")

#define ADD_LINKEDMEANINGDIRECTION(a, b) a,
enum class LinkedMeaningDirection { ADD_LINKEDMEANINGDIRECTION_TABLE };
#undef ADD_LINKEDMEANINGDIRECTION

#define ADD_LINKEDMEANINGDIRECTION(a, b) {LinkedMeaningDirection::a, b},
static const std::map<LinkedMeaningDirection, std::string> _linkedMeaningDirection_toStr = {
    ADD_LINKEDMEANINGDIRECTION_TABLE};
#undef ADD_LINKEDMEANINGDIRECTION

#define ADD_LINKEDMEANINGDIRECTION(a, b) {b, LinkedMeaningDirection::a},
static const std::map<std::string, LinkedMeaningDirection> _linkedMeaningDirection_fromStr = {
    ADD_LINKEDMEANINGDIRECTION_TABLE};
#undef ADD_LINKEDMEANINGDIRECTION

static inline std::string linkedMeaningDirection_toStr(LinkedMeaningDirection pLinkedMeaningDirection) {
    return _linkedMeaningDirection_toStr.find(pLinkedMeaningDirection)->second;
}

static inline LinkedMeaningDirection linkedMeaningDirection_fromStr(const std::string& pLinkedMeaningDirectionStr) {
    auto it = _linkedMeaningDirection_fromStr.find(pLinkedMeaningDirectionStr);
    if (it != _linkedMeaningDirection_fromStr.end()) {
        return it->second;
    }
    return LinkedMeaningDirection::BOTH;
}

}    // End of namespace onsem

#endif    // ONSEM_COMMON_TYPE_ENUM_LINGENUMLINKEDMEANINGDIRECTION_HPP
