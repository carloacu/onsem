#ifndef ONSEM_SEMANTICTOTEXT_CONTROLLER_STEPS_MANAGECONDITION_HPP
#define ONSEM_SEMANTICTOTEXT_CONTROLLER_STEPS_MANAGECONDITION_HPP



namespace onsem
{
struct SemControllerWorkingStruct;
struct SemanticMemoryBlockViewer;
struct ConditionExpression;
struct GroundedExpression;


void manageCondition(SemControllerWorkingStruct& pWorkStruct,
                     SemanticMemoryBlockViewer& pMemViewer,
                     const ConditionExpression& pCondExp,
                     const GroundedExpression* pOriginalGrdExpPtr);





} // End of namespace onsem



#endif // ONSEM_SEMANTICTOTEXT_CONTROLLER_STEPS_MANAGECONDITION_HPP
