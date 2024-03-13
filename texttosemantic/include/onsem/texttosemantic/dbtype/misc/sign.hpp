#ifndef ONSEM_TEXTTOSEMANTIC_DBTYPE_MISC_SIGN_HPP
#define ONSEM_TEXTTOSEMANTIC_DBTYPE_MISC_SIGN_HPP

#include <vector>
#include <string>
#include <onsem/common/utility/to_underlying.hpp>

namespace onsem {

// Direction values
// ================

#define SEMANTIC_DIRECTION_TABLE      \
    SEMANTIC_DIRECTION(NEGATIVE, "-") \
    SEMANTIC_DIRECTION(POSITIVE, "")

#define SEMANTIC_DIRECTION(a, b) a,
enum class Sign { SEMANTIC_DIRECTION_TABLE };
#undef SEMANTIC_DIRECTION

#define SEMANTIC_DIRECTION(a, b) b,
static const std::vector<std::string> _sign_toStr = {SEMANTIC_DIRECTION_TABLE};
#undef SEMANTIC_DIRECTION
#undef SEMANTIC_DIRECTION_TABLE

static inline std::string sign_toStr(Sign pSign) {
    return _sign_toStr[mystd::to_underlying(pSign)];
}

static inline Sign invert(Sign pSign) {
    if (pSign == Sign::POSITIVE)
        return Sign::NEGATIVE;
    return Sign::POSITIVE;
}

}    // End of namespace onsem

#endif    // ONSEM_TEXTTOSEMANTIC_DBTYPE_MISC_SIGN_HPP
