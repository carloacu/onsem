#ifndef  ONSEM_COMMON_SEMANTICGROUNDING_ENUM_SEMANTICNUMBERTYPE_H
#define  ONSEM_COMMON_SEMANTICGROUNDING_ENUM_SEMANTICNUMBERTYPE_H

#include <string>
#include <map>
#include <ostream>

namespace onsem
{


#define SEMANTIC_NUMBER_TYPE_TABLE                 \
  ADD_SEMANTIC_NUMBER_TYPE(SINGULAR, "singular")   \
  ADD_SEMANTIC_NUMBER_TYPE(PLURAL, "plural")       \
  ADD_SEMANTIC_NUMBER_TYPE(UNKNOWN, "unknown")


#define ADD_SEMANTIC_NUMBER_TYPE(a, b) a,
enum class SemanticNumberType {
  SEMANTIC_NUMBER_TYPE_TABLE
};
#undef ADD_SEMANTIC_NUMBER_TYPE



#define ADD_SEMANTIC_NUMBER_TYPE(a, b) {SemanticNumberType::a, b},
static const std::map<SemanticNumberType, std::string> _semanticNumberType_toStr = {
  SEMANTIC_NUMBER_TYPE_TABLE
};
#undef ADD_SEMANTIC_NUMBER_TYPE

#define ADD_SEMANTIC_NUMBER_TYPE(a, b) {b, SemanticNumberType::a},
static const std::map<std::string, SemanticNumberType> _semanticNumberType_fromStr = {
  SEMANTIC_NUMBER_TYPE_TABLE
};
#undef ADD_SEMANTIC_NUMBER_TYPE


static inline std::string semanticNumberType_toStr
(SemanticNumberType pNumberType)
{
  return _semanticNumberType_toStr.find(pNumberType)->second;
}


static inline SemanticNumberType semanticNumberType_fromStr
(const std::string& pNumberTypeStr)
{
  auto it = _semanticNumberType_fromStr.find(pNumberTypeStr);
  if (it != _semanticNumberType_fromStr.end())
  {
    return it->second;
  }
  return SemanticNumberType::UNKNOWN;
}


static inline bool numbersAreWeaklyEqual(SemanticNumberType pNumber1,
                                         SemanticNumberType pNumber2)
{
  return pNumber1 == pNumber2 ||
      pNumber1 == SemanticNumberType::UNKNOWN ||
      pNumber2 == SemanticNumberType::UNKNOWN;
}

static inline void number_toConcisePrint
(std::ostream& pOs,
 SemanticNumberType pNumber)
{
  switch (pNumber)
  {
  case SemanticNumberType::SINGULAR:
    pOs << 's';
    return;
  case SemanticNumberType::PLURAL:
    pOs << 'p';
    return;
  case SemanticNumberType::UNKNOWN:
    return;
  }
}

static inline bool number_fromConcisePrint
(SemanticNumberType& pNumber,
 char pChar)
{
  switch (pChar)
  {
  case 's':
    pNumber = SemanticNumberType::SINGULAR;
    return true;
  case 'p':
    pNumber = SemanticNumberType::PLURAL;
    return true;
  default:
    return false;
  }
}


} // End of namespace onsem

#endif //  ONSEM_COMMON_SEMANTICGROUNDING_ENUM_SEMANTICNUMBERTYPE_H
