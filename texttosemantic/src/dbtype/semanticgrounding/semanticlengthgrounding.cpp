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


std::string SemanticLength::getRawValueStr() const
{
  const SemanticLengthUnity* unityPtr = nullptr;
  long long res = 0;
  for (auto& currElt : lengthInfos)
  {
    res += currElt.second * semanticLengthUnity_toNbOfMilliseconds(currElt.first);
    if (unityPtr == nullptr)
      unityPtr = &currElt.first;
  }

  std::stringstream ss;
  if (unityPtr != nullptr)
    ss << res / semanticLengthUnity_toNbOfMilliseconds(*unityPtr);
  return ss.str();
}


} // End of namespace onsem
