#ifndef SEMANTICDURATIONGROUNDINGS_H
#define SEMANTICDURATIONGROUNDINGS_H

#include <list>
#include <vector>
#include <string>
#include <onsem/common/utility/to_underlying.hpp>
#include "semanticgrouding.hpp"
#include "../../api.hpp"


namespace onsem
{

// Time Unity
// ==========

#define SEMANTIC_TIME_UNITY_TABLE                                                     \
  SEMANTIC_TIME_UNITY(YEAR, "year", "y", 31104000000, 4)                              \
  SEMANTIC_TIME_UNITY(MONTH, "month", "mo", 2592000000, 2)                            \
  SEMANTIC_TIME_UNITY(DAY, "day", "d", 86400000, 2)                                   \
  SEMANTIC_TIME_UNITY(HOUR, "hour", "h", 3600000, 2)                                  \
  SEMANTIC_TIME_UNITY(MINUTE, "minute", "m", 60000, 2)                                \
  SEMANTIC_TIME_UNITY(SECOND, "second", "s", 1000, 2)                                 \
  SEMANTIC_TIME_UNITY(MILLISECOND, "millisecond", "ms", 1, 3)                         \
  SEMANTIC_TIME_UNITY(LESS_THAN_A_MILLISECOND, "less_than_a_millisecond", "", 0, 10)


#define SEMANTIC_TIME_UNITY(a, b, c, d, e) a,
enum class SemanticTimeUnity : char
{
  SEMANTIC_TIME_UNITY_TABLE
};
#undef SEMANTIC_TIME_UNITY



#define SEMANTIC_TIME_UNITY(a, b, c, d, e) b,
static const std::vector<std::string> _semanticTimeUnity_toStr = {
  SEMANTIC_TIME_UNITY_TABLE
};
#undef SEMANTIC_TIME_UNITY

#define SEMANTIC_TIME_UNITY(a, b, c, d, e) {b, SemanticTimeUnity::a},
static const std::map<std::string, SemanticTimeUnity> _semanticTimeUnity_fromStr = {
  SEMANTIC_TIME_UNITY_TABLE
};
#undef SEMANTIC_TIME_UNITY

#define SEMANTIC_TIME_UNITY(a, b, c, d, e) c,
static const std::vector<std::string> _semanticTimeUnity_toAbreviation = {
  SEMANTIC_TIME_UNITY_TABLE
};
#undef SEMANTIC_TIME_UNITY

#define SEMANTIC_TIME_UNITY(a, b, c, d, e) d,
static const std::vector<int64_t> _semanticTimeUnity_toNbOfMilliseconds = {
  SEMANTIC_TIME_UNITY_TABLE
};
#undef SEMANTIC_TIME_UNITY

#define SEMANTIC_TIME_UNITY(a, b, c, d, e) e,
static const std::vector<int> _semanticTimeUnity_toNbOfDigits = {
  SEMANTIC_TIME_UNITY_TABLE
};
#undef SEMANTIC_TIME_UNITY
static const std::size_t _semanticTimeUnity_size = _semanticTimeUnity_toStr.size();
#undef SEMANTIC_TIME_UNITY_TABLE



static inline char semanticTimeUnity_toChar(SemanticTimeUnity pTimeUnity)
{
  return static_cast<char>(pTimeUnity);
}

static inline char semanticTimeUnity_toUnorderredChar(SemanticTimeUnity pTimeUnity)
{
  return _semanticTimeUnity_size - static_cast<char>(pTimeUnity);
}

static inline SemanticTimeUnity semanticTimeUnity_fromChar(unsigned char pTimeUnity)
{
  return static_cast<SemanticTimeUnity>(pTimeUnity);
}

static inline SemanticTimeUnity semanticTimeUnity_fromUnorderredChar(unsigned char pTimeUnity)
{
  return static_cast<SemanticTimeUnity>(_semanticTimeUnity_size - pTimeUnity);
}

static inline char semanticTimeUnity_fromUnorderredChar(SemanticTimeUnity pTimeUnity)
{
  return _semanticTimeUnity_toStr.size() - static_cast<char>(pTimeUnity);
}

static inline std::string semanticTimeUnity_toStr(SemanticTimeUnity pTimeUnity)
{
  return _semanticTimeUnity_toStr[semanticTimeUnity_toChar(pTimeUnity)];
}


static inline SemanticTimeUnity semanticTimeUnity_fromStr
(const std::string& pTimeUnityStr)
{
  auto it = _semanticTimeUnity_fromStr.find(pTimeUnityStr);
  if (it != _semanticTimeUnity_fromStr.end())
    return it->second;
  assert(false);
  return SemanticTimeUnity::LESS_THAN_A_MILLISECOND;
}


static inline std::string semanticTimeUnity_toAbreviation
(SemanticTimeUnity pTimeUnity)
{
  return _semanticTimeUnity_toAbreviation[semanticTimeUnity_toChar(pTimeUnity)];
}

static inline int64_t semanticTimeUnity_toNbOfMilliseconds
(SemanticTimeUnity pTimeUnity)
{
  return _semanticTimeUnity_toNbOfMilliseconds[semanticTimeUnity_toChar(pTimeUnity)];
}

static inline int semanticTimeUnity_toNbOfDigits(SemanticTimeUnity pTimeUnity)
{
  return _semanticTimeUnity_toNbOfDigits[semanticTimeUnity_toChar(pTimeUnity)];
}




// Month Values
// ============

#define SEMANTIC_TIME_MONTH_TABLE                             \
  SEMANTIC_TIME_MONTH(JANUARY, "time_month_january", 1)       \
  SEMANTIC_TIME_MONTH(FEBRUARY, "time_month_february", 2)     \
  SEMANTIC_TIME_MONTH(MARCH, "time_month_march", 3)           \
  SEMANTIC_TIME_MONTH(APRIL, "time_month_april", 4)           \
  SEMANTIC_TIME_MONTH(MAY, "time_month_may", 5)               \
  SEMANTIC_TIME_MONTH(JUNE, "time_month_june", 6)             \
  SEMANTIC_TIME_MONTH(JULY, "time_month_july", 7)             \
  SEMANTIC_TIME_MONTH(AUGUST, "time_month_august", 8)         \
  SEMANTIC_TIME_MONTH(SEPTEMBER, "time_month_september", 9)   \
  SEMANTIC_TIME_MONTH(OCTOBER, "time_month_october", 10)      \
  SEMANTIC_TIME_MONTH(NOVEMBER, "time_month_november", 11)    \
  SEMANTIC_TIME_MONTH(DECEMBER, "time_month_december", 12)


#define SEMANTIC_TIME_MONTH(a, b, c) a,
enum class SmanticTimeMonth
{
  SEMANTIC_TIME_MONTH_TABLE
};
#undef SEMANTIC_TIME_MONTH


#define SEMANTIC_TIME_MONTH(a, b, c) b,
static const std::vector<std::string> _semanticTimeMonth_toStr = {
  SEMANTIC_TIME_MONTH_TABLE
};
#undef SEMANTIC_TIME_MONTH

#define SEMANTIC_TIME_MONTH(a, b, c) {b, SmanticTimeMonth::a},
static const std::map<std::string, SmanticTimeMonth> _semanticTimeMonth_fromStr = {
  SEMANTIC_TIME_MONTH_TABLE
};
#undef SEMANTIC_TIME_MONTH

#define SEMANTIC_TIME_MONTH(a, b, c) c,
static const std::vector<int> _semanticTimeMonth_toId = {
  SEMANTIC_TIME_MONTH_TABLE
};
#undef SEMANTIC_TIME_MONTH

#define SEMANTIC_TIME_MONTH(a, b, c) {c, SmanticTimeMonth::a},
static const std::map<int, SmanticTimeMonth> _semanticTimeMonth_fromId = {
  SEMANTIC_TIME_MONTH_TABLE
};
#undef SEMANTIC_TIME_MONTH
#undef SEMANTIC_TIME_MONTH_TABLE


static inline std::string semanticTimeMonth_toStr(SmanticTimeMonth pMonth)
{
  return _semanticTimeMonth_toStr[mystd::to_underlying(pMonth)];
}

static inline SmanticTimeMonth semanticTimeMonth_fromStr(const std::string& pMonthStr)
{
  auto it = _semanticTimeMonth_fromStr.find(pMonthStr);
  assert(it != _semanticTimeMonth_fromStr.end());
  return it->second;
}

static inline int semanticTimeMonth_toId(SmanticTimeMonth pMonth)
{
  return _semanticTimeMonth_toId[mystd::to_underlying(pMonth)];
}

static inline SmanticTimeMonth semanticTimeMonth_fromId(int pMonthId)
{
  return _semanticTimeMonth_fromId.find(pMonthId)->second;
}

inline static std::string monthConceptStr_fromMonthId(int pMonthId)
{
  return semanticTimeMonth_toStr(semanticTimeMonth_fromId(pMonthId));
}




// Direction values
// ================


#define SEMANTIC_DURATION_DIRECTION_TABLE       \
  SEMANTIC_DURATION_DIRECTION(NEGATIVE, "-")    \
  SEMANTIC_DURATION_DIRECTION(POSITIVE, "")


#define SEMANTIC_DURATION_DIRECTION(a, b) a,
enum class SemanticDurationSign
{
  SEMANTIC_DURATION_DIRECTION_TABLE
};
#undef SEMANTIC_DURATION_DIRECTION


#define SEMANTIC_DURATION_DIRECTION(a, b) b,
static const std::vector<std::string> _semanticDurationSigne_toStr = {
  SEMANTIC_DURATION_DIRECTION_TABLE
};
#undef SEMANTIC_DURATION_DIRECTION
#undef SEMANTIC_DURATION_DIRECTION_TABLE



static inline std::string semanticDurationSigne_toStr(SemanticDurationSign pDirection)
{
  return _semanticDurationSigne_toStr[mystd::to_underlying(pDirection)];
}




// Duration Grounding
// ==================

struct ONSEM_TEXTTOSEMANTIC_API SemanticDuration
{
  bool operator<(const SemanticDuration& pOther) const;
  bool operator<=(const SemanticDuration& pOther) const { return operator<(pOther) || operator==(pOther); }
  bool operator>=(const SemanticDuration& pOther) const { return !operator<(pOther); }
  bool operator==(const SemanticDuration& pOther) const;
  bool operator!=(const SemanticDuration& pOther) const { return !operator==(pOther); }
  SemanticDuration operator+(const SemanticDuration& pOther) const;
  SemanticDuration operator-(const SemanticDuration& pOther) const;

