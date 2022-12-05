#include <onsem/tester/detailedreactionanswer.hpp>

namespace onsem
{


std::string DetailedReactionAnswer::toStr() const
{
  std::string res = "\"" + answer + "\"";
  if (!references.empty())
    res += " (" + referencesToStr() + ")";
  return res;
}


std::string DetailedReactionAnswer::referencesToStr() const
{
  std::string res = "";
  bool firstIt = true;
  for (const auto& currRef : references)
  {
    if (firstIt)
      firstIt = false;
    else
      res += ", ";
    res += "\"" + currRef + "\"";
  }
  return res;
}


} // End of namespace onsem

