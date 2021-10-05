#ifndef ONSEM_TEXTTOSEMANTIC_TYPE_INFLECTION_ADJECTIVALINFLECTIONS_HXX
#define ONSEM_TEXTTOSEMANTIC_TYPE_INFLECTION_ADJECTIVALINFLECTIONS_HXX

#include "../adjectivalinflections.hpp"


namespace onsem
{

inline AdjectivalInflection::AdjectivalInflection(SemanticGenderType pGender,
                                                  SemanticNumberType pNumber)
  : gender(pGender),
    number(pNumber),
    comparisonType(ComparisonType::NONE)
{
}

inline AdjectivalInflection::AdjectivalInflection(ComparisonType pComparisonType)
  : gender(SemanticGenderType::UNKNOWN),
    number(SemanticNumberType::UNKNOWN),
    comparisonType(pComparisonType)
{
}

inline AdjectivalInflection::AdjectivalInflection(const std::string& pInflectionalCode)
  : gender(SemanticGenderType::UNKNOWN),
    number(SemanticNumberType::UNKNOWN),
    comparisonType(ComparisonType::NONE)
{
  bool genderFilled = false;
  bool numberFilled = false;
  bool comparisonTypeFilled = false;
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
    if (!comparisonTypeFilled)
    {
      comparisonTypeFilled = comparisonType_fromConcisePrint(comparisonType, c);
      if (comparisonTypeFilled)
        continue;
    }
    throw std::runtime_error("The inflectional code: \"" + pInflectionalCode + "\" is badly formatted!");
  }
}

inline bool AdjectivalInflection::operator==(const AdjectivalInflection& pOther) const
{
  return gender == pOther.gender &&
      number == pOther.number &&
      comparisonType == pOther.comparisonType;
}







inline AdjectivalInflections::AdjectivalInflections()
  : Inflections(InflectionType::ADJECTIVAL),
    inflections()
{
}

inline AdjectivalInflections::AdjectivalInflections(const std::vector<std::string>& pInflectionalCodes)
  : Inflections(InflectionType::ADJECTIVAL),
    inflections()
{
  for (const auto& currInflCode : pInflectionalCodes)
    inflections.emplace_back(currInflCode);
}

inline bool AdjectivalInflections::operator==(const AdjectivalInflections& pOther) const
{
  return inflections == pOther.inflections;
}


} // End of namespace onsem


#endif // ONSEM_TEXTTOSEMANTIC_TYPE_INFLECTION_ADJECTIVALINFLECTIONS_HXX
