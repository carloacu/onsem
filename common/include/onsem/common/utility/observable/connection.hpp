#ifndef ONSEM_COMMON_UTILITY_SIGNALS_CONNECTION_HPP
#define ONSEM_COMMON_UTILITY_SIGNALS_CONNECTION_HPP

#include <functional>

namespace onsem
{
namespace mystd
{
namespace observable
{

struct Connection
{
  Connection(): _id(-1), _disconnect() {}
  Connection(int pId,
             std::function<void()>&& pDisconnect): _id(pId), _disconnect(pDisconnect) {}
  int getId() const { return _id; }
  void disconnect() { if (_id != -1) _disconnect(); }

private:
  int _id;
  std::function<void()> _disconnect;
};


} // End of namespace observable
} // End of namespace mystd
} // End of namespace onsem



#endif // ONSEM_COMMON_UTILITY_SIGNALS_CONNECTION_HPP
