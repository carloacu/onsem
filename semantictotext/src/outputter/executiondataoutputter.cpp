#include <onsem/semantictotext/outputter/executiondataoutputter.hpp>


namespace onsem
{

bool ExecutionData::hasData() const
{
  return !text.empty() || resource;
}

std::list<ExecutionData>& ExecutionData::linkToChildList(VirtualOutputter::Link pLink)
{
  switch (pLink) {
  case VirtualOutputter::Link::AND:
    return toRunInParallel;
  case VirtualOutputter::Link::IN_BACKGROUND:
    return toRunInBackground;
  default:
    return toRunSequencially;
  }
}


ExecutionDataOutputter::ExecutionDataOutputter(
    SemanticMemory& pSemanticMemory,
     const linguistics::LinguisticDatabase& pLingDb,
    VirtualOutputterLogger& pLogOnSynchronousExecutionCase)
  : VirtualOutputter(pSemanticMemory, pLingDb, SemanticSourceEnum::ASR, &pLogOnSynchronousExecutionCase),
    rootExecutionData(),
    _currentLink(Link::THEN),
    _executionDataStack(1, &rootExecutionData)
{
}


void ExecutionDataOutputter::_exposeResource(
    const SemanticResource& pResource,
    const std::map<std::string, std::vector<std::string>>& pParameters)
{
  auto& newElt = _getOrCreateNewElt();
  newElt.resource = std::make_unique<SemanticResource>(pResource);
  newElt.resourceParameters = pParameters;
}

void ExecutionDataOutputter::_exposeText(
    const std::string& pText,
    SemanticLanguageEnum pLanguage)
{
  auto& newElt = _getOrCreateNewElt();
  newElt.text = pText;
  newElt.textLanguage = pLanguage;
}

void ExecutionDataOutputter::_beginOfScope(Link pLink)
{
  _currentLink = pLink;
  auto& childList  = _executionDataStack.back()->linkToChildList(_currentLink);
  childList.emplace_back();
  auto& newElt = childList.back();
  _executionDataStack.push_back(&newElt);
}

void ExecutionDataOutputter::_endOfScope()
{
  assert(!_executionDataStack.empty());
  _executionDataStack.pop_back();
}


void ExecutionDataOutputter::_insideScopeRepetition(int pNumberOfRepetitions)
{
  _executionDataStack.back()->numberOfRepetitions = pNumberOfRepetitions;
}

ExecutionData& ExecutionDataOutputter::_getOrCreateNewElt()
{
  ExecutionData* newEltPtr = _executionDataStack.back();
  if (newEltPtr->hasData())
  {
    auto& childList  = newEltPtr->linkToChildList(_currentLink);
    childList.emplace_back();
    return childList.back();
  }
  return *newEltPtr;
}

} // End of namespace onsem
