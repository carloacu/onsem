#ifndef ONSEM_COMMON_UTILITY_ENUM_VECTOR_HPP
#define ONSEM_COMMON_UTILITY_ENUM_VECTOR_HPP

#include <vector>
#include <onsem/common/utility/to_underlying.hpp>

namespace onsem {
namespace mystd {

template<typename ENUM_TYPE, typename VALUE_TYPE>
struct enum_vector {
    enum_vector(std::size_t pSizeOfEnum);
    VALUE_TYPE& operator[](const ENUM_TYPE& pEnumValue);
    const VALUE_TYPE& operator[](const ENUM_TYPE& pEnumValue) const;
    VALUE_TYPE& operator[](std::size_t pId);
    const VALUE_TYPE& operator[](std::size_t pId) const;
    std::size_t size() const { return _content.size(); }
    const ENUM_TYPE& getKey(std::size_t pId) const;

private:
    std::vector<VALUE_TYPE> _content;
};

template<typename ENUM_TYPE, typename VALUE_TYPE>
enum_vector<ENUM_TYPE, VALUE_TYPE>::enum_vector(std::size_t pSizeOfEnum)
    : _content(pSizeOfEnum) {}

template<typename ENUM_TYPE, typename VALUE_TYPE>
VALUE_TYPE& enum_vector<ENUM_TYPE, VALUE_TYPE>::operator[](const ENUM_TYPE& pEnumValue) {
    return _content[mystd::to_underlying(pEnumValue)];
}

template<typename ENUM_TYPE, typename VALUE_TYPE>
const VALUE_TYPE& enum_vector<ENUM_TYPE, VALUE_TYPE>::operator[](const ENUM_TYPE& pEnumValue) const {
    return _content[mystd::to_underlying(pEnumValue)];
}

template<typename ENUM_TYPE, typename VALUE_TYPE>
VALUE_TYPE& enum_vector<ENUM_TYPE, VALUE_TYPE>::operator[](std::size_t pId) {
    return _content[pId];
}

template<typename ENUM_TYPE, typename VALUE_TYPE>
const VALUE_TYPE& enum_vector<ENUM_TYPE, VALUE_TYPE>::operator[](std::size_t pId) const {
    return _content[pId];
}

}    // end namespace mystd
}    // end namespace onsem

#endif    // ONSEM_COMMON_UTILITY_ENUM_VECTOR_HPP
