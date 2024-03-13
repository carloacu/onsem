#ifndef ONSEM_SEMANTICTOTEXT_TYPES8ENUM_SEMANTICOPERATORENUM_HPP
#define ONSEM_SEMANTICTOTEXT_TYPES8ENUM_SEMANTICOPERATORENUM_HPP

namespace onsem {

enum class SemanticOperatorEnum {
    ANSWER,
    CHECK,
    FEEDBACK,
    FIND,
    GET,
    RESOLVECOMMAND,          // convert to an order or an affirmation to the corresponding action without the starting
                             // condition
    EXECUTEBEHAVIOR,         // convert an order to the corresponding action without the starting condition
    EXECUTEFROMCONDITION,    // text of a starting condition to the corresponding action
    HOWYOUKNOW,
    INFORM,
    REACT,
    REACTFROMTRIGGER,
    SHOW,
    TEACHBEHAVIOR,
    TEACHCONDITION,
    TEACHINFORMATION,
    UNINFORM
};

}    // End of namespace onsem

#endif    // ONSEM_SEMANTICTOTEXT_TYPES8ENUM_SEMANTICOPERATORENUM_HPP
