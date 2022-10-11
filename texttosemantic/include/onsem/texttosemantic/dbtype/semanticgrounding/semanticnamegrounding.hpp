#ifndef ONSEM_TEXTTOSEMANTIC_DBTYPE_SEMANTICGROUNDING_SEMANTICNAMEGROUNDING_HPP
#define ONSEM_TEXTTOSEMANTIC_DBTYPE_SEMANTICGROUNDING_SEMANTICNAMEGROUNDING_HPP

#include <string>
#include <vector>
#include <list>
#include <set>
#include <onsem/common/enum/semanticgendertype.hpp>
#include "semanticgrounding.hpp"
#include "../../api.hpp"


namespace onsem
{

struct ONSEM_TEXTTOSEMANTIC_API NameInfos
{
  NameInfos()
  : names(),
    possibleGenders()
  {
  }

  NameInfos(const std::string& pName)
  : names(1, pName),
    possibleGenders()
  {
  }

  NameInfos(const std::vector<std::string>& pNames)
  : names(pNames),
    possibleGenders()
  {
  }

  NameInfos(const std::list<std::string>& pNames)
  : names(pNames.begin(), pNames.end()),
    possibleGenders()
  {
  }

  bool operator==(const NameInfos& pOther) const
  {
    return names == pOther.names &&
        possibleGenders == pOther.possibleGenders;
  }

  std::vector<std::string> names;
  std::set<SemanticGenderType> possibleGenders;
};


struct ONSEM_TEXTTOSEMANTIC_API SemanticNameGrounding : public SemanticGrounding
{
  SemanticNameGrounding()
  : SemanticGrounding(SemanticGroudingType::NAME),
    nameInfos()
  {
  }

  SemanticNameGrounding(const std::string& pName)
  : SemanticGrounding(SemanticGroudingType::NAME),
    nameInfos(pName)
  {
  }

  SemanticNameGrounding(const std::vector<std::string>& pNames)
  : SemanticGrounding(SemanticGroudingType::NAME),
    nameInfos(pNames)
  {
  }

  SemanticNameGrounding(const std::list<std::string>& pNames)
  : SemanticGrounding(SemanticGroudingType::NAME),
    nameInfos(pNames)
  {
  }

  SemanticNameGrounding(const std::string& pName,
                        const std::string& pUserId)
  : SemanticGrounding(SemanticGroudingType::NAME),
    nameInfos(pName)
  {
    concepts.emplace("agent_userId_" + pUserId, 4);
  }

  const SemanticNameGrounding& getNameGrounding() const override { return *this; }
  SemanticNameGrounding& getNameGrounding() override { return *this; }
  const SemanticNameGrounding* getNameGroundingPtr() const override { return this; }
  SemanticNameGrounding* getNameGroundingPtr() override { return this; }

  bool operator==(const SemanticNameGrounding& pOther) const;
  bool isEqual(const SemanticNameGrounding& pOther) const;

  static std::string namesToStr(const std::vector<std::string>& pNames);
  static std::string namesToStrFromBinary(unsigned char pNbOfStrings,
                                          const unsigned char*& pUChars);

  NameInfos nameInfos;
};





inline bool SemanticNameGrounding::operator==(const SemanticNameGrounding& pOther) const
{
  return this->isEqual(pOther);
}

inline bool SemanticNameGrounding::isEqual(const SemanticNameGrounding& pOther) const
{
  return _isMotherClassEqual(pOther) &&
      nameInfos == pOther.nameInfos;
}



} // End of namespace onsem


#endif // ONSEM_TEXTTOSEMANTIC_DBTYPE_SEMANTICGROUNDING_SEMANTICNAMEGROUNDING_HPP
