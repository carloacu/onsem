#ifndef ONSEM_TEXTTOSEMANTIC_TYPES_TEXTPROCESSINGCONTEXT_HPP
#define ONSEM_TEXTTOSEMANTIC_TYPES_TEXTPROCESSINGCONTEXT_HPP

#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticagentgrounding.hpp>
#include <onsem/common/enum/semanticlanguageenum.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/semanticexpression.hpp>
#include <onsem/texttosemantic/dbtype/linguisticanalysisconfig.hpp>
#include "../api.hpp"

namespace onsem {

struct ONSEM_TEXTTOSEMANTIC_API TextProcessingContext {
    TextProcessingContext(const std::string& pAuthorId, const std::string& pReceiverId, SemanticLanguageEnum pLangType);
    TextProcessingContext(const std::string& pAuthorId,
                          const std::string& pReceiverId,
                          SemanticLanguageEnum pLangType,
                          UniqueSemanticExpression pUsSemExp);
    TextProcessingContext(const TextProcessingContext& pOther);

    void setUsAsYouAndMe();    // default config
    void setUsAsEverybody();

    static TextProcessingContext getTextProcessingContextFromRobot(SemanticLanguageEnum pLanguage);
    static TextProcessingContext getTextProcessingContextToRobot(SemanticLanguageEnum pLanguage);

    const SemanticAgentGrounding author;
    const SemanticAgentGrounding receiver;
    SemanticLanguageEnum langType;
    bool isTimeDependent;
    UniqueSemanticExpression usSemExp;
    bool vouvoiement;
    bool writeParametersToFill;
    bool writeOriginalText;
    LinguisticAnalysisConfig linguisticAnalysisConfig;
};

}    // End of namespace onsem

#endif    // ONSEM_TEXTTOSEMANTIC_TYPES_TEXTPROCESSINGCONTEXT_HPP
