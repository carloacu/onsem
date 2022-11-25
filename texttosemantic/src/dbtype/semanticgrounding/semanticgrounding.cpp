#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticgrounding.hpp>
#include <onsem/texttosemantic/dbtype/semanticgroundings.hpp>

namespace onsem
{

std::unique_ptr<SemanticGrounding> SemanticGrounding::make(SemanticGroundingType pType)
{
  switch (pType)
  {
  case SemanticGroundingType::AGENT:
    return std::make_unique<SemanticAgentGrounding>();
  case SemanticGroundingType::GENERIC:
    return std::make_unique<SemanticGenericGrounding>();
  case SemanticGroundingType::STATEMENT:
    return std::make_unique<SemanticStatementGrounding>();
  case SemanticGroundingType::TIME:
    return std::make_unique<SemanticTimeGrounding>();
  case SemanticGroundingType::TEXT:
    return std::make_unique<SemanticTextGrounding>("");
  case SemanticGroundingType::DURATION:
    return std::make_unique<SemanticDurationGrounding>();
  case SemanticGroundingType::LANGUAGE:
    return std::make_unique<SemanticLanguageGrounding>(SemanticLanguageEnum::UNKNOWN);
  case SemanticGroundingType::RELATIVELOCATION:
    return std::make_unique<SemanticRelativeLocationGrounding>(SemanticRelativeLocationType::L_ABOVE);
  case SemanticGroundingType::RELATIVETIME:
    return std::make_unique<SemanticRelativeTimeGrounding>(SemanticRelativeTimeType::AFTER);
  case SemanticGroundingType::RELATIVEDURATION:
    return std::make_unique<SemanticRelativeDurationGrounding>(SemanticRelativeDurationType::UNTIL);
  case SemanticGroundingType::RESOURCE:
    return std::make_unique<SemanticResourceGrounding>("", SemanticLanguageEnum::UNKNOWN, "");
  case SemanticGroundingType::LENGTH:
    return std::make_unique<SemanticLengthGrounding>();
  case SemanticGroundingType::META:
    return std::make_unique<SemanticMetaGrounding>(SemanticGroundingType::META, 0);
  case SemanticGroundingType::NAME:
    return std::make_unique<SemanticNameGrounding>("");
  case SemanticGroundingType::CONCEPTUAL:
    return std::make_unique<SemanticConceptualGrounding>();
  case SemanticGroundingType::UNITY:
    return std::make_unique<SemanticUnityGrounding>(SemanticLengthUnity::CENTIMETER);
  }

  assert(false);
  return std::unique_ptr<SemanticGrounding>();
}


const SemanticGenericGrounding& SemanticGrounding::getGenericGrounding() const
{
  assert(false);
  return *dynamic_cast<const SemanticGenericGrounding*>(this);
}

SemanticGenericGrounding& SemanticGrounding::getGenericGrounding()
{
  assert(false);
  return *dynamic_cast<SemanticGenericGrounding*>(this);
}


const SemanticStatementGrounding& SemanticGrounding::getStatementGrounding() const
{
  assert(false);
  return *dynamic_cast<const SemanticStatementGrounding*>(this);
}

SemanticStatementGrounding& SemanticGrounding::getStatementGrounding()
{
  assert(false);
  return *dynamic_cast<SemanticStatementGrounding*>(this);
}


const SemanticAgentGrounding& SemanticGrounding::getAgentGrounding() const
{
  assert(false);
  return *dynamic_cast<const SemanticAgentGrounding*>(this);
}

SemanticAgentGrounding& SemanticGrounding::getAgentGrounding()
{
  assert(false);
  return *dynamic_cast<SemanticAgentGrounding*>(this);
}


const SemanticTimeGrounding& SemanticGrounding::getTimeGrounding() const
{
  assert(false);
  return *dynamic_cast<const SemanticTimeGrounding*>(this);
}

SemanticTimeGrounding& SemanticGrounding::getTimeGrounding()
{
  assert(false);
  return *dynamic_cast<SemanticTimeGrounding*>(this);
}


const SemanticLengthGrounding& SemanticGrounding::getLengthGrounding() const
{
  assert(false);
  return *dynamic_cast<const SemanticLengthGrounding*>(this);
}

SemanticLengthGrounding& SemanticGrounding::getLengthGrounding()
{
  assert(false);
  return *dynamic_cast<SemanticLengthGrounding*>(this);
}

const SemanticDurationGrounding& SemanticGrounding::getDurationGrounding() const
{
  assert(false);
  return *dynamic_cast<const SemanticDurationGrounding*>(this);
}

SemanticDurationGrounding& SemanticGrounding::getDurationGrounding()
{
  assert(false);
  return *dynamic_cast<SemanticDurationGrounding*>(this);
}


const SemanticTextGrounding& SemanticGrounding::getTextGrounding() const
{
  assert(false);
  return *dynamic_cast<const SemanticTextGrounding*>(this);
}

SemanticTextGrounding& SemanticGrounding::getTextGrounding()
{
  assert(false);
  return *dynamic_cast<SemanticTextGrounding*>(this);
}


const SemanticLanguageGrounding& SemanticGrounding::getLanguageGrounding() const
{
  assert(false);
  return *dynamic_cast<const SemanticLanguageGrounding*>(this);
}

SemanticLanguageGrounding& SemanticGrounding::getLanguageGrounding()
{
  assert(false);
  return *dynamic_cast<SemanticLanguageGrounding*>(this);
}


const SemanticRelativeLocationGrounding& SemanticGrounding::getRelLocationGrounding() const
{
  assert(false);
  return *dynamic_cast<const SemanticRelativeLocationGrounding*>(this);
}

SemanticRelativeLocationGrounding& SemanticGrounding::getRelLocationGrounding()
{
  assert(false);
  return *dynamic_cast<SemanticRelativeLocationGrounding*>(this);
}


const SemanticRelativeTimeGrounding& SemanticGrounding::getRelTimeGrounding() const
{
  assert(false);
  return *dynamic_cast<const SemanticRelativeTimeGrounding*>(this);
}

SemanticRelativeTimeGrounding& SemanticGrounding::getRelTimeGrounding()
{
  assert(false);
  return *dynamic_cast<SemanticRelativeTimeGrounding*>(this);
}


const SemanticRelativeDurationGrounding& SemanticGrounding::getRelDurationGrounding() const
{
  assert(false);
  return *dynamic_cast<const SemanticRelativeDurationGrounding*>(this);
}

SemanticRelativeDurationGrounding& SemanticGrounding::getRelDurationGrounding()
{
  assert(false);
  return *dynamic_cast<SemanticRelativeDurationGrounding*>(this);
}

const SemanticResourceGrounding& SemanticGrounding::getResourceGrounding() const
{
  assert(false);
  return *dynamic_cast<const SemanticResourceGrounding*>(this);
}

SemanticResourceGrounding& SemanticGrounding::getResourceGrounding()
{
  assert(false);
  return *dynamic_cast<SemanticResourceGrounding*>(this);
}


const SemanticMetaGrounding& SemanticGrounding::getMetaGrounding() const
{
  assert(false);
  return *dynamic_cast<const SemanticMetaGrounding*>(this);
}

SemanticMetaGrounding& SemanticGrounding::getMetaGrounding()
{
  assert(false);
  return *dynamic_cast<SemanticMetaGrounding*>(this);
}


const SemanticNameGrounding& SemanticGrounding::getNameGrounding() const
{
  assert(false);
  return *dynamic_cast<const SemanticNameGrounding*>(this);
}

SemanticNameGrounding& SemanticGrounding::getNameGrounding()
{
  assert(false);
  return *dynamic_cast<SemanticNameGrounding*>(this);
}


const SemanticConceptualGrounding& SemanticGrounding::getConceptualGrounding() const
{
  assert(false);
  return *dynamic_cast<const SemanticConceptualGrounding*>(this);
}

SemanticConceptualGrounding& SemanticGrounding::getConceptualGrounding()
{
  assert(false);
  return *dynamic_cast<SemanticConceptualGrounding*>(this);
}

const SemanticUnityGrounding& SemanticGrounding::getUnityGrounding() const
{
  assert(false);
  return *dynamic_cast<const SemanticUnityGrounding*>(this);
}

SemanticUnityGrounding& SemanticGrounding::getUnityGrounding()
{
  assert(false);
  return *dynamic_cast<SemanticUnityGrounding*>(this);
}


bool SemanticGrounding::operator==(const SemanticGrounding& pOther) const
{
  if (type != pOther.type)
    return false;

  switch (type)
  {
  case SemanticGroundingType::AGENT:
    return getAgentGrounding().isEqual(pOther.getAgentGrounding());
  case SemanticGroundingType::GENERIC:
    return getGenericGrounding().isEqual(pOther.getGenericGrounding());
  case SemanticGroundingType::STATEMENT:
    return getStatementGrounding().isEqual(pOther.getStatementGrounding());
  case SemanticGroundingType::TIME:
    return getTimeGrounding().isEqual(pOther.getTimeGrounding());
  case SemanticGroundingType::TEXT:
    return getTextGrounding().isEqual(pOther.getTextGrounding());
  case SemanticGroundingType::DURATION:
    return getDurationGrounding().isEqual(pOther.getDurationGrounding());
  case SemanticGroundingType::LANGUAGE:
    return getLanguageGrounding().isEqual(pOther.getLanguageGrounding());
  case SemanticGroundingType::RELATIVELOCATION:
    return getRelLocationGrounding().isEqual(pOther.getRelLocationGrounding());
  case SemanticGroundingType::RELATIVETIME:
    return getRelTimeGrounding().isEqual(pOther.getRelTimeGrounding());
  case SemanticGroundingType::RELATIVEDURATION:
    return getRelDurationGrounding().isEqual(pOther.getRelDurationGrounding());
  case SemanticGroundingType::RESOURCE:
    return getResourceGrounding().isEqual(pOther.getResourceGrounding());
  case SemanticGroundingType::LENGTH:
    return getLengthGrounding().isEqual(pOther.getLengthGrounding());
  case SemanticGroundingType::META:
    return getMetaGrounding().isEqual(pOther.getMetaGrounding());
  case SemanticGroundingType::NAME:
    return getNameGrounding().isEqual(pOther.getNameGrounding());
  case SemanticGroundingType::CONCEPTUAL:
    return getConceptualGrounding().isEqual(pOther.getConceptualGrounding());
  case SemanticGroundingType::UNITY:
    return getUnityGrounding().isEqual(pOther.getUnityGrounding());
  }
  assert(false);
  return false;
}


bool SemanticGrounding::_isMotherClassEqual(const SemanticGrounding& pOther) const
{
  return polarity == pOther.polarity &&
      concepts == pOther.concepts;
}

bool SemanticGrounding::_isMotherClassEmpty() const
{
  return polarity && concepts.empty();
}


bool SemanticGrounding::isEmpty() const
{
  const SemanticConceptualGrounding* cptGrd = getConceptualGroundingPtr();
  return cptGrd != nullptr &&
      cptGrd->concepts.empty();
}


} // End of namespace onsem
