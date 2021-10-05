#ifndef ONSEM_SEMANTICTOTEXT_CONTROLLER_STEPS_SIMILARITIESWITHMEMORYFINDER_HPP
#define ONSEM_SEMANTICTOTEXT_CONTROLLER_STEPS_SIMILARITIESWITHMEMORYFINDER_HPP

#include "type/alsemexpcontrollertypes.hpp"

namespace onsem
{
struct SemanticMemoryBlockViewer;
struct GroundedExpression;
namespace similaritesWithMemoryFinder
{


bool reactOnSimilarities(SemControllerWorkingStruct& pWorkStruct,
                         SemanticMemoryBlockViewer& pMemViewer,
                         const GroundedExpression& pGrdExp);



} // End of namespace similaritesWithMemoryFinder
} // End of namespace onsem



#endif // ONSEM_SEMANTICTOTEXT_CONTROLLER_STEPS_SIMILARITIESWITHMEMORYFINDER_HPP
