#include <onsem/common/utility/getendofparenthesis.hpp>

namespace onsem {

std::size_t getEndOfParenthesis(const std::string& pStr, std::size_t pPos, const std::size_t pEnd) {
    char begOfParenthesisChar = pStr[pPos++];
    char endOfParenthesisChar = begOfParenthesisChar == '(' ? ')' : ']';
    std::size_t begOfParenthesisStack = 1;
    while (pPos < pEnd) {
        if (pStr[pPos] == begOfParenthesisChar) {
            ++begOfParenthesisStack;
        } else if (pStr[pPos] == endOfParenthesisChar) {
            if (begOfParenthesisStack > 1) {
                --begOfParenthesisStack;
            } else {
                return pPos;
            }
        }
        ++pPos;
    }
    return std::string::npos;
}

}    // End of namespace onsem
