#include <onsem/chatbotplanner/goal.hpp>

namespace onsem
{
namespace cp
{
const std::string Goal::persistFunctionName = "persist";


Goal::Goal(const std::string& pStr)
  : _isPersistent(false),
    _fact(Fact::fromStr(pStr))
{
  if (_fact.name == persistFunctionName &&
      _fact.parameters.size() == 1 &&
      _fact.value.empty())
  {
    _isPersistent = true;
    _fact = _fact.parameters.front();
  }
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
