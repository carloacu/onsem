#include <onsem/texttosemantic/dbtype/textprocessingcontext.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/listexpression.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/groundedexpression.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticgenericgrouding.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticresourcegrounding.hpp>
#include <onsem/common/utility/make_unique.hpp>


namespace onsem
{

std::unique_ptr<SemanticExpression> _getYouAndMeSemExp
(const SemanticAgentGrounding& pAuthor,
 const SemanticAgentGrounding& pReceiver)
{
  auto agentList = mystd::make_unique<ListExpression>(ListExpressionType::AND);
  agentList->elts.push_back(mystd::make_unique<GroundedExpression>
                            (mystd::make_unique<SemanticAgentGrounding>(pAuthor)));
  agentList->elts.push_back(mystd::make_unique<GroundedExpression>
                            (mystd::make_unique<SemanticAgentGrounding>(pReceiver)));
  return std::move(agentList);
}


TextProcessingContext::TextProcessingContext
(const std::string& pAuthorId,
 const std::string& pReceiverId,
 SemanticLanguageEnum pLangType)
  : author(pAuthorId),
    receiver(pReceiverId),
    langType(pLangType),
    usSemExp(_getYouAndMeSemExp(pAuthorId, pReceiverId)),
    vouvoiement(false),
    cmdGrdExtractorPtr(),
    spellingMistakeTypesPossible()
{
}


TextProcessingContext::TextProcessingContext
(const std::string& pAuthorId,
 const std::string& pReceiverId,
 SemanticLanguageEnum pLangType,
 UniqueSemanticExpression pUsSemExp)
  : author(pAuthorId),
    receiver(pReceiverId),
    langType(pLangType),
    usSemExp(std::move(pUsSemExp)),
    vouvoiement(false),
    cmdGrdExtractorPtr(),
    spellingMistakeTypesPossible()
{
}


TextProcessingContext::TextProcessingContext
(const TextProcessingContext& pOther)
  : author(pOther.author),
    receiver(pOther.receiver),
    langType(pOther.langType),
    usSemExp(pOther.usSemExp->clone()),
    vouvoiement(pOther.vouvoiement),
    cmdGrdExtractorPtr(pOther.cmdGrdExtractorPtr),
    spellingMistakeTypesPossible(pOther.spellingMistakeTypesPossible)
{
}


void TextProcessingContext::setUsAsYouAndMe()
{
  usSemExp = _getYouAndMeSemExp(author.userId, receiver.userId);
}


void TextProcessingContext::setUsAsEverybody()
{
  usSemExp = mystd::make_unique<GroundedExpression>([]
  {
    auto genGrd = mystd::make_unique<SemanticGenericGrounding>();
    genGrd->concepts["agent_*"] = 4;
    genGrd->referenceType = SemanticReferenceType::INDEFINITE;
    genGrd->entityType = SemanticEntityType::HUMAN;
    genGrd->quantity.type = SemanticQuantityType::EVERYTHING;
    return genGrd;
  }());
}


TextProcessingContext TextProcessingContext::getTextProcessingContextFromRobot(SemanticLanguageEnum pLanguage)
{
  return TextProcessingContext(SemanticAgentGrounding::me,
                               SemanticAgentGrounding::currentUser,
                               pLanguage);
}

TextProcessingContext TextProcessingContext::getTextProcessingContextToRobot(SemanticLanguageEnum pLanguage)
{
  return TextProcessingContext(SemanticAgentGrounding::currentUser,
                               SemanticAgentGrounding::me,
                               pLanguage);
}


} // End of namespace onsem


