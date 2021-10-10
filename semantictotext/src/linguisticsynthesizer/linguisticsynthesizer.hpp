#ifndef ONSEM_SEMANTICTOTEXT_LINGUISTICSYNTHESIZER_LINGUISTICSYNTHESIZER_HPP
#define ONSEM_SEMANTICTOTEXT_LINGUISTICSYNTHESIZER_LINGUISTICSYNTHESIZER_HPP

#include <string>
#include <list>
#include "synthesizerresulttypes.hpp"

namespace onsem
{
namespace linguistics
{
struct LinguisticDatabase;
}
struct UniqueSemanticExpression;
struct SemanticMemoryBlock;
struct TextProcessingContext;
struct CommandExpression;
struct SemLineToPrint;


void synthesize(std::list<std::unique_ptr<SynthesizerResult>>& pNaturalLanguageResult,
                UniqueSemanticExpression pSemExp,
                bool pOneLinePerSentence,
                const SemanticMemoryBlock& pMemBlock,
                const std::string& pCurrentUserId,
                const TextProcessingContext& pTextProcContext,
                const linguistics::LinguisticDatabase& pLingDb,
                std::list<std::list<SemLineToPrint> >* pDebugOutput);

void writeCommand(std::string& pRes,
                  const CommandExpression& pCommandExp);


} // End of namespace onsem

#endif // ONSEM_SEMANTICTOTEXT_LINGUISTICSYNTHESIZER_LINGUISTICSYNTHESIZER_HPP
