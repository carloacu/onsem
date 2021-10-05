#ifndef ONSEM_COMMON_TYPE_ENUM_RELATIVEPERSONWITHOUTNUMBER_HPP
#define ONSEM_COMMON_TYPE_ENUM_RELATIVEPERSONWITHOUTNUMBER_HPP

#include <string>
#include <map>

namespace onsem
{

#define SEMANTIC_REATIVEPERSONWITHOUTNUMBER_TYPE_TABLE                   \
  ADD_SEMANTIC_REATIVEPERSONWITHOUTNUMBER_TYPE(FIRST, "first", '1')      \
  ADD_SEMANTIC_REATIVEPERSONWITHOUTNUMBER_TYPE(SECOND, "second", '2')    \
  ADD_SEMANTIC_REATIVEPERSONWITHOUTNUMBER_TYPE(THIRD, "third", '3')      \
  ADD_SEMANTIC_REATIVEPERSONWITHOUTNUMBER_TYPE(UNKNOWN, "unknown", '#')

#define ADD_SEMANTIC_REATIVEPERSONWITHOUTNUMBER_TYPE(a, b, c) a,
enum class RelativePersonWithoutNumber {
  SEMANTIC_REATIVEPERSONWITHOUTNUMBER_TYPE_TABLE
};
#undef ADD_SEMANTIC_REATIVEPERSONWITHOUTNUMBER_TYPE



#define ADD_SEMANTIC_REATIVEPERSONWITHOUTNUMBER_TYPE(a, b, c) {RelativePersonWithoutNumber::a, b},
static const std::map<RelativePersonWithoutNumber, std::string> _relativePersonWithoutNumber_toStr = {
  SEMANTIC_REATIVEPERSONWITHOUTNUMBER_TYPE_TABLE
};
#undef ADD_SEMANTIC_REATIVEPERSONWITHOUTNUMBER_TYPE

#define ADD_SEMANTIC_REATIVEPERSONWITHOUTNUMBER_TYPE(a, b, c) {b, RelativePersonWithoutNumber::a},
static const std::map<std::string, RelativePersonWithoutNumber> _relativePersonWithoutNumber_fromStr = {
  SEMANTIC_REATIVEPERSONWITHOUTNUMBER_TYPE_TABLE
};
#undef ADD_SEMANTIC_REATIVEPERSONWITHOUTNUMBER_TYPE




static inline std::string relativePersonWithoutNumber_toStr
(RelativePersonWithoutNumber pRelPerson)
{
  return _relativePersonWithoutNumber_toStr.find(pRelPerson)->second;
}


static inline RelativePersonWithoutNumber relativePersonWithoutNumber_fromStr
(const std::string& pRelPersonStr)
{
  auto it = _relativePersonWithoutNumber_fromStr.find(pRelPersonStr);
  if (it != _relativePersonWithoutNumber_fromStr.end())
  {
    return it->second;
  }
  return RelativePersonWithoutNumber::UNKNOWN;
}



static inline void relativePersonWithoutNumber_toConcisePrint
(std::ostream& pOs,
 RelativePersonWithoutNumber pRelPerson)
{
  switch (pRelPerson)
  {
  case RelativePersonWithoutNumber::FIRST:
    pOs << '1';
    return;
  case RelativePersonWithoutNumber::SECOND:
    pOs << '2';
    return;
  case RelativePersonWithoutNumber::THIRD:
    pOs << '3';
    return;
  case RelativePersonWithoutNumber::UNKNOWN:
    return;
  }
}


static inline bool relativePersonWithoutNumber_fromConcisePrint
(RelativePersonWithoutNumber& pRelPerson,
 char pChar)
{
  switch (pChar)
  {
  case '1':
    pRelPerson = RelativePersonWithoutNumber::FIRST;
    return true;
  case '2':
    pRelPerson = RelativePersonWithoutNumber::SECOND;
    return true;
  case '3':
    pRelPerson = RelativePersonWithoutNumber::THIRD;
    return true;
  default:
    return false;
  }
}


} // End of namespace onsem

#endif // ONSEM_COMMON_TYPE_ENUM_RELATIVEPERSONWITHOUTNUMBER_HPP
