#ifndef ALSEMANTICANALYZER_ALSEMANTICPRINTER_H
#define ALSEMANTICANALYZER_ALSEMANTICPRINTER_H

#include <memory>
#include "../api.hpp"
#include <onsem/common/enum/semanticsourceenum.hpp>

namespace onsem
{
struct SemanticExpression;
struct SemanticAgentGrounding;
class ConceptSet;


namespace SemanticPrinter
{


ONSEMSEMANTICDEBUGGER_API
void printSentiments(std::string& pRes,
                     const SemanticExpression& pSemExp,
                     const std::string& pCurrentUserId,
                     const ConceptSet& pConceptSet,
                     std::unique_ptr<SemanticAgentGrounding> pAuthorPtr,
                     SemanticSourceEnum pFrom = SemanticSourceEnum::UNKNOWN);



} // End of namespace SemanticPrinter


} // End of namespace onsem

#endif // ALSEMEXPDATABASES_PRINTSEMEXPDIFFS_H
