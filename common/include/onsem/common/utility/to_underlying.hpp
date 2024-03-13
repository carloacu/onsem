#ifndef ONSEM_COMMON_UTILITY_TO_UNDERLYING_HPP
#define ONSEM_COMMON_UTILITY_TO_UNDERLYING_HPP

#include <type_traits>

namespace onsem {

namespace mystd {

template<typename E>
constexpr typename std::underlying_type<E>::type to_underlying(E e) noexcept {
    return static_cast<typename std::underlying_type<E>::type>(e);
}

}    // End of namespace mystd
}    // End of namespace onsem

#endif    // ONSEM_COMMON_UTILITY_TO_UNDERLYING_HPP
