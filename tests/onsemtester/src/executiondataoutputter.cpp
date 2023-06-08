#include <onsem/tester/executiondataoutputter.hpp>
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

bool ExecutionData::hasChildren() const
{
  return !toRunInBackground.empty() || !toRunInParallel.empty() || !toRunSequencially.empty();
}

void ExecutionData::setResourceNbOfTimes(int pNumberOfTimes)
{
  if (hasData())
  {
    resourceNbOfTimes = pNumberOfTimes;
    return;
  }
  if (toRunInBackground.size())
  {
    toRunInBackground.back().setResourceNbOfTimes(pNumberOfTimes);
    return;
  }
  if (toRunInParallel.size())
  {
    toRunInParallel.back().setResourceNbOfTimes(pNumberOfTimes);
    return;
  }
  if (toRunSequencially.size())
  {
    toRunSequencially.back().setResourceNbOfTimes(pNumberOfTimes);
    return;
  }
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
                               const linguistics::LinguisticDatabase& pLingDb,
                               bool pHasAlreadyData) const
{
  if (punctualAssertion)
  {
    memoryOperation::notifyPunctually(**punctualAssertion, InformationType::ASSERTION,
                                      pSemanticMemory, pLingDb);
  }

  std::string dataStr = _dataToStr();
  std::string res = dataStr;

  if (resourceNbOfTimes > 1)
  {
    auto newRes = res;
    std::stringstream ss;
    ss << resourceNbOfTimes;
    newRes += "\tNUMBER_OF_TIMES: " + ss.str();
    if (pHasAlreadyData || !toRunInParallel.empty() || !toRunSequencially.empty() || !toRunInBackground.empty())
      res = "(\t" + newRes + "\t)";
    else
      res = newRes;
  }

  auto printList = [&] (const std::list<ExecutionData>& pListToPrint,
                        const std::string& pSeparator)
  {
    if (!pListToPrint.empty())
    {
      auto newRes = res;
      for (auto& currElt : pListToPrint)
      {
        if (currElt.hasData() || currElt.hasChildren())
        {
          if (!newRes.empty())
            newRes += "\t" + pSeparator + "\t";
          newRes += currElt.run(pSemanticMemory, pLingDb, !newRes.empty() || pListToPrint.size() > 1);
        }
      }
      if (pHasAlreadyData || res != dataStr || !toRunInBackground.empty())
        res = "(\t" + newRes + "\t)";
      else
        res = newRes;
    }
  };
  printList(toRunInParallel, "AND");
  printList(toRunSequencially, "THEN");

  if (!toRunInBackground.empty())
  {
    auto newRes = res;
    for (auto& currElt : toRunInBackground)
    {
      newRes += "\tIN_BACKGROUND: " +
          currElt.run(pSemanticMemory, pLingDb, !newRes.empty() || toRunInBackground.size() > 1);
    }
    if (pHasAlreadyData || res != dataStr)
      res = "(\t" + newRes + "\t)";
    else
      res = newRes;
  }

  if (numberOfTimes > 1)
  {
    res = "(\t" + res;
    std::stringstream ss;
    ss << numberOfTimes;
    res += "\tNUMBER_OF_TIMES: " + ss.str();
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
    _stack(1, LinkAndExecutionData(Link::THEN, rootExecutionData))
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

void ExecutionDataOutputter::_assertPunctually(const SemanticExpression& pSemExp)
{
  _getLasExectInStack().punctualAssertion =
      std::make_unique<UniqueSemanticExpression>(pSemExp.clone());
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
  if (!_stack.empty())
  {
    auto childLink = pLink == Link::IN_BACKGROUND ? pLink : _stack.back().link;
    ExecutionData& currElt = _getLasExectInStack();
    auto& childList  = currElt.linkToChildList(childLink);
    childList.emplace_back();
    auto& newElt = childList.back();
    _stack.emplace_back(pLink, newElt);
  }
  else
  {
    assert(false);
  }
}

void ExecutionDataOutputter::_endOfScope()
{
  if (_stack.size() > 1)
    _stack.pop_back();
  else
    assert(false);
}

void ExecutionDataOutputter::_resourceNbOfTimes(int pNumberOfTimes)
{
  _getLasExectInStack().setResourceNbOfTimes(pNumberOfTimes);
}

void ExecutionDataOutputter::_insideScopeNbOfTimes(int pNumberOfTimes)
{
  _getLasExectInStack().numberOfTimes = pNumberOfTimes;
}

ExecutionData& ExecutionDataOutputter::_getOrCreateNewElt()
{
  ExecutionData& newElt = _getLasExectInStack();
  if (newElt.hasData() || newElt.hasChildren())
  {
    if (!_stack.empty())
    {
      auto& childList  = newElt.linkToChildList(_stack.back().link);
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
  if (!_stack.empty())
    return _stack.back().executionData;
  assert(false);
  return rootExecutionData;
}


} // End of namespace onsem
