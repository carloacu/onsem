#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticanglegrounding.hpp>
#include <sstream>

namespace onsem
{

bool SemanticAngle::operator==(const SemanticAngle& pOther) const
{
  return angleInfos == pOther.angleInfos;
}


void SemanticAngle::printAngle(std::list<std::string>& pListElts,
                               const std::string& pLabelName) const
{
  if (angleInfos.empty())
    return;
  std::stringstream ss;
  ss << pLabelName << "(";
  for (const auto& currElt : angleInfos)
    ss << currElt.second << semanticAngleUnity_toAbreviation(currElt.first);
  ss << ")";
  pListElts.emplace_back(ss.str());
}


std::string SemanticAngle::getRawValueStr() const
{
  for (auto& currElt : angleInfos)
  {
    std::stringstream ss;
    ss << currElt.second;
    return ss.str();
  }
  return "0";
}

} // End of namespace onsem
