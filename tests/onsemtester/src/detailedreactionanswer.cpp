#include <onsem/tester/detailedreactionanswer.hpp>

namespace onsem
{


std::string DetailedReactionAnswer::toStr() const
{
  std::string res = "\"" + answer + "\"";
  if (!references.empty())
  {
    res += " (";
    bool firstIt = true;
    for (const auto& currRef : references)
    {
      if (firstIt)
        firstIt = false;
      else
        res += ", ";
      res += "\"" + currRef + "\"";
    }
    res += ")";
  }
  return res;
}


} // End of namespace onsem

