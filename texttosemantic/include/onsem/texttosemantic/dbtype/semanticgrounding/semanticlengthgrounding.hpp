#ifndef ONSEM_TEXTTOSEMANTIC_DBTYPE_SEMANTICGROUNDING_SEMANTICLENGTHGROUNDING_HPP
#define ONSEM_TEXTTOSEMANTIC_DBTYPE_SEMANTICGROUNDING_SEMANTICLENGTHGROUNDING_HPP

#include <list>
#include <string>
#include "../../api.hpp"
#include "semanticgrounding.hpp"
#include "../semanticquantity.hpp"

namespace onsem
{


// Length Unity
// ============

#define SEMANTIC_LENGTH_UNITY_TABLE                                    \
  SEMANTIC_LENGTH_UNITY(KILOMETER, "km", "kilometer", 1000000)         \
  SEMANTIC_LENGTH_UNITY(HECTOMETER, "hm", "hectometer", 100000)        \
  SEMANTIC_LENGTH_UNITY(DECAMETER, "dam", "decameter", 10000)          \
  SEMANTIC_LENGTH_UNITY(METER, "m", "meter", 1000)                     \
  SEMANTIC_LENGTH_UNITY(DECIMETER, "dm", "decimeter", 100)             \
  SEMANTIC_LENGTH_UNITY(CENTIMETER, "cm", "centimeter", 10)            \
  SEMANTIC_LENGTH_UNITY(MILLIMETER, "mm", "millimeter", 1)


#define SEMANTIC_LENGTH_UNITY(a, b, c, d) a,
enum class SemanticLengthUnity : char
{
  SEMANTIC_LENGTH_UNITY_TABLE
};
#undef SEMANTIC_LENGTH_UNITY

#define SEMANTIC_LENGTH_UNITY(a, b, c, d) b,
static const std::vector<std::string> _semanticLengthUnity_toAbreviation = {
  SEMANTIC_LENGTH_UNITY_TABLE
};
#undef SEMANTIC_LENGTH_UNITY

#define SEMANTIC_LENGTH_UNITY(a, b, c, d) c,
static const std::vector<std::string> _semanticLengthUnity_toStr = {
  SEMANTIC_LENGTH_UNITY_TABLE
};
#undef SEMANTIC_LENGTH_UNITY

#define SEMANTIC_LENGTH_UNITY(a, b, c, d) {b, SemanticLengthUnity::a},
static const std::map<std::string, SemanticLengthUnity> _semanticLengthUnity_fromAbreviation = {
  SEMANTIC_LENGTH_UNITY_TABLE
};
#undef SEMANTIC_LENGTH_UNITY

#define SEMANTIC_LENGTH_UNITY(a, b, c, d) {c, SemanticLengthUnity::a},
static const std::map<std::string, SemanticLengthUnity> _semanticLengthUnity_fromStr = {
  SEMANTIC_LENGTH_UNITY_TABLE
};
#undef SEMANTIC_LENGTH_UNITY

#define SEMANTIC_LENGTH_UNITY(a, b, c, d) d,
static const std::vector<int64_t> _semanticLengthUnity_toNbOfMillimeters = {
  SEMANTIC_LENGTH_UNITY_TABLE
};
#undef SEMANTIC_LENGTH_UNITY
static const std::size_t _semanticLengthUnity_size = _semanticLengthUnity_toAbreviation.size();

#define SEMANTIC_LENGTH_UNITY(a, b, c, d) SemanticLengthUnity::a,
static const std::vector<SemanticLengthUnity> semanticLengthUnities = {
  SEMANTIC_LENGTH_UNITY_TABLE
};
#undef SEMANTIC_LENGTH_UNITY
#undef SEMANTIC_LENGTH_UNITY_TABLE



static inline char semanticLengthUnity_toChar(SemanticLengthUnity pLengthUnity)
{
  return static_cast<char>(pLengthUnity);
}

static inline char semanticLengthUnity_toUnorderredChar(SemanticLengthUnity pLengthUnity)
{
  return _semanticLengthUnity_size - static_cast<char>(pLengthUnity);
}

static inline SemanticLengthUnity semanticLengthUnity_fromChar(unsigned char pLengthUnity)
{
  return static_cast<SemanticLengthUnity>(pLengthUnity);
}

static inline SemanticLengthUnity semanticLengthUnity_fromUnorderredChar(unsigned char pLengthUnity)
{
  return static_cast<SemanticLengthUnity>(_semanticLengthUnity_size - pLengthUnity);
}

static inline char SemanticLengthUnity_fromUnorderredChar(SemanticLengthUnity pLengthUnity)
{
  return _semanticLengthUnity_size - static_cast<char>(pLengthUnity);
}

static inline std::string semanticLengthUnity_toAbreviation(SemanticLengthUnity pLengthUnity)
{
  return _semanticLengthUnity_toAbreviation[semanticLengthUnity_toChar(pLengthUnity)];
}

static inline std::string semanticLengthUnity_toConcept(SemanticLengthUnity pLengthUnity)
{
  return "length_" + _semanticLengthUnity_toStr[semanticLengthUnity_toChar(pLengthUnity)];
}

static inline std::string semanticLengthUnity_toStr(SemanticLengthUnity pLengthUnity)
{
  return _semanticLengthUnity_toStr[semanticLengthUnity_toChar(pLengthUnity)];
}

static inline SemanticLengthUnity semanticLengthUnity_fromStr
(const std::string& pLengthUnityStr)
{
  auto it = _semanticLengthUnity_fromStr.find(pLengthUnityStr);
  if (it != _semanticLengthUnity_fromStr.end())
    return it->second;
  assert(false);
  return SemanticLengthUnity::MILLIMETER;
}

static inline SemanticLengthUnity semanticLengthUnity_fromAbreviation
(const std::string& pLengthUnityStr)
{
  auto it = _semanticLengthUnity_fromAbreviation.find(pLengthUnityStr);
  if (it != _semanticLengthUnity_fromAbreviation.end())
    return it->second;
  assert(false);
  return SemanticLengthUnity::MILLIMETER;
}

static inline int64_t semanticLengthUnity_untityConvertionCoeficient
(SemanticLengthUnity pLengthUnity1,
 SemanticLengthUnity pLengthUnity2)
{
  return _semanticLengthUnity_toNbOfMillimeters[semanticLengthUnity_toChar(pLengthUnity1)] /
      _semanticLengthUnity_toNbOfMillimeters[semanticLengthUnity_toChar(pLengthUnity2)];
}



// Length Grounding
// ================


struct ONSEM_TEXTTOSEMANTIC_API SemanticLength
{
  bool operator==(const SemanticLength& pOther) const;
  void printLength(std::list<std::string>& pListElts,
                   const std::string& pLabelName) const;
  void convertToUnity(SemanticLengthUnity pUnity);

