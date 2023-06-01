#ifndef ONSEM_SEMANTICTOTEXT_OUTPUTTER_TEXTOUTPUTTER_HPP
#define ONSEM_SEMANTICTOTEXT_OUTPUTTER_TEXTOUTPUTTER_HPP

#include "virtualoutputter.hpp"
#include "../api.hpp"
#include <onsem/semantictotext/semanticmemory/semanticmemory.hpp>


namespace onsem
{

struct ONSEMSEMANTICTOTEXT_API TextOutputter : public VirtualOutputter
{
  TextOutputter(SemanticMemory& pSemanticMemory,
                const linguistics::LinguisticDatabase& pLingDb,
                VirtualOutputterLogger& pLogOnSynchronousExecutionCase);
  virtual ~TextOutputter() {}
};

} // End of namespace onsem

#endif // ONSEM_SEMANTICTOTEXT_OUTPUTTER_TEXTOUTPUTTER_HPP

