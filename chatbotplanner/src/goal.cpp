#include <onsem/chatbotplanner/goal.hpp>

namespace onsem
{
namespace cp
{
const std::string Goal::persistFunctionName = "persist";
const std::string Goal::forallFunctionName = "forall";
const std::string Goal::implyFunctionName = "imply";


Goal::Goal(const std::string& pStr)
  : _isPersistent(false),
    _parameter(),
    _conditionFactPtr(),
    _fact(Fact::fromStr(pStr))
{
  if (_fact.name == persistFunctionName &&
      _fact.parameters.size() == 1 &&
      _fact.value.empty())
  {
    _isPersistent = true;
    _fact = _fact.parameters.front();
  }

  if (_fact.name == forallFunctionName &&
      _fact.parameters.size() == 2 &&
      _fact.parameters[0].parameters.empty() &&
      _fact.parameters[0].value.empty() &&
      _fact.value.empty())
  {
    _parameter = _fact.parameters[0].name;
    _fact = _fact.parameters[1];
  }

  if (_fact.name == implyFunctionName &&
      _fact.parameters.size() == 2 &&
      _fact.value.empty())
  {
    _conditionFactPtr = std::unique_ptr<Fact>(new Fact(_fact.parameters[0]));
    _fact = _fact.parameters[1];
  }
}

Goal::Goal(const Goal& pOther)
  : _isPersistent(pOther._isPersistent),
    _parameter(pOther._parameter),
    _conditionFactPtr(pOther._conditionFactPtr ? std::unique_ptr<Fact>(new Fact(*pOther._conditionFactPtr)) : std::unique_ptr<Fact>()),
    _fact(pOther._fact)
{
}

void Goal::operator=(const Goal& pOther)
{
  _isPersistent = pOther._isPersistent;
  _parameter = pOther._parameter;
  _conditionFactPtr = pOther._conditionFactPtr ? std::unique_ptr<Fact>(new Fact(*pOther._conditionFactPtr)) : std::unique_ptr<Fact>();
  _fact = pOther._fact;
}

bool Goal::operator==(const Goal& pOther) const
{
  return _isPersistent == pOther._isPersistent &&
      _parameter == pOther._parameter &&
      _conditionFactPtr == pOther._conditionFactPtr &&
      _fact == pOther._fact;
}


std::string Goal::toStr() const
{
  return _fact.toStr();
}


} // !cp
} // !onsem
