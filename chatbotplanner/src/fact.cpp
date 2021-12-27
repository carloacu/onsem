#include <onsem/chatbotplanner/fact.hpp>
#include <assert.h>

namespace onsem
{
namespace cp
{


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
  _parametersToStr(paramStr);
  std::string otherParamStr;
  _parametersToStr(otherParamStr);
  return paramStr < otherParamStr;
}

bool Fact::operator==(const Fact& pOther) const
{
  return name == pOther.name && value == pOther.value && parameters == pOther.parameters;
}

std::string Fact::toStr() const
{
  std::string res = name;
  if (!parameters.empty())
  {
    res += "(";
    _parametersToStr(res);
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


void Fact::_parametersToStr(std::string& pStr) const
{
  bool firstIteration = true;
  for (auto& param : parameters)
  {
    if (firstIteration)
      firstIteration = false;
    else
      pStr += ", ";
    pStr += param.toStr();
  }
}


} // !cp
} // !onsem
