#ifndef SEMANTICREASONERTESTER_SYNTACTICGRAPHRESULT_HPP
#define SEMANTICREASONERTESTER_SYNTACTICGRAPHRESULT_HPP

#include <string>
#include <onsem/common/enum/semanticlanguageenum.hpp>
#include <onsem/texttosemantic/type/syntacticgraph.hpp>
#include "api.hpp"

namespace onsem {

struct ONSEMSEMANTICDEBUGGER_API SyntacticGraphResult {
    SyntacticGraphResult(const linguistics::LinguisticDatabase& pLingDb, SemanticLanguageEnum pLanguage)
        : inputText()
        , syntGraph(pLingDb, pLanguage) {}

    std::string inputText;
    linguistics::SyntacticGraph syntGraph;
};

}    // End of namespace onsem

#endif    // !SEMANTICREASONERTESTER_SYNTACTICGRAPHRESULT_HPP
