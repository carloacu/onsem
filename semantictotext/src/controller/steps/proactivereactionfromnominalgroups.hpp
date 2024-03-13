#ifndef ONSEM_SEMANTICTOTEXT_CONTROLLER_STEPS_PROACTIVEREACTIONFROMNOMINALGROUPS_HPP
#define ONSEM_SEMANTICTOTEXT_CONTROLLER_STEPS_PROACTIVEREACTIONFROMNOMINALGROUPS_HPP

namespace onsem {
struct SemControllerWorkingStruct;
struct SemanticMemoryBlockViewer;
struct GroundedExpression;

namespace proactiveReactionFromNominalGroup {

bool reactOnSentimentsFromNominalGroup(SemControllerWorkingStruct& pWorkStruct, const GroundedExpression& pGrdExp);

bool react(SemControllerWorkingStruct& pWorkStruct,
           SemanticMemoryBlockViewer& pMemViewer,
           const GroundedExpression& pGrdExp);

}    // End of namespace proactiveReactionFromNominalGroup
}    // End of namespace onsem

#endif    // ONSEM_SEMANTICTOTEXT_CONTROLLER_STEPS_PROACTIVEREACTIONFROMNOMINALGROUPS_HPP
