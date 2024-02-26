#ifndef ONSEM_TEXTTOSEMANTIC_TYPE_INFLECTION_INFLECTIONS_HPP
#define ONSEM_TEXTTOSEMANTIC_TYPE_INFLECTION_INFLECTIONS_HPP

#include <memory>
#include <vector>
#include <onsem/texttosemantic/dbtype/inflection/inflectiontype.hpp>
#include "../../api.hpp"

namespace onsem
{
struct AdjectivalInflections;
struct AdverbialInflections;
struct NominalInflections;
struct PronominalInflections;
struct VerbalInflections;



struct ONSEM_TEXTTOSEMANTIC_API Inflections
{
  virtual ~Inflections() {}
  InflectionType type;

  static std::unique_ptr<Inflections> create(InflectionType pType,
                                             const std::vector<std::string>& pInflectionalCodes);
  static std::unique_ptr<Inflections> create(InflectionType pType,
                                             const std::string& pInflectionalCodes);
  std::unique_ptr<Inflections> clone() const;
  std::unique_ptr<Inflections> getOtherInflectionsType(InflectionType pType) const;
  bool operator==(const Inflections& pOther) const;
  void concisePrint(std::ostream& pOs) const;

  virtual AdjectivalInflections& getAdjectivalI();
  virtual const AdjectivalInflections& getAdjectivalI() const;
  virtual AdjectivalInflections* getAdjectivalIPtr() { return nullptr; }
  virtual const AdjectivalInflections* getAdjectivalIPtr() const { return nullptr; }

  virtual AdverbialInflections& getAdverbialI();
  virtual const AdverbialInflections& getAdverbialI() const;
  virtual AdverbialInflections* getAdverbialIPtr() { return nullptr; }
  virtual const AdverbialInflections* getAdverbialIPtr() const { return nullptr; }

  virtual NominalInflections& getNominalI();
  virtual const NominalInflections& getNominalI() const;
  virtual NominalInflections* getNominalIPtr() { return nullptr; }
  virtual const NominalInflections* getNominalIPtr() const { return nullptr; }

  virtual PronominalInflections& getPronominalI();
  virtual const PronominalInflections& getPronominalI() const;
  virtual PronominalInflections* getPronominalIPtr() { return nullptr; }
  virtual const PronominalInflections* getPronominalIPtr() const { return nullptr; }

  virtual VerbalInflections& getVerbalI();
  virtual const VerbalInflections& getVerbalI() const;
  virtual VerbalInflections* getVerbalIPtr() { return nullptr; }
  virtual const VerbalInflections* getVerbalIPtr() const { return nullptr; }


protected:
  Inflections(InflectionType pType)
    : type(pType)
  {
  }
};


struct ONSEM_TEXTTOSEMANTIC_API EmptyInflections : public Inflections
{
  EmptyInflections()
    : Inflections(InflectionType::EMPTY)
  {
  }
};


} // End of namespace onsem


#endif // ONSEM_TEXTTOSEMANTIC_TYPE_INFLECTION_INFLECTIONS_HPP
