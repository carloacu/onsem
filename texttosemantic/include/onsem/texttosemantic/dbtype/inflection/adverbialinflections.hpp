#ifndef ONSEM_TEXTTOSEMANTIC_TYPE_INFLECTION_ADVERBIALINFLECTIONS_HPP
#define ONSEM_TEXTTOSEMANTIC_TYPE_INFLECTION_ADVERBIALINFLECTIONS_HPP

#include <list>
#include <onsem/common/enum/comparisontype.hpp>
#include <onsem/common/enum/semanticgendertype.hpp>
#include <onsem/common/enum/semanticnumbertype.hpp>
#include <onsem/texttosemantic/dbtype/inflection/inflections.hpp>
#include "../../api.hpp"


namespace onsem
{

struct ONSEM_TEXTTOSEMANTIC_API AdverbialInflection
{
  AdverbialInflection() = default;
  AdverbialInflection(ComparisonType pComparisonType);
  AdverbialInflection(const std::string& pInflectionalCode);
  bool operator==(const AdverbialInflection& pOther) const;

  ComparisonType comparisonType{ComparisonType::NONE};
};


struct ONSEM_TEXTTOSEMANTIC_API AdverbialInflections : public Inflections
{
  AdverbialInflections();
  AdverbialInflections(const std::vector<std::string>& pInflectionalCodes);
  bool operator==(const AdverbialInflections& pOther) const;

  virtual AdverbialInflections& getAdverbialI() { return *this; }
  virtual const AdverbialInflections& getAdverbialI() const { return *this; }
  virtual AdverbialInflections* getAdverbialIPtr() { return this; }
  virtual const AdverbialInflections* getAdverbialIPtr() const { return this; }

  std::list<AdverbialInflection> inflections;
};



inline std::ostream& operator<<(std::ostream& pOs, const AdverbialInflection& pAdvInfl)
{
  comparisonType_toConcisePrint(pOs, pAdvInfl.comparisonType);
  return pOs;
}

inline std::ostream& operator<<(std::ostream& pOs, const AdverbialInflections& pAdvInfls)
{
  bool firstLoop = true;
  for (const auto& currInfl : pAdvInfls.inflections)
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

#include "detail/adverbialinflections.hxx"

#endif // ONSEM_TEXTTOSEMANTIC_TYPE_INFLECTION_ADVERBIALINFLECTIONS_HPP
