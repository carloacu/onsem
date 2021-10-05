#ifndef ONSEM_COMMON_TYPE8ENUM_SEMANTICEXPRESSIONCATEGORY_HPP
#define ONSEM_COMMON_TYPE8ENUM_SEMANTICEXPRESSIONCATEGORY_HPP

#include <string>

namespace onsem
{


#define SEMANTIC_KNOWLEDGECATEGORY_TABLE                                                    \
  ADD_SEMANTIC_KNOWLEDGECATEGORY(ACTIONDEFINITION, "actionDefinition")                      \
  ADD_SEMANTIC_KNOWLEDGECATEGORY(AFFIRMATION, "affirmation")                                \
  ADD_SEMANTIC_KNOWLEDGECATEGORY(COMMAND, "command")                                        \
  ADD_SEMANTIC_KNOWLEDGECATEGORY(CONDITION, "condition")                                    \
  ADD_SEMANTIC_KNOWLEDGECATEGORY(CONDITIONTOCOMMAND, "conditionToCommand")                  \
  ADD_SEMANTIC_KNOWLEDGECATEGORY(EXTERNALTEACHING, "externalTeaching")                      \
  ADD_SEMANTIC_KNOWLEDGECATEGORY(NOMINALGROUP, "nominalGroup")                              \
  ADD_SEMANTIC_KNOWLEDGECATEGORY(QUESTION, "question")

#define ADD_SEMANTIC_KNOWLEDGECATEGORY(a, b) a,
enum class SemanticExpressionCategory
{
  SEMANTIC_KNOWLEDGECATEGORY_TABLE
};
#undef ADD_SEMANTIC_KNOWLEDGECATEGORY



} // End of namespace onsem

#endif // ONSEM_COMMON_TYPE8ENUM_SEMANTICEXPRESSIONCATEGORY_HPP
