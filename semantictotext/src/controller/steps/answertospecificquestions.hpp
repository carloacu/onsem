#ifndef ONSEM_SEMANTICTOTEXT_CONTROLLER_STEPS_ANSWERTOSPECIFICQUESTIONS_HPP
#define ONSEM_SEMANTICTOTEXT_CONTROLLER_STEPS_ANSWERTOSPECIFICQUESTIONS_HPP

#include <functional>
#include <onsem/common/enum/semanticrequesttype.hpp>
#include "type/alsemexpcontrollertypes.hpp"

namespace onsem
{
struct SemanticMemoryBlockViewer;
struct GroundedExpression;

namespace answerToSpecificQuestions
{

bool handleNameQuestion(const GroundedExpression& pGrdExp,
                        SemanticRequestType pRequestType,
                        const std::function<bool(const std::string&,
                                                 UniqueSemanticExpression,
                                                 const std::string&)>& pAnswerNameQuestion);

bool process(SemControllerWorkingStruct& pWorkStruct,
             SemanticMemoryBlockViewer& pMemViewer,
             const GroundedExpression& pGrdExp,
             SemanticRequestType pRequestType);


} // End of namespace answerToSpecificQuestions
} // End of namespace onsem



#endif // ONSEM_SEMANTICTOTEXT_CONTROLLER_STEPS_ANSWERTOSPECIFICQUESTIONS_HPP
