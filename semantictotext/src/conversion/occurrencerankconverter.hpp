#ifndef SEMANTICCONTROLLER_CONVERSION_OCCURRENCERANKCONVERTER_HPP
#define SEMANTICCONTROLLER_CONVERSION_OCCURRENCERANKCONVERTER_HPP

#include <onsem/texttosemantic/dbtype/semanticexpression/semanticexpression.hpp>


namespace onsem
{

namespace semanticOccurrenceRankConverter
{


/**
 * Eg: "what is the last thing that I said?"
 *  -> "what is the thing I said for the last time?"
 * @param pSemExp
 */
void process(UniqueSemanticExpression& pSemExp);


} // End of namespace semanticOccurrenceRankConverter

} // End of namespace onsem



#endif // SEMANTICCONTROLLER_CONVERSION_OCCURRENCERANKCONVERTER_HPP
