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
    ss << currElt.second.toStr() << semanticLengthUnity_toAbreviation(currElt.first);
  ss << ")";
  pListElts.emplace_back(ss.str());
}


void SemanticLength::convertToUnity(SemanticLengthUnity pUnity)
{
  SemanticFloat nbInGoodUnity;

  for (auto it = lengthInfos.begin(); it != lengthInfos.end(); )
  {
    if (it->first != pUnity)
    {
      nbInGoodUnity += it->second * semanticLengthUnity_untityConvertionCoeficient(it->first, pUnity);
      it = lengthInfos.erase(it);
    }
    else
    {
      ++it;
    }
  }

  if (nbInGoodUnity > 0)
    lengthInfos[pUnity] += nbInGoodUnity;
}



} // End of namespace onsem
