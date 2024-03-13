#include <onsem/semantictotext/semanticmemory/links/groundedexpwithlinks.hpp>
#include <onsem/common/binary/binarysaver.hpp>
#include <onsem/common/binary/enummapsaver.hpp>
#include <onsem/common/utility/random.hpp>
#include <onsem/common/utility/uppercasehandler.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpressions.hpp>
#include <onsem/texttosemantic/dbtype/semanticgroundings.hpp>
#include <onsem/texttosemantic/tool/semexpgetter.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase.hpp>
#include <onsem/semantictotext/semanticmemory/links/expressionwithlinks.hpp>
#include <onsem/semantictotext/semanticmemory/links/sentencewithlinks.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemoryblock.hpp>
#include "../../tool/userinfosfiller.hpp"
#include "../semanticmemoryblockprivate.hpp"

namespace onsem {

namespace {

bool _isOtherGrdExpMoreRevelant(const SemanticMemoryGrdExp& pSemMemGrdExp1,
                                const SemanticMemoryGrdExp& pSemMemGrdExp2) {
    return pSemMemGrdExp1.getMemSentence().isOtherSentenceMoreRevelant(pSemMemGrdExp2.getMemSentence());
}

bool _areLinksMoreRevelant(const SemanticSplitAssertAndInformLinks<MemoryGrdExpLinks>& pNewLinks,
                           const SemanticSplitAssertAndInformLinks<MemoryGrdExpLinks>& pMainLinks) {
    if (!pNewLinks.assertions.empty()) {
        auto& newGrdExps = (--pNewLinks.assertions.end())->second;
        if (!newGrdExps.empty()) {
            if (pMainLinks.assertions.empty())
                return true;
            auto& mainGrdExps = (--pMainLinks.assertions.end())->second;
            if (mainGrdExps.empty())
                return true;
            return _isOtherGrdExpMoreRevelant(mainGrdExps.back(), newGrdExps.back());
        }
    }
    if (!pMainLinks.assertions.empty() || pNewLinks.informations.empty())
        return false;
    auto& newGrdExps = (--pNewLinks.informations.end())->second;
    if (newGrdExps.empty())
        return false;
    if (pMainLinks.informations.empty())
        return true;
    auto& mainGrdExps = (--pMainLinks.informations.end())->second;
    if (mainGrdExps.empty())
        return true;
    return _isOtherGrdExpMoreRevelant(mainGrdExps.back(), newGrdExps.back());
}

template<typename TVALUE, typename MAP_TYPE1, typename MAP_TYPE2, LinksAccessType access>
void _addValueLinksFrom(
    SemanticValueLinks<TVALUE, MemoryGrdExpLinks, MAP_TYPE1, access>& pToFill,
    SemanticValueLinks<TVALUE, MemoryGrdExpLinksForAMemSentence, MAP_TYPE2, LinksAccessType::ALL>& pRef,
    intSemId pMemSentenceId) {
    pRef.iterateOnKeysValues(
        [&](const TVALUE& pKey, SemanticSplitAssertAndInformLinks<MemoryGrdExpLinksForAMemSentence>& pLinksToAdd) {
            pToFill.addValue(
                pKey,
                [&](SemanticSplitAssertAndInformLinks<MemoryGrdExpLinks>& pLinks) {
                    if (!pLinksToAdd.assertions.empty()) {
                        auto& linksWithId = pLinks.assertions[pMemSentenceId];
                        for (auto& currElt : pLinksToAdd.assertions)
                            linksWithId.emplace_back(currElt);
                    }
                    if (!pLinksToAdd.informations.empty()) {
                        auto& linksWithId = pLinks.informations[pMemSentenceId];
                        for (auto& currElt : pLinksToAdd.informations)
                            linksWithId.emplace_back(currElt);
                    }
                },
                &_areLinksMoreRevelant);
        });
}

template<typename TVALUE, typename MAP_TYPE1, typename MAP_TYPE2, LinksAccessType access>
void _removeValueLinksFrom(
    SemanticValueLinks<TVALUE, MemoryGrdExpLinks, MAP_TYPE1, access>& pToFilter,
    const SemanticValueLinks<TVALUE, MemoryGrdExpLinksForAMemSentence, MAP_TYPE2, LinksAccessType::ALL>& pToRemove,
    intSemId pMemSentenceId) {
    pToRemove.iterateOnKeys([&](const TVALUE& pKey) {
        pToFilter.removeValue(
            pKey,
            [&](SemanticSplitAssertAndInformLinks<MemoryGrdExpLinks>& pLinks) {
                pLinks.assertions.erase(pMemSentenceId);
                pLinks.informations.erase(pMemSentenceId);
            },
            &_areLinksMoreRevelant);
    });
}

template<typename TVALUE, typename MAP_TYPE>
void _addValueForAMemGrdExp(
    SemanticValueLinks<TVALUE, MemoryGrdExpLinksForAMemSentence, MAP_TYPE, LinksAccessType::ALL>& pToFill,
    const TVALUE& pValue,
    SemanticMemoryGrdExp& pMemGrdExp) {
    pToFill.addValue(
        pValue,
        [&](SemanticSplitAssertAndInformLinks<MemoryGrdExpLinksForAMemSentence>& pLinks) {
            if (pMemGrdExp.getMemSentence().getContextAxiom().informationType == InformationType::ASSERTION)
                pLinks.assertions.emplace_back(pMemGrdExp);
            else
                pLinks.informations.emplace_back(pMemGrdExp);
        },
        [&](const SemanticSplitAssertAndInformLinks<MemoryGrdExpLinksForAMemSentence>&,
            const SemanticSplitAssertAndInformLinks<MemoryGrdExpLinksForAMemSentence>&) {
            return false;
        }    // we don't care about the main value here
    );
}

struct RegisterNameLinks : public SemExpUserInfosFiller::UserInfosContainer {
    RegisterNameLinks(SemanticUserCenteredMemoryLinksForAMemSentence& pUserCenteredLinks,
                      SemanticMemoryGrdExp& pNewMemGrdExp)
        : _userCenteredLinks(pUserCenteredLinks)
        , _newMemGrdExp(pNewMemGrdExp) {}

    void addNames(const std::string& pUserId, const std::vector<std::string>& pNames) override {
        UserNames userNames;
        userNames.names = pNames;
        _addValueForAMemGrdExp(
            _userCenteredLinks.userIdToUserCharacteristics[pUserId].nameLinks, userNames, _newMemGrdExp);
        for (const auto& currName : pNames)
            _addValueForAMemGrdExp(_userCenteredLinks.nameToUserIds[currName], pUserId, _newMemGrdExp);
    }

    void addGenders(const std::string& pUserId, const std::set<SemanticGenderType>& pPossibleGenders) override {
        auto& genderRef = _userCenteredLinks.userIdToUserCharacteristics[pUserId].genderLinks;
        for (const auto& currGender : pPossibleGenders)
            _addValueForAMemGrdExp(genderRef, currGender, _newMemGrdExp);
    }

    void addEquivalentUserIds(const std::string& pSubjectUserId, const std::string& pObjectUserId) override {
        _addValueForAMemGrdExp(_userCenteredLinks.userIdToUserCharacteristics[pSubjectUserId].equivalentUserIdLinks,
                               pObjectUserId,
                               _newMemGrdExp);
        _addValueForAMemGrdExp(_userCenteredLinks.userIdToUserCharacteristics[pObjectUserId].equivalentUserIdLinks,
                               pSubjectUserId,
                               _newMemGrdExp);
    }

