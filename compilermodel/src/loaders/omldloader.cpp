#include "omldloader.hpp"
#include <sstream>
#include <fstream>
#include <onsem/common/utility/lexical_cast.hpp>
#include <onsem/common/utility/string.hpp>
#include <onsem/compilermodel/linguisticintermediarydatabase.hpp>
#include <onsem/compilermodel/lingdbmeaning.hpp>
#include <onsem/compilermodel/lingdbmodifier.hpp>

namespace onsem {
namespace {
void _readAttributtes(const std::string& pLine,
                      std::size_t pPos,
                      const std::function<void(char, const std::string&)>& pProcessAttr) {
    auto lineSize = pLine.size();

    while (pPos < lineSize) {
        pPos = pLine.find('=', pPos);
        if (pPos == std::string::npos)
            break;
        char attrName = pLine[pPos - 1];

        auto beginOfValuePos = pPos + 2;
        if (beginOfValuePos >= lineSize)
            break;
        pPos = pLine.find('"', beginOfValuePos);
        if (pPos == std::string::npos)
            break;

        auto attrValue = pLine.substr(beginOfValuePos, pPos - beginOfValuePos);
        pProcessAttr(attrName, attrValue);
    }
}

}

void mergeOmld(const std::string& pFilename, LinguisticIntermediaryDatabase& pWords) {
    std::ifstream infile(pFilename, std::ifstream::in);
    if (!infile.is_open())
        throw std::runtime_error("Can't open " + pFilename + " file !");

    std::string currentLemma;
    PartOfSpeech currentLemmaPos = PartOfSpeech::UNKNOWN;

    std::string line;
    while (getline(infile, line)) {
        if (line.empty())
            continue;

        auto beginOfBeaconPos = line.find('<');
        if (beginOfBeaconPos == std::string::npos)
            continue;

        auto lineSize = line.size();
        auto beanNamePos = beginOfBeaconPos + 1;
        if (beanNamePos >= lineSize)
            continue;

        switch (line[beanNamePos]) {
            case 'w': {
                _readAttributtes(line, beanNamePos, [&](char pAttrName, const std::string& pAttrValue) {
                    switch (pAttrName) {
                        case 'l': currentLemma = pAttrValue; break;
                        case 'p': currentLemmaPos = partOfSpeech_fromStr(pAttrValue); break;
                    }
                });
                break;
            }

            case 'i': {
                std::string word = currentLemma;
                PartOfSpeech inflPos = currentLemmaPos;
                std::vector<std::string> flexions;
                char frequency = 4;
                _readAttributtes(line, beanNamePos, [&](char pAttrName, const std::string& pAttrValue) {
                    switch (pAttrName) {
                        case 'l':
                            currentLemma = pAttrValue;
                            word = pAttrValue;
                            break;
                        case 'i': word = pAttrValue; break;
                        case 'p': inflPos = partOfSpeech_fromStr(pAttrValue); break;
                        case 'f': mystd::splitNotEmpty(flexions, pAttrValue, "|"); break;
                        case 'y': frequency = mystd::lexical_cast<char>(pAttrValue); break;
                    }
                });
                pWords.addWord(word, currentLemma, inflPos, flexions, frequency);
                break;
            }

            case 'r': {
                std::string word = currentLemma;
                PartOfSpeech inflPos = currentLemmaPos;
                _readAttributtes(line, beanNamePos, [&](char pAttrName, const std::string& pAttrValue) {
                    switch (pAttrName) {
                        case 'l': word = pAttrValue; break;
                        case 'p': inflPos = partOfSpeech_fromStr(pAttrValue); break;
                    }
                });
                pWords.removeWordForm(word, inflPos);
                break;
            }
        }
    }
}

}    // End of namespace onsem
