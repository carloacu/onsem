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

static const std::string garbadgeIntent = "__garbadge__";
static const std::string notAdressedToMeIntent = "__not_adressed_to_me__";
static const std::string agreementIntent = "__agreement__";
static const std::string disagreementIntent = "__disagreement__";
static const std::string unknownIntent = "__unknown__";


class ONSEMSEMANTICTOTEXT_API ActionRecognizer {
public:
    ActionRecognizer(SemanticLanguageEnum pLanguage);

    struct Intent {
        Intent(const std::string& pName = "")
            : name(pName),
              params()
        {
        }
        std::string name;
        std::map<std::string, std::vector<std::string>> params;

        std::string toStr() const;
        std::string toJson() const;
    };

    struct ActionRecognized {
        std::optional<Intent> intent;
        std::unique_ptr<ActionRecognized> condition;

        std::list<ActionRecognized> toRunSequentially;
        std::list<ActionRecognized> toRunInParallel;
        std::list<ActionRecognized> toRunInBackground;

        bool isOnlyAnIntent() const;
        bool empty() const;
        std::string toJson() const;
        std::string toJsonWithIntentInStr() const;
    };

    struct ParamInfo {
        ParamInfo(const std::string& pType,
                  const std::vector<std::string>& pQuestions);
        std::string type;
        std::vector<std::string> questions;
    };

    void addType(const std::string& pType,
                 const std::vector<std::string>& pFormulations,
                 bool pIsValueConsideredAsOwner);

    void clearEntities();
    void addEntity(const std::string& pType,
                   const std::string& pEntityId,
                   const std::vector<std::string>& pEntityLabels,
                   const linguistics::LinguisticDatabase& pLingDb);

    void addPredicate(const std::string& pPredicateName,
                      const std::vector<std::string>& pPredicateFormulations,
                      const linguistics::LinguisticDatabase& pLingDb);

    void clearActions();
    void addAction(const std::string& pActionIntentName,
                   const std::vector<std::string>& pIntentFormulations,
                   const std::map<std::string, ParamInfo>& pParameterLabelToInfos,
                   const linguistics::LinguisticDatabase& pLingDb);

    bool isObviouslyWrong(const std::string& pActionIntentName,
                          const SemanticExpression& pUtteranceSemExp,
                          const linguistics::LinguisticDatabase& pLingDb) const;

    std::optional<ActionRecognized> recognize(UniqueSemanticExpression pUtteranceSemExp,
                                              const linguistics::LinguisticDatabase& pLingDb);

    SemanticLanguageEnum getLanguage() const { return _language; }

    void setNameOfSelf(const std::string& pNameOfSelf) { _nameOfSelf = pNameOfSelf; }

private:
    SemanticLanguageEnum _language;
    std::string _nameOfSelf;
    SemanticMemory _actionSemanticMemory;
    SemanticMemory _predicateSemanticMemory;
    std::map<std::string, std::vector<std::string>> _typeToFormulations;
    std::set<std::string> _typeWithValueConsideredAsOwner;
    std::map<std::string, SemanticMemory> _typeToMemory;
    std::map<std::string, std::list<UniqueSemanticExpression>> _actionToSemExps;

    std::optional<ActionRecognizer::ActionRecognized> _extractAction(UniqueSemanticExpression pUtteranceSemExp,
                                                                     const linguistics::LinguisticDatabase& pLingDb);
};


}    // End of namespace onsem

#endif    // ONSEM_SEMANTICTOTEXT_ACTIONRECOGNIZER_HPP
