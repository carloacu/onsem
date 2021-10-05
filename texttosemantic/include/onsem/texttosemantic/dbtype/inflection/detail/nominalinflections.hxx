#ifndef ONSEM_TEXTTOSEMANTIC_TYPE_INFLECTION_NOMINALINFLECTIONS_HXX
#define ONSEM_TEXTTOSEMANTIC_TYPE_INFLECTION_NOMINALINFLECTIONS_HXX

#include "../nominalinflections.hpp"


namespace onsem
{

inline NominalInflection::NominalInflection(SemanticGenderType pGender,
                                            SemanticNumberType pNumber)
  : gender(pGender),
    number(pNumber)
{
}

inline NominalInflection::NominalInflection(const std::string& pInflectionalCode)
  : gender(SemanticGenderType::UNKNOWN),
    number(SemanticNumberType::UNKNOWN)
{
  bool genderFilled = false;
  bool numberFilled = false;
  for (const char& c : pInflectionalCode)
  {
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

inline bool NominalInflection::operator==(const NominalInflection& pOther) const
{
  return gender == pOther.gender &&
      number == pOther.number;
}

inline bool NominalInflection::operator<(const NominalInflection& pOther) const
{
  if (gender != pOther.gender)
    return gender < pOther.gender;
  return number < pOther.number;
}





inline NominalInflections::NominalInflections()
  : Inflections(InflectionType::NOMINAL),
    inflections()
{
}

inline NominalInflections::NominalInflections(const std::vector<std::string>& pInflectionalCodes)
  : Inflections(InflectionType::NOMINAL),
    inflections()
{
  for (const auto& currInflCode : pInflectionalCodes)
    inflections.emplace_back(currInflCode);
}


inline bool NominalInflections::operator==(const NominalInflections& pOther) const
{
  return inflections == pOther.inflections;
}


} // End of namespace onsem


#endif // ONSEM_TEXTTOSEMANTIC_TYPE_INFLECTION_NOMINALINFLECTIONS_HXX
