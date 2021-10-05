#ifndef ONSEM_SEMANTICTOTEXT_SEMANTICMEMORY_REFERENCESGETTER_HPP
#define ONSEM_SEMANTICTOTEXT_SEMANTICMEMORY_REFERENCESGETTER_HPP

#include <list>
#include <string>
#include "../api.hpp"


namespace onsem
{

struct ONSEMSEMANTICTOTEXT_API ReferencesGetter
{
  virtual ~ReferencesGetter() {}
  virtual void getReferences(std::list<std::string>& pReferences) const = 0;
};



} // End of namespace onsem


#endif // ONSEM_SEMANTICTOTEXT_SEMANTICMEMORY_REFERENCESGETTER_HPP