  std::map<SemanticLengthUnity, SemanticFloat> lengthInfos{};
};


struct ONSEM_TEXTTOSEMANTIC_API SemanticLengthGrounding : public SemanticGrounding
{
  SemanticLengthGrounding()
    : SemanticGrounding(SemanticGroundingType::LENGTH),
      length()
  {
  }

  const SemanticLengthGrounding& getLengthGrounding() const override { return *this; }
  SemanticLengthGrounding& getLengthGrounding() override { return *this; }
  const SemanticLengthGrounding* getLengthGroundingPtr() const override { return this; }
  SemanticLengthGrounding* getLengthGroundingPtr() override { return this; }

  bool operator==(const SemanticLengthGrounding& pOther) const;
  bool isEqual(const SemanticLengthGrounding& pOther) const;

  SemanticLength length;
};



inline bool SemanticLengthGrounding::operator==(const SemanticLengthGrounding& pOther) const
{
  return this->isEqual(pOther);
}

inline bool SemanticLengthGrounding::isEqual(const SemanticLengthGrounding& pOther) const
{
  return _isMotherClassEqual(pOther) &&
      length == pOther.length;
}


} // End of namespace onsem


#endif // ONSEM_TEXTTOSEMANTIC_DBTYPE_SEMANTICGROUNDING_SEMANTICLENGTHGROUNDING_HPP
