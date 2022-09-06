#include <gtest/gtest.h>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semantictimegrounding.hpp>

using namespace onsem;


TEST(SemanticBase, check_dates)
{
  static const unsigned int nbOfDecalages = 10;
  SemanticDate date;
  date.year.emplace(2016);
  date.month.emplace(2);
  date.day.emplace(21);

  TimeWeekdayEnum currWeekDay = TimeWeekdayEnum::SUNDAY;
  ASSERT_EQ(currWeekDay, date.getWeekDay());

  for (unsigned int i = 0; i < nbOfDecalages; ++i)
  {
    currWeekDay = getNextWeekDay(currWeekDay);
    date.moveOfANumberOfDaysInFuture(1);
    ASSERT_EQ(currWeekDay, date.getWeekDay());
  }

  date.moveOfANumberOfDaysInPast(nbOfDecalages);
  currWeekDay = TimeWeekdayEnum::SUNDAY;
  ASSERT_EQ(2016, date.year);
  ASSERT_EQ(2, date.month);
  ASSERT_EQ(21, date.day);

  for (unsigned int i = 0; i < nbOfDecalages; ++i)
  {
    currWeekDay = getPrevWeekDay(currWeekDay);
    date.moveOfANumberOfDaysInPast(1);
    ASSERT_EQ(currWeekDay, date.getWeekDay());
  }
}
