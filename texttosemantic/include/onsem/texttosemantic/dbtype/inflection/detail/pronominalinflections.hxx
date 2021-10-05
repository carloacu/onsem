#ifndef ONSEM_TEXTTOSEMANTIC_TYPE_INFLECTION_PRONOMINALINFLECTIONS_HXX
#define ONSEM_TEXTTOSEMANTIC_TYPE_INFLECTION_PRONOMINALINFLECTIONS_HXX

#include "../pronominalinflections.hpp"


namespace onsem
{

inline bool PronominalInflection::operator==(const PronominalInflection& pOther) const
{
  return personWithoutNumber == pOther.personWithoutNumber &&
      gender == pOther.gender &&
      number == pOther.number;
}


inline PronominalInflection::PronominalInflection(const std::string& pInflectionalCode)
  : personWithoutNumber(RelativePersonWithoutNumber::UNKNOWN),
    gender(SemanticGenderType::UNKNOWN),
    number(SemanticNumberType::UNKNOWN)
{
  bool personWithoutNumberFilled = false;
  bool genderFilled = false;
  bool numberFilled = false;
  for (const char& c : pInflectionalCode)
  {
    if (!personWithoutNumberFilled)
    {
      personWithoutNumberFilled = relativePersonWithoutNumber_fromConcisePrint(personWithoutNumber, c);
      if (personWithoutNumberFilled)
        continue;
    }
    if (!genderFilled)
    {
      genderFilled = gender_fromConcisePrint(gender, c);
      if (genderFilled)
        continue;
    }
    if (!numberFilled)
    {
      numberFilled = number_fromConcisePrint(number, c);
      if (numberFilled)
        continue;
    }
    throw std::runtime_error("The inflectional code: \"" + pInflectionalCode + "\" is badly formatted!");
  }
}


inline RelativePerson PronominalInflection::person() const
{
  return relativePerson_fromPersonWithoutNumberAndNumber(personWithoutNumber, number);
}







inline PronominalInflections::PronominalInflections()
  : Inflections(InflectionType::PRONOMINAL),
    inflections()
{
}

inline PronominalInflections::PronominalInflections(const std::vector<std::string>& pInflectionalCodes)
  : Inflections(InflectionType::PRONOMINAL),
    inflections()
{
  for (const auto& currInflCode : pInflectionalCodes)
    inflections.emplace_back(currInflCode);
}


inline bool PronominalInflections::operator==(const PronominalInflections& pOther) const
{
  return inflections == pOther.inflections;
}


} // End of namespace onsem


#endif // ONSEM_TEXTTOSEMANTIC_TYPE_INFLECTION_PRONOMINALINFLECTIONS_HXX
