#include <contextualplanner/fact.hpp>
#include <assert.h>


namespace cp
{
const std::string Fact::anyValue = "<any>_it_is_a_language_token_for_the_planner_engine";


Fact::Fact(const std::string& pName)
  : name(pName),
    parameters(),
    value()
{
}

bool Fact::operator<(const Fact& pOther) const
{
  if (name != pOther.name)
    return name < pOther.name;
  if (value != pOther.value)
    return value < pOther.value;
  std::string paramStr;
  _parametersToStr(paramStr, parameters);
  std::string otherParamStr;
  _parametersToStr(otherParamStr, pOther.parameters);
  return paramStr < otherParamStr;
}

bool Fact::operator==(const Fact& pOther) const
{
  return name == pOther.name && value == pOther.value && parameters == pOther.parameters;
}

bool Fact::areEqualExceptAnyValues(const Fact& pOther) const
{
  if (!(name == pOther.name &&
        (value == pOther.value || value == anyValue || pOther.value == anyValue) &&
        parameters.size() == pOther.parameters.size()))
    return false;

  auto itParam = parameters.begin();
  auto itOtherParam = parameters.begin();
  while (itParam != parameters.end())
  {
    if (*itParam != *itOtherParam && *itParam != anyValue && *itOtherParam != anyValue)
      return false;
    ++itParam;
    ++itOtherParam;
  }
  return true;
}


std::string Fact::tryToExtractParameterValueFromExemple(
    const std::string& pParameter,
    const Fact& pOther) const
{
  if (name != pOther.name ||
      parameters.size() != pOther.parameters.size())
    return "";

  std::string res;
  if (value != pOther.value)
  {
    if (value == pParameter)
      res = pOther.value;
    else
      return "";
  }

  auto itParam = parameters.begin();
  auto itOtherParam = parameters.begin();
  while (itParam != parameters.end())
  {
    if (*itParam != *itOtherParam)
    {
      if (itParam->name == pParameter)
        res = itOtherParam->name;
      else
        return "";
    }
    ++itParam;
    ++itOtherParam;
  }
  return res;
}


void Fact::fillParameters(
    const std::map<std::string, std::string>& pParameters)
{
  auto itValueParam = pParameters.find(value);
  if (itValueParam != pParameters.end())
    value = itValueParam->second;

  for (auto& currParam : parameters)
  {
    if (currParam.value.empty() && currParam.parameters.empty())
    {
      auto itValueParam = pParameters.find(currParam.name);
      if (itValueParam != pParameters.end())
        currParam.name = itValueParam->second;
    }
    else
    {
      currParam.fillParameters(pParameters);
    }
  }
}



std::string Fact::toStr() const
{
  std::string res = name;
  if (!parameters.empty())
  {
    res += "(";
    _parametersToStr(res, parameters);
    res += ")";
  }
  if (!value.empty())
    res += "=" + value;
  return res;
}

std::size_t Fact::fillFactFromStr(
    const std::string& pStr,
    std::size_t pBeginPos,
    char pSeparator)
{
  std::size_t pos = pBeginPos;
  while (pos < pStr.size())
  {
    if (pStr[pos] == ' ')
    {
      ++pos;
      continue;
    }
    if (pStr[pos] == pSeparator || pStr[pos] == ')')
      return pos;

    bool insideParenthesis = false;
    auto beginPos = pos;
    while (pos < pStr.size())
    {
      if (!insideParenthesis && (pStr[pos] == pSeparator || pStr[pos] == ' ' || pStr[pos] == ')'))
        break;
      if (pStr[pos] == '(' || pStr[pos] == pSeparator)
      {
        insideParenthesis = true;
        if (name.empty())
          name = pStr.substr(beginPos, pos - beginPos);
        parameters.emplace_back();
        ++pos;
        pos = parameters.back().fillFactFromStr(pStr, pos, ',');
        beginPos = pos;
        continue;
      }
      if (pStr[pos] == ')' || pStr[pos] == '=')
      {
        insideParenthesis = false;
        if (name.empty())
          name = pStr.substr(beginPos, pos - beginPos);
        ++pos;
        beginPos = pos;
        continue;
      }
      ++pos;
    }
    if (name.empty())
      name = pStr.substr(beginPos, pos - beginPos);
    else
      value = pStr.substr(beginPos, pos - beginPos);
  }
  return pos;
}

Fact Fact::fromStr(const std::string& pStr)
{
  Fact res;
  auto endPos = res.fillFactFromStr(pStr, 0, ',');
  assert(!res.name.empty());
  assert(endPos == pStr.size());
  return res;
}


bool Fact::replaceParametersByAny(const std::vector<std::string>& pParameters)
{
  bool res = false;
  for (const auto& currParam : pParameters)
  {
    for (auto& currFactParam : parameters)
    {
      if (currFactParam == currParam)
      {
        currFactParam = anyValue;
        res = true;
      }
    }
    if (value == currParam)
    {
      value = anyValue;
      res = true;
    }
  }
  return res;
}


void Fact::_parametersToStr(std::string& pStr,
                            const std::vector<Fact>& pParameters)
{
  bool firstIteration = true;
  for (auto& param : pParameters)
  {
    if (firstIteration)
      firstIteration = false;
    else
      pStr += ", ";
    pStr += param.toStr();
  }
}


} // !cp
