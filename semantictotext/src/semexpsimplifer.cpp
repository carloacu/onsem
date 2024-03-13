#include <onsem/semantictotext/semexpsimplifer.hpp>
#include <onsem/texttosemantic/tool/semexpmodifier.hpp>
#include <onsem/semantictotext/tool/semexpcomparator.hpp>
#include <onsem/common/enum/listexpressiontype.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpressions.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticstatementgrounding.hpp>
#include <onsem/texttosemantic/tool/semexpgetter.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemory.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemoryblock.hpp>

namespace onsem {
namespace simplifier {

void _getChildrenThatDiffer(std::list<std::map<GrammaticalType, UniqueSemanticExpression>::iterator>& pDiffChidren,
                            std::map<GrammaticalType, UniqueSemanticExpression>& pChildren1,
                            const std::map<GrammaticalType, UniqueSemanticExpression>& pChildren2,
                            const SemanticMemoryBlock& pMemBlock,
                            const linguistics::LinguisticDatabase& pLingDb) {
    for (auto itChild1 = pChildren1.begin(); itChild1 != pChildren1.end(); ++itChild1) {
        auto itChild2 = pChildren2.find(itChild1->first);
        if (itChild2 == pChildren2.end()) {
            pDiffChidren.push_back(itChild1);
            continue;
        }

        SemExpComparator::ComparisonExceptions compExceptions;
        SemExpGetter::getStatementSubordinates(compExceptions.semExps1ToSkip, *itChild1->second);
        SemExpGetter::getStatementSubordinates(compExceptions.semExps2ToSkip, *itChild2->second);
        if (!SemExpComparator::semExpsAreEqualFromMemBlock(
                *itChild1->second, *itChild2->second, pMemBlock, pLingDb, &compExceptions))
            pDiffChidren.push_back(itChild1);
    }
}

/**
 * Handle the case:
 *     A       +     A        =>     A          +      A     (but if this function succeed the second element will be
 * removed)
 *    / \           / \             / \               / \
 *  B    C         B   D           B  AND            B   D
 *                                    /  \
 *                                   C    D
 */
bool _tryToMerge2SemExpsWithOnlyOneChildThatDiffer(UniqueSemanticExpression& pSemExp1,
                                                   UniqueSemanticExpression& pSemExp2,
                                                   ListExpressionType pListType,
                                                   const SemanticMemoryBlock& pMemBlock,
                                                   const linguistics::LinguisticDatabase& pLingDb) {
    if (pSemExp1->type == SemanticExpressionType::GROUNDED && pSemExp2->type == SemanticExpressionType::GROUNDED) {
        GroundedExpression& grdExp1 = pSemExp1->getGrdExp();
        GroundedExpression& grdExp2 = pSemExp2->getGrdExp();
        if (SemExpComparator::groundingsAreEqual(grdExp1.grounding(), grdExp2.grounding(), pMemBlock, pLingDb)) {
            std::list<std::map<GrammaticalType, UniqueSemanticExpression>::iterator> diffChildren1;
            _getChildrenThatDiffer(diffChildren1, grdExp1.children, grdExp2.children, pMemBlock, pLingDb);
            if (diffChildren1.size() <= 1) {
                std::list<std::map<GrammaticalType, UniqueSemanticExpression>::iterator> diffChildren2;
                _getChildrenThatDiffer(diffChildren2, grdExp2.children, grdExp1.children, pMemBlock, pLingDb);
                if (diffChildren1.size() == 1 && diffChildren2.empty()) {
                    return true;
                } else if (diffChildren2.size() == 1 && diffChildren1.empty()) {
                    pSemExp1.swap(pSemExp2);
                    return true;
                } else if (diffChildren1.size() == 1 && diffChildren2.size() == 1) {
                    auto itDiffChild1 = diffChildren1.front();
                    auto itDiffChild2 = diffChildren2.front();
                    if (itDiffChild1->first == itDiffChild2->first) {
                        auto firstGrdExpPtr = itDiffChild1->second->getGrdExpPtr_SkipWrapperPtrs();
                        if (firstGrdExpPtr != nullptr
                            && firstGrdExpPtr->grounding().type == SemanticGroundingType::STATEMENT
                            && _tryToMerge2SemExpsWithOnlyOneChildThatDiffer(
                                itDiffChild1->second, itDiffChild2->second, pListType, pMemBlock, pLingDb))
                            return true;

                        SemExpModifier::addChild(
                            grdExp1, itDiffChild2->first, std::move(itDiffChild2->second), pListType);
                        return true;
                    }
                }
            }
        } else {
        }
    }
    return false;
}

bool _canMergeTwoVerbs(const SemanticExpression& pSemExp1,
                       const SemanticExpression& pSemExp2,
                       const SemanticMemoryBlock& pMemBlock,
                       const linguistics::LinguisticDatabase& pLingDb) {
    const GroundedExpression* grdExpPtr1 = pSemExp1.getGrdExpPtr_SkipWrapperPtrs();
    if (grdExpPtr1 == nullptr)
        return false;
    const GroundedExpression& grdExp1 = *grdExpPtr1;
    const GroundedExpression* grdExpPtr2 = pSemExp2.getGrdExpPtr_SkipWrapperPtrs();
    if (grdExpPtr2 == nullptr)
        return false;
    const GroundedExpression& grdExp2 = *grdExpPtr2;
    const SemanticStatementGrounding* statGrdPtr1 = grdExp1->getStatementGroundingPtr();
    const SemanticStatementGrounding* statGrdPtr2 = grdExp2->getStatementGroundingPtr();
    if (statGrdPtr1 != nullptr && statGrdPtr2 != nullptr && statGrdPtr1->verbTense == statGrdPtr2->verbTense
        && statGrdPtr1->verbGoal == statGrdPtr2->verbGoal && statGrdPtr1->verbGoal == VerbGoalEnum::ABILITY) {
        auto itSubject1 = grdExp1.children.find(GrammaticalType::SUBJECT);
        if (itSubject1 == grdExp1.children.end())
            return false;
        auto itSubject2 = grdExp2.children.find(GrammaticalType::SUBJECT);
        if (itSubject2 == grdExp2.children.end())
            return false;
        return SemExpComparator::semExpsAreEqualFromMemBlock(
            *itSubject1->second, *itSubject2->second, pMemBlock, pLingDb, nullptr);
    }
    return false;
}

ListExpressionType _getListType(const ListExpression& pListExp, bool pConvertUnrelatedListToAndList) {
    ListExpressionType listType = pListExp.listType;
    if (pConvertUnrelatedListToAndList && listType == ListExpressionType::UNRELATED)
        return ListExpressionType::AND;
    return listType;
}

/**
 * Handle the case:
 *         AND         =>          AND
 *      /       \                  /
 *     A         A                A
 *    / \       / \              / \
 *  B    C     B   D            B   AND
 *                                 /  \
 *                                C    D
 */
bool _tryToMergeListElements(ListExpression& pListExp,
                             const SemanticMemoryBlock& pMemBlock,
                             const linguistics::LinguisticDatabase& pLingDb,
                             bool pConvertUnrelatedListToAndList) {
    for (auto itFirstListElt = pListExp.elts.begin(); itFirstListElt != pListExp.elts.end(); ++itFirstListElt) {
        auto itSecondListElt = itFirstListElt;
        ++itSecondListElt;
        for (; itSecondListElt != pListExp.elts.end(); ++itSecondListElt) {
            ListExpressionType listType = _getListType(pListExp, pConvertUnrelatedListToAndList);
            if (_tryToMerge2SemExpsWithOnlyOneChildThatDiffer(
                    *itFirstListElt, *itSecondListElt, listType, pMemBlock, pLingDb)) {
                pListExp.elts.erase(itSecondListElt);
                return true;
            }
        }
    }
    return false;
}

bool _tryToMergeVerbs(ListExpression& pListExp,
                      const SemanticMemoryBlock& pMemBlock,
                      const linguistics::LinguisticDatabase& pLingDb,
                      bool pConvertUnrelatedListToAndList) {
    for (auto itFirstListElt = pListExp.elts.begin(); itFirstListElt != pListExp.elts.end(); ++itFirstListElt) {
        auto itSecondListElt = itFirstListElt;
        ++itSecondListElt;
        auto itEndOfListToMerge = pListExp.elts.end();
        for (; itSecondListElt != pListExp.elts.end(); ++itSecondListElt) {
            if (_canMergeTwoVerbs(**itFirstListElt, **itSecondListElt, pMemBlock, pLingDb)) {
                itEndOfListToMerge = itSecondListElt;
                continue;
            }
            break;
        }
        if (itEndOfListToMerge != pListExp.elts.end()) {
            auto resStatGrd = std::make_unique<SemanticStatementGrounding>();
            std::unique_ptr<GroundedExpression> res;

            bool firstIt = true;
            auto itCurrElt = itFirstListElt;
            while (itCurrElt != itSecondListElt) {
                GroundedExpression* currEltGrdExpPtr = (*itCurrElt)->getGrdExpPtr_SkipWrapperPtrs();
                if (currEltGrdExpPtr == nullptr)
                    return false;
                SemanticStatementGrounding* currStatGrdPtr = currEltGrdExpPtr->grounding().getStatementGroundingPtr();
                if (currStatGrdPtr == nullptr)
                    return false;
                SemanticStatementGrounding& currStatGrd = *currStatGrdPtr;
                auto itSubject = currEltGrdExpPtr->children.find(GrammaticalType::SUBJECT);
                if (itSubject == currEltGrdExpPtr->children.end())
                    return false;
                if (firstIt) {
                    resStatGrd->verbTense = currStatGrd.verbTense;
                    resStatGrd->verbGoal = currStatGrd.verbGoal;
                    resStatGrd->requests = currStatGrd.requests;
                    res = std::make_unique<GroundedExpression>(std::move(resStatGrd));
                    res->children.emplace(GrammaticalType::SUBJECT, std::move(itSubject->second));
                }
                currEltGrdExpPtr->children.erase(itSubject);
                currStatGrd.verbTense = SemanticVerbTense::UNKNOWN;
                currStatGrd.verbGoal = VerbGoalEnum::NOTIFICATION;
                currStatGrd.requests.clear();
                ListExpressionType listType = _getListType(pListExp, pConvertUnrelatedListToAndList);
                SemExpModifier::addChild(*res, GrammaticalType::OBJECT, std::move(*itCurrElt), listType);
                if (firstIt) {
                    ++itCurrElt;
                    firstIt = false;
                } else {
                    itCurrElt = pListExp.elts.erase(itCurrElt);
                }
            }
            *itFirstListElt = std::move(res);
        }
    }
    return false;
}

/**
 * Handle the case:
 *         AND         =>      A
 *      /       \             / \
 *     A         A           B   AND
 *    / \       / \             /  \
 *  B    C     B   D           C    D
 *
 */
void processFromMemBlock(UniqueSemanticExpression& pSemExp,
                         const SemanticMemoryBlock& pMemBlock,
                         const linguistics::LinguisticDatabase& pLingDb,
                         bool pConvertUnrelatedListToAndList) {
    switch (pSemExp->type) {
        case SemanticExpressionType::LIST: {
            SemExpModifier::removeEmptyListElements(pSemExp);

            ListExpression* listExpPtr = pSemExp->getListExpPtr();
            if (listExpPtr != nullptr) {
                ListExpression& listExp = *listExpPtr;
                for (auto& currElt : listExp.elts)
                    processFromMemBlock(currElt, pMemBlock, pLingDb, pConvertUnrelatedListToAndList);
                while (_tryToMergeListElements(listExp, pMemBlock, pLingDb, pConvertUnrelatedListToAndList))
                    continue;
                while (_tryToMergeVerbs(listExp, pMemBlock, pLingDb, pConvertUnrelatedListToAndList))
                    continue;
                if (listExp.elts.size() == 1)
                    pSemExp = std::move(listExp.elts.front());
            }
            break;
        }
        case SemanticExpressionType::CONDITION: {
            ConditionExpression& condExp = pSemExp->getCondExp();
            processFromMemBlock(condExp.conditionExp, pMemBlock, pLingDb);
            processFromMemBlock(condExp.thenExp, pMemBlock, pLingDb);
            if (condExp.elseExp)
                processFromMemBlock(*condExp.elseExp, pMemBlock, pLingDb);
            break;
        }
        case SemanticExpressionType::GROUNDED: {
            GroundedExpression& grdExp = pSemExp->getGrdExp();
            for (auto& currChild : grdExp.children)
                processFromMemBlock(currChild.second, pMemBlock, pLingDb);
            break;
        }
        case SemanticExpressionType::INTERPRETATION: {
            InterpretationExpression& intExp = pSemExp->getIntExp();
            processFromMemBlock(intExp.interpretedExp, pMemBlock, pLingDb);
            break;
        }
        case SemanticExpressionType::FEEDBACK: {
            FeedbackExpression& fdkExp = pSemExp->getFdkExp();
            processFromMemBlock(fdkExp.concernedExp, pMemBlock, pLingDb);
            break;
        }
        case SemanticExpressionType::ANNOTATED: {
            AnnotatedExpression& annExp = pSemExp->getAnnExp();
            processFromMemBlock(annExp.semExp, pMemBlock, pLingDb);
            break;
        }
        case SemanticExpressionType::METADATA: {
            MetadataExpression& metadataExp = pSemExp->getMetadataExp();
            processFromMemBlock(metadataExp.semExp, pMemBlock, pLingDb);
            break;
        }
        case SemanticExpressionType::FIXEDSYNTHESIS:
        case SemanticExpressionType::COMMAND:
        case SemanticExpressionType::COMPARISON:
        case SemanticExpressionType::SETOFFORMS: break;
    }
}

void process(UniqueSemanticExpression& pSemExp,
             const SemanticMemory& pSemanticMemory,
             const linguistics::LinguisticDatabase& pLingDb,
             bool pConvertUnrelatedListToAndList) {
    processFromMemBlock(pSemExp, pSemanticMemory.memBloc, pLingDb, pConvertUnrelatedListToAndList);
}

void solveConditionsInplace(UniqueSemanticExpression& pSemExp,
                            const SemanticMemoryBlock& pMemBlock,
                            const linguistics::LinguisticDatabase& pLingDb) {
    std::list<ConditionSolvedResult<UniqueSemanticExpression>> solvedConditons;
    solveConditions(solvedConditons, pSemExp, pMemBlock, pLingDb);
    if (solvedConditons.empty())
        return;
    for (const auto& currSolvedCondition : solvedConditons) {
        ConditionExpression& condExp = currSolvedCondition.rootSemExp->getCondExp();
        if (currSolvedCondition.truenessValue == TruenessValue::VAL_TRUE) {
            currSolvedCondition.rootSemExp = std::move(condExp.thenExp);
            solveConditionsInplace(currSolvedCondition.rootSemExp, pMemBlock, pLingDb);
        } else if (currSolvedCondition.truenessValue == TruenessValue::VAL_FALSE) {
            if (condExp.elseExp) {
                currSolvedCondition.rootSemExp = std::move(*condExp.elseExp);
                solveConditionsInplace(currSolvedCondition.rootSemExp, pMemBlock, pLingDb);
            } else
                currSolvedCondition.rootSemExp.clear();
        }
    }
    SemExpModifier::removeEmptyListElements(pSemExp);
}

}    // End of namespace simplifier
}    // End of namespace onsem
