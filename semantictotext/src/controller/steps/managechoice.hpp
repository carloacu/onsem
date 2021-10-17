#ifndef ONSEM_SEMANTICTOTEXT_CONTROLLER_STEPS_MANAGECHOICE_HPP
#define ONSEM_SEMANTICTOTEXT_CONTROLLER_STEPS_MANAGECHOICE_HPP

#include <onsem/texttosemantic/dbtype/semanticexpression/groundedexpression.hpp>
#include "type/semcontrollerworkingstruct.hpp"

namespace onsem
{
struct SemanticMemoryBlockViewer;

/**
 * @brief manageChoice Manage a question that represent a choice according to the current operator (get, answer, react, ...)
 * Get will list all the elements that are true according to the memory
 * Answer and react will produce an answer ready to be said like: "Neither, ..."
 * @param pWorkStruct The object that gather the work context (linguistic database, meomory object, operator, ...)
 * @param pGrdExp The grounded expression of the choice question.
 * @param pGramTypeOfTheChoice Grammatical type that refer to the child that enumerate the choice of the question.
 * @param pChoiceListExp List that represent all the elements of the choice.
 * @return True if an answer has been added in pWorkStruct, false otherwise.
 */
bool manageChoice(SemControllerWorkingStruct& pWorkStruct,
                  SemanticMemoryBlockViewer& pMemViewer,
                  const GroundedExpression& pGrdExp,
                  GrammaticalType pGramTypeOfTheChoice,
                  const ListExpression& pChoiceListExp);





} // End of namespace onsem



#endif // ONSEM_SEMANTICTOTEXT_CONTROLLER_STEPS_MANAGECHOICE_HPP
