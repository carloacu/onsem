#include "wikiutils.hpp"

namespace wikiutil {

void extractWordOfTitleLine(std::string& pWord, const std::string& pLine) {
    std::size_t wordEnd = pLine.find_first_of('<', 11);
    if (wordEnd != std::string::npos && wordEnd > 11)
        pWord = pLine.substr(11, wordEnd - 11);
    else
        pWord.clear();
}

}    // End of namespace wikiutil
