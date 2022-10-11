#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticgrouding.hpp>
#include <onsem/texttosemantic/dbtype/semanticgroundings.hpp>
#include <onsem/common/utility/make_unique.hpp>

namespace onsem
{

std::unique_ptr<SemanticGrounding> SemanticGrounding::make(SemanticGroudingType pType)
{
  switch (pType)
  {
  case SemanticGroudingType::AGENT:
    return mystd::make_unique<SemanticAgentGrounding>();
  case SemanticGroudingType::GENERIC:
    return mystd::make_unique<SemanticGenericGrounding>();
  case SemanticGroudingType::STATEMENT:
    return mystd::make_unique<SemanticStatementGrounding>();
  case SemanticGroudingType::TIME:
    return mystd::make_unique<SemanticTimeGrounding>();
  case SemanticGroudingType::TEXT:
    return mystd::make_unique<SemanticTextGrounding>("");
  case SemanticGroudingType::DURATION:
    return mystd::make_unique<SemanticDurationGrounding>();
  case SemanticGroudingType::LANGUAGE:
    return mystd::make_unique<SemanticLanguageGrounding>(SemanticLanguageEnum::UNKNOWN);
  case SemanticGroudingType::RELATIVELOCATION:
    return mystd::make_unique<SemanticRelativeLocationGrounding>(SemanticRelativeLocationType::L_ABOVE);
  case SemanticGroudingType::RELATIVETIME:
    return mystd::make_unique<SemanticRelativeTimeGrounding>(SemanticRelativeTimeType::AFTER);
  case SemanticGroudingType::RELATIVEDURATION:
    return mystd::make_unique<SemanticRelativeDurationGrounding>(SemanticRelativeDurationType::UNTIL);
  case SemanticGroudingType::RESOURCE:
    return mystd::make_unique<SemanticResourceGrounding>("", SemanticLanguageEnum::UNKNOWN, "");
  case SemanticGroudingType::LENGTH:
    return mystd::make_unique<SemanticLengthGrounding>();
  case SemanticGroudingType::META:
    return mystd::make_unique<SemanticMetaGrounding>(SemanticGroudingType::META, 0);
  case SemanticGroudingType::NAME:
    return mystd::make_unique<SemanticNameGrounding>("");
  case SemanticGroudingType::CONCEPTUAL:
    return mystd::make_unique<SemanticConceptualGrounding>();
  case SemanticGroudingType::UNITY:
    return mystd::make_unique<SemanticUnityGrounding>(SemanticLengthUnity::CENTIMETER);
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
  case SemanticGroudingType::AGENT:
    return getAgentGrounding().isEqual(pOther.getAgentGrounding());
  case SemanticGroudingType::GENERIC:
    return getGenericGrounding().isEqual(pOther.getGenericGrounding());
  case SemanticGroudingType::STATEMENT:
    return getStatementGrounding().isEqual(pOther.getStatementGrounding());
  case SemanticGroudingType::TIME:
    return getTimeGrounding().isEqual(pOther.getTimeGrounding());
  case SemanticGroudingType::TEXT:
    return getTextGrounding().isEqual(pOther.getTextGrounding());
  case SemanticGroudingType::DURATION:
    return getDurationGrounding().isEqual(pOther.getDurationGrounding());
  case SemanticGroudingType::LANGUAGE:
    return getLanguageGrounding().isEqual(pOther.getLanguageGrounding());
  case SemanticGroudingType::RELATIVELOCATION:
    return getRelLocationGrounding().isEqual(pOther.getRelLocationGrounding());
  case SemanticGroudingType::RELATIVETIME:
    return getRelTimeGrounding().isEqual(pOther.getRelTimeGrounding());
  case SemanticGroudingType::RELATIVEDURATION:
    return getRelDurationGrounding().isEqual(pOther.getRelDurationGrounding());
  case SemanticGroudingType::RESOURCE:
    return getResourceGrounding().isEqual(pOther.getResourceGrounding());
  case SemanticGroudingType::LENGTH:
    return getLengthGrounding().isEqual(pOther.getLengthGrounding());
  case SemanticGroudingType::META:
    return getMetaGrounding().isEqual(pOther.getMetaGrounding());
  case SemanticGroudingType::NAME:
    return getNameGrounding().isEqual(pOther.getNameGrounding());
  case SemanticGroudingType::CONCEPTUAL:
    return getConceptualGrounding().isEqual(pOther.getConceptualGrounding());
  case SemanticGroudingType::UNITY:
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

bool SemanticGrounding::isEmpty() const
{
  const SemanticConceptualGrounding* cptGrd = getConceptualGroundingPtr();
  return cptGrd != nullptr &&
      cptGrd->concepts.empty();
}


} // End of namespace onsem
