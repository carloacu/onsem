#ifndef ONSEM_COMMON_UTILITY_UNIQUEPROPAGATECONST_HPP
#define ONSEM_COMMON_UTILITY_UNIQUEPROPAGATECONST_HPP

#include <memory>
#include <onsem/common/utility/make_unique.hpp>

namespace onsem
{

namespace mystd
{


template<typename T>
class unique_propagate_const
{
public:
  unique_propagate_const();
  unique_propagate_const(std::unique_ptr<T> pPtr);

  unique_propagate_const(unique_propagate_const&& pOther);
  unique_propagate_const& operator=(unique_propagate_const&& pOther);
  template<typename... Args>
  unique_propagate_const(Args&&... args);
  template<typename... Args>
  unique_propagate_const& operator=(Args&&... args);

  unique_propagate_const(const unique_propagate_const&) = delete;
  unique_propagate_const& operator=(const unique_propagate_const&) = delete;

  operator bool() const noexcept { return _ptr.operator bool(); }
  T* operator->() { return &*_ptr; }
  T& operator*() { return *_ptr; }
  const T* operator->() const { return &*_ptr; }
  const T& operator*() const { return *_ptr; }
  template<typename... Args>
  void emplace(Args&&... args);
  bool has_value() const;
  void reset();

private:
  std::unique_ptr<T> _ptr;
};


template<typename T, typename... Args>
unique_propagate_const<T> make_unique_pc(Args&&... args)
{
  return unique_propagate_const<T>(onsem::mystd::make_unique<T>(std::forward<Args>(args)...));
}



template<typename T>
unique_propagate_const<T>::unique_propagate_const()
  : _ptr()
{
}

template<typename T>
unique_propagate_const<T>::unique_propagate_const(std::unique_ptr<T> pPtr)
  : _ptr(std::move(pPtr))
{
}


template<typename T>
unique_propagate_const<T>::unique_propagate_const(unique_propagate_const&& pOther)
  : _ptr(std::move(pOther._ptr))
{
}


template<typename T>
template<typename... Args>
unique_propagate_const<T>::unique_propagate_const(Args&&... args)
  : _ptr(onsem::mystd::make_unique<T>(std::forward<Args>(args)...))
{
}

template<typename T>
template<typename... Args>
unique_propagate_const<T>& unique_propagate_const<T>::operator=(Args&&... args)
{
  _ptr = onsem::mystd::make_unique<T>(std::forward<Args>(args)...);
  return *this;
}


template<typename T>
template<typename... Args>
void unique_propagate_const<T>::emplace(Args&&... args)
{
  _ptr = onsem::mystd::make_unique<T>(std::forward<Args>(args)...);
}

template<typename T>
bool unique_propagate_const<T>::has_value() const
{
  return _ptr.operator bool();
}


template<typename T>
unique_propagate_const<T>& unique_propagate_const<T>::operator=(unique_propagate_const&& pOther)
{
  _ptr = std::move(pOther._ptr);
  return *this;
}


template<typename T>
void unique_propagate_const<T>::reset()
{
  _ptr.reset();
}

} // End of namespace mystd
} // End of namespace onsem



#endif // ONSEM_COMMON_UTILITY_UNIQUEPROPAGATECONST_HPP
