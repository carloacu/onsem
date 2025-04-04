#ifndef ONSEM_SEMANTICTOTEXT_TOOL_SEMEXPCOMPARATOR_HPP
#define ONSEM_SEMANTICTOTEXT_TOOL_SEMEXPCOMPARATOR_HPP

#include <list>
#include <map>
#include <set>
#include <string>
#include <onsem/common/enum/comparisonoperator.hpp>
#include <onsem/common/enum/listexpressiontype.hpp>
#include <onsem/common/enum/semanticrequesttype.hpp>
#include <onsem/common/utility/optional.hpp>
#include <onsem/texttosemantic/dbtype/misc/imbricationType.hpp>
#include <onsem/common/enum/semanticverbtense.hpp>
#include "../api.hpp"

namespace onsem {
namespace linguistics {
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

namespace SemExpComparator {

/// A list of attributes to ignore in a comparison.
struct ComparisonExceptions {
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

    /// Whether to ignore the interpretations of InterpretationExpressions.
    bool interpretations = false;

    /// The grammatical types to ignore in the comparison.
    std::set<GrammaticalType> grammaticalTypes;
};

struct ListExpPtr {
    ListExpPtr(const SemanticExpression& pGrdExp)
        : elts(1, &pGrdExp)
        , listType() {}

    ListExpPtr(const std::list<const SemanticExpression*>& pElts = {},
               const mystd::optional<ListExpressionType>& pListType = mystd::optional<ListExpressionType>())
        : elts(pElts)
        , listType(pListType) {}

    std::list<const SemanticExpression*> elts;
    mystd::optional<ListExpressionType> listType;
};

enum ComparisonTypeOfError : char { NO_ERROR, PARAMETER_DIFF, SPECIFIER, REQUEST, TENSE, NORMAL };

struct ComparisonErrorsCoef {
    explicit ComparisonErrorsCoef(std::size_t pValue = 0, ComparisonTypeOfError pType = ComparisonTypeOfError::NO_ERROR)
        : value(pValue)
        , type(pType) {
        if (value == 0)
            type = ComparisonTypeOfError::NO_ERROR;
    }

    bool operator==(const ComparisonErrorsCoef& pOther) const { return type == pOther.type && value == pOther.value; }
    bool operator<(const ComparisonErrorsCoef& pOther) const {
        if (type != pOther.type)
            return type < pOther.type;
        return value < pOther.value;
    }
    bool operator>(const ComparisonErrorsCoef& pOther) const { return !operator<(pOther) && !operator==(pOther); }

    void add(const ComparisonErrorsCoef& pOther) {
        value += pOther.value;
        type = std::max(type, pOther.type);
    }

    bool empty() const { return type == ComparisonTypeOfError::NO_ERROR; }

    std::size_t value;
    ComparisonTypeOfError type;
};

struct ComparisonErrors {
    ComparisonErrors(const ListExpPtr& pChild1Ptr, const ListExpPtr& pChild2Ptr, const ComparisonErrorsCoef& pErrorCoef)
        : child1Ptr(pChild1Ptr)
        , child2Ptr(pChild2Ptr)
        , errorCoef(pErrorCoef) {}

    const ListExpPtr child1Ptr;
    const ListExpPtr child2Ptr;
    ComparisonErrorsCoef errorCoef;
};

struct ComparisonErrorReporting {
    std::size_t numberOfEqualities = 0;
    std::map<GrammaticalType, std::map<ImbricationType, ComparisonErrors>> childrenThatAreNotEqual{};

    void addError(GrammaticalType pGrammType,
                  ImbricationType pImpbricationType,
                  const ListExpPtr& pChild1Ptr,
                  const ListExpPtr& pChild2Ptr,
                  const ComparisonErrorsCoef& pErrorCoef) {
        auto& childForAGram = childrenThatAreNotEqual[pGrammType];
        auto it = childForAGram.find(pImpbricationType);
        if (it != childForAGram.end())
            it->second.errorCoef.add(pErrorCoef);
        else
            childrenThatAreNotEqual[pGrammType].emplace(pImpbricationType,
                                                        ComparisonErrors(pChild1Ptr, pChild2Ptr, pErrorCoef));
    }

    enum class SmimilarityValue { YES, YES_BUT_INCOMPLETE, NO };

    SmimilarityValue canBeConsideredHasSimilar() const;

    ComparisonErrorsCoef getErrorCoef() {
        ComparisonErrorsCoef res;
        for (auto& currGramChild : childrenThatAreNotEqual)
            for (auto& currImbr : currGramChild.second)
                res.add(currImbr.second.errorCoef);
        if (res.type == ComparisonTypeOfError::PARAMETER_DIFF)
          res.value -= numberOfEqualities * 5;
        return res;
    }
};

ONSEMSEMANTICTOTEXT_API
ImbricationType getQuantityImbrication(const SemanticQuantity& pQuantity1, const SemanticQuantity& pQuantity2);

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
                      const ConceptSet& pConceptsDb,
                      bool pFollowInterpretations);

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
                                             const SemanticExpression& pSemExp2,
                                             bool pFollowInterpretations);

ONSEMSEMANTICTOTEXT_API
bool grdExpsReferToSameInstance(const GroundedExpression& pGrdExp1,
                                const GroundedExpression& pGrdExp2,
                                const SemanticMemoryBlock& pMemBlock,
                                const linguistics::LinguisticDatabase& pLingDb);

}    // End of namespace SemExpComparator
}    // End of namespace onsem

// Implemenation

namespace onsem {
namespace SemExpComparator {

inline bool semExpsAreEqualFromMemBlock(const SemanticExpression& pSemExp1,
                                        const SemanticExpression& pSemExp2,
                                        const SemanticMemoryBlock& pMemBlock,
                                        const linguistics::LinguisticDatabase& pLingDb,
                                        const ComparisonExceptions* pExceptionsPtr) {
    return getSemExpsImbrications(pSemExp1, pSemExp2, pMemBlock, pLingDb, pExceptionsPtr) == ImbricationType::EQUALS;
}

inline bool semExpsAreEqualOrIsContainedFromMemBlock(const SemanticExpression& pSemExp1,
                                                     const SemanticExpression& pSemExp2,
                                                     const SemanticMemoryBlock& pMemBlock,
                                                     const linguistics::LinguisticDatabase& pLingDb,
                                                     const ComparisonExceptions* pExceptionsPtr) {
    ImbricationType res = getSemExpsImbrications(pSemExp1, pSemExp2, pMemBlock, pLingDb, pExceptionsPtr);
    return res == ImbricationType::EQUALS || res == ImbricationType::ISCONTAINED
        || res == ImbricationType::LESS_DETAILED || res == ImbricationType::HYPONYM;
}

inline bool grdExpsAreEqual(const GroundedExpression& pGrdExp1,
                            const GroundedExpression& pGrdExp2,
                            const SemanticMemoryBlock& pMemBlock,
                            const linguistics::LinguisticDatabase& pLingDb,
                            const ComparisonExceptions* pExceptionsPtr,
                            ComparisonErrorReporting* pComparisonErrorReportingPtr) {
    return getGrdExpsImbrications(pGrdExp1,
                                  pGrdExp2,
                                  pMemBlock,
                                  pLingDb,
                                  pExceptionsPtr,
                                  pComparisonErrorReportingPtr,
                                  GrammaticalType::UNKNOWN)
        == ImbricationType::EQUALS;
}

}    // End of namespace SemExpComparator
}    // End of namespace onsem

#endif    // ONSEM_SEMANTICTOTEXT_TOOL_SEMEXPCOMPARATOR_HPP
