#include <onsem/texttosemantic/dbtype/semanticgrounding/semantictimegrounding.hpp>
#include <cmath>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace onsem {
std::unique_ptr<SemanticDate> SemanticDate::hardCodedCurrentDate;
std::unique_ptr<SemanticDuration> SemanticTimeGrounding::hardCodedCurrentTimeOfDay;
std::unique_ptr<int> SemanticTimeGrounding::hardCodedLessThanASecondNextValue;
int SemanticTimeGrounding::lessThanASecondNextValue = 0;
std::mutex SemanticTimeGrounding::_lastGivenTimeMutex;
std::unique_ptr<SemanticDuration> SemanticTimeGrounding::_lastGivenTime;

static const std::vector<int> number_of_days_per_month_for_not_leap_years =
    {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

bool _isALeapYear(int pYear) {
    if (pYear % 4 != 0)
        return false;
    if (pYear % 100 != 0)
        return true;
    if (pYear % 400 != 0)
        return false;
    return true;
}

int _weekDayCalculation_GetMonthId(SmanticTimeMonth pMonth, int pYear) {
    switch (pMonth) {
        case SmanticTimeMonth::JANUARY: return _isALeapYear(pYear) ? 6 : 0;
        case SmanticTimeMonth::FEBRUARY: return _isALeapYear(pYear) ? 2 : 3;
        case SmanticTimeMonth::MARCH: return 3;
        case SmanticTimeMonth::APRIL: return 6;
        case SmanticTimeMonth::MAY: return 1;
        case SmanticTimeMonth::JUNE: return 4;
        case SmanticTimeMonth::JULY: return 6;
        case SmanticTimeMonth::AUGUST: return 2;
        case SmanticTimeMonth::SEPTEMBER: return 5;
        case SmanticTimeMonth::OCTOBER: return 0;
        case SmanticTimeMonth::NOVEMBER: return 3;
        case SmanticTimeMonth::DECEMBER: return 5;
    }
    assert(false);
    return -1;
}

int _weekDayCalculation_GetCenturyNumber(int pYear) {
    if (pYear < 1700 || pYear >= 2500) {
        return -1;
    }
    int yearDividedByHundred = static_cast<int>(std::floor(pYear / 100.0));
    switch (yearDividedByHundred) {
        case 19:
        case 23: return 0;
        case 18:
        case 22: return 2;
        case 17:
        case 21: return 4;
        case 20:
        case 24: return 6;
    }
    return -1;
}

bool SemanticDate::operator==(const SemanticDate& pOther) const {
    return year == pOther.year && month == pOther.month && day == pOther.day;
}

bool SemanticDate::operator<(const SemanticDate& pOther) const {
    if (year < pOther.year)
        return true;
    if (year > pOther.year)
        return false;
    if (month < pOther.month)
        return true;
    if (month > pOther.month)
        return false;
    return day < pOther.day;
}

TimeWeekdayEnum SemanticDate::getWeekDay() const {
    if (!day || !month || !year)
        return TimeWeekdayEnum::UNKNOWN;

    int monthId = _weekDayCalculation_GetMonthId(semanticTimeMonth_fromId(*month), *year);
    int centuryNumber = _weekDayCalculation_GetCenturyNumber(*year);
    if (monthId == -1 || centuryNumber == -1)
        return TimeWeekdayEnum::UNKNOWN;
    int yearLastTwoDigits = (*year) % 100;
    int yearLastTwoDigitsDivByFour = static_cast<int>(std::floor(yearLastTwoDigits / 4));
    return TimeWeekdayEnum(((*day) + monthId + yearLastTwoDigits + yearLastTwoDigitsDivByFour + centuryNumber) % 7);
}

void SemanticDate::moveToAWeekdayEqualOrBeforeToday(TimeWeekdayEnum pWeekday) {
    if (pWeekday == TimeWeekdayEnum::UNKNOWN)
        return;
    TimeWeekdayEnum currentWeekday = getWeekDay();
    std::size_t nbOfDayInPast = 0;
    while (currentWeekday != pWeekday) {
        pWeekday = getPrevWeekDay(pWeekday);
        ++nbOfDayInPast;
    }
    moveOfANumberOfDaysInPast(nbOfDayInPast);
}

bool SemanticDate::isTheSameDay(const SemanticDate& pOther) const {
    return year && month && day && year == pOther.year && month == pOther.month && day == pOther.day;
}

int SemanticDate::getAge() const {
    if (!year)
        return 0;
    auto nowDate = now();
    if (!nowDate.year)
        return 0;

    int res = (*nowDate.year) - (*year);
    if (month && nowDate.month) {
        if (*month == *nowDate.month) {
            if (day && nowDate.day && *day > *nowDate.day)
                --res;
        } else if (*month > *nowDate.month) {
            --res;
        }
    }
    return res;
}

void _printNbWithSpecificNbOfDigits(std::stringstream& pSs, int pNbOfDigits, int pNb) {
    pSs << std::setfill('0') << std::setw(pNbOfDigits) << pNb;
}

void SemanticDate::prettyPrint(std::stringstream& pSs) const {
    if (year || month || day) {
        pSs << "date(";
        if (!day)
            pSs << "??";
        else
            _printNbWithSpecificNbOfDigits(pSs, 2, *day);
        pSs << "/";
        if (!month)
            pSs << "??";
        else
            _printNbWithSpecificNbOfDigits(pSs, 2, *month);
        pSs << "/";
        if (!year)
            pSs << "????";
        else
            pSs << *year;
        pSs << ")";
    }
}

void SemanticDate::moveOfANumberOfDaysInFuture(std::size_t pNumberOfDaysOffsets) {
    if (!day)
        day.emplace(1);
    if (!month)
        month.emplace(1);
    if (!year)
        year.emplace(0);

    for (std::size_t i = 0; i < pNumberOfDaysOffsets; ++i) {
        auto number_of_days_per_month = number_of_days_per_month_for_not_leap_years;
        number_of_days_per_month[2] = _isALeapYear(*year) ? 29 : 28;
        ++(*day);

        if (*day > number_of_days_per_month[*month]) {
            day.emplace(1);
            ++(*month);
        }

        if (*month > 12) {
            month.emplace(1);
            ++(*year);
        }
    }
}

void SemanticDate::moveOfANumberOfDaysInPast(std::size_t pNumberOfDaysOffsets) {
    if (!day)
        day.emplace(1);
    if (!month)
        month.emplace(1);
    if (!year)
        year.emplace(0);

    for (std::size_t i = 0; i < pNumberOfDaysOffsets; ++i) {
        auto number_of_days_per_month = number_of_days_per_month_for_not_leap_years;
        number_of_days_per_month[2] = _isALeapYear(*year) ? 29 : 28;

        --(*day);
        if (*day < 1) {
            --(*month);
            if (*month < 1) {
                --(*year);
                month.emplace(12);
            }
            day.emplace(number_of_days_per_month[*month]);
        }
    }
}

void SemanticDate::dayEqualToAWeekDayOfThisWeek(TimeWeekdayEnum pWeekday, bool pShouldBeInPast) {
    _equalToNow();
    TimeWeekdayEnum currWekkDay = getWeekDay();

    auto getWeekDayOffset = [](TimeWeekdayEnum paramWeekDay) {
        int res = static_cast<int>(paramWeekDay) - 1;
        if (res == -1)
            return 6;
        return res;
    };

    int offsetToDo = getWeekDayOffset(pWeekday) - getWeekDayOffset(currWekkDay);

    if (pShouldBeInPast) {
        if (offsetToDo > 0)
            offsetToDo -= 7;
    } else {
        if (offsetToDo < 0)
            offsetToDo += 7;
    }

    if (offsetToDo > 0)
        moveOfANumberOfDaysInFuture(static_cast<std::size_t>(offsetToDo));
    else if (offsetToDo < 0)
        moveOfANumberOfDaysInPast(static_cast<std::size_t>(-offsetToDo));
}

SemanticDate SemanticDate::now() {
    SemanticDate res;
    res._equalToNow();
    return res;
}

void SemanticDate::_equalToNow()    // TODO: put it in private
{
    if (hardCodedCurrentDate) {
        *this = *hardCodedCurrentDate;
    } else {
        std::time_t t = std::time(nullptr);
        std::tm* tmPtr = std::localtime(&t);
        if (tmPtr == nullptr) {
            std::cerr << "Cannot get local time" << std::endl;
            return;
        }
        auto& tm = *tmPtr;
        year.emplace(1900 + tm.tm_year);
        month.emplace(1 + tm.tm_mon);
        day.emplace(tm.tm_mday);
    }
}

void SemanticDate::getCorrespondingDuration(SemanticDuration& pDuration) const {
    pDuration.sign = Sign::POSITIVE;
    if (day)
        pDuration.timeInfos[SemanticTimeUnity::DAY] += *day;
    if (month)
        pDuration.timeInfos[SemanticTimeUnity::MONTH] += *month;
    if (year)
        pDuration.timeInfos[SemanticTimeUnity::YEAR] += *year;
}

std::string SemanticDate::getDayConcept() const {
    if (!day)
        return "";
    std::stringstream dayCpt;
    dayCpt << "time_dayNb_" << *day;
    return dayCpt.str();
}

std::string SemanticDate::getYearConcept() const {
    if (!year)
        return "";
    std::stringstream yearCpt;
    yearCpt << "time_year_" << *year;
    return yearCpt.str();
}

std::unique_ptr<SemanticTimeGrounding> SemanticTimeGrounding::nowInstance() {
    auto timeGrd = std::make_unique<SemanticTimeGrounding>();
    timeGrd->equalToNow();
    return timeGrd;
}

bool SemanticTimeGrounding::operator==(const SemanticTimeGrounding& pOther) const {
    return this->isEqual(pOther);
}

bool SemanticTimeGrounding::operator<(const SemanticTimeGrounding& pOther) const {
    if (date != pOther.date)
        return date < pOther.date;
    if (timeOfDay != pOther.timeOfDay)
        return timeOfDay < pOther.timeOfDay;
    return length < pOther.length;
}

bool SemanticTimeGrounding::operator<=(const SemanticTimeGrounding& pOther) const {
    if (date != pOther.date)
        return date < pOther.date;
    if (timeOfDay != pOther.timeOfDay)
        return timeOfDay < pOther.timeOfDay;
    return length <= pOther.length;
}

bool SemanticTimeGrounding::isEqual(const SemanticTimeGrounding& pOther) const {
    return _isMotherClassEqual(pOther) && date == pOther.date && timeOfDay == pOther.timeOfDay
        && length == pOther.length && fromConcepts == pOther.fromConcepts;
}

bool SemanticTimeGrounding::isAfter(const SemanticTimeGrounding& pOther) const {
    if (pOther.date < date)
        return true;
    if (date == pOther.date)
        return (pOther.timeOfDay + pOther.length) < timeOfDay;
    return false;
}

void SemanticTimeGrounding::equalToNow() {
    _setToNow(date, timeOfDay);
}

void SemanticTimeGrounding::setEndOfThisTimeNow() {
    SemanticDate semDate;
    SemanticDuration endTimeDuration;
    _setToNow(semDate, endTimeDuration);
    length = endTimeDuration - timeOfDay;
}

void SemanticTimeGrounding::getBeginInDurationStruct(SemanticDuration& pDuration) const {
    date.getCorrespondingDuration(pDuration);
    pDuration = pDuration + timeOfDay;
}

bool SemanticTimeGrounding::modifyTimeGrdAccordingToADayPart(const std::string& pConceptStr) {
    if (pConceptStr == "time_day_morning") {
        length.add(SemanticTimeUnity::HOUR, 12);
        return true;
    }
    if (pConceptStr == "time_day_noon") {
        timeOfDay.add(SemanticTimeUnity::HOUR, 12);
        length.add(SemanticTimeUnity::HOUR, 1);
        return true;
    }
    if (pConceptStr == "time_day_afternoon") {
        timeOfDay.add(SemanticTimeUnity::HOUR, 12);
        length.add(SemanticTimeUnity::HOUR, 12);
        return true;
    }
    if (pConceptStr == "time_day_evening") {
        timeOfDay.add(SemanticTimeUnity::HOUR, 18);
        length.add(SemanticTimeUnity::HOUR, 6);
        return true;
    }
    if (pConceptStr == "time_day_midnight") {
        date.moveOfANumberOfDaysInFuture(1);
        return true;
    }
    if (pConceptStr == "time_day_night") {
        date.moveOfANumberOfDaysInFuture(1);
        length.add(SemanticTimeUnity::HOUR, 6);
        return true;
    }
    return false;
}

bool SemanticTimeGrounding::tryToConvertToADayConcept(std::string& pRelativedayConcept,
                                                      SemanticTimeGrounding& pRefToday) const {
    static const std::vector<std::string> dayConcepts = {"time_day_morning",
                                                         "time_day_noon",
                                                         "time_day_afternoon",
                                                         "time_day_evening",
                                                         "time_day_midnight",
                                                         "time_day_night"};

    for (const auto& currDayConcept : dayConcepts) {
        SemanticTimeGrounding localTimeGrd;
        if (localTimeGrd.modifyTimeGrdAccordingToADayPart(currDayConcept)
            && localTimeGrd.timeOfDay.isNearlyEqual(timeOfDay) && localTimeGrd.length == length) {
            pRelativedayConcept = currDayConcept;
            if (currDayConcept == "time_day_midnight" || currDayConcept == "time_day_night")
                pRefToday.date.moveOfANumberOfDaysInFuture(1);
            return true;
        }
    }
    return false;
}

bool SemanticTimeGrounding::tryToConvertToATimeConcept(std::string& pRelativedayConcept,
                                                       SemanticTimeGrounding& pRefTime) const {
    if (date.isTheSameDay(pRefTime.date)) {
        if (pRefTime.length.isEmpty() && isEqualMoreOrLess10Seconds(pRefTime)) {
            pRelativedayConcept = "time_relative_now";
            return true;
        } else {
            pRelativedayConcept = "time_relativeDay_today";
            return true;
        }
    }

    pRefTime.date.moveOfANumberOfDaysInPast(1);
    if (date.isTheSameDay(pRefTime.date)) {
        pRelativedayConcept = "time_relativeDay_yesterday";
        return true;
    }

    pRefTime.date.moveOfANumberOfDaysInPast(1);
    if (date.isTheSameDay(pRefTime.date)) {
        pRelativedayConcept = "time_relativeDay_dayBeforeYesterday";
        return true;
    }

    pRefTime.date.moveOfANumberOfDaysInFuture(3);
    if (date.isTheSameDay(pRefTime.date)) {
        pRelativedayConcept = "time_relativeDay_tomorrow";
        return true;
    }

    pRefTime.date.moveOfANumberOfDaysInFuture(1);
    if (date.isTheSameDay(pRefTime.date)) {
        pRelativedayConcept = "time_relativeDay_dayAfterTomorrow";
        return true;
    }
    return false;
}

bool SemanticTimeGrounding::isEqualMoreOrLess10Seconds(const SemanticTimeGrounding& pOther) const {
    if (date == pOther.date) {
        SemanticDuration tenSeconds;
        tenSeconds.sign = Sign::POSITIVE;
        tenSeconds.timeInfos.emplace(SemanticTimeUnity::SECOND, 10);
        return timeOfDay.isEqualWithMarginOfError(pOther.timeOfDay, tenSeconds);
    }
    return false;
}

void SemanticTimeGrounding::mergeWith(const SemanticTimeGrounding& pOther) {
    if (pOther.length.isEmpty())
        length.clear();
    for (const auto& currTimeInfo : pOther.timeOfDay.timeInfos)
        timeOfDay.timeInfos.emplace(currTimeInfo.first, currTimeInfo.second);
    fromConcepts.insert(pOther.fromConcepts.begin(), pOther.fromConcepts.end());
}

std::unique_ptr<SemanticTimeGrounding> SemanticTimeGrounding::nowPtr() {
    auto res = std::make_unique<SemanticTimeGrounding>();
    res->equalToNow();
    return res;
}

SemanticTimeGrounding SemanticTimeGrounding::now() {
    SemanticTimeGrounding res;
    res.equalToNow();
    return res;
}

SemanticDuration SemanticTimeGrounding::relativeToAbsolute(const SemanticDuration& pRelativeDuration) {
    SemanticDuration nowDuration;
    SemanticTimeGrounding::now().getBeginInDurationStruct(nowDuration);
    return nowDuration + pRelativeDuration;
}

SemanticDuration SemanticTimeGrounding::absoluteToRelative(const SemanticDuration& pAbsoluteDuration) {
    SemanticDuration nowDuration;
    SemanticTimeGrounding::now().getBeginInDurationStruct(nowDuration);
    SemanticDuration res = pAbsoluteDuration - nowDuration;
    auto itLessThanAMillisecond = res.timeInfos.find(SemanticTimeUnity::LESS_THAN_A_MILLISECOND);
    if (itLessThanAMillisecond != res.timeInfos.end())
        res.timeInfos.erase(itLessThanAMillisecond);
    return res;
}

void SemanticTimeGrounding::setAnHardCodedTimeElts(bool pDateAndTimeOfTheDay, bool pLessThanASecond) {
    if (pDateAndTimeOfTheDay) {
        SemanticDate::hardCodedCurrentDate = std::make_unique<SemanticDate>();
        SemanticDate::hardCodedCurrentDate->year.emplace(2000);
        SemanticDate::hardCodedCurrentDate->month.emplace(1);
        SemanticDate::hardCodedCurrentDate->day.emplace(1);
        hardCodedCurrentTimeOfDay = std::make_unique<SemanticDuration>();
        hardCodedCurrentTimeOfDay->add(SemanticTimeUnity::HOUR, 7);
    } else {
        SemanticDate::hardCodedCurrentDate.reset();
        hardCodedCurrentTimeOfDay.reset();
    }
    if (pLessThanASecond)
        hardCodedLessThanASecondNextValue = std::make_unique<int>(0);
    else
        hardCodedLessThanASecondNextValue.reset();
}

void SemanticTimeGrounding::_setToNow(SemanticDate& pDate, SemanticDuration& pTimeDuration) {
    if (SemanticDate::hardCodedCurrentDate) {
        pDate = *SemanticDate::hardCodedCurrentDate;
        assert(hardCodedCurrentTimeOfDay);
        pTimeDuration = *hardCodedCurrentTimeOfDay;
    } else {
        std::time_t t = std::time(nullptr);
        std::tm* tmPtr = std::localtime(&t);
        if (tmPtr == nullptr) {
            std::cerr << "Cannot get local time" << std::endl;
            return;
        }
        auto& tm = *tmPtr;
        pDate.year.emplace(1900 + tm.tm_year);
        pDate.month.emplace(1 + tm.tm_mon);
        pDate.day.emplace(tm.tm_mday);
        pTimeDuration.timeInfos[SemanticTimeUnity::HOUR] = tm.tm_hour;
        pTimeDuration.timeInfos[SemanticTimeUnity::MINUTE] = tm.tm_min;
        pTimeDuration.timeInfos[SemanticTimeUnity::SECOND] = tm.tm_sec;
        pTimeDuration.removeEmptyValues();
    }

    if (hardCodedLessThanASecondNextValue) {
        if (*hardCodedLessThanASecondNextValue != 0)
            pTimeDuration.timeInfos[SemanticTimeUnity::LESS_THAN_A_MILLISECOND] = *hardCodedLessThanASecondNextValue;
    } else {
        std::unique_lock<std::mutex> lock(_lastGivenTimeMutex);
        if (_lastGivenTime && pTimeDuration == *_lastGivenTime) {
            pTimeDuration.timeInfos[SemanticTimeUnity::LESS_THAN_A_MILLISECOND] = ++lessThanASecondNextValue;
        } else {
            _lastGivenTime = std::make_unique<SemanticDuration>(pTimeDuration);
            lessThanASecondNextValue = 0;
        }
    }

    if (!pTimeDuration.timeInfos.empty())
        pTimeDuration.sign = Sign::POSITIVE;
}

}    // End of namespace onsem
