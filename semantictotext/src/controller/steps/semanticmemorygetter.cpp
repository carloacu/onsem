#include "semanticmemorygetter.hpp"
#include <set>
#include <list>
#include <onsem/common/binary/enummapreader.hpp>
#include <onsem/common/binary/radixmapreader.hpp>
#include <onsem/common/binary/simpleintmapping.hpp>
#include <onsem/common/utility/uppercasehandler.hpp>
#include <onsem/texttosemantic/dbtype/binary/semexploader.hpp>
#include <onsem/texttosemantic/dbtype/semanticgroundings.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/groundedexpression.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/listexpression.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/conceptset.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/staticlinguisticdictionary.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase.hpp>
#include <onsem/semantictotext/tool/semexpcomparator.hpp>
#include <onsem/texttosemantic/tool/semexpgetter.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemory.hpp>
#include <onsem/semantictotext/semanticmemory/links/sentencewithlinks.hpp>
#include "../../semanticmemory/memorymodifier.hpp"
#include "../../semanticmemory/semanticmemoryblockprivate.hpp"
#include "../../semanticmemory/semanticmemoryblockviewer.hpp"
#include "../../semanticmemory/semanticmemoryblockbinaryreader.hpp"
#include "semanticmemorylinker.hpp"

namespace onsem {
namespace semanticMemoryGetter {
namespace {
const SemanticTriggerAxiomId _emptyAxiomId;
const SemanticMemoryBlock _emptypMemBlock;
const std::set<const SemanticExpression*> _emptySemExpsToSkip;

SemExpComparator::ComparisonExceptions _createCompWithOrWithoutInterpretations(bool pWithOrWithoutInterpretation) {
    SemExpComparator::ComparisonExceptions res;
    res.interpretations = !pWithOrWithoutInterpretation;
    return res;
}

enum class OtherConceptsLinkStrategy {
    LINK_WITH_CHILDREN_CONCEPTS,
    LINK_WITH_MOTHER_CONCEPTS,
    NO_LINK_TO_OTHER_CONCEPTS
};

template<bool IS_MODIFIABLE>
bool _getRelationsFromSemExp(RelationsThatMatch<IS_MODIFIABLE>& pRelations,
                             const SentenceLinks<IS_MODIFIABLE>& pAlreadyMatchedSentences,
                             MemoryLinksAccessor<IS_MODIFIABLE>& pLinksToSemExps,
                             const SemanticExpression& pSemExpToLookFor,
                             const std::set<const SemanticExpression*>& pChildSemExpsToSkip,
                             RequestContext pRequestContext,
                             const SemanticMemoryBlockPrivate* pMemBlockPrivatePtr,
                             const linguistics::LinguisticDatabase& pLingDb,
                             bool pCheckChildren,
                             SemanticLanguageEnum pLanguage,
                             bool pIsATrigger,
                             const SemanticRelativeTimeType* pRelativeTimePtr = nullptr);

OtherConceptsLinkStrategy _requestCategoryToLinkStrategy(RequestContext pRequestContext) {
    switch (pRequestContext) {
        case RequestContext::SENTENCE_TO_CONDITION: return OtherConceptsLinkStrategy::LINK_WITH_MOTHER_CONCEPTS;
        default: return OtherConceptsLinkStrategy::LINK_WITH_CHILDREN_CONCEPTS;
    }
}

bool _hasAStaticLink(const unsigned char* pLinksToGrdExpFromBinary,
                     const SemanticMemoryBlockPrivate* pMemBlockPrivatePtr,
                     const unsigned char* pMemoryGrdExpPtrToFind) {
    if (pLinksToGrdExpFromBinary != nullptr && pMemBlockPrivatePtr != nullptr
        && pMemBlockPrivatePtr->subBinaryBlockPtr) {
        const unsigned char* beginOfSemExpPtr = pMemBlockPrivatePtr->subBinaryBlockPtr->getSemanticExpressionsBlocPtr();
        uint32_t nbOfElement = binaryloader::loadIntInThreeBytes(pLinksToGrdExpFromBinary);
        pLinksToGrdExpFromBinary += 3;
        const int32_t* intPtr = reinterpret_cast<const int32_t*>(pLinksToGrdExpFromBinary);
        for (uint32_t i = 0; i < nbOfElement; ++i) {
            const unsigned char* memoryGrdExpPtr = beginOfSemExpPtr + *(intPtr++);
            if (memoryGrdExpPtr == pMemoryGrdExpPtrToFind)
                return true;
        }
    }
    return false;
}

template<bool IS_MODIFIABLE>
void _iterateOverStaticLinks(const SentenceLinks<IS_MODIFIABLE>& pAlreadyMatchedSentences,
                             const unsigned char* pLinksToGrdExpFromBinary,
                             const SemanticMemoryBlockPrivate* pMemBlockPrivatePtr,
                             const std::function<void(const unsigned char*, const unsigned char*)>& pMemoryGrdExpPtr) {
    if (pLinksToGrdExpFromBinary != nullptr && pMemBlockPrivatePtr != nullptr
        && pMemBlockPrivatePtr->subBinaryBlockPtr) {
        const bool pAlreadyMatchedSentencesIsEmpty = pAlreadyMatchedSentences.staticLinks.empty();
        const unsigned char* beginOfSemExpPtr = pMemBlockPrivatePtr->subBinaryBlockPtr->getSemanticExpressionsBlocPtr();
        uint32_t nbOfElement = binaryloader::loadIntInThreeBytes(pLinksToGrdExpFromBinary);
        pLinksToGrdExpFromBinary += 3;
        const int32_t* intPtr = reinterpret_cast<const int32_t*>(pLinksToGrdExpFromBinary);
        for (uint32_t i = 0; i < nbOfElement; ++i) {
            const unsigned char* memoryGrdExpPtr = beginOfSemExpPtr + *(intPtr++);
            auto* memorySentencePtr = SemanticMemoryBlockBinaryReader::memoryGrdExpToMemorySentencePtr(memoryGrdExpPtr);
            if (pAlreadyMatchedSentencesIsEmpty
                || pAlreadyMatchedSentences.staticLinks.count(
                       SemanticMemoryBlockBinaryReader::memorySentenceToId(memorySentencePtr))
                       != 0)
                pMemoryGrdExpPtr(memoryGrdExpPtr, memorySentencePtr);
        }
    }
}

struct StaticBinaryLinks {
    StaticBinaryLinks(const unsigned char* pLinksToGrdExp)
        : linksToGrdExp(pLinksToGrdExp)
        , grdExpStaticLinks() {}
    StaticBinaryLinks(std::map<intSemId, const unsigned char*>&& pGrdExpThatStart)
        : linksToGrdExp(nullptr)
        , grdExpStaticLinks(std::move(pGrdExpThatStart)) {}
    const unsigned char* linksToGrdExp;
    std::map<intSemId, const unsigned char*> grdExpStaticLinks;
};

/**
 * We need to iterate in one map and do a find in the other one.
 * For optimization purpose we choose to do iteration in the smalest map.
 */
template<bool IS_MODIFIABLE, typename MEMSENTENCES>
void _executeForLinksAlreadyMatched(
    const SentenceLinks<IS_MODIFIABLE>& pAlreadyMatchedSentences,
    IntIdToMemSentenceAccessor<IS_MODIFIABLE>* pLinksToGrdExp,
    const StaticBinaryLinks* pLinksToGrdExpFromBinary,
    const SemanticMemoryBlockPrivate* pMemBlockPrivatePtr,
    const linguistics::LinguisticDatabase& pLingDb,
    const std::function<void(MEMSENTENCES*,
                             const unsigned char*,
                             const std::function<std::unique_ptr<GroundedExpressionContainer>()>&)>&
        pLinkManagementFunction) {
    if (pLinksToGrdExp != nullptr) {
        if (pAlreadyMatchedSentences.dynamicLinks.empty()) {
            for (auto& eltGrdExp : pLinksToGrdExp->m) {
                for (auto& currGrdExp : eltGrdExp.second) {
                    auto& memSentToAdd = currGrdExp.getMemSentence();
                    pLinkManagementFunction(
                        &memSentToAdd, nullptr, [&]() -> std::unique_ptr<GroundedExpressionContainer> {
                            return std::make_unique<GroundedExpressionRef>(currGrdExp.grdExp);
                        });
                }
            }
        } else if (pLinksToGrdExp->m.size() < pAlreadyMatchedSentences.dynamicLinks.size()) {
            for (auto& eltGrdExp : pLinksToGrdExp->m) {
                if (pAlreadyMatchedSentences.dynamicLinks.count(eltGrdExp.first) != 0) {
                    for (auto& currGrdExp : eltGrdExp.second) {
                        auto& memSentToAdd = currGrdExp.getMemSentence();
                        pLinkManagementFunction(
                            &memSentToAdd, nullptr, [&]() -> std::unique_ptr<GroundedExpressionContainer> {
                                return std::make_unique<GroundedExpressionRef>(currGrdExp.grdExp);
                            });
                    }
                }
            }
        } else {
            for (auto& currAlreadyMatchedSentences : pAlreadyMatchedSentences.dynamicLinks) {
                auto itLink = pLinksToGrdExp->m.find(currAlreadyMatchedSentences.first);
                if (itLink != pLinksToGrdExp->m.end()) {
                    for (auto& currGrdExp : itLink->second) {
                        auto& memSentToAdd = currGrdExp.getMemSentence();
                        pLinkManagementFunction(
                            &memSentToAdd, nullptr, [&]() -> std::unique_ptr<GroundedExpressionContainer> {
                                return std::make_unique<GroundedExpressionRef>(currGrdExp.grdExp);
                            });
                    }
                }
            }
        }
    } else {
        auto considerAMemoryGrdExp = [&](const unsigned char* pMemoryGrdExpPtr,
                                         const unsigned char* pMemorySentencePtr) {
            pLinkManagementFunction(nullptr,
                                    pMemorySentencePtr,
                                    [&pLingDb, pMemoryGrdExpPtr]() -> std::unique_ptr<GroundedExpressionContainer> {
                                        const unsigned char* grdExpPtr =
                                            SemanticMemoryBlockBinaryReader::memoryGrdExpToGrdExpPtr(pMemoryGrdExpPtr);
                                        return std::make_unique<GroundedExpressionFromSemExp>(
                                            UniqueSemanticExpression(semexploader::loadGrdExp(grdExpPtr, pLingDb)));
                                    });
        };

        if (pLinksToGrdExpFromBinary->linksToGrdExp != nullptr) {
            auto* linksToGrdExpPtr = pLinksToGrdExpFromBinary->linksToGrdExp;
            uint32_t nbOfElement = binaryloader::loadIntInThreeBytes(linksToGrdExpPtr);
            if (pAlreadyMatchedSentences.empty() || nbOfElement < pAlreadyMatchedSentences.staticLinks.size()) {
                _iterateOverStaticLinks(
                    pAlreadyMatchedSentences, linksToGrdExpPtr, pMemBlockPrivatePtr, considerAMemoryGrdExp);
            } else {
                if (pMemBlockPrivatePtr != nullptr && pMemBlockPrivatePtr->subBinaryBlockPtr) {
                    const unsigned char* beginOfSemExpPtr =
                        pMemBlockPrivatePtr->subBinaryBlockPtr->getSemanticExpressionsBlocPtr();
                    linksToGrdExpPtr += 3;
                    const uint32_t* intPtr = reinterpret_cast<const uint32_t*>(linksToGrdExpPtr);
                    for (auto& currAlreadyMatchedSentences : pAlreadyMatchedSentences.staticLinks) {
                        const uint32_t* memoryGrdExpOffsetPtr =
                            readSimpleIntMapping(intPtr, nbOfElement, [&](uint32_t pMemoryGrdExp) {
                                const unsigned char* memoryGrdExpPtr = beginOfSemExpPtr + pMemoryGrdExp;
                                auto* memorySentencePtr =
                                    SemanticMemoryBlockBinaryReader::memoryGrdExpToMemorySentencePtr(memoryGrdExpPtr);
                                uint32_t id = SemanticMemoryBlockBinaryReader::memorySentenceToId(memorySentencePtr);
                                if (id == currAlreadyMatchedSentences.first)
                                    return ComparisonEnum::EQUAL;
                                if (id > currAlreadyMatchedSentences.first)
                                    return ComparisonEnum::MORE;
                                return ComparisonEnum::LESS;
                            });
                        if (memoryGrdExpOffsetPtr != nullptr) {
                            const unsigned char* memoryGrdExpPtr = beginOfSemExpPtr + *memoryGrdExpOffsetPtr;
                            auto* memorySentencePtr =
                                SemanticMemoryBlockBinaryReader::memoryGrdExpToMemorySentencePtr(memoryGrdExpPtr);
                            considerAMemoryGrdExp(memoryGrdExpPtr, memorySentencePtr);
                        }
                    }
                }
            }
        } else if (pAlreadyMatchedSentences.empty()) {
            for (auto& currMemoryGrdExp : pLinksToGrdExpFromBinary->grdExpStaticLinks)
                considerAMemoryGrdExp(
                    currMemoryGrdExp.second,
                    SemanticMemoryBlockBinaryReader::memoryGrdExpToMemorySentencePtr(currMemoryGrdExp.second));
        } else if (pLinksToGrdExpFromBinary->grdExpStaticLinks.size() < pAlreadyMatchedSentences.staticLinks.size()) {
            for (auto& currMemoryGrdExp : pLinksToGrdExpFromBinary->grdExpStaticLinks)
                if (pAlreadyMatchedSentences.staticLinks.count(currMemoryGrdExp.first) != 0)
                    considerAMemoryGrdExp(
                        currMemoryGrdExp.second,
                        SemanticMemoryBlockBinaryReader::memoryGrdExpToMemorySentencePtr(currMemoryGrdExp.second));
        } else {
            for (auto& currAlreadyMatchedSentences : pAlreadyMatchedSentences.staticLinks) {
                auto it = pLinksToGrdExpFromBinary->grdExpStaticLinks.find(currAlreadyMatchedSentences.first);
                if (it != pLinksToGrdExpFromBinary->grdExpStaticLinks.end())
                    considerAMemoryGrdExp(it->second,
                                          SemanticMemoryBlockBinaryReader::memoryGrdExpToMemorySentencePtr(it->second));
            }
        }
    }
}

template<bool IS_MODIFIABLE>
bool _addPotentialNewRelationsFromLinks(RelationsThatMatch<IS_MODIFIABLE>& pRelations,
                                        const SentenceLinks<IS_MODIFIABLE>& pAlreadyMatchedSentences,
                                        IntIdToMemSentenceAccessor<IS_MODIFIABLE>* pLinksToGrdExp,
                                        const StaticBinaryLinks* pLinksToGrdExpFromBinary,
                                        const GroundedExpression* pGrdExpToLookFor,
                                        const std::set<const SemanticExpression*>& pChildSemExpsToSkip,
                                        const SemanticMemoryBlockPrivate* pMemBlockPrivatePtr,
                                        bool pIsATrigger,
                                        const linguistics::LinguisticDatabase& pLingDb,
                                        bool pCheckChildren) {
    typedef typename SentenceLinks<IS_MODIFIABLE>::memSentType MEMSENTENCES;
    bool res = false;

    if (pGrdExpToLookFor == nullptr) {
        _executeForLinksAlreadyMatched<IS_MODIFIABLE, MEMSENTENCES>(
            pAlreadyMatchedSentences,
            pLinksToGrdExp,
            pLinksToGrdExpFromBinary,
            pMemBlockPrivatePtr,
            pLingDb,
            [&](MEMSENTENCES* pMemSentToAdd,
                const unsigned char* pStaticMemorySentencePtr,
                const std::function<std::unique_ptr<GroundedExpressionContainer>()>&) {
                if (pMemSentToAdd != nullptr)
                    pRelations.addMemSent(*pMemSentToAdd);
                else
                    pRelations.addMemSentStatic(pStaticMemorySentencePtr, pLingDb);
                res = true;
            });
        return res;
    }

    auto& grdExpToLookFor = *pGrdExpToLookFor;
    auto grdToLookFor = grdExpToLookFor.grounding().type;
    if (grdToLookFor == SemanticGroundingType::STATEMENT && pCheckChildren) {
        _executeForLinksAlreadyMatched<IS_MODIFIABLE, MEMSENTENCES>(
            pAlreadyMatchedSentences,
            pLinksToGrdExp,
            pLinksToGrdExpFromBinary,
            pMemBlockPrivatePtr,
            pLingDb,
            [&](MEMSENTENCES* pMemSentToAdd,
                const unsigned char* pStaticMemorySentencePtr,
                const std::function<std::unique_ptr<GroundedExpressionContainer>()>& pGetGrdExpFromMem) {
                bool hasAChildThatDoesntMatch = false;
                if (pMemBlockPrivatePtr != nullptr) {
                    auto grdExpFromMemContainer = pGetGrdExpFromMem();
                    const auto& grdExpFromMem = grdExpFromMemContainer->getGrdExp();
                    auto& memBlock = pMemBlockPrivatePtr->getMemBlock();
                    for (const auto& currChild : grdExpToLookFor.children) {
                        auto itChildFromMemory = grdExpFromMem.children.find(currChild.first);
                        if (itChildFromMemory != grdExpFromMem.children.end()) {
                            auto compWithOrWithoutInterpretations =
                                _createCompWithOrWithoutInterpretations(!pIsATrigger);
                            if (!SemExpComparator::semExpsAreEqualOrIsContainedFromMemBlock(
                                    *currChild.second,
                                    *itChildFromMemory->second,
                                    memBlock,
                                    pLingDb,
                                    &compWithOrWithoutInterpretations)) {
                                hasAChildThatDoesntMatch = true;
                                break;
                            }
                        }
                    }
                }
                if (!hasAChildThatDoesntMatch) {
                    if (pMemSentToAdd != nullptr)
                        pRelations.addMemSent(*pMemSentToAdd);
                    else
                        pRelations.addMemSentStatic(pStaticMemorySentencePtr, pLingDb);
                    res = true;
                }
            });
        return res;
    }



    const SemanticExpression* itSpecifierToLookForPtr = nullptr;
    const SemanticExpression* itOwnerLookForPtr = nullptr;
    const SemanticExpression* itSubConceptToLookForPtr = nullptr;
    const SemanticExpression* itTimeLookForPtr = nullptr;
    const SemanticExpression* itOtherThanPtr = nullptr;
    bool hasAnChildToCheck = false;
    if (pMemBlockPrivatePtr != nullptr) {
        for (const auto& currChild : grdExpToLookFor.children) {
            switch (currChild.first) {
                case GrammaticalType::SPECIFIER:
                    itSpecifierToLookForPtr = &*currChild.second;
                    hasAnChildToCheck = true;
                    break;
                case GrammaticalType::OWNER:
                    itOwnerLookForPtr = &*currChild.second;
                    hasAnChildToCheck = true;
                    break;
                case GrammaticalType::SUB_CONCEPT:
                    itSubConceptToLookForPtr = &*currChild.second;
                    hasAnChildToCheck = true;
                    break;
                case GrammaticalType::TIME:
                    itTimeLookForPtr = &*currChild.second;
                    hasAnChildToCheck = true;
                    break;
                case GrammaticalType::OTHER_THAN:
                    itOtherThanPtr = &*currChild.second;
                    hasAnChildToCheck = true;
                    break;
                default: break;
            }
        }
    }

    _executeForLinksAlreadyMatched<IS_MODIFIABLE, MEMSENTENCES>(
        pAlreadyMatchedSentences,
        pLinksToGrdExp,
        pLinksToGrdExpFromBinary,
        pMemBlockPrivatePtr,
        pLingDb,
        [&](MEMSENTENCES* pMemSentToAdd,
            const unsigned char* pStaticMemorySentencePtr,
            const std::function<std::unique_ptr<GroundedExpressionContainer>()>& pGetGrdExpFromMem) {
            auto grdExpFromMemContainer = pGetGrdExpFromMem();
            const GroundedExpression& grdExpFromMem = grdExpFromMemContainer->getGrdExp();

            if (grdToLookFor != SemanticGroundingType::STATEMENT) {
                if (!pCheckChildren) {
                    auto itOwnerFromMemory = grdExpFromMem.children.find(GrammaticalType::OWNER);
                    if (itOwnerFromMemory != grdExpFromMem.children.end()) {
                        if (itOwnerLookForPtr == nullptr)
                            return;
                        auto compWithOrWithoutInterpretations = _createCompWithOrWithoutInterpretations(!pIsATrigger);
                        if (pMemBlockPrivatePtr != nullptr) {
                            auto& memBlock = pMemBlockPrivatePtr->getMemBlock();
                            if (!SemExpComparator::semExpsAreEqualOrIsContainedFromMemBlock(
                                        *itOwnerLookForPtr,
                                        *itOwnerFromMemory->second,
                                        memBlock,
                                        pLingDb,
                                        &compWithOrWithoutInterpretations))
                                return;
                        }
                    }

                    auto itSubConceptFromMemory = grdExpFromMem.children.find(GrammaticalType::SUB_CONCEPT);
                    if (itSubConceptFromMemory != grdExpFromMem.children.end()) {
                        if (itSubConceptToLookForPtr == nullptr)
                            return;
                        auto compWithOrWithoutInterpretations = _createCompWithOrWithoutInterpretations(!pIsATrigger);
                        if (pMemBlockPrivatePtr != nullptr) {
                            auto& memBlock = pMemBlockPrivatePtr->getMemBlock();
                            if (!SemExpComparator::semExpsAreEqualOrIsContainedFromMemBlock(
                                        *itSubConceptToLookForPtr,
                                        *itSubConceptFromMemory->second,
                                        memBlock,
                                        pLingDb,
                                        &compWithOrWithoutInterpretations))
                                return;
                        }
                    }

                    auto itSpecifierFromMemory = grdExpFromMem.children.find(GrammaticalType::SPECIFIER);
                    if (itSpecifierFromMemory != grdExpFromMem.children.end()) {
                        if (itSpecifierToLookForPtr == nullptr)
                            return;
                        auto compWithOrWithoutInterpretations = _createCompWithOrWithoutInterpretations(!pIsATrigger);
                        if (pMemBlockPrivatePtr != nullptr) {
                            auto& memBlock = pMemBlockPrivatePtr->getMemBlock();
                            if (!SemExpComparator::semExpsAreEqualOrIsContainedFromMemBlock(
                                        *itSpecifierToLookForPtr,
                                        *itSpecifierFromMemory->second,
                                        memBlock,
                                        pLingDb,
                                        &compWithOrWithoutInterpretations))
                                return;
                        }
                    }
                }
            }

            if (pCheckChildren) {
                if (grdToLookFor == SemanticGroundingType::AGENT
                    && grdExpFromMem.grounding().type != SemanticGroundingType::AGENT
                    && grdExpFromMem.grounding().type != SemanticGroundingType::NAME && !grdExpFromMem.children.empty())
                    return;

                if (pMemBlockPrivatePtr != nullptr && (hasAnChildToCheck || grdExpFromMem.children.count(GrammaticalType::OWNER) > 0)) {
                    auto compWithOrWithoutInterpretations = _createCompWithOrWithoutInterpretations(!pIsATrigger);

                    auto& memBlock = pMemBlockPrivatePtr->getMemBlock();
                    if (!pIsATrigger && itSpecifierToLookForPtr != nullptr
                        && pChildSemExpsToSkip.count(itSpecifierToLookForPtr) == 0) {
                        auto compWithoutStatementGrd = _createCompWithOrWithoutInterpretations(!pIsATrigger);
                        compWithoutStatementGrd.semExps1ToSkip = pChildSemExpsToSkip;    // TODO: not copy
                        auto itSpecifierFromMemory = grdExpFromMem.children.find(GrammaticalType::SPECIFIER);
                        if (itSpecifierFromMemory == grdExpFromMem.children.end()) {
                            auto itOwnerFromMemory = grdExpFromMem.children.find(GrammaticalType::OWNER);
                            if (!SemExpGetter::isAModifier(*itSpecifierToLookForPtr, !pIsATrigger)
                                && (itOwnerFromMemory == grdExpFromMem.children.end()
                                    || !SemExpComparator::semExpsAreEqualOrIsContainedFromMemBlock(
                                        *itSpecifierToLookForPtr,
                                        *itOwnerFromMemory->second,
                                        memBlock,
                                        pLingDb,
                                        &compWithoutStatementGrd)))
                                return;
                        } else if (!SemExpComparator::semExpsAreEqualOrIsContainedFromMemBlock(
                                       *itSpecifierToLookForPtr,
                                       *itSpecifierFromMemory->second,
                                       memBlock,
                                       pLingDb,
                                       &compWithoutStatementGrd)) {
                            return;
                        }
                    }

                    if (!pIsATrigger && itOwnerLookForPtr != nullptr) {
                        auto itOwnerFromMemory = grdExpFromMem.children.find(GrammaticalType::OWNER);
                        if (itOwnerFromMemory == grdExpFromMem.children.end()) {
                            auto itSpecifierFromMemory = grdExpFromMem.children.find(GrammaticalType::SPECIFIER);
                            if (itSpecifierFromMemory == grdExpFromMem.children.end()
                                || !SemExpComparator::semExpsAreEqualOrIsContainedFromMemBlock(
                                    *itOwnerLookForPtr,
                                    *itSpecifierFromMemory->second,
                                    memBlock,
                                    pLingDb,
                                    &compWithOrWithoutInterpretations))
                                return;
                        } else if (SemExpComparator::getSemExpsImbrications(*itOwnerLookForPtr,
                                                                            *itOwnerFromMemory->second,
                                                                            memBlock,
                                                                            pLingDb,
                                                                            &compWithOrWithoutInterpretations)
                                   != ImbricationType::EQUALS) {
                            return;
                        }
                    }

                    if (itSubConceptToLookForPtr != nullptr) {
                        auto itSpecifierFromMemory = grdExpFromMem.children.find(GrammaticalType::SUB_CONCEPT);
                        if (itSpecifierFromMemory == grdExpFromMem.children.end()
                            || !SemExpComparator::semExpsAreEqualOrIsContainedFromMemBlock(
                                *itSubConceptToLookForPtr,
                                *itSpecifierFromMemory->second,
                                memBlock,
                                pLingDb,
                                &compWithOrWithoutInterpretations))
                            return;
                    }

                    if (itTimeLookForPtr != nullptr) {
                        auto itTimeFromMemory = grdExpFromMem.children.find(GrammaticalType::TIME);
                        if (itTimeFromMemory == grdExpFromMem.children.end()
                            || !SemExpComparator::semExpsAreEqualOrIsContainedFromMemBlock(
                                *itTimeLookForPtr,
                                *itTimeFromMemory->second,
                                memBlock,
                                pLingDb,
                                &compWithOrWithoutInterpretations))
                            return;
                    }

                    if (itOtherThanPtr != nullptr
                        && SemExpComparator::semExpsAreEqualOrIsContainedFromMemBlock(
                            *itOtherThanPtr, grdExpFromMem, memBlock, pLingDb, &compWithOrWithoutInterpretations))
                        return;
                }
            }

            if (pMemSentToAdd != nullptr)
                pRelations.addMemSent(*pMemSentToAdd);
            else
                pRelations.addMemSentStatic(pStaticMemorySentencePtr, pLingDb);
            res = true;
        });
    return res;
}

template<bool IS_MODIFIABLE>
bool _getRelationsOfAMeaning(RelationsThatMatch<IS_MODIFIABLE>& pRelations,
                             const SentenceLinks<IS_MODIFIABLE>& pAlreadyMatchedSentences,
                             MemoryLinksAccessor<IS_MODIFIABLE>& pLinksToSemExps,
                             SemanticLanguageEnum pLanguage,
                             int pMeaningId,
                             const GroundedExpression& pGrdExpToLookFor,
                             const std::set<const SemanticExpression*>& pChildSemExpsToSkip,
                             const SemanticMemoryBlockPrivate* pMemBlockPrivatePtr,
                             bool pIsATrigger,
                             const linguistics::LinguisticDatabase& pLingDb,
                             bool pCheckChildren) {
    bool res = false;
    if (pLinksToSemExps.d != nullptr) {
        auto itMeaningToSemExp = pLinksToSemExps.d->meaningsToSemExps.find(pLanguage);
        if (itMeaningToSemExp != pLinksToSemExps.d->meaningsToSemExps.end()) {
            auto itSemExpList = itMeaningToSemExp->second.find(pMeaningId);
            if (itSemExpList != itMeaningToSemExp->second.end()) {
                IntIdToMemSentenceAccessor<IS_MODIFIABLE> accessor(itSemExpList->second);
                res = _addPotentialNewRelationsFromLinks(pRelations,
                                                         pAlreadyMatchedSentences,
                                                         &accessor,
                                                         nullptr,
                                                         &pGrdExpToLookFor,
                                                         pChildSemExpsToSkip,
                                                         pMemBlockPrivatePtr,
                                                         pIsATrigger,
                                                         pLingDb,
                                                         pCheckChildren)
                   || res;
            }
        }
    }
    if (pLinksToSemExps.c != nullptr) {
        auto* meaningsPtr = SemanticMemoryBlockBinaryReader::moveToMeaningsPtr(pLinksToSemExps.c);
        const unsigned char* languageNodePtr =
            readEnumMap(meaningsPtr, semanticLanguageEnum_toChar(pLanguage), semanticLanguageEnum_size);
        if (languageNodePtr != nullptr) {
            StaticBinaryLinks meaningLinks(readIntMap(languageNodePtr, pMeaningId));
            if (meaningLinks.linksToGrdExp != nullptr)
                res = _addPotentialNewRelationsFromLinks<IS_MODIFIABLE>(pRelations,
                                                                        pAlreadyMatchedSentences,
                                                                        nullptr,
                                                                        &meaningLinks,
                                                                        &pGrdExpToLookFor,
                                                                        pChildSemExpsToSkip,
                                                                        pMemBlockPrivatePtr,
                                                                        pIsATrigger,
                                                                        pLingDb,
                                                                        pCheckChildren)
                   || res;
        }
    }
    return res;
}

bool _hasStaticLinks(const unsigned char* pPtr) {
    return pPtr != nullptr && binaryloader::loadIntInThreeBytes(pPtr) != 0x00FFFFFF;
}

template<bool IS_MODIFIABLE>
bool _getNumberRelations(RelationsThatMatch<IS_MODIFIABLE>& pRelations,
                         const SentenceLinks<IS_MODIFIABLE>& pAlreadyMatchedSentences,
                         MemoryLinksAccessor<IS_MODIFIABLE>& pLinksToSemExps,
                         const SemanticFloat& pNumber,
                         const GroundedExpression& pGrdExpToLookFor,
                         const std::set<const SemanticExpression*>& pChildSemExpsToSkip,
                         const SemanticMemoryBlockPrivate* pMemBlockPrivatePtr,
                         bool pIsATrigger,
                         const linguistics::LinguisticDatabase& pLingDb,
                         bool pCheckChildren) {
    bool res = false;
    if (pLinksToSemExps.d != nullptr) {
        auto itNbToSemExp = pLinksToSemExps.d->numberToSemExps.find(pNumber);
        if (itNbToSemExp != pLinksToSemExps.d->numberToSemExps.end()) {
            IntIdToMemSentenceAccessor<IS_MODIFIABLE> accessor(itNbToSemExp->second);
            res = _addPotentialNewRelationsFromLinks(pRelations,
                                                     pAlreadyMatchedSentences,
                                                     &accessor,
                                                     nullptr,
                                                     &pGrdExpToLookFor,
                                                     pChildSemExpsToSkip,
                                                     pMemBlockPrivatePtr,
                                                     pIsATrigger,
                                                     pLingDb,
                                                     pCheckChildren)
               || res;
        }
    }
    /*
    if (pLinksToSemExps.c != nullptr)
    {
      // TODO: add binary memory search
    }
    */
    return res;
}

template<bool IS_MODIFIABLE>
bool _getQuantityTypesRelations(RelationsThatMatch<IS_MODIFIABLE>& pRelations,
                                const SentenceLinks<IS_MODIFIABLE>& pAlreadyMatchedSentences,
                                MemoryLinksAccessor<IS_MODIFIABLE>& pLinksToSemExps,
                                SemanticQuantityType pQuantityType,
                                const GroundedExpression& pGrdExpToLookFor,
                                const std::set<const SemanticExpression*>& pChildSemExpsToSkip,
                                const SemanticMemoryBlockPrivate* pMemBlockPrivatePtr,
                                bool pIsATrigger,
                                const linguistics::LinguisticDatabase& pLingDb,
                                bool pCheckChildren) {
    bool res = false;
    if (pLinksToSemExps.d != nullptr) {
        auto itToSemExp = pLinksToSemExps.d->quantityTypeToSemExps.find(pQuantityType);
        if (itToSemExp != pLinksToSemExps.d->quantityTypeToSemExps.end()) {
            IntIdToMemSentenceAccessor<IS_MODIFIABLE> accessor(itToSemExp->second);
            res = _addPotentialNewRelationsFromLinks(pRelations,
                                                     pAlreadyMatchedSentences,
                                                     &accessor,
                                                     nullptr,
                                                     &pGrdExpToLookFor,
                                                     pChildSemExpsToSkip,
                                                     pMemBlockPrivatePtr,
                                                     pIsATrigger,
                                                     pLingDb,
                                                     pCheckChildren)
               || res;
        }
    }
    /*
    if (pLinksToSemExps.c != nullptr)
    {
      // TODO: add binary memory search
    }
    */
    return res;
}

template<bool IS_MODIFIABLE>
bool _getReferenceWithoutConceptRelations(RelationsThatMatch<IS_MODIFIABLE>& pRelations,
                                          const SentenceLinks<IS_MODIFIABLE>& pAlreadyMatchedSentences,
                                          MemoryLinksAccessor<IS_MODIFIABLE>& pLinksToSemExps,
                                          SemanticReferenceType pReferenceType,
                                          const GroundedExpression& pGrdExpToLookFor,
                                          const std::set<const SemanticExpression*>& pChildSemExpsToSkip,
                                          const SemanticMemoryBlockPrivate* pMemBlockPrivatePtr,
                                          bool pIsATrigger,
                                          const linguistics::LinguisticDatabase& pLingDb,
                                          bool pCheckChildren) {
    bool res = false;
    if (pLinksToSemExps.d != nullptr) {
        auto itToSemExp = pLinksToSemExps.d->referenceWithoutConceptToSemExps.find(pReferenceType);
        if (itToSemExp != pLinksToSemExps.d->referenceWithoutConceptToSemExps.end()) {
            IntIdToMemSentenceAccessor<IS_MODIFIABLE> accessor(itToSemExp->second);
            res = _addPotentialNewRelationsFromLinks(pRelations,
                                                     pAlreadyMatchedSentences,
                                                     &accessor,
                                                     nullptr,
                                                     &pGrdExpToLookFor,
                                                     pChildSemExpsToSkip,
                                                     pMemBlockPrivatePtr,
                                                     pIsATrigger,
                                                     pLingDb,
                                                     pCheckChildren)
               || res;
        }
    }
    /*
    if (pLinksToSemExps.c != nullptr)
    {
      // TODO: add binary memory search
    }
    */
    return res;
}

template<bool IS_MODIFIABLE>
bool _getCoreferenceWithoutConceptOrReferenceRelations(RelationsThatMatch<IS_MODIFIABLE>& pRelations,
                                                       const SentenceLinks<IS_MODIFIABLE>& pAlreadyMatchedSentences,
                                                       MemoryLinksAccessor<IS_MODIFIABLE>& pLinksToSemExps,
                                                       CoreferenceDirectionEnum pCoreferenceDirection,
                                                       const GroundedExpression& pGrdExpToLookFor,
                                                       const std::set<const SemanticExpression*>& pChildSemExpsToSkip,
                                                       const SemanticMemoryBlockPrivate* pMemBlockPrivatePtr,
                                                       bool pIsATrigger,
                                                       const linguistics::LinguisticDatabase& pLingDb,
                                                       bool pCheckChildren) {
    bool res = false;
    if (pLinksToSemExps.d != nullptr) {
        auto itToSemExp = pLinksToSemExps.d->coreferenceWithoutConceptOrReferenceToSemExps.find(pCoreferenceDirection);
        if (itToSemExp != pLinksToSemExps.d->coreferenceWithoutConceptOrReferenceToSemExps.end()) {
            IntIdToMemSentenceAccessor<IS_MODIFIABLE> accessor(itToSemExp->second);
            res = _addPotentialNewRelationsFromLinks(pRelations,
                                                     pAlreadyMatchedSentences,
                                                     &accessor,
                                                     nullptr,
                                                     &pGrdExpToLookFor,
                                                     pChildSemExpsToSkip,
                                                     pMemBlockPrivatePtr,
                                                     pIsATrigger,
                                                     pLingDb,
                                                     pCheckChildren)
               || res;
        }
    }
    /*
    if (pLinksToSemExps.c != nullptr)
    {
      // TODO: add binary memory search
    }
    */
    return res;
}

template<bool IS_MODIFIABLE>
bool _getStatementCoreferenceWithoutConceptRelations(RelationsThatMatch<IS_MODIFIABLE>& pRelations,
                                                     const SentenceLinks<IS_MODIFIABLE>& pAlreadyMatchedSentences,
                                                     MemoryLinksAccessor<IS_MODIFIABLE>& pLinksToSemExps,
                                                     CoreferenceDirectionEnum pCoreferenceDirection,
                                                     const GroundedExpression& pGrdExpToLookFor,
                                                     const std::set<const SemanticExpression*>& pChildSemExpsToSkip,
                                                     const SemanticMemoryBlockPrivate* pMemBlockPrivatePtr,
                                                     bool pIsATrigger,
                                                     const linguistics::LinguisticDatabase& pLingDb,
                                                     bool pCheckChildren) {
    bool res = false;
    if (pLinksToSemExps.d != nullptr) {
        auto itToSemExp = pLinksToSemExps.d->statementCoreferenceWithoutConceptToSemExps.find(pCoreferenceDirection);
        if (itToSemExp != pLinksToSemExps.d->statementCoreferenceWithoutConceptToSemExps.end()) {
            IntIdToMemSentenceAccessor<IS_MODIFIABLE> accessor(itToSemExp->second);
            res = _addPotentialNewRelationsFromLinks(pRelations,
                                                     pAlreadyMatchedSentences,
                                                     &accessor,
                                                     nullptr,
                                                     &pGrdExpToLookFor,
                                                     pChildSemExpsToSkip,
                                                     pMemBlockPrivatePtr,
                                                     pIsATrigger,
                                                     pLingDb,
                                                     pCheckChildren)
               || res;
        }
    }
    /*
    if (pLinksToSemExps.c != nullptr)
    {
      // TODO: add binary memory search
    }
    */
    return res;
}

template<bool IS_MODIFIABLE>
bool _getTextRelations(RelationsThatMatch<IS_MODIFIABLE>& pRelations,
                       const SentenceLinks<IS_MODIFIABLE>& pAlreadyMatchedSentences,
                       MemoryLinksAccessor<IS_MODIFIABLE>& pLinksToSemExps,
                       const std::string& pText,
                       SemanticLanguageEnum pForLanguage,
                       const GroundedExpression& pGrdExpToLookFor,
                       const std::set<const SemanticExpression*>& pChildSemExpsToSkip,
                       const SemanticMemoryBlockPrivate* pMemBlockPrivatePtr,
                       bool pIsATrigger,
                       const linguistics::LinguisticDatabase& pLingDb,
                       bool pCheckChildren) {
    bool res = false;
    if (pLinksToSemExps.d != nullptr) {
        auto itGoodLanguageText = pLinksToSemExps.d->textToSemExps.find(pForLanguage);
        if (itGoodLanguageText != pLinksToSemExps.d->textToSemExps.end()) {
            auto* textToSemExpPtr = itGoodLanguageText->second.find_ptr(pText);
            if (textToSemExpPtr != nullptr) {
                IntIdToMemSentenceAccessor<IS_MODIFIABLE> accessor(*textToSemExpPtr);
                res = _addPotentialNewRelationsFromLinks(pRelations,
                                                         pAlreadyMatchedSentences,
                                                         &accessor,
                                                         nullptr,
                                                         &pGrdExpToLookFor,
                                                         pChildSemExpsToSkip,
                                                         pMemBlockPrivatePtr,
                                                         pIsATrigger,
                                                         pLingDb,
                                                         pCheckChildren)
                   || res;
            }
        }
    }
    if (pLinksToSemExps.c != nullptr) {
        auto* textsPtr = SemanticMemoryBlockBinaryReader::moveToTextPtr(pLinksToSemExps.c);
        const unsigned char* languageNodePtr =
            readEnumMap(textsPtr, semanticLanguageEnum_toChar(pForLanguage), semanticLanguageEnum_size);
        if (languageNodePtr != nullptr) {
            StaticBinaryLinks nodeLinks(radixmap::read(languageNodePtr, pText, _hasStaticLinks));
            if (nodeLinks.linksToGrdExp != nullptr)
                res = _addPotentialNewRelationsFromLinks<IS_MODIFIABLE>(pRelations,
                                                                        pAlreadyMatchedSentences,
                                                                        nullptr,
                                                                        &nodeLinks,
                                                                        &pGrdExpToLookFor,
                                                                        pChildSemExpsToSkip,
                                                                        pMemBlockPrivatePtr,
                                                                        pIsATrigger,
                                                                        pLingDb,
                                                                        pCheckChildren)
                   || res;
        }
    }
    return res;
}

template<bool IS_MODIFIABLE>
bool _specificUserIdToRelationsFromMemory(RelationsThatMatch<IS_MODIFIABLE>& pRelations,
                                          const SentenceLinks<IS_MODIFIABLE>& pAlreadyMatchedSentences,
                                          MemoryLinksAccessor<IS_MODIFIABLE>& pLinksToSemExps,
                                          const GroundedExpression& pGrdExpToLookFor,
                                          const std::set<const SemanticExpression*>& pChildSemExpsToSkip,
                                          const std::string& pUserId,
                                          const SemanticMemoryBlockPrivate* pMemBlockPrivatePtr,
                                          bool pIsATrigger,
                                          const linguistics::LinguisticDatabase& pLingDb,
                                          bool pCheckChildren) {
    bool res = false;
    if (pLinksToSemExps.d != nullptr) {
        auto* semExpListPtr = pLinksToSemExps.d->userIdToSemExps.find_ptr(pUserId);
        if (semExpListPtr != nullptr) {
            IntIdToMemSentenceAccessor<IS_MODIFIABLE> accessor(*semExpListPtr);
            res = _addPotentialNewRelationsFromLinks(pRelations,
                                                     pAlreadyMatchedSentences,
                                                     &accessor,
                                                     nullptr,
                                                     &pGrdExpToLookFor,
                                                     pChildSemExpsToSkip,
                                                     pMemBlockPrivatePtr,
                                                     pIsATrigger,
                                                     pLingDb,
                                                     pCheckChildren)
               || res;
        }
    }
    if (pLinksToSemExps.c != nullptr) {
        const unsigned char* userIdPtr = SemanticMemoryBlockBinaryReader::moveToUserIdPtr(pLinksToSemExps.c);
        StaticBinaryLinks nodeLinks(radixmap::read(userIdPtr, pUserId, _hasStaticLinks));
        if (nodeLinks.linksToGrdExp != nullptr)
            res = _addPotentialNewRelationsFromLinks<IS_MODIFIABLE>(pRelations,
                                                                    pAlreadyMatchedSentences,
                                                                    nullptr,
                                                                    &nodeLinks,
                                                                    &pGrdExpToLookFor,
                                                                    pChildSemExpsToSkip,
                                                                    pMemBlockPrivatePtr,
                                                                    pIsATrigger,
                                                                    pLingDb,
                                                                    pCheckChildren)
               || res;
    }
    return res;
}

template<bool IS_MODIFIABLE>
bool _userIdWithoutContextToRelationsFromMemory(RelationsThatMatch<IS_MODIFIABLE>& pRelations,
                                                const SentenceLinks<IS_MODIFIABLE>& pAlreadyMatchedSentences,
                                                MemoryLinksAccessor<IS_MODIFIABLE>& pLinksToSemExps,
                                                const GroundedExpression& pGrdExpToLookFor,
                                                const std::set<const SemanticExpression*>& pChildSemExpsToSkip,
                                                const std::string& pUserIdWithoutontext,
                                                const SemanticMemoryBlockPrivate* pMemBlockPrivatePtr,
                                                bool pIsATrigger,
                                                const linguistics::LinguisticDatabase& pLingDb,
                                                bool pCheckChildren) {
    bool res = false;
    if (pLinksToSemExps.d != nullptr) {
        auto* semExpListPtr = pLinksToSemExps.d->userIdWithoutContextToSemExps.find_ptr(pUserIdWithoutontext);
        if (semExpListPtr != nullptr) {
            IntIdToMemSentenceAccessor<IS_MODIFIABLE> accessor(*semExpListPtr);
            res = _addPotentialNewRelationsFromLinks(pRelations,
                                                     pAlreadyMatchedSentences,
                                                     &accessor,
                                                     nullptr,
                                                     &pGrdExpToLookFor,
                                                     pChildSemExpsToSkip,
                                                     pMemBlockPrivatePtr,
                                                     pIsATrigger,
                                                     pLingDb,
                                                     pCheckChildren)
               || res;
        }
    }
    /*
    if (pLinksToSemExps.c != nullptr)
    {
    // TODO: add binary memory search
    }
    */
    return res;
}

template<bool IS_MODIFIABLE>
bool _oneUserIdToRelationsFromMemory(RelationsThatMatch<IS_MODIFIABLE>& pRelations,
                                     const SentenceLinks<IS_MODIFIABLE>& pAlreadyMatchedSentences,
                                     MemoryLinksAccessor<IS_MODIFIABLE>& pLinksToSemExps,
                                     const GroundedExpression& pGrdExpToLookFor,
                                     const std::set<const SemanticExpression*>& pChildSemExpsToSkip,
                                     const std::string& pUserId,
                                     const SemanticMemoryBlockPrivate* pMemBlockPrivatePtr,
                                     bool pIsATrigger,
                                     const linguistics::LinguisticDatabase& pLingDb,
                                     bool pCheckChildren) {
    if (pMemBlockPrivatePtr != nullptr) {
        std::list<std::string> userIds;
        pMemBlockPrivatePtr->getMemBlock().getAllEquivalentUserIds(userIds, pUserId);
        bool res = false;
        for (const auto& currUserId : userIds)
            res = _specificUserIdToRelationsFromMemory(pRelations,
                                                       pAlreadyMatchedSentences,
                                                       pLinksToSemExps,
                                                       pGrdExpToLookFor,
                                                       pChildSemExpsToSkip,
                                                       currUserId,
                                                       pMemBlockPrivatePtr,
                                                       pIsATrigger,
                                                       pLingDb,
                                                       pCheckChildren)
               || res;
        return res;
    }
    return _specificUserIdToRelationsFromMemory(pRelations,
                                                pAlreadyMatchedSentences,
                                                pLinksToSemExps,
                                                pGrdExpToLookFor,
                                                pChildSemExpsToSkip,
                                                pUserId,
                                                pMemBlockPrivatePtr,
                                                pIsATrigger,
                                                pLingDb,
                                                pCheckChildren);
}

template<bool IS_MODIFIABLE>
bool _oneConceptToRelationsFromMemory(RelationsThatMatch<IS_MODIFIABLE>& pRelations,
                                      const SentenceLinks<IS_MODIFIABLE>& pAlreadyMatchedSentences,
                                      MemoryLinksAccessor<IS_MODIFIABLE>& pLinksToSemExps,
                                      const GroundedExpression* pGrdExpToLookFor,
                                      const std::set<const SemanticExpression*>& pChildSemExpsToSkip,
                                      const std::string& pConceptName,
                                      const SemanticMemoryBlockPrivate* pMemBlockPrivatePtr,
                                      bool pIsATrigger,
                                      const linguistics::LinguisticDatabase& pLingDb,
                                      bool pCheckChildren) {
    bool res = false;
    if (pLinksToSemExps.d != nullptr) {
        auto* semExpListPtr = pLinksToSemExps.d->conceptsToSemExps.find_ptr(pConceptName);
        if (semExpListPtr != nullptr) {
            IntIdToMemSentenceAccessor<IS_MODIFIABLE> accessor(*semExpListPtr);
            res = _addPotentialNewRelationsFromLinks(pRelations,
                                                     pAlreadyMatchedSentences,
                                                     &accessor,
                                                     nullptr,
                                                     pGrdExpToLookFor,
                                                     pChildSemExpsToSkip,
                                                     pMemBlockPrivatePtr,
                                                     pIsATrigger,
                                                     pLingDb,
                                                     pCheckChildren)
               || res;
        }
    }
    if (pLinksToSemExps.c != nullptr) {
        const unsigned char* cptsPtr = SemanticMemoryBlockBinaryReader::moveToConceptsPtr(pLinksToSemExps.c);
        StaticBinaryLinks cptLinks(radixmap::read(cptsPtr, pConceptName, _hasStaticLinks));
        if (cptLinks.linksToGrdExp != nullptr)
            res = _addPotentialNewRelationsFromLinks<IS_MODIFIABLE>(pRelations,
                                                                    pAlreadyMatchedSentences,
                                                                    nullptr,
                                                                    &cptLinks,
                                                                    pGrdExpToLookFor,
                                                                    pChildSemExpsToSkip,
                                                                    pMemBlockPrivatePtr,
                                                                    pIsATrigger,
                                                                    pLingDb,
                                                                    pCheckChildren)
               || res;
    }
    return res;
}

template<bool IS_MODIFIABLE>
bool _conceptsToRelationsFromMemory(RelationsThatMatch<IS_MODIFIABLE>& pRelations,
                                    const SentenceLinks<IS_MODIFIABLE>& pAlreadyMatchedSentences,
                                    MemoryLinksAccessor<IS_MODIFIABLE>& pLinksToSemExps,
                                    const std::map<std::string, char>& pConcepts,
                                    const GroundedExpression* pGrdExpToLookFor,
                                    const std::set<const SemanticExpression*>& pChildSemExpsToSkip,
                                    OtherConceptsLinkStrategy pOtherConceptsLinkStrategy,
                                    const SemanticMemoryBlockPrivate* pMemBlockPrivatePtr,
                                    bool pIsATrigger,
                                    const linguistics::LinguisticDatabase& pLingDb,
                                    bool pCheckChildren) {
    bool res = false;
    for (const auto& currConcept : pConcepts) {
        const std::string& conceptName = currConcept.first;

        switch (pOtherConceptsLinkStrategy) {
            case OtherConceptsLinkStrategy::LINK_WITH_CHILDREN_CONCEPTS: {
                if (conceptName == "sentiment")
                    break;
                std::vector<std::string> subConcepts;
                pLingDb.conceptSet.conceptToChildConcepts(subConcepts, conceptName);
                for (const auto& currSubConcept : subConcepts) {
                    res = _oneConceptToRelationsFromMemory(pRelations,
                                                           pAlreadyMatchedSentences,
                                                           pLinksToSemExps,
                                                           pGrdExpToLookFor,
                                                           pChildSemExpsToSkip,
                                                           currSubConcept,
                                                           pMemBlockPrivatePtr,
                                                           pIsATrigger,
                                                           pLingDb,
                                                           pCheckChildren)
                       || res;
                }
                break;
            }
            case OtherConceptsLinkStrategy::LINK_WITH_MOTHER_CONCEPTS: {
                std::vector<std::string> parentConcepts;
                ConceptSet::conceptToParentConcepts(parentConcepts, conceptName);
                for (const auto& currParentConcept : parentConcepts) {
                    res = _oneConceptToRelationsFromMemory(pRelations,
                                                           pAlreadyMatchedSentences,
                                                           pLinksToSemExps,
                                                           pGrdExpToLookFor,
                                                           pChildSemExpsToSkip,
                                                           currParentConcept,
                                                           pMemBlockPrivatePtr,
                                                           pIsATrigger,
                                                           pLingDb,
                                                           pCheckChildren)
                       || res;
                }
                break;
            }
            case OtherConceptsLinkStrategy::NO_LINK_TO_OTHER_CONCEPTS: break;
        }

        {
            std::vector<std::string> nearlyEqualConcepts;
            pLingDb.conceptSet.conceptToNearlyEqualConcepts(nearlyEqualConcepts, conceptName);
            for (const auto& currNearlyEqualConcept : nearlyEqualConcepts)
                if (!ConceptSet::isAConceptAny(currNearlyEqualConcept))
                    res = _oneConceptToRelationsFromMemory(pRelations,
                                                           pAlreadyMatchedSentences,
                                                           pLinksToSemExps,
                                                           pGrdExpToLookFor,
                                                           pChildSemExpsToSkip,
                                                           currNearlyEqualConcept,
                                                           pMemBlockPrivatePtr,
                                                           pIsATrigger,
                                                           pLingDb,
                                                           pCheckChildren)
                       || res;
        }

        if (!ConceptSet::isAConceptAny(conceptName)) {
            res = _oneConceptToRelationsFromMemory(pRelations,
                                                   pAlreadyMatchedSentences,
                                                   pLinksToSemExps,
                                                   pGrdExpToLookFor,
                                                   pChildSemExpsToSkip,
                                                   conceptName,
                                                   pMemBlockPrivatePtr,
                                                   pIsATrigger,
                                                   pLingDb,
                                                   pCheckChildren)
               || res;
        }

        std::set<std::string> oppositeConcepts;
        pLingDb.conceptSet.getOppositeConcepts(oppositeConcepts, conceptName);
        for (const auto& oppCpt : oppositeConcepts) {
            res = _oneConceptToRelationsFromMemory(pRelations,
                                                   pAlreadyMatchedSentences,
                                                   pLinksToSemExps,
                                                   pGrdExpToLookFor,
                                                   pChildSemExpsToSkip,
                                                   oppCpt,
                                                   pMemBlockPrivatePtr,
                                                   pIsATrigger,
                                                   pLingDb,
                                                   pCheckChildren)
               || res;
        }
    }
    return res;
}

template<bool IS_MODIFIABLE>
bool _entityCheckMatching(RelationsThatMatch<IS_MODIFIABLE>& pRelations,
                          const SentenceLinks<IS_MODIFIABLE>& pAlreadyMatchedSentences,
                          EntityTypeToGrdExpAccessor<IS_MODIFIABLE>* pEntityToSemExps,
                          const unsigned char* pEntityToSemExpsStaticPtr,
                          SemanticEntityType pEntityType,
                          const GroundedExpression& pGrdExpToLookFor,
                          const std::set<const SemanticExpression*>& pChildSemExpsToSkip,
                          const SemanticMemoryBlockPrivate* pMemBlockPrivatePtr,
                          bool pIsATrigger,
                          const linguistics::LinguisticDatabase& pLingDb,
                          bool pCheckChildren) {
    bool res = false;
    if (pEntityType != SemanticEntityType::UNKNOWN) {
        if (pEntityType == SemanticEntityType::HUMAN || pEntityType == SemanticEntityType::AGENTORTHING) {
            if (pEntityToSemExps != nullptr) {
                auto itAnyAgent = pEntityToSemExps->m.find(SemanticEntityType::HUMAN);
                if (itAnyAgent != pEntityToSemExps->m.end()) {
                    IntIdToMemSentenceAccessor<IS_MODIFIABLE> accessor(itAnyAgent->second);
                    res = _addPotentialNewRelationsFromLinks(pRelations,
                                                             pAlreadyMatchedSentences,
                                                             &accessor,
                                                             nullptr,
                                                             &pGrdExpToLookFor,
                                                             pChildSemExpsToSkip,
                                                             pMemBlockPrivatePtr,
                                                             pIsATrigger,
                                                             pLingDb,
                                                             pCheckChildren)
                       || res;
                }
            }
            if (pEntityToSemExpsStaticPtr != nullptr) {
                StaticBinaryLinks nodeLinks(readEnumMap(pEntityToSemExpsStaticPtr,
                                                        semanticEntityType_toChar(SemanticEntityType::HUMAN),
                                                        semanticEntityType_size));
                if (nodeLinks.linksToGrdExp != nullptr)
                    res = _addPotentialNewRelationsFromLinks<IS_MODIFIABLE>(pRelations,
                                                                            pAlreadyMatchedSentences,
                                                                            nullptr,
                                                                            &nodeLinks,
                                                                            &pGrdExpToLookFor,
                                                                            pChildSemExpsToSkip,
                                                                            pMemBlockPrivatePtr,
                                                                            pIsATrigger,
                                                                            pLingDb,
                                                                            pCheckChildren)
                       || res;
            }
        }
        if (pEntityType == SemanticEntityType::THING || pEntityType == SemanticEntityType::AGENTORTHING) {
            if (pEntityToSemExps != nullptr) {
                auto itAnyThing = pEntityToSemExps->m.find(SemanticEntityType::THING);
                if (itAnyThing != pEntityToSemExps->m.end()) {
                    IntIdToMemSentenceAccessor<IS_MODIFIABLE> accessor(itAnyThing->second);
                    res = _addPotentialNewRelationsFromLinks(pRelations,
                                                             pAlreadyMatchedSentences,
                                                             &accessor,
                                                             nullptr,
                                                             &pGrdExpToLookFor,
                                                             pChildSemExpsToSkip,
                                                             pMemBlockPrivatePtr,
                                                             pIsATrigger,
                                                             pLingDb,
                                                             pCheckChildren)
                       || res;
                }
            }
            if (pEntityToSemExpsStaticPtr != nullptr) {
                StaticBinaryLinks nodeLinks(readEnumMap(pEntityToSemExpsStaticPtr,
                                                        semanticEntityType_toChar(SemanticEntityType::THING),
                                                        semanticEntityType_size));
                if (nodeLinks.linksToGrdExp != nullptr)
                    res = _addPotentialNewRelationsFromLinks<IS_MODIFIABLE>(pRelations,
                                                                            pAlreadyMatchedSentences,
                                                                            nullptr,
                                                                            &nodeLinks,
                                                                            &pGrdExpToLookFor,
                                                                            pChildSemExpsToSkip,
                                                                            pMemBlockPrivatePtr,
                                                                            pIsATrigger,
                                                                            pLingDb,
                                                                            pCheckChildren)
                       || res;
            }
        }
        if (pEntityToSemExps != nullptr) {
            auto itAnyAgentOrThing = pEntityToSemExps->m.find(SemanticEntityType::AGENTORTHING);
            if (itAnyAgentOrThing != pEntityToSemExps->m.end()) {
                IntIdToMemSentenceAccessor<IS_MODIFIABLE> accessor(itAnyAgentOrThing->second);
                res = _addPotentialNewRelationsFromLinks(pRelations,
                                                         pAlreadyMatchedSentences,
                                                         &accessor,
                                                         nullptr,
                                                         &pGrdExpToLookFor,
                                                         pChildSemExpsToSkip,
                                                         pMemBlockPrivatePtr,
                                                         pIsATrigger,
                                                         pLingDb,
                                                         pCheckChildren)
                   || res;
            }
        }
        if (pEntityToSemExpsStaticPtr != nullptr) {
            StaticBinaryLinks nodeLinks(readEnumMap(pEntityToSemExpsStaticPtr,
                                                    semanticEntityType_toChar(SemanticEntityType::AGENTORTHING),
                                                    semanticEntityType_size));
            if (nodeLinks.linksToGrdExp != nullptr)
                res = _addPotentialNewRelationsFromLinks<IS_MODIFIABLE>(pRelations,
                                                                        pAlreadyMatchedSentences,
                                                                        nullptr,
                                                                        &nodeLinks,
                                                                        &pGrdExpToLookFor,
                                                                        pChildSemExpsToSkip,
                                                                        pMemBlockPrivatePtr,
                                                                        pIsATrigger,
                                                                        pLingDb,
                                                                        pCheckChildren)
                   || res;
        }
    }
    return res;
}

template<bool IS_MODIFIABLE>
bool _indefiniteThingsCheckMatching(RelationsThatMatch<IS_MODIFIABLE>& pRelations,
                                    const SentenceLinks<IS_MODIFIABLE>& pAlreadyMatchedSentences,
                                    MemoryLinksAccessor<IS_MODIFIABLE>& pLinksToSemExps,
                                    SemanticEntityType pEntityType,
                                    const GroundedExpression& pGrdExpToLookFor,
                                    const std::set<const SemanticExpression*>& pChildSemExpsToSkip,
                                    RequestContext pRequestContext,
                                    const SemanticMemoryBlockPrivate* pMemBlockPrivatePtr,
                                    bool pIsATrigger,
                                    const linguistics::LinguisticDatabase& pLingDb,
                                    bool pCheckChildren) {
    bool res = false;
    if (pLinksToSemExps.d != nullptr) {
        EntityTypeToGrdExpAccessor<IS_MODIFIABLE> everythingAccessor(
            pLinksToSemExps.d->everythingOrNoEntityTypeToSemExps);
        res = _entityCheckMatching(pRelations,
                                   pAlreadyMatchedSentences,
                                   &everythingAccessor,
                                   nullptr,
                                   pEntityType,
                                   pGrdExpToLookFor,
                                   pChildSemExpsToSkip,
                                   pMemBlockPrivatePtr,
                                   pIsATrigger,
                                   pLingDb,
                                   pCheckChildren)
           || res;
        if (pRequestContext == RequestContext::SENTENCE_TO_CONDITION) {
            EntityTypeToGrdExpAccessor<IS_MODIFIABLE> genGrdAccessor(pLinksToSemExps.d->genGroundingTypeToSemExps);
            res = _entityCheckMatching(pRelations,
                                       pAlreadyMatchedSentences,
                                       &genGrdAccessor,
                                       nullptr,
                                       pEntityType,
                                       pGrdExpToLookFor,
                                       pChildSemExpsToSkip,
                                       pMemBlockPrivatePtr,
                                       pIsATrigger,
                                       pLingDb,
                                       pCheckChildren)
               || res;
        }
    }
    if (pLinksToSemExps.c != nullptr) {
        const unsigned char* evThingPtr =
            SemanticMemoryBlockBinaryReader::moveToEverythingOrNothingEntityPtr(pLinksToSemExps.c);
        res = _entityCheckMatching<IS_MODIFIABLE>(pRelations,
                                                  pAlreadyMatchedSentences,
                                                  nullptr,
                                                  evThingPtr,
                                                  pEntityType,
                                                  pGrdExpToLookFor,
                                                  pChildSemExpsToSkip,
                                                  pMemBlockPrivatePtr,
                                                  pIsATrigger,
                                                  pLingDb,
                                                  pCheckChildren)
           || res;
        if (pRequestContext == RequestContext::SENTENCE_TO_CONDITION) {
            const unsigned char* genGrdThingPtr =
                SemanticMemoryBlockBinaryReader::moveToGenGrdTypePtr(pLinksToSemExps.c);
            res = _entityCheckMatching<IS_MODIFIABLE>(pRelations,
                                                      pAlreadyMatchedSentences,
                                                      nullptr,
                                                      genGrdThingPtr,
                                                      pEntityType,
                                                      pGrdExpToLookFor,
                                                      pChildSemExpsToSkip,
                                                      pMemBlockPrivatePtr,
                                                      pIsATrigger,
                                                      pLingDb,
                                                      pCheckChildren)
               || res;
        }
    }
    return res;
}

template<bool IS_MODIFIABLE>
bool _relationsInLowerCaseFromMemory(RelationsThatMatch<IS_MODIFIABLE>& pRelations,
                                     const SentenceLinks<IS_MODIFIABLE>& pAlreadyMatchedSentences,
                                     MemoryLinksAccessor<IS_MODIFIABLE>& pLinksToSemExps,
                                     const std::string& pLemma,
                                     const GroundedExpression& pGrdExpToLookFor,
                                     const std::set<const SemanticExpression*>& pChildSemExpsToSkip,
                                     RequestContext pRequestContext,
                                     const SemanticMemoryBlockPrivate* pMemBlockPrivatePtr,
                                     bool pIsATrigger,
                                     const linguistics::LinguisticDatabase& pLingDb,
                                     bool pCheckChildren,
                                     SemanticLanguageEnum pLanguage) {
    bool res = false;
    auto wordInLowerCase = pLemma;
    if (lowerCaseText(wordInLowerCase) && !wordInLowerCase.empty()) {
        const auto& lingDico = pLingDb.langToSpec[pLanguage].lingDico;
        auto lowerCaseStaticLingMeaning = lingDico.statDb.getLingMeaning(wordInLowerCase, PartOfSpeech::NOUN, false);

        if (lowerCaseStaticLingMeaning.meaningId != LinguisticMeaning_noMeaningId) {
            auto lowerCaseLingMeaning = LinguisticMeaning(lowerCaseStaticLingMeaning);
            std::map<std::string, char> concepts;
            lingDico.getConcepts(concepts, lowerCaseLingMeaning);
            if (!concepts.empty()) {
                OtherConceptsLinkStrategy otherConceptsLinkStrategy = _requestCategoryToLinkStrategy(pRequestContext);
                // add semantic expressions that have a concept in common
                res = _conceptsToRelationsFromMemory(pRelations,
                                                     pAlreadyMatchedSentences,
                                                     pLinksToSemExps,
                                                     concepts,
                                                     &pGrdExpToLookFor,
                                                     pChildSemExpsToSkip,
                                                     otherConceptsLinkStrategy,
                                                     pMemBlockPrivatePtr,
                                                     pIsATrigger,
                                                     pLingDb,
                                                     pCheckChildren)
                   || res;
            }
        }

        res = _getTextRelations(pRelations,
                                pAlreadyMatchedSentences,
                                pLinksToSemExps,
                                wordInLowerCase,
                                SemanticLanguageEnum::UNKNOWN,
                                pGrdExpToLookFor,
                                pChildSemExpsToSkip,
                                pMemBlockPrivatePtr,
                                pIsATrigger,
                                pLingDb,
                                pCheckChildren)
           || res;
    }
    return res;
}

template<bool IS_MODIFIABLE>
bool _genGroundingToRelationsFromMemory(RelationsThatMatch<IS_MODIFIABLE>& pRelations,
                                        const SentenceLinks<IS_MODIFIABLE>& pAlreadyMatchedSentences,
                                        MemoryLinksAccessor<IS_MODIFIABLE>& pLinksToSemExps,
                                        const SemanticGenericGrounding& pGenGrd,
                                        const GroundedExpression& pGrdExpToLookFor,
                                        const std::set<const SemanticExpression*>& pChildSemExpsToSkip,
                                        RequestContext pRequestContext,
                                        const SemanticMemoryBlockPrivate* pMemBlockPrivatePtr,
                                        bool pIsATrigger,
                                        const linguistics::LinguisticDatabase& pLingDb,
                                        bool pCheckChildren,
                                        SemanticLanguageEnum pLanguage) {
    bool res = false;
    StaticLinguisticMeaning lingMeaning = pLingDb.langToSpec[pGenGrd.word.language].lingDico.statDb.getLingMeaning(
        pGenGrd.word.lemma, pGenGrd.word.partOfSpeech, true);
    if (!lingMeaning.isEmpty()) {
        // add semantic expressions that have a meaning in common
        res = _getRelationsOfAMeaning(pRelations,
                                      pAlreadyMatchedSentences,
                                      pLinksToSemExps,
                                      lingMeaning.language,
                                      lingMeaning.meaningId,
                                      pGrdExpToLookFor,
                                      pChildSemExpsToSkip,
                                      pMemBlockPrivatePtr,
                                      pIsATrigger,
                                      pLingDb,
                                      pCheckChildren)
           || res;
    }

    if (!pGenGrd.word.lemma.empty()) {
        res = _getTextRelations(pRelations,
                                pAlreadyMatchedSentences,
                                pLinksToSemExps,
                                pGenGrd.word.lemma,
                                SemanticLanguageEnum::UNKNOWN,
                                pGrdExpToLookFor,
                                pChildSemExpsToSkip,
                                pMemBlockPrivatePtr,
                                pIsATrigger,
                                pLingDb,
                                pCheckChildren)
           || res;

        _relationsInLowerCaseFromMemory(pRelations,
                                        pAlreadyMatchedSentences,
                                        pLinksToSemExps,
                                        pGenGrd.word.lemma,
                                        pGrdExpToLookFor,
                                        pChildSemExpsToSkip,
                                        pRequestContext,
                                        pMemBlockPrivatePtr,
                                        pIsATrigger,
                                        pLingDb,
                                        pCheckChildren,
                                        pLanguage);
    }

    if (!pGenGrd.concepts.empty()) {
        OtherConceptsLinkStrategy otherConceptsLinkStrategy = _requestCategoryToLinkStrategy(pRequestContext);
        if (pIsATrigger)
          otherConceptsLinkStrategy = OtherConceptsLinkStrategy::LINK_WITH_MOTHER_CONCEPTS;
        // add semantic expressions that have a concept in common
        res = _conceptsToRelationsFromMemory(pRelations,
                                             pAlreadyMatchedSentences,
                                             pLinksToSemExps,
                                             pGenGrd.concepts,
                                             &pGrdExpToLookFor,
                                             pChildSemExpsToSkip,
                                             otherConceptsLinkStrategy,
                                             pMemBlockPrivatePtr,
                                             pIsATrigger,
                                             pLingDb,
                                             pCheckChildren)
           || res;
    }

    if (pGenGrd.word.lemma.empty() && !ConceptSet::haveAConceptNotAny(pGenGrd.concepts)) {
        if (pGenGrd.quantity.type == SemanticQuantityType::NUMBER) {
            if (pGenGrd.word.lemma.empty() && pGenGrd.concepts.empty())
                res = _getNumberRelations(pRelations,
                                          pAlreadyMatchedSentences,
                                          pLinksToSemExps,
                                          pGenGrd.quantity.nb,
                                          pGrdExpToLookFor,
                                          pChildSemExpsToSkip,
                                          pMemBlockPrivatePtr,
                                          pIsATrigger,
                                          pLingDb,
                                          pCheckChildren)
                   || res;
        } else if (pGenGrd.quantity.type != SemanticQuantityType::UNKNOWN) {
            res = _getQuantityTypesRelations(pRelations,
                                             pAlreadyMatchedSentences,
                                             pLinksToSemExps,
                                             pGenGrd.quantity.type,
                                             pGrdExpToLookFor,
                                             pChildSemExpsToSkip,
                                             pMemBlockPrivatePtr,
                                             pIsATrigger,
                                             pLingDb,
                                             pCheckChildren)
               || res;
            if (pGenGrd.quantity.type == SemanticQuantityType::EVERYTHING)
                res = _getQuantityTypesRelations(pRelations,
                                                 pAlreadyMatchedSentences,
                                                 pLinksToSemExps,
                                                 SemanticQuantityType::ANYTHING,
                                                 pGrdExpToLookFor,
                                                 pChildSemExpsToSkip,
                                                 pMemBlockPrivatePtr,
                                                 pIsATrigger,
                                                 pLingDb,
                                                 pCheckChildren)
                   || res;
        } else if (pGenGrd.referenceType != SemanticReferenceType::UNDEFINED) {
            res = _getReferenceWithoutConceptRelations(pRelations,
                                                       pAlreadyMatchedSentences,
                                                       pLinksToSemExps,
                                                       pGenGrd.referenceType,
                                                       pGrdExpToLookFor,
                                                       pChildSemExpsToSkip,
                                                       pMemBlockPrivatePtr,
                                                       pIsATrigger,
                                                       pLingDb,
                                                       pCheckChildren)
               || res;
        } else if (pGenGrd.coreference) {
            _getCoreferenceWithoutConceptOrReferenceRelations(pRelations,
                                                              pAlreadyMatchedSentences,
                                                              pLinksToSemExps,
                                                              pGenGrd.coreference->getDirection(),
                                                              pGrdExpToLookFor,
                                                              pChildSemExpsToSkip,
                                                              pMemBlockPrivatePtr,
                                                              pIsATrigger,
                                                              pLingDb,
                                                              pCheckChildren)
                || res;
        }
    }

    res = _indefiniteThingsCheckMatching(pRelations,
                                         pAlreadyMatchedSentences,
                                         pLinksToSemExps,
                                         pGenGrd.entityType,
                                         pGrdExpToLookFor,
                                         pChildSemExpsToSkip,
                                         pRequestContext,
                                         pMemBlockPrivatePtr,
                                         pIsATrigger,
                                         pLingDb,
                                         pCheckChildren)
       || res;
    return res;
}

template<bool IS_MODIFIABLE>
bool _statementGroundingToRelationsFromMemory(RelationsThatMatch<IS_MODIFIABLE>& pRelations,
                                              const SentenceLinks<IS_MODIFIABLE>& pAlreadyMatchedSentences,
                                              MemoryLinksAccessor<IS_MODIFIABLE>& pLinksToSemExps,
                                              const SemanticStatementGrounding& pStatGrd,
                                              const GroundedExpression& pGrdExpToLookFor,
                                              const std::set<const SemanticExpression*>& pChildSemExpsToSkip,
                                              RequestContext pRequestContext,
                                              const SemanticMemoryBlockPrivate* pMemBlockPrivatePtr,
                                              bool pIsATrigger,
                                              const linguistics::LinguisticDatabase& pLingDb,
                                              bool pCheckChildren) {
    bool res = false;
    OtherConceptsLinkStrategy otherConceptsLinkStrategy = pIsATrigger || pRequestContext == RequestContext::COMMAND
                                                            ? OtherConceptsLinkStrategy::NO_LINK_TO_OTHER_CONCEPTS
                                                            : _requestCategoryToLinkStrategy(pRequestContext);

    // add semantic expressions that have a concept in common
    res = _conceptsToRelationsFromMemory(pRelations,
                                         pAlreadyMatchedSentences,
                                         pLinksToSemExps,
                                         pStatGrd.concepts,
                                         &pGrdExpToLookFor,
                                         pChildSemExpsToSkip,
                                         otherConceptsLinkStrategy,
                                         pMemBlockPrivatePtr,
                                         pIsATrigger,
                                         pLingDb,
                                         pCheckChildren)
       || res;

    StaticLinguisticMeaning lingMeaning = pLingDb.langToSpec[pStatGrd.word.language].lingDico.statDb.getLingMeaning(
        pStatGrd.word.lemma, pStatGrd.word.partOfSpeech, true);
    if (!lingMeaning.isEmpty()) {
        // add semantic expressions that have a meaning in common
        res = _getRelationsOfAMeaning(pRelations,
                                      pAlreadyMatchedSentences,
                                      pLinksToSemExps,
                                      lingMeaning.language,
                                      lingMeaning.meaningId,
                                      pGrdExpToLookFor,
                                      pChildSemExpsToSkip,
                                      pMemBlockPrivatePtr,
                                      pIsATrigger,
                                      pLingDb,
                                      pCheckChildren)
           || res;
    }

    if (pStatGrd.coreference && pStatGrd.word.lemma.empty() && pStatGrd.concepts.empty()) {
        _getStatementCoreferenceWithoutConceptRelations(pRelations,
                                                        pAlreadyMatchedSentences,
                                                        pLinksToSemExps,
                                                        pStatGrd.coreference->getDirection(),
                                                        pGrdExpToLookFor,
                                                        pChildSemExpsToSkip,
                                                        pMemBlockPrivatePtr,
                                                        pIsATrigger,
                                                        pLingDb,
                                                        pCheckChildren)
            || res;
    }

    return res;
}

template<bool IS_MODIFIABLE>
bool _agentGroundingToRelationsFromMemory(RelationsThatMatch<IS_MODIFIABLE>& pRelations,
                                          const SentenceLinks<IS_MODIFIABLE>& pAlreadyMatchedSentences,
                                          MemoryLinksAccessor<IS_MODIFIABLE>& pLinksToSemExps,
                                          const SemanticAgentGrounding& pAgentGrd,
                                          RequestContext pRequestContext,
                                          const GroundedExpression& pGrdExpToLookFor,
                                          const std::set<const SemanticExpression*>& pChildSemExpsToSkip,
                                          const SemanticMemoryBlockPrivate* pMemBlockPrivatePtr,
                                          bool pIsATrigger,
                                          const linguistics::LinguisticDatabase& pLingDb,
                                          bool pCheckChildren,
                                          SemanticLanguageEnum pLanguage) {
    bool res = false;
    res = _indefiniteThingsCheckMatching(pRelations,
                                         pAlreadyMatchedSentences,
                                         pLinksToSemExps,
                                         SemanticEntityType::AGENTORTHING,
                                         pGrdExpToLookFor,
                                         pChildSemExpsToSkip,
                                         pRequestContext,
                                         pMemBlockPrivatePtr,
                                         pIsATrigger,
                                         pLingDb,
                                         pCheckChildren)
       || res;

    // add semantic expressions that have a user id in common
    res = _oneUserIdToRelationsFromMemory(pRelations,
                                          pAlreadyMatchedSentences,
                                          pLinksToSemExps,
                                          pGrdExpToLookFor,
                                          pChildSemExpsToSkip,
                                          pAgentGrd.userId,
                                          pMemBlockPrivatePtr,
                                          pIsATrigger,
                                          pLingDb,
                                          pCheckChildren)
       || res;

    res = _userIdWithoutContextToRelationsFromMemory(pRelations,
                                                     pAlreadyMatchedSentences,
                                                     pLinksToSemExps,
                                                     pGrdExpToLookFor,
                                                     pChildSemExpsToSkip,
                                                     pAgentGrd.userIdWithoutContext,
                                                     pMemBlockPrivatePtr,
                                                     pIsATrigger,
                                                     pLingDb,
                                                     pCheckChildren)
       || res;

    if (!pAgentGrd.concepts.empty()) {
        OtherConceptsLinkStrategy otherConceptsLinkStrategy = _requestCategoryToLinkStrategy(pRequestContext);
        res = _conceptsToRelationsFromMemory(pRelations,
                                             pAlreadyMatchedSentences,
                                             pLinksToSemExps,
                                             pAgentGrd.concepts,
                                             &pGrdExpToLookFor,
                                             pChildSemExpsToSkip,
                                             otherConceptsLinkStrategy,
                                             pMemBlockPrivatePtr,
                                             pIsATrigger,
                                             pLingDb,
                                             pCheckChildren)
           || res;
    }

    if (pAgentGrd.nameInfos) {
        for (auto& currName : pAgentGrd.nameInfos->names)
            res = _relationsInLowerCaseFromMemory(pRelations,
                                                  pAlreadyMatchedSentences,
                                                  pLinksToSemExps,
                                                  currName,
                                                  pGrdExpToLookFor,
                                                  pChildSemExpsToSkip,
                                                  pRequestContext,
                                                  pMemBlockPrivatePtr,
                                                  pIsATrigger,
                                                  pLingDb,
                                                  pCheckChildren,
                                                  pLanguage)
               || res;
    }

    return res;
}

template<bool IS_MODIFIABLE>
bool _textGroundingToRelationsFromMemory(RelationsThatMatch<IS_MODIFIABLE>& pRelations,
                                         const SentenceLinks<IS_MODIFIABLE>& pAlreadyMatchedSentences,
                                         MemoryLinksAccessor<IS_MODIFIABLE>& pLinksToSemExps,
                                         const SemanticTextGrounding& pTextGrd,
                                         const GroundedExpression& pGrdExpToLookFor,
                                         const std::set<const SemanticExpression*>& pChildSemExpsToSkip,
                                         RequestContext pRequestContext,
                                         const SemanticMemoryBlockPrivate* pMemBlockPrivatePtr,
                                         bool pIsATrigger,
                                         const linguistics::LinguisticDatabase& pLingDb,
                                         bool pCheckChildren) {
    OtherConceptsLinkStrategy otherConceptsLinkStrategy = _requestCategoryToLinkStrategy(pRequestContext);
    bool res = _getTextRelations(pRelations,
                                 pAlreadyMatchedSentences,
                                 pLinksToSemExps,
                                 pTextGrd.text,
                                 pTextGrd.forLanguage,
                                 pGrdExpToLookFor,
                                 pChildSemExpsToSkip,
                                 pMemBlockPrivatePtr,
                                 pIsATrigger,
                                 pLingDb,
                                 pCheckChildren);
    res = _conceptsToRelationsFromMemory(pRelations,
                                         pAlreadyMatchedSentences,
                                         pLinksToSemExps,
                                         pTextGrd.concepts,
                                         &pGrdExpToLookFor,
                                         pChildSemExpsToSkip,
                                         otherConceptsLinkStrategy,
                                         pMemBlockPrivatePtr,
                                         pIsATrigger,
                                         pLingDb,
                                         pCheckChildren)
       || res;
    return res;
}

template<bool IS_MODIFIABLE>
bool _nameGroundingToRelationsFromMemory(RelationsThatMatch<IS_MODIFIABLE>& pRelations,
                                         const SentenceLinks<IS_MODIFIABLE>& pAlreadyMatchedSentences,
                                         MemoryLinksAccessor<IS_MODIFIABLE>& pLinksToSemExps,
                                         const SemanticNameGrounding& pNameGrd,
                                         const GroundedExpression& pGrdExpToLookFor,
                                         const std::set<const SemanticExpression*>& pChildSemExpsToSkip,
                                         RequestContext pRequestContext,
                                         const SemanticMemoryBlockPrivate* pMemBlockPrivatePtr,
                                         bool pIsATrigger,
                                         const linguistics::LinguisticDatabase& pLingDb,
                                         bool pCheckChildren,
                                         SemanticLanguageEnum pLanguage) {
    bool res = false;
    for (const auto& currName : pNameGrd.nameInfos.names)
        res = _getTextRelations(pRelations,
                                pAlreadyMatchedSentences,
                                pLinksToSemExps,
                                currName,
                                SemanticLanguageEnum::UNKNOWN,
                                pGrdExpToLookFor,
                                pChildSemExpsToSkip,
                                pMemBlockPrivatePtr,
                                pIsATrigger,
                                pLingDb,
                                pCheckChildren);
    OtherConceptsLinkStrategy otherConceptsLinkStrategy = _requestCategoryToLinkStrategy(pRequestContext);
    res = _conceptsToRelationsFromMemory(pRelations,
                                         pAlreadyMatchedSentences,
                                         pLinksToSemExps,
                                         pNameGrd.concepts,
                                         &pGrdExpToLookFor,
                                         pChildSemExpsToSkip,
                                         otherConceptsLinkStrategy,
                                         pMemBlockPrivatePtr,
                                         pIsATrigger,
                                         pLingDb,
                                         pCheckChildren)
       || res;
    res = _indefiniteThingsCheckMatching(pRelations,
                                         pAlreadyMatchedSentences,
                                         pLinksToSemExps,
                                         SemanticEntityType::AGENTORTHING,
                                         pGrdExpToLookFor,
                                         pChildSemExpsToSkip,
                                         pRequestContext,
                                         pMemBlockPrivatePtr,
                                         pIsATrigger,
                                         pLingDb,
                                         pCheckChildren)
       || res;

    for (auto& currName : pNameGrd.nameInfos.names)
        res = _relationsInLowerCaseFromMemory(pRelations,
                                              pAlreadyMatchedSentences,
                                              pLinksToSemExps,
                                              currName,
                                              pGrdExpToLookFor,
                                              pChildSemExpsToSkip,
                                              pRequestContext,
                                              pMemBlockPrivatePtr,
                                              pIsATrigger,
                                              pLingDb,
                                              pCheckChildren,
                                              pLanguage)
           || res;

    return res;
}

template<bool IS_MODIFIABLE, typename DURATIONITERATOR>
bool _addARelativeTimeElt(RelationsThatMatch<IS_MODIFIABLE>& pRelations,
                          const DURATIONITERATOR* pPrevItPtr,
                          const DURATIONITERATOR& pItTime,
                          const SentenceLinks<IS_MODIFIABLE>& pAlreadyMatchedSentences,
                          const GroundedExpression& pGrdExpToLookFor,
                          const std::set<const SemanticExpression*>& pChildSemExpsToSkip,
                          const SemanticMemoryBlockPrivate* pMemBlockPrivatePtr,
                          bool pIsATrigger,
                          const linguistics::LinguisticDatabase& pLingDb,
                          bool pCheckChildren) {
    auto* grdExpThatStartPtr = &pItTime->second;
    MemoryGrdExpLinks grdExpThatStart;
    if (pPrevItPtr != nullptr) {
        auto& prevIt = *pPrevItPtr;
        for (const auto& currTimeElt : pItTime->second)
            if (prevIt->second.count(currTimeElt.first) == 0
                && pAlreadyMatchedSentences.dynamicLinks.count(currTimeElt.first) > 0)
                grdExpThatStart.emplace(currTimeElt);
        if (grdExpThatStart.empty())
            return false;
        grdExpThatStartPtr = &grdExpThatStart;
    }
    IntIdToMemSentenceAccessor<IS_MODIFIABLE> accessor(*grdExpThatStartPtr);
    return _addPotentialNewRelationsFromLinks(pRelations,
                                              pAlreadyMatchedSentences,
                                              &accessor,
                                              nullptr,
                                              &pGrdExpToLookFor,
                                              pChildSemExpsToSkip,
                                              pMemBlockPrivatePtr,
                                              pIsATrigger,
                                              pLingDb,
                                              pCheckChildren);

    return false;
}

template<bool IS_MODIFIABLE>
bool _addARelativeTimeEltStatic(RelationsThatMatch<IS_MODIFIABLE>& pRelations,
                                const radixmap::StaticRadixMapIterator& pPrevIt,
                                const radixmap::StaticRadixMapIterator& pItTime,
                                const SentenceLinks<IS_MODIFIABLE>& pAlreadyMatchedSentences,
                                const GroundedExpression& pGrdExpToLookFor,
                                const std::set<const SemanticExpression*>& pChildSemExpsToSkip,
                                const SemanticMemoryBlockPrivate* pMemBlockPrivatePtr,
                                bool pIsATrigger,
                                const linguistics::LinguisticDatabase& pLingDb,
                                bool pCheckChildren) {
    std::map<intSemId, const unsigned char*> grdExpThatStart;
    _iterateOverStaticLinks(
        pAlreadyMatchedSentences,
        pItTime.value,
        pMemBlockPrivatePtr,
        [&](const unsigned char* pMemoryGrdExpPtr, const unsigned char* pStaticMemorySentencePtr) {
            if (pPrevIt.empty() || !_hasAStaticLink(pPrevIt.value, pMemBlockPrivatePtr, pMemoryGrdExpPtr))
                grdExpThatStart.emplace(SemanticMemoryBlockBinaryReader::memorySentenceToId(pStaticMemorySentencePtr),
                                        pMemoryGrdExpPtr);
        });
    if (!grdExpThatStart.empty()) {
        StaticBinaryLinks nodeLinks(std::move(grdExpThatStart));
        return _addPotentialNewRelationsFromLinks<IS_MODIFIABLE>(pRelations,
                                                                 pAlreadyMatchedSentences,
                                                                 nullptr,
                                                                 &nodeLinks,
                                                                 &pGrdExpToLookFor,
                                                                 pChildSemExpsToSkip,
                                                                 pMemBlockPrivatePtr,
                                                                 pIsATrigger,
                                                                 pLingDb,
                                                                 pCheckChildren);
    }
    return false;
}

template<bool IS_MODIFIABLE>
bool _timeToRelationsFromMemory(RelationsThatMatch<IS_MODIFIABLE>& pRelations,
                                const SentenceLinks<IS_MODIFIABLE>& pAlreadyMatchedSentences,
                                MemoryLinksAccessor<IS_MODIFIABLE>& pLinksToSemExps,
                                const SemanticTimeGrounding& pTimeGrd,
                                RequestContext pRequestContext,
                                const GroundedExpression& pGrdExpToLookFor,
                                const std::set<const SemanticExpression*>& pChildSemExpsToSkip,
                                const SemanticMemoryBlockPrivate* pMemBlockPrivatePtr,
                                bool pIsATrigger,
                                const linguistics::LinguisticDatabase& pLingDb,
                                bool pCheckChildren,
                                const SemanticRelativeTimeType* pRelativeTimePtr) {
    bool res = false;
    SemanticDuration beginTimeLimit;

    if (!pTimeGrd.concepts.empty()) {
        OtherConceptsLinkStrategy otherConceptsLinkStrategy = _requestCategoryToLinkStrategy(pRequestContext);
        // add semantic expressions that have a concept in common
        res = _conceptsToRelationsFromMemory(pRelations,
                                             pAlreadyMatchedSentences,
                                             pLinksToSemExps,
                                             pTimeGrd.concepts,
                                             &pGrdExpToLookFor,
                                             pChildSemExpsToSkip,
                                             otherConceptsLinkStrategy,
                                             pMemBlockPrivatePtr,
                                             pIsATrigger,
                                             pLingDb,
                                             pCheckChildren)
           || res;
    }

    if (pIsATrigger && !pTimeGrd.fromConcepts.empty()) {
        OtherConceptsLinkStrategy otherConceptsLinkStrategy = _requestCategoryToLinkStrategy(pRequestContext);
        // add semantic expressions that have a concept in common
        res = _conceptsToRelationsFromMemory(pRelations,
                                             pAlreadyMatchedSentences,
                                             pLinksToSemExps,
                                             pTimeGrd.fromConcepts,
                                             &pGrdExpToLookFor,
                                             pChildSemExpsToSkip,
                                             otherConceptsLinkStrategy,
                                             pMemBlockPrivatePtr,
                                             pIsATrigger,
                                             pLingDb,
                                             pCheckChildren)
           || res;
    }

    if (pRelativeTimePtr != nullptr) {
        switch (*pRelativeTimePtr) {
            case SemanticRelativeTimeType::AFTER:
            case SemanticRelativeTimeType::JUSTAFTER: {
                pTimeGrd.getBeginInDurationStruct(beginTimeLimit);
                if (pLinksToSemExps.d != nullptr) {
                    auto itPrev = pLinksToSemExps.d->timeToSemExps.find(beginTimeLimit);
                    auto itEnd = pLinksToSemExps.d->timeToSemExps.end();
                    if (itPrev != itEnd) {
                        auto itFirst = itPrev;
                        ++itFirst;
                        if (itFirst != itEnd) {
                            for (auto itTime = itFirst; itTime != itEnd; ++itTime) {
                                if (_addARelativeTimeElt(pRelations,
                                                         &itPrev,
                                                         itTime,
                                                         pAlreadyMatchedSentences,
                                                         pGrdExpToLookFor,
                                                         pChildSemExpsToSkip,
                                                         pMemBlockPrivatePtr,
                                                         pIsATrigger,
                                                         pLingDb,
                                                         pCheckChildren)
                                    && *pRelativeTimePtr == SemanticRelativeTimeType::JUSTAFTER)
                                    break;
                                itPrev = itTime;
                            }
                        }
                    }
                }
                if (pLinksToSemExps.c != nullptr) {
                    const unsigned char* timePtr = SemanticMemoryBlockBinaryReader::moveToTimePtr(pLinksToSemExps.c);
                    const auto beginTimeLimitRadixMapStr = beginTimeLimit.toRadixMapStr();
                    auto itPrev = radixmap::read_lower_bound(timePtr, beginTimeLimitRadixMapStr, &_hasStaticLinks);
                    if (itPrev.key == beginTimeLimitRadixMapStr && !itPrev.empty()) {
                        auto itFirst = radixmap::read_upper_bound(timePtr, itPrev.key, &_hasStaticLinks);
                        if (!itFirst.empty()) {
                            for (auto itTime = itFirst; !itTime.empty();
                                 itTime = radixmap::read_upper_bound(timePtr, itTime.key, &_hasStaticLinks)) {
                                if (_addARelativeTimeEltStatic(pRelations,
                                                               itPrev,
                                                               itTime,
                                                               pAlreadyMatchedSentences,
                                                               pGrdExpToLookFor,
                                                               pChildSemExpsToSkip,
                                                               pMemBlockPrivatePtr,
                                                               pIsATrigger,
                                                               pLingDb,
                                                               pCheckChildren)
                                    && *pRelativeTimePtr == SemanticRelativeTimeType::JUSTAFTER)
                                    break;
                                itPrev = itTime;
                            }
                        }
                    }
                }
                break;
            }
            case SemanticRelativeTimeType::BEFORE:
            case SemanticRelativeTimeType::JUSTBEFORE: {
                pTimeGrd.getBeginInDurationStruct(beginTimeLimit);
                if (pLinksToSemExps.d != nullptr) {
                    auto itBegin = pLinksToSemExps.d->timeToSemExps.begin();
                    auto itTime = pLinksToSemExps.d->timeToSemExps.find(beginTimeLimit);
                    if (itTime != pLinksToSemExps.d->timeToSemExps.end() && itTime != itBegin) {
                        auto itPrev = itTime;
                        --itPrev;
                        while (itTime != itBegin) {
                            itTime = itPrev;
                            auto* itPrevPtr = itPrev != itBegin ? &(--itPrev) : nullptr;
                            if (_addARelativeTimeElt(pRelations,
                                                     itPrevPtr,
                                                     itTime,
                                                     pAlreadyMatchedSentences,
                                                     pGrdExpToLookFor,
                                                     pChildSemExpsToSkip,
                                                     pMemBlockPrivatePtr,
                                                     pIsATrigger,
                                                     pLingDb,
                                                     pCheckChildren)
                                && *pRelativeTimePtr == SemanticRelativeTimeType::JUSTBEFORE)
                                break;
                        }
                    }
                }
                if (pLinksToSemExps.c != nullptr) {
                    const unsigned char* timePtr = SemanticMemoryBlockBinaryReader::moveToTimePtr(pLinksToSemExps.c);
                    const auto beginTimeLimitRadixMapStr = beginTimeLimit.toRadixMapStr();
                    auto itTime = radixmap::read_lower_bound(timePtr, beginTimeLimitRadixMapStr, &_hasStaticLinks);
                    if (itTime.key == beginTimeLimitRadixMapStr && !itTime.empty()) {
                        auto itPrev = radixmap::read_previous(timePtr, itTime.key, &_hasStaticLinks);
                        while (!itPrev.empty()) {
                            itTime = itPrev;
                            itPrev = radixmap::read_previous(timePtr, itPrev.key, &_hasStaticLinks);
                            if (_addARelativeTimeEltStatic(pRelations,
                                                           itPrev,
                                                           itTime,
                                                           pAlreadyMatchedSentences,
                                                           pGrdExpToLookFor,
                                                           pChildSemExpsToSkip,
                                                           pMemBlockPrivatePtr,
                                                           pIsATrigger,
                                                           pLingDb,
                                                           pCheckChildren)
                                && *pRelativeTimePtr == SemanticRelativeTimeType::JUSTBEFORE)
                                break;
                        }
                    }
                }
                break;
            }
            case SemanticRelativeTimeType::DELAYEDSTART:
            case SemanticRelativeTimeType::DURING:
            case SemanticRelativeTimeType::SINCE: break;
        }
    } else {
        pTimeGrd.getBeginInDurationStruct(beginTimeLimit);
        SemanticDuration endTimeLimit = beginTimeLimit + pTimeGrd.length;
        endTimeLimit.add(SemanticTimeUnity::LESS_THAN_A_MILLISECOND, 1);
        if (pLinksToSemExps.d != nullptr) {
            auto itFirst = pLinksToSemExps.d->timeToSemExps.lower_bound(beginTimeLimit);
            auto itLast = pLinksToSemExps.d->timeToSemExps.upper_bound(endTimeLimit);
            for (auto itTime = itFirst; itTime != itLast; ++itTime) {
                IntIdToMemSentenceAccessor<IS_MODIFIABLE> accessor(itTime->second);
                res = _addPotentialNewRelationsFromLinks(pRelations,
                                                         pAlreadyMatchedSentences,
                                                         &accessor,
                                                         nullptr,
                                                         &pGrdExpToLookFor,
                                                         pChildSemExpsToSkip,
                                                         pMemBlockPrivatePtr,
                                                         pIsATrigger,
                                                         pLingDb,
                                                         pCheckChildren)
                   || res;
            }
        }
        if (pLinksToSemExps.c != nullptr) {
            const unsigned char* timePtr = SemanticMemoryBlockBinaryReader::moveToTimePtr(pLinksToSemExps.c);
            auto itFirst = radixmap::read_lower_bound(timePtr, beginTimeLimit.toRadixMapStr(), &_hasStaticLinks);
            if (!itFirst.empty()) {
                auto itLast = radixmap::read_upper_bound(timePtr, endTimeLimit.toRadixMapStr(), &_hasStaticLinks);
                for (auto itTime = itFirst; itTime.key != itLast.key;
                     itTime = radixmap::read_upper_bound(timePtr, itTime.key, &_hasStaticLinks)) {
                    StaticBinaryLinks nodeLinks(itTime.value);
                    res = _addPotentialNewRelationsFromLinks<IS_MODIFIABLE>(pRelations,
                                                                            pAlreadyMatchedSentences,
                                                                            nullptr,
                                                                            &nodeLinks,
                                                                            &pGrdExpToLookFor,
                                                                            pChildSemExpsToSkip,
                                                                            pMemBlockPrivatePtr,
                                                                            pIsATrigger,
                                                                            pLingDb,
                                                                            pCheckChildren)
                       || res;
                }
            }
        }

        if (pTimeGrd.date.day) {
            if (pTimeGrd.date.month && !pTimeGrd.date.year) {
                auto dayCpt = pTimeGrd.date.getDayConcept();
                RelationsThatMatch<IS_MODIFIABLE> dayRelations;
                _oneConceptToRelationsFromMemory(dayRelations,
                                                 pAlreadyMatchedSentences,
                                                 pLinksToSemExps,
                                                 &pGrdExpToLookFor,
                                                 pChildSemExpsToSkip,
                                                 dayCpt,
                                                 pMemBlockPrivatePtr,
                                                 pIsATrigger,
                                                 pLingDb,
                                                 pCheckChildren)
                    || res;
                if (!dayRelations.empty()) {
                    auto monthCpt = monthConceptStr_fromMonthId(*pTimeGrd.date.month);
                    res = _oneConceptToRelationsFromMemory(pRelations,
                                                           dayRelations.res,
                                                           pLinksToSemExps,
                                                           &pGrdExpToLookFor,
                                                           pChildSemExpsToSkip,
                                                           monthCpt,
                                                           pMemBlockPrivatePtr,
                                                           pIsATrigger,
                                                           pLingDb,
                                                           pCheckChildren)
                       || res;
                }
            }
        } else {
            if (pTimeGrd.date.month) {
                auto monthCpt = monthConceptStr_fromMonthId(*pTimeGrd.date.month);
                if (pTimeGrd.date.year) {
                    RelationsThatMatch<IS_MODIFIABLE> monthRelations;
                    _oneConceptToRelationsFromMemory(monthRelations,
                                                     pAlreadyMatchedSentences,
                                                     pLinksToSemExps,
                                                     &pGrdExpToLookFor,
                                                     pChildSemExpsToSkip,
                                                     monthCpt,
                                                     pMemBlockPrivatePtr,
                                                     pIsATrigger,
                                                     pLingDb,
                                                     pCheckChildren)
                        || res;
                    if (!monthRelations.empty()) {
                        auto yearCpt = pTimeGrd.date.getYearConcept();
                        if (!yearCpt.empty())
                            res = _oneConceptToRelationsFromMemory(pRelations,
                                                                   monthRelations.res,
                                                                   pLinksToSemExps,
                                                                   &pGrdExpToLookFor,
                                                                   pChildSemExpsToSkip,
                                                                   yearCpt,
                                                                   pMemBlockPrivatePtr,
                                                                   pIsATrigger,
                                                                   pLingDb,
                                                                   pCheckChildren)
                               || res;
                    }
                } else {
                    res = _oneConceptToRelationsFromMemory(pRelations,
                                                           pAlreadyMatchedSentences,
                                                           pLinksToSemExps,
                                                           &pGrdExpToLookFor,
                                                           pChildSemExpsToSkip,
                                                           monthCpt,
                                                           pMemBlockPrivatePtr,
                                                           pIsATrigger,
                                                           pLingDb,
                                                           pCheckChildren)
                       || res;
                }
            } else {
                auto yearCpt = pTimeGrd.date.getYearConcept();
                if (!yearCpt.empty())
                    res = _oneConceptToRelationsFromMemory(pRelations,
                                                           pAlreadyMatchedSentences,
                                                           pLinksToSemExps,
                                                           &pGrdExpToLookFor,
                                                           pChildSemExpsToSkip,
                                                           yearCpt,
                                                           pMemBlockPrivatePtr,
                                                           pIsATrigger,
                                                           pLingDb,
                                                           pCheckChildren)
                       || res;
            }
        }
    }
    return res;
}

template<bool IS_MODIFIABLE>
bool _durationToRelationsFromMemory(RelationsThatMatch<IS_MODIFIABLE>& pRelations,
                                    const SentenceLinks<IS_MODIFIABLE>& pAlreadyMatchedSentences,
                                    MemoryLinksAccessor<IS_MODIFIABLE>& pLinksToSemExps,
                                    const SemanticDurationGrounding& pDurationGrd,
                                    RequestContext pRequestContext,
                                    const GroundedExpression& pGrdExpToLookFor,
                                    const std::set<const SemanticExpression*>& pChildSemExpsToSkip,
                                    const SemanticMemoryBlockPrivate* pMemBlockPrivatePtr,
                                    bool pIsATrigger,
                                    const linguistics::LinguisticDatabase& pLingDb,
                                    bool pCheckChildren) {
    bool res = false;

    if (!pDurationGrd.concepts.empty()) {
        OtherConceptsLinkStrategy otherConceptsLinkStrategy = _requestCategoryToLinkStrategy(pRequestContext);
        // add semantic expressions that have a concept in common
        res = _conceptsToRelationsFromMemory(pRelations,
                                             pAlreadyMatchedSentences,
                                             pLinksToSemExps,
                                             pDurationGrd.concepts,
                                             &pGrdExpToLookFor,
                                             pChildSemExpsToSkip,
                                             otherConceptsLinkStrategy,
                                             pMemBlockPrivatePtr,
                                             pIsATrigger,
                                             pLingDb,
                                             pCheckChildren)
           || res;
    }

    if (pLinksToSemExps.d != nullptr) {
        auto itToSemExp = pLinksToSemExps.d->durationToSemExps.find(pDurationGrd.duration);
        if (itToSemExp != pLinksToSemExps.d->durationToSemExps.end()) {
            IntIdToMemSentenceAccessor<IS_MODIFIABLE> accessor(itToSemExp->second);
            res = _addPotentialNewRelationsFromLinks(pRelations,
                                                     pAlreadyMatchedSentences,
                                                     &accessor,
                                                     nullptr,
                                                     &pGrdExpToLookFor,
                                                     pChildSemExpsToSkip,
                                                     pMemBlockPrivatePtr,
                                                     pIsATrigger,
                                                     pLingDb,
                                                     pCheckChildren)
               || res;
        }
    }
    if (pLinksToSemExps.c != nullptr) {
        // TODO
    }
    return res;
}

template<bool IS_MODIFIABLE>
bool _relLocationGroundingToRelationsFromMemory(RelationsThatMatch<IS_MODIFIABLE>& pRelations,
                                                const SentenceLinks<IS_MODIFIABLE>& pAlreadyMatchedSentences,
                                                MemoryLinksAccessor<IS_MODIFIABLE>& pLinksToSemExps,
                                                const SemanticRelativeLocationGrounding& pRelLocationGrd,
                                                const GroundedExpression& pGrdExpToLookFor,
                                                const std::set<const SemanticExpression*>& pChildSemExpsToSkip,
                                                const SemanticMemoryBlockPrivate* pMemBlockPrivatePtr,
                                                bool pIsATrigger,
                                                const linguistics::LinguisticDatabase& pLingDb,
                                                bool pCheckChildren) {
    bool res = false;
    if (pLinksToSemExps.d != nullptr) {
        auto itRelLocationToSemExp = pLinksToSemExps.d->relLocationToSemExps.find(pRelLocationGrd.locationType);
        if (itRelLocationToSemExp != pLinksToSemExps.d->relLocationToSemExps.end()) {
            IntIdToMemSentenceAccessor<IS_MODIFIABLE> accessor(itRelLocationToSemExp->second);
            res = _addPotentialNewRelationsFromLinks(pRelations,
                                                     pAlreadyMatchedSentences,
                                                     &accessor,
                                                     nullptr,
                                                     &pGrdExpToLookFor,
                                                     pChildSemExpsToSkip,
                                                     pMemBlockPrivatePtr,
                                                     pIsATrigger,
                                                     pLingDb,
                                                     pCheckChildren)
               || res;
        }
    }
    if (pLinksToSemExps.c != nullptr) {
        const unsigned char* relLocationPtr = SemanticMemoryBlockBinaryReader::moveToRelLocationPtr(pLinksToSemExps.c);
        StaticBinaryLinks nodeLinks(readEnumMap(relLocationPtr,
                                                semanticRelativeLocationType_toChar(pRelLocationGrd.locationType),
                                                semanticRelativeLocationType_size));
        if (nodeLinks.linksToGrdExp != nullptr)
            res = _addPotentialNewRelationsFromLinks<IS_MODIFIABLE>(pRelations,
                                                                    pAlreadyMatchedSentences,
                                                                    nullptr,
                                                                    &nodeLinks,
                                                                    &pGrdExpToLookFor,
                                                                    pChildSemExpsToSkip,
                                                                    pMemBlockPrivatePtr,
                                                                    pIsATrigger,
                                                                    pLingDb,
                                                                    pCheckChildren)
               || res;
    }
    return res;
}

template<bool IS_MODIFIABLE>
bool _relTimeGroundingToRelationsFromMemory(RelationsThatMatch<IS_MODIFIABLE>& pRelations,
                                            const SentenceLinks<IS_MODIFIABLE>& pAlreadyMatchedSentences,
                                            MemoryLinksAccessor<IS_MODIFIABLE>& pLinksToSemExps,
                                            const SemanticRelativeTimeGrounding& pRelTimeGrd,
                                            const GroundedExpression& pGrdExpToLookFor,
                                            const std::set<const SemanticExpression*>& pChildSemExpsToSkip,
                                            const SemanticMemoryBlockPrivate* pMemBlockPrivatePtr,
                                            bool pIsATrigger,
                                            const linguistics::LinguisticDatabase& pLingDb,
                                            bool pCheckChildren) {
    bool res = false;
    if (pLinksToSemExps.d != nullptr) {
        auto itRelTimeToSemExp = pLinksToSemExps.d->relTimeToSemExps.find(pRelTimeGrd.timeType);
        if (itRelTimeToSemExp != pLinksToSemExps.d->relTimeToSemExps.end()) {
            IntIdToMemSentenceAccessor<IS_MODIFIABLE> accessor(itRelTimeToSemExp->second);
            res = _addPotentialNewRelationsFromLinks(pRelations,
                                                     pAlreadyMatchedSentences,
                                                     &accessor,
                                                     nullptr,
                                                     &pGrdExpToLookFor,
                                                     pChildSemExpsToSkip,
                                                     pMemBlockPrivatePtr,
                                                     pIsATrigger,
                                                     pLingDb,
                                                     pCheckChildren)
               || res;
        }
    }
    if (pLinksToSemExps.c != nullptr) {
        const unsigned char* relLocationPtr = SemanticMemoryBlockBinaryReader::moveToRelTimePtr(pLinksToSemExps.c);
        StaticBinaryLinks nodeLinks(readEnumMap(
            relLocationPtr, semanticRelativeTimeType_toChar(pRelTimeGrd.timeType), semanticRelativeTimeType_size));
        if (nodeLinks.linksToGrdExp != nullptr)
            res = _addPotentialNewRelationsFromLinks<IS_MODIFIABLE>(pRelations,
                                                                    pAlreadyMatchedSentences,
                                                                    nullptr,
                                                                    &nodeLinks,
                                                                    &pGrdExpToLookFor,
                                                                    pChildSemExpsToSkip,
                                                                    pMemBlockPrivatePtr,
                                                                    pIsATrigger,
                                                                    pLingDb,
                                                                    pCheckChildren)
               || res;
    }
    return res;
}

template<bool IS_MODIFIABLE>
bool _languageGroundingToRelationsFromMemory(RelationsThatMatch<IS_MODIFIABLE>& pRelations,
                                             const SentenceLinks<IS_MODIFIABLE>& pAlreadyMatchedSentences,
                                             MemoryLinksAccessor<IS_MODIFIABLE>& pLinksToSemExps,
                                             const SemanticLanguageGrounding& pLangGrd,
                                             const GroundedExpression& pGrdExpToLookFor,
                                             const std::set<const SemanticExpression*>& pChildSemExpsToSkip,
                                             const SemanticMemoryBlockPrivate* pMemBlockPrivatePtr,
                                             bool pIsATrigger,
                                             const linguistics::LinguisticDatabase& pLingDb,
                                             bool pCheckChildren) {
    bool res = false;
    if (pLinksToSemExps.d != nullptr) {
        auto itUrlToSemExp = pLinksToSemExps.d->languageToSemExps.find(pLangGrd.language);
        if (itUrlToSemExp != pLinksToSemExps.d->languageToSemExps.end()) {
            IntIdToMemSentenceAccessor<IS_MODIFIABLE> accessor(itUrlToSemExp->second);
            res = _addPotentialNewRelationsFromLinks(pRelations,
                                                     pAlreadyMatchedSentences,
                                                     &accessor,
                                                     nullptr,
                                                     &pGrdExpToLookFor,
                                                     pChildSemExpsToSkip,
                                                     pMemBlockPrivatePtr,
                                                     pIsATrigger,
                                                     pLingDb,
                                                     pCheckChildren)
               || res;
        }
    }
    if (pLinksToSemExps.c != nullptr) {
        auto* languagePtr = SemanticMemoryBlockBinaryReader::moveToLanguagePtr(pLinksToSemExps.c);
        StaticBinaryLinks nodeLinks(
            readEnumMap(languagePtr, semanticLanguageEnum_toChar(pLangGrd.language), semanticLanguageEnum_size));
        if (nodeLinks.linksToGrdExp != nullptr)
            res = _addPotentialNewRelationsFromLinks<IS_MODIFIABLE>(pRelations,
                                                                    pAlreadyMatchedSentences,
                                                                    nullptr,
                                                                    &nodeLinks,
                                                                    &pGrdExpToLookFor,
                                                                    pChildSemExpsToSkip,
                                                                    pMemBlockPrivatePtr,
                                                                    pIsATrigger,
                                                                    pLingDb,
                                                                    pCheckChildren)
               || res;
    }
    return res;
}

template<bool IS_MODIFIABLE>
bool _getRelationsFromListExp(RelationsThatMatch<IS_MODIFIABLE>& pRelations,
                              const SentenceLinks<IS_MODIFIABLE>& pAlreadyMatchedSentences,
                              MemoryLinksAccessor<IS_MODIFIABLE>& pLinksToSemExps,
                              const ListExpression& pListExpToLookFor,
                              const std::set<const SemanticExpression*>& pChildSemExpsToSkip,
                              RequestContext pRequestContext,
                              const SemanticMemoryBlockPrivate* pMemBlockPrivatePtr,
                              const linguistics::LinguisticDatabase& pLingDb,
                              bool pCheckChildren,
                              SemanticLanguageEnum pLanguage,
                              bool pIsATrigger) {
    bool res = false;
    for (const auto& currElt : pListExpToLookFor.elts) {
        bool subRes = _getRelationsFromSemExp(pRelations,
                                              pAlreadyMatchedSentences,
                                              pLinksToSemExps,
                                              *currElt,
                                              pChildSemExpsToSkip,
                                              pRequestContext,
                                              pMemBlockPrivatePtr,
                                              pLingDb,
                                              pCheckChildren,
                                              pLanguage,
                                              pIsATrigger);
        res = subRes || res;
        if (pListExpToLookFor.listType != ListExpressionType::OR && !subRes)
            return false;
    }
    return res;
}

template<bool IS_MODIFIABLE>
bool _getRelationsFromGrdExp(RelationsThatMatch<IS_MODIFIABLE>& pRelations,
                             const SentenceLinks<IS_MODIFIABLE>& pAlreadyMatchedSentences,
                             MemoryLinksAccessor<IS_MODIFIABLE>& pLinksToSemExps,
                             const GroundedExpression& pGrdExpToLookFor,
                             const std::set<const SemanticExpression*>& pChildSemExpsToSkip,
                             RequestContext pRequestContext,
                             const SemanticMemoryBlockPrivate* pMemBlockPrivatePtr,
                             const linguistics::LinguisticDatabase& pLingDb,
                             bool pCheckChildren,
                             SemanticLanguageEnum pLanguage,
                             bool pIsATrigger,
                             const SemanticRelativeTimeType* pRelativeTimePtr = nullptr) {
    switch (pGrdExpToLookFor->type) {
        case SemanticGroundingType::GENERIC: {
            auto& genGrd = pGrdExpToLookFor->getGenericGrounding();
            return _genGroundingToRelationsFromMemory(pRelations,
                                                      pAlreadyMatchedSentences,
                                                      pLinksToSemExps,
                                                      genGrd,
                                                      pGrdExpToLookFor,
                                                      pChildSemExpsToSkip,
                                                      pRequestContext,
                                                      pMemBlockPrivatePtr,
                                                      pIsATrigger,
                                                      pLingDb,
                                                      pCheckChildren,
                                                      pLanguage);
        }
        case SemanticGroundingType::STATEMENT: {
            auto& statGrd = pGrdExpToLookFor->getStatementGrounding();
            if (statGrd.empty())
                return true;
            return _statementGroundingToRelationsFromMemory(pRelations,
                                                            pAlreadyMatchedSentences,
                                                            pLinksToSemExps,
                                                            statGrd,
                                                            pGrdExpToLookFor,
                                                            pChildSemExpsToSkip,
                                                            pRequestContext,
                                                            pMemBlockPrivatePtr,
                                                            pIsATrigger,
                                                            pLingDb,
                                                            pCheckChildren);
        }
        case SemanticGroundingType::AGENT: {
            auto& agentGrd = pGrdExpToLookFor->getAgentGrounding();
            return _agentGroundingToRelationsFromMemory(pRelations,
                                                        pAlreadyMatchedSentences,
                                                        pLinksToSemExps,
                                                        agentGrd,
                                                        pRequestContext,
                                                        pGrdExpToLookFor,
                                                        pChildSemExpsToSkip,
                                                        pMemBlockPrivatePtr,
                                                        pIsATrigger,
                                                        pLingDb,
                                                        pCheckChildren,
                                                        pLanguage);
        }
        case SemanticGroundingType::TEXT: {
            auto& textGrd = pGrdExpToLookFor->getTextGrounding();
            return _textGroundingToRelationsFromMemory(pRelations,
                                                       pAlreadyMatchedSentences,
                                                       pLinksToSemExps,
                                                       textGrd,
                                                       pGrdExpToLookFor,
                                                       pChildSemExpsToSkip,
                                                       pRequestContext,
                                                       pMemBlockPrivatePtr,
                                                       pIsATrigger,
                                                       pLingDb,
                                                       pCheckChildren);
        }
        case SemanticGroundingType::NAME: {
            auto& nameGrd = pGrdExpToLookFor->getNameGrounding();
            return _nameGroundingToRelationsFromMemory(pRelations,
                                                       pAlreadyMatchedSentences,
                                                       pLinksToSemExps,
                                                       nameGrd,
                                                       pGrdExpToLookFor,
                                                       pChildSemExpsToSkip,
                                                       pRequestContext,
                                                       pMemBlockPrivatePtr,
                                                       pIsATrigger,
                                                       pLingDb,
                                                       pCheckChildren,
                                                       pLanguage);
        }
        case SemanticGroundingType::DURATION: {
            const auto& durationGrd = pGrdExpToLookFor->getDurationGrounding();
            return _durationToRelationsFromMemory(pRelations,
                                                  pAlreadyMatchedSentences,
                                                  pLinksToSemExps,
                                                  durationGrd,
                                                  pRequestContext,
                                                  pGrdExpToLookFor,
                                                  pChildSemExpsToSkip,
                                                  pMemBlockPrivatePtr,
                                                  pIsATrigger,
                                                  pLingDb,
                                                  pCheckChildren);
        }
        case SemanticGroundingType::TIME: {
            const auto& timeGrd = pGrdExpToLookFor->getTimeGrounding();
            return _timeToRelationsFromMemory(pRelations,
                                              pAlreadyMatchedSentences,
                                              pLinksToSemExps,
                                              timeGrd,
                                              pRequestContext,
                                              pGrdExpToLookFor,
                                              pChildSemExpsToSkip,
                                              pMemBlockPrivatePtr,
                                              pIsATrigger,
                                              pLingDb,
                                              pCheckChildren,
                                              pRelativeTimePtr);
        }
        case SemanticGroundingType::RELATIVELOCATION: {
            const auto& relLocationGrd = pGrdExpToLookFor->getRelLocationGrounding();
            bool res = _relLocationGroundingToRelationsFromMemory(pRelations,
                                                                  pAlreadyMatchedSentences,
                                                                  pLinksToSemExps,
                                                                  relLocationGrd,
                                                                  pGrdExpToLookFor,
                                                                  pChildSemExpsToSkip,
                                                                  pMemBlockPrivatePtr,
                                                                  pIsATrigger,
                                                                  pLingDb,
                                                                  pCheckChildren);
            if (relLocationGrd.locationType == SemanticRelativeLocationType::L_ON
                || relLocationGrd.locationType == SemanticRelativeLocationType::L_INSIDE) {
                auto itSpecifictionOfRelLocation = pGrdExpToLookFor.children.find(GrammaticalType::SPECIFIER);
                if (itSpecifictionOfRelLocation != pGrdExpToLookFor.children.end())
                    res = _getRelationsFromSemExp(pRelations,
                                                  pAlreadyMatchedSentences,
                                                  pLinksToSemExps,
                                                  *itSpecifictionOfRelLocation->second,
                                                  pChildSemExpsToSkip,
                                                  pRequestContext,
                                                  pMemBlockPrivatePtr,
                                                  pLingDb,
                                                  pCheckChildren,
                                                  pLanguage,
                                                  pIsATrigger,
                                                  pRelativeTimePtr)
                       || res;
            }
            return res;
        }
        case SemanticGroundingType::RELATIVETIME: {
            const auto& relTimeGrd = pGrdExpToLookFor->getRelTimeGrounding();
            bool res = false;
            res = _relTimeGroundingToRelationsFromMemory(pRelations,
                                                         pAlreadyMatchedSentences,
                                                         pLinksToSemExps,
                                                         relTimeGrd,
                                                         pGrdExpToLookFor,
                                                         pChildSemExpsToSkip,
                                                         pMemBlockPrivatePtr,
                                                         pIsATrigger,
                                                         pLingDb,
                                                         pCheckChildren)
               || res;
            auto itSpecifictionOfRelTime = pGrdExpToLookFor.children.find(GrammaticalType::SPECIFIER);
            if (itSpecifictionOfRelTime != pGrdExpToLookFor.children.end()) {
                res = _getRelationsFromSemExp(pRelations,
                                              pAlreadyMatchedSentences,
                                              pLinksToSemExps,
                                              *itSpecifictionOfRelTime->second,
                                              pChildSemExpsToSkip,
                                              pRequestContext,
                                              pMemBlockPrivatePtr,
                                              pLingDb,
                                              pCheckChildren,
                                              pLanguage,
                                              pIsATrigger,
                                              &relTimeGrd.timeType)
                   || res;
            }
            return res;
        }
        case SemanticGroundingType::LANGUAGE: {
            auto& langGrd = pGrdExpToLookFor->getLanguageGrounding();
            return _languageGroundingToRelationsFromMemory(pRelations,
                                                           pAlreadyMatchedSentences,
                                                           pLinksToSemExps,
                                                           langGrd,
                                                           pGrdExpToLookFor,
                                                           pChildSemExpsToSkip,
                                                           pMemBlockPrivatePtr,
                                                           pIsATrigger,
                                                           pLingDb,
                                                           pCheckChildren);
        }
        case SemanticGroundingType::CONCEPTUAL: {
            OtherConceptsLinkStrategy otherConceptsLinkStrategy = _requestCategoryToLinkStrategy(pRequestContext);
            auto& concepts = pGrdExpToLookFor.grounding().concepts;
            return _conceptsToRelationsFromMemory(pRelations,
                                                  pAlreadyMatchedSentences,
                                                  pLinksToSemExps,
                                                  concepts,
                                                  &pGrdExpToLookFor,
                                                  pChildSemExpsToSkip,
                                                  otherConceptsLinkStrategy,
                                                  pMemBlockPrivatePtr,
                                                  pIsATrigger,
                                                  pLingDb,
                                                  pCheckChildren);
        }
        case SemanticGroundingType::RESOURCE: {
            bool res = false;
            const auto& resGrd = pGrdExpToLookFor->getResourceGrounding();
            if (pLinksToSemExps.d != nullptr) {
                auto resToSemExpPtr = pLinksToSemExps.d->resourceToSemExps.find_ptr(resGrd.resource);
                if (resToSemExpPtr != nullptr) {
                    IntIdToMemSentenceAccessor<IS_MODIFIABLE> accessor(*resToSemExpPtr);
                    res = _addPotentialNewRelationsFromLinks(pRelations,
                                                             pAlreadyMatchedSentences,
                                                             &accessor,
                                                             nullptr,
                                                             &pGrdExpToLookFor,
                                                             pChildSemExpsToSkip,
                                                             pMemBlockPrivatePtr,
                                                             pIsATrigger,
                                                             pLingDb,
                                                             pCheckChildren)
                       || res;
                }
            }
            if (pLinksToSemExps.c != nullptr) {
                const unsigned char* resourcePtr =
                    SemanticMemoryBlockBinaryReader::moveToResourcePtr(pLinksToSemExps.c);
                StaticBinaryLinks nodeLinks(
                    radixmap::read(resourcePtr, resGrd.resource.toRadixMapStr(), _hasStaticLinks));
                if (nodeLinks.linksToGrdExp != nullptr)
                    res = _addPotentialNewRelationsFromLinks<IS_MODIFIABLE>(pRelations,
                                                                            pAlreadyMatchedSentences,
                                                                            nullptr,
                                                                            &nodeLinks,
                                                                            &pGrdExpToLookFor,
                                                                            pChildSemExpsToSkip,
                                                                            pMemBlockPrivatePtr,
                                                                            pIsATrigger,
                                                                            pLingDb,
                                                                            pCheckChildren)
                       || res;
            }
            return res;
        }
        case SemanticGroundingType::UNITY: {
            const auto& unityGrd = pGrdExpToLookFor->getUnityGrounding();
            return _oneConceptToRelationsFromMemory(pRelations,
                                                    pAlreadyMatchedSentences,
                                                    pLinksToSemExps,
                                                    &pGrdExpToLookFor,
                                                    pChildSemExpsToSkip,
                                                    unityGrd.getValueConcept(),
                                                    pMemBlockPrivatePtr,
                                                    pIsATrigger,
                                                    pLingDb,
                                                    pCheckChildren);
        }
        case SemanticGroundingType::ANGLE:
        case SemanticGroundingType::RELATIVEDURATION:
        case SemanticGroundingType::LENGTH:
        case SemanticGroundingType::META:
        case SemanticGroundingType::PERCENTAGE: return false;
    }
    return false;
}

template<bool IS_MODIFIABLE>
bool _getRelationsFromSemExp(RelationsThatMatch<IS_MODIFIABLE>& pRelations,
                             const SentenceLinks<IS_MODIFIABLE>& pAlreadyMatchedSentences,
                             MemoryLinksAccessor<IS_MODIFIABLE>& pLinksToSemExps,
                             const SemanticExpression& pSemExpToLookFor,
                             const std::set<const SemanticExpression*>& pChildSemExpsToSkip,
                             RequestContext pRequestContext,
                             const SemanticMemoryBlockPrivate* pMemBlockPrivatePtr,
                             const linguistics::LinguisticDatabase& pLingDb,
                             bool pCheckChildren,
                             SemanticLanguageEnum pLanguage,
                             bool pIsATrigger,
                             const SemanticRelativeTimeType* pRelativeTimePtr) {
    bool followInterpretations = !pIsATrigger && pRequestContext != RequestContext::SENTENCE_TO_CONDITION;
    const GroundedExpression* grdExpToLookForPtr = pSemExpToLookFor.getGrdExpPtr_SkipWrapperPtrs(followInterpretations);
    if (grdExpToLookForPtr != nullptr) {
        const GroundedExpression& grdExpToLookFor = *grdExpToLookForPtr;
        if (!pIsATrigger && SemExpGetter::isAnything(grdExpToLookFor)) {
            pRelations.res.dynamicLinks.insert(pAlreadyMatchedSentences.dynamicLinks.begin(),
                                               pAlreadyMatchedSentences.dynamicLinks.end());
            return !pRelations.empty();
        }
        return _getRelationsFromGrdExp(pRelations,
                                       pAlreadyMatchedSentences,
                                       pLinksToSemExps,
                                       grdExpToLookFor,
                                       pChildSemExpsToSkip,
                                       pRequestContext,
                                       pMemBlockPrivatePtr,
                                       pLingDb,
                                       pCheckChildren,
                                       pLanguage,
                                       pIsATrigger,
                                       pRelativeTimePtr);
    }

    const ListExpression* listExpToLookFor = pSemExpToLookFor.getListExpPtr();
    if (listExpToLookFor != nullptr)
        return _getRelationsFromListExp(pRelations,
                                        pAlreadyMatchedSentences,
                                        pLinksToSemExps,
                                        *listExpToLookFor,
                                        pChildSemExpsToSkip,
                                        pRequestContext,
                                        pMemBlockPrivatePtr,
                                        pLingDb,
                                        pCheckChildren,
                                        pLanguage,
                                        pIsATrigger);

    return false;
}

bool _shouldSemExpBeSkipped(const SemanticExpression& pSemExp) {
    const auto* grdExpPtr = pSemExp.getGrdExpPtr_SkipWrapperPtrs();
    if (grdExpPtr != nullptr)
        return SemExpComparator::doesGrdExpContainEverything(*grdExpPtr);
    return false;
}

struct RelationsThatMatchFilterOnChildren : public RelationsThatMatchFilter {
    RelationsThatMatchFilterOnChildren(std::function<void(const SemanticExpression&)>&& pFunction)
        : grammTypeOfChild(GrammaticalType::UNKNOWN)
        , function(std::move(pFunction)) {}

