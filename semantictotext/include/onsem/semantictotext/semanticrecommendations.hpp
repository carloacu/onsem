#ifndef ONSEM_SEMANTICTOTEXT_SRC_SEMANTICMEMORY_SEMANTICRECOMMENDATIONS_HPP
#define ONSEM_SEMANTICTOTEXT_SRC_SEMANTICMEMORY_SEMANTICRECOMMENDATIONS_HPP

#include <set>
#include <onsem/semantictotext/semanticmemory/semanticmemorybloc.hpp>
#include "api.hpp"

namespace onsem
{

struct ONSEMSEMANTICTOTEXT_API SemanticGroundingsToRecommendationCoef
{
  std::list<std::shared_ptr<ExpressionHandleInMemory>> expsWrappedInMemory{};
  std::map<const ExpressionHandleInMemory*, int> expToCoef{};
  SemanticMemoryBlock memBlock{SemanticMemoryBlock::infinteMemory};
};


struct ONSEMSEMANTICTOTEXT_API SemanticRecommendationsContainer
{
  std::set<std::string> recommendationIds{};
  std::list<std::shared_ptr<ExpressionHandleInMemory>> expsWrappedInMemory{};
  std::map<const ExpressionHandleInMemory*, std::pair<std::string, int>> recommendationsToNumberOfLinks{};
  SemanticMemoryBlock memBlock{SemanticMemoryBlock::infinteMemory};
  SemanticGroundingsToRecommendationCoef goundingsToCoef{};
};


ONSEMSEMANTICTOTEXT_API
void addGroundingCoef(SemanticGroundingsToRecommendationCoef& pGrdContainer,
                      UniqueSemanticExpression pGroundings,
                      int pCoef,
                      const linguistics::LinguisticDatabase& pLingDb);


ONSEMSEMANTICTOTEXT_API
void addARecommendation(SemanticRecommendationsContainer& pContainer,
                        UniqueSemanticExpression pRecommendation,
                        const std::string& pRecommendationId,
                        const linguistics::LinguisticDatabase& pLingDb);


ONSEMSEMANTICTOTEXT_API
void getRecommendations(std::map<int, std::set<std::string>>& pRecommendations,
                        const SemanticExpression& pInput,
                        const SemanticRecommendationsContainer& pContainer,
                        const linguistics::LinguisticDatabase& pLingDb);


} // End of namespace onsem

#endif // ONSEM_SEMANTICTOTEXT_SRC_SEMANTICMEMORY_SEMANTICRECOMMENDATIONS_HPP
