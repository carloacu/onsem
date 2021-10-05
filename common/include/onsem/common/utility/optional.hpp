#ifndef ONSEM_COMMON_UTILITY_OPTIONAL_HPP
#define ONSEM_COMMON_UTILITY_OPTIONAL_HPP

#include <memory>
#include <utility>
#include <assert.h>
#include <onsem/common/utility/make_unique.hpp>

namespace onsem
{

namespace mystd
{


template<typename T>
struct optional
{
  explicit optional()
    : _val(nullptr)
  {
  }

  template<typename... Args>
  static optional<T> make_optional(Args&&... args)
  {
    optional<T> res;
    res.emplace(std::forward<Args>(args)...);
    return res;
  }

  optional(const optional<T>& pOther, typename std::enable_if<std::is_copy_constructible<T>::value>::type* = 0)
    : _val()
  {
    if (pOther._val)
      _val = mystd::make_unique<T>(*pOther._val);
  }


  optional(const T& pOther)
    : _val(mystd::make_unique<T>(pOther))
  {
  }

  optional(optional&& pOther)
  {
    _val = std::move(pOther._val);
  }
  optional& operator=(optional&& pOther)
  {
    _val = std::move(pOther._val);
    return *this;
  }

  void operator=(const optional<T>& pOther)
  {
    if (pOther._val)
      _val = mystd::make_unique<T>(*pOther._val);
    else
      _val.reset();
  }

  bool operator<(const optional<T>& pOther) const
  {
    if (_val && pOther._val)
      return *_val < *pOther._val;
    return !_val && pOther._val;
  }
  bool operator<=(const optional<T>& pOther) const
  {
    return _val < pOther._val || _val == pOther._val;
  }
  bool operator>(const optional<T>& pOther) const
  {
    if (_val && pOther._val)
      return *_val > *pOther._val;
    return _val && !pOther._val;
  }
  bool operator>=(const optional<T>& pOther) const
  {
    return _val > pOther._val || _val == pOther._val;
  }

  template<typename... Args>
  void emplace(Args&&... args)
  {
    _val = mystd::make_unique<T>(std::forward<Args>(args)...);
  }

  void reset()
  {
    _val.reset();
  }

  bool has_value() const
  {
    return _val.operator bool();
  }

  const T& value() const
  {
    return *_val;
  }

  operator bool() const
  {
    return _val.operator bool();
  }

  T* operator->()
  {
    return &*_val;
  }

  const T* operator->() const
  {
    return &*_val;
  }

  const T& operator*() const
  {
    assert(_val);
    return *_val;
  }

  T& operator*()
  {
    assert(_val);
    return *_val;
  }

  bool operator==(const optional& pOther) const
  {
    if (!_val && !pOther)
      return true;
    if (_val && pOther)
      return *_val == *pOther;
    return false;
  }
  bool operator!=(const optional& pOther) const
  {
    return !operator==(pOther);
  }

private:
  std::unique_ptr<T> _val;
};



template<typename T>
bool operator==(const optional<T>& pOpt,
                const T& pOther)
{
  if (pOpt.has_value())
    return *pOpt == pOther;
  return false;
}

template<typename T>
bool operator==(const T& pOther,
                const optional<T>& pOpt)
{
  if (pOpt.has_value())
    return *pOpt == pOther;
  return false;
}

template<typename T>
bool operator!=(const optional<T>& pOpt,
                const T& pOther)
{
  return !operator==(pOpt, pOther);
}

template<typename T>
bool operator!=(const T& pOther,
                const optional<T>& pOpt)
{
  return !operator==(pOther, pOpt);
}


} // End of namespace mystd
} // End of namespace onsem


#endif // ONSEM_COMMON_UTILITY_OPTIONAL_HPP
