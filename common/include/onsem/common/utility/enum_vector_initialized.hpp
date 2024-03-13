#ifndef ONSEM_COMMON_UTILITY_ENUM_VECTOR_INITIALIZED_HPP
#define ONSEM_COMMON_UTILITY_ENUM_VECTOR_INITIALIZED_HPP

#include <memory>
#include <vector>
#include <assert.h>
#include <functional>
#include <onsem/common/utility/to_underlying.hpp>

namespace onsem {
namespace mystd {

template<typename ENUM_TYPE, typename VALUE_TYPE>
struct enum_vector_initialized {
    enum_vector_initialized(std::size_t pSizeOfEnum,
                            const std::function<std::unique_ptr<VALUE_TYPE>(std::size_t)>& pInitilizer);
    enum_vector_initialized(const enum_vector_initialized& pOther);
    enum_vector_initialized& operator=(const enum_vector_initialized& pOther);

    VALUE_TYPE& operator[](const ENUM_TYPE& pEnumValue);
    const VALUE_TYPE& operator[](const ENUM_TYPE& pEnumValue) const;
    VALUE_TYPE& operator[](std::size_t pId);
    const VALUE_TYPE& operator[](std::size_t pId) const;
    std::size_t size() const { return _content.size(); }
    const ENUM_TYPE& getKey(std::size_t pId) const;

private:
    std::vector<std::unique_ptr<VALUE_TYPE>> _content;

    void _copyContent(const enum_vector_initialized& pOther);
};

template<typename ENUM_TYPE, typename VALUE_TYPE>
enum_vector_initialized<ENUM_TYPE, VALUE_TYPE>::enum_vector_initialized(
    std::size_t pSizeOfEnum,
    const std::function<std::unique_ptr<VALUE_TYPE>(std::size_t)>& pInitilizer)
    : _content(pSizeOfEnum) {
    for (std::size_t i = 0; i < pSizeOfEnum; ++i) {
        auto valuePtr = pInitilizer(i);
        assert(valuePtr);
        _content[i] = std::move(valuePtr);
    }
}

template<typename ENUM_TYPE, typename VALUE_TYPE>
enum_vector_initialized<ENUM_TYPE, VALUE_TYPE>::enum_vector_initialized(const enum_vector_initialized& pOther)
    : _content(pOther._content.size()) {
    _copyContent(pOther);
}

template<typename ENUM_TYPE, typename VALUE_TYPE>
enum_vector_initialized<ENUM_TYPE, VALUE_TYPE>& enum_vector_initialized<ENUM_TYPE, VALUE_TYPE>::operator=(
    const enum_vector_initialized& pOther) {
    _content = std::vector<std::unique_ptr<VALUE_TYPE>>(pOther._content.size());
    _copyContent(pOther);
    return *this;
}

template<typename ENUM_TYPE, typename VALUE_TYPE>
VALUE_TYPE& enum_vector_initialized<ENUM_TYPE, VALUE_TYPE>::operator[](const ENUM_TYPE& pEnumValue) {
    return *_content[mystd::to_underlying(pEnumValue)];
}

template<typename ENUM_TYPE, typename VALUE_TYPE>
const VALUE_TYPE& enum_vector_initialized<ENUM_TYPE, VALUE_TYPE>::operator[](const ENUM_TYPE& pEnumValue) const {
    return *_content[mystd::to_underlying(pEnumValue)];
}

template<typename ENUM_TYPE, typename VALUE_TYPE>
VALUE_TYPE& enum_vector_initialized<ENUM_TYPE, VALUE_TYPE>::operator[](std::size_t pId) {
    return *_content[pId];
}

template<typename ENUM_TYPE, typename VALUE_TYPE>
const VALUE_TYPE& enum_vector_initialized<ENUM_TYPE, VALUE_TYPE>::operator[](std::size_t pId) const {
    return *_content[pId];
}

template<typename ENUM_TYPE, typename VALUE_TYPE>
void enum_vector_initialized<ENUM_TYPE, VALUE_TYPE>::_copyContent(const enum_vector_initialized& pOther) {
    for (std::size_t i = 0; i < pOther._content.size(); ++i) {
        if (pOther._content[i])
            _content[i] = std::make_unique<VALUE_TYPE>(*pOther._content[i]);
        else
            _content[i] = std::make_unique<VALUE_TYPE>();
    }
}

}    // end namespace mystd
}    // end namespace onsem

#endif    // ONSEM_COMMON_UTILITY_ENUM_VECTOR_INITIALIZED_HPP
