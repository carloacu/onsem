#ifndef ONSEM_COMMON_UTILITY_VECTOR_OF_REFS_HPP
#define ONSEM_COMMON_UTILITY_VECTOR_OF_REFS_HPP

#include <vector>

namespace onsem {

namespace mystd {

template<typename T>
class vector_of_refs {
public:
    class iterator;
    class const_iterator {
    public:
        const_iterator(typename std::vector<T*>::const_iterator pIt);

        const_iterator& operator++();
        const_iterator& operator--();
        const T* operator->();
        const T& operator*();
        bool operator==(const const_iterator& pOther) const;
        bool operator!=(const const_iterator& pOther) const;
        bool operator==(const iterator& pOther) const;
        bool operator!=(const iterator& pOther) const;

    private:
        friend class vector_of_refs<T>;
        typename std::vector<T*>::const_iterator _it;
    };
    class iterator {
    public:
        iterator(typename std::vector<T*>::iterator pIt);

        iterator& operator++();
        iterator& operator--();
        T* operator->();
        T& operator*();
        bool operator==(const iterator& pOther) const;
        bool operator!=(const iterator& pOther) const;
        bool operator==(const const_iterator& pOther) const;
        bool operator!=(const const_iterator& pOther) const;

    private:
        friend class vector_of_refs<T>;
        typename std::vector<T*>::iterator _it;
    };

    void insert(iterator pIteratorPos, iterator pIteratorBegin, iterator pIteratorEnd);
    void emplace_back(T& pElt);
    void push_back(T& pElt);
    void shrink_to_fit();
    template<typename ITTYPE>
    void insert(ITTYPE pBegin, ITTYPE pEnd);
    bool empty() const;
    std::size_t size() const;
    bool operator==(const vector_of_refs<T>& pOther) const;
    bool operator!=(const vector_of_refs<T>& pOther) const;
    T& back();
    const T& back() const;

    iterator begin();
    iterator end();
    iterator find(T& pElt);
    iterator erase(const iterator& pIt);
    iterator erase(const const_iterator& pIt);
    void erase(const T& pElt);

