#include <onsem/semantictotext/outputter/outputterlogger.hpp>

namespace onsem
{

namespace
{
std::string _parameterToStr(const std::map<std::string, std::vector<std::string>>& pParameters)
{
  std::string res;
  for (const auto& currParam : pParameters)
  {
    if (!res.empty())
      res += ", ";
    res += currParam.first + "=";
    bool firstIteration = true;
    for (const auto& currValue : currParam.second)
    {
      if (firstIteration)
        firstIteration = false;
      else
        res += "|";
      res += currValue;
    }
  }
  return res;
}

}

void DefaultOutputterLogger::onAutoResource(
    const SemanticResource& pResource,
    const std::map<std::string, std::vector<std::string>>& pParameters)
{
  std::string log;
  log = "\\" +  pResource.toStr();
  if (!pParameters.empty())
    log += "(" + _parameterToStr(pParameters) + ")";
  log += "\\";
  _addTextToLog(log);
}


void OutputterLoggerWithoutMetaInformation::onAutoResource(
    const SemanticResource& pResource,
    const std::map<std::string, std::vector<std::string>>& pParameters)
{
  if (!_logsStr.empty())
    _logsStr += " ";
  _logsStr += "\\" + pResource.toStr();
  if (!pParameters.empty())
    _logsStr += "(" + _parameterToStr(pParameters) + ")";
  _logsStr += "\\";
}


} // End of namespace onsem
