#include <onsem/texttosemantic/dbtype/inflection/inflections.hpp>
#include <onsem/texttosemantic/dbtype/inflection/adjectivalinflections.hpp>
#include <onsem/texttosemantic/dbtype/inflection/adverbialinflections.hpp>
#include <onsem/texttosemantic/dbtype/inflection/nominalinflections.hpp>
#include <onsem/texttosemantic/dbtype/inflection/pronominalinflections.hpp>
#include <onsem/texttosemantic/dbtype/inflection/verbalinflections.hpp>
#include <onsem/common/utility/string.hpp>
#include <assert.h>

namespace onsem
{

AdjectivalInflections& Inflections::getAdjectivalI()
{
  assert(false);
  return *dynamic_cast<AdjectivalInflections*>(this);
}

const AdjectivalInflections& Inflections::getAdjectivalI() const
{
  assert(false);
  return *dynamic_cast<const AdjectivalInflections*>(this);
}


AdverbialInflections& Inflections::getAdverbialI()
{
  assert(false);
  return *dynamic_cast<AdverbialInflections*>(this);
}

const AdverbialInflections& Inflections::getAdverbialI() const
{
  assert(false);
  return *dynamic_cast<const AdverbialInflections*>(this);
}


NominalInflections& Inflections::getNominalI()
{
  assert(false);
  return *dynamic_cast<NominalInflections*>(this);
}

const NominalInflections& Inflections::getNominalI() const
{
  assert(false);
  return *dynamic_cast<const NominalInflections*>(this);
}


PronominalInflections& Inflections::getPronominalI()
{
  assert(false);
  return *dynamic_cast<PronominalInflections*>(this);
}

const PronominalInflections& Inflections::getPronominalI() const
{
  assert(false);
  return *dynamic_cast<const PronominalInflections*>(this);
}



VerbalInflections& Inflections::getVerbalI()
{
  assert(false);
  return *dynamic_cast<VerbalInflections*>(this);
}

const VerbalInflections& Inflections::getVerbalI() const
{
  assert(false);
  return *dynamic_cast<const VerbalInflections*>(this);
}


std::unique_ptr<Inflections> Inflections::create
(InflectionType pType,
 const std::vector<std::string>& pInflectionalCodes)
{
  switch (pType)
  {
  case InflectionType::ADJECTIVAL:
    return std::make_unique<AdjectivalInflections>(pInflectionalCodes);
  case InflectionType::ADVERBIAL:
    return std::make_unique<AdverbialInflections>(pInflectionalCodes);
  case InflectionType::NOMINAL:
    return std::make_unique<NominalInflections>(pInflectionalCodes);
  case InflectionType::PRONOMINAL:
    return std::make_unique<PronominalInflections>(pInflectionalCodes);
  case InflectionType::VERBAL:
    return std::make_unique<VerbalInflections>(pInflectionalCodes);
  case InflectionType::EMPTY:
    return std::make_unique<EmptyInflections>();
  }
  assert(false);
  return std::unique_ptr<Inflections>();
}

std::unique_ptr<Inflections> Inflections::create
(InflectionType pType,
 const std::string& pInflectionalCodes)
{
  if (!pInflectionalCodes.empty())
  {
    std::vector<std::string> vecOfInflectionalCodes;
    mystd::split(vecOfInflectionalCodes, pInflectionalCodes, ",");
    return create(pType, vecOfInflectionalCodes);
  }
  return create(pType, std::vector<std::string>());
}


std::unique_ptr<Inflections> Inflections::clone() const
{
  switch (type)
  {
  case InflectionType::ADJECTIVAL:
    return std::make_unique<AdjectivalInflections>(getAdjectivalI());
  case InflectionType::ADVERBIAL:
    return std::make_unique<AdverbialInflections>(getAdverbialI());
  case InflectionType::NOMINAL:
    return std::make_unique<NominalInflections>(getNominalI());
  case InflectionType::PRONOMINAL:
    return std::make_unique<PronominalInflections>(getPronominalI());
  case InflectionType::VERBAL:
    return std::make_unique<VerbalInflections>(getVerbalI());
  case InflectionType::EMPTY:
    return std::make_unique<EmptyInflections>();
  }
  assert(false);
  return std::unique_ptr<Inflections>();
}

std::unique_ptr<Inflections> Inflections::getOtherInflectionsType(InflectionType pType) const
{
  switch (type)
  {
  case InflectionType::PRONOMINAL:
  {
    if (pType == InflectionType::NOMINAL)
    {
      auto res = std::make_unique<NominalInflections>();
      const auto& inflections = getPronominalI().inflections;
      for (const auto& currInfl : inflections)
        res->inflections.emplace_back(currInfl.gender, currInfl.number);
      return res;
    }
    break;
  }
  default:
    break;
  }

  if (type == pType)
    return clone();
  return {};
}


bool Inflections::operator==(const Inflections& pOther) const
{
  if (type != pOther.type)
    return false;
  switch (type)
  {
  case InflectionType::ADJECTIVAL:
    return getAdjectivalI() == pOther.getAdjectivalI();
  case InflectionType::ADVERBIAL:
    return getAdverbialI() == pOther.getAdverbialI();
  case InflectionType::NOMINAL:
    return getNominalI() == pOther.getNominalI();
  case InflectionType::PRONOMINAL:
    return getPronominalI() == pOther.getPronominalI();
  case InflectionType::VERBAL:
    return getVerbalI() == pOther.getVerbalI();
  case InflectionType::EMPTY:
    return true;
  }
  assert(false);
  return true;
}


void Inflections::concisePrint(std::ostream& pOs) const
{
  switch (type)
  {
  case InflectionType::ADJECTIVAL:
    pOs << getAdjectivalI();
    break;
  case InflectionType::ADVERBIAL:
    pOs << getAdverbialI();
    break;
  case InflectionType::NOMINAL:
    pOs << getNominalI();
    break;
  case InflectionType::PRONOMINAL:
    pOs << getPronominalI();
    break;
  case InflectionType::VERBAL:
    pOs << getVerbalI();
    break;
  case InflectionType::EMPTY:
    break;
  }
}



} // End of namespace onsem
