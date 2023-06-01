#include <onsem/semantictotext/outputter/executiondataoutputter.hpp>
#include <sstream>
#include <onsem/semantictotext/semexpoperators.hpp>


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


std::string ExecutionData::run(SemanticMemory& pSemanticMemory,
                               const linguistics::LinguisticDatabase& pLingDb)
{
  if (punctualAssertion)
  {
    memoryOperation::notifyPunctually(**punctualAssertion, InformationType::ASSERTION,
                                      pSemanticMemory, pLingDb);
  }

  std::string res = _dataToStr();

  if (numberOfRepetitions > 1)
  {
    res = "(\t" + res;
    std::stringstream ss;
    ss << numberOfRepetitions;
    res += "\tNUMBER_OF_TIMES: " + ss.str();
    res += "\t)";
  }

  if (!toRunInParallel.empty())
  {
    res = "(\t" + res;
    for (auto& currElt : toRunInParallel)
      res += "\tAND\t" + currElt.run(pSemanticMemory, pLingDb);
    res += "\t)";
  }

  if (!toRunSequencially.empty())
  {
    res = "(\t" + res;
    for (auto& currElt : toRunSequencially)
      res += "\tTHEN\t" + currElt.run(pSemanticMemory, pLingDb);
    res += "\t)";
  }

  if (!toRunInBackground.empty())
  {
    res = "(\t" + res;
    for (auto& currElt : toRunInBackground)
      res += "\tIN_BACKGROUND\t" + currElt.run(pSemanticMemory, pLingDb);
    res += "\t)";
  }

  if (permanentAssertion)
  {
    memoryOperation::informAxiom(std::move(*permanentAssertion),
                                 pSemanticMemory, pLingDb);
  }
  if (informationToTeach)
  {
    mystd::unique_propagate_const<UniqueSemanticExpression> reaction;
    memoryOperation::teach(reaction, pSemanticMemory, std::move(*informationToTeach), pLingDb,
                           memoryOperation::SemanticActionOperatorEnum::INFORMATION);
  }
  return res;
}


std::string ExecutionData::_dataToStr() const
{
  if (!text.empty())
    return text;

  if (resource)
  {
    std::string res;
    res = "\\" +  resource->toStr();
    if (!resourceParameters.empty())
      res += "(" + _parameterToStr(resourceParameters) + ")";
    res += "\\";
    return res;
  }

  return "";
}

ExecutionDataOutputter::ExecutionDataOutputter(SemanticMemory& pSemanticMemory,
     const linguistics::LinguisticDatabase& pLingDb)
  : VirtualOutputter(pSemanticMemory, pLingDb, SemanticSourceEnum::ASR),
    rootExecutionData(),
    _linksStack(1, Link::THEN),
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

void ExecutionDataOutputter::_assertPunctually(UniqueSemanticExpression pUSemExp)
{
  _getLasExectInStack().punctualAssertion =
      std::make_unique<UniqueSemanticExpression>(std::move(pUSemExp));
}

void ExecutionDataOutputter::_teachInformation(UniqueSemanticExpression pUSemExp)
{
  _getLasExectInStack().informationToTeach =
      std::make_unique<UniqueSemanticExpression>(std::move(pUSemExp));
}

void ExecutionDataOutputter::_assertPermanently(UniqueSemanticExpression pUSemExp)
{
  _getLasExectInStack().permanentAssertion =
      std::make_unique<UniqueSemanticExpression>(std::move(pUSemExp));
}


void ExecutionDataOutputter::_beginOfScope(Link pLink)
{
  if (!_linksStack.empty())
  {
    auto currentLink = _linksStack.back();
    _linksStack.push_back(pLink);
    if (!_executionDataStack.empty())
    {
      ExecutionData& currElt = _getLasExectInStack();
      if (currElt.hasData())
      {
        auto& childList  = currElt.linkToChildList(currentLink);
        childList.emplace_back();
        auto& newElt = childList.back();
        _executionDataStack.push_back(&newElt);
      }
      else
      {
        _executionDataStack.push_back(nullptr);
      }
    }
    else
    {
      assert(false);
    }
  }
  else
  {
    assert(false);
  }
}

void ExecutionDataOutputter::_endOfScope()
{
  if (_linksStack.size() > 1)
    _linksStack.pop_back();
  else
    assert(false);
  if (_executionDataStack.size() > 1)
    _executionDataStack.pop_back();
  else
    assert(false);
}


void ExecutionDataOutputter::_insideScopeRepetition(int pNumberOfRepetitions)
{
  _getLasExectInStack().numberOfRepetitions = pNumberOfRepetitions;
}

ExecutionData& ExecutionDataOutputter::_getOrCreateNewElt()
{
  ExecutionData& newElt = _getLasExectInStack();
  if (newElt.hasData())
  {
    if (!_linksStack.empty())
    {
      auto& childList  = newElt.linkToChildList(_linksStack.back());
      childList.emplace_back();
      return childList.back();
    }
    else
    {
      assert(false);
    }
  }
  return newElt;
}

ExecutionData& ExecutionDataOutputter::_getLasExectInStack()
{
  for (auto it = _executionDataStack.rbegin(); it != _executionDataStack.rend(); ++it)
  {
    ExecutionData* executionDataPtr = *it;
    if (executionDataPtr != nullptr)
      return *executionDataPtr;
  }
  assert(false);
  return rootExecutionData;
}


} // End of namespace onsem
