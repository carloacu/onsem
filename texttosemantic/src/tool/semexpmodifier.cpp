#include <onsem/texttosemantic/tool/semexpmodifier.hpp>
#include <optional>
#include <onsem/texttosemantic/dbtype/semanticgroundings.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpressions.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/conceptset.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase.hpp>
#include <onsem/texttosemantic/tool/semexpgetter.hpp>

namespace onsem {

namespace SemExpModifier {

namespace {

void _setRequest(GroundedExpression& pGrdExp, SemanticRequestType pRequestType) {
    auto& grd = pGrdExp.grounding();
    if (grd.type == SemanticGroundingType::STATEMENT)
        grd.getStatementGrounding().requests.set(pRequestType);
}

void _removeContextualAdvebs(UniqueSemanticExpression& pSemExp) {
    GroundedExpression* grdExpPtr = pSemExp->getGrdExpPtr_SkipWrapperPtrs();
    if (grdExpPtr != nullptr) {
        if (ConceptSet::haveAConcept(grdExpPtr->grounding().concepts, "also"))
            pSemExp.clear();
        return;
    }

    ListExpression* listExpPtr = pSemExp->getListExpPtr_SkipWrapperPtrs();
    if (listExpPtr != nullptr) {
        ListExpression& listExp = *listExpPtr;
        for (auto& currElt : listExp.elts)
            _removeContextualAdvebs(currElt);
        SemExpModifier::removeEmptyListElements(pSemExp);
    }
}

void _addAnythingChildFromGrdExp(GroundedExpression& pGrdExp, GrammaticalType pGrammaticalType) {
    if (pGrdExp->getStatementGroundingPtr() != nullptr)
        SemExpModifier::addChild(pGrdExp,
                                 pGrammaticalType,
                                 std::make_unique<GroundedExpression>(
                                     std::make_unique<SemanticConceptualGrounding>("stuff_informationToFill")));
}

}

void removeSpecificationsNotNecessaryForAnAnswer(GroundedExpression& pGrdExp) {
    auto& grd = *pGrdExp.getGrdExp();
    if (semanticGroundingsType_isRelativeType(grd.type))
        return;
    auto* agentGrdPtr = grd.getAgentGroundingPtr();
    if (agentGrdPtr != nullptr) {
        if (agentGrdPtr->isSpecificUser()) {
            pGrdExp.children.erase(GrammaticalType::SPECIFIER);
            return;
        }
    } else {
        auto* nameGrdPtr = grd.getNameGroundingPtr();
        if (nameGrdPtr != nullptr) {
            pGrdExp.children.erase(GrammaticalType::SPECIFIER);
            return;
        }
    }

    auto itSpec = pGrdExp.children.find(GrammaticalType::SPECIFIER);
    if (itSpec != pGrdExp.children.end()) {
        _removeContextualAdvebs(itSpec->second);
        if (itSpec->second->isEmpty())
            pGrdExp.children.erase(itSpec);
    }
}

void removeSpecificationsNotNecessaryForAnAnswerFromSemExp(SemanticExpression& pSemExp) {
    auto* grdExpPtr = pSemExp.getGrdExpPtr_SkipWrapperPtrs();
    if (grdExpPtr != nullptr)
        removeSpecificationsNotNecessaryForAnAnswer(*grdExpPtr);
}

void infGrdExpToMandatoryForm(GroundedExpression& pGrdExp) {
    auto* statGrdPtr = pGrdExp.grounding().getStatementGroundingPtr();
    if (statGrdPtr != nullptr) {
        auto& statGrd = *statGrdPtr;
        statGrd.verbTense = SemanticVerbTense::PRESENT;
        statGrd.verbGoal = VerbGoalEnum::MANDATORY;
        pGrdExp.children.emplace(
            GrammaticalType::SUBJECT,
            std::make_unique<GroundedExpression>(std::make_unique<SemanticConceptualGrounding>("generic")));
    }
}

std::list<GroundedExpression*> listTopGroundedExpressionsPtr(SemanticExpression& pSemExp) {
    auto grdExpPtr = pSemExp.getGrdExpPtr_SkipWrapperPtrs();
    if (grdExpPtr)
        return std::list<GroundedExpression*>{grdExpPtr};

    auto* lstExpPtr = pSemExp.getListExpPtr_SkipWrapperPtrs();
    if (lstExpPtr) {
        std::list<GroundedExpression*> result;
        for (auto& childSemExp : lstExpPtr->elts) {
            auto childGrdExpPtr = childSemExp->getGrdExpPtr_SkipWrapperPtrs();
            if (childGrdExpPtr)
                result.push_back(childGrdExpPtr);
        }
        return result;
    }

    return std::list<GroundedExpression*>{};
}

void setNumberFromSemExp(UniqueSemanticExpression& pSemExp, const SemanticFloat& pNumber) {
    auto* grdExpPtr = pSemExp->getGrdExpPtr_SkipWrapperPtrs();
    if (grdExpPtr != nullptr) {
        auto* genGrdPtr = grdExpPtr->grounding().getGenericGroundingPtr();
        if (genGrdPtr != nullptr) {
            if (pNumber == 1) {
                genGrdPtr->quantity.clear();
                auto res = std::make_unique<GroundedExpression>([] {
                    auto newNumberGenGrd = std::make_unique<SemanticGenericGrounding>();
                    newNumberGenGrd->quantity.setNumber(1);
                    newNumberGenGrd->entityType = SemanticEntityType::NUMBER;
                    return newNumberGenGrd;
                }());
                res->children.emplace(GrammaticalType::SPECIFIER, std::move(pSemExp));
                pSemExp = std::move(res);
            } else {
                genGrdPtr->referenceType = SemanticReferenceType::INDEFINITE;
                genGrdPtr->quantity.setNumber(pNumber);
            }
            removeChild(*grdExpPtr, GrammaticalType::INTRODUCTING_WORD);
        }
    }
}

void setAtPluralFromSemExp(SemanticExpression& pSemExp) {
    auto* grdExpPtr = pSemExp.getGrdExpPtr_SkipWrapperPtrs();
    if (grdExpPtr != nullptr) {
        auto* genGrdPtr = grdExpPtr->grounding().getGenericGroundingPtr();
        if (genGrdPtr != nullptr)
            genGrdPtr->quantity.setPlural();
    }
}

void removeChild(GroundedExpression& pGrdExp, GrammaticalType pChildType) {
    auto itChild = pGrdExp.children.find(pChildType);
    if (itChild != pGrdExp.children.end())
        pGrdExp.children.erase(itChild);
}

void removeChildFromSemExp(SemanticExpression& pSemExp, GrammaticalType pChildType) {
    auto* grdExpPtr = pSemExp.getGrdExpPtr_SkipWrapperPtrs();
    if (grdExpPtr != nullptr) {
        removeChild(*grdExpPtr, pChildType);
        return;
    }

    auto* listExpPtr = pSemExp.getListExpPtr_SkipWrapperPtrs();
    if (listExpPtr != nullptr)
        for (auto& currElt : listExpPtr->elts)
            removeChildFromSemExp(*currElt, pChildType);
}

void setChild(GroundedExpression& pGrdExp, GrammaticalType pChildGramType, UniqueSemanticExpression pChildSemExp) {
    if (!pChildSemExp->isEmpty())
        pGrdExp.children[pChildGramType] = std::move(pChildSemExp);
    else
        removeChild(pGrdExp, pChildGramType);
}

void clearRequestList(GroundedExpression& pGrdExp) {
    auto* requests = SemExpGetter::getRequestList(pGrdExp);
    if (requests != nullptr)
        requests->clear();
}

void swapRequests(GroundedExpression& pGrdExp, SemanticRequests& pRequests) {
    if (pGrdExp.grounding().type == SemanticGroundingType::STATEMENT)
        pGrdExp->getStatementGrounding().requests.swap(pRequests);
}

void clearRequestListOfSemExp(SemanticExpression& pSemExp) {
    switch (pSemExp.type) {
        case SemanticExpressionType::GROUNDED: clearRequestList(pSemExp.getGrdExp()); return;
        case SemanticExpressionType::INTERPRETATION:
            clearRequestListOfSemExp(*pSemExp.getIntExp().interpretedExp);
            return;
        case SemanticExpressionType::FEEDBACK: clearRequestListOfSemExp(*pSemExp.getFdkExp().concernedExp); return;
        case SemanticExpressionType::ANNOTATED: clearRequestListOfSemExp(*pSemExp.getAnnExp().semExp); return;
        case SemanticExpressionType::METADATA: clearRequestListOfSemExp(*pSemExp.getMetadataExp().semExp); return;
        case SemanticExpressionType::LIST: {
            ListExpression& listExp = pSemExp.getListExp();
            for (auto& currElt : listExp.elts) {
                clearRequestListOfSemExp(*currElt);
            }
            return;
        }
        case SemanticExpressionType::FIXEDSYNTHESIS:
        case SemanticExpressionType::COMMAND:
        case SemanticExpressionType::COMPARISON:
        case SemanticExpressionType::CONDITION:
        case SemanticExpressionType::SETOFFORMS: return;
    }
    assert(false);
}

void modifyRequestIfAtPassiveForm(SemanticRequestType& pRequest) {
    if (pRequest == SemanticRequestType::OBJECT)
        pRequest = SemanticRequestType::SUBJECT;
    else if (pRequest == SemanticRequestType::SUBJECT)
        pRequest = SemanticRequestType::OBJECT;
}

void invertSubjectAndObjectGrdExp(GroundedExpression& pGrdExp) {
    std::optional<UniqueSemanticExpression> subjectSemExpOpt;
    {
        auto itSubject = pGrdExp.children.find(GrammaticalType::SUBJECT);
        if (itSubject != pGrdExp.children.end()) {
            subjectSemExpOpt = std::move(itSubject->second);
            pGrdExp.children.erase(itSubject);
        }
    }
    std::optional<UniqueSemanticExpression> objectSemExpOpt;
    {
        auto itObject = pGrdExp.children.find(GrammaticalType::OBJECT);
        if (itObject != pGrdExp.children.end()) {
            subjectSemExpOpt = std::move(itObject->second);
            pGrdExp.children.erase(itObject);
        }
    }
    if (subjectSemExpOpt)
        pGrdExp.children.emplace(GrammaticalType::OBJECT, std::move(*subjectSemExpOpt));
    if (objectSemExpOpt)
        pGrdExp.children.emplace(GrammaticalType::SUBJECT, std::move(*objectSemExpOpt));
}

void invertPolarityFromGrdExp(GroundedExpression& pGrdExp) {
    SemanticStatementGrounding* statementGrdPtr = pGrdExp->getStatementGroundingPtr();
    if (statementGrdPtr != nullptr) {
        auto itObject = pGrdExp.children.find(GrammaticalType::OBJECT);
        if (itObject != pGrdExp.children.end()) {
            SemanticExpression& objSemExp = *itObject->second;
            GroundedExpression* objGrdExpPtr = objSemExp.getGrdExpPtr_SkipWrapperPtrs();
            if (objGrdExpPtr != nullptr) {
                SemanticGrounding& objGrd = objGrdExpPtr->grounding();
                if (!objGrd.polarity) {
                    objGrd.polarity = !objGrd.polarity;
                    return;
                }
                if (statementGrdPtr->polarity
                    && !ConceptSet::haveAConcept(statementGrdPtr->concepts, "verb_equal_be")) {
                    SemanticGenericGrounding* objGenGrdExpPtr = objGrd.getGenericGroundingPtr();
                    if (objGenGrdExpPtr != nullptr
                        && objGenGrdExpPtr->referenceType == SemanticReferenceType::INDEFINITE
                        && objGenGrdExpPtr->quantity.isEqualToOne()) {
                        objGenGrdExpPtr->quantity.setNumber(0);
                        return;
                    }
                }
            }
        }
        statementGrdPtr->polarity = !statementGrdPtr->polarity;
    }
}

void invertPolarity(SemanticExpression& pSemExp) {
    GroundedExpression* grdExpPtr = pSemExp.getGrdExpPtr_SkipWrapperPtrs();
    if (grdExpPtr != nullptr) {
        invertPolarityFromGrdExp(*grdExpPtr);
        return;
    }

    ListExpression* listExpPtr = pSemExp.getListExpPtr_SkipWrapperPtrs();
    if (listExpPtr != nullptr)
        for (auto& currElt : listExpPtr->elts)
            invertPolarity(*currElt);
}

void removeEmptyListElements(UniqueSemanticExpression& pListExp) {
    if (pListExp->type != SemanticExpressionType::LIST)
        return;
    ListExpression& listExp = pListExp->getListExp();

    // remove the nothing semExps
    for (auto itElt = listExp.elts.begin(); itElt != listExp.elts.end();) {
        if ((*itElt)->isEmpty())
            itElt = listExp.elts.erase(itElt);
        else
            ++itElt;
    }
    if (listExp.listType == ListExpressionType::UNRELATED && listExp.elts.empty())
        pListExp.clear();
    if (listExp.elts.size() == 1)
        pListExp = std::move(listExp.elts.front());
}

void setReference(GroundedExpression& pGrdExp, SemanticReferenceType pRefType) {
    auto* genGrdPtr = pGrdExp->getGenericGroundingPtr();
    if (genGrdPtr != nullptr)
        genGrdPtr->referenceType = pRefType;
}

void setReferenceTypeOfSemExp(SemanticExpression& pSemExp, SemanticReferenceType pRefType) {
    switch (pSemExp.type) {
        case SemanticExpressionType::GROUNDED: setReference(pSemExp.getGrdExp(), pRefType); return;
        case SemanticExpressionType::INTERPRETATION:
            setReferenceTypeOfSemExp(*pSemExp.getIntExp().interpretedExp, pRefType);
            return;
        case SemanticExpressionType::FEEDBACK:
            setReferenceTypeOfSemExp(*pSemExp.getFdkExp().concernedExp, pRefType);
            return;
        case SemanticExpressionType::ANNOTATED: setReferenceTypeOfSemExp(*pSemExp.getAnnExp().semExp, pRefType); return;
        case SemanticExpressionType::METADATA:
            setReferenceTypeOfSemExp(*pSemExp.getMetadataExp().semExp, pRefType);
            return;
        case SemanticExpressionType::LIST: {
            ListExpression& listExp = pSemExp.getListExp();
            for (auto& currElt : listExp.elts)
                setReferenceTypeOfSemExp(*currElt, pRefType);
            return;
        }
        case SemanticExpressionType::FIXEDSYNTHESIS:
        case SemanticExpressionType::COMMAND:
        case SemanticExpressionType::COMPARISON:
        case SemanticExpressionType::CONDITION:
        case SemanticExpressionType::SETOFFORMS: return;
    }
    assert(false);
}

void addRequest(GroundedExpression& pGrdExp, SemanticRequestType pRequestType) {
    if (pGrdExp.grounding().type == SemanticGroundingType::STATEMENT)
        pGrdExp->getStatementGrounding().requests.add(pRequestType);
}

void addRequest(SemanticExpression& pSemExp, SemanticRequestType pRequestType) {
    auto grdExpPtrs = listTopGroundedExpressionsPtr(pSemExp);
    for (auto& grdExpPtr : grdExpPtrs)
        addRequest(*grdExpPtr, pRequestType);
}

void setRequests(SemanticExpression& pSemExp, const SemanticRequests& pRequests) {
    auto grdExpPtrs = listTopGroundedExpressionsPtr(pSemExp);
    for (auto& grdExpPtr : grdExpPtrs) {
        auto& grd = grdExpPtr->grounding();
        if (grd.type == SemanticGroundingType::STATEMENT)
            grd.getStatementGrounding().requests = pRequests;
    }
}

void setRequest(SemanticExpression& pSemExp, SemanticRequestType pRequestType) {
    auto grdExpPtrs = listTopGroundedExpressionsPtr(pSemExp);
    for (auto& grdExpPtr : grdExpPtrs)
        _setRequest(*grdExpPtr, pRequestType);
}

void addCoreferenceMotherSemExp(UniqueSemanticExpression& pSemExp, GrammaticalType pGrammaticalType) {
    auto newRoot = std::make_unique<GroundedExpression>([] {
        auto coreferenceStatGrd = std::make_unique<SemanticStatementGrounding>();
        coreferenceStatGrd->coreference.emplace();
        return coreferenceStatGrd;
    }());
    newRoot->children.emplace(pGrammaticalType, std::move(pSemExp));
    pSemExp = std::move(newRoot);
}

void addReferences(UniqueSemanticExpression& pSemExp, const std::list<std::string>& pReferences) {
    assert(!pReferences.empty());
    auto metaExp = std::make_unique<MetadataExpression>(std::move(pSemExp));
    metaExp->from = SemanticSourceEnum::SEMREACTION;
    metaExp->contextualAnnotation = ContextualAnnotation::ANSWER;
    metaExp->references = pReferences;
    pSemExp = std::move(metaExp);
}

void fillVerbGoal(GroundedExpression& pGrdExp, VerbGoalEnum pVerbGoal) {
    SemanticStatementGrounding* currStatementPtr = pGrdExp->getStatementGroundingPtr();
    if (currStatementPtr != nullptr)
        currStatementPtr->verbGoal = pVerbGoal;
}

bool putItAdjectival(SemanticExpression& pSemExp, const linguistics::LinguisticDatabase& pLingDb) {
    auto* grdExpPtr = pSemExp.getGrdExpPtr_SkipWrapperPtrs();
    if (grdExpPtr != nullptr) {
        auto* genGrdPtr = grdExpPtr->grounding().getGenericGroundingPtr();
        if (genGrdPtr != nullptr && !genGrdPtr->word.lemma.empty()) {
            std::list<linguistics::InflectedWord> inflectedWords;
            pLingDb.langToSpec[genGrdPtr->word.language].lingDico.getGramPossibilities(
                inflectedWords, genGrdPtr->word.lemma, 0, genGrdPtr->word.lemma.size());
            for (const auto& currInflWord : inflectedWords) {
                if (currInflWord.word.partOfSpeech == PartOfSpeech::ADJECTIVE) {
                    genGrdPtr->word = currInflWord.word;
                    genGrdPtr->quantity.clear();
                    genGrdPtr->entityType = SemanticEntityType::MODIFIER;
                    return true;
                }
            }
        }
    }
    return false;
}

void replaceAgentOfSemExp(SemanticExpression& pSemExp,
                          const std::string& pNewUserId,
                          const std::string& pUserIdToReplace) {
    switch (pSemExp.type) {
        case SemanticExpressionType::GROUNDED: {
            GroundedExpression& grdExp = pSemExp.getGrdExp();
            SemanticAgentGrounding* agentGrd = grdExp->getAgentGroundingPtr();
            if (agentGrd != nullptr && agentGrd->userId == pUserIdToReplace)
                pSemExp.getGrdExp().moveGrounding(std::make_unique<SemanticAgentGrounding>(pNewUserId));
            for (auto& currChild : grdExp.children)
                replaceAgentOfSemExp(*currChild.second, pNewUserId, pUserIdToReplace);
            return;
        }
        case SemanticExpressionType::INTERPRETATION:
            replaceAgentOfSemExp(*pSemExp.getIntExp().interpretedExp, pNewUserId, pUserIdToReplace);
            return;
        case SemanticExpressionType::FEEDBACK:
            replaceAgentOfSemExp(*pSemExp.getFdkExp().concernedExp, pNewUserId, pUserIdToReplace);
            return;
        case SemanticExpressionType::ANNOTATED: {
            auto& annExp = pSemExp.getAnnExp();
            replaceAgentOfSemExp(*annExp.semExp, pNewUserId, pUserIdToReplace);
            for (auto& currAnnotation : annExp.annotations)
                replaceAgentOfSemExp(*currAnnotation.second, pNewUserId, pUserIdToReplace);
            return;
        }
        case SemanticExpressionType::METADATA: {
            MetadataExpression& metaExp = pSemExp.getMetadataExp();
            if (metaExp.source)
                replaceAgentOfSemExp(**metaExp.source, pNewUserId, pUserIdToReplace);
            replaceAgentOfSemExp(*metaExp.semExp, pNewUserId, pUserIdToReplace);
            return;
        }
        case SemanticExpressionType::LIST: {
            ListExpression& listExp = pSemExp.getListExp();
            for (auto& currElt : listExp.elts)
                replaceAgentOfSemExp(*currElt, pNewUserId, pUserIdToReplace);
            return;
        }
        case SemanticExpressionType::COMPARISON: {
            ComparisonExpression& compExp = pSemExp.getCompExp();
            if (compExp.whatIsComparedExp)
                replaceAgentOfSemExp(**compExp.whatIsComparedExp, pNewUserId, pUserIdToReplace);
            replaceAgentOfSemExp(*compExp.leftOperandExp, pNewUserId, pUserIdToReplace);
            if (compExp.rightOperandExp)
                replaceAgentOfSemExp(**compExp.rightOperandExp, pNewUserId, pUserIdToReplace);
            return;
        }
        case SemanticExpressionType::CONDITION: {
            ConditionExpression& condExp = pSemExp.getCondExp();
            replaceAgentOfSemExp(*condExp.conditionExp, pNewUserId, pUserIdToReplace);
            replaceAgentOfSemExp(*condExp.thenExp, pNewUserId, pUserIdToReplace);
            if (condExp.elseExp)
                replaceAgentOfSemExp(**condExp.elseExp, pNewUserId, pUserIdToReplace);
            return;
        }
        case SemanticExpressionType::FIXEDSYNTHESIS:
        case SemanticExpressionType::COMMAND:
        case SemanticExpressionType::SETOFFORMS: return;
    }
    assert(false);
}

void applyVerbTenseModif(GroundedExpression& pGrdExp, std::function<void(SemanticVerbTense&)> pVerbTenseModif) {
    if (pGrdExp.grounding().type == SemanticGroundingType::STATEMENT)
        pVerbTenseModif(pGrdExp->getStatementGrounding().verbTense);
}

void applyVerbTenseModifOfSemExp(SemanticExpression& pSemExp, std::function<void(SemanticVerbTense&)> pVerbTenseModif) {
    switch (pSemExp.type) {
        case SemanticExpressionType::GROUNDED: applyVerbTenseModif(pSemExp.getGrdExp(), pVerbTenseModif); return;
        case SemanticExpressionType::INTERPRETATION:
            applyVerbTenseModifOfSemExp(*pSemExp.getIntExp().interpretedExp, pVerbTenseModif);
            return;
        case SemanticExpressionType::FEEDBACK:
            applyVerbTenseModifOfSemExp(*pSemExp.getFdkExp().concernedExp, pVerbTenseModif);
            return;
        case SemanticExpressionType::ANNOTATED:
            applyVerbTenseModifOfSemExp(*pSemExp.getAnnExp().semExp, pVerbTenseModif);
            return;
        case SemanticExpressionType::METADATA:
            applyVerbTenseModifOfSemExp(*pSemExp.getMetadataExp().semExp, pVerbTenseModif);
            return;
        case SemanticExpressionType::LIST: {
            ListExpression& listExp = pSemExp.getListExp();
            for (auto& currElt : listExp.elts)
                applyVerbTenseModifOfSemExp(*currElt, pVerbTenseModif);
            return;
        }
        case SemanticExpressionType::COMPARISON: {
            ComparisonExpression& compExp = pSemExp.getCompExp();
            pVerbTenseModif(compExp.tense);
            if (compExp.whatIsComparedExp)
                applyVerbTenseModifOfSemExp(**compExp.whatIsComparedExp, pVerbTenseModif);
            applyVerbTenseModifOfSemExp(*compExp.leftOperandExp, pVerbTenseModif);
            if (compExp.rightOperandExp)
                applyVerbTenseModifOfSemExp(**compExp.rightOperandExp, pVerbTenseModif);
            return;
        }
        case SemanticExpressionType::CONDITION: {
            ConditionExpression& condExp = pSemExp.getCondExp();
            applyVerbTenseModifOfSemExp(*condExp.conditionExp, pVerbTenseModif);
            applyVerbTenseModifOfSemExp(*condExp.thenExp, pVerbTenseModif);
            if (condExp.elseExp)
                applyVerbTenseModifOfSemExp(**condExp.elseExp, pVerbTenseModif);
            return;
        }
        case SemanticExpressionType::FIXEDSYNTHESIS:
        case SemanticExpressionType::COMMAND:
        case SemanticExpressionType::SETOFFORMS: return;
    }
    assert(false);
}

void modifyVerbTense(GroundedExpression& pGrdExp, SemanticVerbTense pVerbTense) {
    applyVerbTenseModif(pGrdExp,
                        [pVerbTense](SemanticVerbTense& pVerbTenseToModify) { pVerbTenseToModify = pVerbTense; });
}

void modifyVerbTenseOfSemExp(SemanticExpression& pSemExp, SemanticVerbTense pVerbTense) {
    applyVerbTenseModifOfSemExp(
        pSemExp, [pVerbTense](SemanticVerbTense& pVerbTenseToModify) { pVerbTenseToModify = pVerbTense; });
}

void putInPastWithTimeAnnotation(UniqueSemanticExpression& pSemExp, std::unique_ptr<SemanticTimeGrounding> pTimeGrd) {
    GroundedExpression* grdExpPtr = pSemExp->getGrdExpPtr_SkipWrapperPtrs();
    if (grdExpPtr != nullptr) {
        GroundedExpression& grdExp = *grdExpPtr;
        SemanticStatementGrounding* statGrdExpPtr = grdExp->getStatementGroundingPtr();
        if (statGrdExpPtr != nullptr) {
            SemanticStatementGrounding& statGrdExp = *statGrdExpPtr;
            statGrdExp.verbTense = SemanticVerbTense::PUNCTUALPAST;
        }
    }

    auto res = std::make_unique<AnnotatedExpression>(std::move(pSemExp));
    res->annotations.emplace(GrammaticalType::TIME, std::make_unique<GroundedExpression>(std::move(pTimeGrd)));
    pSemExp = std::move(res);
}

void replaceSayByAskToRobot(SemanticExpression& pSemExp) {
    GroundedExpression* grdExpPtr = pSemExp.getGrdExpPtr_SkipWrapperPtrs();
    if (grdExpPtr != nullptr) {
        auto& grdExp = *grdExpPtr;
        SemanticStatementGrounding* statGrdPtr = grdExp->getStatementGroundingPtr();
        if (statGrdPtr != nullptr) {
            auto itSayCpt = statGrdPtr->concepts.find("verb_action_say");
            if (itSayCpt == statGrdPtr->concepts.end())
                itSayCpt = statGrdPtr->concepts.find("verb_action_write");
            if (itSayCpt != statGrdPtr->concepts.end()) {
                statGrdPtr->concepts.emplace("verb_action_say_ask", itSayCpt->second);
                statGrdPtr->concepts.erase(itSayCpt);
                grdExp.children.emplace(
                    GrammaticalType::RECEIVER,
                    std::make_unique<GroundedExpression>(SemanticAgentGrounding::getRobotAgentPtr()));
            }
        }
    }
}

void addNewSemExp(UniqueSemanticExpression& pRootSemExp,
                  UniqueSemanticExpression&& pSemExpToAdd,
                  ListExpressionType pListType) {
    ListExpression* listExp = pRootSemExp->getListExpPtr();
    if (listExp != nullptr && listExp->listType == pListType) {
        listExp->elts.emplace_back(std::move(pSemExpToAdd));
        return;
    }

    auto newListExp = std::make_unique<ListExpression>(pListType);
    newListExp->elts.emplace_back(std::move(pRootSemExp));
    newListExp->elts.emplace_back(std::move(pSemExpToAdd));
    pRootSemExp = std::move(newListExp);
}

void addNewChildWithConcept(std::map<GrammaticalType, UniqueSemanticExpression>& pChildren,
                            GrammaticalType pGramOfChild,
                            const std::string& pConceptName) {
    auto genGrd = std::make_unique<SemanticGenericGrounding>();
    genGrd->concepts[pConceptName] = 4;
    if (pConceptName == "agent") {
        genGrd->entityType = SemanticEntityType::HUMAN;
    }
    pChildren.emplace(pGramOfChild, std::make_unique<GroundedExpression>(std::move(genGrd)));
}

void addASemExp(UniqueSemanticExpression& pSemExp, UniqueSemanticExpression pSemExpToAdd) {
    if (pSemExp->isEmpty()) {
        pSemExp = std::move(pSemExpToAdd);
        return;
    }

    ListExpression* listExp = pSemExp->getListExpPtr();
    if (listExp != nullptr && listExp->listType == ListExpressionType::UNRELATED) {
        listExp->elts.emplace_back(std::move(pSemExpToAdd));
        return;
    }

    auto newListExp = std::make_unique<ListExpression>();
    newListExp->elts.emplace_back(std::move(pSemExp));
    newListExp->elts.emplace_back(std::move(pSemExpToAdd));
    pSemExp = std::move(newListExp);
}

void moveChildrenOfAGrdExp(GroundedExpression& pGrdExpToFill, GroundedExpression& pGrdExp) {
    auto itChild = pGrdExp.children.begin();
    while (itChild != pGrdExp.children.end()) {
        GrammaticalType newChildType = itChild->first;
        auto itGramChild = pGrdExpToFill.children.find(newChildType);
        if (itGramChild == pGrdExpToFill.children.end()) {
            pGrdExpToFill.children.emplace(newChildType, std::move(itChild->second));
            itChild = pGrdExp.children.erase(itChild);
            continue;
        }

        addASemExp(itGramChild->second, std::move(itChild->second));
        itChild = pGrdExp.children.erase(itChild);
    }
}

UniqueSemanticExpression grdExpsToUniqueSemExp(const std::list<const GroundedExpression*>& pGrdExps) {
    assert(!pGrdExps.empty());
    if (pGrdExps.size() == 1) {
        return pGrdExps.front()->clone();
    }

    auto resListExp = std::make_unique<ListExpression>(ListExpressionType::AND);
    for (const auto& currGrdExp : pGrdExps) {
        resListExp->elts.push_back(currGrdExp->clone());
    }
    return UniqueSemanticExpression(std::move(resListExp));
}

UniqueSemanticExpression fromImperativeToActionDescription(const GroundedExpression& pGrdExp) {
    auto copiedGrdExp = pGrdExp.clone();
    SemanticStatementGrounding* statGrdExpPtr = copiedGrdExp->grounding().getStatementGroundingPtr();
    if (statGrdExpPtr != nullptr) {
        statGrdExpPtr->verbTense = SemanticVerbTense::UNKNOWN;
        statGrdExpPtr->requests.clear();
    }
    auto itSubject = copiedGrdExp->children.find(GrammaticalType::SUBJECT);
    if (itSubject != copiedGrdExp->children.end())
        copiedGrdExp->children.erase(itSubject);
    return std::move(copiedGrdExp);
}

UniqueSemanticExpression fromActionDescriptionToSentenceInPresentTense(const GroundedExpression& pGrdExp) {
    auto copiedGrdExp = pGrdExp.clone();
    SemanticStatementGrounding* statGrdExpPtr = copiedGrdExp->grounding().getStatementGroundingPtr();
    if (statGrdExpPtr != nullptr)
        statGrdExpPtr->verbTense = SemanticVerbTense::PRESENT;
    if (!copiedGrdExp->children.count(GrammaticalType::SUBJECT))
        copiedGrdExp->children.emplace(
            GrammaticalType::SUBJECT, std::make_unique<GroundedExpression>(SemanticAgentGrounding::getRobotAgentPtr()));
    return std::move(copiedGrdExp);
}

void addChildrenOfAnotherSemExp(SemanticExpression& pSemExpToFill,
                                const SemanticExpression& pSemExp,
                                ListExpressionType pListType) {
    const auto* grdExpPtr = pSemExp.getGrdExpPtr_SkipWrapperPtrs();
    if (grdExpPtr != nullptr)
        for (const auto& currChild : grdExpPtr->children)
            addChildFromSemExp(pSemExpToFill, currChild.first, currChild.second->clone(), pListType);
}

void addEmptyIntroductingWord(GroundedExpression& pGrdExp) {
    pGrdExp.children[GrammaticalType::INTRODUCTING_WORD] = UniqueSemanticExpression();
}

void addChildFromSemExp(SemanticExpression& pSemExp,
                        GrammaticalType pGramType,
                        UniqueSemanticExpression pNewChild,
                        ListExpressionType pListType) {
    GroundedExpression* grdExpPtr = pSemExp.getGrdExpPtr_SkipWrapperPtrs();
    if (grdExpPtr != nullptr) {
        addChild(*grdExpPtr, pGramType, std::move(pNewChild), pListType);
    } else {
        ListExpression* listExpPtr = pSemExp.getListExpPtr_SkipWrapperPtrs();
        if (listExpPtr != nullptr) {
            for (auto itElt = listExpPtr->elts.begin(); itElt != listExpPtr->elts.end();) {
                auto itNextElt = itElt;
                ++itNextElt;
                if (itNextElt == listExpPtr->elts.end()) {
                    addChildFromSemExp(**itElt, pGramType, std::move(pNewChild), pListType);
                    break;
                }
                addChildFromSemExp(**itElt, pGramType, pNewChild->clone(), pListType);
                itElt = itNextElt;
            }
        }
    }
}

bool fillReferenceFromConcepts(SemanticReferenceType& pReferenceType, const std::map<std::string, char>& pConcepts) {
    if (ConceptSet::haveAConceptThatBeginWith(pConcepts, "reference_")) {
        if (ConceptSet::haveAConcept(pConcepts, "reference_indefinite"))
            pReferenceType = SemanticReferenceType::INDEFINITE;
        else if (ConceptSet::haveAConcept(pConcepts, "reference_definite"))
            pReferenceType = SemanticReferenceType::DEFINITE;
        return true;
    }
    return false;
}

bool fillQuantityAndReferenceFromConcepts(SemanticQuantity& pQuantity,
                                          SemanticReferenceType& pReferenceType,
                                          const std::map<std::string, char>& pConcepts) {
    if (ConceptSet::haveAConceptThatBeginWith(pConcepts, "quantity_")) {
        if (ConceptSet::haveAConcept(pConcepts, "quantity_anything")) {
            pReferenceType = SemanticReferenceType::INDEFINITE;
            pQuantity.type = SemanticQuantityType::ANYTHING;
            return true;
        } else if (ConceptSet::haveAConcept(pConcepts, "quantity_everything")) {
            pReferenceType = SemanticReferenceType::INDEFINITE;
            pQuantity.type = SemanticQuantityType::EVERYTHING;
            return true;
        } else if (ConceptSet::haveAConcept(pConcepts, "quantity_maxNumber")) {
            pReferenceType = SemanticReferenceType::DEFINITE;
            pQuantity.type = SemanticQuantityType::MAXNUMBER;
            return true;
        } else if (ConceptSet::haveAConcept(pConcepts, "quantity_1")) {
            pQuantity.setNumber(1);
            return true;
        } else if (ConceptSet::haveAConcept(pConcepts, "quantity_nothing")) {
            pReferenceType = SemanticReferenceType::INDEFINITE;
            pQuantity.setNumber(0);
            return true;
        }
    }
    return false;
}

void fillCoreference(mystd::optional<Coreference>& pCoreference, const std::map<std::string, char>& pConcepts) {
    if (ConceptSet::haveAConceptThatBeginWith(pConcepts, "fromRecentContext"))
        pCoreference.emplace();
}

void fillConceptsForPronouns(std::map<std::string, char>& pSemConcepts,
                             const std::map<std::string, char>& pLingConcepts) {
    for (const auto& currLingCpt : pLingConcepts) {
        if (ConceptSet::doesConceptBeginWith(currLingCpt.first, "tolink_")
            || ConceptSet::doesConceptBeginWith(currLingCpt.first, "reference_")
            || ConceptSet::doesConceptBeginWith(currLingCpt.first, "quantity_") || currLingCpt.first == "agent"
            || currLingCpt.first == "agent_human" || currLingCpt.first == "fromRecentContext")
            continue;
        pSemConcepts.emplace(currLingCpt);
    }
}

void fillSemanticConcepts(std::map<std::string, char>& pConcepts, const std::map<std::string, char>& pConceptsToAdd) {
    for (const auto& currLingCpt : pConceptsToAdd) {
        if (ConceptSet::doesConceptBeginWith(currLingCpt.first, "number_")
            || ConceptSet::doesConceptBeginWith(currLingCpt.first, "reference_")
            || ConceptSet::doesConceptBeginWith(currLingCpt.first, "quantity_")
            || currLingCpt.first == "fromRecentContext")
            continue;
        pConcepts.emplace(currLingCpt);
    }
}

void removeSemExpPartsThatDoesntHaveAnAgent(UniqueSemanticExpression& pSemExp,
                                            const SemanticAgentGrounding& pAgentGrd) {
    auto& rooSemExp = [&]() -> UniqueSemanticExpression& {
        auto* metaExpPtr = pSemExp->getMetadataExpPtr();
        if (metaExpPtr != nullptr)
            return metaExpPtr->semExp;
        return pSemExp;
    }();

    auto* listExpPtr = rooSemExp->getListExpPtr();
    if (listExpPtr != nullptr) {
        auto& listExp = *listExpPtr;
        for (auto it = listExp.elts.begin(); it != listExp.elts.end();) {
            if (!SemExpGetter::doesSemExpHaveAnAgent(**it, pAgentGrd))
                it = listExp.elts.erase(it);
            else
                ++it;
        }
        auto size = listExp.elts.size();
        if (size == 0)
            rooSemExp = UniqueSemanticExpression();
        else if (size == 1)
            rooSemExp = std::move(listExp.elts.front());
    } else if (!SemExpGetter::doesSemExpHaveAnAgent(*rooSemExp, pAgentGrd)) {
        rooSemExp = UniqueSemanticExpression();
    }
}

bool removeYearInformation(SemanticExpression& pSemExp) {
    auto* grdExpPtr = pSemExp.getGrdExpPtr_SkipWrapperPtrs();
    if (grdExpPtr != nullptr) {
        auto* timeGrdPtr = grdExpPtr->grounding().getTimeGroundingPtr();
        if (timeGrdPtr != nullptr && timeGrdPtr->date.year && timeGrdPtr->date.month) {
            timeGrdPtr->date.year.reset();
            return true;
        }
    }
    return false;
}

bool removeDayInformation(SemanticExpression& pSemExp) {
    auto* grdExpPtr = pSemExp.getGrdExpPtr_SkipWrapperPtrs();
    if (grdExpPtr != nullptr) {
        auto* timeGrdPtr = grdExpPtr->grounding().getTimeGroundingPtr();
        if (timeGrdPtr != nullptr && timeGrdPtr->date.day && timeGrdPtr->date.month) {
            timeGrdPtr->date.day.reset();
            return true;
        }
    }
    return false;
}

void addAnythingChild(SemanticExpression& pSemExp, GrammaticalType pGrammaticalType) {
    std::list<GroundedExpression*> grdExpPtrs;
    pSemExp.getGrdExpPtrs_SkipWrapperLists(grdExpPtrs, true, false, false);
    for (const auto& currGrdExpPtr : grdExpPtrs)
        _addAnythingChildFromGrdExp(*currGrdExpPtr, pGrammaticalType);
}

}    // End of namespace SemExpModifier

}    // End of namespace onsem
