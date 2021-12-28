#include <onsem/chatbotplanner/goal.hpp>

namespace onsem
{
namespace cp
{


Goal::Goal(const std::string& pStr)
  : _isPersistent(false),
    _fact(Fact::fromStr(pStr))
{
}

bool Goal::operator==(const Goal& pOther) const
{
  return _isPersistent == pOther._isPersistent &&
      _fact == pOther._fact;
}


std::string Goal::toStr() const
{
  return _fact.toStr();
}


} // !cp
} // !onsem
