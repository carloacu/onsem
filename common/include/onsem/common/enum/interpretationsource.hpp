#ifndef ONSEM_COMMON_TYPES_SEMANTICEXPRESSION_ENUM_INTERPRETATIONSOURCE_HPP
#define ONSEM_COMMON_TYPES_SEMANTICEXPRESSION_ENUM_INTERPRETATIONSOURCE_HPP

#include <string>
#include <map>
#include <vector>

namespace onsem
{


#define SEMANTIC_INTERPREATIONFROM_TABLE                                    \
  ADD_SEMANTIC_INTERPREATIONFROM(AGENTGRDEXP, "agent_grdExp")               \
  ADD_SEMANTIC_INTERPREATIONFROM(FIRSTAGENTOFTEXT, "first_agent_of_text")   \
  ADD_SEMANTIC_INTERPREATIONFROM(RESTRUCTURING, "restructuring")            \
  ADD_SEMANTIC_INTERPREATIONFROM(YES_NO_REPLACEMENT, "yes_no_replacement")  \
  ADD_SEMANTIC_INTERPREATIONFROM(RECENTCONTEXT, "recent_context")           \
  ADD_SEMANTIC_INTERPREATIONFROM(TEACHING_FOLLOW_UP, "teaching_follow_up")  \
  ADD_SEMANTIC_INTERPREATIONFROM(ANDTHEN, "and_then")


#define ADD_SEMANTIC_INTERPREATIONFROM(a, b) a,
enum class InterpretationSource : char
{
  SEMANTIC_INTERPREATIONFROM_TABLE
};
#undef ADD_SEMANTIC_INTERPREATIONFROM



static inline char interpretationFrom_toChar(InterpretationSource pIntFrom)
{
  return static_cast<char>(pIntFrom);
}

static inline InterpretationSource interpretationFrom_fromChar(unsigned char pIntFrom)
{
  return static_cast<InterpretationSource>(pIntFrom);
}

static inline std::string interpretationFrom_toStr(InterpretationSource pIntFrom)
{
#define ADD_SEMANTIC_INTERPREATIONFROM(a, b) b,
static const std::vector<std::string> _interpretationFrom_toStr = {
  SEMANTIC_INTERPREATIONFROM_TABLE
};
#undef ADD_SEMANTIC_INTERPREATIONFROM
  return _interpretationFrom_toStr[interpretationFrom_toChar(pIntFrom)];
}

static inline InterpretationSource interpretationFrom_fromStr(const std::string& pIntFromStr)
{
#define ADD_SEMANTIC_INTERPREATIONFROM(a, b) {b, InterpretationSource::a},
static const std::map<std::string, InterpretationSource> _interpretationFrom_fromStr = {
  SEMANTIC_INTERPREATIONFROM_TABLE
};
#undef ADD_SEMANTIC_INTERPREATIONFROM
  auto it = _interpretationFrom_fromStr.find(pIntFromStr);
  if (it != _interpretationFrom_fromStr.end())
  {
    return it->second;
  }
  return InterpretationSource::RECENTCONTEXT;
}

#undef SEMANTIC_INTERPREATIONFROM_TABLE

} // End of namespace onsem

#endif // ONSEM_COMMON_TYPES_SEMANTICEXPRESSION_ENUM_INTERPRETATIONSOURCE_HPP
