#include <onsem/semanticdebugger/timechecker.hpp>
#include <assert.h>
#include <iostream>
#include <sstream>

namespace onsem
{


TimeChecker::TimeChecker()
  : _timeSlots(),
    _currTimeSlotStart()
{
}



void TimeChecker::beginOfTimeSlot()
{
  _currTimeSlotStart = std::chrono::system_clock::time_point
      (std::chrono::system_clock::now().time_since_epoch());
}



void TimeChecker::endOfTimeSlot
(const std::string& pName)
{
  std::chrono::system_clock::time_point currTimeSlotEnd
      (std::chrono::system_clock::now().time_since_epoch());
  std::chrono::microseconds millisecTime =
      std::chrono::duration_cast<std::chrono::microseconds>(currTimeSlotEnd - _currTimeSlotStart);
  _timeSlots.emplace_back(pName, millisecTime.count());
}


void _microToStr(std::stringstream& pSs,
                 std::int_least64_t pTimeInMicro)
{
  std::int_least64_t timeInMilli = pTimeInMicro / 1000;
  if (timeInMilli == 0)
  {
    pSs << timeInMilli << " ms";
    return;
  }
  if (timeInMilli >= 1000)
  {
    std::int_least64_t timeInSecond = timeInMilli / 1000;
    timeInMilli %= 1000;
    if (timeInSecond >= 60)
    {
      std::int_least64_t timeInMinute = timeInSecond / 60;
      timeInSecond %= 60;
      if (timeInMinute >= 60)
      {
        std::int_least64_t timeInHour = timeInMinute / 60;
        timeInMinute %= 60;
        pSs << timeInHour << " h ";
      }
      if (timeInMinute > 0)
        pSs << timeInMinute << " min ";
    }
    if (timeInSecond > 0)
      pSs << timeInSecond << " s ";
  }
  if (timeInMilli > 0)
    pSs << timeInMilli << " ms";
}


void TimeChecker::printBilanOfTimeSlots(std::string& pBilan) const
{
  std::int_least64_t totalTimeMicro = 0;
  for (const auto& currTS : _timeSlots)
  {
    totalTimeMicro += currTS.timeInMicro;
    std::stringstream ss;
    ss << currTS.name << ": \t";
    _microToStr(ss, currTS.timeInMicro);
    ss << "\n";
    pBilan += ss.str();
  }
  if (_timeSlots.size() > 1)
  {
    std::stringstream ss;
    ss << "-------------------------\n";
    ss << "Total: \t";
    _microToStr(ss, totalTimeMicro);
    ss << "\n";
    pBilan += ss.str();
  }
}

void TimeChecker::ioPrintBilanOfTimeSlots() const
{
  std::string bilan;
  printBilanOfTimeSlots(bilan);
  std::cout << bilan << std::endl;
}


void TimeChecker::concatenate(const TimeChecker& pOther)
{
  if (_timeSlots.size() == pOther._timeSlots.size())
  {
    auto otherItTS = pOther._timeSlots.begin();
    for (auto itTS = _timeSlots.begin(); itTS != _timeSlots.end(); ++itTS)
    {
      itTS->timeInMicro += otherItTS->timeInMicro;
      ++otherItTS;
    }
  }
  else
  {
    assert(false);
  }
}

void TimeChecker::clear()
{
  _timeSlots.clear();
}


TimeChecker::TimeSlot::TimeSlot()
 : name(),
   timeInMicro(0)
{
}

TimeChecker::TimeSlot::TimeSlot
(std::string pName,
 std::int_least64_t pTimeInMs)
  : name(pName),
    timeInMicro(pTimeInMs)
{
}



} // End of namespace onsem
