#ifndef ONSEM_COMMON_UTILITY_MAKEUNIQUE_HPP
#define ONSEM_COMMON_UTILITY_MAKEUNIQUE_HPP

#include <memory>

namespace onsem
{

namespace mystd
{
template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args)
{
  return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}
}


} // End of namespace onsem


#endif // ONSEM_COMMON_UTILITY_MAKEUNIQUE_HPP
