#ifndef ONSEM_TEXTTOSEMANTIC_TYPES_SEMANTICEXPRESSION_METADATAEXPRESSIONEXPRESSION_HPP
#define ONSEM_TEXTTOSEMANTIC_TYPES_SEMANTICEXPRESSION_METADATAEXPRESSIONEXPRESSION_HPP

#include "semanticexpression.hpp"
#include <memory>
#include <onsem/common/enum/contextualannotation.hpp>
#include <onsem/common/enum/semanticsourceenum.hpp>
#include <onsem/texttosemantic/dbtype/interactioncontext.hpp>
#include "../../api.hpp"

namespace onsem {
static const std::string linkedInfoLabel_serviceName = "serviceName";
static const std::string linkedInfoLabel_propertyName = "propertyName";
static const std::string linkedInfoLabel_eventName = "eventName";
static const std::string linkedInfoLabel_eventValue = "eventValue";
static const std::string linkedInfoLabel_informationFromKnowledge = "informationFromKnowledge";
static const std::string linkedInfoLabel_removableInfo = "removableInfo";

struct ONSEM_TEXTTOSEMANTIC_API MetadataExpression : public SemanticExpression {
    template<typename TSEMEXP>
    MetadataExpression(std::unique_ptr<TSEMEXP> pSemExp);
    MetadataExpression(UniqueSemanticExpression&& pSemExp);
    MetadataExpression(SemanticSourceEnum pFrom,
                       UniqueSemanticExpression&& pSource,
                       UniqueSemanticExpression&& pSemExp);

    MetadataExpression(const MetadataExpression&) = delete;
    MetadataExpression& operator=(const MetadataExpression&) = delete;

    MetadataExpression& getMetadataExp() override { return *this; }
    const MetadataExpression& getMetadataExp() const override { return *this; }
    MetadataExpression* getMetadataExpPtr() override { return this; }
    const MetadataExpression* getMetadataExpPtr() const override { return this; }

    bool operator==(const MetadataExpression& pOther) const;
    bool isEqual(const MetadataExpression& pOther) const;
    void assertEltsEqual(const MetadataExpression& pOther) const;

    std::unique_ptr<MetadataExpression> clone(
        const IndexToSubNameToParameterValue* pParams = nullptr,
        bool pRemoveRecentContextInterpretations = false,
        const std::set<SemanticExpressionType>* pExpressionTypesToSkip = nullptr) const;

    bool isSourceASentence() const;
    const SemanticExpression* getAuthorSemExpPtr() const;

    const SemanticAgentGrounding* getAuthorPtr() const;

    /**
     * @brief getAuthorId gets an identifier for the current user, usable on SemanticMemory
     * @return a string user identifier
     */
    const std::string& getAuthorId() const;

    const SemanticExpression* getReceiverSemExpPtr() const;

    static void addReference(std::list<std::string>& pReferences, const std::string& pReference);

    static UniqueSemanticExpression constructSourceFromSourceEnum(std::unique_ptr<SemanticAgentGrounding> pAuthor,
                                                                  std::unique_ptr<SemanticAgentGrounding> pReceiver,
                                                                  SemanticSourceEnum pFrom,
                                                                  std::unique_ptr<SemanticTimeGrounding> pTimeGrd);

    static UniqueSemanticExpression constructSourceFromSourceEnumInPresent(
        std::unique_ptr<SemanticAgentGrounding> pAuthor,
        SemanticSourceEnum pFrom,
        ContextualAnnotation pContextualAnnotation);

    /// From where the semantic epxression comes from.
    SemanticSourceEnum from;

    /// What is the type of semantic expression (answer, question, proactive, ...).
    ContextualAnnotation contextualAnnotation;

    /// From what language the semantic expression comes from.
    SemanticLanguageEnum fromLanguage;

    /// Confidence between 0 and 100 about the semantic expression.
    unsigned char confidence;

    /**
     * @brief Set of references (identifiers) linked to the semantic expression.<br/>
     * It is not used by onsem. Onsem only keeps the information and forwards it.<br/>
     * It is written and read by the user.
     */
    std::list<std::string> references;

    /// Semantic expression describing the metadata.
    mystd::unique_propagate_const<UniqueSemanticExpression> source;

    /// Semantic expression.
    UniqueSemanticExpression semExp;

    /// Structure to store some potential parameters associated to this semantic expression.
    std::unique_ptr<InteractionContextContainer> interactionContextContainer;
};

template<typename TSEMEXP>
MetadataExpression::MetadataExpression(std::unique_ptr<TSEMEXP> pSemExp)
    : SemanticExpression(SemanticExpressionType::METADATA)
    , from(SemanticSourceEnum::UNKNOWN)
    , contextualAnnotation(ContextualAnnotation::PROACTIVE)
    , fromLanguage(SemanticLanguageEnum::UNKNOWN)
    , confidence(100)
    , references()
    , source()
    , semExp(std::move(pSemExp))
    , interactionContextContainer() {}

inline bool MetadataExpression::operator==(const MetadataExpression& pOther) const {
    return isEqual(pOther);
}

inline bool MetadataExpression::isEqual(const MetadataExpression& pOther) const {
    return from == pOther.from && contextualAnnotation == pOther.contextualAnnotation
        && fromLanguage == pOther.fromLanguage && confidence == pOther.confidence
        && references == pOther.references && source == pOther.source && semExp == pOther.semExp
        && interactionContextContainer == pOther.interactionContextContainer;
}

}    // End of namespace onsem

#endif    // ONSEM_TEXTTOSEMANTIC_TYPES_SEMANTICEXPRESSION_METADATAEXPRESSIONEXPRESSION_HPP
