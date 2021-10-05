#ifndef ONSEM_SEMANTICTOTEXT_CONTROLLER_STEPS_SPECIFICACTIONHANDLER_HPP
#define ONSEM_SEMANTICTOTEXT_CONTROLLER_STEPS_SPECIFICACTIONHANDLER_HPP


namespace onsem
{
struct GroundedExpression;
struct SemanticStatementGrounding;
struct SemControllerWorkingStruct;
struct SemanticMemoryBlockViewer;

namespace specificActionsHandler
{

bool process(SemControllerWorkingStruct& pWorkStruct,
             SemanticMemoryBlockViewer& pMemViewer,
             const GroundedExpression& pGrdExp,
             const SemanticStatementGrounding& pGrdExpStatement);

void process_forShowOperator(SemControllerWorkingStruct& pWorkStruct,
                             SemanticMemoryBlockViewer& pMemViewer,
                             const GroundedExpression& pGrdExp,
                             const SemanticStatementGrounding& pGrdExpStatement);



} // End of namespace specificActionsHandler
} // End of namespace onsem

#endif // ONSEM_SEMANTICTOTEXT_CONTROLLER_STEPS_SPECIFICACTIONHANDLER_HPP
