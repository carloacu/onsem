#include <onsem/semantictotext/outputter/textoutputter.hpp>


namespace onsem
{


TextOutputter::TextOutputter(
    SemanticMemory& pSemanticMemory,
     const linguistics::LinguisticDatabase& pLingDb,
    VirtualOutputterLogger& pLogOnSynchronousExecutionCase)
  : VirtualOutputter(pSemanticMemory, pLingDb, SemanticSourceEnum::ASR, &pLogOnSynchronousExecutionCase)
{
}

} // End of namespace onsem
