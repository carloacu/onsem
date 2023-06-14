#ifndef ONSEM_SEMANTICTOTEXT_CONTROLLER_STEPS_UNKNOWNINFOSGETTER_HPP
#define ONSEM_SEMANTICTOTEXT_CONTROLLER_STEPS_UNKNOWNINFOSGETTER_HPP

#include <set>
#include <map>
#include <list>
#include <cstdint>
#include <onsem/common/enum/grammaticaltype.hpp>
#include <onsem/common/enum/semanticlanguageenum.hpp>
#include <onsem/common/enum/semanticrequesttype.hpp>
#include <onsem/texttosemantic/dbtype/misc/parameterswithvalue.hpp>
#include <onsem/semantictotext/semanticmemory/links/groundedexpwithlinksid.hpp>
#include "../../semanticmemory/sentenceslinks.hpp"

namespace onsem
{
struct GroundedExpWithLinks;
struct GroundedExpression;
struct SemanticExpression;
struct SemControllerWorkingStruct;
struct SemanticMemoryBlockViewer;
struct InteractionContextContainer;
// semExp -> childType to complete
typedef std::map<intSemId, std::list<GrammaticalType> > GrdKnowToUnlinked;

namespace unknownInfosGetter
{

bool checkIfMatchAndGetParams(IndexToSubNameToParameterValue& pParam,
                              std::unique_ptr<onsem::InteractionContextContainer>& pSubIntContext,
                              GrdKnowToUnlinked* pIncompleteRelations,
                              const GroundedExpWithLinks& pSemMemSent,
                              const GroundedExpression& pGrdExp,
                              const SemControllerWorkingStruct& pWorkStruct,
                              const SemanticMemoryBlockViewer& pMemViewer);

bool splitCompeleteIncompleteOfActions(SemControllerWorkingStruct& pWorkStruct,
                                       SemanticMemoryBlockViewer& pMemViewer,
                                       GrdKnowToUnlinked& pIncompleteRelations,
                                       GroundedExpWithLinksWithParameters& pMemSentWithParams,
                                       const GroundedExpression& pGrdExp);

bool getRequestToAskForPrecision(SemanticRequestType& pRequestType,
                                 const GrdKnowToUnlinked& pIncompleteRelations);

bool isItfUnknown(const SemControllerWorkingStruct& pWorkStruct,
                  const SemanticMemoryBlockViewer& pMemViewer,
                  const GroundedExpression& pGrdExp);


} // End of namespace unknownInfosGetter
} // End of namespace onsem



#endif // ONSEM_SEMANTICTOTEXT_CONTROLLER_STEPS_UNKNOWNINFOSGETTER_HPP
