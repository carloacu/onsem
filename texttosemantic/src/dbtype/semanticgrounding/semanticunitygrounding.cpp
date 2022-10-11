#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticunitygrounding.hpp>
#include <assert.h>
#include <iostream>

namespace onsem
{


void SemanticUnityGrounding::setValue(SemanticLengthUnity pLengthUnity)
{
  typeOfUnity = TypeOfUnity::LENGTH;
  value = semanticLengthUnity_toChar(pLengthUnity);
}


void SemanticUnityGrounding::setValue(SemanticTimeUnity pTimeUnity)
{
  typeOfUnity = TypeOfUnity::TIME;
  value = semanticTimeUnity_toChar(pTimeUnity);
}

void SemanticUnityGrounding::setValue(TypeOfUnity pTypeOfUnity,
                                      const std::string& pValueStr)
{
  switch (pTypeOfUnity)
  {
  case TypeOfUnity::LENGTH:
    setValue(semanticLengthUnity_fromStr(pValueStr));
    break;
  case TypeOfUnity::TIME:
    setValue(semanticTimeUnity_fromStr(pValueStr));
    break;
  }
}

SemanticLengthUnity SemanticUnityGrounding::getLengthUnity() const
{
  if (typeOfUnity != TypeOfUnity::LENGTH)
  {
    assert(false);
    std::cerr << "getLengthUnity() called with a wrong unity" << std::endl;
    return SemanticLengthUnity::CENTIMETER;
  }
  return semanticLengthUnity_fromChar(value);
}

SemanticTimeUnity SemanticUnityGrounding::getTimeUnity() const
{
  if (typeOfUnity != TypeOfUnity::TIME)
  {
    assert(false);
    std::cerr << "getTimeUnity() called with a wrong unity" << std::endl;
    return SemanticTimeUnity::DAY;
  }
  return semanticTimeUnity_fromChar(value);
}

std::string SemanticUnityGrounding::getValueStr() const
{
  switch (typeOfUnity)
  {
  case TypeOfUnity::LENGTH:
    return semanticLengthUnity_toStr(semanticLengthUnity_fromChar(value));
  case TypeOfUnity::TIME:
    return semanticTimeUnity_toStr(semanticTimeUnity_fromChar(value));
  }
  assert(false);
  return "";
}

std::string SemanticUnityGrounding::getValueConcept() const
{
  switch (typeOfUnity)
  {
  case TypeOfUnity::LENGTH:
    return semanticLengthUnity_toConcept(semanticLengthUnity_fromChar(value));
  case TypeOfUnity::TIME:
    return semanticTimeUnity_toConcept(semanticTimeUnity_fromChar(value));
  }
  assert(false);
  return "";
}

} // End of namespace onsem
