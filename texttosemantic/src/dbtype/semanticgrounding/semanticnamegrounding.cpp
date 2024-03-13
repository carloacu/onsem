#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticnamegrounding.hpp>
#include <onsem/common/binary/binaryloader.hpp>
#include <numeric>

namespace onsem {

std::string SemanticNameGrounding::namesToStr(const std::vector<std::string>& pNames) {
    if (pNames.empty())
        return "";
    return std::accumulate(
        std::next(pNames.begin()), pNames.end(), pNames.front(), [](const std::string& s1, const std::string& s2) {
            return s1 + ' ' + s2;
        });
}

std::string SemanticNameGrounding::namesToStrFromBinary(unsigned char pNbOfStrings, const unsigned char*& pUChars) {
    if (pNbOfStrings == 0)
        return "";
    std::string res;
    bool firstIteration = true;
    for (unsigned char i = 0; i < pNbOfStrings; ++i) {
        if (!firstIteration)
            res += ' ';
        else
            firstIteration = false;
        res += binaryloader::loadString(pUChars);
    }
    return res;
}

}    // End of namespace onsem
