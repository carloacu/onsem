#ifndef ONSEM_SEMANTICTOTEXT_SRC_SEMANTICMEMORY_SEMANTICMEMORYBLOCKPRIVATE_HPP
#define ONSEM_SEMANTICTOTEXT_SRC_SEMANTICMEMORY_SEMANTICMEMORYBLOCKPRIVATE_HPP

#include <map>
#include <unordered_map>
#include <onsem/texttosemantic/dbtype/misc/truenessvalue.hpp>
#include <onsem/semantictotext/enum/semanticexpressioncategory.hpp>
#include "semanticlinkstogrdexps.hpp"
#include "sentenceslinks.hpp"

namespace onsem {
struct SemanticTriggerMemoryLinks {
    bool empty() const {
        return actionLinks.empty() && questionLinks.empty() && affirmationLinks.empty() && nominalGroupsLinks.empty()
            && recommendationsLinks.empty();
    }
    void clear() {
        actionLinks.clear();
        questionLinks.clear();
        affirmationLinks.clear();
        nominalGroupsLinks.clear();
        recommendationsLinks.clear();
    }

    /// The order that already have a defined answer
    SemanticMemoryLinksForAnyVerbGoal actionLinks;
    /// The question that already have a defined answer
    SemanticMemoryLinksForAnyVerbGoal questionLinks;
    /// The affirmations that already have a defined reaction
    SemanticMemoryLinksForAnyVerbGoal affirmationLinks;
    /// The nominal groups that already have a defined reaction
    SemanticLinksToGrdExps nominalGroupsLinks;
    /// The links to do recommendation matching
    SemanticLinksToGrdExps recommendationsLinks;
};
namespace linguistics {
struct LinguisticDatabase;
}
struct SemanticTriggerAxiomId;
struct ExpressionWithLinks;
class SemanticTracker;
struct SentenceWithLinks;
struct SemanticMemoryBlock;
struct SemanticMemoryBlockBinaryReader;

struct SemanticMemoryBlockPrivate {
    SemanticMemoryBlockPrivate(SemanticMemoryBlock& pMemBlock);

    bool empty() const;
    void clearLocalInformationButNotTheSubBloc();
    void clear();

    SemanticMemoryLinksForAnyVerbGoal& ensureSentenceTriggersLinks(SemanticExpressionCategory pSemExpCategory,
                                                                   const SemanticTriggerAxiomId& pAxiomId);
    SemanticMemoryLinksForAnyVerbGoal* getSentenceTriggersLinks(SemanticExpressionCategory pSemExpCategory,
                                                                const SemanticTriggerAxiomId& pAxiomId);
    const SemanticMemoryLinksForAnyVerbGoal* getSentenceTriggersLinks(SemanticExpressionCategory pSemExpCategory,
                                                                      const SemanticTriggerAxiomId& pAxiomId) const;

    SemanticLinksToGrdExps& ensureNominalGroupsTriggersLinks(const SemanticTriggerAxiomId& pAxiomId);
    SemanticLinksToGrdExps* getNominalGroupsTriggersLinks(const SemanticTriggerAxiomId& pAxiomId);
    const SemanticLinksToGrdExps* getNominalGroupsTriggersLinks(const SemanticTriggerAxiomId& pAxiomId) const;

    SemanticLinksToGrdExps& ensureRecommendationsTriggersLinks(const SemanticTriggerAxiomId& pAxiomId);
    SemanticLinksToGrdExps* getRecommendationsTriggersLinks(const SemanticTriggerAxiomId& pAxiomId);
    const SemanticLinksToGrdExps* getRecommendationsTriggersLinks(const SemanticTriggerAxiomId& pAxiomId) const;

    std::unique_ptr<SemanticNameGrounding> getNameGrd(const std::string& pUserId,
                                                      const SemanticMemoryGrdExp*& pSemMemoryGrdExpPtr) const;

