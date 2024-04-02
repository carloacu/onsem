#ifndef ONSEM_SEMANTICTOTEXT_ACTIONRECOGNIZER_HPP
#define ONSEM_SEMANTICTOTEXT_ACTIONRECOGNIZER_HPP

#include <optional>
#include <string>
#include <vector>
#include <onsem/common/enum/semanticlanguageenum.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemory.hpp>
#include "api.hpp"

namespace onsem {
namespace linguistics {
struct LinguisticDatabase;
}
struct UniqueSemanticExpression;


class ONSEMSEMANTICTOTEXT_API ActionRecognizer {
public:
    ActionRecognizer(SemanticLanguageEnum pLanguage);

    struct Intent {
        std::string name;
        std::map<std::string, std::vector<std::string>> slotNameToValues;

        std::string toStr() const;
    };

    struct ActionRecognized {
        std::optional<Intent> action;
        std::optional<Intent> condition;

        bool empty() const;
        std::string toJson() const;
    };

    struct ParamInfo {
        ParamInfo(const std::string& pType,
                  const std::vector<std::string>& pQuestions);
        std::string type;
        std::vector<std::string> questions;
    };

    void addType(const std::string& pType,
                 const std::vector<std::string>& pFormulations);

    void addEntity(const std::string& pType,
                   const std::string& pEntityId,
                   const std::vector<std::string>& pEntityLabels,
                   const linguistics::LinguisticDatabase& pLingDb);

    void addPredicate(const std::string& pPredicateName,
                      const std::vector<std::string>& pPredicateFormulations,
                      const linguistics::LinguisticDatabase& pLingDb);

    void addAction(const std::string& pActionIntentName,
                   const std::vector<std::string>& pIntentFormulations,
                   const std::map<std::string, ParamInfo>& pParameterLabelToInfos,
                   const linguistics::LinguisticDatabase& pLingDb);

    std::optional<ActionRecognized> recognize(UniqueSemanticExpression pUtteranceSemExp,
                                              const linguistics::LinguisticDatabase& pLingDb);

    SemanticLanguageEnum getLanguage() { return _language; }


private:
    SemanticLanguageEnum _language;
    SemanticMemory _actionSemanticMemory;
    SemanticMemory _predicateSemanticMemory;
    std::map<std::string, std::vector<std::string>> _typeToFormulations;
    std::map<std::string, SemanticMemory> _typeToMemory;
};


}    // End of namespace onsem

#endif    // ONSEM_SEMANTICTOTEXT_ACTIONRECOGNIZER_HPP
