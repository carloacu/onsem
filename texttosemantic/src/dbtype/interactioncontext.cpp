#include <onsem/texttosemantic/dbtype/interactioncontext.hpp>

namespace onsem
{


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




} // End of namespace onsem