    void addGrdExpToUserId(const GroundedExpression& pSubjectGrdExp, const std::string& pObjectUserId) override {
        std::string strGrdExpId = Random::generateUuid();
        _addValueForAMemGrdExp(_userCenteredLinks.semExpToUserIds[&pSubjectGrdExp], strGrdExpId, _newMemGrdExp);
        _addValueForAMemGrdExp(
            _userCenteredLinks.userIdToUserCharacteristics[strGrdExpId].semExpLinks, &pSubjectGrdExp, _newMemGrdExp);
        addEquivalentUserIds(strGrdExpId, pObjectUserId);
    }

private:
    SemanticUserCenteredMemoryLinksForAMemSentence& _userCenteredLinks;
    SemanticMemoryGrdExp& _newMemGrdExp;
};

template<typename MAP_KEY_MEMBLOCLINKS, typename MAP_KEY_LINKS>
void _fillMemBlocLinks(MAP_KEY_MEMBLOCLINKS& pMemBlocLinks, MAP_KEY_LINKS& pLinks, intSemId pMemSentenceId) {
    for (auto& currElt : pLinks) {
        currElt.second.shrink_to_fit();
        auto& vectToFill = pMemBlocLinks[currElt.first][pMemSentenceId];
        if (vectToFill.empty())
            vectToFill = currElt.second;
        else
            vectToFill.insert(vectToFill.end(), currElt.second.begin(), currElt.second.end());
    }
}

template<typename LINKS_TYPE, typename LINKS_TYPE_FOR_A_MEM_SENT>
void _fillMemBlocLinksForRadixMap(mystd::radix_map_str<LINKS_TYPE>& pMemBlocLinks,
                                  mystd::radix_map_str<LINKS_TYPE_FOR_A_MEM_SENT>& pLinks,
                                  intSemId pMemSentenceId) {
    pLinks.for_each([&](const std::string& pKey, LINKS_TYPE_FOR_A_MEM_SENT& pValue) {
        pValue.shrink_to_fit();
        auto& vectToFill = pMemBlocLinks[pKey][pMemSentenceId];
        if (vectToFill.empty())
            vectToFill = pValue;
        else
            vectToFill.insert(vectToFill.end(), pValue.begin(), pValue.end());
    });
}

template<typename LINKSTYPE, typename DURATIONTYPE>
typename mystd::radix_map_struct<SemanticDuration, LINKSTYPE>::iterator _insertTimeElt(
    mystd::radix_map_struct<SemanticDuration, LINKSTYPE>& pTimeToSemExps,
    const DURATIONTYPE& pDuration) {
    auto insertRes = pTimeToSemExps.insert(pDuration);
    auto& itInMemory = insertRes.first;
    assert(itInMemory != pTimeToSemExps.end());
    if (!insertRes.second)    // if it already existed we return it
        return itInMemory;

    // init with previous element value
    if (itInMemory != pTimeToSemExps.begin()) {
        auto itBefore = itInMemory;
        --itBefore;
        itInMemory->second = itBefore->second;
    }
    return itInMemory;
}

void _addDurationSlotLinks(SemanticLinksToGrdExpsForAMemSentence& pLinksToGrdExps,
                           const SemanticDuration pBeginDuration,
                           const SemanticDuration pEndDuration,
                           SemanticMemoryGrdExp& pNewMemGrdExp) {
    _insertTimeElt(pLinksToGrdExps.timeToSemExps, pEndDuration);

    // add begin of the time slot
    auto itInMemory = _insertTimeElt(pLinksToGrdExps.timeToSemExps, pBeginDuration);
    itInMemory->second.push_back(pNewMemGrdExp);

    // update the links that are inside the new time slot
    ++itInMemory;
    if (itInMemory != pLinksToGrdExps.timeToSemExps.end()) {
        // while (itInMemory->first < pEndDuration)
        auto endRadixMapStr = pEndDuration.toRadixMapStr();
        while (itInMemory->first < endRadixMapStr) {
            itInMemory->second.push_back(pNewMemGrdExp);
            ++itInMemory;
            if (itInMemory == pLinksToGrdExps.timeToSemExps.end())
                break;
        }
    }
}

void _addTimeLinksFromRelativeDurationGrounding(SemanticLinksToGrdExpsForAMemSentence& pLinksToGrdExps,
                                                const SemanticDuration& pBeginDuration,
                                                SemanticMemoryGrdExp& pNewMemGrdExpPtr) {
    // add end of the time slot
    SemanticDuration endDuration = pBeginDuration;
    endDuration.add(SemanticTimeUnity::LESS_THAN_A_MILLISECOND, 1);
    // fill the links
    _addDurationSlotLinks(pLinksToGrdExps, pBeginDuration, endDuration, pNewMemGrdExpPtr);
}

void _addTimeLinksFromGrounding(SemanticLinksToGrdExpsForAMemSentence& pLinksToGrdExps,
                                const SemanticTimeGrounding& pTimeGrounding,
                                SemanticMemoryGrdExp& pNewMemGrdExp) {
    SemanticDuration beginDuration;
    pTimeGrounding.getBeginInDurationStruct(beginDuration);
    // add end of the time slot
    SemanticDuration endDuration = beginDuration + pTimeGrounding.length;
    endDuration.add(SemanticTimeUnity::LESS_THAN_A_MILLISECOND, 1);
    // fill the links
    _addDurationSlotLinks(pLinksToGrdExps, beginDuration, endDuration, pNewMemGrdExp);

    // add day concept
    auto dayCpt = pTimeGrounding.date.getDayConcept();
    if (!dayCpt.empty())
        pLinksToGrdExps.conceptsToSemExps[dayCpt].emplace_back(pNewMemGrdExp);

    // add month concept
    if (pTimeGrounding.date.month) {
        auto ponthCpt = monthConceptStr_fromMonthId(*pTimeGrounding.date.month);
        pLinksToGrdExps.conceptsToSemExps[ponthCpt].emplace_back(pNewMemGrdExp);
    }

    // add year concept
    auto yearCpt = pTimeGrounding.date.getYearConcept();
    if (!yearCpt.empty())
        pLinksToGrdExps.conceptsToSemExps[yearCpt].emplace_back(pNewMemGrdExp);
}

void _addTimeToSemExpsLinks(
    mystd::radix_map_struct<SemanticDuration, MemoryGrdExpLinks>& pTimeToSemExps,
    mystd::radix_map_struct<SemanticDuration, MemoryGrdExpLinksForAMemSentence>& pTmeToSemExpsFromSentence,
    intSemId pMemSentenceId) {
    if (!pTmeToSemExpsFromSentence.empty()) {
        // insert new time elts
        auto itRefTimeElt = pTmeToSemExpsFromSentence.end();
        while (itRefTimeElt != pTmeToSemExpsFromSentence.begin()) {
            --itRefTimeElt;
            _insertTimeElt(pTimeToSemExps, itRefTimeElt->first);
        }

        auto itInMemory = pTimeToSemExps.find(itRefTimeElt->first);
        assert(itInMemory != pTimeToSemExps.end());
        while (true) {
            if (!itRefTimeElt->second.empty()) {
                // fill the time modifications
                auto& linksToFill = itInMemory->second[pMemSentenceId];
                linksToFill.insert(linksToFill.end(), itRefTimeElt->second.begin(), itRefTimeElt->second.end());

                auto itRefNext = itRefTimeElt;
                ++itRefNext;
                if (itRefNext == pTmeToSemExpsFromSentence.end()) {
                    assert(false);
                    break;
                }

                // update the times between 2 time modifications
                ++itInMemory;
                while (itInMemory->first < itRefNext->first) {
                    auto& currLinksToFill = itInMemory->second[pMemSentenceId];
                    currLinksToFill.insert(
                        currLinksToFill.end(), itRefTimeElt->second.begin(), itRefTimeElt->second.end());
                    ++itInMemory;
                    assert(itInMemory != pTimeToSemExps.end());
                }
                itRefTimeElt = itRefNext;
                assert(itRefTimeElt->first == itInMemory->first);
                continue;
            }

            // jump to next time slot
            ++itRefTimeElt;
            if (itRefTimeElt == pTmeToSemExpsFromSentence.end())
                break;
            itInMemory = pTimeToSemExps.find(itRefTimeElt->first);
            assert(itInMemory != pTimeToSemExps.end());
        }
    }
}

void _addLinksFrom(SemanticLinksToGrdExps& pLinks,
                   SemanticLinksToGrdExpsTemplate<MemoryGrdExpLinksForAMemSentence>& pLinksFromMemSentence,
                   intSemId pMemSentenceId) {
    _fillMemBlocLinksForRadixMap(pLinks.conceptsToSemExps, pLinksFromMemSentence.conceptsToSemExps, pMemSentenceId);

    for (auto& currElt : pLinksFromMemSentence.meaningsToSemExps)
        _fillMemBlocLinks(pLinks.meaningsToSemExps[currElt.first], currElt.second, pMemSentenceId);

    _fillMemBlocLinks(pLinks.everythingOrNoEntityTypeToSemExps,
                      pLinksFromMemSentence.everythingOrNoEntityTypeToSemExps,
                      pMemSentenceId);
    _fillMemBlocLinks(
        pLinks.genGroundingTypeToSemExps, pLinksFromMemSentence.genGroundingTypeToSemExps, pMemSentenceId);

    _addTimeToSemExpsLinks(pLinks.timeToSemExps, pLinksFromMemSentence.timeToSemExps, pMemSentenceId);
    _fillMemBlocLinks(pLinks.durationToSemExps, pLinksFromMemSentence.durationToSemExps, pMemSentenceId);

    _fillMemBlocLinks(pLinks.relLocationToSemExps, pLinksFromMemSentence.relLocationToSemExps, pMemSentenceId);
    _fillMemBlocLinks(pLinks.relTimeToSemExps, pLinksFromMemSentence.relTimeToSemExps, pMemSentenceId);
    _fillMemBlocLinks(pLinks.numberToSemExps, pLinksFromMemSentence.numberToSemExps, pMemSentenceId);
    _fillMemBlocLinks(pLinks.quantityTypeToSemExps, pLinksFromMemSentence.quantityTypeToSemExps, pMemSentenceId);
    _fillMemBlocLinks(pLinks.referenceWithoutConceptToSemExps,
                      pLinksFromMemSentence.referenceWithoutConceptToSemExps,
                      pMemSentenceId);
    _fillMemBlocLinks(pLinks.coreferenceWithoutConceptOrReferenceToSemExps,
                      pLinksFromMemSentence.coreferenceWithoutConceptOrReferenceToSemExps,
                      pMemSentenceId);
    _fillMemBlocLinks(pLinks.statementCoreferenceWithoutConceptToSemExps,
                      pLinksFromMemSentence.statementCoreferenceWithoutConceptToSemExps,
                      pMemSentenceId);
    _fillMemBlocLinks(pLinks.grdTypeToSemExps, pLinksFromMemSentence.grdTypeToSemExps, pMemSentenceId);
    _fillMemBlocLinksForRadixMap(pLinks.userIdToSemExps, pLinksFromMemSentence.userIdToSemExps, pMemSentenceId);
    _fillMemBlocLinksForRadixMap(
        pLinks.userIdWithoutContextToSemExps, pLinksFromMemSentence.userIdWithoutContextToSemExps, pMemSentenceId);
    for (auto& currElt : pLinksFromMemSentence.textToSemExps)
        _fillMemBlocLinksForRadixMap(pLinks.textToSemExps[currElt.first], currElt.second, pMemSentenceId);
    _fillMemBlocLinks(pLinks.languageToSemExps, pLinksFromMemSentence.languageToSemExps, pMemSentenceId);
    _fillMemBlocLinksForRadixMap(pLinks.resourceToSemExps, pLinksFromMemSentence.resourceToSemExps, pMemSentenceId);
}

template<typename MAP_KEY_MEMBLOCLINKS, typename MAP_KEY_LINKS>
void _removeMemoryLinks(MAP_KEY_MEMBLOCLINKS& pToFilter, const MAP_KEY_LINKS& pRef, intSemId pMemSentenceId) {
    for (const auto& currElt : pRef) {
        auto subItToFilter = pToFilter.find(currElt.first);
        subItToFilter->second.erase(pMemSentenceId);
        if (subItToFilter->second.empty())
            pToFilter.erase(subItToFilter);
    }
}

template<typename LINKS_TYPE, typename LINKS_TYPE_FOR_A_MEM_SENT>
void _removeMemoryLinksForRadixMap(mystd::radix_map_str<LINKS_TYPE>& pToFilter,
                                   const mystd::radix_map_str<LINKS_TYPE_FOR_A_MEM_SENT>& pRef,
                                   intSemId pMemSentenceId) {
    pRef.for_each([&](const std::string& pKey, const LINKS_TYPE_FOR_A_MEM_SENT&) {
        auto subItToFilter = pToFilter.find(pKey);
        assert(subItToFilter != pToFilter.end());
        subItToFilter->second.erase(pMemSentenceId);
        if (subItToFilter->second.empty())
            pToFilter.erase(subItToFilter);
    });
}

void _removeGrdExpsLinks(std::map<intSemId, MemoryGrdExpLinksForAMemSentence>& pToFilter,
                         const MemoryGrdExpLinksForAMemSentence& pLinksFromMemSentence,
                         intSemId pMemSentenceId) {
    if (pLinksFromMemSentence.empty())
        return;
    auto itToFilter = pToFilter.find(pMemSentenceId);
    if (itToFilter != pToFilter.end()) {
        auto itLinks = itToFilter->second.begin();
        for (auto& currLink : pLinksFromMemSentence) {
            while (itLinks != itToFilter->second.end()) {
                if (&*itLinks == &currLink) {
                    itLinks = itToFilter->second.erase(itLinks);
                    break;
                } else
                    ++itLinks;
            }
        }

        if (itToFilter->second.empty())
            pToFilter.erase(itToFilter);
    } else {
        assert(false);
    }
}

void _removeTimeToSemExpsLinks(
    mystd::radix_map_struct<SemanticDuration, MemoryGrdExpLinks>& pTimeToSemExps,
    const mystd::radix_map_struct<SemanticDuration, MemoryGrdExpLinksForAMemSentence>& pTimeToSemExpsFromSentence,
    intSemId pMemSentenceId) {
    if (!pTimeToSemExpsFromSentence.empty()) {
        auto itTimeElt = pTimeToSemExpsFromSentence.begin();
        while (true) {
            auto subItToFilter = pTimeToSemExps.find(itTimeElt->first);
            assert(subItToFilter != pTimeToSemExps.end());
            _removeGrdExpsLinks(subItToFilter->second, itTimeElt->second, pMemSentenceId);

            if (subItToFilter != pTimeToSemExps.begin()) {
                auto itPrev = subItToFilter;
                --itPrev;
                if (subItToFilter->second == itPrev->second)
                    subItToFilter = pTimeToSemExps.erase(subItToFilter);
                else
                    ++subItToFilter;
            } else if (subItToFilter->second.empty()) {
                subItToFilter = pTimeToSemExps.erase(subItToFilter);
            } else {
                ++subItToFilter;
            }

            auto itTimeNextElt = itTimeElt;
            ++itTimeNextElt;
            if (itTimeNextElt == pTimeToSemExpsFromSentence.end())
                break;
            while (subItToFilter != pTimeToSemExps.end() && subItToFilter->first < itTimeNextElt->first) {
                _removeGrdExpsLinks(subItToFilter->second, itTimeElt->second, pMemSentenceId);
                ++subItToFilter;
                assert(subItToFilter != pTimeToSemExps.end());
            }
            itTimeElt = itTimeNextElt;
        }
    }
}

void _removeLinksFrom(SemanticLinksToGrdExps& pToFilter,
                      const SemanticLinksToGrdExpsTemplate<MemoryGrdExpLinksForAMemSentence>& pLinksFromMemSentence,
                      intSemId pMemSentenceId) {
    _removeMemoryLinksForRadixMap(pToFilter.conceptsToSemExps, pLinksFromMemSentence.conceptsToSemExps, pMemSentenceId);

    for (const auto& currElt : pLinksFromMemSentence.meaningsToSemExps) {
        auto subItToFilter = pToFilter.meaningsToSemExps.find(currElt.first);
        _removeMemoryLinks(subItToFilter->second, currElt.second, pMemSentenceId);
        if (subItToFilter->second.empty())
            pToFilter.meaningsToSemExps.erase(subItToFilter);
    }

    _removeMemoryLinks(pToFilter.everythingOrNoEntityTypeToSemExps,
                       pLinksFromMemSentence.everythingOrNoEntityTypeToSemExps,
                       pMemSentenceId);
    _removeMemoryLinks(
        pToFilter.genGroundingTypeToSemExps, pLinksFromMemSentence.genGroundingTypeToSemExps, pMemSentenceId);

    _removeTimeToSemExpsLinks(pToFilter.timeToSemExps, pLinksFromMemSentence.timeToSemExps, pMemSentenceId);
    _removeMemoryLinks(pToFilter.durationToSemExps, pLinksFromMemSentence.durationToSemExps, pMemSentenceId);

    _removeMemoryLinks(pToFilter.relLocationToSemExps, pLinksFromMemSentence.relLocationToSemExps, pMemSentenceId);
    _removeMemoryLinks(pToFilter.relTimeToSemExps, pLinksFromMemSentence.relTimeToSemExps, pMemSentenceId);
    _removeMemoryLinks(pToFilter.numberToSemExps, pLinksFromMemSentence.numberToSemExps, pMemSentenceId);
    _removeMemoryLinks(pToFilter.quantityTypeToSemExps, pLinksFromMemSentence.quantityTypeToSemExps, pMemSentenceId);
    _removeMemoryLinks(pToFilter.referenceWithoutConceptToSemExps,
                       pLinksFromMemSentence.referenceWithoutConceptToSemExps,
                       pMemSentenceId);
    _removeMemoryLinks(pToFilter.coreferenceWithoutConceptOrReferenceToSemExps,
                       pLinksFromMemSentence.coreferenceWithoutConceptOrReferenceToSemExps,
                       pMemSentenceId);
    _removeMemoryLinks(pToFilter.statementCoreferenceWithoutConceptToSemExps,
                       pLinksFromMemSentence.statementCoreferenceWithoutConceptToSemExps,
                       pMemSentenceId);
    _removeMemoryLinks(pToFilter.grdTypeToSemExps, pLinksFromMemSentence.grdTypeToSemExps, pMemSentenceId);
    _removeMemoryLinksForRadixMap(pToFilter.userIdToSemExps, pLinksFromMemSentence.userIdToSemExps, pMemSentenceId);
    _removeMemoryLinksForRadixMap(
        pToFilter.userIdWithoutContextToSemExps, pLinksFromMemSentence.userIdWithoutContextToSemExps, pMemSentenceId);

    for (const auto& currElt : pLinksFromMemSentence.textToSemExps) {
        auto subItToFilter = pToFilter.textToSemExps.find(currElt.first);
        _removeMemoryLinksForRadixMap(subItToFilter->second, currElt.second, pMemSentenceId);
        if (subItToFilter->second.empty())
            pToFilter.textToSemExps.erase(subItToFilter);
    }

    _removeMemoryLinks(pToFilter.languageToSemExps, pLinksFromMemSentence.languageToSemExps, pMemSentenceId);
    _removeMemoryLinksForRadixMap(pToFilter.resourceToSemExps, pLinksFromMemSentence.resourceToSemExps, pMemSentenceId);
}

enum class LinkedState { LINKED, LINKDISABLED, NOTLINKED };

}

struct GroundedExpWithLinksPrivate {
    GroundedExpWithLinksPrivate(GroundedExpWithLinks& pMemSent,
                                const std::map<GrammaticalType, const SemanticExpression*>& pAnnotations,
                                const linguistics::LinguisticDatabase& pLingDb);

