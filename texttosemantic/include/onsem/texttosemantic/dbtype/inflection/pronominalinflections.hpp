#ifndef ONSEM_TEXTTOSEMANTIC_TYPE_INFLECTION_PRONOMINALINFLECTIONS_HPP
#define ONSEM_TEXTTOSEMANTIC_TYPE_INFLECTION_PRONOMINALINFLECTIONS_HPP

#include <list>
#include <onsem/common/enum/semanticgendertype.hpp>
#include <onsem/common/enum/semanticnumbertype.hpp>
#include <onsem/common/enum/relativepersonwithoutnumber.hpp>
#include <onsem/common/enum/relativeperson.hpp>
#include <onsem/texttosemantic/dbtype/inflection/inflections.hpp>
#include "../../api.hpp"


namespace onsem
{

struct ONSEM_TEXTTOSEMANTIC_API PronominalInflection
{
  PronominalInflection() = default;
  PronominalInflection(const std::string& pInflectionalCode);
  bool operator==(const PronominalInflection& pOther) const;
  RelativePerson person() const;

  RelativePersonWithoutNumber personWithoutNumber{RelativePersonWithoutNumber::UNKNOWN};
  SemanticGenderType gender{SemanticGenderType::UNKNOWN};
  SemanticNumberType number{SemanticNumberType::UNKNOWN};
};


struct ONSEM_TEXTTOSEMANTIC_API PronominalInflections : public Inflections
{
  PronominalInflections();
  PronominalInflections(const std::vector<std::string>& pInflectionalCodes);
  bool operator==(const PronominalInflections& pOther) const;
  bool empty() const { return inflections.empty(); }

  virtual PronominalInflections& getPronominalI() { return *this; }
  virtual const PronominalInflections& getPronominalI() const { return *this; }
  virtual PronominalInflections* getPronominalIPtr() { return this; }
  virtual const PronominalInflections* getPronominalIPtr() const { return this; }

  std::list<PronominalInflection> inflections;
};





inline std::ostream& operator<<(std::ostream& pOs, const PronominalInflection& pPronInfl)
{
  relativePersonWithoutNumber_toConcisePrint(pOs, pPronInfl.personWithoutNumber);
  gender_toConcisePrint(pOs, pPronInfl.gender);
  number_toConcisePrint(pOs, pPronInfl.number);
  return pOs;
}

inline std::ostream& operator<<(std::ostream& pOs, const PronominalInflections& pPronInfls)
{
  bool firstLoop = true;
  for (const auto& currInfl : pPronInfls.inflections)
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

#include "detail/pronominalinflections.hxx"

#endif // ONSEM_TEXTTOSEMANTIC_TYPE_INFLECTION_PRONOMINALINFLECTIONS_HPP
