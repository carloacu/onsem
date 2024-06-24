#include "../semanticreasonergtests.hpp"
#include <gtest/gtest.h>
#include <onsem/common/binary/binarysaver.hpp>
#include <onsem/texttosemantic/dbtype/linguisticanalysisconfig.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase.hpp>
#include <onsem/texttosemantic/type/syntacticgraph.hpp>
#include <onsem/texttosemantic/type/chunklink.hpp>
#include <onsem/texttosemantic/type/chunk.hpp>
#include <onsem/texttosemantic/linguisticanalyzer.hpp>

using namespace onsem;


std::string _toTextToTopRootChunkStrs(const std::string& pText, const linguistics::LinguisticDatabase& pLingDb, SemanticLanguageEnum pLanguageType) {
    std::string res;

    linguistics::SyntacticGraph syntGraph(pLingDb, pLanguageType);
    LinguisticAnalysisConfig linguisticAnalysisConfig;
    linguistics::tokenizationAndSyntacticalAnalysis(syntGraph, pText, linguisticAnalysisConfig);
    for (const auto& currChunkLink : syntGraph.firstChildren) {
        const linguistics::Chunk& currChunk = *currChunkLink.chunk;
        auto tokenRangeWithChildren = currChunk.getTokRangeWrappingChildren();
        if (!res.empty())
            res += ", ";
        res += "'" + tokenRangeWithChildren.toStr() + "'";
    }
    return res;
}



TEST_F(SemanticReasonerGTests, chunk_to_original_text) {
    linguistics::LinguisticDatabase& lingdb = *lingDbPtr;
    SemanticLanguageEnum frLang = SemanticLanguageEnum::FRENCH;

    EXPECT_EQ("'je suis  content de te voir ', 'tu es sympa'", _toTextToTopRootChunkStrs("je suis  content de te voir tu es sympa", lingdb, frLang));
}