    void enableUserCenteredLinks();

    void linkToRequToAnswers();
    void linkToConditionToInformation();
    void linkToSentWithAction();
    void linkToSentWithInfAction();
    void linkToActionTrigger();
    void linkToQuestionTrigger();
    void linkToAffirmationTrigger();
    void linkToNominalTrigger();
    void linkToRecommendationsTrigger();
    void disableUserCenteredLinks();

    void writeInBinary(binarymasks::Ptr& pPtr,
                       MemGrdExpPtrOffsets& pMemGrdExpPtrs,
                       const semexpsaver::SemExpPtrOffsets& pSemExpPtrOffsets) const;
    void filterMemLinks(SemanticMemoryLinks& pSemanticMemoryLinksToFilter) const;
    void filterNominalGroupLinks(SemanticLinksToGrdExps& pLinksToFilter) const;
    void filterRecommendationsLinks(SemanticLinksToGrdExps& pLinksToFilter) const;

    SemanticVerbTense tense;
    VerbGoalEnum verbGoal;
    SemanticUserCenteredMemoryLinksForAMemSentence userCenteredLinks;
    LinkedState isLinkedInRequToAnswers;
    LinkedState isLinkedInRequToCondition;
    LinkedState isLinkedInRequToSentWithAction;
    LinkedState isLinkedInRequToInfAction;
    LinkedState isLinkedInRequToActionTrigger;
    LinkedState isLinkedInRequToQuestionTrigger;
    LinkedState isLinkedInRequToAffirmationTrigger;
    LinkedState isLinkedInRequToNominalGroupsTrigger;
    LinkedState isLinkedInRequToRecommendationsTrigger;

private:
    GroundedExpWithLinks& _memSent;
    std::list<SemanticMemoryGrdExp> _childGrdExpMemories;
    SemanticMemoryLinksForAMemSentence _links;
    std::unique_ptr<SemanticLinksToGrdExpsForAMemSentence> _recomendationLinksPtr;

    void _linkRecommendationSemExp(const SemanticExpression& pSemExp, const linguistics::LinguisticDatabase& pLingDb);
    void _linkRecommendationGrdExp(const GroundedExpression& pGrdExp, const linguistics::LinguisticDatabase& pLingDb);

