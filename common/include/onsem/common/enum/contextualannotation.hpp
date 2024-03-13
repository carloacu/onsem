#ifndef ONSEM_COMMON_ENUM_CONTEXTUALANNOTATION_HPP
#define ONSEM_COMMON_ENUM_CONTEXTUALANNOTATION_HPP

#include <string>
#include <map>
#include <vector>

namespace onsem {

// TODO: remove value REMOVEALLCONDITIONS

#define ONSEM_SEMANTIC_CONTEXTUALANNOTATION_TABLE                                                       \
    ADD_ONSEM_SEMANTIC_CONTEXTUALANNOTATION(NOTIFYSOMETHINGWILLBEDONE, "notify_something_will_be_done") \
    ADD_ONSEM_SEMANTIC_CONTEXTUALANNOTATION(TEACHINGFEEDBACK, "teaching_feedback")                      \
    ADD_ONSEM_SEMANTIC_CONTEXTUALANNOTATION(EXTERNALTEACHINGREQUEST, "external_teaching_request")       \
    ADD_ONSEM_SEMANTIC_CONTEXTUALANNOTATION(BEHAVIOR, "behavior")                                       \
    ADD_ONSEM_SEMANTIC_CONTEXTUALANNOTATION(ANSWER, "answer")                                           \
    ADD_ONSEM_SEMANTIC_CONTEXTUALANNOTATION(QUESTION, "question")                                       \
    ADD_ONSEM_SEMANTIC_CONTEXTUALANNOTATION(FEEDBACK, "feedback")                                       \
    ADD_ONSEM_SEMANTIC_CONTEXTUALANNOTATION(BEHAVIORNOTFOUND, "behavior_not_found")                     \
    ADD_ONSEM_SEMANTIC_CONTEXTUALANNOTATION(ANSWERNOTFOUND, "answer_not_found")                         \
    ADD_ONSEM_SEMANTIC_CONTEXTUALANNOTATION(REMOVEALLCONDITIONS, "remove_all_conditions")               \
    ADD_ONSEM_SEMANTIC_CONTEXTUALANNOTATION(PROACTIVE, "proactive")

#define ADD_ONSEM_SEMANTIC_CONTEXTUALANNOTATION(a, b) a,
enum class ContextualAnnotation : char { ONSEM_SEMANTIC_CONTEXTUALANNOTATION_TABLE };
#undef ADD_ONSEM_SEMANTIC_CONTEXTUALANNOTATION

#define ADD_ONSEM_SEMANTIC_CONTEXTUALANNOTATION(a, b) b,
static const std::vector<std::string> _contextualAnnotation_toStr = {ONSEM_SEMANTIC_CONTEXTUALANNOTATION_TABLE};
#undef ADD_ONSEM_SEMANTIC_CONTEXTUALANNOTATION

#define ADD_ONSEM_SEMANTIC_CONTEXTUALANNOTATION(a, b) {b, ContextualAnnotation::a},
static const std::map<std::string, ContextualAnnotation> _contextualAnnotation_fromStr = {
    ONSEM_SEMANTIC_CONTEXTUALANNOTATION_TABLE};
#undef ADD_ONSEM_SEMANTIC_CONTEXTUALANNOTATION
#undef ONSEM_SEMANTIC_CONTEXTUALANNOTATION_TABLE

static inline char contextualAnnotation_toChar(ContextualAnnotation pContextualAnnotation) {
    return static_cast<char>(pContextualAnnotation);
}

static inline ContextualAnnotation contextualAnnotation_fromChar(unsigned char pContextualAnnotation) {
    return static_cast<ContextualAnnotation>(pContextualAnnotation);
}

static inline std::string contextualAnnotation_toStr(ContextualAnnotation pContextualAnnotation) {
    return _contextualAnnotation_toStr[contextualAnnotation_toChar(pContextualAnnotation)];
}

static inline ContextualAnnotation contextualAnnotation_fromStr(const std::string& pContextualAnnotationStr) {
    auto it = _contextualAnnotation_fromStr.find(pContextualAnnotationStr);
    if (it != _contextualAnnotation_fromStr.end())
        return it->second;
    return ContextualAnnotation::PROACTIVE;
}

}    // End of namespace onsem

#endif    // ONSEM_COMMON_ENUM_CONTEXTUALANNOTATION_HPP
