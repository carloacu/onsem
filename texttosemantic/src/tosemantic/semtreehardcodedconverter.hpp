#ifndef ONSEM_TEXTTOSEMANTIC_TOSEMANTICEXPRESSION_SEMTREEHARDCODEDCONVERTER_HPP
#define ONSEM_TEXTTOSEMANTIC_TOSEMANTICEXPRESSION_SEMTREEHARDCODEDCONVERTER_HPP

#include <map>
#include <memory>
#include <onsem/common/enum/grammaticaltype.hpp>

namespace onsem {
struct GroundedExpression;
struct SemanticExpression;
struct ConditionExpression;
struct TextProcessingContext;

namespace semTreeHardCodedConverter {

bool manageEnglishAuxiliaries(GroundedExpression& pGrdExp, const std::map<std::string, char>& pAuxConcepts);

std::unique_ptr<SemanticExpression> convertEnglishSentenceToASemExp(std::unique_ptr<GroundedExpression> pGrdExp);

void refactorEnglishSentencesWithAGoal(
    std::unique_ptr<SemanticExpression>& pSemExp,
    std::map<GrammaticalType, std::unique_ptr<ConditionExpression>>& pChildTypeToCondition);

void refactorFrenchSentencesWithAGoal(
    std::unique_ptr<SemanticExpression>& pSemExp,
    std::map<GrammaticalType, std::unique_ptr<ConditionExpression>>& pChildTypeToCondition,
    const TextProcessingContext& pTextProcContext,
    bool pIsPassive);

}    // End of namespace semTreeHardCodedConverter
}    // End of namespace onsem

#endif    // ONSEM_TEXTTOSEMANTIC_TOSEMANTICEXPRESSION_SEMTREEHARDCODEDCONVERTER_HPP
