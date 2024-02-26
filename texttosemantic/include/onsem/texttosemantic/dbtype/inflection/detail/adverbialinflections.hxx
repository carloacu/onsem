#ifndef ONSEM_TEXTTOSEMANTIC_TYPE_INFLECTION_ADVERBIALINFLECTIONS_HXX
#define ONSEM_TEXTTOSEMANTIC_TYPE_INFLECTION_ADVERBIALINFLECTIONS_HXX

#include "../adverbialinflections.hpp"


namespace onsem
{

inline AdverbialInflection::AdverbialInflection(ComparisonType pComparisonType)
  : comparisonType(pComparisonType)
{
}

inline AdverbialInflection::AdverbialInflection(const std::string& pInflectionalCode)
  : comparisonType(ComparisonType::NONE)
{
  bool comparisonTypeFilled = false;
  for (const char& c : pInflectionalCode)
  {
    if (!comparisonTypeFilled)
    {
      comparisonTypeFilled = comparisonType_fromConcisePrint(comparisonType, c);
      if (comparisonTypeFilled)
        continue;
    }
  }
}

inline bool AdverbialInflection::operator==(const AdverbialInflection& pOther) const
{
  return comparisonType == pOther.comparisonType;
}






inline AdverbialInflections::AdverbialInflections()
  : Inflections(InflectionType::ADVERBIAL),
    inflections()
{
}

inline AdverbialInflections::AdverbialInflections(const std::vector<std::string>& pInflectionalCodes)
  : Inflections(InflectionType::ADVERBIAL),
    inflections()
{
  for (const auto& currInflCode : pInflectionalCodes)
    inflections.emplace_back(currInflCode);
}

inline bool AdverbialInflections::operator==(const AdverbialInflections& pOther) const
{
  return inflections == pOther.inflections;
}


} // End of namespace onsem


#endif // ONSEM_TEXTTOSEMANTIC_TYPE_INFLECTION_ADVERBIALINFLECTIONS_HXX
