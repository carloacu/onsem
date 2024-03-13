#ifndef ONSEM_COMMON_UTILITY_SIZEPRINTERINBYTES_HPP
#define ONSEM_COMMON_UTILITY_SIZEPRINTERINBYTES_HPP

#include <ostream>

namespace onsem {

inline static void prettyPrintSizeNbInBytes(std::ostream& pOs, std::size_t pSize) {
    if (pSize < 1024) {
        pOs << pSize << "o";
    } else if (pSize < 1048576) {
        pOs << pSize / 1024 << "ko";
    } else if (pSize < 1073741824) {
        pOs << pSize / 1048576 << "Mo";
    } else {
        pOs << pSize / 1073741824 << "Go";
    }
}

}    // End of namespace onsem

#endif    // ONSEM_COMMON_UTILITY_SIZEPRINTERINBYTES_HPP
