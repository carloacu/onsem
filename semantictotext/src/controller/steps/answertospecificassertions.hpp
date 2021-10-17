#ifndef ONSEM_SEMANTICTOTEXT_CONTROLLER_STEPS_ANSWERTOSPECIFICASSERTIONS_HPP
#define ONSEM_SEMANTICTOTEXT_CONTROLLER_STEPS_ANSWERTOSPECIFICASSERTIONS_HPP

#include <onsem/common/enum/semanticrequesttype.hpp>
#include "type/semcontrollerworkingstruct.hpp"

namespace onsem
{
struct GroundedExpression;
struct SemanticMemoryBlockViewer;

namespace answerToSpecificAssertions
{


bool process(SemControllerWorkingStruct& pWorkStruct,
             SemanticMemoryBlockViewer& pMemViewer,
             const GroundedExpression& pGrdExp);


} // End of namespace answerToSpecificAssertions
} // End of namespace onsem



#endif // ONSEM_SEMANTICTOTEXT_CONTROLLER_STEPS_ANSWERTOSPECIFICASSERTIONS_HPP
