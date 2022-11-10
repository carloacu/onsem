#ifndef ONSEM_SEMANTICTOTEXT_SEMANTICMEMORY_SEMANTICMEMORYBLOC_HPP
#define ONSEM_SEMANTICTOTEXT_SEMANTICMEMORY_SEMANTICMEMORYBLOC_HPP

#include <map>
#include <list>
#include <memory>
#include <set>
#include "../api.hpp"
#include <onsem/common/enum/infomationtype.hpp>
#include <onsem/common/enum/semanticlanguagetype.hpp>
#include <onsem/common/enum/semanticrequesttype.hpp>
#include <onsem/common/enum/semanticverbtense.hpp>
#include <onsem/common/enum/semanticgendertype.hpp>
#include <onsem/common/binary/binarymasks.hpp>
#include <onsem/common/utility/radix_map.hpp>
#include <onsem/common/utility/observable/observable.hpp>
#include <onsem/common/utility/observable/observableunsafe.hpp>
#include <onsem/common/utility/unique_propagate_const.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticdurationgrounding.hpp>
#include <onsem/texttosemantic/dbtype/misc/truenessvalue.hpp>
#include <onsem/semantictotext/enum/semanticexpressioncategory.hpp>
#include <onsem/semantictotext/semanticmemory/groundedexpwithlinksid.hpp>


namespace onsem
{
namespace linguistics
{
struct LinguisticDatabase;
}
struct SemLineToPrint;
struct RelatedContextAxiom;
struct SemanticMemoryBlockPrivate;
struct SemanticBehaviorDefinition;
struct ExpressionWithLinks;
struct SentenceWithLinks;
struct UniqueSemanticExpression;
class SemanticTracker;
struct SemanticExpression;
struct GroundedExpression;
struct GroundedExpWithLinks;
struct GroundedExpressionContainer;



struct ONSEMSEMANTICTOTEXT_API SemanticMemoryBlock
{
  SemanticMemoryBlock(std::size_t pMaxNbOfKnowledgesInAMemoryBloc = 200);
  ~SemanticMemoryBlock();

  SemanticMemoryBlock(SemanticMemoryBlock&& pOther) = delete;
  SemanticMemoryBlock& operator=(SemanticMemoryBlock&& pOther) = delete;

  SemanticMemoryBlock(const SemanticMemoryBlock&) = delete;
  SemanticMemoryBlock& operator=(const SemanticMemoryBlock&) = delete;

  static const std::size_t infinteMemory;

  void ensureFallbacksBlock();

  void addExpHandleInMemory(const std::shared_ptr<ExpressionWithLinks>& pNewSemMemKnowledge,
                            const linguistics::LinguisticDatabase& pLingDb,
                            std::map<const SentenceWithLinks*, TruenessValue>* pAxiomToConditionCurrentStatePtr);
  std::shared_ptr<ExpressionWithLinks> addRootSemExp(UniqueSemanticExpression pNewRootSemExp,
                                                          const linguistics::LinguisticDatabase& pLingDb,
                                                          const mystd::radix_map_str<std::string>* pLinkedInfosPtr = nullptr,
                                                          std::map<const SentenceWithLinks*, TruenessValue>* pAxiomToConditionCurrentStatePtr = nullptr);
  void addTrackerSemExp(UniqueSemanticExpression pNewRootSemExp,
                        std::shared_ptr<SemanticTracker>& pSemTracker,
                        const linguistics::LinguisticDatabase& pLingDb);
  void removeTrackerSemExp(std::shared_ptr<SemanticTracker>& pSemTracker,
                           const linguistics::LinguisticDatabase& pLingDb,
                           std::map<const SentenceWithLinks*, TruenessValue>* pAxiomToConditionCurrentStatePtr);

  void addRootSemExpWithHisContent(InformationType pInformationType,
                                   UniqueSemanticExpression pNewRootSemExp,
                                   const linguistics::LinguisticDatabase& pLingDb);
  void removeExpression(ExpressionWithLinks& pExpressionToRemove,
                        const linguistics::LinguisticDatabase& pLingDb,
                        std::map<const SentenceWithLinks*, TruenessValue>* pAxiomToConditionCurrentStatePtr);


  const std::list<std::shared_ptr<ExpressionWithLinks>>& getExpressionHandleInMemories() const;
  std::list<std::shared_ptr<ExpressionWithLinks>>& getExpressionHandleInMemories();

  bool empty() const;
  void clearLocalInformationButNotTheSubBloc();
  void clear();
  void clearKnowledgeByKnowledgeOnlyForTests(const linguistics::LinguisticDatabase& pLingDb,
                                             std::map<const SentenceWithLinks*, TruenessValue>* pAxiomToConditionCurrentStatePtr);
  void removeLinkedActions();

  std::size_t nbOfKnowledges() const;
  void copySemExps(std::list<UniqueSemanticExpression>& pCopiedSemExps) const;
  const SemanticExpression* getLastSemExpFromContext() const;
  const SemanticExpression* getBeforeLastSemExpOfAnAuthor(const SemanticExpression& pAuthor,
                                                          const linguistics::LinguisticDatabase& pLingDb) const;
  const SemanticExpression* getBeforeLastSemExpOfNotAnAuthor(const SemanticExpression& pAuthor,
                                                             const linguistics::LinguisticDatabase& pLingDb) const;

  void print(std::list<SemLineToPrint>& pLines,
             const mystd::optional<std::size_t>& pMaxNbOfSemanticExpressionsToPrint) const;

