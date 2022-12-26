#ifndef SEMANTICCONTROLLER_CONVERSIONS_MANDATORYFORMCONVERTER_HPP
#define SEMANTICCONTROLLER_CONVERSIONS_MANDATORYFORMCONVERTER_HPP

#include <onsem/texttosemantic/dbtype/semanticexpression/semanticexpression.hpp>


namespace onsem
{

namespace mandatoryFormConverter
{

void process(UniqueSemanticExpression& pSemExp,
             const std::string& pUserId = "");


} // End of namespace mandatoryFormConverter

} // End of namespace onsem



#endif // SEMANTICCONTROLLER_CONVERSIONS_MANDATORYFORMCONVERTER_HPP
