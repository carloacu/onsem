#ifndef LINGDBVIEWER_TIMECHECKER_H
#define LINGDBVIEWER_TIMECHECKER_H

#include <string>
#include <list>
#include <chrono>
#include "api.hpp"

namespace onsem
{

class ONSEMSEMANTICDEBUGGER_API TimeChecker
{
public:
  TimeChecker();

  void beginOfTimeSlot();

  void endOfTimeSlot(const std::string& pName);

  void printBilanOfTimeSlots(std::string& pBilan) const;
  void ioPrintBilanOfTimeSlots() const;
  void concatenate(const TimeChecker& pOther);

  void clear();

private:
  struct TimeSlot
  {
    TimeSlot();
    TimeSlot(std::string pName,
             std::int_least64_t pTimeInMs);

    std::string name;
    std::int_least64_t timeInMicro;
  };
  std::list<TimeSlot> _timeSlots;
  std::chrono::system_clock::time_point _currTimeSlotStart;
};

} // End of namespace onsem

#endif // LINGDBVIEWER_TIMECHECKER_H
