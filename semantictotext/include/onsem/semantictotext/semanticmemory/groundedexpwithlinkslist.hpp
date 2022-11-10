#ifndef ONSEM_SEMANTICTOTEXT_SEMANTICMEMORY_GROUNDEDEXPWITHLINKSLIST_HPP
#define ONSEM_SEMANTICTOTEXT_SEMANTICMEMORY_GROUNDEDEXPWITHLINKSLIST_HPP

#include <list>
#include "../api.hpp"
#include "groundedexpwithlinks.hpp"


namespace onsem
{


struct ONSEMSEMANTICTOTEXT_API GroundedExpWithLinksList
{
  void setEnabled(bool pEnabled);
  void clear();
  std::string getName(const std::string& pUserId) const;
  bool hasEquivalentUserIds(const std::string& pUserId) const;
  intSemId getIdOfFirstSentence() const;

  bool and_or{true};
  std::list<GroundedExpWithLinks> elts{};
};



} // End of namespace onsem


#endif // ONSEM_SEMANTICTOTEXT_SEMANTICMEMORY_GROUNDEDEXPWITHLINKSLIST_HPP
