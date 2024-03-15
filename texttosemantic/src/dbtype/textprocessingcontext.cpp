#include <onsem/texttosemantic/dbtype/textprocessingcontext.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/listexpression.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/groundedexpression.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticgenericgrounding.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticresourcegrounding.hpp>

namespace onsem {

std::unique_ptr<SemanticExpression> _getYouAndMeSemExp(const SemanticAgentGrounding& pAuthor,
                                                       const SemanticAgentGrounding& pReceiver) {
    auto agentList = std::make_unique<ListExpression>(ListExpressionType::AND);
    agentList->elts.push_back(std::make_unique<GroundedExpression>(std::make_unique<SemanticAgentGrounding>(pAuthor)));
    agentList->elts.push_back(
        std::make_unique<GroundedExpression>(std::make_unique<SemanticAgentGrounding>(pReceiver)));
    return agentList;
}

TextProcessingContext::TextProcessingContext(const std::string& pAuthorId,
                                             const std::string& pReceiverId,
                                             SemanticLanguageEnum pLangType)
    : author(pAuthorId)
    , receiver(pReceiverId)
    , langType(pLangType)
    , isTimeDependent(true)
    , usSemExp(_getYouAndMeSemExp(pAuthorId, pReceiverId))
    , vouvoiement(false)
    , writeParametersToFill(false)
    , cmdGrdExtractorPtr()
    , spellingMistakeTypesPossible() {}

TextProcessingContext::TextProcessingContext(const std::string& pAuthorId,
                                             const std::string& pReceiverId,
                                             SemanticLanguageEnum pLangType,
                                             UniqueSemanticExpression pUsSemExp)
    : author(pAuthorId)
    , receiver(pReceiverId)
    , langType(pLangType)
    , isTimeDependent(true)
    , usSemExp(std::move(pUsSemExp))
    , vouvoiement(false)
    , writeParametersToFill(false)
    , cmdGrdExtractorPtr()
    , spellingMistakeTypesPossible() {}

TextProcessingContext::TextProcessingContext(const TextProcessingContext& pOther)
    : author(pOther.author)
    , receiver(pOther.receiver)
    , langType(pOther.langType)
    , isTimeDependent(true)
    , usSemExp(pOther.usSemExp->clone())
    , vouvoiement(pOther.vouvoiement)
    , writeParametersToFill(pOther.writeParametersToFill)
    , cmdGrdExtractorPtr(pOther.cmdGrdExtractorPtr)
    , spellingMistakeTypesPossible(pOther.spellingMistakeTypesPossible) {}

void TextProcessingContext::setUsAsYouAndMe() {
    usSemExp = _getYouAndMeSemExp(author.userId, receiver.userId);
}

void TextProcessingContext::setUsAsEverybody() {
    usSemExp = std::make_unique<GroundedExpression>([] {
        auto genGrd = std::make_unique<SemanticGenericGrounding>();
        genGrd->concepts["agent_*"] = 4;
        genGrd->referenceType = SemanticReferenceType::INDEFINITE;
        genGrd->entityType = SemanticEntityType::HUMAN;
        genGrd->quantity.type = SemanticQuantityType::EVERYTHING;
        return genGrd;
    }());
}

TextProcessingContext TextProcessingContext::getTextProcessingContextFromRobot(SemanticLanguageEnum pLanguage) {
    return TextProcessingContext(SemanticAgentGrounding::me, SemanticAgentGrounding::currentUser, pLanguage);
}

TextProcessingContext TextProcessingContext::getTextProcessingContextToRobot(SemanticLanguageEnum pLanguage) {
    return TextProcessingContext(SemanticAgentGrounding::currentUser, SemanticAgentGrounding::me, pLanguage);
}

}    // End of namespace onsem
