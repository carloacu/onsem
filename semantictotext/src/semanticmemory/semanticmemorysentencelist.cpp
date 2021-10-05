#include <onsem/semantictotext/semanticmemory/semanticmemorysentencelist.hpp>

namespace onsem
{

void SemanticMemorySentenceList::setEnabled(bool pEnabled)
{
  for (SemanticMemorySentence& currElt : elts)
    currElt.setEnabled(pEnabled);
}

void SemanticMemorySentenceList::clear()
{
  auto itElt = elts.begin();
  while (itElt != elts.end())
  {
    SemanticMemorySentence& memSen = *itElt;
    memSen.setEnabled(false);
    itElt = elts.erase(itElt);
  }
}


std::string SemanticMemorySentenceList::getName(const std::string& pUserId) const
{
  if (elts.size() == 1)
    return elts.front().getName(pUserId);
  if (and_or)
  {
    for (const auto& currElt : elts)
    {
      const std::string name = currElt.getName(pUserId);
      if (!name.empty())
        return name;
    }
  }
  return "";
}


bool SemanticMemorySentenceList::hasEquivalentUserIds(const std::string& pUserId) const
{
  if (and_or)
    for (const auto& currElt : elts)
      if (currElt.hasEquivalentUserIds(pUserId))
        return true;
  return false;
}

intSemId SemanticMemorySentenceList::getIdOfFirstSentence() const
{
  for (const auto& currElt : elts)
    return currElt.id;
  return 0;
}


} // End of namespace onsem
