#ifndef ONSEM_COMMON_UTILITY_SIGNALS_SIGNAL_HPP
#define ONSEM_COMMON_UTILITY_SIGNALS_SIGNAL_HPP

#include <list>
#include <map>
#include <iostream>
#include <functional>
#include <onsem/common/utility/observable/connection.hpp>
#include <mutex>

namespace onsem
{
namespace mystd
{
namespace observable
{

// /!\ This implemenation of signal is not thread safe!
template<typename FuncSignature>
class Observable
{
public:
  Observable();

  Connection connect(std::function<FuncSignature>&& pFunction) const;
  void disconnect(const Connection& pConnection) const;

  template<typename... Args>
  void operator()(Args&&... pArgs);

private:
  mutable std::mutex _mutex;
  mutable std::map<int, std::pair<Connection, std::function<FuncSignature>>> _connections;
  void _disconnectId(int pId) const;
};





template<typename FuncSignature>
Observable<FuncSignature>::Observable()
  : _mutex(),
    _connections()
{
}


template<typename FuncSignature>
Connection Observable<FuncSignature>::connect(std::function<FuncSignature>&& pFunction) const
{
  std::lock_guard<std::mutex> lock(_mutex);
  int id = _connections.empty() ? 0 : _connections.rbegin()->first + 1;
  Connection c(id, [this, id] { _disconnectId(id); });
  _connections.emplace(id, std::pair<Connection, std::function<FuncSignature>>(c, pFunction));
  return c;
}


template<typename FuncSignature>
void Observable<FuncSignature>::disconnect(const Connection& pConnection) const
{
  std::lock_guard<std::mutex> lock(_mutex);
  _disconnectId(pConnection.getId());
}


template<typename FuncSignature>
void Observable<FuncSignature>::_disconnectId(int pId) const
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
void Observable<FuncSignature>::operator()(Args&&... pArgs)
{
  std::list<std::function<FuncSignature>> functions;
  {
    std::lock_guard<std::mutex> lock(_mutex);
    for (auto& currConnection : _connections)
      functions.emplace_back(currConnection.second.second);
  }
  for (auto& currFunction : functions)
    currFunction(std::forward<Args>(pArgs)...);
}


} // End of namespace observable
} // End of namespace mystd
} // End of namespace onsem



#endif // ONSEM_COMMON_UTILITY_SIGNALS_SIGNAL_HPP
