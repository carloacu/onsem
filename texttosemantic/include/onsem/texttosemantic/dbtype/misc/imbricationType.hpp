#ifndef ONSEM_TEXTTOSEMANTIC_TYPES_MISC_IMBRICATIONTYPE_HPP
#define ONSEM_TEXTTOSEMANTIC_TYPES_MISC_IMBRICATIONTYPE_HPP

namespace onsem {

enum class ImbricationType {
    EQUALS,           // eg. "I like banana"  & "I am fond of banana"
    OPPOSES,          // eg. "I like banana"  & "I don't like banana"
    CONTAINS,         // eg. "everybody"      & "me"
    ISCONTAINED,      // eg. "me"             & "everybody"
    HYPONYM,          // eg. "I ask"          & "I say"
    HYPERNYM,         // eg. "I say"          & "I ask"
    MORE_DETAILED,    // eg. "I look left"    & "I look"
    LESS_DETAILED,    // eg. "I look"         & "I look left"
    DIFFERS           // eg. "I am speaking"  & "I am smiling"
};

static inline ImbricationType bool_toImbricationType(bool pBool) {
    if (pBool)
        return ImbricationType::EQUALS;
    return ImbricationType::DIFFERS;
}

}    // End of namespace onsem

#endif    // ONSEM_TEXTTOSEMANTIC_TYPES_MISC_IMBRICATIONTYPE_HPP
