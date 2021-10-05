#ifndef ONSEM_COMMON_UTILITY_SIGNALS_SIGNAL_UNSAFE_HPP
#define ONSEM_COMMON_UTILITY_SIGNALS_SIGNAL_UNSAFE_HPP

#include <map>
#include <iostream>
#include <functional>
#include <onsem/common/utility/observable/connection.hpp>

namespace onsem
{
namespace mystd
{
namespace observable
{

// /!\ This implemenation of signal is not thread safe!
template<typename FuncSignature>
class ObservableUnsafe
{
public:
  ObservableUnsafe();

  Connection connectUnsafe(std::function<FuncSignature>&& pFunction) const;
  void disconnectUnsafe(const Connection& pConnection) const;

  template<typename... Args>
  void operator()(Args&&... pArgs);

private:
  mutable std::map<int, std::pair<Connection, std::function<FuncSignature>>> _connections;
  void _disconnectId(int pId) const;
};





template<typename FuncSignature>
ObservableUnsafe<FuncSignature>::ObservableUnsafe()
  : _connections()
{
}


template<typename FuncSignature>
Connection ObservableUnsafe<FuncSignature>::connectUnsafe(std::function<FuncSignature>&& pFunction) const
{
  int id = _connections.empty() ? 0 : _connections.rbegin()->first + 1;
  Connection c(id, [this, id] { _disconnectId(id); });
  _connections.emplace(id, std::pair<Connection, std::function<FuncSignature>>(c, pFunction));
  return c;
}


template<typename FuncSignature>
void ObservableUnsafe<FuncSignature>::disconnectUnsafe(const Connection& pConnection) const
{
  _disconnectId(pConnection.getId());
}


template<typename FuncSignature>
void ObservableUnsafe<FuncSignature>::_disconnectId(int pId) const
{
  auto it = _connections.find(pId);
  if (it == _connections.end())
  {
    std::cerr << "Try to disconnect a connection that does not exist!" << std::endl;
    return;
  }
  _connections.erase(it);
}


template<typename FuncSignature>
template<typename... Args>
void ObservableUnsafe<FuncSignature>::operator()(Args&&... pArgs)
{
  for (auto& currConnection : _connections)
    currConnection.second.second(std::forward<Args>(pArgs)...);
}


} // End of namespace observable
} // End of namespace mystd
} // End of namespace onsem



#endif // ONSEM_COMMON_UTILITY_SIGNALS_SIGNAL_UNSAFE_HPP
