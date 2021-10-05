#ifndef ONSEM_COMMON_TYPE_SEMANTICGROUNDING_TIMEWEEKDAYENUM_H
#define ONSEM_COMMON_TYPE_SEMANTICGROUNDING_TIMEWEEKDAYENUM_H

#include <string>
#include <map>
#include <assert.h>


namespace onsem
{

#define SEMANTICTIME_WEEKDAY_TABLE                                         \
  ADD_SEMANTICTIME_WEEKDAY_TYPE(SUNDAY, "time_weekday_sunday")             \
  ADD_SEMANTICTIME_WEEKDAY_TYPE(MONDAY, "time_weekday_monday")             \
  ADD_SEMANTICTIME_WEEKDAY_TYPE(TUESDAY, "time_weekday_tuesday")           \
  ADD_SEMANTICTIME_WEEKDAY_TYPE(WEDNESDAY, "time_weekday_wednesday")       \
  ADD_SEMANTICTIME_WEEKDAY_TYPE(THURSDAY, "time_weekday_thursday")         \
  ADD_SEMANTICTIME_WEEKDAY_TYPE(FRIDAY, "time_weekday_friday")             \
  ADD_SEMANTICTIME_WEEKDAY_TYPE(SATURDAY, "time_weekday_saturday")         \
  ADD_SEMANTICTIME_WEEKDAY_TYPE(UNKNOWN, "unknown")



#define ADD_SEMANTICTIME_WEEKDAY_TYPE(a, b) a,
enum class TimeWeekdayEnum {
  SEMANTICTIME_WEEKDAY_TABLE
};
#undef ADD_SEMANTICTIME_WEEKDAY_TYPE


#define ADD_SEMANTICTIME_WEEKDAY_TYPE(a, b) {TimeWeekdayEnum::a, b},
static const std::map<TimeWeekdayEnum, std::string> _semanticTimeWeekdayEnum_toStr = {
  SEMANTICTIME_WEEKDAY_TABLE
};
#undef ADD_SEMANTICTIME_WEEKDAY_TYPE


#define ADD_SEMANTICTIME_WEEKDAY_TYPE(a, b) {b, TimeWeekdayEnum::a},
static const std::map<std::string, TimeWeekdayEnum> _semanticTimeWeekdayEnum_fromStr = {
  SEMANTICTIME_WEEKDAY_TABLE
};
#undef ADD_SEMANTICTIME_WEEKDAY_TYPE


static inline TimeWeekdayEnum getNextWeekDay
(TimeWeekdayEnum pWeekDay)
{
  assert(pWeekDay != TimeWeekdayEnum::UNKNOWN);
  auto res = static_cast<TimeWeekdayEnum>(static_cast<int>(pWeekDay) + 1);
  if (res == TimeWeekdayEnum::UNKNOWN)
  {
    return TimeWeekdayEnum::SUNDAY;
  }
  return res;
}

static inline TimeWeekdayEnum getPrevWeekDay
(TimeWeekdayEnum pWeekDay)
{
  if (pWeekDay == TimeWeekdayEnum::SUNDAY)
  {
    return TimeWeekdayEnum::SATURDAY;
  }
  return TimeWeekdayEnum(static_cast<int>(pWeekDay) - 1);
}



static inline std::string semanticTimeWeekdayEnum_toStr
(TimeWeekdayEnum pWeekDay)
{
  return _semanticTimeWeekdayEnum_toStr.find(pWeekDay)->second;
}



static inline TimeWeekdayEnum semanticTimeWeekdayEnum_fromStr
(const std::string& pWeekDayStr)
{
  auto it = _semanticTimeWeekdayEnum_fromStr.find(pWeekDayStr);
  if (it != _semanticTimeWeekdayEnum_fromStr.end())
  {
    return it->second;
  }
  assert(false);
  return TimeWeekdayEnum::UNKNOWN;
}


} // End of namespace onsem

#endif // ONSEM_COMMON_TYPE_SEMANTICGROUNDING_TIMEWEEKDAYENUM_H
