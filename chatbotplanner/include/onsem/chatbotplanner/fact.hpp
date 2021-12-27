#ifndef ONSEM_CHATBOTPLANNER_FACT_HPP
#define ONSEM_CHATBOTPLANNER_FACT_HPP

#include <string>
#include <vector>

namespace onsem
{
namespace cp
{

struct Fact
{
  Fact(const std::string& pName = "");

  bool operator<(const Fact& pOther) const;
  bool operator==(const Fact& pOther) const;
  bool operator!=(const Fact& pOther) const { return !operator==(pOther); }

  std::string toStr() const;

  std::size_t fillFactFromStr(
      const std::string& pStr,
      std::size_t pBeginPos,
      char pSeparator);

  static Fact fromStr(const std::string& pStr);

  std::string name;
  std::vector<Fact> parameters;
  std::string value;

private:

  static void _parametersToStr(std::string& pStr,
                               const std::vector<Fact>& pParameters);

};

} // !cp
} // !onsem


#endif // ONSEM_CHATBOTPLANNER_FACT_HPP
