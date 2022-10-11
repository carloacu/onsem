#ifndef ONSEM_TEXTTOSEMANTIC_DBTYPE_SEMANTICGROUNDING_SEMANTICUNITYGROUNDING_HPP
#define ONSEM_TEXTTOSEMANTIC_DBTYPE_SEMANTICGROUNDING_SEMANTICUNITYGROUNDING_HPP

#include <list>
#include <string>
#include "../../api.hpp"
#include "semanticgrounding.hpp"
#include "semanticlengthgrounding.hpp"
#include "semanticdurationgrounding.hpp"

namespace onsem
{

#define SEMANTIC_TYPEOFUNITY_UNITY_TABLE              \
  SEMANTIC_TYPEOFUNITY_UNITY(LENGTH, "length")        \
  SEMANTIC_TYPEOFUNITY_UNITY(TIME, "time")


#define SEMANTIC_TYPEOFUNITY_UNITY(a, b) a,
enum class TypeOfUnity
{
  SEMANTIC_TYPEOFUNITY_UNITY_TABLE
};
#undef SEMANTIC_TYPEOFUNITY_UNITY


#define SEMANTIC_TYPEOFUNITY_UNITY(a, b) b,
static const std::vector<std::string> _typeOfUnity_toStr = {
  SEMANTIC_TYPEOFUNITY_UNITY_TABLE
};
#undef SEMANTIC_TYPEOFUNITY_UNITY

#define SEMANTIC_TYPEOFUNITY_UNITY(a, b) {b, TypeOfUnity::a},
static const std::map<std::string, TypeOfUnity> _typeOfUnity_fromStr = {
  SEMANTIC_TYPEOFUNITY_UNITY_TABLE
};
#undef SEMANTIC_TYPEOFUNITY_UNITY


static inline char typeOfUnity_toChar(TypeOfUnity pTypeOfUnity)
{
  return static_cast<char>(pTypeOfUnity);
}

static inline TypeOfUnity typeOfUnity_fromChar(unsigned char pTypeOfUnity)
{
  return static_cast<TypeOfUnity>(pTypeOfUnity);
}

static inline std::string typeOfUnity_toStr(TypeOfUnity pTypeOfUnity)
{
  return _typeOfUnity_toStr[typeOfUnity_toChar(pTypeOfUnity)];
}

static inline TypeOfUnity typeOfUnity_fromStr(const std::string& pTypeOfUnityStr)
{
  auto it = _typeOfUnity_fromStr.find(pTypeOfUnityStr);
  if (it != _typeOfUnity_fromStr.end())
    return it->second;
  assert(false);
  return TypeOfUnity::LENGTH;
}




struct ONSEM_TEXTTOSEMANTIC_API SemanticUnityGrounding : public SemanticGrounding
{
  SemanticUnityGrounding(SemanticLengthUnity pLengthUnity)
    : SemanticGrounding(SemanticGroundingType::UNITY),
      typeOfUnity(TypeOfUnity::LENGTH),
      value(semanticLengthUnity_toChar(pLengthUnity))
  {
  }
  SemanticUnityGrounding(SemanticTimeUnity pTimeUnity)
    : SemanticGrounding(SemanticGroundingType::UNITY),
      typeOfUnity(TypeOfUnity::TIME),
      value(semanticTimeUnity_toChar(pTimeUnity))
  {
  }
  SemanticUnityGrounding(TypeOfUnity pTypeOfUnity,
                         const std::string& pValueStr)
    : SemanticGrounding(SemanticGroundingType::UNITY),
      typeOfUnity(TypeOfUnity::TIME),
      value(0)
  {
    setValue(pTypeOfUnity, pValueStr);
  }

  const SemanticUnityGrounding& getUnityGrounding() const override { return *this; }
  SemanticUnityGrounding& getUnityGrounding() override { return *this; }
  const SemanticUnityGrounding* getUnityGroundingPtr() const override { return this; }
  SemanticUnityGrounding* getUnityGroundingPtr() override { return this; }

  bool operator==(const SemanticUnityGrounding& pOther) const;
  bool isEqual(const SemanticUnityGrounding& pOther) const;

  void setValue(SemanticLengthUnity pLengthUnity);
  void setValue(SemanticTimeUnity pTimeUnity);
  void setValue(TypeOfUnity pTypeOfUnity,
                const std::string& pValueStr);

  SemanticLengthUnity getLengthUnity() const;
  SemanticTimeUnity getTimeUnity() const;
  std::string getValueStr() const;
  std::string getValueConcept() const;

  TypeOfUnity typeOfUnity;
  unsigned char value;
};



inline bool SemanticUnityGrounding::operator==(const SemanticUnityGrounding& pOther) const
{
  return this->isEqual(pOther);
}

inline bool SemanticUnityGrounding::isEqual(const SemanticUnityGrounding& pOther) const
{
  return _isMotherClassEqual(pOther) &&
      typeOfUnity == pOther.typeOfUnity &&
      value == pOther.value;
}


} // End of namespace onsem


#endif // ONSEM_TEXTTOSEMANTIC_DBTYPE_SEMANTICGROUNDING_SEMANTICUNITYGROUNDING_HPP
