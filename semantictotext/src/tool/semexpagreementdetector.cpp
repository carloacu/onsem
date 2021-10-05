#include <onsem/semantictotext/tool/semexpagreementdetector.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpressions.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/conceptset.hpp>

namespace onsem
{
namespace semExpAgreementDetector
{

namespace
{
void _getAgreementValueRec(TruenessValue& pRes,
                           char& pResConfidence,
                           const GroundedExpression& pGrdExp);

void _getAgreementOfChild(TruenessValue& pRes,
                          char& pResConfidence,
                          const GroundedExpression& pGrdExp,
                          GrammaticalType pGrammTypeOfChild)
{
  auto itChild = pGrdExp.children.find(pGrammTypeOfChild);
  if (itChild != pGrdExp.children.end())
  {
    const auto* specifierGrdExpPtr = itChild->second->getGrdExpPtr_SkipWrapperPtrs();
    if (specifierGrdExpPtr != nullptr)
      _getAgreementValueRec(pRes, pResConfidence, *specifierGrdExpPtr);
  }
}


void _getAgreementValueRec(TruenessValue& pRes,
                           char& pResConfidence,
                           const GroundedExpression& pGrdExp)
{
  bool hasEquBeVerb = false;
  for (const auto& currConcept : pGrdExp.grounding().concepts)
  {
    const std::string& conceptName = currConcept.first;
    if (pResConfidence < currConcept.second)
    {
      if (ConceptSet::doesConceptBeginWith(conceptName, "accordance_agreement_"))
      {
        pResConfidence = currConcept.second;
        pRes = TruenessValue::VAL_TRUE;
        break;
      }
      if (ConceptSet::doesConceptBeginWith(conceptName, "accordance_disagreement_"))
      {
        pResConfidence = currConcept.second;
        pRes = TruenessValue::VAL_FALSE;
        break;
      }
    }
    if (conceptName == "verb_equal_be")
      hasEquBeVerb = true;
  }

  if (hasEquBeVerb)
    _getAgreementOfChild(pRes, pResConfidence, pGrdExp, GrammaticalType::OBJECT);
  _getAgreementOfChild(pRes, pResConfidence, pGrdExp, GrammaticalType::SPECIFIER);
}
}


TruenessValue semExpToAgreementValue(const SemanticExpression& pSemExp)
{
  switch (pSemExp.type)
  {
  case SemanticExpressionType::GROUNDED:
  {
    const GroundedExpression& grdExp = pSemExp.getGrdExp();
    return getAgreementValue(grdExp);
  }
  case SemanticExpressionType::LIST:
  {
    const ListExpression& listExp = pSemExp.getListExp();
    for (const auto& currElt : listExp.elts)
    {
      TruenessValue agreementVal = semExpToAgreementValue(*currElt);
      if (agreementVal != TruenessValue::UNKNOWN)
        return agreementVal;
    }
    break;
  }
  case SemanticExpressionType::INTERPRETATION:
  {
    const InterpretationExpression& intExp = pSemExp.getIntExp();
    return semExpToAgreementValue(*intExp.interpretedExp);
  }
  case SemanticExpressionType::FEEDBACK:
  {
    const FeedbackExpression& fdkExp = pSemExp.getFdkExp();
    TruenessValue agreementVal = semExpToAgreementValue(*fdkExp.feedbackExp);
    if (agreementVal != TruenessValue::UNKNOWN)
      return agreementVal;
    return semExpToAgreementValue(*fdkExp.concernedExp);
  }
  case SemanticExpressionType::ANNOTATED:
  {
    const AnnotatedExpression& annExp = pSemExp.getAnnExp();
    return semExpToAgreementValue(*annExp.semExp);
  }
  case SemanticExpressionType::METADATA:
  {
    const MetadataExpression& metadataExp = pSemExp.getMetadataExp();
    return semExpToAgreementValue(*metadataExp.semExp);
  }
  case SemanticExpressionType::FIXEDSYNTHESIS:
  {
    const FixedSynthesisExpression& fSynthExp = pSemExp.getFSynthExp();
    return semExpToAgreementValue(fSynthExp.getSemExp());
  }
  case SemanticExpressionType::COMMAND:
  case SemanticExpressionType::CONDITION:
  case SemanticExpressionType::COMPARISON:
  case SemanticExpressionType::SETOFFORMS:
    break;
  }

  return TruenessValue::UNKNOWN;
}



TruenessValue getAgreementValue(const GroundedExpression& pGrdExp)
{
  TruenessValue res = TruenessValue::UNKNOWN;
  char resConfidence = 0;
  _getAgreementValueRec(res, resConfidence, pGrdExp);
  return res;
}


} // End of namespace semExpAgreementDetector
} // End of namespace onsem
