#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticlengthgrounding.hpp>
#include <sstream>

namespace onsem
{

bool SemanticLength::operator==
(const SemanticLength& pOther) const
{
  return lengthInfos == pOther.lengthInfos;
}

void SemanticLength::printLength
(std::list<std::string>& pListElts,
 const std::string& pLabelName) const
{
  if (lengthInfos.empty())
    return;
  std::stringstream ss;
  ss << pLabelName << "(";
  for (const auto& currElt : lengthInfos)
    ss << currElt.second << semanticLengthUnity_toAbreviation(currElt.first);
  ss << ")";
  pListElts.emplace_back(ss.str());
}


void SemanticLength::convertToUnity(SemanticLengthUnity pUnity)
{
  long long nbInMillimeter = 0;

  for (auto it = lengthInfos.begin(); it != lengthInfos.end(); )
  {
    if (it->first != pUnity)
    {
      nbInMillimeter += it->second * semanticLengthUnity_toNbOfMilliseconds(it->first);
      it = lengthInfos.erase(it);
    }
    else
    {
      ++it;
    }
  }

  if (nbInMillimeter > 0)
    lengthInfos[pUnity] += nbInMillimeter / semanticLengthUnity_toNbOfMilliseconds(pUnity);
}


} // End of namespace onsem