    void _linkStatementGrdExp(const SemanticStatementGrounding& pStatGrd,
                              const linguistics::LinguisticDatabase& pLingDb);

    bool _linkChildSemExp(const SemanticExpression& pSemExpression,
                          SemanticRequestType pFromRequest,
                          const linguistics::LinguisticDatabase& pLingDb,
                          bool pLinkNonSpecificStuffs);

    void _fillUserCenteredLinks(SemanticMemoryGrdExp& pNewMemGrdExp, const GroundedExpression& pGrdExp);

    /**
     * @brief Fill the link of a gounded expression.
     * @param[out] pLinksToGrdExps Links of the grounded expression.
     * @param pGrdExp The grounded expression to add.
     * @param pGrounding The grounding of the grounded expression.
     * @param pLingDb The linguistic dictionary.
     * @param pLinkNonSpecificStuffs If we shall add the grounding that is not linked to a word,
     * a concept and that his reference is indefinite.
     */
    bool _linkGrdExp(
        const std::function<SemanticLinksToGrdExpsTemplate<MemoryGrdExpLinksForAMemSentence>&()>& pEnsureLinksToGrdExps,
        const GroundedExpression& pGrdExp,
        const SemanticGrounding& pGrounding,
        const linguistics::LinguisticDatabase& pLingDb,
        bool pLinkNonSpecificStuffs);

    void _linkLingMeaning(SemanticLinksToGrdExpsTemplate<MemoryGrdExpLinksForAMemSentence>& pLinksToGrdExps,
                          SemanticMemoryGrdExp& pNewMemGrdExp,
                          const SemanticWord& pInWord,
                          const StaticLinguisticMeaning& pInLingMeaning);

    void _mergeRequToGrdExps(SemanticMemoryLinks& pToComplete, SemanticMemoryLinksForAMemSentence& pRef);

    void _addGenderFromGrdExp(SemanticMemoryGrdExp& pNewMemGrdExp, const GroundedExpression& pGrdExp);
    void _addGenderFromSemExp(SemanticMemoryGrdExp& pNewMemGrdExp, const UniqueSemanticExpression& pSemExp);

