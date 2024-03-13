#include "../semanticreasonergtests.hpp"
#include <gtest/gtest.h>
#include <onsem/common/binary/binarysaver.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticagentgrounding.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/groundedexpression.hpp>
#include <onsem/texttosemantic/dbtype/binary/semexploader.hpp>
#include <onsem/texttosemantic/dbtype/binary/semexpsaver.hpp>
#include <onsem/tester/sentencesloader.hpp>

using namespace onsem;

void _checkSemExp_binarization(binarymasks::Ptr pPtr,
                               const SemanticExpression& pSemExp,
                               const linguistics::LinguisticDatabase& pLingDb) {
    auto beginPtr = pPtr;

    semexpsaver::writeSemExp(pPtr, pSemExp, pLingDb, nullptr);

    auto readSemExp = semexploader::loadSemExp(beginPtr.pcuchar, pLingDb);

    if (pSemExp != *readSemExp) {
        std::cerr << "exp1:\n" << printSemExp(pSemExp) << std::endl;
        std::cerr << "exp2:\n" << printSemExp(*readSemExp) << std::endl;
        EXPECT_EQ(pSemExp, *readSemExp);
    }
}

void _test_binarization(binarymasks::Ptr pPtr,
                        const std::string& pTextCorpusFolder,
                        const std::string& pLanguageStr,
                        const linguistics::LinguisticDatabase& pLingDb) {
    SemanticLanguageEnum language = semanticLanguageTypeGroundingEnumFromStr(pLanguageStr);

    SentencesLoader sentencesXml;
    sentencesXml.loadFolder(pTextCorpusFolder + "/" + pLanguageStr);
    const std::vector<std::string>& sentences = sentencesXml.getSentences();
    for (const std::string& sent : sentences) {
        auto semExp = textToContextualSemExp(sent, pLingDb, language);
        _checkSemExp_binarization(pPtr, *semExp, pLingDb);
    }
}

TEST_F(SemanticReasonerGTests, binarization_of_semexps) {
    const std::size_t maxSize = 10000;
    binarymasks::Ptr mem = ::operator new(maxSize);
    EXPECT_EQ(mem.val, binarysaver::alignMemory(mem).val);
    binarymasks::Ptr beginPtr = mem;

    const linguistics::LinguisticDatabase& lingdb = *lingDbPtr;
    _test_binarization(mem, _corpusInputFolder, frenchStr, lingdb);
    _test_binarization(mem, _corpusInputFolder, englishStr, lingdb);
    _test_binarization(mem, _corpusInputFolder, japaneseStr, lingdb);

    // Check agent grounding serialization
    auto agentGrdExp = UniqueSemanticExpression(std::make_unique<GroundedExpression>(
        std::make_unique<SemanticAgentGrounding>("a", "b", std::vector<std::string>{"b"})));
    _checkSemExp_binarization(mem, *agentGrdExp, lingdb);

    ::operator delete(beginPtr.pchar);
}
