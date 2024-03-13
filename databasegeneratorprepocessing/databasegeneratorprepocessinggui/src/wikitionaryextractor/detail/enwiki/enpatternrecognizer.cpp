#include "enpatternrecognizer.hpp"
#include <assert.h>
#include <iostream>

EnPatternRecognizer::EnPatternRecognizer()
    : fBeginLineOutLang("* French:")
    , fBeginLineOutLang_size(fBeginLineOutLang.size())
    , fBeginLineOutLang2("*French:")
    , fBeginLineOutLang_size2(fBeginLineOutLang2.size()) {}

void EnPatternRecognizer::getSperator(std::string& pSeparatorName,
                                      PatternRecognizerSeparatorType& pSeparatorType,
                                      const std::string& pLine) const {
    assert(pLine.size() > 6);
    pSeparatorType = PATTERNRECO_SEP_UNKNOWN;
    std::size_t firstChar = pLine.find_first_not_of('=');

    std::size_t endGram = pLine.find_first_of('=', firstChar);
    if (endGram != std::string::npos && endGram > firstChar) {
        pSeparatorName = pLine.substr(firstChar, endGram - firstChar);
        if (pSeparatorName == "Synonyms") {
            pSeparatorType = PATTERNRECO_SEP_SYNONYMS;
        }
    } else {
        std::cerr << "isSeparator TRUE, but getGramInLineType fails, for line: " << pLine << std::endl;
    }
}
