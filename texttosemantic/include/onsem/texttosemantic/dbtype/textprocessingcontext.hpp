#ifndef ONSEM_TEXTTOSEMANTIC_TYPES_TEXTPROCESSINGCONTEXT_HPP
#define ONSEM_TEXTTOSEMANTIC_TYPES_TEXTPROCESSINGCONTEXT_HPP

#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticagentgrounding.hpp>
#include <onsem/common/enum/semanticlanguagetype.hpp>
#include <onsem/texttosemantic/dbtype/misc/spellingmistaketype.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/semanticexpression.hpp>
#include <onsem/texttosemantic/dbtype/resourcegroundingextractor.hpp>
#include "../api.hpp"

namespace onsem
{


struct ONSEM_TEXTTOSEMANTIC_API TextProcessingContext
{
  TextProcessingContext(const std::string& pAuthorId,
                        const std::string& pReceiverId,
                        SemanticLanguageEnum pLangType);
  TextProcessingContext(const std::string& pAuthorId,
                        const std::string& pReceiverId,
                        SemanticLanguageEnum pLangType,
                        UniqueSemanticExpression pUsSemExp);
  TextProcessingContext(const TextProcessingContext& pOther);

  void setUsAsYouAndMe(); // default config
  void setUsAsEverybody();

  static TextProcessingContext getTextProcessingContextFromRobot(SemanticLanguageEnum pLanguage);
  static TextProcessingContext getTextProcessingContextToRobot(SemanticLanguageEnum pLanguage);

  const SemanticAgentGrounding author;
  const SemanticAgentGrounding receiver;
  SemanticLanguageEnum langType;
  bool isTimeDependent;
  UniqueSemanticExpression usSemExp;
  bool vouvoiement;
  std::shared_ptr<ResourceGroundingExtractor> cmdGrdExtractorPtr;
  std::set<SpellingMistakeType> spellingMistakeTypesPossible;
  bool rawValue;
};



} // End of namespace onsem


#endif // ONSEM_TEXTTOSEMANTIC_TYPES_TEXTPROCESSINGCONTEXT_HPP
