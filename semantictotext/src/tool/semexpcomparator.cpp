#include <onsem/semantictotext/tool/semexpcomparator.hpp>
#include <stdlib.h>
#include <onsem/common/utility/uppercasehandler.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpressions.hpp>
#include <onsem/texttosemantic/dbtype/semanticgroundings.hpp>
#include <onsem/texttosemantic/dbtype/linguisticmeaning.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/conceptset.hpp>
#include <onsem/texttosemantic/tool/semexpgetter.hpp>
#include <onsem/texttosemantic/tool/inflectionschecker.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemory.hpp>

namespace onsem {

namespace SemExpComparator {

namespace {

bool _verbTenseAreNearlyEqual(SemanticVerbTense pVerbTense1, SemanticVerbTense pVerbTense2) {
    if (isAPastTense(pVerbTense1))
        return isAPastTense(pVerbTense2);
    if (isAPresentTense(pVerbTense1))
        return isAPresentTense(pVerbTense2);
    return pVerbTense1 == pVerbTense2;
}

bool _verbGoalAreNearlyEqual(VerbGoalEnum pVerbGoal1, VerbGoalEnum pVerbGoal2) {
    if (pVerbGoal1 == VerbGoalEnum::NOTIFICATION && pVerbGoal2 == VerbGoalEnum::CONDITIONAL)
        return true;
    if (pVerbGoal2 == VerbGoalEnum::NOTIFICATION && pVerbGoal1 == VerbGoalEnum::CONDITIONAL)
        return true;
    return pVerbGoal1 == pVerbGoal2;
}

bool _isAnyHuman(const SemanticGenericGrounding& pGenGrd) {
    return pGenGrd.entityType == SemanticEntityType::HUMAN
        && (pGenGrd.quantity.type == SemanticQuantityType::ANYTHING
            || pGenGrd.quantity.type == SemanticQuantityType::EVERYTHING
            || (pGenGrd.referenceType == SemanticReferenceType::INDEFINITE && pGenGrd.quantity.isUnknown()));
}

bool _isAHuman(const SemanticGenericGrounding& pGenGrd) {
    return pGenGrd.entityType == SemanticEntityType::HUMAN && pGenGrd.referenceType == SemanticReferenceType::INDEFINITE
        && pGenGrd.quantity.isEqualToOne()
        && (ConceptSet::haveAConcept(pGenGrd.concepts, "agent_human")
            || (pGenGrd.concepts.empty() && pGenGrd.word.isEmpty()));
}

bool _isExcluded(const GroundedExpression& pGrdExp1,
                 const GroundedExpression& pGrdExp2,
                 const SemanticMemoryBlock& pMemBlock,
                 const linguistics::LinguisticDatabase& pLingDb) {
    auto itOther = pGrdExp1.children.find(GrammaticalType::OTHER_THAN);
    if (itOther != pGrdExp1.children.end())
        return semExpsAreEqualFromMemBlock(pGrdExp2, *itOther->second, pMemBlock, pLingDb, nullptr);
    return false;
}

ImbricationType _invertImbrication(ImbricationType pImbrication) {
    if (pImbrication == ImbricationType::EQUALS)
        return ImbricationType::OPPOSES;
    if (pImbrication == ImbricationType::OPPOSES)
        return ImbricationType::EQUALS;
    return ImbricationType::DIFFERS;
}

bool _arePartOfSpeechEqual(PartOfSpeech pPos1, PartOfSpeech pPos2) {
    if (pPos1 == pPos2)
        return true;
    return partOfSpeech_isNominal(pPos1) && partOfSpeech_isNominal(pPos2);
}

bool _wordsAreEqual(const SemanticWord& pWord1,
                    const SemanticWord& pWord2,
                    const linguistics::LinguisticDatabase& pLingDb) {
    if (pWord1.language == pWord2.language || pWord1.language == SemanticLanguageEnum::UNKNOWN
        || pWord2.language == SemanticLanguageEnum::UNKNOWN) {
        if (areTextEqualWithoutCaseSensitivity(pWord1.lemma, pWord2.lemma)
            && _arePartOfSpeechEqual(pWord1.partOfSpeech, pWord2.partOfSpeech))
            return true;
    } else {
        int meaningId1 = SemExpGetter::wordToMeaningId(pWord1, pWord1.language, pLingDb);
        if (meaningId1 != LinguisticMeaning_noMeaningId) {
            int meaningId2 = SemExpGetter::wordToMeaningId(pWord2, pWord1.language, pLingDb);
            if (meaningId1 == meaningId2)
                return true;
        }
    }

    if (pWord1.isReflexive())
        return _wordsAreEqual(pWord1.getRootFormFromReflexive(), pWord2, pLingDb);
    if (pWord2.isReflexive())
        return _wordsAreEqual(pWord1, pWord2.getRootFormFromReflexive(), pLingDb);
    return false;
}

const SemanticGrounding& _getSubCptGrd(const GroundedExpression& pGrdExp,
                                       const SemanticGrounding& pDefault,
                                       bool pFollowInterpretations) {
    auto it = pGrdExp.children.find(GrammaticalType::SUB_CONCEPT);
    if (it != pGrdExp.children.end()) {
        auto* grdExpPtr = it->second->getGrdExpPtr_SkipWrapperPtrs(pFollowInterpretations);
        if (grdExpPtr != nullptr)
            return grdExpPtr->grounding();
    }
    auto itSpec = pGrdExp.children.find(GrammaticalType::SPECIFIER);
    if (itSpec != pGrdExp.children.end()) {
        auto* grdExpPtr = itSpec->second->getGrdExpPtr_SkipWrapperPtrs(pFollowInterpretations);
        if (grdExpPtr != nullptr
            && (SemExpGetter::isNominal(grdExpPtr->grounding())
                || ConceptSet::haveAConcept(pGrdExp->concepts, "level")))
            return grdExpPtr->grounding();
    }
    return pDefault;
}

ImbricationType _getGenericGrdsImbrications(const SemanticGenericGrounding& pGenGrd1,
                                            const SemanticGenericGrounding& pGenGrd2,
                                            const linguistics::LinguisticDatabase& pLingDb,
                                            const ComparisonExceptions* pExceptionsPtr) {
    auto res = ImbricationType::EQUALS;
    if ((pExceptionsPtr == nullptr || !pExceptionsPtr->quantity) &&
        pGenGrd1.concepts.count("stuff") == 0 && pGenGrd2.concepts.count("stuff") == 0) {
        res = getQuantityImbrication(pGenGrd1.quantity, pGenGrd2.quantity);
        if (res != ImbricationType::EQUALS)
            return res;
    }

    if (pExceptionsPtr != nullptr && pExceptionsPtr->corefenceExcludeEquality
        && (pGenGrd1.coreference || pGenGrd2.coreference))
        return ImbricationType::DIFFERS;
    bool doesMeaningDiffers = pGenGrd1.coreference != pGenGrd2.coreference;
    if (!doesMeaningDiffers) {
        auto areCptsEqual = pLingDb.conceptSet.areConceptsCompatibles(pGenGrd1.concepts, pGenGrd2.concepts);
        doesMeaningDiffers = (!areCptsEqual && !pGenGrd1.word.isEmpty() && !pGenGrd2.word.isEmpty()
                              && !_wordsAreEqual(pGenGrd1.word, pGenGrd2.word, pLingDb))
                          || (areCptsEqual && !*areCptsEqual);
    }

    if (doesMeaningDiffers) {
        if (pLingDb.conceptSet.haveOppositeConcepts(pGenGrd1.concepts, pGenGrd2.concepts))
            return ImbricationType::OPPOSES;
        if (ConceptSet::cpts1HaveAParentCptOf2(pGenGrd1.concepts, pGenGrd2.concepts))
            return ImbricationType::CONTAINS;
        if (ConceptSet::cpts1HaveAParentCptOf2(pGenGrd2.concepts, pGenGrd1.concepts))
            return ImbricationType::ISCONTAINED;
        return ImbricationType::DIFFERS;
    }
    return res;
}

ImbricationType _getStatementGrdsImbrications(const SemanticStatementGrounding& pStatGrd1,
                                              const SemanticStatementGrounding& pStatGrd2,
                                              const linguistics::LinguisticDatabase& pLingDb,
                                              ComparisonErrorsCoef* pErrorCoefPtr,
                                              const ComparisonExceptions* pExceptionsPtr) {
    ImbricationType res = ImbricationType::EQUALS;
    const bool oppositeConcepts = pLingDb.conceptSet.haveOppositeConcepts(pStatGrd1.concepts, pStatGrd2.concepts);
    if (!_wordsAreEqual(pStatGrd1.word, pStatGrd2.word, pLingDb)) {
        auto areCptsEqual = pLingDb.conceptSet.areConceptsCompatibles(pStatGrd1.concepts, pStatGrd2.concepts);
        if (!areCptsEqual || (areCptsEqual && !*areCptsEqual && !oppositeConcepts)) {
            bool cpt1Parentof2 = ConceptSet::haveAConceptParentOf(pStatGrd1.concepts, pStatGrd2.concepts);
            bool cpt2Parentof1 = ConceptSet::haveAConceptParentOf(pStatGrd2.concepts, pStatGrd1.concepts);
            if (cpt1Parentof2 != cpt2Parentof1)
                res = cpt1Parentof2 ? ImbricationType::HYPONYM : ImbricationType::HYPERNYM;
            else
                res = ImbricationType::DIFFERS;
        }
    }

    if (res != ImbricationType::DIFFERS) {
        if ((pExceptionsPtr == nullptr || !pExceptionsPtr->request) && pStatGrd1.requests != pStatGrd2.requests) {
            if (pErrorCoefPtr != nullptr && !pStatGrd1.requests.empty() && !pStatGrd2.requests.empty()) {
                pErrorCoefPtr->value = 7;
                pErrorCoefPtr->type = ComparisonTypeOfError::REQUEST;
            }
            return ImbricationType::DIFFERS;
        }

        if ((pExceptionsPtr == nullptr || !pExceptionsPtr->verbTense)
            && !_verbTenseAreNearlyEqual(pStatGrd1.verbTense, pStatGrd2.verbTense)) {
            if (pErrorCoefPtr != nullptr) {
                pErrorCoefPtr->value = 7;
                pErrorCoefPtr->type = ComparisonTypeOfError::TENSE;
            }
            return ImbricationType::DIFFERS;
        }

        if (!_verbGoalAreNearlyEqual(pStatGrd1.verbGoal, pStatGrd2.verbGoal))
            return ImbricationType::DIFFERS;
    }

    if ((pExceptionsPtr == nullptr || !pExceptionsPtr->polarity)
        && (pStatGrd1.polarity != pStatGrd2.polarity) != oppositeConcepts)
        return _invertImbrication(res);
    return res;
}

bool _needToCheckSpecifierOfFirstGrounding(const SemanticGrounding& pGrounding1, const SemanticGrounding& pGrounding2) {
    if (pGrounding1.type == SemanticGroundingType::RELATIVELOCATION
        && pGrounding2.type != SemanticGroundingType::RELATIVELOCATION) {
        const SemanticRelativeLocationGrounding& relLocationGrd1 = pGrounding1.getRelLocationGrounding();
        if (canBeSemanticallyEqualToLocationWithoutRelativeInformation(relLocationGrd1.locationType))
            return true;
    }
    return false;
}

bool _checkSpecifierOfFirstGrdExpIfNeeded(ImbricationType& pRes,
                                          const GroundedExpression& pGrdExp1,
                                          const GroundedExpression& pGrdExp2,
                                          const SemanticMemoryBlock& pMemBlock,
                                          const linguistics::LinguisticDatabase& pLingDb,
                                          const ComparisonExceptions* pExceptionsPtr,
                                          ComparisonErrorReporting* pComparisonErrorReportingPtr,
                                          GrammaticalType pParentGrammaticalType) {
    const SemanticGrounding& grd1 = pGrdExp1.grounding();
    const SemanticGrounding& grd2 = pGrdExp2.grounding();
    if (_needToCheckSpecifierOfFirstGrounding(grd1, grd2)) {
        auto itSpecifier = pGrdExp1.children.find(GrammaticalType::SPECIFIER);
        if (itSpecifier != pGrdExp1.children.end()) {
            bool followInterpretations = pExceptionsPtr == nullptr || !pExceptionsPtr->interpretations;
            const GroundedExpression* specGrdExpPtr =
                itSpecifier->second->getGrdExpPtr_SkipWrapperPtrs(followInterpretations);
            if (specGrdExpPtr != nullptr) {
                pRes = getGrdExpsImbrications(*specGrdExpPtr,
                                              pGrdExp2,
                                              pMemBlock,
                                              pLingDb,
                                              pExceptionsPtr,
                                              pComparisonErrorReportingPtr,
                                              pParentGrammaticalType);
                return true;
            }
        }
    }
    return false;
}

bool _userIdsAreEqual(const std::string& pUserId1,
                      const std::string& pUserId2,
                      const SemanticMemoryBlock& pMemBlock,
                      const ComparisonExceptions* pExceptionsPtr) {
    if (pExceptionsPtr != nullptr && pExceptionsPtr->equivalentUserId)
        return pUserId1 == pUserId2;
    return pMemBlock.areSameUserConst(pUserId1, pUserId2);
}

bool _modifyImbricationFromOptInt(ImbricationType& pRes,
                                  const mystd::optional<int>& pOptInt1,
                                  const mystd::optional<int>& pOptInt2) {
    if (pOptInt1) {
        if (!pOptInt2) {
            if (pRes == ImbricationType::EQUALS || pRes == ImbricationType::MORE_DETAILED)
                pRes = ImbricationType::MORE_DETAILED;
            else
                return false;
        } else if (*pOptInt1 != *pOptInt2) {
            return false;
        }
    } else if (pOptInt2) {
        if (pRes == ImbricationType::EQUALS || pRes == ImbricationType::LESS_DETAILED)
            pRes = ImbricationType::LESS_DETAILED;
        else
            return false;
    }
    return true;
}

ImbricationType _compareNames(const std::vector<std::string>& pNames1, const std::vector<std::string>& pNames2) {
    for (const auto& currName1 : pNames1)
        if (std::find(pNames2.begin(), pNames2.end(), currName1) != pNames2.end())
            return ImbricationType::EQUALS;
    for (const auto& currName2 : pNames2)
        if (std::find(pNames1.begin(), pNames1.end(), currName2) != pNames1.end())
            return ImbricationType::EQUALS;
    return ImbricationType::DIFFERS;
}

ImbricationType _getGroundingsImbrications(const SemanticGrounding& pGrounding1,
                                           const SemanticGrounding& pGrounding2,
                                           const SemanticMemoryBlock& pMemBlock,
                                           const linguistics::LinguisticDatabase& pLingDb,
                                           ComparisonErrorsCoef* pErrorCoefPtr,
                                           const ComparisonExceptions* pExceptionsPtr) {
    switch (pGrounding1.type) {
        case SemanticGroundingType::GENERIC: {
            const SemanticGenericGrounding& genGrd1 = pGrounding1.getGenericGrounding();
            const SemanticGenericGrounding* genGrd2Ptr = pGrounding2.getGenericGroundingPtr();
            if (genGrd2Ptr != nullptr)
                return _getGenericGrdsImbrications(genGrd1, *genGrd2Ptr, pLingDb, pExceptionsPtr);
            if ((pGrounding2.getAgentGroundingPtr() != nullptr || pGrounding2.getNameGroundingPtr() != nullptr)) {
                auto word2 = SemExpGetter::getWord(pGrounding2);
                if (word2 != "" && areTextEqualWithoutCaseSensitivity(genGrd1.word.lemma, word2))
                    return ImbricationType::EQUALS;
                if (_isAnyHuman(genGrd1))
                    return ImbricationType::CONTAINS;
                if (_isAHuman(genGrd1))
                    return ImbricationType::HYPERNYM;
            }
            break;
        }
        case SemanticGroundingType::STATEMENT: {
            const SemanticStatementGrounding* statGrd2 = pGrounding2.getStatementGroundingPtr();
            if (statGrd2 != nullptr) {
                const auto& statGrd1 = pGrounding1.getStatementGrounding();
                return _getStatementGrdsImbrications(statGrd1, *statGrd2, pLingDb, pErrorCoefPtr, pExceptionsPtr);
            }
            break;
        }
        case SemanticGroundingType::AGENT: {
            switch (pGrounding2.type) {
                case SemanticGroundingType::AGENT: {
                    const auto& agentGrd1 = pGrounding1.getAgentGrounding();
                    const auto& agentGrd2 = pGrounding2.getAgentGrounding();
                    bool res = _userIdsAreEqual(agentGrd1.userId, agentGrd2.userId, pMemBlock, pExceptionsPtr);
                    if (!res)
                        res = agentGrd1.userIdWithoutContext == agentGrd2.userIdWithoutContext
                           && agentGrd1.userIdWithoutContext != SemanticAgentGrounding::userNotIdentified;
                    return bool_toImbricationType(res);
                }
                case SemanticGroundingType::NAME: {
                    const auto& nameInfosOpt1 = pGrounding1.getAgentGrounding().nameInfos;
                    if (nameInfosOpt1) {
                        const auto& grd2Names = pGrounding2.getNameGrounding().nameInfos.names;
                        return _compareNames(nameInfosOpt1->names, grd2Names);
                    }
                    return ImbricationType::DIFFERS;
                }
                case SemanticGroundingType::GENERIC: {
                    const SemanticGenericGrounding& genGrd2 = pGrounding2.getGenericGrounding();
                    auto word1 = SemExpGetter::getWord(pGrounding1);
                    if (word1 != "" && areTextEqualWithoutCaseSensitivity(word1, genGrd2.word.lemma))
                        return ImbricationType::EQUALS;
                    if (_isAnyHuman(genGrd2))
                        return ImbricationType::ISCONTAINED;
                    if (_isAHuman(genGrd2))
                        return ImbricationType::HYPONYM;
                    break;
                }
                default: break;
            }
            break;
        }
        case SemanticGroundingType::ANGLE: {
            const SemanticAngleGrounding* angleGrd2 = pGrounding2.getAngleGroundingPtr();
            if (angleGrd2 != nullptr)
                return bool_toImbricationType(pGrounding1.getAngleGrounding().angle == angleGrd2->angle);
            break;
        }
        case SemanticGroundingType::TIME: {
            const auto* timeGrd2Ptr = pGrounding2.getTimeGroundingPtr();
            if (timeGrd2Ptr != nullptr) {
                const auto& timeGrd1 = pGrounding1.getTimeGrounding();
                const auto& timeGrd2 = *timeGrd2Ptr;
                auto res = ImbricationType::EQUALS;
                // get date imbrication
                if (!_modifyImbricationFromOptInt(res, timeGrd1.date.year, timeGrd2.date.year)
                    || !_modifyImbricationFromOptInt(res, timeGrd1.date.month, timeGrd2.date.month)
                    || !_modifyImbricationFromOptInt(res, timeGrd1.date.day, timeGrd2.date.day)
                    || timeGrd1.length != timeGrd2.length || timeGrd1.timeOfDay != timeGrd2.timeOfDay)
                    return ImbricationType::DIFFERS;
                return res;
            } else {
                const auto* conceptualGrd2Ptr = pGrounding2.getConceptualGroundingPtr();
                if (conceptualGrd2Ptr != nullptr) {
                    const auto& timeGrd1 = pGrounding1.getTimeGrounding();
                    auto areCptsEqual =
                        pLingDb.conceptSet.areConceptsCompatibles(timeGrd1.fromConcepts, pGrounding2.concepts);
                    if (areCptsEqual) {
                        if (!*areCptsEqual)
                            return ImbricationType::DIFFERS;
                        return ImbricationType::EQUALS;
                    }
                }
            }
            break;
        }
        case SemanticGroundingType::DURATION: {
            const SemanticDurationGrounding* durationGrd2 = pGrounding2.getDurationGroundingPtr();
            if (durationGrd2 != nullptr)
                return bool_toImbricationType(pGrounding1.getDurationGrounding().duration == durationGrd2->duration);
            break;
        }
        case SemanticGroundingType::TEXT: {
            const SemanticTextGrounding* textGrd2 = pGrounding2.getTextGroundingPtr();
            if (textGrd2 != nullptr)
                return bool_toImbricationType(pGrounding1.getTextGrounding().text == textGrd2->text);
            break;
        }
        case SemanticGroundingType::LANGUAGE: {
            const SemanticLanguageGrounding* langGrd2 = pGrounding2.getLanguageGroundingPtr();
            if (langGrd2 != nullptr)
                return bool_toImbricationType(pGrounding1.getLanguageGrounding().language == langGrd2->language);
            break;
        }
        case SemanticGroundingType::RELATIVELOCATION: {
            const SemanticRelativeLocationGrounding* relLocationGrd2 = pGrounding2.getRelLocationGroundingPtr();
            if (relLocationGrd2 != nullptr)
                return bool_toImbricationType(pGrounding1.getRelLocationGrounding().locationType
                                              == relLocationGrd2->locationType);
            break;
        }
        case SemanticGroundingType::RELATIVETIME: {
            const SemanticRelativeTimeGrounding* relTimeGrd2 = pGrounding2.getRelTimeGroundingPtr();
            if (relTimeGrd2 != nullptr)
                return bool_toImbricationType(pGrounding1.getRelTimeGrounding().timeType == relTimeGrd2->timeType);
            break;
        }
        case SemanticGroundingType::RELATIVEDURATION: {
            const SemanticRelativeDurationGrounding* relDurationGrd2 = pGrounding2.getRelDurationGroundingPtr();
            if (relDurationGrd2 != nullptr)
                return bool_toImbricationType(pGrounding1.getRelDurationGrounding().durationType
                                              == relDurationGrd2->durationType);
            break;
        }
        case SemanticGroundingType::RESOURCE: {
            const SemanticResourceGrounding* resGrd2 = pGrounding2.getResourceGroundingPtr();
            if (resGrd2 != nullptr)
                return bool_toImbricationType(pGrounding1.getResourceGrounding().resource == resGrd2->resource);
            break;
        }
        case SemanticGroundingType::LENGTH: {
            const SemanticLengthGrounding* lengthGrd2 = pGrounding2.getLengthGroundingPtr();
            if (lengthGrd2 != nullptr)
                return bool_toImbricationType(pGrounding1.getLengthGrounding().length == lengthGrd2->length);
            break;
        }
        case SemanticGroundingType::META: {
            const auto& metaGrd1 = pGrounding1.getMetaGrounding();
            const SemanticMetaGrounding* metaGrd2 = pGrounding2.getMetaGroundingPtr();
            if (metaGrd2 != nullptr)
                return bool_toImbricationType(metaGrd1.refToType == metaGrd2->refToType);
            if (metaGrd1.refToType == pGrounding2.type)
                return ImbricationType::HYPERNYM;
            return ImbricationType::HYPONYM;
        }
        case SemanticGroundingType::UNITY: {
            const auto& unityGrd1 = pGrounding1.getUnityGrounding();
            const auto* unityGrd2 = pGrounding2.getUnityGroundingPtr();
            if (unityGrd2 != nullptr)
                return bool_toImbricationType(unityGrd1.typeOfUnity == unityGrd2->typeOfUnity
                                              && unityGrd1.value == unityGrd2->value);
            break;
        }
        case SemanticGroundingType::NAME: {
            switch (pGrounding2.type) {
                case SemanticGroundingType::NAME: {
                    const auto& grd1Names = pGrounding1.getNameGrounding().nameInfos.names;
                    const auto& grd2Names = pGrounding2.getNameGrounding().nameInfos.names;
                    return _compareNames(grd1Names, grd2Names);
                }
                case SemanticGroundingType::AGENT: {
                    const auto& nameInfosOpt2 = pGrounding2.getAgentGrounding().nameInfos;
                    if (nameInfosOpt2) {
                        const auto& grd1Names = pGrounding1.getNameGrounding().nameInfos.names;
                        return _compareNames(grd1Names, nameInfosOpt2->names);
                    }
                    return ImbricationType::DIFFERS;
                }
                case SemanticGroundingType::GENERIC: {
                    const SemanticGenericGrounding& genGrd2 = pGrounding2.getGenericGrounding();
                    auto word1 = SemExpGetter::getWord(pGrounding1);
                    if (word1 != "" && areTextEqualWithoutCaseSensitivity(word1, genGrd2.word.lemma))
                        return ImbricationType::EQUALS;
                    if (_isAnyHuman(genGrd2))
                        return ImbricationType::ISCONTAINED;
                    if (_isAHuman(genGrd2))
                        return ImbricationType::HYPONYM;
                    break;
                }
                default: break;
            }
            break;
        }
        case SemanticGroundingType::PERCENTAGE: {
            const SemanticPercentageGrounding* percentageGrd2 = pGrounding2.getPercentageGroundingPtr();
            if (percentageGrd2 != nullptr)
                return bool_toImbricationType(pGrounding1.getPercentageGrounding().value == percentageGrd2->value);
            break;
        }
        case SemanticGroundingType::CONCEPTUAL: break;    // The concepts will be checked after
    }

    const SemanticMetaGrounding* metaGrd2 = pGrounding2.getMetaGroundingPtr();
    if (metaGrd2 != nullptr) {
        if (metaGrd2->refToType == pGrounding1.type)
            return ImbricationType::HYPONYM;
        return ImbricationType::HYPERNYM;
    } else {
        const auto* timeGrd2Ptr = pGrounding2.getTimeGroundingPtr();
        if (timeGrd2Ptr != nullptr) {
            auto areCptsEqual =
                pLingDb.conceptSet.areConceptsCompatibles(pGrounding1.concepts, timeGrd2Ptr->fromConcepts);
            if (areCptsEqual) {
                if (!*areCptsEqual)
                    return ImbricationType::DIFFERS;
                return ImbricationType::EQUALS;
            }
        }
    }

    auto areCptsEqual = pLingDb.conceptSet.areConceptsCompatibles(pGrounding1.concepts, pGrounding2.concepts);
    if (areCptsEqual) {
        if (!*areCptsEqual) {
            if (pLingDb.conceptSet.haveOppositeConcepts(pGrounding1.concepts, pGrounding2.concepts))
                return ImbricationType::OPPOSES;
            return ImbricationType::DIFFERS;
        }
        return ImbricationType::EQUALS;
    }
    return ImbricationType::DIFFERS;
}

ImbricationType _mergeChildImbrications(ImbricationType pChildImbrication1, ImbricationType pChildImbrication2) {
    if (pChildImbrication1 == pChildImbrication2)
        return pChildImbrication1;
    if (pChildImbrication1 == ImbricationType::EQUALS)
        return pChildImbrication2;
    if (pChildImbrication2 == ImbricationType::EQUALS)
        return pChildImbrication1;
    if (pChildImbrication1 == ImbricationType::OPPOSES || pChildImbrication2 == ImbricationType::OPPOSES)
        return ImbricationType::DIFFERS;
    if (pChildImbrication1 == ImbricationType::CONTAINS && pChildImbrication2 != ImbricationType::ISCONTAINED)
        return pChildImbrication2;
    if (pChildImbrication2 == ImbricationType::CONTAINS && pChildImbrication1 != ImbricationType::ISCONTAINED)
        return pChildImbrication1;
    if (pChildImbrication1 == ImbricationType::ISCONTAINED && pChildImbrication2 != ImbricationType::CONTAINS)
        return pChildImbrication2;
    if (pChildImbrication2 == ImbricationType::ISCONTAINED && pChildImbrication1 != ImbricationType::CONTAINS)
        return pChildImbrication1;
    if (pChildImbrication1 == ImbricationType::HYPERNYM && pChildImbrication2 == ImbricationType::LESS_DETAILED)
        return pChildImbrication2;
    if (pChildImbrication2 == ImbricationType::HYPERNYM && pChildImbrication1 == ImbricationType::LESS_DETAILED)
        return pChildImbrication1;
    if (pChildImbrication1 == ImbricationType::HYPONYM && pChildImbrication2 == ImbricationType::MORE_DETAILED)
        return pChildImbrication2;
    if (pChildImbrication2 == ImbricationType::HYPONYM && pChildImbrication1 == ImbricationType::MORE_DETAILED)
        return pChildImbrication1;
    return ImbricationType::DIFFERS;
}

void _addSemExpPtr(std::list<const SemanticExpression*>& pElts,
                   const SemanticExpression& pSemExp,
                   const ComparisonExceptions* pExceptionsPtr,
                   bool pFirstOrSecondArg) {
    if (pExceptionsPtr != nullptr
        && ((pFirstOrSecondArg && pExceptionsPtr->semExps1ToSkip.count(&pSemExp) > 0)
            || (!pFirstOrSecondArg && pExceptionsPtr->semExps2ToSkip.count(&pSemExp) > 0)))
        return;
    pElts.emplace_back(&pSemExp);
}

void _fillListExpPtr(ListExpPtr& pListExpPtr,
                     const SemanticExpression& pSemExp,
                     const ComparisonExceptions* pExceptionsPtr,
                     bool pFirstOrSecondArg) {
    bool followInterpretations = pExceptionsPtr == nullptr || !pExceptionsPtr->interpretations;
    auto* listExpPtr = pSemExp.getListExpPtr_SkipWrapperPtrs(followInterpretations);
    if (listExpPtr != nullptr) {
        for (const auto& currElt : listExpPtr->elts)
            _fillListExpPtr(pListExpPtr, *currElt, pExceptionsPtr, pFirstOrSecondArg);
        pListExpPtr.listType.emplace(listExpPtr->listType);
        return;
    }
    _addSemExpPtr(pListExpPtr.elts, pSemExp, pExceptionsPtr, pFirstOrSecondArg);
}

void _getConvertResultIfSemExpEqualTheFeedbackOfTheOther(mystd::optional<ImbricationType>& pImbricationType,
                                                         bool pDirection) {
    if (pImbricationType) {
        auto imbricationIfEqual = pDirection ? ImbricationType::LESS_DETAILED : ImbricationType::MORE_DETAILED;
        if (*pImbricationType == ImbricationType::EQUALS)
            pImbricationType.emplace(imbricationIfEqual);
        else if (*pImbricationType != imbricationIfEqual && *pImbricationType != ImbricationType::OPPOSES)
            pImbricationType.reset();
    }
}

mystd::optional<ImbricationType> _getSemExpsWithoutListImbrications(
    const SemanticExpression& pSemExp1,
    const SemanticExpression& pSemExp2,
    const SemanticMemoryBlock& pMemBlock,
    const linguistics::LinguisticDatabase& pLingDb,
    const ComparisonExceptions* pExceptionsPtr,
    ComparisonErrorReporting* pComparisonErrorReportingPtr,
    GrammaticalType pParentGrammaticalType) {
    switch (pSemExp1.type) {
        case SemanticExpressionType::GROUNDED: {
            switch (pSemExp2.type) {
                case SemanticExpressionType::GROUNDED: {
                    auto& grdExp2 = pSemExp2.getGrdExp();
                    if (pExceptionsPtr == nullptr
                        || (pExceptionsPtr->semExps1ToSkip.count(&pSemExp1) == 0
                            && pExceptionsPtr->semExps2ToSkip.count(&pSemExp2) == 0)) {
                        auto& grdExp1 = pSemExp1.getGrdExp();
                        return getGrdExpsImbrications(grdExp1,
                                                      grdExp2,
                                                      pMemBlock,
                                                      pLingDb,
                                                      pExceptionsPtr,
                                                      pComparisonErrorReportingPtr,
                                                      pParentGrammaticalType);
                    }
                    break;
                }
                case SemanticExpressionType::INTERPRETATION: {
                    if (pExceptionsPtr == nullptr || !pExceptionsPtr->interpretations) {
                        auto& intExp = *pSemExp2.getIntExp().interpretedExp;
                        return _getSemExpsWithoutListImbrications(pSemExp1,
                                                                  intExp,
                                                                  pMemBlock,
                                                                  pLingDb,
                                                                  pExceptionsPtr,
                                                                  pComparisonErrorReportingPtr,
                                                                  pParentGrammaticalType);
                    }
                    auto& originalExp = *pSemExp2.getIntExp().originalExp;
                    return _getSemExpsWithoutListImbrications(pSemExp1,
                                                              originalExp,
                                                              pMemBlock,
                                                              pLingDb,
                                                              pExceptionsPtr,
                                                              pComparisonErrorReportingPtr,
                                                              pParentGrammaticalType);
                }
                case SemanticExpressionType::FEEDBACK: {
                    auto& fdkExp = pSemExp2.getFdkExp();
                    auto subResult = _getSemExpsWithoutListImbrications(pSemExp1,
                                                                        *fdkExp.feedbackExp,
                                                                        pMemBlock,
                                                                        pLingDb,
                                                                        pExceptionsPtr,
                                                                        pComparisonErrorReportingPtr,
                                                                        pParentGrammaticalType);
                    _getConvertResultIfSemExpEqualTheFeedbackOfTheOther(subResult, true);
                    if (subResult)
                        return subResult;
                    auto& concernedExp = *fdkExp.concernedExp;
                    return _getSemExpsWithoutListImbrications(pSemExp1,
                                                              concernedExp,
                                                              pMemBlock,
                                                              pLingDb,
                                                              pExceptionsPtr,
                                                              pComparisonErrorReportingPtr,
                                                              pParentGrammaticalType);
                }
                case SemanticExpressionType::ANNOTATED: {
                    auto& semExp = *pSemExp2.getAnnExp().semExp;
                    return _getSemExpsWithoutListImbrications(pSemExp1,
                                                              semExp,
                                                              pMemBlock,
                                                              pLingDb,
                                                              pExceptionsPtr,
                                                              pComparisonErrorReportingPtr,
                                                              pParentGrammaticalType);
                }
                case SemanticExpressionType::METADATA: {
                    auto& semExp = *pSemExp2.getMetadataExp().semExp;
                    return _getSemExpsWithoutListImbrications(pSemExp1,
                                                              semExp,
                                                              pMemBlock,
                                                              pLingDb,
                                                              pExceptionsPtr,
                                                              pComparisonErrorReportingPtr,
                                                              pParentGrammaticalType);
                }
                case SemanticExpressionType::COMMAND: {
                    auto& semExp = *pSemExp2.getCmdExp().semExp;
                    return _getSemExpsWithoutListImbrications(pSemExp1,
                                                              semExp,
                                                              pMemBlock,
                                                              pLingDb,
                                                              pExceptionsPtr,
                                                              pComparisonErrorReportingPtr,
                                                              pParentGrammaticalType);
                }
                case SemanticExpressionType::SETOFFORMS: {
                    UniqueSemanticExpression* originalFrom = pSemExp2.getSetOfFormsExp().getOriginalForm();
                    if (originalFrom != nullptr)
                        return _getSemExpsWithoutListImbrications(pSemExp1,
                                                                  **originalFrom,
                                                                  pMemBlock,
                                                                  pLingDb,
                                                                  pExceptionsPtr,
                                                                  pComparisonErrorReportingPtr,
                                                                  pParentGrammaticalType);
                    break;
                }
                case SemanticExpressionType::FIXEDSYNTHESIS: {
                    auto* semExp = pSemExp2.getFSynthExp().getSemExpPtr();
                    if (semExp != nullptr)
                        return _getSemExpsWithoutListImbrications(pSemExp1,
                                                                  *semExp,
                                                                  pMemBlock,
                                                                  pLingDb,
                                                                  pExceptionsPtr,
                                                                  pComparisonErrorReportingPtr,
                                                                  pParentGrammaticalType);
                    break;
                }
                case SemanticExpressionType::COMPARISON:
                case SemanticExpressionType::CONDITION:
                case SemanticExpressionType::LIST: break;
            }
            break;
        }
        case SemanticExpressionType::INTERPRETATION: {
            if (pExceptionsPtr == nullptr || !pExceptionsPtr->interpretations) {
                auto& interpretedExp = *pSemExp1.getIntExp().interpretedExp;
                return _getSemExpsWithoutListImbrications(interpretedExp,
                                                          pSemExp2,
                                                          pMemBlock,
                                                          pLingDb,
                                                          pExceptionsPtr,
                                                          pComparisonErrorReportingPtr,
                                                          pParentGrammaticalType);
            }
            auto& originalExp = *pSemExp1.getIntExp().originalExp;
            return _getSemExpsWithoutListImbrications(originalExp,
                                                      pSemExp2,
                                                      pMemBlock,
                                                      pLingDb,
                                                      pExceptionsPtr,
                                                      pComparisonErrorReportingPtr,
                                                      pParentGrammaticalType);
        }
        case SemanticExpressionType::FEEDBACK: {
            auto& fdkExp = pSemExp1.getFdkExp();
            auto subResult = _getSemExpsWithoutListImbrications(*fdkExp.feedbackExp,
                                                                pSemExp2,
                                                                pMemBlock,
                                                                pLingDb,
                                                                pExceptionsPtr,
                                                                pComparisonErrorReportingPtr,
                                                                pParentGrammaticalType);
            _getConvertResultIfSemExpEqualTheFeedbackOfTheOther(subResult, false);
            if (subResult)
                return subResult;
            auto& concernedExp = *fdkExp.concernedExp;
            return _getSemExpsWithoutListImbrications(concernedExp,
                                                      pSemExp2,
                                                      pMemBlock,
                                                      pLingDb,
                                                      pExceptionsPtr,
                                                      pComparisonErrorReportingPtr,
                                                      pParentGrammaticalType);
        }
        case SemanticExpressionType::ANNOTATED: {
            auto& semExp = *pSemExp1.getAnnExp().semExp;
            return _getSemExpsWithoutListImbrications(semExp,
                                                      pSemExp2,
                                                      pMemBlock,
                                                      pLingDb,
                                                      pExceptionsPtr,
                                                      pComparisonErrorReportingPtr,
                                                      pParentGrammaticalType);
        }
        case SemanticExpressionType::METADATA: {
            auto& semExp = *pSemExp1.getMetadataExp().semExp;
            return _getSemExpsWithoutListImbrications(semExp,
                                                      pSemExp2,
                                                      pMemBlock,
                                                      pLingDb,
                                                      pExceptionsPtr,
                                                      pComparisonErrorReportingPtr,
                                                      pParentGrammaticalType);
        }
        case SemanticExpressionType::COMMAND: {
            auto& cmdExp = *pSemExp1.getCmdExp().semExp;
            return _getSemExpsWithoutListImbrications(cmdExp,
                                                      pSemExp2,
                                                      pMemBlock,
                                                      pLingDb,
                                                      pExceptionsPtr,
                                                      pComparisonErrorReportingPtr,
                                                      pParentGrammaticalType);
        }
        case SemanticExpressionType::SETOFFORMS: {
            UniqueSemanticExpression* originalFrom = pSemExp1.getSetOfFormsExp().getOriginalForm();
            if (originalFrom != nullptr)
                return _getSemExpsWithoutListImbrications(**originalFrom,
                                                          pSemExp2,
                                                          pMemBlock,
                                                          pLingDb,
                                                          pExceptionsPtr,
                                                          pComparisonErrorReportingPtr,
                                                          pParentGrammaticalType);
            break;
        }
        case SemanticExpressionType::FIXEDSYNTHESIS: {
            auto* semExp = pSemExp1.getFSynthExp().getSemExpPtr();
            if (semExp != nullptr)
                return _getSemExpsWithoutListImbrications(*semExp,
                                                          pSemExp2,
                                                          pMemBlock,
                                                          pLingDb,
                                                          pExceptionsPtr,
                                                          pComparisonErrorReportingPtr,
                                                          pParentGrammaticalType);
            break;
        }
        case SemanticExpressionType::COMPARISON: break;
        case SemanticExpressionType::LIST: break;
        case SemanticExpressionType::CONDITION: {
            auto& condExp1 = pSemExp1.getCondExp();
            const ConditionExpression* condExp2 = pSemExp2.getCondExpPtr_SkipWrapperPtrs();
            if (condExp2 != nullptr
                && (pExceptionsPtr == nullptr
                    || (pExceptionsPtr->semExps1ToSkip.count(&pSemExp1) == 0
                        && pExceptionsPtr->semExps2ToSkip.count(&pSemExp2) == 0))) {
                auto resCond = getSemExpsImbrications(*condExp1.conditionExp,
                                                      *condExp2->conditionExp,
                                                      pMemBlock,
                                                      pLingDb,
                                                      pExceptionsPtr,
                                                      pComparisonErrorReportingPtr,
                                                      pParentGrammaticalType);
                if (resCond == ImbricationType::DIFFERS)
                    return resCond;
                auto resThen = getSemExpsImbrications(*condExp1.thenExp,
                                                      *condExp2->thenExp,
                                                      pMemBlock,
                                                      pLingDb,
                                                      pExceptionsPtr,
                                                      pComparisonErrorReportingPtr,
                                                      pParentGrammaticalType);
                if (resThen == ImbricationType::DIFFERS)
                    return resThen;
                if (resThen == ImbricationType::EQUALS)
                    resThen = resCond;

                if (condExp1.elseExp) {
                    if (condExp2->elseExp) {
                        auto resElse = getSemExpsImbrications(*condExp1.thenExp,
                                                              *condExp2->thenExp,
                                                              pMemBlock,
                                                              pLingDb,
                                                              pExceptionsPtr,
                                                              pComparisonErrorReportingPtr,
                                                              pParentGrammaticalType);
                        return resElse == ImbricationType::EQUALS ? resThen : resElse;
                    } else {
                        return ImbricationType::MORE_DETAILED;
                    }
                } else if (condExp2->elseExp) {
                    return ImbricationType::LESS_DETAILED;
                }
                return resThen;
            }
        }
    }
    return mystd::optional<ImbricationType>();
}

bool _hasInformationToFill(const SemanticExpression& pSemExp, bool pFollowInterpretations) {
    auto* grdExpPtr = pSemExp.getGrdExpPtr_SkipWrapperPtrs(pFollowInterpretations);
    return grdExpPtr != nullptr && grdExpPtr->grounding().concepts.count("stuff_informationToFill") > 0;
}

bool _hasInformationToFillFromListExpPtr(const ListExpPtr& pListExpPtr, bool pFollowInterpretations) {
    for (const auto& currElt : pListExpPtr.elts) {
        if (_hasInformationToFill(*currElt, pFollowInterpretations))
            return true;
    }
    return false;
}

bool _hasOnlyInformationToFillFromListExpPtr(const ListExpPtr& pListExpPtr, bool pFollowInterpretations) {
    bool res = false;
    for (const auto& currElt : pListExpPtr.elts) {
        if (!_hasInformationToFill(*currElt, pFollowInterpretations))
            return false;
        res = true;
    }
    return res;
}

ComparisonErrorsCoef _getErrorCoefFromListExpPtr(GrammaticalType pGrammType,
                                                 const ListExpPtr& pListExpPtr,
                                                 bool pFollowInterpretations) {
    if (_hasOnlyInformationToFillFromListExpPtr(pListExpPtr, pFollowInterpretations))
        return ComparisonErrorsCoef(1, ComparisonTypeOfError::PARAMETER_DIFF);
    if (pGrammType == GrammaticalType::SPECIFIER || pGrammType == GrammaticalType::OWNER
        || pGrammType == GrammaticalType::OTHER_THAN)
        return ComparisonErrorsCoef(5, ComparisonTypeOfError::SPECIFIER);
    return ComparisonErrorsCoef(10, ComparisonTypeOfError::NORMAL);
}

ImbricationType _getListExpsImbrications(const ListExpPtr& pListExpPtr1,
                                         const ListExpPtr& pListExpPtr2,
                                         const SemanticMemoryBlock& pMemBlock,
                                         const linguistics::LinguisticDatabase& pLingDb,
                                         const ComparisonExceptions* pExceptionsPtr,
                                         ComparisonErrorsCoef* pComparisonErrorsCoefPtr,
                                         GrammaticalType pParentGrammaticalType,
                                         std::size_t* pNumberOfEqualitiesPtr) {
    auto res = ImbricationType::EQUALS;
    for (const auto& currEltList1 : pListExpPtr1.elts) {
        bool found = false;
        mystd::optional<ImbricationType> currentImbricationType;
        ComparisonErrorsCoef smallerSubComparisonErrorsCoef;
        for (const auto& currEltList2 : pListExpPtr2.elts) {
            ComparisonErrorReporting subComparisonErrorReporting;
            auto optRes = _getSemExpsWithoutListImbrications(*currEltList1,
                                                             *currEltList2,
                                                             pMemBlock,
                                                             pLingDb,
                                                             pExceptionsPtr,
                                                             &subComparisonErrorReporting,
                                                             pParentGrammaticalType);
            if (optRes && *optRes == ImbricationType::EQUALS) {
                if (pNumberOfEqualitiesPtr != nullptr)
                    ++(*pNumberOfEqualitiesPtr);
                found = true;
                break;
            }

            auto newCoef = subComparisonErrorReporting.getErrorCoef();
            if (smallerSubComparisonErrorsCoef.type <= ComparisonTypeOfError::PARAMETER_DIFF
                || newCoef > smallerSubComparisonErrorsCoef) {
                smallerSubComparisonErrorsCoef = newCoef;
                if (optRes)
                    currentImbricationType.emplace(*optRes);
            }
        }
        if (!found) {
            if (currentImbricationType)
                res = *currentImbricationType;
            else
                res = ImbricationType::DIFFERS;
            if (pComparisonErrorsCoefPtr == nullptr)
                return res;
            pComparisonErrorsCoefPtr->add(smallerSubComparisonErrorsCoef);
        }
    }
    return res;
}

}

ComparisonErrorReporting::SmimilarityValue ComparisonErrorReporting::canBeConsideredHasSimilar() const {
    for (auto& currGramChild : childrenThatAreNotEqual) {
        for (auto& currImbrication : currGramChild.second) {
            if (currImbrication.second.errorCoef.type == ComparisonTypeOfError::PARAMETER_DIFF)
                continue;

            switch (currGramChild.first) {
                case GrammaticalType::SPECIFIER: {
                    if (currImbrication.first == ImbricationType::DIFFERS
                        && currImbrication.second.errorCoef.type == ComparisonTypeOfError::NORMAL)
                        return ComparisonErrorReporting::SmimilarityValue::NO;
                    break;
                }
                case GrammaticalType::OBJECT: {
                    if (currImbrication.first == ImbricationType::DIFFERS
                        && currImbrication.second.errorCoef.type != ComparisonTypeOfError::SPECIFIER)
                        return ComparisonErrorReporting::SmimilarityValue::NO;

                    if (currImbrication.first == ImbricationType::LESS_DETAILED
                        && currImbrication.second.child1Ptr.elts.empty())
                        return ComparisonErrorReporting::SmimilarityValue::YES_BUT_INCOMPLETE;
                    break;
                }
                case GrammaticalType::OWNER: {
                    if (currImbrication.first != ImbricationType::LESS_DETAILED
                        && currImbrication.first != ImbricationType::MORE_DETAILED)
                        return ComparisonErrorReporting::SmimilarityValue::NO;
                    break;
                }
                case GrammaticalType::UNKNOWN: {
                    if (currImbrication.second.errorCoef.type == ComparisonTypeOfError::REQUEST)
                        return ComparisonErrorReporting::SmimilarityValue::NO;
                    break;
                }
                default: break;
            }
        }
    }
    return ComparisonErrorReporting::SmimilarityValue::YES;
}

ImbricationType getQuantityImbrication(const SemanticQuantity& pQuantity1, const SemanticQuantity& pQuantity2) {
    if (pQuantity1.type != pQuantity2.type) {
        if (pQuantity1.type == SemanticQuantityType::EVERYTHING)
            return ImbricationType::CONTAINS;
        if (pQuantity2.type == SemanticQuantityType::EVERYTHING)
            return ImbricationType::ISCONTAINED;
        if ((pQuantity1.type == SemanticQuantityType::UNKNOWN && pQuantity2.type == SemanticQuantityType::NUMBER
             && pQuantity2.nb == 1)
            || (pQuantity2.type == SemanticQuantityType::UNKNOWN && pQuantity1.type == SemanticQuantityType::NUMBER
                && pQuantity1.nb == 1))
            return ImbricationType::EQUALS;
        return ImbricationType::DIFFERS;
    }

    if (pQuantity1.type == SemanticQuantityType::NUMBER
        || pQuantity1.type == SemanticQuantityType::MOREOREQUALTHANNUMBER) {
        if (pQuantity1.nb == pQuantity2.nb)
            return ImbricationType::EQUALS;
        else
            return ImbricationType::DIFFERS;
    }
    return ImbricationType::EQUALS;
}

ImbricationType switchOrderOfEltsImbrication(ImbricationType pImbrication) {
    switch (pImbrication) {
        case ImbricationType::EQUALS:
        case ImbricationType::DIFFERS:
        case ImbricationType::OPPOSES: return pImbrication;
        case ImbricationType::CONTAINS: return ImbricationType::ISCONTAINED;
        case ImbricationType::ISCONTAINED: return ImbricationType::CONTAINS;
        case ImbricationType::MORE_DETAILED: return ImbricationType::LESS_DETAILED;
        case ImbricationType::LESS_DETAILED: return ImbricationType::MORE_DETAILED;
        case ImbricationType::HYPONYM: return ImbricationType::HYPERNYM;
        case ImbricationType::HYPERNYM: return ImbricationType::HYPONYM;
    }
    return pImbrication;
}

bool grdHaveNbSetToZero(const SemanticGrounding& pGrd) {
    const SemanticGenericGrounding* genGrd = pGrd.getGenericGroundingPtr();
    return genGrd != nullptr && genGrd->quantity.type == SemanticQuantityType::NUMBER && genGrd->quantity.nb == 0;
}

bool grdsHaveSamePolarity(const SemanticGrounding& pGrd1,
                          const SemanticGrounding& pGrd2,
                          const ConceptSet& pConceptsDb) {
    if ((pGrd1.type == SemanticGroundingType::STATEMENT && pGrd2.type == SemanticGroundingType::STATEMENT)
        || (pGrd1.type != SemanticGroundingType::STATEMENT && pGrd2.type != SemanticGroundingType::STATEMENT)) {
        if (pConceptsDb.haveOppositeConcepts(pGrd1.concepts, pGrd2.concepts))
            return pGrd1.polarity != pGrd2.polarity;
        return pGrd1.polarity == pGrd2.polarity;
    }
    return true;
}

bool haveSamePolarity(const GroundedExpression& pGrdExp1,
                      const GroundedExpression& pGrdExp2,
                      const ConceptSet& pConceptsDb,
                      bool pFollowInterpretations) {
    const SemanticGrounding& pGrd1 = pGrdExp1.grounding();
    const SemanticGrounding& pGrd2 = pGrdExp2.grounding();
    bool samePolarity = grdsHaveSamePolarity(pGrd1, pGrd2, pConceptsDb);
    bool bothAreNotASentence =
        pGrd1.type != SemanticGroundingType::STATEMENT && pGrd2.type != SemanticGroundingType::STATEMENT;
    if (bothAreNotASentence
        && (grdHaveNbSetToZero(pGrd1) && pGrdExp1.children.count(GrammaticalType::OTHER_THAN) == 0)
               != (grdHaveNbSetToZero(pGrd2) && pGrdExp2.children.count(GrammaticalType::OTHER_THAN) == 0))
        samePolarity = !samePolarity;

    for (const auto& child1 : pGrdExp1.children) {
        const GroundedExpression* grdExp1 = child1.second->getGrdExpPtr_SkipWrapperPtrs(pFollowInterpretations);
        const auto itChild2 = pGrdExp2.children.find(child1.first);
        if (itChild2 != pGrdExp2.children.end()) {
            const GroundedExpression* grdExp2 = itChild2->second->getGrdExpPtr_SkipWrapperPtrs(pFollowInterpretations);
            if (grdExp1 != nullptr && grdExp2 != nullptr) {
                samePolarity =
                    (samePolarity == haveSamePolarity(*grdExp1, *grdExp2, pConceptsDb, pFollowInterpretations));
            } else if (grdExp1 != nullptr) {
                if (grdHaveNbSetToZero(grdExp1->grounding()))
                    samePolarity = !samePolarity;
            } else if (grdExp2 != nullptr) {
                if (grdHaveNbSetToZero(grdExp2->grounding()))
                    samePolarity = !samePolarity;
            }
        } else if (grdExp1 != nullptr) {
            if (grdHaveNbSetToZero(grdExp1->grounding()))
                samePolarity = !samePolarity;
        }
    }

    for (const auto& child2 : pGrdExp2.children) {
        if (pGrdExp1.children.find(child2.first) == pGrdExp1.children.end()) {
            const GroundedExpression* grdExp2 = child2.second->getGrdExpPtr_SkipWrapperPtrs(pFollowInterpretations);
            if (grdExp2 != nullptr && grdHaveNbSetToZero(grdExp2->grounding()))
                samePolarity = !samePolarity;
        }
    }

    return samePolarity;
}

bool doesGrdExpContainEverything(const GroundedExpression& pGrdExp) {
    const auto* genGrdPtr = pGrdExp->getGenericGroundingPtr();
    return genGrdPtr != nullptr && genGrdPtr->word.isEmpty() && genGrdPtr->concepts.empty()
        && (genGrdPtr->entityType == SemanticEntityType::THING || genGrdPtr->entityType == SemanticEntityType::UNKNOWN);
}

bool isAnInstanceOf(const GroundedExpression& pInstanceOfMetaDesc,
                    const GroundedExpression& pMetaDesc,
                    const linguistics::LinguisticDatabase& pLingDb) {
    if (doesGrdExpContainEverything(pMetaDesc))
        return true;
    const SemanticGrounding& metaDescGrounding = pMetaDesc.grounding();
    if (ConceptSet::haveAConceptOrAHyponym(metaDescGrounding.concepts, "stuff"))
        return true;

    for (const auto& currCpt : pInstanceOfMetaDesc.grounding().concepts) {
        std::vector<std::string> parentConcepts;
        ConceptSet::conceptToParentConcepts(parentConcepts, currCpt.first);
        if (ConceptSet::haveAnyOfConcepts(metaDescGrounding.concepts, parentConcepts))
            return true;
    }
    auto* instGenGrdPtr = pInstanceOfMetaDesc->getGenericGroundingPtr();
    if (instGenGrdPtr == nullptr)
        return false;
    auto* metaGenGrdPtr = pMetaDesc->getGenericGroundingPtr();
    if (metaGenGrdPtr == nullptr)
        return false;
    return _getGenericGrdsImbrications(*instGenGrdPtr, *metaGenGrdPtr, pLingDb, nullptr) == ImbricationType::EQUALS
        && pInstanceOfMetaDesc.children.count(GrammaticalType::SPECIFIER) > 0
        && pMetaDesc.children.count(GrammaticalType::SPECIFIER) == 0;
}

bool areGrdExpEqualsExceptForTheQuantity(const GroundedExpression& pGrdExp1,
                                         const GroundedExpression& pGrdExp2,
                                         const ConceptSet& pConceptSet) {
    const auto& grd1 = pGrdExp1.grounding();
    const auto& grd2 = pGrdExp2.grounding();

    auto areCptsEqual = pConceptSet.areConceptsCompatibles(grd1.concepts, grd2.concepts);
    if (areCptsEqual && *areCptsEqual)
        return true;

    auto grd1EquTogr2ExeptForQuantity = [](const SemanticGrounding& pGrd1, const SemanticGrounding& pGrd2) {
        const SemanticGenericGrounding* genGrd1Ptr = pGrd1.getGenericGroundingPtr();
        const SemanticGenericGrounding* genGrd2Ptr = pGrd2.getGenericGroundingPtr();
        if (genGrd1Ptr != nullptr) {
            if (genGrd2Ptr != nullptr)
                return !genGrd1Ptr->word.isEmpty() && genGrd1Ptr->word == genGrd2Ptr->word;
            if (genGrd1Ptr->entityType == SemanticEntityType::HUMAN)
                return pGrd2.getAgentGroundingPtr() != nullptr || pGrd2.getNameGroundingPtr() != nullptr;
        }
        return false;
    };
    return grd1EquTogr2ExeptForQuantity(grd1, grd2) || grd1EquTogr2ExeptForQuantity(grd2, grd1);
}

bool semExpsAreEqual(const SemanticExpression& pSemExp1,
                     const SemanticExpression& pSemExp2,
                     const SemanticMemory& pSemanticMemory,
                     const linguistics::LinguisticDatabase& pLingDb) {
    return semExpsAreEqual(pSemExp1, pSemExp2, pSemanticMemory.memBloc, pLingDb);
}

bool semExpsAreEqual(const SemanticExpression& pSemExp1,
                     const SemanticExpression& pSemExp2,
                     const SemanticMemoryBlock& pMemBlock,
                     const linguistics::LinguisticDatabase& pLingDb,
                     const ComparisonExceptions* pExceptionsPtr) {
    return semExpsAreEqualFromMemBlock(pSemExp1, pSemExp2, pMemBlock, pLingDb, pExceptionsPtr);
}

ImbricationType getSemExpsImbrications(const SemanticExpression& pSemExp1,
                                       const SemanticExpression& pSemExp2,
                                       const SemanticMemoryBlock& pMemBlock,
                                       const linguistics::LinguisticDatabase& pLingDb,
                                       const ComparisonExceptions* pExceptionsPtr,
                                       ComparisonErrorReporting* pComparisonErrorReportingPtr,
                                       GrammaticalType pParentGrammaticalType) {
    auto optRes = _getSemExpsWithoutListImbrications(
        pSemExp1, pSemExp2, pMemBlock, pLingDb, pExceptionsPtr, pComparisonErrorReportingPtr, pParentGrammaticalType);
    if (optRes)
        return *optRes;

    ListExpPtr listExpPtr1;
    _fillListExpPtr(listExpPtr1, pSemExp1, pExceptionsPtr, true);
    ListExpPtr listExpPtr2;
    _fillListExpPtr(listExpPtr2, pSemExp2, pExceptionsPtr, false);

    bool followInterpretations = pExceptionsPtr == nullptr || !pExceptionsPtr->interpretations;
    const std::size_t size1 = listExpPtr1.elts.size();
    const std::size_t size2 = listExpPtr2.elts.size();
    const bool hasInformationToFill1 = _hasInformationToFillFromListExpPtr(listExpPtr1, followInterpretations);
    const bool hasInformationToFill2 = _hasInformationToFillFromListExpPtr(listExpPtr2, followInterpretations);
    const bool hasInformationToFill = hasInformationToFill1 || hasInformationToFill2;
    ComparisonErrorsCoef errorCoef(std::abs(static_cast<int>(size1) - static_cast<int>(size2)),
                                   ComparisonTypeOfError::NORMAL);
    if (size1 > size2) {
        if (hasInformationToFill1)
            errorCoef.type = ComparisonTypeOfError::PARAMETER_DIFF;
        else
            errorCoef.value *= 10;
    } else {
        if (hasInformationToFill2)
            errorCoef.type = ComparisonTypeOfError::PARAMETER_DIFF;
        else
            errorCoef.value *= 10;
    }
    if ((listExpPtr1.listType.has_value() || listExpPtr2.listType.has_value())
        && listExpPtr1.listType != listExpPtr2.listType
        && (listExpPtr1.listType == ListExpressionType::OR || listExpPtr1.listType == ListExpressionType::THEN
            || listExpPtr1.listType == ListExpressionType::THEN_REVERSED
            || listExpPtr2.listType == ListExpressionType::OR || listExpPtr2.listType == ListExpressionType::THEN
            || listExpPtr2.listType == ListExpressionType::THEN_REVERSED)) {
        if (pComparisonErrorReportingPtr != nullptr) {
            if (hasInformationToFill) /* Addition for list type difference */
                errorCoef.value += 1;
            else
                errorCoef.value += 10;
            pComparisonErrorReportingPtr->addError(
                pParentGrammaticalType, ImbricationType::DIFFERS, listExpPtr1, listExpPtr2, errorCoef);
        }
        return ImbricationType::DIFFERS;
    }

    ComparisonErrorsCoef* nbOfErrorsPtr = pComparisonErrorReportingPtr != nullptr ? &errorCoef : nullptr;
    auto* numberOfEqualitiesPtr =
        pComparisonErrorReportingPtr != nullptr ? &pComparisonErrorReportingPtr->numberOfEqualities : nullptr;
    if (size1 <= size2) {
        ComparisonErrorsCoef subNbOfErrors;
        auto res = _getListExpsImbrications(listExpPtr1,
                                            listExpPtr2,
                                            pMemBlock,
                                            pLingDb,
                                            pExceptionsPtr,
                                            &subNbOfErrors,
                                            pParentGrammaticalType,
                                            numberOfEqualitiesPtr);
        if (res != ImbricationType::DIFFERS || size1 < size2) {
            if (res == ImbricationType::EQUALS && size1 < size2)
                res = ImbricationType::LESS_DETAILED;
            if (nbOfErrorsPtr != nullptr && subNbOfErrors.type != ComparisonTypeOfError::NO_ERROR)
                *nbOfErrorsPtr = subNbOfErrors;
            if (pComparisonErrorReportingPtr != nullptr && !errorCoef.empty())
                pComparisonErrorReportingPtr->addError(
                    pParentGrammaticalType, res, listExpPtr1, listExpPtr2, errorCoef);
            return res;
        }
    }
    auto res = _getListExpsImbrications(listExpPtr2,
                                        listExpPtr1,
                                        pMemBlock,
                                        pLingDb,
                                        pExceptionsPtr,
                                        nbOfErrorsPtr,
                                        pParentGrammaticalType,
                                        numberOfEqualitiesPtr);
    if (res == ImbricationType::EQUALS)
        res = ImbricationType::MORE_DETAILED;
    if (pComparisonErrorReportingPtr != nullptr && !errorCoef.empty())
        pComparisonErrorReportingPtr->addError(pParentGrammaticalType, res, listExpPtr1, listExpPtr2, errorCoef);
    return res;
}

ImbricationType getGrdExpsImbrications(const GroundedExpression& pGrdExp1,
                                       const GroundedExpression& pGrdExp2,
                                       const SemanticMemoryBlock& pMemBlock,
                                       const linguistics::LinguisticDatabase& pLingDb,
                                       const ComparisonExceptions* pExceptionsPtr,
                                       ComparisonErrorReporting* pComparisonErrorReportingPtr,
                                       GrammaticalType pParentGrammaticalType) {
    {
        ImbricationType res = ImbricationType::EQUALS;
        if (_checkSpecifierOfFirstGrdExpIfNeeded(res,
                                                 pGrdExp1,
                                                 pGrdExp2,
                                                 pMemBlock,
                                                 pLingDb,
                                                 pExceptionsPtr,
                                                 pComparisonErrorReportingPtr,
                                                 pParentGrammaticalType))
            return res;
        if (_checkSpecifierOfFirstGrdExpIfNeeded(res,
                                                 pGrdExp2,
                                                 pGrdExp1,
                                                 pMemBlock,
                                                 pLingDb,
                                                 pExceptionsPtr,
                                                 pComparisonErrorReportingPtr,
                                                 pParentGrammaticalType))
            return res;
    }

    if (_isExcluded(pGrdExp1, pGrdExp2, pMemBlock, pLingDb) || _isExcluded(pGrdExp2, pGrdExp1, pMemBlock, pLingDb))
        return ImbricationType::DIFFERS;

    bool followInterpretations = pExceptionsPtr == nullptr || !pExceptionsPtr->interpretations;
    const SemanticGrounding& grd1 = pGrdExp1.grounding();
    const SemanticGrounding& grd2 = pGrdExp2.grounding();
    const auto& grd1ToCmp = _getSubCptGrd(pGrdExp1, grd1, followInterpretations);
    const auto& grd2ToCmp = _getSubCptGrd(pGrdExp2, grd2, followInterpretations);
    ComparisonErrorsCoef errorCoef(10, ComparisonTypeOfError::NORMAL);
    ImbricationType groundingImbrication =
        _getGroundingsImbrications(grd1ToCmp, grd2ToCmp, pMemBlock, pLingDb, &errorCoef, pExceptionsPtr);
    if (groundingImbrication == ImbricationType::DIFFERS) {
        if (&grd1ToCmp != &grd1 && &grd2 == &grd2ToCmp) {
            ImbricationType rootImbrication =
                _getGroundingsImbrications(grd1, grd2, pMemBlock, pLingDb, &errorCoef, pExceptionsPtr);
            if (rootImbrication == ImbricationType::EQUALS)
                groundingImbrication = ImbricationType::MORE_DETAILED;
        }
        if (&grd1ToCmp == &grd1 && &grd2 != &grd2ToCmp) {
            ImbricationType rootImbrication =
                _getGroundingsImbrications(grd1, grd2, pMemBlock, pLingDb, &errorCoef, pExceptionsPtr);
            if (rootImbrication == ImbricationType::EQUALS)
                groundingImbrication = ImbricationType::LESS_DETAILED;
        }
        if (pComparisonErrorReportingPtr != nullptr) {
            if (grd1.concepts.count("stuff_informationToFill") > 0
                || grd2.concepts.count("stuff_informationToFill") > 0) {
                errorCoef.value = 1;
                errorCoef.type = ComparisonTypeOfError::PARAMETER_DIFF;
            }
            pComparisonErrorReportingPtr->addError(
                pParentGrammaticalType, groundingImbrication, ListExpPtr(pGrdExp1), ListExpPtr(pGrdExp2), errorCoef);
        }
        if (pComparisonErrorReportingPtr == nullptr && groundingImbrication == ImbricationType::DIFFERS)
            return groundingImbrication;
    } else if (groundingImbrication != ImbricationType::EQUALS) {
        if (pComparisonErrorReportingPtr != nullptr)
            pComparisonErrorReportingPtr->addError(pParentGrammaticalType,
                                                   groundingImbrication,
                                                   ListExpPtr(pGrdExp1),
                                                   ListExpPtr(pGrdExp2),
                                                   ComparisonErrorsCoef(10, ComparisonTypeOfError::NORMAL));
    } else if (pComparisonErrorReportingPtr != nullptr) {
        ++pComparisonErrorReportingPtr->numberOfEqualities;
    }

    auto tryToAddSemExp = [&](std::map<GrammaticalType, ListExpPtr>& pChildrenForAGramType,
                              std::map<GrammaticalType, UniqueSemanticExpression>::const_iterator pItChild,
                              bool pFirstOrSecondArg) {
        if (pItChild->first == GrammaticalType::INTRODUCTING_WORD || pItChild->first == GrammaticalType::SUB_CONCEPT
            || (pExceptionsPtr != nullptr && pExceptionsPtr->grammaticalTypes.count(pItChild->first) > 0))
            return;
        ListExpPtr listExpPtr;
        _fillListExpPtr(listExpPtr, *pItChild->second, pExceptionsPtr, pFirstOrSecondArg);
        if (!listExpPtr.elts.empty())
            pChildrenForAGramType.emplace(pItChild->first, std::move(listExpPtr));
    };

    mystd::optional<ImbricationType> childImbr;
    std::map<GrammaticalType, ListExpPtr> childrenOnlyIn1;
    std::map<GrammaticalType, ListExpPtr> childrenOnlyIn2;
    if (errorCoef.type != ComparisonTypeOfError::PARAMETER_DIFF) {
        auto itChild1 = pGrdExp1.children.begin();
        auto itChild2 = pGrdExp2.children.begin();
        while (itChild1 != pGrdExp1.children.end() && itChild2 != pGrdExp2.children.end()) {
            if (itChild1->first != itChild2->first) {
                if (itChild1->first < itChild2->first) {
                    tryToAddSemExp(childrenOnlyIn1, itChild1, true);
                    ++itChild1;
                } else {
                    tryToAddSemExp(childrenOnlyIn2, itChild2, false);
                    ++itChild2;
                }
                continue;
            } else {
                // Ignore certain grammatical types if specified.
                if (itChild1->first == GrammaticalType::INTRODUCTING_WORD
                    || itChild1->first == GrammaticalType::SUB_CONCEPT
                    || (pExceptionsPtr != nullptr && pExceptionsPtr->grammaticalTypes.count(itChild1->first) > 0)) {
                    ++itChild1;
                    continue;
                }
                const auto& semExp1 = *itChild1->second;
                const auto& semExp2 = *itChild2->second;
                auto newChildImbr = getSemExpsImbrications(semExp1,
                                                           semExp2,
                                                           pMemBlock,
                                                           pLingDb,
                                                           pExceptionsPtr,
                                                           pComparisonErrorReportingPtr,
                                                           itChild1->first);
                childImbr = childImbr ? _mergeChildImbrications(newChildImbr, *childImbr) : newChildImbr;
            }

            ++itChild1;
            ++itChild2;
        }

        auto addRemainingChildren = [&](std::map<GrammaticalType, ListExpPtr>& pRemainingChildren,
                                        std::map<GrammaticalType, UniqueSemanticExpression>::const_iterator& pItChild,
                                        const std::map<GrammaticalType, UniqueSemanticExpression>& pChildren,
                                        bool pFirstOrSecondArg) {
            while (pItChild != pChildren.end()) {
                tryToAddSemExp(pRemainingChildren, pItChild, pFirstOrSecondArg);
                ++pItChild;
            }
        };
        addRemainingChildren(childrenOnlyIn1, itChild1, pGrdExp1.children, true);
        addRemainingChildren(childrenOnlyIn2, itChild2, pGrdExp2.children, false);

        auto _matchReflexiveChildren = [&](std::map<GrammaticalType, ListExpPtr>& pChildrenOnlyInOne,
                                           const SemanticGrounding& pGrdOther,
                                           const GroundedExpression& pGrdExpOther,
                                           bool pNeedToInvertImbricationOrder) {
            for (auto itChildrenOnlyInOne = pChildrenOnlyInOne.begin();
                 itChildrenOnlyInOne != pChildrenOnlyInOne.end();) {
                if ((itChildrenOnlyInOne->first == GrammaticalType::RECEIVER
                     || itChildrenOnlyInOne->first == GrammaticalType::OBJECT)
                    && SemExpGetter::isGrdReflexive(pGrdOther)) {
                    auto itSubject2 = pGrdExpOther.children.find(GrammaticalType::SUBJECT);
                    if (itSubject2 != pGrdExpOther.children.end()) {
                        if (itChildrenOnlyInOne->second.elts.size() == 1) {
                            auto newChildImbr = getSemExpsImbrications(*itChildrenOnlyInOne->second.elts.front(),
                                                                       *itSubject2->second,
                                                                       pMemBlock,
                                                                       pLingDb,
                                                                       pExceptionsPtr,
                                                                       pComparisonErrorReportingPtr,
                                                                       itChildrenOnlyInOne->first);
                            if (pNeedToInvertImbricationOrder)
                                newChildImbr = switchOrderOfEltsImbrication(newChildImbr);
                            childImbr = childImbr ? _mergeChildImbrications(newChildImbr, *childImbr) : newChildImbr;
                            itChildrenOnlyInOne = pChildrenOnlyInOne.erase(itChildrenOnlyInOne);
                            continue;
                        }
                    }
                }
                ++itChildrenOnlyInOne;
            }
        };
        _matchReflexiveChildren(childrenOnlyIn1, grd2, pGrdExp2, false);
        _matchReflexiveChildren(childrenOnlyIn2, grd1, pGrdExp1, true);

        if (pComparisonErrorReportingPtr != nullptr) {
            for (auto& currChild : childrenOnlyIn1)
                pComparisonErrorReportingPtr->addError(
                    currChild.first,
                    ImbricationType::MORE_DETAILED,
                    currChild.second,
                    ListExpPtr(),
                    _getErrorCoefFromListExpPtr(currChild.first, currChild.second, followInterpretations));
            for (auto& currChild : childrenOnlyIn2)
                pComparisonErrorReportingPtr->addError(
                    currChild.first,
                    ImbricationType::LESS_DETAILED,
                    ListExpPtr(),
                    currChild.second,
                    _getErrorCoefFromListExpPtr(currChild.first, currChild.second, followInterpretations));
        }

        if (!childrenOnlyIn1.empty() && !childrenOnlyIn2.empty())
            return ImbricationType::DIFFERS;
    }

    if (childImbr && *childImbr == ImbricationType::DIFFERS)
        return *childImbr;
    if (!childrenOnlyIn1.empty())
        childImbr = ImbricationType::MORE_DETAILED;
    else if (!childrenOnlyIn2.empty())
        childImbr = ImbricationType::LESS_DETAILED;

    if (!childImbr)
        return groundingImbrication;
    return _mergeChildImbrications(groundingImbrication, *childImbr);
}

bool groundingsAreEqual(const SemanticGrounding& pGrounding1,
                        const SemanticGrounding& pGrounding2,
                        const SemanticMemoryBlock& pMemBlock,
                        const linguistics::LinguisticDatabase& pLingDb) {
    return _getGroundingsImbrications(pGrounding1, pGrounding2, pMemBlock, pLingDb, nullptr, nullptr)
        == ImbricationType::EQUALS;
}

ComparisonOperator numberComparisonOfGrdExps(const GroundedExpression& pGrdExp1, const GroundedExpression& pGrdExp2) {
    const SemanticGenericGrounding* genGrd1 = pGrdExp1->getGenericGroundingPtr();
    if (genGrd1 != nullptr && genGrd1->entityType == SemanticEntityType::NUMBER
        && genGrd1->quantity.type == SemanticQuantityType::NUMBER) {
        const SemanticGenericGrounding* genGrd2 = pGrdExp2->getGenericGroundingPtr();
        if (genGrd2 != nullptr && genGrd2->entityType == SemanticEntityType::NUMBER
            && genGrd2->quantity.type == SemanticQuantityType::NUMBER) {
            if (genGrd1->quantity.nb == genGrd2->quantity.nb) {
                return ComparisonOperator::EQUAL;
            }
            if (genGrd1->quantity.nb > genGrd2->quantity.nb) {
                return ComparisonOperator::MORE;
            }
            return ComparisonOperator::LESS;
        }
    }
    return ComparisonOperator::DIFFERENT;
}

ComparisonOperator numberComparisonOfSemExps(const SemanticExpression& pSemExp1,
                                             const SemanticExpression& pSemExp2,
                                             bool pFollowInterpretations) {
    const GroundedExpression* grdExp1 = pSemExp1.getGrdExpPtr_SkipWrapperPtrs(pFollowInterpretations);
    if (grdExp1 != nullptr) {
        const GroundedExpression* grdExp2 = pSemExp2.getGrdExpPtr_SkipWrapperPtrs(pFollowInterpretations);
        if (grdExp2 != nullptr) {
            return numberComparisonOfGrdExps(*grdExp1, *grdExp2);
        }
    }
    return ComparisonOperator::DIFFERENT;
}

bool grdExpsReferToSameInstance(const GroundedExpression& pGrdExp1,
                                const GroundedExpression& pGrdExp2,
                                const SemanticMemoryBlock& pMemBlock,
                                const linguistics::LinguisticDatabase& pLingDb) {
    auto* agent1Ptr = pGrdExp1->getAgentGroundingPtr();
    if (agent1Ptr != nullptr) {
        auto* agent2Ptr = pGrdExp2->getAgentGroundingPtr();
        if (agent2Ptr != nullptr)
            return _userIdsAreEqual(agent1Ptr->userId, agent2Ptr->getAgentGrounding().userId, pMemBlock, nullptr);
    }
    return grdExpsAreEqual(pGrdExp1, pGrdExp2, pMemBlock, pLingDb);
}

}    // End of namespace SemExpComparator

}    // End of namespace onsem
