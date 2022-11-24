#ifndef ONSEM_SEMANTICTOTEXT_TOOL_SEMEXPCOMPARATOR_HPP
#define ONSEM_SEMANTICTOTEXT_TOOL_SEMEXPCOMPARATOR_HPP

#include <list>
#include <map>
#include <set>
#include <string>
#include <onsem/common/enum/comparisonoperator.hpp>
#include <onsem/common/enum/semanticrequesttype.hpp>
#include <onsem/texttosemantic/dbtype/misc/imbricationType.hpp>
#include <onsem/common/enum/semanticverbtense.hpp>
#include "../api.hpp"

namespace onsem
{
namespace linguistics
{
struct LinguisticDatabase;
}
struct SemanticGrounding;
struct SemanticAgentGrounding;
struct SemanticGenericGrounding;
struct SemanticStatementGrounding;
struct SemanticDurationGrounding;
struct GroundedExpression;
struct SemanticExpression;
struct SemanticQuantity;
class ConceptSet;
struct SemanticMemory;
struct SemanticMemoryBlock;


namespace SemExpComparator
{

/// A list of attributes to ignore in a comparison.
struct ComparisonExceptions
{
  /// Sub semantic expressions of the first semantic exression to ignore.
  std::set<const SemanticExpression*> semExps1ToSkip{};
  /// Sub semantic expressions of the second semantic exression to ignore.
  std::set<const SemanticExpression*> semExps2ToSkip{};

  bool quantity = false;

  /// Whether to ignore the request type of the semantic expression.
  bool request = false;

  bool polarity = false;
  bool equivalentUserId = false;
  bool corefenceExcludeEquality = false;

  /// Whether to ignore the time attributes.
  bool verbTense = false;

  /// The grammatical types to ignore in the comparison.
  std::set<GrammaticalType> grammaticalTypes;
};



struct ComparisonErrorReporting
{
  std::map<GrammaticalType, std::map<ImbricationType, std::size_t>> childrenThatAreNotEqual;

  void addError(GrammaticalType pGrammType,
                ImbricationType pImpbricationType,
                std::size_t pNbOfErrors = 1)
  {
    auto& childForAGram = childrenThatAreNotEqual[pGrammType];
    auto it = childForAGram.find(pImpbricationType);
    if (it != childForAGram.end())
      it->second += pNbOfErrors;
    else
      childrenThatAreNotEqual[pGrammType].emplace(pImpbricationType, pNbOfErrors);
  }

