#ifndef INCLUDE_CONTEXTUALPLANNER_GOAL_HPP
#define INCLUDE_CONTEXTUALPLANNER_GOAL_HPP

#include <memory>
#include <string>
#include "fact.hpp"
#include "api.hpp"



namespace cp
{

struct CONTEXTUALPLANNER_API Goal
{
  Goal(const std::string& pStr);
  Goal(const Goal& pOther);

  void operator=(const Goal& pOther);
  bool operator==(const Goal& pOther) const;
  bool operator!=(const Goal& pOther) const { return !operator==(pOther); }

  std::string toStr() const;
  bool isPersistent() const { return _isPersistent; }
  const Fact* conditionFactPtr() const { return _conditionFactPtr ? &*_conditionFactPtr : nullptr; }
  const Fact& fact() const { return _fact; }

  static const std::string persistFunctionName;
  static const std::string implyFunctionName;

private:
  bool _isPersistent;
  std::unique_ptr<Fact> _conditionFactPtr;
  Fact _fact;
};

} // !cp


#endif // INCLUDE_CONTEXTUALPLANNER_GOAL_HPP
