#include <onsem/semantictotext/semanticmemory/semanticmemorysentence.hpp>
#include <onsem/common/binary/binarysaver.hpp>
#include <onsem/common/binary/enummapsaver.hpp>
#include <onsem/common/utility/random.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpressions.hpp>
#include <onsem/texttosemantic/dbtype/semanticgroundings.hpp>
#include <onsem/texttosemantic/tool/semexpgetter.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase.hpp>
#include <onsem/semantictotext/semanticmemory/semanticcontextaxiom.hpp>
#include <onsem/semantictotext/semanticmemory/expressionhandleinmemory.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemorybloc.hpp>
#include "../tool/userinfosfiller.hpp"
#include "semanticmemoryblockprivate.hpp"

namespace onsem
{

namespace
{

bool _isOtherGrdExpMoreRevelant(const SemanticMemoryGrdExp& pSemMemGrdExp1,
                                const SemanticMemoryGrdExp& pSemMemGrdExp2)
{
  return pSemMemGrdExp1.getMemSentence().isOtherSentenceMoreRevelant(pSemMemGrdExp2.getMemSentence());
}

bool _areLinksMoreRevelant(const SemanticSplitAssertAndInformLinks<MemoryGrdExpLinks>& pNewLinks,
                           const SemanticSplitAssertAndInformLinks<MemoryGrdExpLinks>& pMainLinks)
{
  if (!pNewLinks.assertions.empty())
  {
    auto& newGrdExps = (--pNewLinks.assertions.end())->second;
    if (!newGrdExps.empty())
    {
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
void _addValueLinksFrom(SemanticValueLinks<TVALUE, MemoryGrdExpLinks, MAP_TYPE1, access>& pToFill,
                        SemanticValueLinks<TVALUE, MemoryGrdExpLinksForAMemSentence, MAP_TYPE2, LinksAccessType::ALL>& pRef,
                        intSemId pMemSentenceId)
{
  pRef.iterateOnKeysValues([&](const TVALUE& pKey,
                               SemanticSplitAssertAndInformLinks<MemoryGrdExpLinksForAMemSentence>& pLinksToAdd)
  {
    pToFill.addValue(pKey,
                     [&](SemanticSplitAssertAndInformLinks<MemoryGrdExpLinks>& pLinks)
    {
      if (!pLinksToAdd.assertions.empty())
      {
        auto& linksWithId = pLinks.assertions[pMemSentenceId];
        for (auto& currElt : pLinksToAdd.assertions)
          linksWithId.emplace_back(currElt);
      }
      if (!pLinksToAdd.informations.empty())
      {
        auto& linksWithId = pLinks.informations[pMemSentenceId];
        for (auto& currElt : pLinksToAdd.informations)
          linksWithId.emplace_back(currElt);
      }
    }, &_areLinksMoreRevelant);
  });
}

template<typename TVALUE, typename MAP_TYPE1, typename MAP_TYPE2, LinksAccessType access>
void _removeValueLinksFrom(SemanticValueLinks<TVALUE, MemoryGrdExpLinks, MAP_TYPE1, access>& pToFilter,
                           const SemanticValueLinks<TVALUE, MemoryGrdExpLinksForAMemSentence, MAP_TYPE2, LinksAccessType::ALL>& pToRemove,
                           intSemId pMemSentenceId)
{
  pToRemove.iterateOnKeys([&](const TVALUE& pKey)
  {
    pToFilter.removeValue(pKey,
                          [&](SemanticSplitAssertAndInformLinks<MemoryGrdExpLinks>& pLinks)
    {
      pLinks.assertions.erase(pMemSentenceId);
      pLinks.informations.erase(pMemSentenceId);
    }, &_areLinksMoreRevelant);
  });
}


template<typename TVALUE, typename MAP_TYPE>
void _addValueForAMemGrdExp(SemanticValueLinks<TVALUE, MemoryGrdExpLinksForAMemSentence, MAP_TYPE, LinksAccessType::ALL>& pToFill,
                            const TVALUE& pValue,
                            SemanticMemoryGrdExp& pMemGrdExp)
{
  pToFill.addValue(pValue, [&](SemanticSplitAssertAndInformLinks<MemoryGrdExpLinksForAMemSentence>& pLinks)
  {
    if (pMemGrdExp.getMemSentence().getContextAxiom().informationType == InformationType::ASSERTION)
      pLinks.assertions.emplace_back(pMemGrdExp);
    else
      pLinks.informations.emplace_back(pMemGrdExp);
  }, [&](const SemanticSplitAssertAndInformLinks<MemoryGrdExpLinksForAMemSentence>&,
         const SemanticSplitAssertAndInformLinks<MemoryGrdExpLinksForAMemSentence>&) { return false; } // we don't care about the main value here
  );
}


struct RegisterNameLinks : public SemExpUserInfosFiller::UserInfosContainer
{
  RegisterNameLinks(SemanticUserCenteredMemoryLinksForAMemSentence& pUserCenteredLinks,
                    SemanticMemoryGrdExp& pNewMemGrdExp)
    : _userCenteredLinks(pUserCenteredLinks),
      _newMemGrdExp(pNewMemGrdExp)
  {
  }

  void addNames(const std::string& pUserId,
                const std::vector<std::string>& pNames) override
  {
    UserNames userNames;
    userNames.names = pNames;
    _addValueForAMemGrdExp(_userCenteredLinks.userIdToUserCharacteristics[pUserId].nameLinks, userNames, _newMemGrdExp);
    for (const auto& currName : pNames)
      _addValueForAMemGrdExp(_userCenteredLinks.nameToUserIds[currName], pUserId, _newMemGrdExp);
  }

  void addGenders(const std::string& pUserId,
                  const std::set<SemanticGenderType>& pPossibleGenders) override
  {
    auto& genderRef = _userCenteredLinks.userIdToUserCharacteristics[pUserId].genderLinks;
    for (const auto& currGender : pPossibleGenders)
      _addValueForAMemGrdExp(genderRef, currGender, _newMemGrdExp);
  }

  void addEquivalentUserIds(const std::string& pSubjectUserId,
                            const std::string& pObjectUserId) override
  {
    _addValueForAMemGrdExp(_userCenteredLinks.userIdToUserCharacteristics[pSubjectUserId].equivalentUserIdLinks,
                           pObjectUserId, _newMemGrdExp);
    _addValueForAMemGrdExp(_userCenteredLinks.userIdToUserCharacteristics[pObjectUserId].equivalentUserIdLinks,
                           pSubjectUserId, _newMemGrdExp);
  }

  void addGrdExpToUserId(const GroundedExpression& pSubjectGrdExp,
                         const std::string& pObjectUserId) override
  {
    std::string strGrdExpId = Random::generateUuid();
    _addValueForAMemGrdExp(_userCenteredLinks.semExpToUserIds[&pSubjectGrdExp], strGrdExpId, _newMemGrdExp);
    _addValueForAMemGrdExp(_userCenteredLinks.userIdToUserCharacteristics[strGrdExpId].semExpLinks,
                           &pSubjectGrdExp, _newMemGrdExp);
    addEquivalentUserIds(strGrdExpId, pObjectUserId);
  }

private:
  SemanticUserCenteredMemoryLinksForAMemSentence& _userCenteredLinks;
  SemanticMemoryGrdExp& _newMemGrdExp;
};



template <typename MAP_KEY_MEMBLOCLINKS, typename MAP_KEY_LINKS>
void _fillMemBlocLinks
(MAP_KEY_MEMBLOCLINKS& pMemBlocLinks,
 MAP_KEY_LINKS& pLinks,
 intSemId pMemSentenceId)
{
  for (auto& currElt : pLinks)
  {
    currElt.second.shrink_to_fit();
    auto& vectToFill = pMemBlocLinks[currElt.first][pMemSentenceId];
    if (vectToFill.empty())
      vectToFill = currElt.second;
    else
      vectToFill.insert(vectToFill.end(), currElt.second.begin(), currElt.second.end());
  }
}

template <typename LINKS_TYPE, typename LINKS_TYPE_FOR_A_MEM_SENT>
void _fillMemBlocLinksForRadixMap
(mystd::radix_map_str<LINKS_TYPE>& pMemBlocLinks,
 mystd::radix_map_str<LINKS_TYPE_FOR_A_MEM_SENT>& pLinks,
 intSemId pMemSentenceId)
{
  pLinks.for_each([&](const std::string& pKey, LINKS_TYPE_FOR_A_MEM_SENT& pValue)
  {
    pValue.shrink_to_fit();
    auto& vectToFill = pMemBlocLinks[pKey][pMemSentenceId];
    if (vectToFill.empty())
      vectToFill = pValue;
    else
      vectToFill.insert(vectToFill.end(), pValue.begin(), pValue.end());
  });
}


template <typename LINKSTYPE, typename DURATIONTYPE>
typename mystd::radix_map_struct<SemanticDuration, LINKSTYPE>::iterator
_insertTimeElt(
    mystd::radix_map_struct<SemanticDuration, LINKSTYPE>& pTimeToSemExps,
    const DURATIONTYPE& pDuration)
{
  auto insertRes = pTimeToSemExps.insert(pDuration);
  auto& itInMemory = insertRes.first;
  assert(itInMemory != pTimeToSemExps.end());
  if (!insertRes.second) // if it already existed we return it
    return itInMemory;

  // init with previous element value
  if (itInMemory != pTimeToSemExps.begin())
  {
    auto itBefore = itInMemory;
    --itBefore;
    itInMemory->second = itBefore->second;
  }
  return itInMemory;
}


void _addDurationSlotLinks(
    SemanticLinksToGrdExpsForAMemSentence& pLinksToGrdExps,
    const SemanticDuration pBeginDuration,
    const SemanticDuration pEndDuration,
    SemanticMemoryGrdExp& pNewMemGrdExp)
{
  _insertTimeElt(pLinksToGrdExps.timeToSemExps, pEndDuration);

  // add begin of the time slot
  auto itInMemory = _insertTimeElt(pLinksToGrdExps.timeToSemExps, pBeginDuration);
  itInMemory->second.push_back(pNewMemGrdExp);

  // update the links that are inside the new time slot
  ++itInMemory;
  if (itInMemory != pLinksToGrdExps.timeToSemExps.end())
  {
    //while (itInMemory->first < pEndDuration)
    auto endRadixMapStr = pEndDuration.toRadixMapStr();
    while (itInMemory->first < endRadixMapStr)
    {
      itInMemory->second.push_back(pNewMemGrdExp);
      ++itInMemory;
      if (itInMemory == pLinksToGrdExps.timeToSemExps.end())
        break;
    }
  }
}

void _addTimeLinksFromRelativeDurationGrounding
(SemanticLinksToGrdExpsForAMemSentence& pLinksToGrdExps,
 const SemanticDuration& pBeginDuration,
 SemanticMemoryGrdExp& pNewMemGrdExpPtr)
{
  // add end of the time slot
  SemanticDuration endDuration = pBeginDuration;
  endDuration.add(SemanticTimeUnity::LESS_THAN_A_MILLISECOND, 1);
  // fill the links
  _addDurationSlotLinks(pLinksToGrdExps, pBeginDuration, endDuration, pNewMemGrdExpPtr);
}


void _addTimeLinksFromGrounding(
    SemanticLinksToGrdExpsForAMemSentence& pLinksToGrdExps,
    const SemanticTimeGrounding& pTimeGrounding,
    SemanticMemoryGrdExp& pNewMemGrdExp)
{
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
  if (pTimeGrounding.date.month)
  {
    auto ponthCpt = monthConceptStr_fromMonthId(*pTimeGrounding.date.month);
    pLinksToGrdExps.conceptsToSemExps[ponthCpt].emplace_back(pNewMemGrdExp);
  }

  // add year concept
  auto yearCpt = pTimeGrounding.date.getYearConcept();
  if (!yearCpt.empty())
    pLinksToGrdExps.conceptsToSemExps[yearCpt].emplace_back(pNewMemGrdExp);
}


void _addLinksFrom(SemanticLinksToGrdExpsTemplate<MemoryGrdExpLinks>& pLinks,
                   SemanticLinksToGrdExpsTemplate<MemoryGrdExpLinksForAMemSentence>& pLinksFromMemSentence,
                   intSemId pMemSentenceId)
{
   _fillMemBlocLinksForRadixMap(pLinks.conceptsToSemExps, pLinksFromMemSentence.conceptsToSemExps, pMemSentenceId);

  for (auto& currElt : pLinksFromMemSentence.meaningsToSemExps)
    _fillMemBlocLinks(pLinks.meaningsToSemExps[currElt.first], currElt.second, pMemSentenceId);

  _fillMemBlocLinks(pLinks.everythingOrNoEntityTypeToSemExps, pLinksFromMemSentence.everythingOrNoEntityTypeToSemExps, pMemSentenceId);
  _fillMemBlocLinks(pLinks.genGroundingTypeToSemExps, pLinksFromMemSentence.genGroundingTypeToSemExps, pMemSentenceId);

  if (!pLinksFromMemSentence.timeToSemExps.empty())
  {
    // insert new time elts
    auto itRefTimeElt = pLinksFromMemSentence.timeToSemExps.end();
    while (itRefTimeElt != pLinksFromMemSentence.timeToSemExps.begin())
    {
      --itRefTimeElt;
      _insertTimeElt(pLinks.timeToSemExps, itRefTimeElt->first);
    }

    auto itInMemory = pLinks.timeToSemExps.find(itRefTimeElt->first);
    assert(itInMemory != pLinks.timeToSemExps.end());
    while (true)
    {
      if (!itRefTimeElt->second.empty())
      {
        // fill the time modifications
        auto& linksToFill = itInMemory->second[pMemSentenceId];
        linksToFill.insert(linksToFill.end(), itRefTimeElt->second.begin(), itRefTimeElt->second.end());

        auto itRefNext = itRefTimeElt;
        ++itRefNext;
        if (itRefNext == pLinksFromMemSentence.timeToSemExps.end())
        {
          assert(false);
          break;
        }

        // update the times between 2 time modifications
        ++itInMemory;
        while (itInMemory->first < itRefNext->first)
        {
          auto& currLinksToFill = itInMemory->second[pMemSentenceId];
          currLinksToFill.insert(currLinksToFill.end(), itRefTimeElt->second.begin(), itRefTimeElt->second.end());
          ++itInMemory;
          assert(itInMemory != pLinks.timeToSemExps.end());
        }
        itRefTimeElt = itRefNext;
        assert(itRefTimeElt->first == itInMemory->first);
        continue;
      }

      // jump to next time slot
      ++itRefTimeElt;
      if (itRefTimeElt == pLinksFromMemSentence.timeToSemExps.end())
        break;
      itInMemory = pLinks.timeToSemExps.find(itRefTimeElt->first);
      assert(itInMemory != pLinks.timeToSemExps.end());
    }
  }

  _fillMemBlocLinks(pLinks.relLocationToSemExps, pLinksFromMemSentence.relLocationToSemExps, pMemSentenceId);
  _fillMemBlocLinks(pLinks.relTimeToSemExps, pLinksFromMemSentence.relTimeToSemExps, pMemSentenceId);
  _fillMemBlocLinks(pLinks.grdTypeToSemExps, pLinksFromMemSentence.grdTypeToSemExps, pMemSentenceId);
  _fillMemBlocLinksForRadixMap(pLinks.userIdToSemExps, pLinksFromMemSentence.userIdToSemExps, pMemSentenceId);
  for (auto& currElt : pLinksFromMemSentence.textToSemExps)
    _fillMemBlocLinksForRadixMap(pLinks.textToSemExps[currElt.first], currElt.second, pMemSentenceId);
  _fillMemBlocLinks(pLinks.languageToSemExps, pLinksFromMemSentence.languageToSemExps, pMemSentenceId);
  _fillMemBlocLinksForRadixMap(pLinks.resourceToSemExps, pLinksFromMemSentence.resourceToSemExps, pMemSentenceId);
}


template <typename MAP_KEY_MEMBLOCLINKS, typename MAP_KEY_LINKS>
void _removeMemoryLinks
(MAP_KEY_MEMBLOCLINKS& pToFilter,
 const MAP_KEY_LINKS& pRef,
 intSemId pMemSentenceId)
{
  for (const auto& currElt : pRef)
  {
    auto subItToFilter = pToFilter.find(currElt.first);
    subItToFilter->second.erase(pMemSentenceId);
    if (subItToFilter->second.empty())
      pToFilter.erase(subItToFilter);
  }
}


template <typename LINKS_TYPE, typename LINKS_TYPE_FOR_A_MEM_SENT>
void _removeMemoryLinksForRadixMap
(mystd::radix_map_str<LINKS_TYPE>& pToFilter,
 const mystd::radix_map_str<LINKS_TYPE_FOR_A_MEM_SENT>& pRef,
 intSemId pMemSentenceId)
{
  pRef.for_each([&](const std::string& pKey, const LINKS_TYPE_FOR_A_MEM_SENT&)
  {
    auto subItToFilter = pToFilter.find(pKey);
    assert(subItToFilter != pToFilter.end());
    subItToFilter->second.erase(pMemSentenceId);
    if (subItToFilter->second.empty())
      pToFilter.erase(subItToFilter);
  });
}

void _removeGrdExpsLinks(std::map<intSemId, MemoryGrdExpLinksForAMemSentence>& pToFilter,
                         const MemoryGrdExpLinksForAMemSentence& pLinksFromMemSentence,
                         intSemId pMemSentenceId)
{
  if (pLinksFromMemSentence.empty())
    return;
  auto itToFilter = pToFilter.find(pMemSentenceId);
  if (itToFilter != pToFilter.end())
  {
    auto itLinks = itToFilter->second.begin();
    for (auto& currLink : pLinksFromMemSentence)
    {
      while (itLinks != itToFilter->second.end())
      {
        if (&*itLinks == &currLink)
        {
          itLinks = itToFilter->second.erase(itLinks);
          break;
        }
        else
          ++itLinks;
      }
    }

    if (itToFilter->second.empty())
      pToFilter.erase(itToFilter);
  }
  else
  {
    assert(false);
  }
}


void _removeLinksFrom(SemanticLinksToGrdExpsTemplate<MemoryGrdExpLinks>& pToFilter,
                      const SemanticLinksToGrdExpsTemplate<MemoryGrdExpLinksForAMemSentence>& pLinksFromMemSentence,
                      intSemId pMemSentenceId)
{
  _removeMemoryLinksForRadixMap(pToFilter.conceptsToSemExps,
                                pLinksFromMemSentence.conceptsToSemExps, pMemSentenceId);

  for (const auto& currElt : pLinksFromMemSentence.meaningsToSemExps)
  {
    auto subItToFilter = pToFilter.meaningsToSemExps.find(currElt.first);
    _removeMemoryLinks(subItToFilter->second, currElt.second, pMemSentenceId);
    if (subItToFilter->second.empty())
      pToFilter.meaningsToSemExps.erase(subItToFilter);
  }

  _removeMemoryLinks(pToFilter.everythingOrNoEntityTypeToSemExps,
                     pLinksFromMemSentence.everythingOrNoEntityTypeToSemExps, pMemSentenceId);
  _removeMemoryLinks(pToFilter.genGroundingTypeToSemExps,
                     pLinksFromMemSentence.genGroundingTypeToSemExps, pMemSentenceId);

  if (!pLinksFromMemSentence.timeToSemExps.empty())
  {
    auto itTimeElt = pLinksFromMemSentence.timeToSemExps.begin();
    while (true)
    {
      auto subItToFilter = pToFilter.timeToSemExps.find(itTimeElt->first);
      assert(subItToFilter != pToFilter.timeToSemExps.end());
      _removeGrdExpsLinks(subItToFilter->second, itTimeElt->second, pMemSentenceId);

      if (subItToFilter != pToFilter.timeToSemExps.begin())
      {
        auto itPrev = subItToFilter;
        --itPrev;
        if (subItToFilter->second == itPrev->second)
          subItToFilter = pToFilter.timeToSemExps.erase(subItToFilter);
        else
          ++subItToFilter;
      }
      else if (subItToFilter->second.empty())
      {
        subItToFilter = pToFilter.timeToSemExps.erase(subItToFilter);
      }
      else
      {
        ++subItToFilter;
      }

      auto itTimeNextElt = itTimeElt;
      ++itTimeNextElt;
      if (itTimeNextElt == pLinksFromMemSentence.timeToSemExps.end())
        break;
      while (subItToFilter != pToFilter.timeToSemExps.end() &&
             subItToFilter->first < itTimeNextElt->first)
      {
        _removeGrdExpsLinks(subItToFilter->second, itTimeElt->second, pMemSentenceId);
        ++subItToFilter;
        assert(subItToFilter != pToFilter.timeToSemExps.end());
      }
      itTimeElt = itTimeNextElt;
    }
  }

  _removeMemoryLinks(pToFilter.relLocationToSemExps,
                     pLinksFromMemSentence.relLocationToSemExps, pMemSentenceId);
  _removeMemoryLinks(pToFilter.relTimeToSemExps,
                     pLinksFromMemSentence.relTimeToSemExps, pMemSentenceId);
  _removeMemoryLinks(pToFilter.grdTypeToSemExps,
                     pLinksFromMemSentence.grdTypeToSemExps, pMemSentenceId);
  _removeMemoryLinksForRadixMap(pToFilter.userIdToSemExps,
                                pLinksFromMemSentence.userIdToSemExps, pMemSentenceId);

  for (const auto& currElt : pLinksFromMemSentence.textToSemExps)
  {
    auto subItToFilter = pToFilter.textToSemExps.find(currElt.first);
    _removeMemoryLinksForRadixMap(subItToFilter->second, currElt.second, pMemSentenceId);
    if (subItToFilter->second.empty())
      pToFilter.textToSemExps.erase(subItToFilter);
  }

  _removeMemoryLinks(pToFilter.languageToSemExps,
                     pLinksFromMemSentence.languageToSemExps, pMemSentenceId);
  _removeMemoryLinksForRadixMap(pToFilter.resourceToSemExps,
                                pLinksFromMemSentence.resourceToSemExps, pMemSentenceId);
}

enum class LinkedState
{
  LINKED,
  LINKDISABLED,
  NOTLINKED
};

}


struct SemanticMemorySentencePrivate
{
  SemanticMemorySentencePrivate(SemanticMemorySentence& pMemSent,
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
  void disableUserCenteredLinks();

  void writeInBinary(binarymasks::Ptr& pPtr,
                     MemGrdExpPtrOffsets& pMemGrdExpPtrs,
                     const semexpsaver::SemExpPtrOffsets& pSemExpPtrOffsets) const;
  void filterMemLinks(SemanticMemoryLinks& pSemanticMemoryLinksToFilter) const;

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

private:
  SemanticMemorySentence& _memSent;
  std::list<SemanticMemoryGrdExp> _childGrdExpMemories;
  SemanticMemoryLinksForAMemSentence _links;


  void _gatherAllTheLinksOfSemExp(const SemanticExpression& pSemExp,
                                  const linguistics::LinguisticDatabase& pLingDb);
  void _gatherAllTheLinks(const GroundedExpression& pGrdExp,
                          const linguistics::LinguisticDatabase& pLingDb);

  void _linkStatementGrdExp(const SemanticStatementGrounding& pStatGrd,
                            const linguistics::LinguisticDatabase& pLingDb);

  void _linkConceptsThatBeginWith(const SemanticExpression& pSemExpression,
                                  SemanticRequestType pFromRequest,
                                  const std::string& pBeginOfAConcept);

  bool _linkChildSemExp(const SemanticExpression& pSemExpression,
                        SemanticRequestType pFromRequest,
                        const linguistics::LinguisticDatabase& pLingDb);

  void _fillUserCenteredLinks(SemanticMemoryGrdExp& pNewMemGrdExp,
                              const GroundedExpression& pGrdExp);

  /**
   * @brief Add a gounded expression to this MemorySentence object
   * @param pGrdExp The grounded expression to add.
   * @param pGrounding The grounding of the grounded expression.
   * @param pRequType The type of question if it's a question (SemanticRequestType::NOTHING otherwise)
   * @param pLingDb The linguistic dictionary.
   * @param pLinkNonSpecificStuffs If we shall add the grounding that is not linked to a word,
   * a concept and that his reference is indefinite.
   */
  bool _linkGrdExp
  (const GroundedExpression& pGrdExp,
   const SemanticGrounding& pGrounding,
   SemanticRequestType pRequType,
   const linguistics::LinguisticDatabase& pLingDb,
   bool pLinkNonSpecificStuffs);

  void _linkLingMeaning
  (SemanticMemoryGrdExp& pNewMemGrdExp,
   SemanticRequestType pRequType,
   const SemanticWord& pInWord,
   const StaticLinguisticMeaning& pInLingMeaning);

  void _mergeRequToGrdExps(SemanticMemoryLinks& pToComplete,
                           SemanticMemoryLinksForAMemSentence& pRef);

  void _addGenderFromGrdExp(SemanticMemoryGrdExp& pNewMemGrdExp,
                            const GroundedExpression& pGrdExp);
  void _addGenderFromSemExp(SemanticMemoryGrdExp& pNewMemGrdExp,
                            const UniqueSemanticExpression& pSemExp);

  void _tryToAddAgentGenderFromAStructureEqualityStatement(SemanticMemoryGrdExp& pNewMemGrdExp,
                                                           const GroundedExpression& pGrdExp);
};


SemanticMemorySentencePrivate::SemanticMemorySentencePrivate
(SemanticMemorySentence& pMemSent,
 const std::map<GrammaticalType, const SemanticExpression*>& pAnnotations,
 const linguistics::LinguisticDatabase& pLingDb)
  : tense(SemanticVerbTense::UNKNOWN),
    verbGoal(VerbGoalEnum::NOTIFICATION),
    userCenteredLinks(),
    isLinkedInRequToAnswers(LinkedState::NOTLINKED),
    isLinkedInRequToCondition(LinkedState::NOTLINKED),
    isLinkedInRequToSentWithAction(LinkedState::NOTLINKED),
    isLinkedInRequToInfAction(LinkedState::NOTLINKED),
    isLinkedInRequToActionTrigger(LinkedState::NOTLINKED),
    isLinkedInRequToQuestionTrigger(LinkedState::NOTLINKED),
    isLinkedInRequToAffirmationTrigger(LinkedState::NOTLINKED),
    isLinkedInRequToNominalGroupsTrigger(LinkedState::NOTLINKED),
    _memSent(pMemSent),
    _childGrdExpMemories(),
    _links()
{
  for (const auto& currAnnotation : pAnnotations)
  {
    auto itChild = _memSent.grdExp.children.find(currAnnotation.first);
    if (itChild == _memSent.grdExp.children.end() ||
        SemExpGetter::isACoreference(*itChild->second, CoreferenceDirectionEnum::PARENT, false))
      _memSent._annotations.emplace(currAnnotation.first, currAnnotation.second);
  }

  if (_memSent.gatherAllTheLinks)
  {
    _gatherAllTheLinks(_memSent.grdExp, pLingDb);
    linkToNominalTrigger();
    return;
  }

  const SemanticGrounding& grd = _memSent.grdExp.grounding();
  const SemanticStatementGrounding* statGrdPtr = _memSent.grdExp->getStatementGroundingPtr();
  if (statGrdPtr != nullptr)
  {
    const auto& statGrd = *statGrdPtr;
    tense = statGrd.verbTense;
    verbGoal = statGrd.verbGoal;
    _linkStatementGrdExp(statGrd, pLingDb);
  }
  else
  {
    _memSent._isANoun = true;
    _linkGrdExp(_memSent.grdExp, grd, SemanticRequestType::NOTHING, pLingDb, true);
    if (_memSent._isEnabled && !_memSent._isAConditionToSatisfy)
      enableUserCenteredLinks();
    if (_memSent._contextAxiom.getSemExpWrappedForMemory().outputToAnswerIfTriggerHasMatched)
    {
      if (_memSent._isEnabled)
        linkToNominalTrigger();
      else
        isLinkedInRequToNominalGroupsTrigger = LinkedState::LINKDISABLED;
    }
    else if (_memSent._contextAxiom.semTracker ||
             _memSent._contextAxiom.semExpToDo != nullptr)
    {
      _memSent._isAConditionToSatisfy = true;
      if (_memSent._isEnabled)
        linkToSentWithAction();
      else
        isLinkedInRequToSentWithAction = LinkedState::LINKDISABLED;
    }
  }
}


void SemanticMemorySentencePrivate::enableUserCenteredLinks()
{
  auto& sharedUserCenteredLinks = _memSent._contextAxiom.getSemExpWrappedForMemory().getParentMemBloc()._impl->userCenteredLinks;
  userCenteredLinks.userIdToUserCharacteristics.for_each([&](const std::string& pKey, UserCharacteristics<MemoryGrdExpLinksForAMemSentence, LinksAccessType::ALL>& pUserCharacteristics)
  {
    auto& sharedUserCharacteristics = sharedUserCenteredLinks.userIdToUserCharacteristics[pKey];
    _addValueLinksFrom(sharedUserCharacteristics.genderLinks, pUserCharacteristics.genderLinks, _memSent.id);
    _addValueLinksFrom(sharedUserCharacteristics.nameLinks, pUserCharacteristics.nameLinks, _memSent.id);
    _addValueLinksFrom(sharedUserCharacteristics.equivalentUserIdLinks, pUserCharacteristics.equivalentUserIdLinks, _memSent.id);
    _addValueLinksFrom(sharedUserCharacteristics.semExpLinks, pUserCharacteristics.semExpLinks, _memSent.id);
  });
  userCenteredLinks.nameToUserIds.for_each([&](const std::string& pKey, SemanticStringLinks<MemoryGrdExpLinksForAMemSentence, LinksAccessType::ALL>& pLinks)
  { _addValueLinksFrom(sharedUserCenteredLinks.nameToUserIds[pKey], pLinks, _memSent.id); });
  for (auto& currElt : userCenteredLinks.semExpToUserIds)
    _addValueLinksFrom(sharedUserCenteredLinks.semExpToUserIds[currElt.first], currElt.second, _memSent.id);
}


void SemanticMemorySentencePrivate::linkToRequToAnswers()
{
  auto& memBlocPrivate = *_memSent._contextAxiom.getSemExpWrappedForMemory().getParentMemBloc()._impl;
  _mergeRequToGrdExps(memBlocPrivate.answersLinks.getLinks(tense, verbGoal), _links);
  isLinkedInRequToAnswers = LinkedState::LINKED;
}

void SemanticMemorySentencePrivate::linkToConditionToInformation()
{
  auto& memBlocPrivate = *_memSent._contextAxiom.getSemExpWrappedForMemory().getParentMemBloc()._impl;
  _mergeRequToGrdExps(memBlocPrivate.conditionToInformationLinks.getLinks(tense, verbGoal), _links);
  isLinkedInRequToCondition = LinkedState::LINKED;
}

void SemanticMemorySentencePrivate::linkToSentWithAction()
{
  auto& memBlocPrivate = *_memSent._contextAxiom.getSemExpWrappedForMemory().getParentMemBloc()._impl;
  _mergeRequToGrdExps(memBlocPrivate.sentWithActionLinks.getLinks(tense, verbGoal), _links);
  memBlocPrivate.addConditionToAnAction(_memSent, _links);
  isLinkedInRequToSentWithAction = LinkedState::LINKED;
}

void SemanticMemorySentencePrivate::linkToSentWithInfAction()
{
  auto& memBlocPrivate = *_memSent._contextAxiom.getSemExpWrappedForMemory().getParentMemBloc()._impl;
  _mergeRequToGrdExps(memBlocPrivate.sentWithInfActionLinks, _links);
  memBlocPrivate.addInfAction(_memSent);
  isLinkedInRequToInfAction = LinkedState::LINKED;
}

void SemanticMemorySentencePrivate::linkToActionTrigger()
{
  auto& memBlocPrivate = *_memSent._contextAxiom.getSemExpWrappedForMemory().getParentMemBloc()._impl;
  _mergeRequToGrdExps(memBlocPrivate.ensureSentenceTriggersLinks(SemanticExpressionCategory::COMMAND,
                                                                 _memSent._contextAxiom.triggerAxiomId).getLinks(tense, verbGoal), _links);
  isLinkedInRequToActionTrigger = LinkedState::LINKED;
}

void SemanticMemorySentencePrivate::linkToQuestionTrigger()
{
  auto& memBlocPrivate = *_memSent._contextAxiom.getSemExpWrappedForMemory().getParentMemBloc()._impl;
  _mergeRequToGrdExps(memBlocPrivate.ensureSentenceTriggersLinks(SemanticExpressionCategory::QUESTION,
                                                                 _memSent._contextAxiom.triggerAxiomId).getLinks(tense, verbGoal), _links);
  isLinkedInRequToQuestionTrigger = LinkedState::LINKED;
}

void SemanticMemorySentencePrivate::linkToAffirmationTrigger()
{
  auto& memBlocPrivate = *_memSent._contextAxiom.getSemExpWrappedForMemory().getParentMemBloc()._impl;
  _mergeRequToGrdExps(memBlocPrivate.ensureSentenceTriggersLinks(SemanticExpressionCategory::AFFIRMATION,
                                                                 _memSent._contextAxiom.triggerAxiomId).getLinks(tense, verbGoal), _links);
  isLinkedInRequToAffirmationTrigger = LinkedState::LINKED;
}

void SemanticMemorySentencePrivate::linkToNominalTrigger()
{
  auto& memBlocPrivate = *_memSent._contextAxiom.getSemExpWrappedForMemory().getParentMemBloc()._impl;
  _mergeRequToGrdExps(memBlocPrivate.ensureNominalGroupsTriggersLinks(_memSent._contextAxiom.triggerAxiomId), _links);
  isLinkedInRequToNominalGroupsTrigger = LinkedState::LINKED;
}


void SemanticMemorySentencePrivate::disableUserCenteredLinks()
{
  auto& sharedUserCenteredLinks = _memSent._contextAxiom.getSemExpWrappedForMemory().getParentMemBloc()._impl->userCenteredLinks;
  for (const auto& currElt : userCenteredLinks.userIdToUserCharacteristics)
  {
    auto subItToFilter = sharedUserCenteredLinks.userIdToUserCharacteristics.find(currElt.first);
    assert(subItToFilter != sharedUserCenteredLinks.userIdToUserCharacteristics.end());
    auto& userCharacteristics = subItToFilter->second;
    _removeValueLinksFrom(userCharacteristics.genderLinks, currElt.second.genderLinks, _memSent.id);
    _removeValueLinksFrom(userCharacteristics.nameLinks, currElt.second.nameLinks, _memSent.id);
    _removeValueLinksFrom(userCharacteristics.equivalentUserIdLinks, currElt.second.equivalentUserIdLinks, _memSent.id);
    _removeValueLinksFrom(userCharacteristics.semExpLinks, currElt.second.semExpLinks, _memSent.id);
    if (userCharacteristics.empty())
      sharedUserCenteredLinks.userIdToUserCharacteristics.erase(subItToFilter);
  }
  for (const auto& currElt : userCenteredLinks.nameToUserIds)
  {
    auto subItToFilter = sharedUserCenteredLinks.nameToUserIds.find(currElt.first);
    _removeValueLinksFrom(subItToFilter->second, currElt.second, _memSent.id);
    if (subItToFilter->second.empty())
      sharedUserCenteredLinks.nameToUserIds.erase(subItToFilter);
  }
  for (const auto& currElt : userCenteredLinks.semExpToUserIds)
  {
    auto subItToFilter = sharedUserCenteredLinks.semExpToUserIds.find(currElt.first);
    _removeValueLinksFrom(subItToFilter->second, currElt.second, _memSent.id);
    if (subItToFilter->second.empty())
      sharedUserCenteredLinks.semExpToUserIds.erase(subItToFilter);
  }
}


void SemanticMemorySentencePrivate::writeInBinary(
    binarymasks::Ptr& pPtr,
    MemGrdExpPtrOffsets& pMemGrdExpPtrs,
    const semexpsaver::SemExpPtrOffsets& pSemExpPtrOffsets) const
{
  unsigned char i = 0;
  auto* nbOfEltsPtr = pPtr.puchar++;
  for (const auto& currChild : _childGrdExpMemories)
  {
    pMemGrdExpPtrs.addMemGrdExp(currChild, pPtr.puchar);
    binarysaver::writeInThreeBytes(pPtr.pchar, pSemExpPtrOffsets.grdExpToOffset(currChild.grdExp, pPtr.puchar));
    pPtr.pchar += 3;
    *(pPtr.puchar++) = i++;
  }
  assert(i < 255);
  *nbOfEltsPtr = i;
}


void SemanticMemorySentencePrivate::filterMemLinks(SemanticMemoryLinks& pSemanticMemoryLinksToFilter) const
{
  for (const auto& currReq : _links.reqToGrdExps)
  {
    auto it = pSemanticMemoryLinksToFilter.reqToGrdExps.find(currReq.first);
    assert(it != pSemanticMemoryLinksToFilter.reqToGrdExps.end());
    _removeLinksFrom(it->second, currReq.second, _memSent.id);
    if (it->second.empty())
      pSemanticMemoryLinksToFilter.reqToGrdExps.erase(it);
  }
}


void SemanticMemorySentencePrivate::_gatherAllTheLinksOfSemExp(const SemanticExpression& pSemExp,
                                                               const linguistics::LinguisticDatabase& pLingDb)
{
  const auto* grdExpPtr = pSemExp.getGrdExpPtr_SkipWrapperPtrs();
  if (grdExpPtr != nullptr)
    _gatherAllTheLinks(*grdExpPtr, pLingDb);
  else
  {
    const auto* listExpPtr = pSemExp.getListExpPtr_SkipWrapperPtrs();
    if (listExpPtr != nullptr)
      for (const auto& currElt : listExpPtr->elts)
        _gatherAllTheLinksOfSemExp(*currElt, pLingDb);
  }
}


void SemanticMemorySentencePrivate::_gatherAllTheLinks(const GroundedExpression& pGrdExp,
                                                       const linguistics::LinguisticDatabase& pLingDb)
{
  const SemanticGrounding& grd = pGrdExp.grounding();
  _linkGrdExp(pGrdExp, grd, SemanticRequestType::NOTHING, pLingDb, false);
  for (const auto& currChild : pGrdExp.children)
    _gatherAllTheLinksOfSemExp(*currChild.second, pLingDb);
}


void SemanticMemorySentencePrivate::_linkStatementGrdExp(const SemanticStatementGrounding& pStatGrd,
                                                         const linguistics::LinguisticDatabase& pLingDb)
{
  bool skipLinkage = false;
  skipLinkage = !_linkGrdExp(_memSent.grdExp, pStatGrd, SemanticRequestType::ACTION, pLingDb, true);

  // link sub expressions
  if (!skipLinkage)
  {
    for (const auto& currChild : _memSent.grdExp.children)
    {
      SemanticRequestType newRequ =
          SemExpGetter::convertSemGramToRequestType(currChild.first);
      if (newRequ != SemanticRequestType::NOTHING &&
          !_linkChildSemExp(*currChild.second, newRequ, pLingDb))
      {
        skipLinkage = true;
        break;
      }
    }

    if (!skipLinkage)
    {
      for (const auto& currAnnotation : _memSent._annotations)
      {
        SemanticRequestType newRequ =
            SemExpGetter::convertSemGramToRequestType(currAnnotation.first);
        if (newRequ != SemanticRequestType::NOTHING &&
            !_linkChildSemExp(*currAnnotation.second, newRequ, pLingDb))
          break;
      }
    }
  }


  if (_memSent._isEnabled && !_memSent._isAConditionToSatisfy)
    enableUserCenteredLinks();

  if (_memSent._contextAxiom.getSemExpWrappedForMemory().outputToAnswerIfTriggerHasMatched)
  {
    if (pStatGrd.requests.empty())
    {
      if (_memSent._isEnabled)
        linkToAffirmationTrigger();
      else
        isLinkedInRequToAffirmationTrigger = LinkedState::LINKDISABLED;
    }
    else if (pStatGrd.requests.first() == SemanticRequestType::ACTION)
    {
      if (_memSent._isEnabled)
        linkToActionTrigger();
      else
        isLinkedInRequToActionTrigger = LinkedState::LINKDISABLED;
    }
    else
    {
      if (_memSent._isEnabled)
        linkToQuestionTrigger();
      else
        isLinkedInRequToQuestionTrigger = LinkedState::LINKDISABLED;
    }
  }
  else if (_memSent._contextAxiom.infCommandToDo != nullptr)
  {
    if (_memSent._isEnabled)
      linkToSentWithInfAction();
    else
      isLinkedInRequToInfAction = LinkedState::LINKDISABLED;
  }
  else if (_memSent._contextAxiom.semTracker ||
           _memSent._contextAxiom.semExpToDo != nullptr)
  {
    _memSent._isAConditionToSatisfy = true;
    if (_memSent._isEnabled)
      linkToSentWithAction();
    else
      isLinkedInRequToSentWithAction = LinkedState::LINKDISABLED;
  }
  else if (_memSent._isAConditionToSatisfy)
  {
    if (_memSent._isEnabled)
      linkToConditionToInformation();
    else
      isLinkedInRequToCondition = LinkedState::LINKDISABLED;
  }
  else if (pStatGrd.requests.empty())
  {
    if (_memSent._isEnabled)
      linkToRequToAnswers();
    else
      isLinkedInRequToAnswers = LinkedState::LINKDISABLED;
  }
}


void SemanticMemorySentencePrivate::_linkConceptsThatBeginWith
(const SemanticExpression& pSemExpression,
 SemanticRequestType pFromRequest,
 const std::string& pBeginOfAConcept)
{
  std::list<const GroundedExpression*> grdExpPtr;
  pSemExpression.getGrdExpPtrs_SkipWrapperLists(grdExpPtr);
  for (const auto& currGrdExp : grdExpPtr)
  {
    for (const auto& currCpt : currGrdExp->grounding().concepts)
    {
      if (ConceptSet::doesConceptBeginWith(currCpt.first, pBeginOfAConcept))
      {
        _childGrdExpMemories.emplace_back(_memSent, *currGrdExp);
        SemanticMemoryGrdExp& newMemGrdExp = _childGrdExpMemories.back();
        _links.reqToGrdExps[pFromRequest].conceptsToSemExps[currCpt.first].emplace_back(newMemGrdExp);
        return;
      }
    }
  }
}


bool SemanticMemorySentencePrivate::_linkChildSemExp
(const SemanticExpression& pSemExpression,
 SemanticRequestType pFromRequest,
 const linguistics::LinguisticDatabase& pLingDb)
{
  const GroundedExpression* childGrdExp = pSemExpression.getGrdExpPtr_SkipWrapperPtrs();
  if (childGrdExp != nullptr)
  {
    if (childGrdExp == &_memSent.grdExp)
      return true;

    const auto& childGrounding = childGrdExp->grounding();

    if (childGrounding.type == SemanticGroudingType::AGENT)
    {
      auto* originalIntPtr = pSemExpression.getIntExpPtr();
      if (originalIntPtr != nullptr)
      {
        auto* originalGrdExpPtr = originalIntPtr->originalExp->getGrdExpPtr_SkipWrapperPtrs();
        if (originalGrdExpPtr != nullptr &&
            originalGrdExpPtr->grounding().type == SemanticGroudingType::AGENT)
        {
          _childGrdExpMemories.emplace_back(_memSent, *originalGrdExpPtr);
          SemanticMemoryGrdExp& newMemGrdExp = _childGrdExpMemories.back();
          _fillUserCenteredLinks(newMemGrdExp, *originalGrdExpPtr);
        }
      }
    }

    if (!_linkGrdExp(*childGrdExp, childGrounding, pFromRequest, pLingDb, true))
      return false;

    auto itEquChild = childGrdExp->children.find(GrammaticalType::SPECIFIER);
    if (itEquChild != childGrdExp->children.end())
    {
      if (semanticGroudingsType_isRelativeType(childGrounding.type))
      {
        if (!_linkChildSemExp(*itEquChild->second, pFromRequest, pLingDb))
          return false;
      }
      else
      {
        _linkConceptsThatBeginWith(*itEquChild->second, pFromRequest, "nationality_");
      }
    }

    auto itSubCptChild = childGrdExp->children.find(GrammaticalType::SUB_CONCEPT);
    if (itSubCptChild != childGrdExp->children.end() &&
        !_linkChildSemExp(*itSubCptChild->second, pFromRequest, pLingDb))
      return false;
    return true;
  }

  const ListExpression* chilListExp = pSemExpression.getListExpPtr();
  if (chilListExp != nullptr)
  {
    for (const auto& currElt : chilListExp->elts)
      if (!_linkChildSemExp(*currElt, pFromRequest, pLingDb))
        return false;
  }
  return true;
}


void SemanticMemorySentencePrivate::_fillUserCenteredLinks(SemanticMemoryGrdExp& pNewMemGrdExp,
                                                           const GroundedExpression& pGrdExp)
{
  _addGenderFromGrdExp(pNewMemGrdExp, pGrdExp);
  RegisterNameLinks registerNameLinks(userCenteredLinks, pNewMemGrdExp);
  SemExpUserInfosFiller::tryToAddUserInfosWithTense(registerNameLinks, pGrdExp, tense);
}


bool SemanticMemorySentencePrivate::_linkGrdExp
(const GroundedExpression& pGrdExp,
 const SemanticGrounding& pGrounding,
 SemanticRequestType pRequType,
 const linguistics::LinguisticDatabase& pLingDb,
 bool pLinkNonSpecificStuffs)
{
  if (pGrounding.type == SemanticGroudingType::GENERIC &&
      pGrounding.getGenericGrounding().coreference &&
      pGrounding.getGenericGrounding().coreference->getDirection() == CoreferenceDirectionEnum::PARENT)
    return true;
  _childGrdExpMemories.emplace_back(_memSent, pGrdExp);
  SemanticMemoryGrdExp& newMemGrdExp = _childGrdExpMemories.back();

  // add links of user centered information
  if (&pGrdExp == &_memSent.grdExp)
    _fillUserCenteredLinks(newMemGrdExp, pGrdExp);

  // link the concepts
  if (pGrounding.type != SemanticGroudingType::TIME)
    for (const auto& currCpt : pGrounding.concepts)
      _links.reqToGrdExps[pRequType].conceptsToSemExps[currCpt.first].emplace_back(newMemGrdExp);

  switch (pGrounding.type)
  {
  case SemanticGroudingType::GENERIC:
  {
    const SemanticGenericGrounding& genGrounding = pGrounding.getGenericGrounding();
    if (!ConceptSet::haveAConceptNotAny(genGrounding.concepts))
    {
      StaticLinguisticMeaning lingMeaning =
          pLingDb.langToSpec[genGrounding.word.language].lingDico.statDb.
          getLingMeaning(genGrounding.word.lemma,
                         genGrounding.word.partOfSpeech, true);
      // link the word from static binary dico
      if (!lingMeaning.isEmpty())
      {
        _linkLingMeaning(newMemGrdExp, pRequType, genGrounding.word, lingMeaning);
      }
      else if (!genGrounding.word.lemma.empty())
      {
        // link the lemma
        _links.reqToGrdExps[pRequType].textToSemExps[SemanticLanguageEnum::UNKNOWN][genGrounding.word.lemma].emplace_back(newMemGrdExp);
      }
      else if (genGrounding.coreference)
      {
        return true;
      }
      else if (pLinkNonSpecificStuffs && !genGrounding.coreference &&
               genGrounding.referenceType == SemanticReferenceType::INDEFINITE)
      {
        if (genGrounding.quantity.type == SemanticQuantityType::EVERYTHING ||
            genGrounding.quantity.isEqualToZero())
          _links.reqToGrdExps[pRequType].everythingOrNoEntityTypeToSemExps[genGrounding.entityType].emplace_back(newMemGrdExp);
        else // link the non specific stuffs
          _links.reqToGrdExps[pRequType].genGroundingTypeToSemExps[genGrounding.entityType].emplace_back(newMemGrdExp);
      }
    }
    break;
  }
  case SemanticGroudingType::STATEMENT:
  {
    const SemanticStatementGrounding& statGrounding = pGrounding.getStatementGrounding();
    if (!ConceptSet::haveAConceptNotAny(statGrounding.concepts))
    {
      const auto& statLingDico = pLingDb.langToSpec[statGrounding.word.language].lingDico.statDb;
      StaticLinguisticMeaning lingMeaning = statLingDico.getLingMeaning(statGrounding.word.lemma,
                                                                  statGrounding.word.partOfSpeech, true);

      // link the words from static binary dico
      if (!lingMeaning.isEmpty())
        _linkLingMeaning(newMemGrdExp, pRequType, statGrounding.word, lingMeaning);
    }
    break;
  }
  case SemanticGroudingType::AGENT:
  {
    const SemanticAgentGrounding& agentGrounding = pGrounding.getAgentGrounding();
    _links.reqToGrdExps[pRequType].userIdToSemExps[agentGrounding.userId].emplace_back(newMemGrdExp);
    break;
  }
  case SemanticGroudingType::TEXT:
  {
    const SemanticTextGrounding& textGrounding = pGrounding.getTextGrounding();
    _links.reqToGrdExps[pRequType].textToSemExps[textGrounding.forLanguage][textGrounding.text].emplace_back(newMemGrdExp);
    break;
  }
  case SemanticGroudingType::TIME:
  {
    const SemanticTimeGrounding& timeGrounding = pGrounding.getTimeGrounding();
    auto& linksToGrdExps = _links.reqToGrdExps[pRequType];
    _addTimeLinksFromGrounding(linksToGrdExps, timeGrounding, newMemGrdExp);
    break;
  }
  case SemanticGroudingType::RELATIVEDURATION:
  {
    const SemanticRelativeDurationGrounding& relDurationGrounding = pGrounding.getRelDurationGrounding();
    if (relDurationGrounding.durationType == SemanticRelativeDurationType::DELAYEDSTART)
    {
      auto itSpecifierChild = pGrdExp.children.find(GrammaticalType::SPECIFIER);
      if (itSpecifierChild != pGrdExp.children.end())
      {
        auto* specifierGrdExpPtr = itSpecifierChild->second->getGrdExpPtr_SkipWrapperPtrs();
        if (specifierGrdExpPtr != nullptr)
        {
          auto* durationGrdPtr = specifierGrdExpPtr->grounding().getDurationGroundingPtr();
          if (durationGrdPtr != nullptr)
          {
            SemanticDuration absoluteTimeDuration =
                SemanticTimeGrounding::relativeToAbsolute(durationGrdPtr->duration);
            auto& linksToGrdExps = _links.reqToGrdExps[pRequType];
            _addTimeLinksFromRelativeDurationGrounding(linksToGrdExps, absoluteTimeDuration, newMemGrdExp);
          }
        }
      }
    }
    break;
  }
  case SemanticGroudingType::RELATIVELOCATION:
  {
    const SemanticRelativeLocationGrounding& relLocationGrounding = pGrounding.getRelLocationGrounding();
    _links.reqToGrdExps[pRequType].relLocationToSemExps[relLocationGrounding.locationType].emplace_back(newMemGrdExp);
    break;
  }
  case SemanticGroudingType::RELATIVETIME:
  {
    const SemanticRelativeTimeGrounding& relTimeGrounding = pGrounding.getRelTimeGrounding();
    _links.reqToGrdExps[pRequType].relTimeToSemExps[relTimeGrounding.timeType].emplace_back(newMemGrdExp);
    break;
  }
  case SemanticGroudingType::META:
  {
    const SemanticMetaGrounding& metaGrounding = pGrounding.getMetaGrounding();
    _links.reqToGrdExps[pRequType].grdTypeToSemExps[metaGrounding.refToType].emplace_back(newMemGrdExp);
    break;
  }
  case SemanticGroudingType::NAME:
  {
    const SemanticNameGrounding& nameGrounding = pGrounding.getNameGrounding();
    for (const auto& currName : nameGrounding.nameInfos.names)
      _links.reqToGrdExps[pRequType].textToSemExps[SemanticLanguageEnum::UNKNOWN][currName].emplace_back(newMemGrdExp);
    break;
  }
  case SemanticGroudingType::LANGUAGE:
  {
    const SemanticLanguageGrounding& languageGrounding = pGrounding.getLanguageGrounding();
    _links.reqToGrdExps[pRequType].languageToSemExps[languageGrounding.language].emplace_back(newMemGrdExp);
    break;
  }
  case SemanticGroudingType::RESOURCE:
  {
    const auto& resGrd = pGrounding.getResourceGrounding();
    _links.reqToGrdExps[pRequType].resourceToSemExps[resGrd.resource].emplace_back(newMemGrdExp);
    break;
  }
  case SemanticGroudingType::CONCEPTUAL:
  case SemanticGroudingType::DURATION:
  case SemanticGroudingType::LENGTH:
  case SemanticGroudingType::UNITY:
     break;
  }
  return true;
}


void SemanticMemorySentencePrivate::_linkLingMeaning
(SemanticMemoryGrdExp& pNewMemGrdExp,
 SemanticRequestType pRequType,
 const SemanticWord& pInWord,
 const StaticLinguisticMeaning& pInLingMeaning)
{
  _links.reqToGrdExps[pRequType].meaningsToSemExps[pInWord.language]
      [pInLingMeaning.meaningId].emplace_back(pNewMemGrdExp);
}


void SemanticMemorySentencePrivate::_mergeRequToGrdExps(SemanticMemoryLinks& pToComplete,
                                                        SemanticMemoryLinksForAMemSentence& pRef)
{
  for (auto& currReq : pRef.reqToGrdExps)
    _addLinksFrom(pToComplete.reqToGrdExps[currReq.first], currReq.second, _memSent.id);
}


void SemanticMemorySentencePrivate::_addGenderFromGrdExp
(SemanticMemoryGrdExp& pNewMemGrdExp,
 const GroundedExpression& pGrdExp)
{
  const SemanticGrounding& grounding = pGrdExp.grounding();
  if (grounding.type == SemanticGroudingType::STATEMENT &&
      ConceptSet::haveAConcept(grounding.concepts, "verb_equal_be"))
  {
    _tryToAddAgentGenderFromAStructureEqualityStatement(pNewMemGrdExp, pGrdExp);
    for (auto& currChild : pGrdExp.children)
      _addGenderFromSemExp(pNewMemGrdExp, currChild.second);
  }
}


void SemanticMemorySentencePrivate::_addGenderFromSemExp
(SemanticMemoryGrdExp& pNewMemGrdExp,
 const UniqueSemanticExpression& pSemExp)
{
  const GroundedExpression* grdExp = pSemExp->getGrdExpPtr();
  if (grdExp != nullptr)
  {
    _addGenderFromGrdExp(pNewMemGrdExp, *grdExp);
    return;
  }

  const ListExpression* listExp = pSemExp->getListExpPtr();
  if (listExp != nullptr)
  {
    for (auto& currElt : listExp->elts)
    {
      _addGenderFromSemExp(pNewMemGrdExp, currElt);
    }
    return;
  }
}



void SemanticMemorySentencePrivate::_tryToAddAgentGenderFromAStructureEqualityStatement
(SemanticMemoryGrdExp& pNewMemGrdExp,
 const GroundedExpression& pGrdExp)
{
  auto itSubject = pGrdExp.children.find(GrammaticalType::SUBJECT);
  if (itSubject != pGrdExp.children.end())
  {
    const GroundedExpression* subjectGrdExp = itSubject->second->getGrdExpPtr_SkipWrapperPtrs();
    if (subjectGrdExp != nullptr)
    {
      const SemanticAgentGrounding* subjectAgentGrd = (*subjectGrdExp)->getAgentGroundingPtr();
      if (subjectAgentGrd != nullptr &&
          subjectAgentGrd->isSpecificUser())
      {
        auto itObject = pGrdExp.children.find(GrammaticalType::OBJECT);
        if (itObject != pGrdExp.children.end())
        {
          const GroundedExpression* objectGrdExp = itObject->second->getGrdExpPtr_SkipWrapperPtrs();
          if (objectGrdExp != nullptr)
          {
            const SemanticGenericGrounding* objectGenGrd = (*objectGrdExp)->getGenericGroundingPtr();
            if (objectGenGrd != nullptr)
            {
              SemanticGenderType gender =
                  SemExpGetter::getGenderFromGenGrd(*objectGenGrd);
              if (gender != SemanticGenderType::UNKNOWN)
                _addValueForAMemGrdExp(userCenteredLinks.userIdToUserCharacteristics[subjectAgentGrd->userId].genderLinks,
                    gender, pNewMemGrdExp);
            }
            else
            {
              const SemanticNameGrounding* objectNameGrdPtr = (*objectGrdExp)->getNameGroundingPtr();
              if (objectNameGrdPtr != nullptr)
              {
                SemanticGenderType gender = SemExpGetter::possibleGendersToGender(objectNameGrdPtr->nameInfos.possibleGenders);
                if (gender != SemanticGenderType::UNKNOWN)
                  _addValueForAMemGrdExp(userCenteredLinks.userIdToUserCharacteristics[subjectAgentGrd->userId].genderLinks,
                      gender, pNewMemGrdExp);
              }
            }
          }
        }
      }
    }
  }
}



MemGrdExpPtrOffsets::MemGrdExpPtrOffsets(unsigned char* pBeginPtr)
  : _beginPtr(pBeginPtr),
    _memGrdExpPtrs()
{

}

void MemGrdExpPtrOffsets::addMemGrdExp(const SemanticMemoryGrdExp& pMemGrdExp,
                                       unsigned char* pPtr)
{
  _memGrdExpPtrs.emplace(&pMemGrdExp, pPtr - _beginPtr);
}

uint32_t MemGrdExpPtrOffsets::getOffset(const SemanticMemoryGrdExp& pMemGrdExp) const
{
  auto it = _memGrdExpPtrs.find(&pMemGrdExp);
  if (it != _memGrdExpPtrs.end())
    return it->second;
  assert(false);
  return 0u;
}



SemanticMemorySentence::SemanticMemorySentence
(SemanticContextAxiom& pContextAxiom,
 const GroundedExpression& pGrdExp,
 bool pGatherAllTheLinks,
 const std::map<GrammaticalType, const SemanticExpression*>& pAnnotations,
 const linguistics::LinguisticDatabase& pLingDb,
 bool pIsAConditionToSatisfy,
 bool pIsEnabled,
 intSemId pId)
  : id(pId == 0 ?
         pContextAxiom.getSemExpWrappedForMemory().getParentMemBloc().getNextSemId() :
         pId),
    grdExp(pGrdExp),
    gatherAllTheLinks(pGatherAllTheLinks),
    _contextAxiom(pContextAxiom),
    _annotations(),
    _isEnabled(pIsEnabled),
    _isANoun(false),
    _isAConditionToSatisfy(pIsAConditionToSatisfy),
    _impl(mystd::make_unique<SemanticMemorySentencePrivate>(*this, pAnnotations, pLingDb))
{
}

SemanticMemorySentence::~SemanticMemorySentence()
{
}


bool SemanticMemorySentence::isOtherSentenceMoreRevelant(const SemanticMemorySentence& pOther) const
{
  return _contextAxiom.canOtherInformationTypeBeMoreRevelant(pOther._contextAxiom.informationType) &&
      id < pOther.id;
}

std::string SemanticMemorySentence::getName(const std::string& pUserId) const
{
  return _impl->userCenteredLinks.getName(pUserId);
}

bool SemanticMemorySentence::hasEquivalentUserIds(const std::string& pUserId) const
{
  return _impl->userCenteredLinks.hasEquivalentUserIds(pUserId);
}

const SemanticExpression* SemanticMemorySentence::getSemExpForGrammaticalType(GrammaticalType pGrammType) const
{
  const auto itAnn = _annotations.find(pGrammType);
  if (itAnn != _annotations.end())
    return itAnn->second;
  const auto itChild = grdExp.children.find(pGrammType);
  if (itChild != grdExp.children.end())
    return &*itChild->second;
  return nullptr;
}

void SemanticMemorySentence::writeInBinary(binarymasks::Ptr& pPtr,
                                           MemGrdExpPtrOffsets& pMemGrdExpPtrs,
                                           const semexpsaver::SemExpPtrOffsets& pSemExpPtrOffsets) const
{
  binarysaver::writeInt(pPtr.pint++, id);
  binarysaver::writeInThreeBytes(pPtr.pchar, pSemExpPtrOffsets.grdExpToOffset(grdExp, pPtr.puchar));
  pPtr.pchar += 3;
  _impl->writeInBinary(pPtr, pMemGrdExpPtrs, pSemExpPtrOffsets);
  writeEnumMap<GrammaticalType, const SemanticExpression*>
      (pPtr, _annotations,
       grammaticalType_allValues, [&]
       (binarymasks::Ptr& pSubPtr, const SemanticExpression* const* pSemExpPtr)
  {
    if (pSemExpPtr == nullptr)
      binarysaver::writeInThreeBytes(pPtr.pchar, 0);
    else
      binarysaver::writeInThreeBytes(pPtr.pchar, pSemExpPtrOffsets.semExpToOffset(**pSemExpPtr, pPtr.puchar));
    pPtr.pchar += 3;
  });

}


void SemanticMemorySentence::setEnabled(bool pEnabled)
{
  if (_isEnabled == pEnabled)
    return;
  _isEnabled = pEnabled;

  auto& impl = *_impl;
  if (_isEnabled)
  {
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
  }
  else
  {
    impl.disableUserCenteredLinks();
    auto& memBlocPrivate = *_contextAxiom.getSemExpWrappedForMemory().getParentMemBloc()._impl;
    if (impl.isLinkedInRequToAnswers == LinkedState::LINKED)
    {
      impl.filterMemLinks(memBlocPrivate.answersLinks.getLinks(impl.tense, impl.verbGoal));
      impl.isLinkedInRequToAnswers = LinkedState::LINKDISABLED;
    }

    if (impl.isLinkedInRequToCondition == LinkedState::LINKED)
    {
      impl.filterMemLinks(memBlocPrivate.conditionToInformationLinks.getLinks(impl.tense, impl.verbGoal));
      impl.isLinkedInRequToCondition = LinkedState::LINKDISABLED;
    }

    if (impl.isLinkedInRequToSentWithAction == LinkedState::LINKED)
    {
      impl.filterMemLinks(memBlocPrivate.sentWithActionLinks.getLinks(impl.tense, impl.verbGoal));
      memBlocPrivate.removeConditionToAnAction(*this);
      impl.isLinkedInRequToSentWithAction = LinkedState::LINKDISABLED;
    }

    if (impl.isLinkedInRequToInfAction == LinkedState::LINKED)
    {
      impl.filterMemLinks(memBlocPrivate.sentWithInfActionLinks);
      memBlocPrivate.removeInfAction(*this);
      impl.isLinkedInRequToInfAction = LinkedState::LINKDISABLED;
    }

    if (impl.isLinkedInRequToActionTrigger == LinkedState::LINKED)
    {
      impl.filterMemLinks(memBlocPrivate.ensureSentenceTriggersLinks(SemanticExpressionCategory::COMMAND,
                                                                     _contextAxiom.triggerAxiomId).
                          getLinks(impl.tense, impl.verbGoal));
      memBlocPrivate.removeLinksIfEmpty(_contextAxiom.triggerAxiomId);
      impl.isLinkedInRequToActionTrigger = LinkedState::LINKDISABLED;
    }

    if (impl.isLinkedInRequToQuestionTrigger == LinkedState::LINKED)
    {
      impl.filterMemLinks(memBlocPrivate.ensureSentenceTriggersLinks(SemanticExpressionCategory::QUESTION,
                                                                     _contextAxiom.triggerAxiomId).
                          getLinks(impl.tense, impl.verbGoal));
      memBlocPrivate.removeLinksIfEmpty(_contextAxiom.triggerAxiomId);
      impl.isLinkedInRequToQuestionTrigger = LinkedState::LINKDISABLED;
    }

    if (impl.isLinkedInRequToAffirmationTrigger == LinkedState::LINKED)
    {
      impl.filterMemLinks(memBlocPrivate.ensureSentenceTriggersLinks(SemanticExpressionCategory::AFFIRMATION,
                                                                     _contextAxiom.triggerAxiomId).
                          getLinks(impl.tense, impl.verbGoal));
      memBlocPrivate.removeLinksIfEmpty(_contextAxiom.triggerAxiomId);
      impl.isLinkedInRequToAffirmationTrigger = LinkedState::LINKDISABLED;
    }

    if (impl.isLinkedInRequToNominalGroupsTrigger == LinkedState::LINKED)
    {
      impl.filterMemLinks(memBlocPrivate.ensureNominalGroupsTriggersLinks(_contextAxiom.triggerAxiomId));
      memBlocPrivate.removeLinksIfEmpty(_contextAxiom.triggerAxiomId);
      impl.isLinkedInRequToNominalGroupsTrigger = LinkedState::LINKDISABLED;
    }
  }
}


} // End of namespace onsem

