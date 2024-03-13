#ifndef ONSEM_COMMON_UTILITY_RADIX_MAP_STRUCT_HPP
#define ONSEM_COMMON_UTILITY_RADIX_MAP_STRUCT_HPP

#include <onsem/common/utility/radix_map.hpp>

namespace onsem {
namespace mystd {

template<typename KEY_TYPE, typename VALUE_TYPE>
struct radix_map_struct : public radix_map_str<VALUE_TYPE> {
    VALUE_TYPE& operator[](const KEY_TYPE& pKey);

    template<typename... Args>
    void emplace(const KEY_TYPE& pKey, Args&&... pArgs);
    std::pair<typename radix_map<char, std::string, VALUE_TYPE>::iterator, bool> insert(const KEY_TYPE& pKey);
    std::pair<typename radix_map<char, std::string, VALUE_TYPE>::iterator, bool> insert(const std::string& pKeyStr);

    typename radix_map<char, std::string, VALUE_TYPE>::iterator find(const KEY_TYPE& pKey);
    typename radix_map<char, std::string, VALUE_TYPE>::iterator find(const std::string& pKeyStr);
    typename radix_map<char, std::string, VALUE_TYPE>::iterator lower_bound(const KEY_TYPE& pKey);
    typename radix_map<char, std::string, VALUE_TYPE>::iterator lower_bound(const std::string& pKeyStr);
    typename radix_map<char, std::string, VALUE_TYPE>::iterator upper_bound(const KEY_TYPE& pKey);
    typename radix_map<char, std::string, VALUE_TYPE>::iterator upper_bound(const std::string& pKeyStr);
    VALUE_TYPE* find_ptr(const KEY_TYPE& pKey);
    VALUE_TYPE* find_ptr(const std::string& pKeyStr);

