#ifndef SEMANTICREASONERTESTER_SRC_DEBUG_TEXTANALYZEDEBUGGER_HPP
#define SEMANTICREASONERTESTER_SRC_DEBUG_TEXTANALYZEDEBUGGER_HPP

#include <string>
#include <list>
#include <map>
#include <memory>
#include <onsem/common/enum/semanticlanguageenum.hpp>
#include <onsem/texttosemantic/type/debug/synthanalendingstepfordebug.hpp>
#include <onsem/texttosemantic/dbtype/misc/spellingmistaketype.hpp>
#include <onsem/semanticdebugger/printer/semexplinestostr.hpp>
#include <onsem/semanticdebugger/semanticdebug.hpp>
#include <onsem/semanticdebugger/syntacticgraphresult.hpp>
#include "api.hpp"


namespace onsem
{
struct TextProcessingContext;
class SemExpLinesToStr;
namespace linguistics
{
namespace TextAnalyzeDebugger
{

ONSEMSEMANTICDEBUGGER_API
void saveGramPossibilities(std::list<std::list<std::string>>& pGramPossibilitiesForEachToken,
                           const TokensTree& pTokensTree);

ONSEMSEMANTICDEBUGGER_API
void fillSemAnalResult(SyntacticGraphResult& pResults,
                       SemanticAnalysisHighLevelResults& pHighLevelResults,
                       const std::set<SpellingMistakeType>& pSpellingMistakeTypesPossible,
                       const SemanticAnalysisDebugOptions& pSemanticAnalysisDebugOptions);


} // End of namespace TextAnalyzeDebugger
} // End of namespace linguistics
} // End of namespace onsem


#endif // SEMANTICREASONERTESTER_SRC_DEBUG_TEXTANALYZEDEBUGGER_HPP