    void _tryToAddAgentGenderFromAStructureEqualityStatement(SemanticMemoryGrdExp& pNewMemGrdExp,
                                                             const GroundedExpression& pGrdExp);
};

GroundedExpWithLinksPrivate::GroundedExpWithLinksPrivate(
    GroundedExpWithLinks& pMemSent,
    const std::map<GrammaticalType, const SemanticExpression*>& pAnnotations,
    const linguistics::LinguisticDatabase& pLingDb)
    : tense(SemanticVerbTense::UNKNOWN)
    , verbGoal(VerbGoalEnum::NOTIFICATION)
    , userCenteredLinks()
    , isLinkedInRequToAnswers(LinkedState::NOTLINKED)
    , isLinkedInRequToCondition(LinkedState::NOTLINKED)
    , isLinkedInRequToSentWithAction(LinkedState::NOTLINKED)
    , isLinkedInRequToInfAction(LinkedState::NOTLINKED)
    , isLinkedInRequToActionTrigger(LinkedState::NOTLINKED)
    , isLinkedInRequToQuestionTrigger(LinkedState::NOTLINKED)
    , isLinkedInRequToAffirmationTrigger(LinkedState::NOTLINKED)
    , isLinkedInRequToNominalGroupsTrigger(LinkedState::NOTLINKED)
    , isLinkedInRequToRecommendationsTrigger(LinkedState::NOTLINKED)
    , _memSent(pMemSent)
    , _childGrdExpMemories()
    , _links() {
    for (const auto& currAnnotation : pAnnotations) {
        auto itChild = _memSent.grdExp.children.find(currAnnotation.first);
        if (itChild == _memSent.grdExp.children.end()
            || SemExpGetter::isACoreference(*itChild->second, CoreferenceDirectionEnum::PARENT, false))
            _memSent._annotations.emplace(currAnnotation.first, currAnnotation.second);
    }

    if (_memSent.inRecommendationMode) {
        if (!_recomendationLinksPtr)
            _recomendationLinksPtr = std::make_unique<SemanticLinksToGrdExpsForAMemSentence>();
        _linkRecommendationGrdExp(_memSent.grdExp, pLingDb);
        if (_memSent._isEnabled)
            linkToRecommendationsTrigger();
        else
            isLinkedInRequToRecommendationsTrigger = LinkedState::LINKDISABLED;
        return;
    }

    const SemanticGrounding& grd = _memSent.grdExp.grounding();
    const SemanticStatementGrounding* statGrdPtr = _memSent.grdExp->getStatementGroundingPtr();
    if (statGrdPtr != nullptr) {
        const auto& statGrd = *statGrdPtr;
        tense = statGrd.verbTense;
        verbGoal = statGrd.verbGoal;
        _linkStatementGrdExp(statGrd, pLingDb);
    } else {
        _memSent._isANoun = true;
        _linkGrdExp(
            [&]() -> auto& { return _links.reqToGrdExps[SemanticRequestType::NOTHING]; },
            _memSent.grdExp,
            grd,
            pLingDb,
            true);
        if (_memSent._isEnabled && !_memSent._isAConditionToSatisfy)
            enableUserCenteredLinks();
        if (_memSent._contextAxiom.getSemExpWrappedForMemory().outputToAnswerIfTriggerHasMatched) {
            if (_memSent._isEnabled)
                linkToNominalTrigger();
            else
                isLinkedInRequToNominalGroupsTrigger = LinkedState::LINKDISABLED;
        } else if (_memSent._contextAxiom.semTracker || _memSent._contextAxiom.semExpToDo != nullptr) {
            _memSent._isAConditionToSatisfy = true;
            if (_memSent._isEnabled)
                linkToSentWithAction();
            else
                isLinkedInRequToSentWithAction = LinkedState::LINKDISABLED;
        }
    }
}

void GroundedExpWithLinksPrivate::enableUserCenteredLinks() {
    auto& sharedUserCenteredLinks =
        _memSent._contextAxiom.getSemExpWrappedForMemory().getParentMemBloc()._impl->userCenteredLinks;
    userCenteredLinks.userIdToUserCharacteristics.for_each(
        [&](const std::string& pKey,
            UserCharacteristics<MemoryGrdExpLinksForAMemSentence, LinksAccessType::ALL>& pUserCharacteristics) {
            auto& sharedUserCharacteristics = sharedUserCenteredLinks.userIdToUserCharacteristics[pKey];
            _addValueLinksFrom(sharedUserCharacteristics.genderLinks, pUserCharacteristics.genderLinks, _memSent.id);
            _addValueLinksFrom(sharedUserCharacteristics.nameLinks, pUserCharacteristics.nameLinks, _memSent.id);
            _addValueLinksFrom(sharedUserCharacteristics.equivalentUserIdLinks,
                               pUserCharacteristics.equivalentUserIdLinks,
                               _memSent.id);
            _addValueLinksFrom(sharedUserCharacteristics.semExpLinks, pUserCharacteristics.semExpLinks, _memSent.id);
        });
    userCenteredLinks.nameToUserIds.for_each(
        [&](const std::string& pKey,
            SemanticStringLinks<MemoryGrdExpLinksForAMemSentence, LinksAccessType::ALL>& pLinks) {
            _addValueLinksFrom(sharedUserCenteredLinks.nameToUserIds[pKey], pLinks, _memSent.id);
        });
    for (auto& currElt : userCenteredLinks.semExpToUserIds)
        _addValueLinksFrom(sharedUserCenteredLinks.semExpToUserIds[currElt.first], currElt.second, _memSent.id);
}

void GroundedExpWithLinksPrivate::linkToRequToAnswers() {
    auto& memBlocPrivate = *_memSent._contextAxiom.getSemExpWrappedForMemory().getParentMemBloc()._impl;
    _mergeRequToGrdExps(memBlocPrivate.answersLinks.getLinks(tense, verbGoal), _links);
    isLinkedInRequToAnswers = LinkedState::LINKED;
}

void GroundedExpWithLinksPrivate::linkToConditionToInformation() {
    auto& memBlocPrivate = *_memSent._contextAxiom.getSemExpWrappedForMemory().getParentMemBloc()._impl;
    _mergeRequToGrdExps(memBlocPrivate.conditionToInformationLinks.getLinks(tense, verbGoal), _links);
    isLinkedInRequToCondition = LinkedState::LINKED;
}

void GroundedExpWithLinksPrivate::linkToSentWithAction() {
    auto& memBlocPrivate = *_memSent._contextAxiom.getSemExpWrappedForMemory().getParentMemBloc()._impl;
    _mergeRequToGrdExps(memBlocPrivate.sentWithActionLinks.getLinks(tense, verbGoal), _links);
    memBlocPrivate.addConditionToAnAction(_memSent, _links);
    isLinkedInRequToSentWithAction = LinkedState::LINKED;
}

void GroundedExpWithLinksPrivate::linkToSentWithInfAction() {
    auto& memBlocPrivate = *_memSent._contextAxiom.getSemExpWrappedForMemory().getParentMemBloc()._impl;
    _mergeRequToGrdExps(memBlocPrivate.sentWithInfActionLinks, _links);
    memBlocPrivate.addInfAction(_memSent);
    isLinkedInRequToInfAction = LinkedState::LINKED;
}

void GroundedExpWithLinksPrivate::linkToActionTrigger() {
    auto& memBlocPrivate = *_memSent._contextAxiom.getSemExpWrappedForMemory().getParentMemBloc()._impl;
    _mergeRequToGrdExps(
        memBlocPrivate
            .ensureSentenceTriggersLinks(SemanticExpressionCategory::COMMAND, _memSent._contextAxiom.triggerAxiomId)
            .getLinksForAGoal(verbGoal),
        _links);
    isLinkedInRequToActionTrigger = LinkedState::LINKED;
}

void GroundedExpWithLinksPrivate::linkToQuestionTrigger() {
    auto& memBlocPrivate = *_memSent._contextAxiom.getSemExpWrappedForMemory().getParentMemBloc()._impl;
    _mergeRequToGrdExps(
        memBlocPrivate
            .ensureSentenceTriggersLinks(SemanticExpressionCategory::QUESTION, _memSent._contextAxiom.triggerAxiomId)
            .getLinksForAGoal(verbGoal),
        _links);
    isLinkedInRequToQuestionTrigger = LinkedState::LINKED;
}

void GroundedExpWithLinksPrivate::linkToAffirmationTrigger() {
    auto& memBlocPrivate = *_memSent._contextAxiom.getSemExpWrappedForMemory().getParentMemBloc()._impl;
    _mergeRequToGrdExps(
        memBlocPrivate
            .ensureSentenceTriggersLinks(SemanticExpressionCategory::AFFIRMATION, _memSent._contextAxiom.triggerAxiomId)
            .getLinksForAGoal(verbGoal),
        _links);
    isLinkedInRequToAffirmationTrigger = LinkedState::LINKED;
}

void GroundedExpWithLinksPrivate::linkToNominalTrigger() {
    auto& memBlocPrivate = *_memSent._contextAxiom.getSemExpWrappedForMemory().getParentMemBloc()._impl;
    auto it = _links.reqToGrdExps.find(SemanticRequestType::NOTHING);
    if (it != _links.reqToGrdExps.end())
        _addLinksFrom(memBlocPrivate.ensureNominalGroupsTriggersLinks(_memSent._contextAxiom.triggerAxiomId),
                      it->second,
                      _memSent.id);
    isLinkedInRequToNominalGroupsTrigger = LinkedState::LINKED;
}

void GroundedExpWithLinksPrivate::linkToRecommendationsTrigger() {
    if (_recomendationLinksPtr) {
        auto& memBlocPrivate = *_memSent._contextAxiom.getSemExpWrappedForMemory().getParentMemBloc()._impl;
        _addLinksFrom(memBlocPrivate.ensureRecommendationsTriggersLinks(_memSent._contextAxiom.triggerAxiomId),
                      *_recomendationLinksPtr,
                      _memSent.id);
    }
    isLinkedInRequToRecommendationsTrigger = LinkedState::LINKED;
}

void GroundedExpWithLinksPrivate::disableUserCenteredLinks() {
    auto& sharedUserCenteredLinks =
        _memSent._contextAxiom.getSemExpWrappedForMemory().getParentMemBloc()._impl->userCenteredLinks;
    for (const auto& currElt : userCenteredLinks.userIdToUserCharacteristics) {
        auto subItToFilter = sharedUserCenteredLinks.userIdToUserCharacteristics.find(currElt.first);
        assert(subItToFilter != sharedUserCenteredLinks.userIdToUserCharacteristics.end());
        auto& userCharacteristics = subItToFilter->second;
        _removeValueLinksFrom(userCharacteristics.genderLinks, currElt.second.genderLinks, _memSent.id);
        _removeValueLinksFrom(userCharacteristics.nameLinks, currElt.second.nameLinks, _memSent.id);
        _removeValueLinksFrom(
            userCharacteristics.equivalentUserIdLinks, currElt.second.equivalentUserIdLinks, _memSent.id);
        _removeValueLinksFrom(userCharacteristics.semExpLinks, currElt.second.semExpLinks, _memSent.id);
        if (userCharacteristics.empty())
            sharedUserCenteredLinks.userIdToUserCharacteristics.erase(subItToFilter);
    }
    for (const auto& currElt : userCenteredLinks.nameToUserIds) {
        auto subItToFilter = sharedUserCenteredLinks.nameToUserIds.find(currElt.first);
        _removeValueLinksFrom(subItToFilter->second, currElt.second, _memSent.id);
        if (subItToFilter->second.empty())
            sharedUserCenteredLinks.nameToUserIds.erase(subItToFilter);
    }
    for (const auto& currElt : userCenteredLinks.semExpToUserIds) {
        auto subItToFilter = sharedUserCenteredLinks.semExpToUserIds.find(currElt.first);
        _removeValueLinksFrom(subItToFilter->second, currElt.second, _memSent.id);
        if (subItToFilter->second.empty())
            sharedUserCenteredLinks.semExpToUserIds.erase(subItToFilter);
    }
}

void GroundedExpWithLinksPrivate::writeInBinary(binarymasks::Ptr& pPtr,
                                                MemGrdExpPtrOffsets& pMemGrdExpPtrs,
                                                const semexpsaver::SemExpPtrOffsets& pSemExpPtrOffsets) const {
    unsigned char i = 0;
    auto* nbOfEltsPtr = pPtr.puchar++;
    for (const auto& currChild : _childGrdExpMemories) {
        pMemGrdExpPtrs.addMemGrdExp(currChild, pPtr.puchar);
        binarysaver::writeInThreeBytes(pPtr.pchar, pSemExpPtrOffsets.grdExpToOffset(currChild.grdExp, pPtr.puchar));
        pPtr.pchar += 3;
        *(pPtr.puchar++) = i++;
    }
    assert(i < 255);
    *nbOfEltsPtr = i;
}

void GroundedExpWithLinksPrivate::filterMemLinks(SemanticMemoryLinks& pSemanticMemoryLinksToFilter) const {
    for (const auto& currReq : _links.reqToGrdExps) {
        auto it = pSemanticMemoryLinksToFilter.reqToGrdExps.find(currReq.first);
        if (it != pSemanticMemoryLinksToFilter.reqToGrdExps.end()) {
            _removeLinksFrom(it->second, currReq.second, _memSent.id);
            if (it->second.empty())
                pSemanticMemoryLinksToFilter.reqToGrdExps.erase(it);
        } else {
            std::cerr << "Error on unliknking a grounded expression from the memory" << std::endl;
            assert(false);
        }
    }
}

void GroundedExpWithLinksPrivate::filterNominalGroupLinks(SemanticLinksToGrdExps& pLinksToFilter) const {
    auto it = _links.reqToGrdExps.find(SemanticRequestType::NOTHING);
    if (it != _links.reqToGrdExps.end())
        _removeLinksFrom(pLinksToFilter, it->second, _memSent.id);
}

void GroundedExpWithLinksPrivate::filterRecommendationsLinks(SemanticLinksToGrdExps& pLinksToFilter) const {
    if (_recomendationLinksPtr)
        _removeLinksFrom(pLinksToFilter, *_recomendationLinksPtr, _memSent.id);
}

void GroundedExpWithLinksPrivate::_linkRecommendationSemExp(const SemanticExpression& pSemExp,
                                                            const linguistics::LinguisticDatabase& pLingDb) {
    const auto* grdExpPtr = pSemExp.getGrdExpPtr_SkipWrapperPtrs();
    if (grdExpPtr != nullptr)
        _linkRecommendationGrdExp(*grdExpPtr, pLingDb);
    else {
        const auto* listExpPtr = pSemExp.getListExpPtr_SkipWrapperPtrs();
        if (listExpPtr != nullptr)
            for (const auto& currElt : listExpPtr->elts)
                _linkRecommendationSemExp(*currElt, pLingDb);
    }
}

void GroundedExpWithLinksPrivate::_linkRecommendationGrdExp(const GroundedExpression& pGrdExp,
                                                            const linguistics::LinguisticDatabase& pLingDb) {
    const SemanticGrounding& grd = pGrdExp.grounding();

    bool linkGrd = true;
    // Do not link "be" verb in the recommendations
    {
        const auto* startGrdPtr = grd.getStatementGroundingPtr();
        if (startGrdPtr != nullptr && ConceptSet::haveAConceptThatBeginWith(startGrdPtr->concepts, "verb_equal_"))
            linkGrd = false;
    }

    if (linkGrd)
        _linkGrdExp(
            [&]() -> auto& { return *_recomendationLinksPtr; }, pGrdExp, grd, pLingDb, false);
    for (const auto& currChild : pGrdExp.children)
        _linkRecommendationSemExp(*currChild.second, pLingDb);
}

void GroundedExpWithLinksPrivate::_linkStatementGrdExp(const SemanticStatementGrounding& pStatGrd,
                                                       const linguistics::LinguisticDatabase& pLingDb) {
    bool skipLinkage = false;
    skipLinkage = !_linkGrdExp(
        [&]() -> auto& { return _links.reqToGrdExps[SemanticRequestType::ACTION]; },
        _memSent.grdExp,
        pStatGrd,
        pLingDb,
        true);

    // link sub expressions
    if (!skipLinkage) {
        for (const auto& currChild : _memSent.grdExp.children) {
            SemanticRequestType newRequ = SemExpGetter::convertSemGramToRequestType(currChild.first);
            if (newRequ != SemanticRequestType::NOTHING
                && !_linkChildSemExp(*currChild.second, newRequ, pLingDb, true)) {
                skipLinkage = true;
                break;
            }
        }

        if (!skipLinkage) {
            for (const auto& currAnnotation : _memSent._annotations) {
                SemanticRequestType newRequ = SemExpGetter::convertSemGramToRequestType(currAnnotation.first);
                if (newRequ != SemanticRequestType::NOTHING
                    && !_linkChildSemExp(*currAnnotation.second, newRequ, pLingDb, true))
                    break;
            }
        }
    }

    if (_memSent._isEnabled && !_memSent._isAConditionToSatisfy)
        enableUserCenteredLinks();

    if (_memSent._contextAxiom.getSemExpWrappedForMemory().outputToAnswerIfTriggerHasMatched) {
        if (pStatGrd.requests.empty()) {
            if (_memSent._isEnabled)
                linkToAffirmationTrigger();
            else
                isLinkedInRequToAffirmationTrigger = LinkedState::LINKDISABLED;
        } else if (pStatGrd.requests.first() == SemanticRequestType::ACTION) {
            if (_memSent._isEnabled)
                linkToActionTrigger();
            else
                isLinkedInRequToActionTrigger = LinkedState::LINKDISABLED;
        } else {
            if (_memSent._isEnabled)
                linkToQuestionTrigger();
            else
                isLinkedInRequToQuestionTrigger = LinkedState::LINKDISABLED;
        }
    } else if (_memSent._contextAxiom.infCommandToDo != nullptr) {
        if (_memSent._isEnabled)
            linkToSentWithInfAction();
        else
            isLinkedInRequToInfAction = LinkedState::LINKDISABLED;
    } else if (_memSent._contextAxiom.semTracker || _memSent._contextAxiom.semExpToDo != nullptr) {
        _memSent._isAConditionToSatisfy = true;
        if (_memSent._isEnabled)
            linkToSentWithAction();
        else
            isLinkedInRequToSentWithAction = LinkedState::LINKDISABLED;
    } else if (_memSent._isAConditionToSatisfy) {
        if (_memSent._isEnabled)
            linkToConditionToInformation();
        else
            isLinkedInRequToCondition = LinkedState::LINKDISABLED;
    } else if (pStatGrd.requests.empty()) {
        if (_memSent._isEnabled)
            linkToRequToAnswers();
        else
            isLinkedInRequToAnswers = LinkedState::LINKDISABLED;
    }
}

bool GroundedExpWithLinksPrivate::_linkChildSemExp(const SemanticExpression& pSemExpression,
                                                   SemanticRequestType pFromRequest,
                                                   const linguistics::LinguisticDatabase& pLingDb,
                                                   bool pLinkNonSpecificStuffs) {
    const bool followInterpretations = !_memSent.isATrigger();
    const GroundedExpression* childGrdExp = pSemExpression.getGrdExpPtr_SkipWrapperPtrs(followInterpretations);
    if (childGrdExp != nullptr) {
        if (childGrdExp == &_memSent.grdExp)
            return true;

        const auto& childGrounding = childGrdExp->grounding();

        if (childGrounding.type == SemanticGroundingType::AGENT) {
            auto* originalIntPtr = pSemExpression.getIntExpPtr();
            if (originalIntPtr != nullptr) {
                auto* originalGrdExpPtr =
                    originalIntPtr->originalExp->getGrdExpPtr_SkipWrapperPtrs(followInterpretations);
                if (originalGrdExpPtr != nullptr
                    && originalGrdExpPtr->grounding().type == SemanticGroundingType::AGENT) {
                    _childGrdExpMemories.emplace_back(_memSent, *originalGrdExpPtr);
                    SemanticMemoryGrdExp& newMemGrdExp = _childGrdExpMemories.back();
                    _fillUserCenteredLinks(newMemGrdExp, *originalGrdExpPtr);
                }
            }
        }

        if (!_linkGrdExp(
                [&]() -> auto& { return _links.reqToGrdExps[pFromRequest]; },
                *childGrdExp,
                childGrounding,
                pLingDb,
                pLinkNonSpecificStuffs))
            return false;

        auto itSpechild = childGrdExp->children.find(GrammaticalType::SPECIFIER);
        if (itSpechild != childGrdExp->children.end()
            && !_linkChildSemExp(*itSpechild->second, pFromRequest, pLingDb, false))
            return false;

        auto itSubCptChild = childGrdExp->children.find(GrammaticalType::SUB_CONCEPT);
        if (itSubCptChild != childGrdExp->children.end()
            && !_linkChildSemExp(*itSubCptChild->second, pFromRequest, pLingDb, false))
            return false;

        if (pFromRequest == SemanticRequestType::OBJECT) {
            auto itTimeChild = childGrdExp->children.find(GrammaticalType::TIME);
            if (itTimeChild != childGrdExp->children.end()
                && !_linkChildSemExp(*itTimeChild->second, pFromRequest, pLingDb, false))
                return false;
        }

        return true;
    }

    const ListExpression* chilListExp = pSemExpression.getListExpPtr();
    if (chilListExp != nullptr) {
        for (const auto& currElt : chilListExp->elts)
            if (!_linkChildSemExp(*currElt, pFromRequest, pLingDb, pLinkNonSpecificStuffs))
                return false;
    }
    return true;
}

void GroundedExpWithLinksPrivate::_fillUserCenteredLinks(SemanticMemoryGrdExp& pNewMemGrdExp,
                                                         const GroundedExpression& pGrdExp) {
    _addGenderFromGrdExp(pNewMemGrdExp, pGrdExp);
    RegisterNameLinks registerNameLinks(userCenteredLinks, pNewMemGrdExp);
    SemExpUserInfosFiller::tryToAddUserInfosWithTense(registerNameLinks, pGrdExp, tense);
}

bool GroundedExpWithLinksPrivate::_linkGrdExp(
    const std::function<SemanticLinksToGrdExpsTemplate<MemoryGrdExpLinksForAMemSentence>&()>& pEnsureLinksToGrdExps,
    const GroundedExpression& pGrdExp,
    const SemanticGrounding& pGrounding,
    const linguistics::LinguisticDatabase& pLingDb,
    bool pLinkNonSpecificStuffs) {
    if (pGrounding.type == SemanticGroundingType::GENERIC && pGrounding.getGenericGrounding().coreference
        && pGrounding.getGenericGrounding().coreference->getDirection() == CoreferenceDirectionEnum::PARENT)
        return true;
    _childGrdExpMemories.emplace_back(_memSent, pGrdExp);
    SemanticMemoryGrdExp& newMemGrdExp = _childGrdExpMemories.back();

    // add links of user centered information
    if (&pGrdExp == &_memSent.grdExp)
        _fillUserCenteredLinks(newMemGrdExp, pGrdExp);

    // link the concepts
    if (pGrounding.type != SemanticGroundingType::TIME)
        for (const auto& currCpt : pGrounding.concepts)
            pEnsureLinksToGrdExps().conceptsToSemExps[currCpt.first].emplace_back(newMemGrdExp);

    switch (pGrounding.type) {
        case SemanticGroundingType::GENERIC: {
            const SemanticGenericGrounding& genGrounding = pGrounding.getGenericGrounding();
            if (!ConceptSet::haveAConceptNotAny(genGrounding.concepts)) {
                StaticLinguisticMeaning lingMeaning =
                    pLingDb.langToSpec[genGrounding.word.language].lingDico.statDb.getLingMeaning(
                        genGrounding.word.lemma, genGrounding.word.partOfSpeech, true);
                // link the word from static binary dico
                if (!lingMeaning.isEmpty()) {
                    _linkLingMeaning(pEnsureLinksToGrdExps(), newMemGrdExp, genGrounding.word, lingMeaning);
                } else if (!genGrounding.word.lemma.empty()) {
                    // link the lemma
                    pEnsureLinksToGrdExps()
                        .textToSemExps[SemanticLanguageEnum::UNKNOWN][genGrounding.word.lemma]
                        .emplace_back(newMemGrdExp);
                } else if (pLinkNonSpecificStuffs && !genGrounding.coreference
                           && genGrounding.referenceType == SemanticReferenceType::INDEFINITE) {
                    if (genGrounding.quantity.type == SemanticQuantityType::EVERYTHING
                        || genGrounding.quantity.isEqualToZero())
                        pEnsureLinksToGrdExps().everythingOrNoEntityTypeToSemExps[genGrounding.entityType].emplace_back(
                            newMemGrdExp);
                    else    // link the non specific stuffs
                        pEnsureLinksToGrdExps().genGroundingTypeToSemExps[genGrounding.entityType].emplace_back(
                            newMemGrdExp);
                }

                if (genGrounding.word.lemma.empty() && !ConceptSet::haveAConceptNotAny(genGrounding.concepts)) {
                    if (genGrounding.quantity.type == SemanticQuantityType::NUMBER) {
                        pEnsureLinksToGrdExps().numberToSemExps[genGrounding.quantity.nb].emplace_back(newMemGrdExp);
                    } else if (genGrounding.quantity.type != SemanticQuantityType::UNKNOWN) {
                        pEnsureLinksToGrdExps().quantityTypeToSemExps[genGrounding.quantity.type].emplace_back(
                            newMemGrdExp);
                    } else if (genGrounding.referenceType != SemanticReferenceType::UNDEFINED) {
                        pEnsureLinksToGrdExps()
                            .referenceWithoutConceptToSemExps[genGrounding.referenceType]
                            .emplace_back(newMemGrdExp);
                    } else if (genGrounding.coreference) {
                        pEnsureLinksToGrdExps()
                            .coreferenceWithoutConceptOrReferenceToSemExps[genGrounding.coreference->getDirection()]
                            .emplace_back(newMemGrdExp);
                    }
                }

                // Link also the lemma in lower case
                auto lemmaInLowerCase = genGrounding.word.lemma;
                if (lowerCaseText(lemmaInLowerCase))
                    pEnsureLinksToGrdExps().textToSemExps[SemanticLanguageEnum::UNKNOWN][lemmaInLowerCase].emplace_back(
                        newMemGrdExp);
            }
            break;
        }
        case SemanticGroundingType::STATEMENT: {
            const SemanticStatementGrounding& statGrounding = pGrounding.getStatementGrounding();
            if (!ConceptSet::haveAConceptNotAny(statGrounding.concepts)) {
                const auto& statLingDico = pLingDb.langToSpec[statGrounding.word.language].lingDico.statDb;
                StaticLinguisticMeaning lingMeaning =
                    statLingDico.getLingMeaning(statGrounding.word.lemma, statGrounding.word.partOfSpeech, true);

                // link the words from static binary dico
                if (!lingMeaning.isEmpty())
                    _linkLingMeaning(pEnsureLinksToGrdExps(), newMemGrdExp, statGrounding.word, lingMeaning);
            }

            if (statGrounding.coreference && statGrounding.word.lemma.empty() && statGrounding.concepts.empty()) {
                pEnsureLinksToGrdExps()
                    .statementCoreferenceWithoutConceptToSemExps[statGrounding.coreference->getDirection()]
                    .emplace_back(newMemGrdExp);
            }
            break;
        }
        case SemanticGroundingType::AGENT: {
            const SemanticAgentGrounding& agentGrounding = pGrounding.getAgentGrounding();
            pEnsureLinksToGrdExps().userIdToSemExps[agentGrounding.userId].emplace_back(newMemGrdExp);
            pEnsureLinksToGrdExps().userIdWithoutContextToSemExps[agentGrounding.userIdWithoutContext].emplace_back(
                newMemGrdExp);
            break;
        }
        case SemanticGroundingType::TEXT: {
            const SemanticTextGrounding& textGrounding = pGrounding.getTextGrounding();
            pEnsureLinksToGrdExps().textToSemExps[textGrounding.forLanguage][textGrounding.text].emplace_back(
                newMemGrdExp);
            break;
        }
        case SemanticGroundingType::TIME: {
            const SemanticTimeGrounding& timeGrounding = pGrounding.getTimeGrounding();
            _addTimeLinksFromGrounding(pEnsureLinksToGrdExps(), timeGrounding, newMemGrdExp);
            break;
        }
        case SemanticGroundingType::DURATION: {
            const SemanticDurationGrounding& durationGrounding = pGrounding.getDurationGrounding();
            pEnsureLinksToGrdExps().durationToSemExps[durationGrounding.duration].emplace_back(newMemGrdExp);
            break;
        }
        case SemanticGroundingType::RELATIVELOCATION: {
            const SemanticRelativeLocationGrounding& relLocationGrounding = pGrounding.getRelLocationGrounding();
            pEnsureLinksToGrdExps().relLocationToSemExps[relLocationGrounding.locationType].emplace_back(newMemGrdExp);
            break;
        }
        case SemanticGroundingType::RELATIVETIME: {
            const SemanticRelativeTimeGrounding& relTimeGrounding = pGrounding.getRelTimeGrounding();
            if (relTimeGrounding.timeType == SemanticRelativeTimeType::DELAYEDSTART) {
                auto itSpecifierChild = pGrdExp.children.find(GrammaticalType::SPECIFIER);
                if (itSpecifierChild != pGrdExp.children.end()) {
                    auto* specifierGrdExpPtr = itSpecifierChild->second->getGrdExpPtr_SkipWrapperPtrs();
                    if (specifierGrdExpPtr != nullptr) {
                        auto* durationGrdPtr = specifierGrdExpPtr->grounding().getDurationGroundingPtr();
                        if (durationGrdPtr != nullptr) {
                            SemanticDuration absoluteTimeDuration =
                                SemanticTimeGrounding::relativeToAbsolute(durationGrdPtr->duration);
                            _addTimeLinksFromRelativeDurationGrounding(
                                pEnsureLinksToGrdExps(), absoluteTimeDuration, newMemGrdExp);
                        }
                    }
                }
            }
            pEnsureLinksToGrdExps().relTimeToSemExps[relTimeGrounding.timeType].emplace_back(newMemGrdExp);
            break;
        }
        case SemanticGroundingType::META: {
            const SemanticMetaGrounding& metaGrounding = pGrounding.getMetaGrounding();
            pEnsureLinksToGrdExps().grdTypeToSemExps[metaGrounding.refToType].emplace_back(newMemGrdExp);
            break;
        }
        case SemanticGroundingType::NAME: {
            const SemanticNameGrounding& nameGrounding = pGrounding.getNameGrounding();
            for (const auto& currName : nameGrounding.nameInfos.names)
                pEnsureLinksToGrdExps().textToSemExps[SemanticLanguageEnum::UNKNOWN][currName].emplace_back(
                    newMemGrdExp);
            break;
        }
        case SemanticGroundingType::LANGUAGE: {
            const SemanticLanguageGrounding& languageGrounding = pGrounding.getLanguageGrounding();
            pEnsureLinksToGrdExps().languageToSemExps[languageGrounding.language].emplace_back(newMemGrdExp);
            break;
        }
        case SemanticGroundingType::RESOURCE: {
            const auto& resGrd = pGrounding.getResourceGrounding();
            pEnsureLinksToGrdExps().resourceToSemExps[resGrd.resource].emplace_back(newMemGrdExp);
            break;
        }
        case SemanticGroundingType::UNITY: {
            const auto& unityGrd = pGrounding.getUnityGrounding();
            pEnsureLinksToGrdExps().conceptsToSemExps[unityGrd.getValueConcept()].emplace_back(newMemGrdExp);
            break;
        }
        case SemanticGroundingType::ANGLE:
        case SemanticGroundingType::CONCEPTUAL:
        case SemanticGroundingType::LENGTH:
        case SemanticGroundingType::PERCENTAGE:
        case SemanticGroundingType::RELATIVEDURATION: break;
    }
    return true;
}

void GroundedExpWithLinksPrivate::_linkLingMeaning(
    SemanticLinksToGrdExpsTemplate<MemoryGrdExpLinksForAMemSentence>& pLinksToGrdExps,
    SemanticMemoryGrdExp& pNewMemGrdExp,
    const SemanticWord& pInWord,
    const StaticLinguisticMeaning& pInLingMeaning) {
    pLinksToGrdExps.meaningsToSemExps[pInWord.language][pInLingMeaning.meaningId].emplace_back(pNewMemGrdExp);
}

void GroundedExpWithLinksPrivate::_mergeRequToGrdExps(SemanticMemoryLinks& pToComplete,
                                                      SemanticMemoryLinksForAMemSentence& pRef) {
    for (auto& currReq : pRef.reqToGrdExps)
        _addLinksFrom(pToComplete.reqToGrdExps[currReq.first], currReq.second, _memSent.id);
}

void GroundedExpWithLinksPrivate::_addGenderFromGrdExp(SemanticMemoryGrdExp& pNewMemGrdExp,
                                                       const GroundedExpression& pGrdExp) {
    const SemanticGrounding& grounding = pGrdExp.grounding();
    if (grounding.type == SemanticGroundingType::STATEMENT
        && ConceptSet::haveAConcept(grounding.concepts, "verb_equal_be")) {
        _tryToAddAgentGenderFromAStructureEqualityStatement(pNewMemGrdExp, pGrdExp);
        for (auto& currChild : pGrdExp.children)
            _addGenderFromSemExp(pNewMemGrdExp, currChild.second);
    }
}

void GroundedExpWithLinksPrivate::_addGenderFromSemExp(SemanticMemoryGrdExp& pNewMemGrdExp,
                                                       const UniqueSemanticExpression& pSemExp) {
    const GroundedExpression* grdExp = pSemExp->getGrdExpPtr();
    if (grdExp != nullptr) {
        _addGenderFromGrdExp(pNewMemGrdExp, *grdExp);
        return;
    }

    const ListExpression* listExp = pSemExp->getListExpPtr();
    if (listExp != nullptr) {
        for (auto& currElt : listExp->elts) {
            _addGenderFromSemExp(pNewMemGrdExp, currElt);
        }
        return;
    }
}

void GroundedExpWithLinksPrivate::_tryToAddAgentGenderFromAStructureEqualityStatement(
    SemanticMemoryGrdExp& pNewMemGrdExp,
    const GroundedExpression& pGrdExp) {
    auto itSubject = pGrdExp.children.find(GrammaticalType::SUBJECT);
    if (itSubject != pGrdExp.children.end()) {
        const GroundedExpression* subjectGrdExp = itSubject->second->getGrdExpPtr_SkipWrapperPtrs();
        if (subjectGrdExp != nullptr) {
            const SemanticAgentGrounding* subjectAgentGrd = (*subjectGrdExp)->getAgentGroundingPtr();
            if (subjectAgentGrd != nullptr && subjectAgentGrd->isSpecificUser()) {
                auto itObject = pGrdExp.children.find(GrammaticalType::OBJECT);
                if (itObject != pGrdExp.children.end()) {
                    const GroundedExpression* objectGrdExp = itObject->second->getGrdExpPtr_SkipWrapperPtrs();
                    if (objectGrdExp != nullptr) {
                        const SemanticGenericGrounding* objectGenGrd = (*objectGrdExp)->getGenericGroundingPtr();
                        if (objectGenGrd != nullptr) {
                            SemanticGenderType gender = SemExpGetter::getGenderFromGenGrd(*objectGenGrd);
                            if (gender != SemanticGenderType::UNKNOWN)
                                _addValueForAMemGrdExp(
                                    userCenteredLinks.userIdToUserCharacteristics[subjectAgentGrd->userId].genderLinks,
                                    gender,
                                    pNewMemGrdExp);
                        } else {
                            const SemanticNameGrounding* objectNameGrdPtr = (*objectGrdExp)->getNameGroundingPtr();
                            if (objectNameGrdPtr != nullptr) {
                                SemanticGenderType gender =
                                    SemExpGetter::possibleGendersToGender(objectNameGrdPtr->nameInfos.possibleGenders);
                                if (gender != SemanticGenderType::UNKNOWN)
                                    _addValueForAMemGrdExp(
                                        userCenteredLinks.userIdToUserCharacteristics[subjectAgentGrd->userId]
                                            .genderLinks,
                                        gender,
                                        pNewMemGrdExp);
                            }
                        }
                    }
                }
            }
        }
    }
}

MemGrdExpPtrOffsets::MemGrdExpPtrOffsets(unsigned char* pBeginPtr)
    : _beginPtr(pBeginPtr)
    , _memGrdExpPtrs() {}

void MemGrdExpPtrOffsets::addMemGrdExp(const SemanticMemoryGrdExp& pMemGrdExp, unsigned char* pPtr) {
    _memGrdExpPtrs.emplace(&pMemGrdExp, pPtr - _beginPtr);
}

uint32_t MemGrdExpPtrOffsets::getOffset(const SemanticMemoryGrdExp& pMemGrdExp) const {
    auto it = _memGrdExpPtrs.find(&pMemGrdExp);
    if (it != _memGrdExpPtrs.end())
        return it->second;
    assert(false);
    return 0u;
}

GroundedExpWithLinks::GroundedExpWithLinks(SentenceWithLinks& pContextAxiom,
                                           const GroundedExpression& pGrdExp,
                                           bool pInRecommendationMode,
                                           const std::map<GrammaticalType, const SemanticExpression*>& pAnnotations,
                                           bool pIsATrigger,
                                           const linguistics::LinguisticDatabase& pLingDb,
                                           bool pIsAConditionToSatisfy,
                                           bool pIsEnabled,
                                           intSemId pId)
    : id(pId == 0 ? pContextAxiom.getSemExpWrappedForMemory().getParentMemBloc().getNextSemId() : pId)
    , grdExp(pGrdExp)
    , inRecommendationMode(pInRecommendationMode)
    , _contextAxiom(pContextAxiom)
    , _annotations()
    , _isATrigger(pIsATrigger)
    , _isEnabled(pIsEnabled)
    , _isANoun(false)
    , _isAConditionToSatisfy(pIsAConditionToSatisfy)
    , _impl(std::make_unique<GroundedExpWithLinksPrivate>(*this, pAnnotations, pLingDb)) {}

GroundedExpWithLinks::~GroundedExpWithLinks() {}

bool GroundedExpWithLinks::isOtherSentenceMoreRevelant(const GroundedExpWithLinks& pOther) const {
    return _contextAxiom.canOtherInformationTypeBeMoreRevelant(pOther._contextAxiom.informationType) && id < pOther.id;
}

std::string GroundedExpWithLinks::getName(const std::string& pUserId) const {
    return _impl->userCenteredLinks.getName(pUserId);
}

bool GroundedExpWithLinks::hasEquivalentUserIds(const std::string& pUserId) const {
    return _impl->userCenteredLinks.hasEquivalentUserIds(pUserId);
}

const SemanticExpression* GroundedExpWithLinks::getSemExpForGrammaticalType(GrammaticalType pGrammType) const {
    const auto itAnn = _annotations.find(pGrammType);
    if (itAnn != _annotations.end())
        return itAnn->second;
    const auto itChild = grdExp.children.find(pGrammType);
    if (itChild != grdExp.children.end())
        return &*itChild->second;

    // Get also time children linked to an infinite subordinate verb
    if (pGrammType == GrammaticalType::TIME) {
        auto* rawRes = getSemExpForGrammaticalType(GrammaticalType::OBJECT);
        if (rawRes != nullptr) {
            auto* objGrdExpPtr = rawRes->getGrdExpPtr_SkipWrapperPtrs();
            if (objGrdExpPtr != nullptr && SemExpGetter::isAnInfinitiveGrdExp(*objGrdExpPtr))
                return SemExpGetter::getChildFromGrdExp(*objGrdExpPtr, pGrammType);
        }
    }
    return nullptr;
}

void GroundedExpWithLinks::writeInBinary(binarymasks::Ptr& pPtr,
                                         MemGrdExpPtrOffsets& pMemGrdExpPtrs,
                                         const semexpsaver::SemExpPtrOffsets& pSemExpPtrOffsets) const {
    binarysaver::writeInt(pPtr.pint++, id);
    binarysaver::writeInThreeBytes(pPtr.pchar, pSemExpPtrOffsets.grdExpToOffset(grdExp, pPtr.puchar));
    pPtr.pchar += 3;
    _impl->writeInBinary(pPtr, pMemGrdExpPtrs, pSemExpPtrOffsets);
    writeEnumMap<GrammaticalType, const SemanticExpression*>(
        pPtr,
        _annotations,
        grammaticalType_allValues,
        [&](binarymasks::Ptr&, const SemanticExpression* const* pSemExpPtr) {
            if (pSemExpPtr == nullptr)
                binarysaver::writeInThreeBytes(pPtr.pchar, 0);
            else
                binarysaver::writeInThreeBytes(pPtr.pchar, pSemExpPtrOffsets.semExpToOffset(**pSemExpPtr, pPtr.puchar));
            pPtr.pchar += 3;
        });
}

void GroundedExpWithLinks::setEnabled(bool pEnabled) {
    if (_isEnabled == pEnabled)
        return;
    _isEnabled = pEnabled;

    auto& impl = *_impl;
    if (_isEnabled) {
        impl.enableUserCenteredLinks();
        if (impl.isLinkedInRequToAnswers == LinkedState::LINKDISABLED)
            impl.linkToRequToAnswers();

        if (impl.isLinkedInRequToCondition == LinkedState::LINKDISABLED)
            impl.linkToConditionToInformation();

        if (impl.isLinkedInRequToSentWithAction == LinkedState::LINKDISABLED)
            impl.linkToSentWithAction();

        if (impl.isLinkedInRequToInfAction == LinkedState::LINKDISABLED)
            impl.linkToSentWithInfAction();

        if (impl.isLinkedInRequToActionTrigger == LinkedState::LINKDISABLED)
            impl.linkToActionTrigger();

        if (impl.isLinkedInRequToQuestionTrigger == LinkedState::LINKDISABLED)
            impl.linkToQuestionTrigger();

        if (impl.isLinkedInRequToAffirmationTrigger == LinkedState::LINKDISABLED)
            impl.linkToAffirmationTrigger();

        if (impl.isLinkedInRequToNominalGroupsTrigger == LinkedState::LINKDISABLED)
            impl.linkToNominalTrigger();

        if (impl.isLinkedInRequToRecommendationsTrigger == LinkedState::LINKDISABLED)
            impl.linkToRecommendationsTrigger();
    } else {
        impl.disableUserCenteredLinks();
        auto& memBlocPrivate = *_contextAxiom.getSemExpWrappedForMemory().getParentMemBloc()._impl;
        if (impl.isLinkedInRequToAnswers == LinkedState::LINKED) {
            impl.filterMemLinks(memBlocPrivate.answersLinks.getLinks(impl.tense, impl.verbGoal));
            impl.isLinkedInRequToAnswers = LinkedState::LINKDISABLED;
        }

        if (impl.isLinkedInRequToCondition == LinkedState::LINKED) {
            impl.filterMemLinks(memBlocPrivate.conditionToInformationLinks.getLinks(impl.tense, impl.verbGoal));
            impl.isLinkedInRequToCondition = LinkedState::LINKDISABLED;
        }

        if (impl.isLinkedInRequToSentWithAction == LinkedState::LINKED) {
            impl.filterMemLinks(memBlocPrivate.sentWithActionLinks.getLinks(impl.tense, impl.verbGoal));
            memBlocPrivate.removeConditionToAnAction(*this);
            impl.isLinkedInRequToSentWithAction = LinkedState::LINKDISABLED;
        }

        if (impl.isLinkedInRequToInfAction == LinkedState::LINKED) {
            impl.filterMemLinks(memBlocPrivate.sentWithInfActionLinks);
            memBlocPrivate.removeInfAction(*this);
            impl.isLinkedInRequToInfAction = LinkedState::LINKDISABLED;
        }

        if (impl.isLinkedInRequToActionTrigger == LinkedState::LINKED) {
            impl.filterMemLinks(
                memBlocPrivate
                    .ensureSentenceTriggersLinks(SemanticExpressionCategory::COMMAND, _contextAxiom.triggerAxiomId)
                    .getLinksForAGoal(impl.verbGoal));
            memBlocPrivate.removeLinksIfEmpty(_contextAxiom.triggerAxiomId);
            impl.isLinkedInRequToActionTrigger = LinkedState::LINKDISABLED;
        }

        if (impl.isLinkedInRequToQuestionTrigger == LinkedState::LINKED) {
            impl.filterMemLinks(
                memBlocPrivate
                    .ensureSentenceTriggersLinks(SemanticExpressionCategory::QUESTION, _contextAxiom.triggerAxiomId)
                    .getLinksForAGoal(impl.verbGoal));
            memBlocPrivate.removeLinksIfEmpty(_contextAxiom.triggerAxiomId);
            impl.isLinkedInRequToQuestionTrigger = LinkedState::LINKDISABLED;
        }

        if (impl.isLinkedInRequToAffirmationTrigger == LinkedState::LINKED) {
            impl.filterMemLinks(
                memBlocPrivate
                    .ensureSentenceTriggersLinks(SemanticExpressionCategory::AFFIRMATION, _contextAxiom.triggerAxiomId)
                    .getLinksForAGoal(impl.verbGoal));
            memBlocPrivate.removeLinksIfEmpty(_contextAxiom.triggerAxiomId);
            impl.isLinkedInRequToAffirmationTrigger = LinkedState::LINKDISABLED;
        }

        if (impl.isLinkedInRequToNominalGroupsTrigger == LinkedState::LINKED) {
            impl.filterNominalGroupLinks(memBlocPrivate.ensureNominalGroupsTriggersLinks(_contextAxiom.triggerAxiomId));
            memBlocPrivate.removeLinksIfEmpty(_contextAxiom.triggerAxiomId);
            impl.isLinkedInRequToNominalGroupsTrigger = LinkedState::LINKDISABLED;
        }

        if (impl.isLinkedInRequToRecommendationsTrigger == LinkedState::LINKED) {
            impl.filterRecommendationsLinks(
                memBlocPrivate.ensureRecommendationsTriggersLinks(_contextAxiom.triggerAxiomId));
            memBlocPrivate.removeLinksIfEmpty(_contextAxiom.triggerAxiomId);
            impl.isLinkedInRequToRecommendationsTrigger = LinkedState::LINKDISABLED;
        }
    }
}

std::map<std::string, std::vector<UniqueSemanticExpression>> GroundedExpWithLinksWithParameters::cloneParameters()
    const {
    std::map<std::string, std::vector<UniqueSemanticExpression>> res;
    for (auto& currParam : parametersLabelsToValue)
        for (auto& currParamValue : currParam.second)
            res[currParam.first].push_back(currParamValue->clone());
    return res;
}

}    // End of namespace onsem
