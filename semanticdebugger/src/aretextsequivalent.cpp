#include <onsem/semanticdebugger/aretextsequivalent.hpp>
#include <onsem/common/utility/uppercasehandler.hpp>

namespace onsem {

namespace {
std::string _removeChars(const std::string& pStr, const std::string& pCharsToRemove) {
    std::string res;
    for (std::size_t i = 0; i < pStr.size(); ++i)
        if (pCharsToRemove.find(pStr[i]) == std::string::npos)
            res += pStr[i];
    return res;
}

std::string _removeSomeSpaces(std::string& pStr) {
    std::string res;
    for (std::size_t i = 0; i < pStr.size(); ++i) {
        if (pStr[i] == '-')
            res += ' ';
        else if (!(pStr[i] == ' ' && ((i + 1) == pStr.size() || pStr[i + 1] == ' ')))
            res += pStr[i];
    }
    return res;
}

}

bool areTextEquivalent(const std::string& pText1,
                       const std::string& pText2,
                       const std::map<std::string, std::string>* pEquivalencesPtr) {
    if (pText1 == pText2)
        return true;

    if (pEquivalencesPtr != nullptr) {
        auto it = pEquivalencesPtr->find(pText1);
        if (it != pEquivalencesPtr->end() && it->second == pText2)
            return true;
    }

    static const std::string charsToRemove = ",.!?()";
    std::string text1 = _removeChars(pText1, charsToRemove);
    std::string text2 = _removeChars(pText2, charsToRemove);
    lowerCaseFirstLetter(text1);
    lowerCaseFirstLetter(text2);
    text1 = _removeSomeSpaces(text1);
    text2 = _removeSomeSpaces(text2);
    return text1 == text2;
}

}    // End of namespace onsem
