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
    ss << currElt.second << semanticLengthUnity_toStr(currElt.first);
  ss << ")";
  pListElts.emplace_back(ss.str());
}

} // End of namespace onsem
