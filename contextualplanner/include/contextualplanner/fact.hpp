#ifndef INCLUDE_CONTEXTUALPLANNER_FACT_HPP
#define INCLUDE_CONTEXTUALPLANNER_FACT_HPP

#include <string>
#include <vector>
#include <map>
#include "api.hpp"


namespace cp
{

struct CONTEXTUALPLANNER_API Fact
{
  Fact(const std::string& pName = "");

  bool operator<(const Fact& pOther) const;
  bool operator==(const Fact& pOther) const;
  bool operator!=(const Fact& pOther) const { return !operator==(pOther); }
  bool areEqualExceptAnyValues(const Fact& pOther) const;
  std::string tryToExtractParameterValueFromExemple(
      const std::string& pParameter,
      const Fact& pOther) const;
  void fillParameters(
      const std::map<std::string, std::string>& pParameters);

  std::string toStr() const;

  std::size_t fillFactFromStr(
      const std::string& pStr,
      std::size_t pBeginPos,
      char pSeparator);

  static Fact fromStr(const std::string& pStr);
  bool replaceParametersByAny(const std::vector<std::string>& pParameters);

  std::string name;
  std::vector<Fact> parameters;
  std::string value;

  const static std::string anyValue;
private:

  static void _parametersToStr(std::string& pStr,
                               const std::vector<Fact>& pParameters);

};

} // !cp


#endif // INCLUDE_CONTEXTUALPLANNER_FACT_HPP
