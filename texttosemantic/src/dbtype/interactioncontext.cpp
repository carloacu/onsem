#include <onsem/texttosemantic/dbtype/interactioncontext.hpp>

namespace onsem
{


InteractionContext InteractionContext::clone() const
{
  InteractionContext res;
  res.textToSay = textToSay->clone();
  if (fallbackTextToSay)
    res.fallbackTextToSay = std::make_unique<UniqueSemanticExpression>(fallbackTextToSay->getSemExp().clone());
  for (auto& currAnswPoss : answerPossibilities)
    res.answerPossibilities.emplace_back(currAnswPoss.first->clone(), currAnswPoss.second);
  return res;
}

int InteractionContextContainer::addInteractionContext(InteractionContext pInteractionContext)
{
  int newId = 0;
  for (const auto& currIntContext : _interactionContexts)
  {
    if (currIntContext.first != newId)
      break;
    ++newId;
  }
  _interactionContexts.emplace(newId, std::move(pInteractionContext));
  return newId;
}


const InteractionContext* InteractionContextContainer::getCurrentInteractionContextPtr() const
{
  if (!currentPosition)
    return nullptr;
  return getInteractionContextPtr(*currentPosition);
}


const InteractionContext* InteractionContextContainer::getInteractionContextPtr(int pId) const
{
  auto it = _interactionContexts.find(pId);
  if (it != _interactionContexts.end())
    return &it->second;
  return nullptr;
}


InteractionContext* InteractionContextContainer::getInteractionContextPtr(int pId)
{
  auto it = _interactionContexts.find(pId);
  if (it != _interactionContexts.end())
    return &it->second;
  return nullptr;
}


std::unique_ptr<InteractionContextContainer> InteractionContextContainer::clone() const
{
  auto res = std::make_unique<InteractionContextContainer>();
  res->currentPosition = currentPosition;
  for (auto& currElt : _interactionContexts)
    res->_interactionContexts.emplace(currElt.first, currElt.second.clone());
  return res;
}



} // End of namespace onsem