  std::size_t nbOfErrors()
  {
    std::size_t res = 0;
    for (auto& currGramChild : childrenThatAreNotEqual)
      for (auto& currImbr : currGramChild.second)
        res += currImbr.second;
    return res;
  }
};


ONSEMSEMANTICTOTEXT_API
ImbricationType getQuantityImbrication(const SemanticQuantity& pQuantity1,
                                       const SemanticQuantity& pQuantity2);

ONSEMSEMANTICTOTEXT_API
ImbricationType switchOrderOfEltsImbrication(ImbricationType pImbrication);

ONSEMSEMANTICTOTEXT_API
bool grdHaveNbSetToZero(const SemanticGrounding& pGrd);

ONSEMSEMANTICTOTEXT_API
bool grdsHaveSamePolarity(const SemanticGrounding& pGrd1,
                          const SemanticGrounding& pGrd2,
                          const ConceptSet& pConceptsDb);

ONSEMSEMANTICTOTEXT_API
bool haveSamePolarity(const GroundedExpression& pGrdExp1,
                      const GroundedExpression& pGrdExp2,
                      const ConceptSet& pConceptsDb);

ONSEMSEMANTICTOTEXT_API
bool doesGrdExpContainEverything(const GroundedExpression& pGrdExp);

ONSEMSEMANTICTOTEXT_API
bool isAnInstanceOf(const GroundedExpression& pInstanceOfMetaDesc,
                    const GroundedExpression& pMetaDesc,
                    const linguistics::LinguisticDatabase& pLingDb);

ONSEMSEMANTICTOTEXT_API
bool semExpsAreEqualFromMemBlock(const SemanticExpression& pSemExp1,
                                 const SemanticExpression& pSemExp2,
                                 const SemanticMemoryBlock& pMemBlock,
                                 const linguistics::LinguisticDatabase& pLingDb,
                                 const ComparisonExceptions* pExceptionsPtr);

ONSEMSEMANTICTOTEXT_API
bool semExpsAreEqualOrIsContainedFromMemBlock(const SemanticExpression& pSemExp1,
                                              const SemanticExpression& pSemExp2,
                                              const SemanticMemoryBlock& pMemBlock,
                                              const linguistics::LinguisticDatabase& pLingDb,
                                              const ComparisonExceptions* pExceptionsPtr);

ONSEMSEMANTICTOTEXT_API
bool areGrdExpEqualsExceptForTheQuantity(const GroundedExpression& pGrdExp1,
                                         const GroundedExpression& pGrdExp2,
                                         const ConceptSet& pConceptSet);

ONSEMSEMANTICTOTEXT_API
bool semExpsAreEqual(const SemanticExpression& pSemExp1,
                     const SemanticExpression& pSemExp2,
                     const SemanticMemory& pSemanticMemory,
                     const linguistics::LinguisticDatabase& pLingDb);

ONSEMSEMANTICTOTEXT_API
bool semExpsAreEqual(const SemanticExpression& pSemExp1,
                     const SemanticExpression& pSemExp2,
                     const SemanticMemoryBlock& pMemBlock,
                     const linguistics::LinguisticDatabase& pLingDb,
                     const ComparisonExceptions* pExceptionsPtr = nullptr);


ONSEMSEMANTICTOTEXT_API
ImbricationType getSemExpsImbrications(const SemanticExpression& pSemExp1,
                                       const SemanticExpression& pSemExp2,
                                       const SemanticMemoryBlock& pMemBlock,
                                       const linguistics::LinguisticDatabase& pLingDb,
                                       const ComparisonExceptions* pExceptionsPtr,
                                       ComparisonErrorReporting* pComparisonErrorReportingPtr = nullptr,
                                       GrammaticalType pParentGrammaticalType = GrammaticalType::UNKNOWN);

ONSEMSEMANTICTOTEXT_API
ImbricationType getGrdExpsImbrications(const GroundedExpression& pGrdExp1,
                                       const GroundedExpression& pGrdExp2,
                                       const SemanticMemoryBlock& pMemBlock,
                                       const linguistics::LinguisticDatabase& pLingDb,
                                       const ComparisonExceptions* pExceptionsPtr,
                                       ComparisonErrorReporting* pComparisonErrorReportingPtr = nullptr,
                                       GrammaticalType pParentGrammaticalType = GrammaticalType::UNKNOWN);

ONSEMSEMANTICTOTEXT_API
bool grdExpsAreEqual(const GroundedExpression& pGrdExp1,
                     const GroundedExpression& pGrdExp2,
                     const SemanticMemoryBlock& pMemBlock,
                     const linguistics::LinguisticDatabase& pLingDb,
                     const ComparisonExceptions* pExceptionsPtr = nullptr,
                     ComparisonErrorReporting* pComparisonErrorReportingPtr = nullptr);

ONSEMSEMANTICTOTEXT_API
bool groundingsAreEqual(const SemanticGrounding& pGrounding1,
                        const SemanticGrounding& pGrounding2,
                        const SemanticMemoryBlock& pMemBlock,
                        const linguistics::LinguisticDatabase& pLingDb);

ONSEMSEMANTICTOTEXT_API
ComparisonOperator numberComparisonOfSemExps(const SemanticExpression& pSemExp1,
                                             const SemanticExpression& pSemExp2);


ONSEMSEMANTICTOTEXT_API
bool grdExpsReferToSameInstance(const GroundedExpression& pGrdExp1,
                                const GroundedExpression& pGrdExp2,
                                const SemanticMemoryBlock& pMemBlock,
                                const linguistics::LinguisticDatabase& pLingDb);

} // End of namespace SemExpComparator
} // End of namespace onsem





// Implemenation

namespace onsem
{
namespace SemExpComparator
{

inline bool semExpsAreEqualFromMemBlock(const SemanticExpression& pSemExp1,
                                        const SemanticExpression& pSemExp2,
                                        const SemanticMemoryBlock& pMemBlock,
                                        const linguistics::LinguisticDatabase& pLingDb,
                                        const ComparisonExceptions* pExceptionsPtr)
{
  return getSemExpsImbrications(pSemExp1, pSemExp2, pMemBlock, pLingDb, pExceptionsPtr) == ImbricationType::EQUALS;
}

inline bool semExpsAreEqualOrIsContainedFromMemBlock(const SemanticExpression& pSemExp1,
                                                     const SemanticExpression& pSemExp2,
                                                     const SemanticMemoryBlock& pMemBlock,
                                                     const linguistics::LinguisticDatabase& pLingDb,
                                                     const ComparisonExceptions* pExceptionsPtr)
{
  ImbricationType res = getSemExpsImbrications(pSemExp1, pSemExp2, pMemBlock, pLingDb, pExceptionsPtr);
  return res == ImbricationType::EQUALS || res == ImbricationType::ISCONTAINED ||
      res == ImbricationType::LESS_DETAILED || res == ImbricationType::HYPONYM;
}


inline bool grdExpsAreEqual(const GroundedExpression& pGrdExp1,
                            const GroundedExpression& pGrdExp2,
                            const SemanticMemoryBlock& pMemBlock,
                            const linguistics::LinguisticDatabase& pLingDb,
                            const ComparisonExceptions* pExceptionsPtr,
                            ComparisonErrorReporting* pComparisonErrorReportingPtr)
{
  return getGrdExpsImbrications(pGrdExp1, pGrdExp2, pMemBlock, pLingDb,
                                pExceptionsPtr, pComparisonErrorReportingPtr, GrammaticalType::UNKNOWN) == ImbricationType::EQUALS;
}

} // End of namespace SemExpComparator
} // End of namespace onsem

#endif // ONSEM_SEMANTICTOTEXT_TOOL_SEMEXPCOMPARATOR_HPP
