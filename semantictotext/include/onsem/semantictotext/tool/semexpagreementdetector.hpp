#ifndef ONSEM_SEMANTICTOTEXT_TOOL_SEMEXPAGREEMENTDETECTOR_HPP
#define ONSEM_SEMANTICTOTEXT_TOOL_SEMEXPAGREEMENTDETECTOR_HPP

#include <list>
#include <memory>
#include <onsem/texttosemantic/dbtype/misc/truenessvalue.hpp>
#include "../api.hpp"

namespace onsem
{
struct GroundedExpression;
struct SemanticExpression;

namespace semExpAgreementDetector
{

ONSEMSEMANTICTOTEXT_API
TruenessValue semExpToAgreementValue(const SemanticExpression& pSemExp);

ONSEMSEMANTICTOTEXT_API
TruenessValue getAgreementValue(const GroundedExpression& pGrdExp);


} // End of namespace semExpAgreementDetector
} // End of namespace onsem



#endif // ONSEM_SEMANTICTOTEXT_TOOL_SEMEXPAGREEMENTDETECTOR_HPP
