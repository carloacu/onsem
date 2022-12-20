#include <onsem/semantictotext/executor/textexecutor.hpp>
#include <onsem/texttosemantic/tool/semexpmodifier.hpp>
#include <onsem/semantictotext/semexpoperators.hpp>
#include <onsem/semantictotext/semanticconverter.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/groundedexpression.hpp>

namespace onsem
{

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
  if (!pResource.parameterLabelsToQuestions.empty())
  {
    _extractParameters(parameters, pResource.parameterLabelsToQuestions,
                       pResource.language, pInputSemExpPtr);
  }
  _addLogAutoResource(pResource, parameters);
  return FutureVoid();
}


void TextExecutor::_extractParameters(
    std::map<std::string, std::vector<std::string>>& pParameters,
    const std::map<std::string, std::list<UniqueSemanticExpression>>& pParameterLabelsToQuestions,
    SemanticLanguageEnum pLanguage,
    const SemanticExpression* pInputSemExpPtr) const
{
  if (pInputSemExpPtr != nullptr)
  {
    auto clonedInput = pInputSemExpPtr->clone();
    SemExpModifier::imperativeToMandatoryForm(*clonedInput);
    SemanticMemory semMemory;
    memoryOperation::inform(std::move(clonedInput), semMemory, _lingDb);

    for (const auto& currParam : pParameterLabelsToQuestions)
    {
      for (const auto& currQuestion : currParam.second)
      {
        std::vector<std::unique_ptr<GroundedExpression>> answers;
        memoryOperation::get(answers, currQuestion->clone(), semMemory, _lingDb);
        if (!answers.empty())
        {
          TextProcessingContext outContext(SemanticAgentGrounding::currentUser,
                                           SemanticAgentGrounding::me,
                                           pLanguage);
          outContext.rawValue = true;
          std::vector<std::string> res(answers.size());
          std::size_t i = 0;
          for (auto& currAnswer : answers)
          {
            std::string subRes;
            converter::semExpToText(subRes, std::move(currAnswer), outContext,
                                    true, semMemory, _lingDb, nullptr);
            res[i++] = subRes;
          }
          pParameters.emplace(currParam.first, std::move(res));
          break;
        }
      }
    }
  }
}

} // End of namespace onsem
