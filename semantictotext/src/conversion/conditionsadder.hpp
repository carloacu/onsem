#ifndef SEMANTICCONTROLLER_CONVERSIONS_CONDITIONSADDER_HPP
#define SEMANTICCONTROLLER_CONVERSIONS_CONDITIONSADDER_HPP

#include <onsem/texttosemantic/dbtype/semanticexpression/semanticexpression.hpp>


namespace onsem
{

namespace conditionsAdder
{

void addConditonsForSomeTimedGrdExp(UniqueSemanticExpression& pSemExp,
                                    const std::string& pUserId = "",
                                    bool pAddInterpretations = true);


} // End of namespace conditionsAdder

} // End of namespace onsem



#endif // SEMANTICCONTROLLER_CONVERSIONS_CONDITIONSADDER_HPP
