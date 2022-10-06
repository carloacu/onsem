#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticdistancegrounding.hpp>
#include <sstream>

namespace onsem
{

bool SemanticDistance::operator==
(const SemanticDistance& pOther) const
{
  return distanceInfos == pOther.distanceInfos;
}

void SemanticDistance::printDistance
(std::list<std::string>& pListElts,
 const std::string& pLabelName) const
{
  if (distanceInfos.empty())
    return;
  std::stringstream ss;
  ss << pLabelName << "(";
  for (const auto& currElt : distanceInfos)
    ss << currElt.second << semanticDistanceUnity_toStr(currElt.first);
  ss << ")";
  pListElts.emplace_back(ss.str());
}

} // End of namespace onsem
