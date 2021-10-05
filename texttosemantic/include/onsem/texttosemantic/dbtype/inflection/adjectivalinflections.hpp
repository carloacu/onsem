#ifndef ONSEM_TEXTTOSEMANTIC_TYPE_INFLECTION_ADJECTIVALINFLECTIONS_HPP
#define ONSEM_TEXTTOSEMANTIC_TYPE_INFLECTION_ADJECTIVALINFLECTIONS_HPP

#include <list>
#include <onsem/common/enum/comparisontype.hpp>
#include <onsem/common/enum/semanticgendertype.hpp>
#include <onsem/common/enum/semanticnumbertype.hpp>
#include <onsem/texttosemantic/dbtype/inflection/inflections.hpp>
#include "../../api.hpp"


namespace onsem
{

struct ONSEM_TEXTTOSEMANTIC_API AdjectivalInflection
{
  AdjectivalInflection() = default;
  AdjectivalInflection(SemanticGenderType pGender,
                       SemanticNumberType pNumber);
  AdjectivalInflection(ComparisonType pComparisonType);
  AdjectivalInflection(const std::string& pInflectionalCode);
  bool operator==(const AdjectivalInflection& pOther) const;

  SemanticGenderType gender{SemanticGenderType::UNKNOWN};
  SemanticNumberType number{SemanticNumberType::UNKNOWN};
  ComparisonType comparisonType{ComparisonType::NONE};
};


struct ONSEM_TEXTTOSEMANTIC_API AdjectivalInflections : public Inflections
{
  AdjectivalInflections();
  AdjectivalInflections(const std::vector<std::string>& pInflectionalCodes);
  bool operator==(const AdjectivalInflections& pOther) const;

  virtual AdjectivalInflections& getAdjectivalI() { return *this; }
  virtual const AdjectivalInflections& getAdjectivalI() const { return *this; }
  virtual AdjectivalInflections* getAdjectivalIPtr() { return this; }
  virtual const AdjectivalInflections* getAdjectivalIPtr() const { return this; }

  std::list<AdjectivalInflection> inflections;
};





inline std::ostream& operator<<(std::ostream& pOs, const AdjectivalInflection& pAdjInfl)
{
  gender_toConcisePrint(pOs, pAdjInfl.gender);
  number_toConcisePrint(pOs, pAdjInfl.number);
  comparisonType_toConcisePrint(pOs, pAdjInfl.comparisonType);
  return pOs;
}

inline std::ostream& operator<<(std::ostream& pOs, const AdjectivalInflections& pAdjInfls)
{
  bool firstLoop = true;
  for (const auto& currInfl : pAdjInfls.inflections)
  {
    if (firstLoop)
      firstLoop = false;
    else
      pOs << ",";
    pOs << currInfl;
  }
  return pOs;
}

} // End of namespace onsem

#include "detail/adjectivalinflections.hxx"

#endif // ONSEM_TEXTTOSEMANTIC_TYPE_INFLECTION_ADJECTIVALINFLECTIONS_HPP
