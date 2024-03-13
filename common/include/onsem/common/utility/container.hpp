#ifndef ONSEM_COMMON_UTILITY_CONTAINER_HPP
#define ONSEM_COMMON_UTILITY_CONTAINER_HPP

#include <cstddef>
#include <iterator>

namespace onsem {

template<typename CONTAINER_TYPE>
void keepOnlyTheFirstElements(CONTAINER_TYPE& pContainer, std::size_t pMaxSize) {
    if (pContainer.size() > pMaxSize) {
        auto itLastAnswer = pContainer.begin();
        std::advance(itLastAnswer, pMaxSize);
        pContainer.erase(itLastAnswer, pContainer.end());
    }
}

}    // End of namespace onsem

#endif    // ONSEM_COMMON_UTILITY_CONTAINER_HPP
