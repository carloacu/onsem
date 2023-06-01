#include <onsem/semantictotext/outputter/executiondataoutputter.hpp>


namespace onsem
{


ExecutionDataOutputter::ExecutionDataOutputter(
    SemanticMemory& pSemanticMemory,
     const linguistics::LinguisticDatabase& pLingDb,
    VirtualOutputterLogger& pLogOnSynchronousExecutionCase)
  : VirtualOutputter(pSemanticMemory, pLingDb, SemanticSourceEnum::ASR, &pLogOnSynchronousExecutionCase),
    rootExecutionData(),
    _currentExecutionDataPtr(&rootExecutionData)
{
}


void ExecutionDataOutputter::_exposeResource(
    const SemanticResource& pResource,
    const std::map<std::string, std::vector<std::string>>& pParameters)
{
}

void ExecutionDataOutputter::_exposeText(
    const std::string& pText,
    SemanticLanguageEnum pLanguage)
{
  _currentExecutionDataPtr->text = pText;
  _currentExecutionDataPtr->textLanguage = pLanguage;
}

void ExecutionDataOutputter::_beginOfScope(Link pLink)
{
}

void ExecutionDataOutputter::_insideScopeLink(Link pLink)
{
}

void ExecutionDataOutputter::_insideScopeRepetition(int pNumberOfRepetitions)
{
}

void ExecutionDataOutputter::_endOfScope()
{
}

} // End of namespace onsem