    typename radix_map<char, std::string, VALUE_TYPE>::const_iterator find(const KEY_TYPE& pKey) const;
    typename radix_map<char, std::string, VALUE_TYPE>::const_iterator find(const std::string& pKeyStr) const;
    typename radix_map<char, std::string, VALUE_TYPE>::const_iterator lower_bound(const KEY_TYPE& pKey) const;
    typename radix_map<char, std::string, VALUE_TYPE>::const_iterator lower_bound(const std::string& pKeyStry) const;
    typename radix_map<char, std::string, VALUE_TYPE>::const_iterator upper_bound(const KEY_TYPE& pKey) const;
    typename radix_map<char, std::string, VALUE_TYPE>::const_iterator upper_bound(const std::string& pKeyStr) const;
    const VALUE_TYPE* find_ptr(const KEY_TYPE& pKey) const;
    const VALUE_TYPE* find_ptr(const std::string& pKeyStr) const;
};

template<typename KEY_TYPE, typename VALUE_TYPE>
VALUE_TYPE& radix_map_struct<KEY_TYPE, VALUE_TYPE>::operator[](const KEY_TYPE& pKey) {
    return radix_map_str<VALUE_TYPE>::operator[](pKey.toRadixMapStr());
}

template<typename KEY_TYPE, typename VALUE_TYPE>
template<typename... Args>
void radix_map_struct<KEY_TYPE, VALUE_TYPE>::emplace(const KEY_TYPE& pKey, Args&&... pArgs) {
    radix_map_str<VALUE_TYPE>::emplace(pKey.toRadixMapStr(), std::forward<Args>(pArgs)...);
}

template<typename KEY_TYPE, typename VALUE_TYPE>
std::pair<typename radix_map<char, std::string, VALUE_TYPE>::iterator, bool>
    radix_map_struct<KEY_TYPE, VALUE_TYPE>::insert(const KEY_TYPE& pKey) {
    return radix_map_str<VALUE_TYPE>::insert(pKey.toRadixMapStr());
}

template<typename KEY_TYPE, typename VALUE_TYPE>
std::pair<typename radix_map<char, std::string, VALUE_TYPE>::iterator, bool>
    radix_map_struct<KEY_TYPE, VALUE_TYPE>::insert(const std::string& pKeyStr) {
    return radix_map_str<VALUE_TYPE>::insert(pKeyStr);
}

template<typename KEY_TYPE, typename VALUE_TYPE>
typename radix_map<char, std::string, VALUE_TYPE>::iterator radix_map_struct<KEY_TYPE, VALUE_TYPE>::find(
    const KEY_TYPE& pKey) {
    return radix_map_str<VALUE_TYPE>::find(pKey.toRadixMapStr());
}

template<typename KEY_TYPE, typename VALUE_TYPE>
typename radix_map<char, std::string, VALUE_TYPE>::const_iterator radix_map_struct<KEY_TYPE, VALUE_TYPE>::find(
    const KEY_TYPE& pKey) const {
    return radix_map_str<VALUE_TYPE>::find(pKey.toRadixMapStr());
}

template<typename KEY_TYPE, typename VALUE_TYPE>
typename radix_map<char, std::string, VALUE_TYPE>::iterator radix_map_struct<KEY_TYPE, VALUE_TYPE>::find(
    const std::string& pKeyStr) {
    return radix_map_str<VALUE_TYPE>::find(pKeyStr);
}

template<typename KEY_TYPE, typename VALUE_TYPE>
typename radix_map<char, std::string, VALUE_TYPE>::const_iterator radix_map_struct<KEY_TYPE, VALUE_TYPE>::find(
    const std::string& pKeyStr) const {
    return radix_map_str<VALUE_TYPE>::find(pKeyStr);
}

template<typename KEY_TYPE, typename VALUE_TYPE>
typename radix_map<char, std::string, VALUE_TYPE>::iterator radix_map_struct<KEY_TYPE, VALUE_TYPE>::lower_bound(
    const KEY_TYPE& pKey) {
    return radix_map_str<VALUE_TYPE>::lower_bound(pKey.toRadixMapStr());
}

template<typename KEY_TYPE, typename VALUE_TYPE>
typename radix_map<char, std::string, VALUE_TYPE>::const_iterator radix_map_struct<KEY_TYPE, VALUE_TYPE>::lower_bound(
    const KEY_TYPE& pKey) const {
    return radix_map_str<VALUE_TYPE>::lower_bound(pKey.toRadixMapStr());
}

template<typename KEY_TYPE, typename VALUE_TYPE>
typename radix_map<char, std::string, VALUE_TYPE>::iterator radix_map_struct<KEY_TYPE, VALUE_TYPE>::lower_bound(
    const std::string& pKeyStr) {
    return radix_map_str<VALUE_TYPE>::lower_bound(pKeyStr);
}

template<typename KEY_TYPE, typename VALUE_TYPE>
typename radix_map<char, std::string, VALUE_TYPE>::const_iterator radix_map_struct<KEY_TYPE, VALUE_TYPE>::lower_bound(
    const std::string& pKeyStr) const {
    return radix_map_str<VALUE_TYPE>::lower_bound(pKeyStr);
}

template<typename KEY_TYPE, typename VALUE_TYPE>
typename radix_map<char, std::string, VALUE_TYPE>::iterator radix_map_struct<KEY_TYPE, VALUE_TYPE>::upper_bound(
    const KEY_TYPE& pKey) {
    return radix_map_str<VALUE_TYPE>::upper_bound(pKey.toRadixMapStr());
}

template<typename KEY_TYPE, typename VALUE_TYPE>
typename radix_map<char, std::string, VALUE_TYPE>::const_iterator radix_map_struct<KEY_TYPE, VALUE_TYPE>::upper_bound(
    const KEY_TYPE& pKey) const {
    return radix_map_str<VALUE_TYPE>::upper_bound(pKey.toRadixMapStr());
}

template<typename KEY_TYPE, typename VALUE_TYPE>
typename radix_map<char, std::string, VALUE_TYPE>::iterator radix_map_struct<KEY_TYPE, VALUE_TYPE>::upper_bound(
    const std::string& pKeyStr) {
    return radix_map_str<VALUE_TYPE>::upper_bound(pKeyStr);
}

template<typename KEY_TYPE, typename VALUE_TYPE>
typename radix_map<char, std::string, VALUE_TYPE>::const_iterator radix_map_struct<KEY_TYPE, VALUE_TYPE>::upper_bound(
    const std::string& pKeyStr) const {
    return radix_map_str<VALUE_TYPE>::upper_bound(pKeyStr);
}

template<typename KEY_TYPE, typename VALUE_TYPE>
VALUE_TYPE* radix_map_struct<KEY_TYPE, VALUE_TYPE>::find_ptr(const KEY_TYPE& pKey) {
    return radix_map_str<VALUE_TYPE>::find_ptr(pKey.toRadixMapStr());
}

template<typename KEY_TYPE, typename VALUE_TYPE>
VALUE_TYPE* radix_map_struct<KEY_TYPE, VALUE_TYPE>::find_ptr(const std::string& pKeyStr) {
    return radix_map_str<VALUE_TYPE>::find_ptr(pKeyStr);
}

template<typename KEY_TYPE, typename VALUE_TYPE>
const VALUE_TYPE* radix_map_struct<KEY_TYPE, VALUE_TYPE>::find_ptr(const KEY_TYPE& pKey) const {
    return radix_map_str<VALUE_TYPE>::find_ptr(pKey.toRadixMapStr());
}

template<typename KEY_TYPE, typename VALUE_TYPE>
const VALUE_TYPE* radix_map_struct<KEY_TYPE, VALUE_TYPE>::find_ptr(const std::string& pKeyStr) const {
    return radix_map_str<VALUE_TYPE>::find_ptr(pKeyStr);
}

}    // end namespace mystd
}    // end namespace onsem

#endif    // ONSEM_COMMON_UTILITY_RADIX_MAP_STRUCT_HPP
