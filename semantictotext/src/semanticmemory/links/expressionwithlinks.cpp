#include <onsem/semantictotext/semanticmemory/links/expressionwithlinks.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpressions.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticstatementgrounding.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/conceptset.hpp>
#include <onsem/texttosemantic/tool/semexpgetter.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemoryblock.hpp>
#include "../../controller/semexpcontroller.hpp"

namespace onsem {
namespace {

struct GrdExpTriggerComposition {
    ListExpressionType listType = ListExpressionType::UNRELATED;
    std::list<const GroundedExpression*> grdExpPtrs{};
};

void _getGrdExpPtrs_SkipWrapperListsBySetOfFroms(std::list<GrdExpTriggerComposition>& pTriggerSetOfForms,
                                                 std::list<const ConditionExpression*>& pCondExpTriggers,
                                                 const SemanticExpression& pSemExp) {
    const bool followInterpretations = false;    // Because it is a trigger
    const SetOfFormsExpression* setOfFormsPtr = pSemExp.getSetOfFormsPtr_SkipWrapperPtrs(followInterpretations);
    if (setOfFormsPtr != nullptr) {
        for (auto& currForm : setOfFormsPtr->prioToForms) {
            for (auto& currQuestionForm : currForm.second) {
                pTriggerSetOfForms.emplace_back();
                auto& triggerComp = pTriggerSetOfForms.back();
                triggerComp.listType = currQuestionForm->exp->getGrdExpPtrs_SkipWrapperLists(triggerComp.grdExpPtrs,
                                                                                             followInterpretations);
            }
        }
        return;
    }

    const ConditionExpression* condExpPtr = pSemExp.getCondExpPtr_SkipWrapperPtrs(followInterpretations);
    if (condExpPtr != nullptr) {
        pCondExpTriggers.emplace_back(condExpPtr);
    } else {
        pTriggerSetOfForms.emplace_back();
        auto& grdTriggerComp = pTriggerSetOfForms.back();
        grdTriggerComp.listType =
            pSemExp.getGrdExpPtrs_SkipWrapperLists(grdTriggerComp.grdExpPtrs, followInterpretations, true);
        if (grdTriggerComp.grdExpPtrs.empty())
            pTriggerSetOfForms.pop_back();
    }
}

struct GrdExpLinksContext {
    GrdExpLinksContext(const GroundedExpression& pHoldingSentence,
                       const SemanticStatementGrounding& pHoldingStatementGrd,
                       GrammaticalType pParentGrammaticalType,
                       const SemanticExpression& pParentSemExp)
        : holdingSentence(pHoldingSentence)
        , holdingStatementGrd(pHoldingStatementGrd)
        , parentGrammaticalType(pParentGrammaticalType)
        , parentSemExp(pParentSemExp) {}
    const GroundedExpression& holdingSentence;
    const SemanticStatementGrounding& holdingStatementGrd;
    GrammaticalType parentGrammaticalType;
    const SemanticExpression& parentSemExp;
};

const SemanticExpression& _resolveParentCoreferenceLink(const GrdExpLinksContext& pContext) {
    auto* parentGrdExpPtr = pContext.parentSemExp.getGrdExpPtr_SkipWrapperPtrs();
    if (parentGrdExpPtr == nullptr)
        return pContext.parentSemExp;
    auto& parentGrdExp = *parentGrdExpPtr;
    if (ConceptSet::haveAConcept(pContext.holdingStatementGrd.concepts, "verb_equal_be")
        && SemExpGetter::getReferenceTypeFromGrd(*parentGrdExp) != SemanticReferenceType::DEFINITE) {
        auto otherGrammaticalType = SemExpGetter::invertGrammaticalType(pContext.parentGrammaticalType);
        if (otherGrammaticalType != pContext.parentGrammaticalType) {
            auto itOtherChild = pContext.holdingSentence.children.find(otherGrammaticalType);
            if (itOtherChild != pContext.holdingSentence.children.end()) {
                auto* otherChildGrdExpPtr = itOtherChild->second->getGrdExpPtr_SkipWrapperPtrs();
                if (otherChildGrdExpPtr != nullptr
                    && SemExpGetter::getReferenceTypeFromGrd(otherChildGrdExpPtr->grounding())
                           == SemanticReferenceType::DEFINITE)
                    return *otherChildGrdExpPtr;
            }
        }
    }
    return pContext.parentSemExp;
}

void _addGrdExpToAxiom(SentenceWithLinks& pContextAxiom,
                       const GroundedExpression& pGrdExpToAdd,
                       const std::map<GrammaticalType, const SemanticExpression*>& pAnnotations,
                       bool pIsAConditionToSatisfy,
                       const linguistics::LinguisticDatabase& pLingDb,
                       bool pIsATrigger);

void _addSpecifierGrdExp(SentenceWithLinks& pContextAxiom,
                         const GrdExpLinksContext& pContext,
                         const GroundedExpression& pGrdExp,
                         const linguistics::LinguisticDatabase& pLingDb,
                         bool pIsATrigger) {
    std::map<GrammaticalType, const SemanticExpression*> subAnnotations;
    auto itSubject = pGrdExp.children.find(GrammaticalType::SUBJECT);
    if (itSubject != pGrdExp.children.end()
        && SemExpGetter::isACoreference(*itSubject->second, CoreferenceDirectionEnum::PARENT, false)) {
        subAnnotations.emplace(GrammaticalType::SUBJECT, &_resolveParentCoreferenceLink(pContext));
    } else {
        auto itObject = pGrdExp.children.find(GrammaticalType::OBJECT);
        if (itObject != pGrdExp.children.end()
            && SemExpGetter::isACoreference(*itObject->second, CoreferenceDirectionEnum::PARENT, false))
            subAnnotations.emplace(GrammaticalType::OBJECT, &_resolveParentCoreferenceLink(pContext));
    }
    _addGrdExpToAxiom(pContextAxiom, pGrdExp, subAnnotations, false, pLingDb, pIsATrigger);
}

void _addGrdExpToAxiom(SentenceWithLinks& pContextAxiom,
                       const GroundedExpression& pGrdExpToAdd,
                       const std::map<GrammaticalType, const SemanticExpression*>& pAnnotations,
                       bool pIsAConditionToSatisfy,
                       const linguistics::LinguisticDatabase& pLingDb,
                       bool pIsATrigger) {
    pContextAxiom.memorySentences.elts.emplace_back(
        pContextAxiom, pGrdExpToAdd, false, pAnnotations, pIsATrigger, pLingDb, pIsAConditionToSatisfy);

    if (pGrdExpToAdd->type == SemanticGroundingType::STATEMENT) {
        const auto& statGrd = pGrdExpToAdd->getStatementGrounding();
        for (const auto& currChild : pGrdExpToAdd.children) {
            const SemanticExpression& childSemExp = *currChild.second;
            const GroundedExpression* childGrdExpPtr = childSemExp.getGrdExpPtr_SkipWrapperPtrs();
            if (childGrdExpPtr == nullptr)
                continue;
            auto& childGrdExp = *childGrdExpPtr;
            const auto* childStatGrdPtr = childGrdExp->getStatementGroundingPtr();
            if (childStatGrdPtr != nullptr) {
                if (currChild.first == GrammaticalType::OBJECT) {
                    auto itSubject = pGrdExpToAdd.children.find(GrammaticalType::SUBJECT);
                    if (itSubject != pGrdExpToAdd.children.end()) {
                        auto itSubjectOfChild = childGrdExp.children.find(GrammaticalType::SUBJECT);
                        if (itSubjectOfChild != childGrdExp.children.end()
                            && SemExpGetter::isACoreference(*itSubjectOfChild->second,
                                                            CoreferenceDirectionEnum::PARENT)) {
                            GrdExpLinksContext context(
                                childGrdExp, statGrd, GrammaticalType::SUBJECT, *itSubject->second);
                            _addSpecifierGrdExp(pContextAxiom, context, childGrdExp, pLingDb, pIsATrigger);
                        } else {
                            auto itObjectOfChild = childGrdExp.children.find(GrammaticalType::OBJECT);
                            if (itObjectOfChild != childGrdExp.children.end()
                                && SemExpGetter::isACoreference(*itObjectOfChild->second,
                                                                CoreferenceDirectionEnum::PARENT)) {
                                GrdExpLinksContext context(
                                    childGrdExp, statGrd, GrammaticalType::SUBJECT, *itSubject->second);
                                _addSpecifierGrdExp(pContextAxiom, context, childGrdExp, pLingDb, pIsATrigger);
                            }
                        }
                    }
                }
            } else {
                auto itSpecifier = childGrdExp.children.find(GrammaticalType::SPECIFIER);
                if (itSpecifier != childGrdExp.children.end()) {
                    std::list<const GroundedExpression*> specGrdExps;
                    itSpecifier->second->getGrdExpPtrs_SkipWrapperLists(specGrdExps);
                    for (auto& currGrdExpPtr : specGrdExps) {
                        if (currGrdExpPtr != nullptr
                            && currGrdExpPtr->grounding().type == SemanticGroundingType::STATEMENT) {
                            GrdExpLinksContext context(pGrdExpToAdd, statGrd, currChild.first, *currChild.second);
                            _addSpecifierGrdExp(pContextAxiom, context, *currGrdExpPtr, pLingDb, pIsATrigger);
                        }
                    }
                }
            }
        }
    }
}

}

ExpressionWithLinks::ExpressionWithLinks(SemanticMemoryBlock& pParentMemBloc,
                                         UniqueSemanticExpression pSemExp,
                                         const mystd::radix_map_str<std::string>* pLinkedInfosPtr)
    : linkedInfos()
    , semExp(std::move(pSemExp))
    , contextAxioms()
    , outputToAnswerIfTriggerHasMatched()
    , _parentMemBloc(pParentMemBloc) {
    if (pLinkedInfosPtr != nullptr)
        linkedInfos = *pLinkedInfosPtr;
}

void ExpressionWithLinks::clearWrappings(
    SemanticMemoryBlock& pMemBlock,
    const linguistics::LinguisticDatabase& pLingDb,
    std::map<const SentenceWithLinks*, TruenessValue>* pAxiomToConditionCurrentStatePtr) {
    linkedInfos.clear();
    outputToAnswerIfTriggerHasMatched.reset();
    auto itContAxiom = contextAxioms.begin();
    while (itContAxiom != contextAxioms.end()) {
        itContAxiom->setEnabled(false);
        controller::uninform(*itContAxiom, pMemBlock, pLingDb, pAxiomToConditionCurrentStatePtr);
        itContAxiom->clear();
        itContAxiom = contextAxioms.erase(itContAxiom);
    }
}

void ExpressionWithLinks::removeContextAxiomsWithAnActionLinked() {
    for (auto itContAxiom = contextAxioms.begin(); itContAxiom != contextAxioms.end();) {
        if (itContAxiom->informationType != InformationType::INFORMATION) {
            ++itContAxiom;
            continue;
        }

        if (itContAxiom->isAnActionLinked()) {
            itContAxiom->clear();
            itContAxiom = contextAxioms.erase(itContAxiom);
        } else {
            ++itContAxiom;
        }
    }
}

void ExpressionWithLinks::addConditionToAnAction(InformationType pInformationType,
                                                 const ConditionSpecification& pCondExp,
                                                 const linguistics::LinguisticDatabase& pLingDb) {
    addAxiomListToMemory(pCondExp.conditionExp,
                         nullptr,
                         pInformationType,
                         pCondExp.isAlwaysActive,
                         &pCondExp.thenExp,
                         pCondExp.elseExpPtr,
                         nullptr,
                         pLingDb,
                         false);
}

void ExpressionWithLinks::addConditionToAnInfo(InformationType pInformationType,
                                               const ConditionSpecification& pCondExp,
                                               const linguistics::LinguisticDatabase& pLingDb) {
    addAxiomListToMemory(
        pCondExp.thenExp, nullptr, pInformationType, false, nullptr, nullptr, &pCondExp.conditionExp, pLingDb, false);

    if (pCondExp.conditionShouldBeInformed)
        addAxiomListToMemory(
            pCondExp.conditionExp, nullptr, pInformationType, false, nullptr, nullptr, nullptr, pLingDb, false);
}

SentenceWithLinks* ExpressionWithLinks::addAxiomFromGrdExp(
    InformationType pInformationType,
    const GroundedExpression& pGrdSemExpToAdd,
    const std::map<GrammaticalType, const SemanticExpression*>& pAnnotations,
    const linguistics::LinguisticDatabase& pLingDb,
    bool pIsATrigger) {
    contextAxioms.emplace_back(pInformationType, *this);
    SentenceWithLinks& axiom = contextAxioms.back();

    std::list<const GroundedExpression*> subjectGrdPtrs;
    SemExpGetter::extractSubjectAndObjectOfAVerbDefinition(subjectGrdPtrs, axiom.infCommandToDo, pGrdSemExpToAdd);
    if (!subjectGrdPtrs.empty()) {
        assert(axiom.infCommandToDo != nullptr);
        for (const auto& currSubjectGrdPtr : subjectGrdPtrs)
            _addGrdExpToAxiom(axiom, *currSubjectGrdPtr, pAnnotations, true, pLingDb, pIsATrigger);
    } else {
        _addGrdExpToAxiom(axiom, pGrdSemExpToAdd, pAnnotations, false, pLingDb, pIsATrigger);
    }

    if (axiom.memorySentences.elts.empty()) {
        contextAxioms.pop_back();
        return nullptr;
    }
    return &contextAxioms.back();
}

SentenceWithLinks* ExpressionWithLinks::tryToAddTeachFormulation(
    InformationType pInformationType,
    const GroundedExpression& pGrdSemExpToAdd,
    const std::map<GrammaticalType, const SemanticExpression*>& pAnnotations,
    const linguistics::LinguisticDatabase& pLingDb,
    bool pIsATrigger) {
    const GroundedExpression* purposeGrdPtr = nullptr;
    const SemanticExpression* objectSemExpPtr = nullptr;
    SemExpGetter::extractTeachElements(purposeGrdPtr, objectSemExpPtr, pGrdSemExpToAdd);
    if (purposeGrdPtr != nullptr) {
        assert(objectSemExpPtr != nullptr);
        contextAxioms.emplace_back(pInformationType, *this);
        SentenceWithLinks& axiom = contextAxioms.back();
        axiom.infCommandToDo = objectSemExpPtr;
        _addGrdExpToAxiom(axiom, *purposeGrdPtr, pAnnotations, true, pLingDb, pIsATrigger);
        if (axiom.memorySentences.elts.empty()) {
            contextAxioms.pop_back();
            return nullptr;
        }
        return &contextAxioms.back();
    }
    return nullptr;
}

void ExpressionWithLinks::addAxiomForARecommendation(const GroundedExpression& pGrdSemExpToAdd,
                                                     const linguistics::LinguisticDatabase& pLingDb) {
    contextAxioms.emplace_back(InformationType::INFORMATION, *this);
    SentenceWithLinks& axiom = contextAxioms.back();
    std::map<GrammaticalType, const SemanticExpression*> annotations;
    axiom.memorySentences.elts.emplace_back(axiom, pGrdSemExpToAdd, true, annotations, true, pLingDb, false);
    if (axiom.memorySentences.elts.empty())
        contextAxioms.pop_back();
}

void ExpressionWithLinks::_addTriggerGrdExpsLinks(
    InformationType pInformationType,
    const std::list<const GroundedExpression*>& pTriggerGrdExpPtrs,
    const std::function<SemanticTriggerAxiomId(std::size_t)>& pGetAxiomIdFromId,
    const linguistics::LinguisticDatabase& pLingDb) {
    std::size_t i = 0;
    for (const auto& currGrdExpTriggerPtr : pTriggerGrdExpPtrs) {
        assert(currGrdExpTriggerPtr != nullptr);
        auto& currGrdExpTrigger = *currGrdExpTriggerPtr;
        contextAxioms.emplace_back(pInformationType, *this);
        SentenceWithLinks& axiom = contextAxioms.back();
        axiom.triggerAxiomId = pGetAxiomIdFromId(i);
        std::map<GrammaticalType, const SemanticExpression*> annotations;
        _addGrdExpToAxiom(axiom, currGrdExpTrigger, annotations, true, pLingDb, true);
        ++i;
    }
}

void ExpressionWithLinks::addTriggerLinks(InformationType pInformationType,
                                          const SemanticExpression& pSemExp,
                                          const linguistics::LinguisticDatabase& pLingDb) {
    std::list<GrdExpTriggerComposition> triggerSetOfForms;
    std::list<const ConditionExpression*> condExpTriggers;
    _getGrdExpPtrs_SkipWrapperListsBySetOfFroms(triggerSetOfForms, condExpTriggers, pSemExp);

    for (const auto& currTriggerComp : triggerSetOfForms) {
        std::size_t nbOfElts = currTriggerComp.grdExpPtrs.size();
        if (nbOfElts > 0) {
            ListExpressionType listExpType = currTriggerComp.listType;
            auto getAxiomIdFromId = [&nbOfElts, listExpType](std::size_t pId) {
                return SemanticTriggerAxiomId(nbOfElts, pId, listExpType);
            };
            _addTriggerGrdExpsLinks(pInformationType, currTriggerComp.grdExpPtrs, getAxiomIdFromId, pLingDb);
        }
    }

    for (const auto& currCondTrigger : condExpTriggers) {
        std::list<const GroundedExpression*> triggerGrdExpPtrs;
        currCondTrigger->toListOfGrdExpPtrs(triggerGrdExpPtrs);
        std::size_t nbOfElts = triggerGrdExpPtrs.size();
        auto getAxiomIdFromId = [&nbOfElts](std::size_t pId) {
            return SemanticTriggerAxiomId(nbOfElts, pId, SemanticExpressionType::CONDITION);
        };
        _addTriggerGrdExpsLinks(pInformationType, triggerGrdExpPtrs, getAxiomIdFromId, pLingDb);
    }
}

void ExpressionWithLinks::addAxiomListToMemory(const SemanticExpression& pSemExpToAdd,
                                               std::shared_ptr<SemanticTracker>* pSemTracker,
                                               InformationType pInformationType,
                                               bool pActionToDoIsAlwaysActive,
                                               const SemanticExpression* pActionToDo,
                                               const SemanticExpression* pActionToDoElse,
                                               const SemanticExpression* pSemExpToAddWithoutLinks,
                                               const linguistics::LinguisticDatabase& pLingDb,
                                               bool pIsATrigger) {
    const ListExpression* listExp = pSemExpToAdd.getListExpPtr();
    if (listExp != nullptr
        && (listExp->listType == ListExpressionType::UNRELATED
            || (listExp->listType == ListExpressionType::AND && pActionToDo == nullptr && pSemTracker == nullptr))) {
        for (const auto& currElt : listExp->elts)
            addAxiomListToMemory(*currElt,
                                 pSemTracker,
                                 pInformationType,
                                 pActionToDoIsAlwaysActive,
                                 pActionToDo,
                                 pActionToDoElse,
                                 pSemExpToAddWithoutLinks,
                                 pLingDb,
                                 pIsATrigger);
        return;
    }

    contextAxioms.emplace_back(pInformationType, *this);
    SentenceWithLinks& axiom = contextAxioms.back();
    if (pSemTracker != nullptr)
        axiom.semTracker = *pSemTracker;
    axiom.semExpToDoIsAlwaysActive = pActionToDoIsAlwaysActive;
    axiom.semExpToDo = pActionToDo;
    axiom.semExpToDoElse = pActionToDoElse;
    _addContextAxiom(axiom, pSemExpToAdd, pSemExpToAddWithoutLinks, pLingDb, pIsATrigger);
    if (axiom.memorySentences.elts.empty())
        contextAxioms.pop_back();
}

intSemId ExpressionWithLinks::getIdOfFirstSentence() const {
    for (const auto& currAxiom : contextAxioms)
        return currAxiom.memorySentences.getIdOfFirstSentence();
    return 0;
}

void ExpressionWithLinks::_addContextAxiom(SentenceWithLinks& pContextAxiom,
                                           const SemanticExpression& pSemExpToAdd,
                                           const SemanticExpression* pSemExpToAddWithoutLinks,
                                           const linguistics::LinguisticDatabase& pLingDb,
                                           bool pIsATrigger) {
    std::map<GrammaticalType, const SemanticExpression*> annotations;
    if (pSemExpToAddWithoutLinks != nullptr) {
        const GroundedExpression* grdExpToAddWithoutLinks = pSemExpToAddWithoutLinks->getGrdExpPtr_SkipWrapperPtrs();
        if (grdExpToAddWithoutLinks != nullptr) {
            pContextAxiom.memorySentences.and_or = true;
            _addGrdExpToAxiom(pContextAxiom, *grdExpToAddWithoutLinks, annotations, true, pLingDb, pIsATrigger);
        }
    }

    const GroundedExpression* grdExpToAddPtr = pSemExpToAdd.getGrdExpPtr_SkipWrapperPtrs();
    if (grdExpToAddPtr != nullptr) {
        const auto& grdExpToAdd = *grdExpToAddPtr;
        _addGrdExpToAxiom(pContextAxiom, grdExpToAdd, annotations, false, pLingDb, pIsATrigger);
        return;
    }

    const ListExpression* listExp = pSemExpToAdd.getListExpPtr_SkipWrapperPtrs();
    if (listExp != nullptr) {
        if (pSemExpToAddWithoutLinks != nullptr)
            return;

        pContextAxiom.memorySentences.and_or = listExp->listType != ListExpressionType::OR;
        for (const auto& currElt : listExp->elts) {
            const GroundedExpression* grdExpElt = currElt->getGrdExpPtr_SkipWrapperPtrs();
            if (grdExpElt != nullptr) {
                _addGrdExpToAxiom(pContextAxiom, *grdExpElt, annotations, false, pLingDb, pIsATrigger);
            }
        }
        return;
    }
}

}    // End of namespace onsem