    const_iterator begin() const;
    const_iterator end() const;
    const_iterator cbegin() const;
    const_iterator cend() const;
    const_iterator find(const T& pElt) const;

private:
    std::vector<T*> _elts{};
};

template<typename T>
vector_of_refs<T>::const_iterator::const_iterator(typename std::vector<T*>::const_iterator pIt)
    : _it(pIt) {}

template<typename T>
typename vector_of_refs<T>::const_iterator& vector_of_refs<T>::const_iterator::operator++() {
    ++_it;
    return *this;
}

template<typename T>
typename vector_of_refs<T>::const_iterator& vector_of_refs<T>::const_iterator::operator--() {
    --_it;
    return *this;
}

template<typename T>
const T* vector_of_refs<T>::const_iterator::operator->() {
    return *_it;
}

template<typename T>
const T& vector_of_refs<T>::const_iterator::operator*() {
    return **_it;
}

template<typename T>
bool vector_of_refs<T>::const_iterator::operator==(const typename vector_of_refs<T>::const_iterator& pOther) const {
    return _it == pOther._it;
}

template<typename T>
bool vector_of_refs<T>::const_iterator::operator!=(const typename vector_of_refs<T>::const_iterator& pOther) const {
    return _it != pOther._it;
}

template<typename T>
bool vector_of_refs<T>::const_iterator::operator==(const typename vector_of_refs<T>::iterator& pOther) const {
    return _it == pOther._it;
}

template<typename T>
bool vector_of_refs<T>::const_iterator::operator!=(const typename vector_of_refs<T>::iterator& pOther) const {
    return _it != pOther._it;
}

template<typename T>
vector_of_refs<T>::iterator::iterator(typename std::vector<T*>::iterator pIt)
    : _it(pIt) {}

template<typename T>
typename vector_of_refs<T>::iterator& vector_of_refs<T>::iterator::operator++() {
    ++_it;
    return *this;
}

template<typename T>
typename vector_of_refs<T>::iterator& vector_of_refs<T>::iterator::operator--() {
    --_it;
    return *this;
}

template<typename T>
T* vector_of_refs<T>::iterator::operator->() {
    return *&*_it;
}

template<typename T>
T& vector_of_refs<T>::iterator::operator*() {
    return **_it;
}

template<typename T>
bool vector_of_refs<T>::iterator::operator==(const typename vector_of_refs<T>::iterator& pOther) const {
    return _it == pOther._it;
}

template<typename T>
bool vector_of_refs<T>::iterator::operator!=(const typename vector_of_refs<T>::iterator& pOther) const {
    return _it != pOther._it;
}

template<typename T>
bool vector_of_refs<T>::iterator::operator==(const typename vector_of_refs<T>::const_iterator& pOther) const {
    return _it == pOther._it;
}

template<typename T>
bool vector_of_refs<T>::iterator::operator!=(const typename vector_of_refs<T>::const_iterator& pOther) const {
    return _it != pOther._it;
}

template<typename T>
void vector_of_refs<T>::insert(typename vector_of_refs<T>::iterator pIteratorPos,
                               typename vector_of_refs<T>::iterator pIteratorBegin,
                               typename vector_of_refs<T>::iterator pIteratorEnd) {
    _elts.insert(pIteratorPos._it, pIteratorBegin._it, pIteratorEnd._it);
}

template<typename T>
void vector_of_refs<T>::emplace_back(T& pElt) {
    _elts.emplace_back(&pElt);
}

template<typename T>
void vector_of_refs<T>::push_back(T& pElt) {
    _elts.push_back(&pElt);
}

template<typename T>
void vector_of_refs<T>::shrink_to_fit() {
    _elts.shrink_to_fit();
}

template<typename T>
template<typename ITTYPE>
void vector_of_refs<T>::insert(ITTYPE pBegin, ITTYPE pEnd) {
    for (auto it = pBegin; it != pEnd; ++it)
        insert(*it);
}

template<typename T>
bool vector_of_refs<T>::empty() const {
    return _elts.empty();
}

template<typename T>
std::size_t vector_of_refs<T>::size() const {
    return _elts.size();
}

template<typename T>
bool vector_of_refs<T>::operator==(const vector_of_refs<T>& pOther) const {
    return _elts == pOther._elts;
}

template<typename T>
bool vector_of_refs<T>::operator!=(const vector_of_refs<T>& pOther) const {
    return _elts != pOther._elts;
}

template<typename T>
T& vector_of_refs<T>::back() {
    return *_elts.back();
}

template<typename T>
const T& vector_of_refs<T>::back() const {
    return *_elts.back();
}

template<typename T>
typename vector_of_refs<T>::iterator vector_of_refs<T>::begin() {
    return vector_of_refs<T>::iterator(_elts.begin());
}

template<typename T>
typename vector_of_refs<T>::iterator vector_of_refs<T>::end() {
    return vector_of_refs<T>::iterator(_elts.end());
}

template<typename T>
typename vector_of_refs<T>::iterator vector_of_refs<T>::find(T& pElt) {
    return vector_of_refs<T>::iterator(_elts.find(&pElt));
}

template<typename T>
typename vector_of_refs<T>::iterator vector_of_refs<T>::erase(const typename vector_of_refs<T>::iterator& pIt) {
    return vector_of_refs<T>::iterator(_elts.erase(pIt._it));
}

template<typename T>
typename vector_of_refs<T>::iterator vector_of_refs<T>::erase(const typename vector_of_refs<T>::const_iterator& pIt) {
    return vector_of_refs<T>::iterator(_elts.erase(pIt._it));
}

template<typename T>
void vector_of_refs<T>::erase(const T& pElt) {
    _elts.erase(const_cast<T*>(&pElt));    // very rare case where I think it's ok to do a const_cast
}

template<typename T>
typename vector_of_refs<T>::const_iterator vector_of_refs<T>::begin() const {
    return vector_of_refs<T>::const_iterator(_elts.begin());
}

template<typename T>
typename vector_of_refs<T>::const_iterator vector_of_refs<T>::end() const {
    return vector_of_refs<T>::const_iterator(_elts.end());
}

template<typename T>
typename vector_of_refs<T>::const_iterator vector_of_refs<T>::cbegin() const {
    return vector_of_refs<T>::const_iterator(_elts.begin());
}

template<typename T>
typename vector_of_refs<T>::const_iterator vector_of_refs<T>::cend() const {
    return vector_of_refs<T>::const_iterator(_elts.end());
}

template<typename T>
typename vector_of_refs<T>::const_iterator vector_of_refs<T>::find(const T& pElt) const {
    return vector_of_refs<T>::const_iterator(
        _elts.find(const_cast<T*>(&pElt)));    // very rare case where I think it's ok to do a const_cast
}

}    // End of namespace mystd
}    // End of namespace onsem

#endif    // ONSEM_COMMON_UTILITY_VECTOR_OF_REFS_HPP
