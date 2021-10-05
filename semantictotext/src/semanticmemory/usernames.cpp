#include "usernames.hpp"
#include <numeric>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticnamegrounding.hpp>
#include <onsem/semantictotext/semanticmemory/expressionhandleinmemory.hpp>


namespace onsem
{


std::string UserNames::getName() const
{
  return SemanticNameGrounding::namesToStr(names);
}


bool UserNames::operator<(const UserNames& pOther) const
{
  if (names.size() != pOther.names.size())
    return names.size() < pOther.names.size();

  {
    auto itOtherName = pOther.names.begin();
    for (auto itName = names.begin(); itName != names.end(); ++itName, ++itOtherName)
      if (*itName != *itOtherName)
        return *itName < *itOtherName;
  }

  return false;
}


bool UserNames::operator==(const UserNames& pOther) const
{
  return names == pOther.names;
}


} // End of namespace onsem

