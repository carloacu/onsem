#ifndef ONSEM_COMMON_UTILITY_LEXICAL_CAST_HPP
#define ONSEM_COMMON_UTILITY_LEXICAL_CAST_HPP

#include <string>
#include <cstdlib>
#include <stdexcept>

namespace onsem {
namespace mystd {

template<typename T>
T lexical_cast(const std::string& pStr) {
    bool firstChar = true;
    for (const auto& currChar : pStr) {
        if ((currChar < '0' || currChar > '9') && !(firstChar && currChar == '-'))
            throw std::runtime_error("bad lexical cast: source type value could not be interpreted as target");
        firstChar = false;
    }
    return atoi(pStr.c_str());
}

template<typename T>
T lexical_cast_unigned(const std::string& pStr) {
    for (const auto& currChar : pStr)
        if (currChar < '0' || currChar > '9')
            throw std::runtime_error("bad lexical cast: source type value could not be interpreted as target");
    return atoi(pStr.c_str());
}

}    // end namespace mystd
}    // end namespace onsem

#endif    // ONSEM_COMMON_UTILITY_LEXICAL_CAST_HPP
