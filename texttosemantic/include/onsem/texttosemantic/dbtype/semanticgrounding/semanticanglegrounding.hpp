#ifndef ONSEM_TEXTTOSEMANTIC_DBTYPE_SEMANTICGROUNDING_SEMANTICANGLEGROUNDING_HPP
#define ONSEM_TEXTTOSEMANTIC_DBTYPE_SEMANTICGROUNDING_SEMANTICANGLEGROUNDING_HPP

#include <list>
#include <string>
#include "../../api.hpp"
#include "semanticgrounding.hpp"


namespace onsem
{


// Angle Unity
// ===========

#define SEMANTIC_ANGLE_UNITY_TABLE                            \
  SEMANTIC_ANGLE_UNITY(DEGREE, "d", "degree")                 \
  SEMANTIC_ANGLE_UNITY(RADIAN, "r", "radian")


#define SEMANTIC_ANGLE_UNITY(a, b, c) a,
enum class SemanticAngleUnity : char
{
  SEMANTIC_ANGLE_UNITY_TABLE
};
#undef SEMANTIC_ANGLE_UNITY

#define SEMANTIC_ANGLE_UNITY(a, b, c) b,
static const std::vector<std::string> _semanticAngleUnity_toAbreviation = {
  SEMANTIC_ANGLE_UNITY_TABLE
};
#undef SEMANTIC_ANGLE_UNITY

#define SEMANTIC_ANGLE_UNITY(a, b, c) c,
static const std::vector<std::string> _semanticAngleUnity_toStr = {
  SEMANTIC_ANGLE_UNITY_TABLE
};
#undef SEMANTIC_ANGLE_UNITY

#define SEMANTIC_ANGLE_UNITY(a, b, c) {b, SemanticAngleUnity::a},
static const std::map<std::string, SemanticAngleUnity> _semanticAngleUnity_fromAbreviation = {
  SEMANTIC_ANGLE_UNITY_TABLE
};
#undef SEMANTIC_ANGLE_UNITY

#define SEMANTIC_ANGLE_UNITY(a, b, c) {c, SemanticAngleUnity::a},
static const std::map<std::string, SemanticAngleUnity> _semanticAngleUnity_fromStr = {
  SEMANTIC_ANGLE_UNITY_TABLE
};
#undef SEMANTIC_ANGLE_UNITY

static const std::size_t _semanticAngleUnity_size = _semanticAngleUnity_toAbreviation.size();

#define SEMANTIC_ANGLE_UNITY(a, b, c) SemanticAngleUnity::a,
static const std::vector<SemanticAngleUnity> semanticAngleUnities = {
  SEMANTIC_ANGLE_UNITY_TABLE
};
#undef SEMANTIC_ANGLE_UNITY
#undef SEMANTIC_ANGLE_UNITY_TABLE



static inline char semanticAngleUnity_toChar(SemanticAngleUnity pAngleUnity)
{
  return static_cast<char>(pAngleUnity);
}

static inline char semanticAngleUnity_toUnorderredChar(SemanticAngleUnity pAngleUnity)
{
  return _semanticAngleUnity_size - static_cast<char>(pAngleUnity);
}

static inline SemanticAngleUnity semanticAngleUnity_fromChar(unsigned char pAngleUnity)
{
  return static_cast<SemanticAngleUnity>(pAngleUnity);
}

static inline SemanticAngleUnity semanticAngleUnity_fromUnorderredChar(unsigned char pAngleUnity)
{
  return static_cast<SemanticAngleUnity>(_semanticAngleUnity_size - pAngleUnity);
}

static inline char semanticAngleUnity_fromUnorderredChar(SemanticAngleUnity pAngleUnity)
{
  return _semanticAngleUnity_size - static_cast<char>(pAngleUnity);
}

static inline std::string semanticAngleUnity_toAbreviation(SemanticAngleUnity pAngleUnity)
{
  return _semanticAngleUnity_toAbreviation[semanticAngleUnity_toChar(pAngleUnity)];
}

static inline std::string semanticAngleUnity_toConcept(SemanticAngleUnity pAngleUnity)
{
  return "location_relative_angle_" + _semanticAngleUnity_toStr[semanticAngleUnity_toChar(pAngleUnity)];
}

static inline std::string semanticAngleUnity_toStr(SemanticAngleUnity pAngleUnity)
{
  return _semanticAngleUnity_toStr[semanticAngleUnity_toChar(pAngleUnity)];
}

static inline SemanticAngleUnity semanticAngleUnity_fromStr
(const std::string& pAngleUnityStr)
{
  auto it = _semanticAngleUnity_fromStr.find(pAngleUnityStr);
  if (it != _semanticAngleUnity_fromStr.end())
    return it->second;
  assert(false);
  return SemanticAngleUnity::DEGREE;
}

static inline SemanticAngleUnity semanticAngleUnity_fromAbreviation
(const std::string& pAngleUnityStr)
{
  auto it = _semanticAngleUnity_fromAbreviation.find(pAngleUnityStr);
  if (it != _semanticAngleUnity_fromAbreviation.end())
    return it->second;
  assert(false);
  return SemanticAngleUnity::DEGREE;
}




// Angle Grounding
// ===============


struct ONSEM_TEXTTOSEMANTIC_API SemanticAngle
{
  bool operator==(const SemanticAngle& pOther) const;
  void printAngle(std::list<std::string>& pListElts,
                  const std::string& pLabelName) const;
  std::string getRawValueStr() const;

  std::map<SemanticAngleUnity, int> angleInfos{};
};


struct ONSEM_TEXTTOSEMANTIC_API SemanticAngleGrounding : public SemanticGrounding
{
  SemanticAngleGrounding()
    : SemanticGrounding(SemanticGroundingType::ANGLE),
      angle()
  {
  }

  const SemanticAngleGrounding& getAngleGrounding() const override { return *this; }
  SemanticAngleGrounding& getAngleGrounding() override { return *this; }
  const SemanticAngleGrounding* getAngleGroundingPtr() const override { return this; }
  SemanticAngleGrounding* getAngleGroundingPtr() override { return this; }

  bool operator==(const SemanticAngleGrounding& pOther) const;
  bool isEqual(const SemanticAngleGrounding& pOther) const;

  SemanticAngle angle;
};



inline bool SemanticAngleGrounding::operator==(const SemanticAngleGrounding& pOther) const
{
  return this->isEqual(pOther);
}

inline bool SemanticAngleGrounding::isEqual(const SemanticAngleGrounding& pOther) const
{
  return _isMotherClassEqual(pOther) &&
      angle == pOther.angle;
}


} // End of namespace onsem


#endif // ONSEM_TEXTTOSEMANTIC_DBTYPE_SEMANTICGROUNDING_SEMANTICANGLEGROUNDING_HPP
