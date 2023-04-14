/*
 * Allow to do matching of sentences.<br/>
 * Can be usefull for hard coded chatbots or to have triggers that point to actions to do.
 */
#ifndef ONSEM_SEMANTICTOTEXT_TRIGGERS_HPP
#define ONSEM_SEMANTICTOTEXT_TRIGGERS_HPP

#include <map>
#include <string>
#include <vector>
#include <onsem/common/enum/semanticlanguageenum.hpp>
#include <onsem/common/utility/unique_propagate_const.hpp>
#include "api.hpp"


namespace onsem
{
namespace linguistics
{
struct LinguisticDatabase;
}
struct UniqueSemanticExpression;
struct SemanticMemory;
struct ExpressionWithLinks;
struct ReactionOptions;

namespace triggers
{


/**
 * @brief Register a reaction to a trigger.
 * @param pTriggerSemExp Trigger semantic expression.
 * @param pReactionSemExp Reaction semantic expression.
 * @param pSemanticMemory Memory object where this mapping is stored.
 * @param pLingDb Linguistic database.
 */
ONSEMSEMANTICTOTEXT_API
void add(UniqueSemanticExpression pTriggerSemExp,
         UniqueSemanticExpression pReactionSemExp,
         SemanticMemory& pSemanticMemory,
         const linguistics::LinguisticDatabase& pLingDb);


/**
 * @brief Register a resource in reaction of a trigger.
 * @param pTriggerText Trigger text.
 * @param pResourceLabel Label of the resource in reaction.
 * @param pResourceValue Value of the resource in reaction.
 * @param pResourceParameterLabelToQuestions Parameters of the resource in reaction.<br/>
 * After the matching, the executor will try to answer the question based on the user utterance.
 * @param pSemanticMemory Memory object where this mapping is stored.
 * @param pLingDb Linguistic database.
 * @param pLanguage Language of the trigger and the resource.
 */
ONSEMSEMANTICTOTEXT_API
void addToResource(const std::string& pTriggerText,
    const std::string& pResourceLabel,
    const std::string& pResourceValue,
    const std::map<std::string, std::vector<UniqueSemanticExpression>>& pResourceParameterLabelToQuestions,
    SemanticMemory& pSemanticMemory,
    const linguistics::LinguisticDatabase& pLingDb,
    SemanticLanguageEnum pLanguage);


/**
 * @brief Match a user utterance to a reaction stored in the memory object.
 * @param pReaction Result of the matching.
 * @param pSemanticMemory Memory object where this mappings are stored.
 * @param pUtteranceSemExp Semantic expression of the user utterance.
 * @param pLingDb Linguistic database.
 * @param pReactionOptions Some options to add filters in the matching.
 */
ONSEMSEMANTICTOTEXT_API
std::shared_ptr<ExpressionWithLinks> match(
    mystd::unique_propagate_const<UniqueSemanticExpression>& pReaction,
    SemanticMemory& pSemanticMemory,
    UniqueSemanticExpression pUtteranceSemExp,
    const linguistics::LinguisticDatabase& pLingDb,
    const ReactionOptions* pReactionOptions = nullptr);


/**
 * @brief Create semantic expressions of the resource parameters.
 * @param pParameterLabelToQuestionsSemExps Result.
 * @param pParameterLabelToQuestionsStrs Parameters in strings.
 * @param pLingDb Linguistic database.
 * @param pLanguage Language of the parameters.
 */
ONSEMSEMANTICTOTEXT_API
void createParameterSemanticexpressions(
    std::map<std::string, std::vector<UniqueSemanticExpression>>& pParameterLabelToQuestionsSemExps,
    const std::map<std::string, std::vector<std::string>>& pParameterLabelToQuestionsStrs,
    const linguistics::LinguisticDatabase& pLingDb,
    SemanticLanguageEnum pLanguage = SemanticLanguageEnum::UNKNOWN);


} // End of namespace triggers
} // End of namespace onsem

#endif // ONSEM_SEMANTICTOTEXT_TRIGGERS_HPP
