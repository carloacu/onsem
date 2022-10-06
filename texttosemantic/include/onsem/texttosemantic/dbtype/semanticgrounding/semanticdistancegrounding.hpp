#ifndef ONSEM_TEXTTOSEMANTIC_DBTYPE_SEMANTICGROUNDING_SEMANTICDISTANCEGROUNDING_HPP
#define ONSEM_TEXTTOSEMANTIC_DBTYPE_SEMANTICGROUNDING_SEMANTICDISTANCEGROUNDING_HPP

#include <list>
#include <string>
#include "../../api.hpp"
#include "semanticgrouding.hpp"


namespace onsem
{


// Distance Unity
// ==============

#define SEMANTIC_DISTANCE_UNITY_TABLE                           \
  SEMANTIC_DISTANCE_UNITY(KILOMETER, "km", 1000000)             \
  SEMANTIC_DISTANCE_UNITY(HECTOMETER, "hm", 100000)             \
  SEMANTIC_DISTANCE_UNITY(DECAMETER, "dam", 10000)              \
  SEMANTIC_DISTANCE_UNITY(METER, "m", 1000)                     \
  SEMANTIC_DISTANCE_UNITY(DECIMETER, "dm", 100)                 \
  SEMANTIC_DISTANCE_UNITY(CENTIMETER, "cm", 10)                 \
  SEMANTIC_DISTANCE_UNITY(MILLIMETER, "mm", 1)


#define SEMANTIC_DISTANCE_UNITY(a, b, c) a,
enum class SemanticDistanceUnity : char
{
  SEMANTIC_DISTANCE_UNITY_TABLE
};
#undef SEMANTIC_DISTANCE_UNITY

#define SEMANTIC_DISTANCE_UNITY(a, b, c) b,
static const std::vector<std::string> _semanticDistanceUnity_toStr = {
  SEMANTIC_DISTANCE_UNITY_TABLE
};
#undef SEMANTIC_DISTANCE_UNITY

#define SEMANTIC_DISTANCE_UNITY(a, b, c) {b, SemanticDistanceUnity::a},
static const std::map<std::string, SemanticDistanceUnity> _semanticDistanceUnity_fromStr = {
  SEMANTIC_DISTANCE_UNITY_TABLE
};
#undef SEMANTIC_DISTANCE_UNITY

#define SEMANTIC_DISTANCE_UNITY(a, b, c) c,
static const std::vector<int64_t> _semanticDistanceUnity_toNbOfMillimeters = {
  SEMANTIC_DISTANCE_UNITY_TABLE
};
#undef SEMANTIC_DISTANCE_UNITY
static const std::size_t _semanticDistanceUnity_size = _semanticDistanceUnity_toStr.size();
#undef SEMANTIC_DISTANCE_UNITY_TABLE



static inline char semanticDistanceUnity_toChar(SemanticDistanceUnity pDistanceUnity)
{
  return static_cast<char>(pDistanceUnity);
}

static inline char semanticDistanceUnity_toUnorderredChar(SemanticDistanceUnity pDistanceUnity)
{
  return _semanticDistanceUnity_size - static_cast<char>(pDistanceUnity);
}

static inline SemanticDistanceUnity semanticDistanceUnity_fromChar(unsigned char pDistanceUnity)
{
  return static_cast<SemanticDistanceUnity>(pDistanceUnity);
}

static inline SemanticDistanceUnity semanticDistanceUnity_fromUnorderredChar(unsigned char pDistanceUnity)
{
  return static_cast<SemanticDistanceUnity>(_semanticDistanceUnity_size - pDistanceUnity);
}

static inline char semanticDistanceUnity_fromUnorderredChar(SemanticDistanceUnity pDistanceUnity)
{
  return _semanticDistanceUnity_size - static_cast<char>(pDistanceUnity);
}

static inline std::string semanticDistanceUnity_toStr(SemanticDistanceUnity pDistanceUnity)
{
  return _semanticDistanceUnity_toStr[semanticDistanceUnity_toChar(pDistanceUnity)];
}


static inline SemanticDistanceUnity semanticDistanceUnity_fromStr
(const std::string& pDistanceUnityStr)
{
  auto it = _semanticDistanceUnity_fromStr.find(pDistanceUnityStr);
  if (it != _semanticDistanceUnity_fromStr.end())
    return it->second;
  assert(false);
  return SemanticDistanceUnity::MILLIMETER;
}


static inline int64_t semanticDistanceUnity_toNbOfMilliseconds
(SemanticDistanceUnity pDistanceUnity)
{
  return _semanticDistanceUnity_toNbOfMillimeters[semanticDistanceUnity_toChar(pDistanceUnity)];
}


// Distance Grounding
// ==================


struct ONSEM_TEXTTOSEMANTIC_API SemanticDistance
{
  bool operator==(const SemanticDistance& pOther) const;
  void printDistance(std::list<std::string>& pListElts,
                     const std::string& pLabelName) const;

  std::map<SemanticDistanceUnity, int> distanceInfos{};
};


struct ONSEM_TEXTTOSEMANTIC_API SemanticDistanceGrounding : public SemanticGrounding
{
  SemanticDistanceGrounding()
    : SemanticGrounding(SemanticGroudingType::DISTANCE),
      distance()
  {
  }

  const SemanticDistanceGrounding& getDistanceGrounding() const override { return *this; }
  SemanticDistanceGrounding& getDistanceGrounding() override { return *this; }
  const SemanticDistanceGrounding* getDistanceGroundingPtr() const override { return this; }
  SemanticDistanceGrounding* getDistanceGroundingPtr() override { return this; }

  bool operator==(const SemanticDistanceGrounding& pOther) const;
  bool isEqual(const SemanticDistanceGrounding& pOther) const;

  SemanticDistance distance;
};



inline bool SemanticDistanceGrounding::operator==(const SemanticDistanceGrounding& pOther) const
{
  return this->isEqual(pOther);
}

inline bool SemanticDistanceGrounding::isEqual(const SemanticDistanceGrounding& pOther) const
{
  return _isMotherClassEqual(pOther) &&
      distance == pOther.distance;
}


} // End of namespace onsem


#endif // ONSEM_TEXTTOSEMANTIC_DBTYPE_SEMANTICGROUNDING_SEMANTICDISTANCEGROUNDING_HPP
