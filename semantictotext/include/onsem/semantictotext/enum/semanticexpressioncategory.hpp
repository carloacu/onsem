#ifndef ONSEM_SEMANTICTOTEXT_ENUM_SEMANTICEXPRESSIONCATEGORY_HPP
#define ONSEM_SEMANTICTOTEXT_ENUM_SEMANTICEXPRESSIONCATEGORY_HPP

#include <string>
#include <vector>

namespace onsem
{

#define ONSEM_SEMANTICEXPRESSIONCATEGORY_TABLE                                                    \
  ADD_ONSEM_SEMANTICEXPRESSIONCATEGORY(ACTIONDEFINITION, "actionDefinition")                      \
  ADD_ONSEM_SEMANTICEXPRESSIONCATEGORY(AFFIRMATION, "affirmation")                                \
  ADD_ONSEM_SEMANTICEXPRESSIONCATEGORY(COMMAND, "command")                                        \
  ADD_ONSEM_SEMANTICEXPRESSIONCATEGORY(CONDITION, "condition")                                    \
  ADD_ONSEM_SEMANTICEXPRESSIONCATEGORY(CONDITIONTOCOMMAND, "conditionToCommand")                  \
  ADD_ONSEM_SEMANTICEXPRESSIONCATEGORY(EXTERNALTEACHING, "externalTeaching")                      \
  ADD_ONSEM_SEMANTICEXPRESSIONCATEGORY(NOMINALGROUP, "nominalGroup")                              \
  ADD_ONSEM_SEMANTICEXPRESSIONCATEGORY(QUESTION, "question")

#define ADD_ONSEM_SEMANTICEXPRESSIONCATEGORY(a, b) a,
enum class SemanticExpressionCategory
{
  ONSEM_SEMANTICEXPRESSIONCATEGORY_TABLE
};
#undef ADD_ONSEM_SEMANTICEXPRESSIONCATEGORY

#define ADD_ONSEM_SEMANTICEXPRESSIONCATEGORY(a, b) b,
static const std::vector<std::string> _semanticExpressionCategory_toStr = {
  ONSEM_SEMANTICEXPRESSIONCATEGORY_TABLE
};
#undef ADD_ONSEM_SEMANTICEXPRESSIONCATEGORY

#undef ONSEM_SEMANTICEXPRESSIONCATEGORY_TABLE


static inline char semanticExpressionCategory_toChar
(SemanticExpressionCategory pSemanticExpressionCategory)
{
  return static_cast<char>(pSemanticExpressionCategory);
}

static inline std::string semanticExpressionCategory_toStr
(SemanticExpressionCategory pSemanticExpressionCategory)
{
  return _semanticExpressionCategory_toStr[semanticExpressionCategory_toChar(pSemanticExpressionCategory)];
}


} // End of namespace onsem

#endif // ONSEM_SEMANTICTOTEXT_ENUM_SEMANTICEXPRESSIONCATEGORY_HPP
