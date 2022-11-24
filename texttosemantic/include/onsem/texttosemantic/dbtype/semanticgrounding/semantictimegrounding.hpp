#ifndef ONSEM_TEXTTOSEMANTIC_DBTYPE_SEMANTICGROUNDING_SEMANTICTIMEGROUNDINGS_HPP
#define ONSEM_TEXTTOSEMANTIC_DBTYPE_SEMANTICGROUNDING_SEMANTICTIMEGROUNDINGS_HPP

#include <list>
#include <mutex>
#include <onsem/common/utility/optional.hpp>
#include <onsem/common/enum/timeweekdayenum.hpp>
#include <onsem/common/enum/semanticverbtense.hpp>
#include "semanticdurationgrounding.hpp"
#include "../../api.hpp"


namespace onsem
{


struct ONSEM_TEXTTOSEMANTIC_API SemanticDate
{
  bool operator==(const SemanticDate& pOther) const;
  bool operator!=(const SemanticDate& pOther) const { return !operator==(pOther); }
  bool operator<(const SemanticDate& pOther) const;
  TimeWeekdayEnum getWeekDay() const;
  void moveToAWeekdayEqualOrBeforeToday(TimeWeekdayEnum pWeekday);
  bool isTheSameDay(const SemanticDate& pOther) const;
  bool empty() const { return !year.has_value() && !month.has_value() && !day.has_value(); }
  int getAge() const;
  void prettyPrint(std::stringstream& pSs) const;
  void moveOfANumberOfDaysInFuture(std::size_t pNumberOfDaysOffsets);
  void moveOfANumberOfDaysInPast(std::size_t pNumberOfDaysOffsets);
  void dayEqualToAWeekDayOfThisWeek(TimeWeekdayEnum pWeekday);
  void getCorrespondingDuration(SemanticDuration& pDuration) const;
  std::string getDayConcept() const;
  std::string getYearConcept() const;
  static SemanticDate now();

  mystd::optional<int> year;
  mystd::optional<int> month;
  mystd::optional<int> day;

  static std::unique_ptr<SemanticDate> hardCodedCurrentDate;

private:
  void _equalToNow();
};


struct ONSEM_TEXTTOSEMANTIC_API SemanticTimeGrounding : public SemanticGrounding
{
  SemanticTimeGrounding()
    : SemanticGrounding(SemanticGroundingType::TIME),
      date(),
      timeOfDay(),
      length(),
      fromConcepts()
  {
  }

  const SemanticTimeGrounding& getTimeGrounding() const override { return *this; }
  SemanticTimeGrounding& getTimeGrounding() override { return *this; }
  const SemanticTimeGrounding* getTimeGroundingPtr() const override { return this; }
  SemanticTimeGrounding* getTimeGroundingPtr() override { return this; }

  static std::unique_ptr<SemanticTimeGrounding> nowInstance();

  bool operator==(const SemanticTimeGrounding& pOther) const;
  bool operator<(const SemanticTimeGrounding& pOther) const;
  bool operator<=(const SemanticTimeGrounding& pOther) const;
  bool isEqual(const SemanticTimeGrounding& pOther) const;

  bool isAfter(const SemanticTimeGrounding& pOther) const;
  void equalToNow();
  void setEndOfThisTimeNow();
  void getBeginInDurationStruct(SemanticDuration& pDuration) const;
  bool modifyTimeGrdAccordingToADayPart(const std::string& pConceptStr);
  bool tryToConvertToADayConcept(std::string& pRelativedayConcept,
                                 SemanticTimeGrounding& pRefToday) const;
  bool tryToConvertToATimeConcept(std::string& pRelativedayConcept,
                                  SemanticTimeGrounding& pRefTime) const;
  bool isEqualMoreOrLess10Seconds(const SemanticTimeGrounding& pOther) const;
  void mergeWith(const SemanticTimeGrounding& pOther);

  static std::unique_ptr<SemanticTimeGrounding> nowPtr();
  static SemanticTimeGrounding now();
  static SemanticDuration relativeToAbsolute(const SemanticDuration& pRelativeDuration);
  static SemanticDuration absoluteToRelative(const SemanticDuration& pAbsoluteDuration);

  SemanticDate date;
  SemanticDuration timeOfDay;
  SemanticDuration length;
  std::map<std::string, char> fromConcepts;

  static std::unique_ptr<SemanticDuration> hardCodedCurrentTimeOfDay;

  static std::unique_ptr<int> hardCodedLessThanASecondNextValue;
  static int lessThanASecondNextValue;
  static void setAnHardCodedTimeElts(bool pDateAndTimeOfTheDay,
                                     bool pLessThanASecond);


private:
  static std::mutex _lastGivenTimeMutex;
  static std::unique_ptr<SemanticDuration> _lastGivenTime;

  static void _setToNow(SemanticDate& pDate,
                        SemanticDuration& pTimeDuration);
};


} // End of namespace onsem

#endif // ONSEM_TEXTTOSEMANTIC_DBTYPE_SEMANTICGROUNDING_SEMANTICTIMEGROUNDINGS_HPP
