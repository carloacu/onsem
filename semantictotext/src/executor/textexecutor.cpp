#include <onsem/semantictotext/executor/textexecutor.hpp>
#include <onsem/semantictotext/semanticconverter.hpp>
#include "../conversion/mandatoryformconverter.hpp"


namespace onsem
{
namespace
{

void _paramSemExpsToToParamStr(
    std::map<std::string, std::vector<std::string>>& pParameters,
    const std::map<std::string, std::vector<UniqueSemanticExpression>>& pParametersToSemExps,
    const linguistics::LinguisticDatabase& pLingDb,
    SemanticLanguageEnum pLanguage)
{
  for (auto& currParametersToSemExps : pParametersToSemExps)
  {
    auto& semExps = currParametersToSemExps.second;
    if (!semExps.empty())
    {
      auto& strs = pParameters[currParametersToSemExps.first];
      TextProcessingContext outContext(SemanticAgentGrounding::currentUser,
                                       SemanticAgentGrounding::me,
                                       pLanguage);
      SemanticMemory semMemory;

      for (auto& currAnswer : semExps)
      {
        std::string subRes;
        converter::semExpToText(subRes, currAnswer->clone(), outContext,
                                true, semMemory, pLingDb, nullptr);
        strs.push_back(subRes);
      }
    }
  }
}

}


TextExecutor::TextExecutor(SemanticMemory& pSemanticMemory,
                           const linguistics::LinguisticDatabase& pLingDb,
                           VirtualExecutorLogger& pLogOnSynchronousExecutionCase)
  : VirtualExecutor(SemanticSourceEnum::ASR, &pLogOnSynchronousExecutionCase),
    _semanticMemory(pSemanticMemory),
    _lingDb(pLingDb)
{
}

void TextExecutor::_synchronicityWrapper(std::function<void()> pFunction)
{
  pFunction();
}

void TextExecutor::_usageOfMemory(std::function<void(SemanticMemory&)> pFunction)
{
  pFunction(_semanticMemory);
}

void TextExecutor::_usageOfMemblock(std::function<void(const SemanticMemoryBlock&, const std::string&)> pFunction)
{
  pFunction(_semanticMemory.memBloc, _semanticMemory.getCurrUserId());
}

void TextExecutor::_usageOfLingDb(std::function<void(const linguistics::LinguisticDatabase&)> pFunction)
{
  pFunction(_lingDb);
}


FutureVoid TextExecutor::_exposeResource(const SemanticResource& pResource,
                                         const SemanticExpression* pInputSemExpPtr,
                                         const FutureVoid&)
{
  std::map<std::string, std::vector<std::string>> parameters;
  if (!pResource.parameterLabelsToQuestions.empty() && pInputSemExpPtr != nullptr)
  {
    std::map<std::string, std::vector<UniqueSemanticExpression>> parametersToSemExps;
    UniqueSemanticExpression clonedInput = pInputSemExpPtr->clone();
    mandatoryFormConverter::process(clonedInput);
    converter::extractParameters(parametersToSemExps,
                                 pResource.parameterLabelsToQuestions,
                                 std::move(clonedInput), _lingDb);
    _paramSemExpsToToParamStr(parameters, parametersToSemExps, _lingDb, pResource.language);
  }

  _paramSemExpsToToParamStr(parameters, pResource.parametersLabelsToValue, _lingDb, pResource.language);
  _addLogAutoResource(pResource, parameters);
  return FutureVoid();
}


} // End of namespace onsem
