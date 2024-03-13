#ifndef ONSEM_COMMON_TYPE_ENUM_RELATIVEPERSON_HPP
#define ONSEM_COMMON_TYPE_ENUM_RELATIVEPERSON_HPP

#include <string>
#include <onsem/common/enum/semanticnumbertype.hpp>
#include <onsem/common/enum/relativepersonwithoutnumber.hpp>

namespace onsem {

#define SEMANTIC_REATIVEPERSON_TYPE_TABLE                       \
    ADD_SEMANTIC_REATIVEPERSON_TYPE(FIRST_SING, "first_sing")   \
    ADD_SEMANTIC_REATIVEPERSON_TYPE(SECOND_SING, "second_sing") \
    ADD_SEMANTIC_REATIVEPERSON_TYPE(THIRD_SING, "third_sing")   \
    ADD_SEMANTIC_REATIVEPERSON_TYPE(FIRST_PLUR, "first_plur")   \
    ADD_SEMANTIC_REATIVEPERSON_TYPE(SECOND_PLUR, "second_plur") \
    ADD_SEMANTIC_REATIVEPERSON_TYPE(THIRD_PLUR, "third_plur")   \
    ADD_SEMANTIC_REATIVEPERSON_TYPE(UNKNOWN, "unknown")

#define ADD_SEMANTIC_REATIVEPERSON_TYPE(a, b) a,
enum class RelativePerson { SEMANTIC_REATIVEPERSON_TYPE_TABLE };
#undef ADD_SEMANTIC_REATIVEPERSON_TYPE

#define ADD_SEMANTIC_REATIVEPERSON_TYPE(a, b) {RelativePerson::a, b},
static const std::map<RelativePerson, std::string> _relativePerson_toStr = {SEMANTIC_REATIVEPERSON_TYPE_TABLE};
#undef ADD_SEMANTIC_REATIVEPERSON_TYPE

#define ADD_SEMANTIC_REATIVEPERSON_TYPE(a, b) {b, RelativePerson::a},
static const std::map<std::string, RelativePerson> _relativePerson_fromStr = {SEMANTIC_REATIVEPERSON_TYPE_TABLE};
#undef ADD_SEMANTIC_REATIVEPERSON_TYPE

static inline SemanticNumberType relativePerson_toNumberType(RelativePerson pRelPerson) {
    switch (pRelPerson) {
        case RelativePerson::FIRST_SING:
        case RelativePerson::SECOND_SING:
        case RelativePerson::THIRD_SING: return SemanticNumberType::SINGULAR;
        case RelativePerson::FIRST_PLUR:
        case RelativePerson::SECOND_PLUR:
        case RelativePerson::THIRD_PLUR: return SemanticNumberType::PLURAL;
        case RelativePerson::UNKNOWN: return SemanticNumberType::UNKNOWN;
    }
    return SemanticNumberType::UNKNOWN;
}

static inline std::string relativePerson_toStr(RelativePerson pRelPerson) {
    return _relativePerson_toStr.find(pRelPerson)->second;
}

static inline RelativePerson relativePerson_fromStr(const std::string& pRelPersonStr) {
    auto it = _relativePerson_fromStr.find(pRelPersonStr);
    if (it != _relativePerson_fromStr.end()) {
        return it->second;
    }
    return RelativePerson::UNKNOWN;
}

static inline bool relativePersonsAreWeaklyEqual(RelativePerson pRelPerson1, RelativePerson pRelPerson2) {
    return pRelPerson1 == pRelPerson2 || pRelPerson1 == RelativePerson::UNKNOWN
        || pRelPerson2 == RelativePerson::UNKNOWN;
}

static inline RelativePerson relativePerson_fromPersonWithoutNumberAndNumber(
    RelativePersonWithoutNumber pPersonWithoutNumber,
    SemanticNumberType pNumber) {
    switch (pNumber) {
        case SemanticNumberType::SINGULAR: {
            switch (pPersonWithoutNumber) {
                case RelativePersonWithoutNumber::FIRST: return RelativePerson::FIRST_SING;
                case RelativePersonWithoutNumber::SECOND: return RelativePerson::SECOND_SING;
                case RelativePersonWithoutNumber::THIRD:
                case RelativePersonWithoutNumber::UNKNOWN:
                    return RelativePerson::THIRD_SING;
                    return RelativePerson::THIRD_SING;
            }
            break;
        }
        case SemanticNumberType::PLURAL: {
            switch (pPersonWithoutNumber) {
                case RelativePersonWithoutNumber::FIRST: return RelativePerson::FIRST_PLUR;
                case RelativePersonWithoutNumber::SECOND: return RelativePerson::SECOND_PLUR;
                case RelativePersonWithoutNumber::THIRD:
                case RelativePersonWithoutNumber::UNKNOWN:
                    return RelativePerson::THIRD_PLUR;
                    return RelativePerson::THIRD_PLUR;
            }
            break;
        }
        case SemanticNumberType::UNKNOWN: {
            switch (pPersonWithoutNumber) {
                case RelativePersonWithoutNumber::FIRST: return RelativePerson::FIRST_SING;
                case RelativePersonWithoutNumber::SECOND: return RelativePerson::SECOND_SING;
                case RelativePersonWithoutNumber::THIRD: return RelativePerson::THIRD_SING;
                case RelativePersonWithoutNumber::UNKNOWN: return RelativePerson::UNKNOWN;
            }
            break;
        }
    }
    return RelativePerson::UNKNOWN;
}

static inline void relativePerson_toConcisePrintWithoutNumber(std::ostream& pOs, RelativePerson pRelPerson) {
    switch (pRelPerson) {
        case RelativePerson::FIRST_SING:
        case RelativePerson::FIRST_PLUR: pOs << '1'; return;
        case RelativePerson::SECOND_SING:
        case RelativePerson::SECOND_PLUR: pOs << '2'; return;
        case RelativePerson::THIRD_SING:
        case RelativePerson::THIRD_PLUR: pOs << '3'; return;
        case RelativePerson::UNKNOWN: return;
    }
}

}    // End of namespace onsem

#endif    // ONSEM_COMMON_TYPE_ENUM_RELATIVEPERSON_HPP
