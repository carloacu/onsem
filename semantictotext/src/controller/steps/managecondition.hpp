#ifndef ONSEM_SEMANTICTOTEXT_CONTROLLER_STEPS_MANAGECONDITION_HPP
#define ONSEM_SEMANTICTOTEXT_CONTROLLER_STEPS_MANAGECONDITION_HPP



namespace onsem
{
struct SemControllerWorkingStruct;
struct SemanticMemoryBlockViewer;
struct ConditionExpression;


void manageCondition(SemControllerWorkingStruct& pWorkStruct,
                     SemanticMemoryBlockViewer& pMemViewer,
                     const ConditionExpression& pCondExp);





} // End of namespace onsem



#endif // ONSEM_SEMANTICTOTEXT_CONTROLLER_STEPS_MANAGECONDITION_HPP
