#include <onsem/texttosemantic/dbtype/semanticexpression/metadataexpression.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/annotatedexpression.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/groundedexpression.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticagentgrounding.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticstatementgrounding.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticmetagrounding.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semantictimegrounding.hpp>
#include <onsem/texttosemantic/tool/semexpgetter.hpp>

namespace onsem {

MetadataExpression::MetadataExpression(UniqueSemanticExpression&& pSemExp)
    : SemanticExpression(SemanticExpressionType::METADATA)
    , from(SemanticSourceEnum::UNKNOWN)
    , contextualAnnotation(ContextualAnnotation::PROACTIVE)
    , fromLanguage(SemanticLanguageEnum::UNKNOWN)
    , references()
    , source()
    , semExp(std::move(pSemExp))
    , interactionContextContainer() {}

MetadataExpression::MetadataExpression(SemanticSourceEnum pFrom,
                                       UniqueSemanticExpression&& pSource,
                                       UniqueSemanticExpression&& pSemExp)
    : SemanticExpression(SemanticExpressionType::METADATA)
    , from(pFrom)
    , contextualAnnotation(ContextualAnnotation::PROACTIVE)
    , fromLanguage(SemanticLanguageEnum::UNKNOWN)
    , references()
    , source(std::move(pSource))
    , semExp(std::move(pSemExp))
    , interactionContextContainer() {}

void MetadataExpression::assertEltsEqual(const MetadataExpression& pOther) const {
    assert(from == pOther.from);
    assert(contextualAnnotation == pOther.contextualAnnotation);
    assert(fromLanguage == pOther.fromLanguage);
    assert(fromText == pOther.fromText);
    assert(references == pOther.references);
    _assertSemExpOptsEqual(source, pOther.source);
    semExp->assertEqual(*pOther.semExp);
    assert(interactionContextContainer == pOther.interactionContextContainer);
}

std::unique_ptr<MetadataExpression> MetadataExpression::clone(
    const IndexToSubNameToParameterValue* pParams,
    bool pRemoveRecentContextInterpretations,
    const std::set<SemanticExpressionType>* pExpressionTypesToSkip) const {
    auto res = std::make_unique<MetadataExpression>(
        semExp->clone(pParams, pRemoveRecentContextInterpretations, pExpressionTypesToSkip));
    res->from = from;
    res->contextualAnnotation = contextualAnnotation;
    res->fromLanguage = fromLanguage;
    res->references = references;
    if (source)
        res->source.emplace((*source)->clone(pParams, pRemoveRecentContextInterpretations, pExpressionTypesToSkip));
    if (interactionContextContainer)
        res->interactionContextContainer = interactionContextContainer->clone();
    res->fromText = fromText;
    return res;
}

bool MetadataExpression::isSourceASentence() const {
    if (source) {
        const auto* sourceGrdExpPtr = (*source)->getGrdExpPtr_SkipWrapperPtrs();
        if (sourceGrdExpPtr != nullptr) {
            const auto* statGrdPtr = sourceGrdExpPtr->grounding().getStatementGroundingPtr();
            return statGrdPtr != nullptr && (!statGrdPtr->concepts.empty() || !statGrdPtr->word.isEmpty());
        }
    }
    return false;
}

const SemanticExpression* MetadataExpression::getAuthorSemExpPtr() const {
    if (source) {
        const GroundedExpression* sourceGrdExpPtr = (*source)->getGrdExpPtr_SkipWrapperPtrs();
        if (sourceGrdExpPtr != nullptr) {
            auto subjectChild = sourceGrdExpPtr->children.find(GrammaticalType::SUBJECT);
            if (subjectChild != sourceGrdExpPtr->children.end())
                return &*subjectChild->second;
        }
    }
    return nullptr;
}

const SemanticAgentGrounding* MetadataExpression::getAuthorPtr() const {
    const SemanticExpression* sourceSubjectSemExpPtr = getAuthorSemExpPtr();
    if (sourceSubjectSemExpPtr != nullptr)
        return SemExpGetter::extractAgentGrdPtr(*sourceSubjectSemExpPtr);
    return nullptr;
}

const std::string& MetadataExpression::getAuthorId() const {
    const auto* authorPtr = getAuthorPtr();
    if (authorPtr != nullptr)
        return authorPtr->userId;
    static const std::string empty;
    return empty;
}

const SemanticExpression* MetadataExpression::getReceiverSemExpPtr() const {
    if (source) {
        const GroundedExpression* sourceGrdExpPtr = (*source)->getGrdExpPtr_SkipWrapperPtrs();
        if (sourceGrdExpPtr != nullptr) {
            auto receiverChild = sourceGrdExpPtr->children.find(GrammaticalType::RECEIVER);
            if (receiverChild != sourceGrdExpPtr->children.end())
                return &*receiverChild->second;
        }
    }
    return nullptr;
}

void MetadataExpression::addReference(std::list<std::string>& pReferences, const std::string& pReference) {
    for (const std::string& currRef : pReferences)
        if (currRef == pReference)
            return;
    pReferences.emplace_back(pReference);
}

UniqueSemanticExpression _constructSourceWithSpecificTime(std::unique_ptr<SemanticAgentGrounding> pAuthor,
                                                          const std::string& pVerbConceptStr,
                                                          std::unique_ptr<SemanticAgentGrounding> pReceiver,
                                                          std::unique_ptr<SemanticTimeGrounding> pTimeGrd) {
    auto grdExpSource = std::make_unique<GroundedExpression>([&pVerbConceptStr] {
        auto statementGrd = std::make_unique<SemanticStatementGrounding>();
        if (!pVerbConceptStr.empty()) {
            statementGrd->verbTense = SemanticVerbTense::PUNCTUALPAST;
            statementGrd->concepts.emplace(pVerbConceptStr, 4);
        }
        return statementGrd;
    }());
    grdExpSource->children.emplace(GrammaticalType::SUBJECT, std::make_unique<GroundedExpression>(std::move(pAuthor)));
    grdExpSource->children.emplace(GrammaticalType::RECEIVER,
                                   std::make_unique<GroundedExpression>(std::move(pReceiver)));
    grdExpSource->children.emplace(
        GrammaticalType::OBJECT,
        std::make_unique<GroundedExpression>(std::make_unique<SemanticMetaGrounding>(SemanticGroundingType::META, 0)));

    auto res = std::make_unique<AnnotatedExpression>(std::move(grdExpSource));
    res->annotations.emplace(GrammaticalType::TIME, std::make_unique<GroundedExpression>(std::move(pTimeGrd)));
    return res;
}

UniqueSemanticExpression _constructSourceInPresent(std::unique_ptr<SemanticAgentGrounding> pAuthor,
                                                   const std::string& pVerbConceptStr) {
    auto grdExpSource = std::make_unique<GroundedExpression>([&pVerbConceptStr] {
        auto statementGrd = std::make_unique<SemanticStatementGrounding>();
        statementGrd->verbTense = SemanticVerbTense::PRESENT;
        statementGrd->concepts.emplace(pVerbConceptStr, 4);
        return statementGrd;
    }());
    grdExpSource->children.emplace(GrammaticalType::SUBJECT, std::make_unique<GroundedExpression>(std::move(pAuthor)));
    grdExpSource->children.emplace(
        GrammaticalType::OBJECT,
        std::make_unique<GroundedExpression>(std::make_unique<SemanticMetaGrounding>(SemanticGroundingType::META, 0)));
    return grdExpSource;
}

UniqueSemanticExpression MetadataExpression::constructSourceFromSourceEnum(
    std::unique_ptr<SemanticAgentGrounding> pAuthor,
    std::unique_ptr<SemanticAgentGrounding> pReceiver,
    SemanticSourceEnum pFrom,
    std::unique_ptr<SemanticTimeGrounding> pTimeGrd) {
    if (pAuthor && pReceiver && pTimeGrd) {
        switch (pFrom) {
            case SemanticSourceEnum::ASR:
            case SemanticSourceEnum::SEMREACTION:
            case SemanticSourceEnum::TTS:
                return _constructSourceWithSpecificTime(
                    std::move(pAuthor), "verb_action_say", std::move(pReceiver), std::move(pTimeGrd));
            case SemanticSourceEnum::WRITTENTEXT:
                return _constructSourceWithSpecificTime(
                    std::move(pAuthor), "verb_action_write", std::move(pReceiver), std::move(pTimeGrd));
            default:
                return _constructSourceWithSpecificTime(
                    std::move(pAuthor), "", std::move(pReceiver), std::move(pTimeGrd));
        };
    }
    return UniqueSemanticExpression();
}

UniqueSemanticExpression MetadataExpression::constructSourceFromSourceEnumInPresent(
    std::unique_ptr<SemanticAgentGrounding> pAuthor,
    SemanticSourceEnum pFrom,
    ContextualAnnotation pContextualAnnotation) {
    std::string subCptName;
    if (pContextualAnnotation == ContextualAnnotation::ANSWER)
        subCptName = "_answer";
    else if (pContextualAnnotation == ContextualAnnotation::QUESTION)
        subCptName = "_ask";

    if (pAuthor) {
        switch (pFrom) {
            case SemanticSourceEnum::ASR:
            case SemanticSourceEnum::SEMREACTION:
            case SemanticSourceEnum::TTS:
                return _constructSourceInPresent(std::move(pAuthor), "verb_action_say" + subCptName);
            case SemanticSourceEnum::WRITTENTEXT:
                return _constructSourceInPresent(std::move(pAuthor), "verb_action_write" + subCptName);
            default: return UniqueSemanticExpression();
        };
    }
    return UniqueSemanticExpression();
}

}    // End of namespace onsem
