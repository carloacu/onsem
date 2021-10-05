#ifndef ONSEM_SEMANTICTOTEXT_CONTROLLER_STEPS_GETPROACTIVEREACTION_HPP
#define ONSEM_SEMANTICTOTEXT_CONTROLLER_STEPS_GETPROACTIVEREACTION_HPP

#include "type/alsemexpcontrollertypes.hpp"
#include <onsem/texttosemantic/dbtype/misc/truenessvalue.hpp>

namespace onsem
{
struct GroundedExpression;
struct SemanticContextAxiom;
struct SemanticMemoryBlockViewer;

namespace proactiveReaction
{

bool process(bool& pResThatCanHaveAdditionalFeedbacks,
             SemControllerWorkingStruct& pWorkStruct,
             SemanticContextAxiom* pNewContextAxiom,
             SemanticMemoryBlockViewer& pMemViewer,
             const GroundedExpression& pGrdExp,
             TruenessValue pTruenessValue,
             bool pAnswerIsAnAssertion,
             const RelatedContextAxiom& pAnswersContextAxioms);


void processWithUpdatedMemory(bool& pRes,
                              SemControllerWorkingStruct& pWorkStruct,
                              SemanticContextAxiom* pNewContextAxiom,
                              SemanticMemoryBlockViewer& pMemViewer,
                              const GroundedExpression& pGrdExp);


} // End of namespace proactiveReaction
} // End of namespace onsem



#endif // ONSEM_SEMANTICTOTEXT_CONTROLLER_STEPS_GETPROACTIVEREACTION_HPP
