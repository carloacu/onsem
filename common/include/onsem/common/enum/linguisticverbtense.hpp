#ifndef ONSEM_COMMON_TYPE_ENUM_LINGUISTICVERBTENSE_HPP
#define ONSEM_COMMON_TYPE_ENUM_LINGUISTICVERBTENSE_HPP

#include <string>
#include <map>
#include <sstream>

namespace onsem
{


#define SEMANTIC_LINGUISTICVERBTENSE_TABLE                                                 \
  ADD_SEMANTIC_LINGUISTICVERBTENSE(PRESENT_INDICATIVE, "present_indicative", 'P')          \
  ADD_SEMANTIC_LINGUISTICVERBTENSE(PRESENT_CONTINUOUS, "present_continunous", 'c')         \
  ADD_SEMANTIC_LINGUISTICVERBTENSE(IMPERFECT_INDICATIVE, "imperfect_indicative", 'I')      \
  ADD_SEMANTIC_LINGUISTICVERBTENSE(PRESENT_SUBJONCTIVE, "present_subjonctive", 'S')        \
  ADD_SEMANTIC_LINGUISTICVERBTENSE(IMPERFECT_SUBJONCTIVE, "imperfect_subjonctive", 'T')    \
  ADD_SEMANTIC_LINGUISTICVERBTENSE(PRETERIT_CONTINUOUS, "preterit_continuous", 'Z')        \
  ADD_SEMANTIC_LINGUISTICVERBTENSE(PRESENT_IMPERATIVE, "present_imperative", 'Y')          \
  ADD_SEMANTIC_LINGUISTICVERBTENSE(PRESENT_CONDITIONAL, "present_conditional", 'C')        \
  ADD_SEMANTIC_LINGUISTICVERBTENSE(SIMPLE_PAST_INDICATIVE, "simple_past_indicative", 'J')  \
  ADD_SEMANTIC_LINGUISTICVERBTENSE(INFINITIVE, "infinitive", 'W')                          \
  ADD_SEMANTIC_LINGUISTICVERBTENSE(PRESENT_PARTICIPLE, "present_participle", 'G')          \
  ADD_SEMANTIC_LINGUISTICVERBTENSE(PAST_PARTICIPLE, "past_participle", 'K')                \
  ADD_SEMANTIC_LINGUISTICVERBTENSE(FUTURE_INDICATIVE, "future_indicative", 'F')


#define ADD_SEMANTIC_LINGUISTICVERBTENSE(a, b, c) a,
enum class LinguisticVerbTense
{
  SEMANTIC_LINGUISTICVERBTENSE_TABLE
};
#undef ADD_SEMANTIC_LINGUISTICVERBTENSE




#define ADD_SEMANTIC_LINGUISTICVERBTENSE(a, b, c) {LinguisticVerbTense::a, b},
static const std::map<LinguisticVerbTense, std::string> _linguisticVerbTense_toStr = {
  SEMANTIC_LINGUISTICVERBTENSE_TABLE
};
#undef ADD_SEMANTIC_LINGUISTICVERBTENSE

#define ADD_SEMANTIC_LINGUISTICVERBTENSE(a, b, c) {b, LinguisticVerbTense::a},
static const std::map<std::string, LinguisticVerbTense> _linguisticVerbTense_fromStr = {
  SEMANTIC_LINGUISTICVERBTENSE_TABLE
};
#undef ADD_SEMANTIC_LINGUISTICVERBTENSE

#define ADD_SEMANTIC_LINGUISTICVERBTENSE(a, b, c) {LinguisticVerbTense::a, c},
static const std::map<LinguisticVerbTense, char> _linguisticVerbTense_toChar = {
  SEMANTIC_LINGUISTICVERBTENSE_TABLE
};
#undef ADD_SEMANTIC_LINGUISTICVERBTENSE

#define ADD_SEMANTIC_LINGUISTICVERBTENSE(a, b, c) {c, LinguisticVerbTense::a},
static const std::map<char, LinguisticVerbTense> _linguisticVerbTense_fromChar = {
  SEMANTIC_LINGUISTICVERBTENSE_TABLE
};
#undef ADD_SEMANTIC_LINGUISTICVERBTENSE


static inline std::string linguisticVerbTense_toStr
(LinguisticVerbTense pVerbTense)
{
  return _linguisticVerbTense_toStr.find(pVerbTense)->second;
}


static inline LinguisticVerbTense linguisticVerbTense_fromStr
(const std::string& pVerbTenseStr)
{
  auto it = _linguisticVerbTense_fromStr.find(pVerbTenseStr);
  if (it != _linguisticVerbTense_fromStr.end())
  {
    return it->second;
  }
  return LinguisticVerbTense::INFINITIVE;
}


static inline char linguisticVerbTense_toChar
(LinguisticVerbTense pVerbTense)
{
  return _linguisticVerbTense_toChar.find(pVerbTense)->second;
}


static inline LinguisticVerbTense linguisticVerbTense_fromChar
(char pChar)
{
  auto it = _linguisticVerbTense_fromChar.find(pChar);
  if (it != _linguisticVerbTense_fromChar.end())
  {
    return it->second;
  }
  std::stringstream ss;
  ss << pChar << " is not a valid linguistic tense!";
  throw std::runtime_error(ss.str());
  return LinguisticVerbTense::INFINITIVE;
}


static inline bool isAPastVerbTense(LinguisticVerbTense pVerbTense)
{
  return pVerbTense == LinguisticVerbTense::IMPERFECT_INDICATIVE ||
      pVerbTense == LinguisticVerbTense::SIMPLE_PAST_INDICATIVE ||
      pVerbTense == LinguisticVerbTense::PRETERIT_CONTINUOUS;
}

static inline bool isAPresentVerbTense(LinguisticVerbTense pVerbTense)
{
  return pVerbTense == LinguisticVerbTense::PRESENT_INDICATIVE ||
      pVerbTense == LinguisticVerbTense::PRESENT_CONTINUOUS;
}


} // End of namespace onsem

#endif // ONSEM_COMMON_TYPE_ENUM_LINGUISTICVERBTENSE_HPP