  SemanticGenderType getGender(const std::string& pUser) const;
  std::string getName(const std::string& pUser) const;
  std::unique_ptr<GroundedExpressionContainer> getEquivalentGrdExpPtr(const std::string& pUserId,
                                                                      const linguistics::LinguisticDatabase& pLingDb) const;
  void getAllEquivalentUserIds(std::list<std::string>& pRes,
                               const std::string& pUser) const;
  std::string getUserId(const std::vector<std::string>& pNames) const;
  std::string getUserIdFromGrdExp(const GroundedExpression& pGrdExp,
                                  const linguistics::LinguisticDatabase& pLingDb) const;
  bool areSameUser(const std::string& pUserId1,
                   const std::string& pUserId2,
                   RelatedContextAxiom& pRelContextAxiom);
  bool areSameUserConst(const std::string& pUserId1,
                        const std::string& pUserId2,
                        RelatedContextAxiom* pRelContextAxiomPtr = nullptr) const;
  bool isItMe(const std::string& pUserId) const;
  static std::unique_ptr<SemanticAgentGrounding> generateNewAgentGrd(const std::string& pName,
                                                                     SemanticLanguageEnum pLanguage,
                                                                     const linguistics::LinguisticDatabase& pLingDb);

  std::list<const SemanticExpression*> getSemExps() const;
  intSemId getNextSemId();

  std::size_t maxNbOfExpressionsInAMemoryBlock;


  SemanticMemoryBlock* getFallbacksBlockPtr() { return _fallbacksBlockPtr ? &*_fallbacksBlockPtr : nullptr; }
  const SemanticMemoryBlock* getFallbacksBlockPtr() const { return _fallbacksBlockPtr ? &*_fallbacksBlockPtr : nullptr; }

  mystd::observable::Observable<void(const ExpressionWithLinks&)> expressionThatWillBeRemoved;
  mystd::observable::ObservableUnsafe<void(const SemanticExpression&)> semExpAdded;
  mystd::observable::ObservableUnsafe<void(const SemanticExpression&)> expressionRemoved;

  // When a behavior is learnt (ex: "To salute is to say hi")
  mystd::observable::ObservableUnsafe<void(intSemId, const GroundedExpWithLinks*)> infActionAdded;
  mystd::observable::ObservableUnsafe<void(const std::map<intSemId, const GroundedExpWithLinks*>&)> infActionChanged;

  mystd::observable::ObservableUnsafe<void(const std::set<const GroundedExpWithLinks*>&)> conditionToActionChanged;
  mystd::observable::ObservableUnsafe<void(const GroundedExpression&, std::list<const GroundedExpression*>)> grdExpReplacedGrdExps;

  const std::map<intSemId, const GroundedExpWithLinks*>& getInfActions() const;
  const std::set<const GroundedExpWithLinks*>& getConditionToActions() const;

  static SemanticBehaviorDefinition extractActionFromMemorySentence(const GroundedExpWithLinks& pMemorySentence);
  static void extractActions(std::list<SemanticBehaviorDefinition>& pActionDefinitions,
                             const std::map<intSemId, const GroundedExpWithLinks*>& pInfActions);
  static void extractConditionToActions(
      std::list<UniqueSemanticExpression>& pConditionToActionsSemExp,
      const std::set<const GroundedExpWithLinks*>& pConditionToActionsMemSent);

  void writeInBinaryFile(const linguistics::LinguisticDatabase& pLingDb,
                         const std::string& pFilename,
                         const std::size_t pAllocSize) const;
  void loadBinaryFile(const std::string& pPath);
  void addASetOfEquivalentNames(const std::vector<std::string>& pNames,
                                SemanticLanguageEnum pLanguage,
                                const linguistics::LinguisticDatabase& pLingDb);

  const SemanticMemoryBlock* subBlockPtr;
  mystd::observable::ObservableUnsafe<void (const SemanticDuration&)> semanticTimesWithConditionsLinked;
  mystd::observable::ObservableUnsafe<void (UniqueSemanticExpression&)> actionProposalSignal;
  bool disableOldContrarySentences;

private:
  friend struct SemanticMemoryBlockViewer;
  friend struct SemanticMemoryBlockPrivate;

  /// Map of a userId directly generated from a name to the hard coded userId to consider;
  mystd::radix_map_str<std::string> _hardCodedUserIdResolution;

  /// Emitted if responses are programmed for incoming knowledge in the memory.
  mystd::unique_propagate_const<SemanticMemoryBlock> _fallbacksBlockPtr;
  mystd::unique_propagate_const<SemanticMemoryBlockPrivate> _impl;

  std::string _generateNewUserId(const std::string& pRootNewUserIdNameCandidate) const;

  /// Prune the oldest knowledge if the memory size limit has exceeded.
  void _pruneExceedingExpressions(const linguistics::LinguisticDatabase& pLingDb,
                                  std::map<const SentenceWithLinks*, TruenessValue>* pAxiomToConditionCurrentStatePtr);

  /// Factorization of the knowledge addition algorithm. Does not prune memory if size exceeded.
  std::shared_ptr<ExpressionWithLinks> _addRootSemExp(UniqueSemanticExpression pNewRootSemExp,
                                                             const mystd::radix_map_str<std::string>* pLinkedInfosPtr);
  bool _userIdToHardCodedUserId(std::string& pHardCodedUserId,
                                const std::string& pUserId) const;
  void _nameToUserIds(std::set<std::string>& pUserIds,
                      const std::string& pName) const;
  bool _doesUserIdExist(const std::string& pUserId) const;

  void _writeInBinary(binarymasks::Ptr& pPtr,
                      const linguistics::LinguisticDatabase& pLingDb) const;

private:
  friend struct GroundedExpWithLinks;
  friend struct GroundedExpWithLinksPrivate;
};


} // End of namespace onsem


#endif // ONSEM_SEMANTICTOTEXT_SEMANTICMEMORY_SEMANTICMEMORYBLOC_HPP
