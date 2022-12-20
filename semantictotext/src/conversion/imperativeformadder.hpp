#ifndef SEMANTICCONTROLLER_CONVERSIONS_IMPERATIVEFORMADDER_HPP
#define SEMANTICCONTROLLER_CONVERSIONS_IMPERATIVEFORMADDER_HPP

#include <onsem/texttosemantic/dbtype/semanticexpression/semanticexpression.hpp>


namespace onsem
{

namespace imperativeFormAdder
{

void addFormForMandatoryGrdExps(UniqueSemanticExpression& pSemExp,
                                const std::string& pUserId = "");


} // End of namespace imperativeFormAdder

} // End of namespace onsem



#endif // SEMANTICCONTROLLER_CONVERSIONS_IMPERATIVEFORMADDER_HPP