    /// The expressions concerned in this memory bloc
    std::list<std::shared_ptr<ExpressionWithLinks>> expressionsMemories;

    SemanticMemoryBlock& getMemBlock() { return _memBlock; }
    const SemanticMemoryBlock& getMemBlock() const { return _memBlock; }

    /// Add last knowledge to knowledge pointer <-> iterator map.
    void addLastExpressionToPtrToIteratorMap();

    void addConditionToAnAction(const GroundedExpWithLinks& pMemSent,
                                const SemanticMemoryLinksForAMemSentence& pMemSentLinks);

    void addInfAction(const GroundedExpWithLinks& pMemSent);

    void removeInfAction(const GroundedExpWithLinks& pMemSent);
    void removeConditionToAnAction(const GroundedExpWithLinks& pMemSent);
    void removeLinksIfEmpty(const SemanticTriggerAxiomId& pAxiomId);

    RequestToMemoryLinks<true> getLinks(SemanticTypeOfLinks pTypeOfLinks,
                                        SemanticVerbTense pTense,
                                        VerbGoalEnum pVerbGoal);
    RequestToMemoryLinks<false> getLinks(SemanticTypeOfLinks pTypeOfLinks,
                                         SemanticVerbTense pTense,
                                         VerbGoalEnum pVerbGoal) const;

    std::map<std::shared_ptr<SemanticTracker>*, std::shared_ptr<ExpressionWithLinks>> trackerKnowledges;
    using MapMemoryPtrToIterator =
        std::unordered_map<ExpressionWithLinks*, std::list<std::shared_ptr<ExpressionWithLinks>>::iterator>;
    MapMemoryPtrToIterator expressionsMemoriesPtrToIt;

    /**
     * Link to sentences that be used to answer a question.
     * Sentences concerned:
     *  > all sentences except the cases:
     *     >> questions
     *     >> conditions
     *        >>> for sententence like "if A then B", A is not a potential answer so not included here but B yes
     *            (and if B match to answer a question we will check if A is true or not)
     *        >>> Sentences that are in a contextAxiom that have a momentNotifier or a semExpToDo
     */
    SemanticMemoryLinksForAnyVerbTense answersLinks;

    /**
     * "A" phrase of sentences like "if A then B"
     */
    SemanticMemoryLinksForAnyVerbTense conditionToInformationLinks;

    /**
     * Link to sentences that need to update their cached result.
     * Sentences concerned:
     *  > Sentences that are in a contextAxiom that have a momentNotifier or a semExpToDo
     */
    SemanticMemoryLinksForAnyVerbTense sentWithActionLinks;
    std::set<const GroundedExpWithLinks*> conditionToActions;

    /// Link to meanings of actions
    SemanticMemoryLinks sentWithInfActionLinks;
    std::map<intSemId, const GroundedExpWithLinks*> infActions;

    std::shared_ptr<SemanticMemoryBlockBinaryReader> subBinaryBlockPtr;

    /// Links to have information about the users gender, names, ...
    /// (it's for optimization because this informations are frequently used)
    SemanticUserCenteredMemoryLinksForMem userCenteredLinks;

    intSemId nextSemId;

    void removeExpression(MapMemoryPtrToIterator::iterator pItExpMem,
                          const linguistics::LinguisticDatabase& pLingDb,
                          std::map<const SentenceWithLinks*, TruenessValue>* pAxiomToConditionCurrentStatePtr);

    intMemBlockId getId() const { return _id; }

private:
    SemanticMemoryBlock& _memBlock;

    /// Triggers links
    std::map<SemanticTriggerAxiomId, SemanticTriggerMemoryLinks> _triggerLinks;

    intMemBlockId _id;
};

}    // End of namespace onsem

#endif    // ONSEM_SEMANTICTOTEXT_SRC_SEMANTICMEMORY_SEMANTICMEMORYBLOCKPRIVATE_HPP
