#ifndef ONSEM_SEMANTICTOTEXT_CONTROLLER_STEPS_UNKNOWNINFOSGETTER_HPP
#define ONSEM_SEMANTICTOTEXT_CONTROLLER_STEPS_UNKNOWNINFOSGETTER_HPP

#include <set>
#include <map>
#include <list>
#include <cstdint>
#include <onsem/common/enum/grammaticaltype.hpp>
#include <onsem/common/enum/semanticlanguagetype.hpp>
#include <onsem/common/enum/semanticrequesttype.hpp>
#include <onsem/texttosemantic/dbtype/misc/parameterswithvalue.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemorysentenceid.hpp>
#include "../../semanticmemory/sentenceslinks.hpp"

namespace onsem
{
struct SemanticMemorySentence;
struct GroundedExpression;
struct SemanticExpression;
struct SemControllerWorkingStruct;
struct SemanticMemoryBlockViewer;
// semExp -> childType to complete
typedef std::map<intSemId, std::list<GrammaticalType> > GrdKnowToUnlinked;

namespace unknownInfosGetter
{

bool checkIfMatchAndGetParams(IndexToSubNameToParameterValue& pParam,
                              GrdKnowToUnlinked* pIncompleteRelations,
                              const SemanticMemorySentence& pSemMemSent,
                              const GroundedExpression& pGrdExp,
                              const SemControllerWorkingStruct& pWorkStruct,
                              const SemanticMemoryBlockViewer& pMemViewer);

bool splitCompeleteIncompleteOfActions(SemControllerWorkingStruct& pWorkStruct,
                                       SemanticMemoryBlockViewer& pMemViewer,
                                       GrdKnowToUnlinked& pIncompleteRelations,
                                       const SentenceLinks<false>& pIdsToSentences,
                                       const GroundedExpression& pGrdExp);

bool getRequestToAskForPrecision(SemanticRequestType& pRequestType,
                                 const GrdKnowToUnlinked& pIncompleteRelations);

bool isItfUnknown(const SemControllerWorkingStruct& pWorkStruct,
                  const SemanticMemoryBlockViewer& pMemViewer,
                  const GroundedExpression& pGrdExp);


} // End of namespace unknownInfosGetter
} // End of namespace onsem



#endif // ONSEM_SEMANTICTOTEXT_CONTROLLER_STEPS_UNKNOWNINFOSGETTER_HPP
