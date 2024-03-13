#ifndef ONSEM_COMMON_UTILITY_RADIX_MAP_FORWARD_DECLARATION_HPP
#define ONSEM_COMMON_UTILITY_RADIX_MAP_FORWARD_DECLARATION_HPP

#include <string>

namespace onsem {
namespace mystd {
template<typename KEY_TYPE, typename KEY_CONTAINER_TYPE, typename VALUE_TYPE>
struct radix_map;

template<typename VALUE_TYPE>
using radix_map_str = radix_map<char, std::string, VALUE_TYPE>;

}    // end namespace mystd
}    // end namespace onsem

#endif    // ONSEM_COMMON_UTILITY_RADIX_MAP_FORWARD_DECLARATION_HPP
