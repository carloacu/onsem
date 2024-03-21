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
    ActionRecognizer();

    struct Intent {
        std::string name;
        std::map<std::string, std::vector<std::string>> slotNameToValues;

        std::string toStr() const;
    };

    struct ActionRecognized {
        Intent action;
        std::optional<Intent> condition;

        std::string toJson() const;
    };

    void addPredicate(const std::string& pPredicateName,
                      const std::vector<std::string>& pPredicateFormulations,
                      const linguistics::LinguisticDatabase& pLingDb,
                      SemanticLanguageEnum pLanguage);

    void addAction(const std::string& pActionIntentName,
                   const std::vector<std::string>& pIntentFormulations,
                   const linguistics::LinguisticDatabase& pLingDb,
                   SemanticLanguageEnum pLanguage);

    std::optional<ActionRecognized> recognize(UniqueSemanticExpression pUtteranceSemExp,
                                              const linguistics::LinguisticDatabase& pLingDb);


private:
    SemanticMemory _actionSemanticMemory;
    SemanticMemory _predicateSemanticMemory;
};


}    // End of namespace onsem

#endif    // ONSEM_SEMANTICTOTEXT_ACTIONRECOGNIZER_HPP
