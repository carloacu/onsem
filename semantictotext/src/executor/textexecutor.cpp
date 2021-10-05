#include <onsem/semantictotext/executor/textexecutor.hpp>



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


} // End of namespace onsem