    virtual ~RelationsThatMatchFilterOnChildren() {}
    bool filterCondition(const GroundedExpWithLinks& pMemSent) const override {
        auto* subAnswerSemExpPtr = pMemSent.getSemExpForGrammaticalType(grammTypeOfChild);
        if (subAnswerSemExpPtr != nullptr)
            function(*subAnswerSemExpPtr);
        return false;
    }
    bool filterConditionStatic(AnswerElementStatic& pAnswElt) const override {
        auto subAnswerSemExpPtr = pAnswElt.getSemExpForGrammaticalType(grammTypeOfChild, nullptr, nullptr, nullptr);
        if (subAnswerSemExpPtr)
            function(subAnswerSemExpPtr->getSemExp());
        return false;
    }

    GrammaticalType grammTypeOfChild;
    const std::function<void(const SemanticExpression&)> function;
};

template<bool IS_MODIFIABLE>
void _getRelationsFromSubReqLinks(RelationsThatMatch<IS_MODIFIABLE>& pRelations,
                                  const SentenceLinks<IS_MODIFIABLE>& pAlreadyMatchedSentences,
                                  MemoryLinksAccessor<IS_MODIFIABLE>& pLinksToSemExps,
                                  const std::list<semanticMemoryLinker::RequestLinks>& pReqLinks,
                                  RequestContext pRequestContext,
                                  MemoryBlockPrivateAccessor<IS_MODIFIABLE>& pMemBlockPrivate,
                                  bool pIsATrigger,
                                  const linguistics::LinguisticDatabase& pLingDb,
                                  bool pCheckChildren,
                                  SemanticLanguageEnum pLanguage,
                                  bool pCheckTimeRequest,
                                  bool pConsiderCoreferences) {
    RelationsThatMatchFilterOnChildren filter([&](const SemanticExpression& pSemExp) {
        std::set<const SemanticExpression*> childSemExpsToSkip;
        SemExpGetter::getStatementSubordinates(childSemExpsToSkip, pSemExp);
        _getRelationsFromSemExp(pRelations,
                                pAlreadyMatchedSentences,
                                pLinksToSemExps,
                                pSemExp,
                                childSemExpsToSkip,
                                pRequestContext,
                                &pMemBlockPrivate.mb,
                                pLingDb,
                                pCheckChildren,
                                pLanguage,
                                pIsATrigger);
    });
    for (const auto& currSubReqList : pReqLinks) {
        filter.grammTypeOfChild = currSubReqList.gramTypeOfTheAnswer;
        auto awLinks =
            pMemBlockPrivate.mb.getLinks(SemanticTypeOfLinks::ANSWER, currSubReqList.tense, currSubReqList.verbGoal);
        RelationsThatMatch<IS_MODIFIABLE> semExpToAdd;
        MemoryBlockPrivateAccessorPtr<IS_MODIFIABLE> memBlockPrivateAccessorPtr(&pMemBlockPrivate.mb);
        getResultFromMemory(semExpToAdd,
                            awLinks,
                            currSubReqList,
                            pRequestContext,
                            memBlockPrivateAccessorPtr,
                            pIsATrigger,
                            pLingDb,
                            pCheckTimeRequest,
                            pConsiderCoreferences,
                            false,
                            &filter);
    }
}

bool _hasToSkip(SemanticRequestType pChildRequest,
                const semanticMemoryLinker::SubRequestLinks& pSubReqLinks,
                bool pCheckTimeRequest,
                bool pIsATrigger) {
    if (pChildRequest == SemanticRequestType::TIME) {
        if (!pCheckTimeRequest)
            return true;
        bool hasASemExpToCheck = false;

        for (const auto& currSemExp : pSubReqLinks.semExps) {
            auto* timeGrdPtr = SemExpGetter::semExpToTimeGrounding(*currSemExp);
            if (timeGrdPtr != nullptr
                && ConceptSet::haveAnyOfConcepts(timeGrdPtr->fromConcepts,
                                                 {"time_relative_now", "time_relative_rightAway"}))
                continue;
            hasASemExpToCheck = true;
        }
        if (!hasASemExpToCheck)
            return true;
    }
    if (!pIsATrigger && !pSubReqLinks.semExps.empty()) {
        bool skipThisChild = true;
        for (const auto& currSemExp : pSubReqLinks.semExps)
            skipThisChild = skipThisChild && _shouldSemExpBeSkipped(*currSemExp);
        if (skipThisChild)
            return true;
    }
    return false;
}

template<bool IS_MODIFIABLE>
void _getResultFromLink(RelationsThatMatch<IS_MODIFIABLE>& pRes,
                        RequestToMemoryLinksVirtual<IS_MODIFIABLE>& pReqToList,
                        bool& pFirstIteration,
                        SemanticRequestType pChildRequest,
                        const semanticMemoryLinker::SubRequestLinks& pSubReqLinks,
                        RequestContext pRequestContext,
                        MemoryBlockPrivateAccessorPtr<IS_MODIFIABLE>& pMemBlockPrivatePtr,
                        const linguistics::LinguisticDatabase& pLingDb,
                        bool pCheckTimeRequest,
                        bool pConsiderCoreferences,
                        const semanticMemoryLinker::RequestLinks& pReqLinks,
                        bool pIsATrigger) {
    if (_hasToSkip(pChildRequest, pSubReqLinks, pCheckTimeRequest, pIsATrigger))
        return;

    RelationsThatMatch<IS_MODIFIABLE> matchedSemExp;
    GrammaticalType gramType = semanticRequestType_toSemGram(pChildRequest);
    MemoryLinksAccessor<IS_MODIFIABLE> linksGramToSemExp = pReqToList.getMemoryLinksAccessors(pChildRequest);
    if (!linksGramToSemExp.empty()) {
        matchedSemExp.filterPtr = pRes.filterPtr;
        bool checkChildren = gramType != GrammaticalType::UNKNOWN;

        // add children that can be anything
        if (linksGramToSemExp.d != nullptr) {
            auto itMetaSemExp = linksGramToSemExp.d->grdTypeToSemExps.find(SemanticGroundingType::META);
            if (itMetaSemExp != linksGramToSemExp.d->grdTypeToSemExps.end()) {
                IntIdToMemSentenceAccessor<IS_MODIFIABLE> accessor(itMetaSemExp->second);
                _addPotentialNewRelationsFromLinks(matchedSemExp,
                                                   pRes.res,
                                                   &accessor,
                                                   nullptr,
                                                   nullptr,
                                                   pSubReqLinks.crossedLinks.semExpsWithSpecificFilter,
                                                   pMemBlockPrivatePtr.mb,
                                                   pIsATrigger,
                                                   pLingDb,
                                                   checkChildren);
            }
        }
        if (linksGramToSemExp.c != nullptr) {
            const unsigned char* grdTypePtr = SemanticMemoryBlockBinaryReader::moveToGrdTypePtr(linksGramToSemExp.c);
            StaticBinaryLinks nodeLinks(readEnumMap(
                grdTypePtr, semanticGroundingsType_toChar(SemanticGroundingType::META), semanticGroundingType_size));
            if (nodeLinks.linksToGrdExp != nullptr)
                _addPotentialNewRelationsFromLinks<IS_MODIFIABLE>(matchedSemExp,
                                                                  pRes.res,
                                                                  nullptr,
                                                                  &nodeLinks,
                                                                  nullptr,
                                                                  pSubReqLinks.crossedLinks.semExpsWithSpecificFilter,
                                                                  pMemBlockPrivatePtr.mb,
                                                                  pIsATrigger,
                                                                  pLingDb,
                                                                  checkChildren);
        }

        const bool metaMatched = !matchedSemExp.empty();

        for (const auto& currSemExp : pSubReqLinks.semExps)
            _getRelationsFromSemExp(matchedSemExp,
                                    pRes.res,
                                    linksGramToSemExp,
                                    *currSemExp,
                                    pSubReqLinks.crossedLinks.semExpsWithSpecificFilter,
                                    pRequestContext,
                                    pMemBlockPrivatePtr.mb,
                                    pLingDb,
                                    checkChildren,
                                    pReqLinks.language,
                                    pIsATrigger);
        if (!pSubReqLinks.concepts.empty()) {
            OtherConceptsLinkStrategy otherConceptsLinkStrategy = _requestCategoryToLinkStrategy(pRequestContext);
            _conceptsToRelationsFromMemory(matchedSemExp,
                                           pRes.res,
                                           linksGramToSemExp,
                                           pSubReqLinks.concepts,
                                           nullptr,
                                           pSubReqLinks.crossedLinks.semExpsWithSpecificFilter,
                                           otherConceptsLinkStrategy,
                                           pMemBlockPrivatePtr.mb,
                                           pIsATrigger,
                                           pLingDb,
                                           pCheckTimeRequest);
        }
        _oneConceptToRelationsFromMemory(matchedSemExp,
                                         pRes.res,
                                         linksGramToSemExp,
                                         nullptr,
                                         pSubReqLinks.crossedLinks.semExpsWithSpecificFilter,
                                         "stuff_informationToFill",
                                         pMemBlockPrivatePtr.mb,
                                         pIsATrigger,
                                         pLingDb,
                                         pCheckTimeRequest);

        if (pMemBlockPrivatePtr.mb != nullptr) {
            auto& memBlock = *pMemBlockPrivatePtr.mb;
            if (!pSubReqLinks.crossedLinks.subReqListsToAdd.empty()) {
                MemoryBlockPrivateAccessor<IS_MODIFIABLE> memBlockAccessor(memBlock);
                _getRelationsFromSubReqLinks(matchedSemExp,
                                             pRes.res,
                                             linksGramToSemExp,
                                             pSubReqLinks.crossedLinks.subReqListsToAdd,
                                             pRequestContext,
                                             memBlockAccessor,
                                             pIsATrigger,
                                             pLingDb,
                                             checkChildren,
                                             pReqLinks.language,
                                             pCheckTimeRequest,
                                             pConsiderCoreferences);
            }

            if (!pSubReqLinks.crossedLinks.subReqListsToFilter.empty() && !matchedSemExp.empty() && !metaMatched) {
                MemoryLinksAccessor<IS_MODIFIABLE> linksGramOfTheAnswer(nullptr, nullptr);
                if (pReqLinks.isEquVerb && pReqLinks.gramTypeOfTheAnswer != gramType
                    && pReqLinks.gramTypeOfTheAnswer != GrammaticalType::UNKNOWN) {
                    auto reqOfTheAnswer = semanticRequestType_fromSemGram(pReqLinks.gramTypeOfTheAnswer);
                    linksGramOfTheAnswer = pReqToList.getMemoryLinksAccessors(reqOfTheAnswer);
                }

                typename std::decay<decltype(pRes)>::type subSetOfMemorySentencesToConsiderAfterSubordinateFilter;
                RelationsThatMatchFilterOnChildren filter([&](const SemanticExpression& pSemExp) {
                    if (!SemExpGetter::isDefinite(pSemExp))
                        return;
                    _getRelationsFromSemExp(subSetOfMemorySentencesToConsiderAfterSubordinateFilter,
                                            matchedSemExp.res,
                                            linksGramToSemExp,
                                            pSemExp,
                                            _emptySemExpsToSkip,
                                            pRequestContext,
                                            pMemBlockPrivatePtr.mb,
                                            pLingDb,
                                            false,
                                            pReqLinks.language,
                                            pIsATrigger);
                    if (!linksGramOfTheAnswer.empty()) {
                        _getRelationsFromSemExp(subSetOfMemorySentencesToConsiderAfterSubordinateFilter,
                                                matchedSemExp.res,
                                                linksGramOfTheAnswer,
                                                pSemExp,
                                                _emptySemExpsToSkip,
                                                pRequestContext,
                                                pMemBlockPrivatePtr.mb,
                                                pLingDb,
                                                false,
                                                pReqLinks.language,
                                                pIsATrigger);
                    }
                });
                for (const auto& currSubReqList : pSubReqLinks.crossedLinks.subReqListsToFilter) {
                    filter.grammTypeOfChild = currSubReqList.gramTypeOfTheAnswer;
                    auto awLinks =
                        memBlock.getLinks(SemanticTypeOfLinks::ANSWER, currSubReqList.tense, currSubReqList.verbGoal);
                    typename std::decay<decltype(pRes)>::type semExpResultOfSubordinate;
                    getResultFromMemory(semExpResultOfSubordinate,
                                        awLinks,
                                        currSubReqList,
                                        pRequestContext,
                                        pMemBlockPrivatePtr,
                                        pIsATrigger,
                                        pLingDb,
                                        pCheckTimeRequest,
                                        pConsiderCoreferences,
                                        false,
                                        &filter);
                }
                matchedSemExp = std::move(subSetOfMemorySentencesToConsiderAfterSubordinateFilter);
            }
        }
    }

    if (pConsiderCoreferences && !pIsATrigger) {
        bool isOnlyACoreference = true;
        for (const auto& currSemExp : pSubReqLinks.semExps) {
            auto* grdexpPtr = currSemExp->getGrdExpPtr_SkipWrapperPtrs();
            if (grdexpPtr != nullptr) {
                auto* genGrdPtr = grdexpPtr->grounding().getGenericGroundingPtr();
                if (genGrdPtr != nullptr && genGrdPtr->coreference && genGrdPtr->word.isEmpty()
                    && genGrdPtr->concepts.empty())
                    continue;
            }
            isOnlyACoreference = false;
            break;
        }
        if (isOnlyACoreference)
            return;
    }

    if (pFirstIteration) {
        pFirstIteration = false;
        pRes = std::move(matchedSemExp);
    } else if (pRequestContext != RequestContext::SENTENCE_TO_CONDITION) {
        pRes = std::move(matchedSemExp);
    } else {
        MemoryModifier::semExpSetIntersectionInPlace(pRes.res, matchedSemExp.res, gramType);
    }
}

}

void findGrdExpInRecommendationLinks(std::set<const ExpressionWithLinks*>& pRes,
                                     const GroundedExpression& pGrdExp,
                                     const SemanticMemoryBlock& pMemBlock,
                                     const linguistics::LinguisticDatabase& pLingDb,
                                     SemanticLanguageEnum pLanguage) {
    SemanticMemoryBlockViewer semMemBlockViewer(nullptr, pMemBlock, SemanticAgentGrounding::userNotIdentified);
    auto& memBlockPrivate = semMemBlockViewer.getConstViewPrivate();
    auto* recLinks = memBlockPrivate.getRecommendationsTriggersLinks(_emptyAxiomId);
    if (recLinks != nullptr) {
        RelationsThatMatch<false> links;
        const SentenceLinks<false> emptyLinks;
        MemoryLinksAccessor<false> linksAccessor(recLinks, nullptr);
        _getRelationsFromGrdExp(links,
                                emptyLinks,
                                linksAccessor,
                                pGrdExp,
                                _emptySemExpsToSkip,
                                RequestContext::SENTENCE,
                                &memBlockPrivate,
                                pLingDb,
                                false,
                                pLanguage,
                                false);
        for (const auto& currLink : links.res.dynamicLinks)
            pRes.insert(&currLink.second->getContextAxiom().getSemExpWrappedForMemory());
    }
}

void findGrdExpWithCoefInRecommendationLinks(std::map<const ExpressionWithLinks*, int>& pRes,
                                             const GroundedExpression& pGrdExp,
                                             const mystd::optional<int>& pGroundingCoef,
                                             const SemanticMemoryBlock& pMemBlock,
                                             const linguistics::LinguisticDatabase& pLingDb,
                                             SemanticLanguageEnum pLanguage) {
    SemanticMemoryBlockViewer semMemBlockViewer(nullptr, pMemBlock, SemanticAgentGrounding::userNotIdentified);
    auto& memBlockPrivate = semMemBlockViewer.getConstViewPrivate();
    auto* recLinks = memBlockPrivate.getRecommendationsTriggersLinks(_emptyAxiomId);
    if (recLinks != nullptr) {
        RelationsThatMatch<false> links;
        const SentenceLinks<false> emptyLinks;
        MemoryLinksAccessor<false> linksAccessor(recLinks, nullptr);
        _getRelationsFromGrdExp(links,
                                emptyLinks,
                                linksAccessor,
                                pGrdExp,
                                _emptySemExpsToSkip,
                                RequestContext::SENTENCE,
                                nullptr,
                                pLingDb,
                                false,
                                pLanguage,
                                false);
        for (const auto& currLink : links.res.dynamicLinks) {
            int coefOfGrdExp = 0;
            if (pGroundingCoef)
                coefOfGrdExp = *pGroundingCoef;
            else if (pGrdExp->type == SemanticGroundingType::STATEMENT)
                coefOfGrdExp = 4;
            else if (SemExpGetter::isAModifierFromGrdExp(pGrdExp))
                coefOfGrdExp = 8;
            else if (pGrdExp->type != SemanticGroundingType::AGENT && pGrdExp->type != SemanticGroundingType::NAME)
                coefOfGrdExp = 10;
            else
                coefOfGrdExp = 13;

            const auto* expWrappedPtr = &currLink.second->getContextAxiom().getSemExpWrappedForMemory();
            auto itInRes = pRes.find(expWrappedPtr);
            if (itInRes != pRes.end())
                itInRes->second += coefOfGrdExp;
            else
                pRes[expWrappedPtr] = coefOfGrdExp;
        }
    }
}

template<bool IS_MODIFIABLE>
void getResultFromMemory(RelationsThatMatch<IS_MODIFIABLE>& pRes,
                         RequestToMemoryLinksVirtual<IS_MODIFIABLE>& pReqToList,
                         const semanticMemoryLinker::RequestLinks& pReqLinks,
                         RequestContext pRequestContext,
                         MemoryBlockPrivateAccessorPtr<IS_MODIFIABLE>& pMemBlockPrivate,
                         bool pIsATrigger,
                         const linguistics::LinguisticDatabase& pLingDb,
                         bool pCheckTimeRequest,
                         bool pConsiderCoreferences,
                         bool pFirstIteration,
                         const RelationsThatMatchFilter* pFilterPtr) {
    if (pRequestContext != RequestContext::SENTENCE_TO_CONDITION) {
        auto& links = pReqLinks.semExpLinksSorted();
        std::size_t size = links.size();
        std::size_t i = 1;
        for (auto& currLk : links) {
            if (pFilterPtr != nullptr) {
                if (i == size)
                    pRes.filterPtr = pFilterPtr;
                else
                    ++i;
            }
            SemanticRequestType currRequest = currLk.first;
            const auto& currSemExps = *currLk.second;
            _getResultFromLink(pRes,
                               pReqToList,
                               pFirstIteration,
                               currRequest,
                               currSemExps,
                               pRequestContext,
                               pMemBlockPrivate,
                               pLingDb,
                               pCheckTimeRequest,
                               pConsiderCoreferences,
                               pReqLinks,
                               pIsATrigger);
            if (pRes.empty())
                return;
        }
    } else {
        auto& links = pReqLinks.semExpLinks();
        std::size_t size = links.size();
        std::size_t i = 1;
        for (auto& currLk : links) {
            if (pFilterPtr != nullptr) {
                if (i == size)
                    pRes.filterPtr = pFilterPtr;
                else
                    ++i;
            }
            SemanticRequestType currRequest = currLk.first;
            const auto& currSemExps = currLk.second;
            _getResultFromLink(pRes,
                               pReqToList,
                               pFirstIteration,
                               currRequest,
                               currSemExps,
                               pRequestContext,
                               pMemBlockPrivate,
                               pLingDb,
                               pCheckTimeRequest,
                               pConsiderCoreferences,
                               pReqLinks,
                               pIsATrigger);
            if (pRes.empty())
                return;
        }
    }
}

bool getResultMatchingNowTimeFromMemory(RelationsThatMatch<true>& pRelations,
                                        const SentenceLinks<true>& pAlreadyMatchedSentences,
                                        RequestToMemoryLinks<true>& pReqToGrdExps,
                                        const SemanticDuration& pNowTimeDuration,
                                        const SemanticMemoryBlockPrivate& pMemBlockPrivate,
                                        bool pIsATrigger,
                                        const linguistics::LinguisticDatabase& pLingDb,
                                        bool pCheckChildren) {
    MemoryLinksAccessor<true> linksGramToSemExp = pReqToGrdExps.getMemoryLinksAccessors(SemanticRequestType::NOTHING);
    if (!linksGramToSemExp.empty()) {
        if (linksGramToSemExp.d != nullptr) {
            auto& pLinksToSemExps = *linksGramToSemExp.d;
            auto* timePtr = pLinksToSemExps.timeToSemExps.find_ptr(pNowTimeDuration);
            if (timePtr != nullptr) {
                IntIdToMemSentenceAccessor<true> accessor(*timePtr);
                return _addPotentialNewRelationsFromLinks(pRelations,
                                                          pAlreadyMatchedSentences,
                                                          &accessor,
                                                          nullptr,
                                                          nullptr,
                                                          _emptySemExpsToSkip,
                                                          &pMemBlockPrivate,
                                                          pIsATrigger,
                                                          pLingDb,
                                                          pCheckChildren);
            }
        }
        if (linksGramToSemExp.c != nullptr) {
            const unsigned char* timePtr = SemanticMemoryBlockBinaryReader::moveToTimePtr(linksGramToSemExp.c);
            StaticBinaryLinks nodeLinks(radixmap::read(timePtr, pNowTimeDuration.toRadixMapStr(), _hasStaticLinks));
            if (nodeLinks.linksToGrdExp != nullptr)
                return _addPotentialNewRelationsFromLinks<true>(pRelations,
                                                                pAlreadyMatchedSentences,
                                                                nullptr,
                                                                &nodeLinks,
                                                                nullptr,
                                                                _emptySemExpsToSkip,
                                                                &pMemBlockPrivate,
                                                                pIsATrigger,
                                                                pLingDb,
                                                                pCheckChildren);
        }
    }
    return false;
}

// Explicit decalrations
template void getResultFromMemory(RelationsThatMatch<true>& pRes,
                                  RequestToMemoryLinksVirtual<true>& pReqToList,
                                  const semanticMemoryLinker::RequestLinks& pReqLinks,
                                  RequestContext pRequestContext,
                                  MemoryBlockPrivateAccessorPtr<true>& pMemBlockPrivate,
                                  bool pIsATrigger,
                                  const linguistics::LinguisticDatabase& pLingDb,
                                  bool pCheckTimeRequest,
                                  bool pConsiderCoreferences,
                                  bool pFirstIteration = true,
                                  const RelationsThatMatchFilter* pFilterPtr = nullptr);
template void getResultFromMemory(RelationsThatMatch<false>& pRes,
                                  RequestToMemoryLinksVirtual<false>& pReqToList,
                                  const semanticMemoryLinker::RequestLinks& pReqLinks,
                                  RequestContext pRequestContext,
                                  MemoryBlockPrivateAccessorPtr<false>& pMemBlockPrivate,
                                  bool pIsATrigger,
                                  const linguistics::LinguisticDatabase& pLingDb,
                                  bool pCheckTimeRequest,
                                  bool pConsiderCoreferences,
                                  bool pFirstIteration = true,
                                  const RelationsThatMatchFilter* pFilterPtr = nullptr);

}    // End of namespace semanticMemoryGetter
}    // End of namespace onsem
