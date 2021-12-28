#ifndef ONSEM_CHATBOTPLANNER_GOAL_HPP
#define ONSEM_CHATBOTPLANNER_GOAL_HPP

#include <string>
#include "fact.hpp"

namespace onsem
{
namespace cp
{

struct Goal
{
  Goal(const std::string& pStr);

  bool operator==(const Goal& pOther) const;
  bool operator!=(const Goal& pOther) const { return !operator==(pOther); }

  std::string toStr() const;
  bool isPersistent() const { return _isPersistent; }
  const Fact& fact() const { return _fact; }

private:
  bool _isPersistent;
  Fact _fact;
};

} // !cp
} // !onsem


#endif // ONSEM_CHATBOTPLANNER_GOAL_HPP
