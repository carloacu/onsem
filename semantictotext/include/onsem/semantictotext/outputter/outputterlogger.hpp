#ifndef ONSEM_SEMANTICTOTEXT_OUTPUTTER_OUTPUTTERLOGGER_HPP
#define ONSEM_SEMANTICTOTEXT_OUTPUTTER_OUTPUTTERLOGGER_HPP

#include <string>
#include "../api.hpp"
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticresourcegrounding.hpp>


namespace onsem
{
struct SemanticResourceGrounding;


/// Mother class to log the outputter.
class ONSEMSEMANTICTOTEXT_API VirtualOutputterLogger
{
public:
  virtual ~VirtualOutputterLogger() {}
  /**
   * @brief onMetaInformation Print Link between elements like "AND", "OR", "THEN", ...
   * @param pLog Value to print
   */
  virtual void onMetaInformation(const std::string& pLog) = 0;

  /// Print the begin of a scope for list of elements.
  virtual void onMetaInformation_BeginOfScope() = 0;

  /// Print the end of a scope for list of elements.
  virtual void onMetaInformation_EndOfScope() = 0;

  /**
   * @brief onAutoCommand Print a command (it will soon be converted to the print of any resource)
   * @param pResource Resource to print.
   * @param pParameters Resolved parameters to print.
   */
  virtual void onAutoResource(const SemanticResource& pResource,
                              const std::map<std::string, std::vector<std::string>>& pParameters) = 0;

  /**
   * @brief Print a text that outputter expose.
   * @param pLog Value to print.
   */
  virtual void onAutoSaidText(const std::string& pLog) = 0;
};


/// Default implementation of an outputter logger class.
class ONSEMSEMANTICTOTEXT_API DefaultOutputterLogger : public VirtualOutputterLogger
{
public:
  DefaultOutputterLogger(std::string& pLogsStr)
    : VirtualOutputterLogger(),
      _logsStr(pLogsStr)
  {
  }

  void onMetaInformation(const std::string& pLog) override { _addTextToLog(pLog); }
  void onMetaInformation_BeginOfScope() override { _addTextToLog("("); }
  void onMetaInformation_EndOfScope() override { _addTextToLog(")"); }
  void onAutoResource(const SemanticResource& pResource,
                      const std::map<std::string, std::vector<std::string>>& pParameters) override;
  void onAutoSaidText(const std::string& pLog) override { _addTextToLog(pLog); }

private:
  std::string& _logsStr;

  void _addTextToLog(const std::string& pText) { _logsStr += (_logsStr.empty() || _logsStr.back() == ' ') ? pText : ("\t" + pText); }
};



class ONSEMSEMANTICTOTEXT_API OutputterLoggerWithoutMetaInformation : public VirtualOutputterLogger
{
public:
  OutputterLoggerWithoutMetaInformation(std::string& pLogsStr)
    : VirtualOutputterLogger(),
      _logsStr(pLogsStr)
  {
  }

  void onMetaInformation(const std::string&) override {}
  void onMetaInformation_BeginOfScope() override {}
  void onMetaInformation_EndOfScope() override {}
  void onAutoResource(const SemanticResource& pResource,
                      const std::map<std::string, std::vector<std::string>>& pParameters) override;
  void onAutoSaidText(const std::string& pLog) override {
    if (!_logsStr.empty())
      _logsStr += " ";
    _logsStr += pLog;
  }

private:
  std::string& _logsStr;
};


} // End of namespace onsem


#endif // ONSEM_SEMANTICTOTEXT_OUTPUTTER_OUTPUTTERLOGGER_HPP
