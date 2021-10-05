#ifndef ONSEM_SEMANTICTOTEXT_CONVERSION_SIMPLESENTENCESPLITTER_HPP
#define ONSEM_SEMANTICTOTEXT_CONVERSION_SIMPLESENTENCESPLITTER_HPP

#include <onsem/texttosemantic/dbtype/semanticexpression/semanticexpression.hpp>

namespace onsem
{
namespace splitter
{


void splitInVerySimpleSentences(UniqueSemanticExpression& pSemExp,
                                bool pDoWeSplitQuestions);


} // End of namespace splitter
} // End of namespace onsem



#endif // ONSEM_SEMANTICTOTEXT_CONVERSION_SIMPLESENTENCESPLITTER_HPP
