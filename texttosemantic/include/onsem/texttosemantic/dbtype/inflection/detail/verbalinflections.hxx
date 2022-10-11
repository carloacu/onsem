#ifndef ONSEM_TEXTTOSEMANTIC_TYPE_INFLECTION_VERBALINFLECTIONS_HXX
#define ONSEM_TEXTTOSEMANTIC_TYPE_INFLECTION_VERBALINFLECTIONS_HXX

#include "../verbalinflections.hpp"
#include <onsem/common/enum/relativepersonwithoutnumber.hpp>

namespace onsem
{

inline VerbalInflection::VerbalInflection(const std::string& pInflectionalCode)
  : person(RelativePerson::UNKNOWN),
    tense(LinguisticVerbTense::INFINITIVE),
    gender(SemanticGenderType::UNKNOWN)
{
  if (pInflectionalCode.empty())
    throw std::runtime_error("A verbal inflectional code should not be empty!");
  tense = linguisticVerbTense_fromChar(pInflectionalCode[0]);

  RelativePersonWithoutNumber personWithoutNumber = RelativePersonWithoutNumber::UNKNOWN;
  SemanticNumberType number = SemanticNumberType::UNKNOWN;
  bool personWithoutNumberFilled = false;
  bool genderFilled = false;
  bool numberFilled = false;
  for (std::size_t i = 1; i < pInflectionalCode.size(); ++i)
  {
    const char& c = pInflectionalCode[i];
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
  person = relativePerson_fromPersonWithoutNumberAndNumber(personWithoutNumber, number);
}

inline bool VerbalInflection::operator==(const VerbalInflection& pOther) const
{
  return person == pOther.person &&
      tense == pOther.tense &&
      gender == pOther.gender;
}




inline VerbalInflections::VerbalInflections()
  : Inflections(InflectionType::VERBAL),
    inflections()
{
}

inline VerbalInflections::VerbalInflections(const std::vector<std::string>& pInflectionalCodes)
  : Inflections(InflectionType::VERBAL),
    inflections()
{
  for (const auto& currInflCode : pInflectionalCodes)
    inflections.emplace_back(currInflCode);
}

inline bool VerbalInflections::operator==(const VerbalInflections& pOther) const
{
  return inflections == pOther.inflections;
}

inline std::unique_ptr<VerbalInflections> VerbalInflections::get_inflections_infinitive()
{
  auto res = std::make_unique<VerbalInflections>();
  res->inflections.emplace_back("W");
  return res;
}

inline std::unique_ptr<VerbalInflections> VerbalInflections::get_inflections_imperative()
{
  auto res = std::make_unique<VerbalInflections>();
  res->inflections.emplace_back("Y");
  return res;
}


} // End of namespace onsem


#endif // ONSEM_TEXTTOSEMANTIC_TYPE_INFLECTION_VERBALINFLECTIONS_HXX