  static SemanticDuration fromRadixMapStr(const std::string& pRadixMapStr);
  std::string toRadixMapStr() const;
  bool isNearlyEqual(const SemanticDuration& pOther) const;
  void add(SemanticTimeUnity pTimeUnity, int pNbToAdd);
  static SemanticDuration abs(const SemanticDuration& pDuration);
  void printDuration(std::list<std::string>& pListElts, const std::string& pDurationLabelName) const;
  SemanticTimeUnity precision() const;
  void clear();
  bool isEmpty() const;
  void removeEmptyValues();
  int64_t nbMilliseconds() const;
  bool isPositive() const { return sign == SemanticDurationSign::POSITIVE; }
  bool isEqualWithMarginOfError(const SemanticDuration& pOther,
                                const SemanticDuration& pMarginOfError) const;

  SemanticDurationSign sign{SemanticDurationSign::POSITIVE};
  std::map<SemanticTimeUnity, int> timeInfos{};



private:
  void _normalize();
  bool _invertDirectionIfNecessary();
  SemanticDuration _addition(const SemanticDuration& pOther,
                             int pCoefOfOther) const;
  bool _isNearlyEqualOneSideCheck(const SemanticDuration& pOther) const;
};


struct ONSEM_TEXTTOSEMANTIC_API SemanticDurationGrounding : public SemanticGrounding
{
  SemanticDurationGrounding()
    : SemanticGrounding(SemanticGroudingType::DURATION),
      duration()
  {
  }

  const SemanticDurationGrounding& getDurationGrounding() const override { return *this; }
  SemanticDurationGrounding& getDurationGrounding() override { return *this; }
  const SemanticDurationGrounding* getDurationGroundingPtr() const override { return this; }
  SemanticDurationGrounding* getDurationGroundingPtr() override { return this; }

  bool operator==(const SemanticDurationGrounding& pOther) const;
  bool isEqual(const SemanticDurationGrounding& pOther) const;

  SemanticDuration duration;
};



inline bool SemanticDurationGrounding::operator==(const SemanticDurationGrounding& pOther) const
{
  return this->isEqual(pOther);
}

inline bool SemanticDurationGrounding::isEqual(const SemanticDurationGrounding& pOther) const
{
  return _isMotherClassEqual(pOther) &&
      duration == pOther.duration;
}


} // End of namespace onsem


#endif // SEMANTICDURATIONGROUNDINGS_H
