#ifndef ONSEM_SEMANTICTOTEXT_SRC_CONTROLLER_STEPS_SEMANTICMEMORYLINKER_HPP
#define ONSEM_SEMANTICTOTEXT_SRC_CONTROLLER_STEPS_SEMANTICMEMORYLINKER_HPP

#include <map>
#include <onsem/common/enum/semanticrequesttype.hpp>
#include <onsem/common/enum/semanticverbtense.hpp>
#include <onsem/common/enum/verbgoalenum.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemorysentenceid.hpp>
#include "type/alsemexpcontrollertypes.hpp"
#include "../../semanticmemory/sentenceslinks.hpp"


namespace onsem
{
namespace linguistics
{
struct LinguisticDatabase;
}
struct SemanticStatementGrounding;
struct SemanticExpression;
struct SemanticMemory;
struct SemanticMemorySentence;
struct SemanticMemoryBlockViewer;
struct SemanticTriggerAxiomId;
struct SemanticDuration;
struct SemanticMemoryBlockPrivate;


namespace semanticMemoryLinker
{
struct SubRequestLinks;

struct RequestLinks
{
  bool isEmpty() const
  {
    return tense == SemanticVerbTense::UNKNOWN &&
        verbGoal == VerbGoalEnum::NOTIFICATION &&
        isEquVerb == false &&
        _semExpLinks.empty() && _semExpLinksSorted.empty() && occurenceRankFilter == nullptr;
  }

  void eraseChild(SemanticRequestType pRequestType);
  void fillSortedSemExps();
  void clear();
  SubRequestLinks& _addChildWithoutSortedContainer(SemanticRequestType pRequestType);

  const std::map<SemanticRequestType, SubRequestLinks>& semExpLinks() const { return _semExpLinks; }
  const std::list<std::pair<SemanticRequestType, const SubRequestLinks*>>& semExpLinksSorted() const { return _semExpLinksSorted; }

  SemanticVerbTense tense{SemanticVerbTense::UNKNOWN};
  VerbGoalEnum verbGoal{VerbGoalEnum::NOTIFICATION};
  bool isEquVerb{false};
  GrammaticalType gramTypeOfTheAnswer{GrammaticalType::UNKNOWN};
  const SemanticExpression* occurenceRankFilter{nullptr};

private:
  std::map<SemanticRequestType, SubRequestLinks> _semExpLinks{};
  std::list<std::pair<SemanticRequestType, const SubRequestLinks*>> _semExpLinksSorted{};
};


struct CrossedLinks
{
  // ask what is the semExps to add new matches
  std::list<RequestLinks> subReqListsToAdd{};
  // subordinates
  std::list<RequestLinks> subReqListsToFilter{};
  // sem exp subordinates stored in subReqListsToFilter
  std::set<const SemanticExpression*> semExpsWithSpecificFilter{};
};

struct SubRequestLinks
{
  // semantic expression
  std::list<const SemanticExpression*> semExps{};
  // some concepts to define what we look (can be filled instead of semExps)
  std::map<std::string, char> concepts{};

  CrossedLinks crossedLinks{};
};


void getLinksOfAGrdExp(RequestLinks& pReqLinks,
                       SemControllerWorkingStruct& pWorkStruct,
                       SemanticMemoryBlockViewer& pMemViewer,
                       const GroundedExpression& pGrdExp,
                       bool pAddSubordinateLinks = true);

bool matchAffirmationTrigger(SemControllerWorkingStruct& pWorkStruct,
                             SemanticMemoryBlockViewer& pMemViewer,
                             const RequestLinks& pReqLinks,
                             const GroundedExpression& pInputGrdExp);

bool addTriggerListExp(SemControllerWorkingStruct& pWorkStruct,
                       SemanticMemoryBlockViewer& pMemViewer,
                       const ListExpression& pListExp);

bool addTriggerCondExp(SemControllerWorkingStruct& pWorkStruct,
                       SemanticMemoryBlockViewer& pMemViewer,
                       const ConditionExpression& pCondExp);

bool addTriggerSentencesAnswer(SemControllerWorkingStruct& pWorkStruct,
                               bool& pAnAnswerHasBeenAdded,
                               SemanticMemoryBlockViewer& pMemViewer,
                               const RequestLinks& pReqLinks,
                               SemanticExpressionCategory pExpCategory,
                               const SemanticTriggerAxiomId& pAxiomId,
                               const GroundedExpression& pInputGrdExp);

void getInformationsLinkedToCondition(std::set<const SemanticMemorySentence*>& pNewInformations,
                                      SemControllerWorkingStruct& pWorkStruct,
                                      SemanticMemoryBlockViewer& pMemViewer,
                                      const RequestLinks& pReqLinks);

void getNowConditions(SemControllerWorkingStruct& pWorkStruct,
                      SemanticMemoryBlockViewer& pMemViewer,
                      const SemanticDuration& pNowTimeDuration,
                      const linguistics::LinguisticDatabase& pLingDb);

const SemanticExpression* getActionComposition(SemControllerWorkingStruct& pWorkStruct,
                                               SemanticMemoryBlockViewer& pMemViewer,
                                               const GroundedExpression& pGrdExp);

bool checkForConditionsLinkedToStatement(SemControllerWorkingStruct& pWorkStruct,
                                         SemanticMemoryBlockViewer& pMemViewer,
                                         const RequestLinks& pReqLinks,
                                         const GroundedExpression& pGrdExp);

void disableActionsLinkedToASentence(SemanticMemory& pSemanticMemory,
                                     const GroundedExpression& pGrdExp,
                                     const linguistics::LinguisticDatabase& pLingDb);

bool satisfyAnAction(SemControllerWorkingStruct& pWorkStruct,
                     SemanticMemoryBlockViewer& pMemViewer,
                     const GroundedExpression& pGrdExp,
                     const SemanticStatementGrounding& pGrdExpStatement);

void checkNominalGrdExp(SemControllerWorkingStruct& pWorkStruct,
                        const GroundedExpression& pGrdExp);

bool satisfyAQuestion(SemControllerWorkingStruct& pWorkStruct,
                      SemanticMemoryBlockViewer& pMemViewer,
                      const GroundedExpression& pGrdExp,
                      const std::list<const GroundedExpression*>& pOtherGrdExps,
                      const GroundedExpression& pOriginalGrdExp,
                      const SemanticRequests& pRequests);

void getSentsThatContain(RelationsThatMatch<false>& pRes,
                         const SemanticExpression& pNominalGroup,
                         const SemanticMemoryBlock& pMemBlock,
                         const linguistics::LinguisticDatabase& pLingDb);


} // End of namespace semanticMemoryLinker
} // End of namespace onsem


#endif // ONSEM_SEMANTICTOTEXT_SRC_CONTROLLER_STEPS_SEMANTICMEMORYLINKER_HPP
