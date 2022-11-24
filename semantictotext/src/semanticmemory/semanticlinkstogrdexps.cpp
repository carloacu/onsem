#include "semanticlinkstogrdexps.hpp"
#include <onsem/common/binary/radixmapsaver.hpp>
#include <onsem/common/binary/enummapsaver.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/groundedexpression.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticnamegrounding.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semantictimegrounding.hpp>
#include <onsem/semantictotext/semanticmemory/links/groundedexpwithlinks.hpp>
#include <onsem/semantictotext/semanticmemory/links/sentencewithlinks.hpp>
#include <onsem/semantictotext/tool/semexpcomparator.hpp>

namespace onsem
{
namespace
{
void _writeLinks(binarymasks::Ptr& pPtr,
                 uint32_t& pMemSentSize,
                 const std::map<intSemId, MemoryGrdExpLinksForAMemSentence>& pMemSentMap,
                 const MemGrdExpPtrOffsets& pMemGrdExpPtrOffsets)
{
  for (const auto& currLinks : pMemSentMap)
  {
    for (const auto& currGrdExp : currLinks.second)
    {
      binarysaver::writeInt(pPtr.pint++, pMemGrdExpPtrOffsets.getOffset(currGrdExp));
      ++pMemSentSize;
    }
  }
}


void _writeLinksOpt(binarymasks::Ptr& pPtr,
                    const std::map<intSemId, MemoryGrdExpLinksForAMemSentence>* pMemSentMapPtr,
                    const MemGrdExpPtrOffsets& pMemGrdExpPtrOffsets)
{
  binarymasks::Ptr beginOfTheLinks = pPtr;
  pPtr.pchar += 3;
  if (pMemSentMapPtr != nullptr)
  {
    uint32_t memSentSize = 0;
    _writeLinks(pPtr, memSentSize, *pMemSentMapPtr, pMemGrdExpPtrOffsets);
    binarysaver::writeInThreeBytes(beginOfTheLinks.pchar, memSentSize);
  }
  else
  {
    binarysaver::writeInThreeBytes(beginOfTheLinks.pchar, 0x00FFFFFF);
  }
}


void _writeLinksFromAssertAndInform(binarymasks::Ptr& pPtr,
                                    const SemanticSplitAssertAndInformLinks<MemoryGrdExpLinks>& pAssertAndInformLinks,
                                    const MemGrdExpPtrOffsets& pMemGrdExpPtrOffsets)
{
  binarymasks::Ptr beginOfTheLinks = pPtr;
  uint32_t memSentSize = 0;
  pPtr.pchar += 3;
  _writeLinks(pPtr, memSentSize, pAssertAndInformLinks.informations, pMemGrdExpPtrOffsets);
  _writeLinks(pPtr, memSentSize, pAssertAndInformLinks.assertions, pMemGrdExpPtrOffsets);
  binarysaver::writeInThreeBytes(beginOfTheLinks.pchar, memSentSize);
}



void _writeLinksGrdExps(binarymasks::Ptr& pPtr,
                        const SemanticLinksToGrdExps* pLinksToGrdExpsMapPtr,
                        const MemGrdExpPtrOffsets& pMemGrdExpPtrOffsets)
{
  auto _writeGrdExpLinks = [&] (binarymasks::Ptr& pSubPtr,
                                const MemoryGrdExpLinks* pMemSentMapPtr)
  {
    _writeLinksOpt(pSubPtr, pMemSentMapPtr, pMemGrdExpPtrOffsets);
  };
  auto _writeGrdExpLinksRef = [&] (binarymasks::Ptr& pSubPtr,
                                   const MemoryGrdExpLinks& pMemSentMap)
  {
    _writeLinksOpt(pSubPtr, &pMemSentMap, pMemGrdExpPtrOffsets);
  };

  auto offsets = pPtr;
  pPtr.pint += 11;

  pPtr = radixmap::write<MemoryGrdExpLinks>(pLinksToGrdExpsMapPtr->conceptsToSemExps,
                                            pPtr, _writeGrdExpLinks);

  offsets.pint[0] = pPtr.val - offsets.val;
  writeEnumMap<SemanticLanguageEnum, std::map<int, MemoryGrdExpLinks>>
      (pPtr, pLinksToGrdExpsMapPtr->meaningsToSemExps, semanticLanguageEnum_allValues, [&]
       (binarymasks::Ptr& pSubPtr, const std::map<int, MemoryGrdExpLinks>* pIntToGrdExpLinks)
  {
    writeIntMap<MemoryGrdExpLinks>(pSubPtr, *pIntToGrdExpLinks, _writeGrdExpLinksRef);
  });

  offsets.pint[1] = pPtr.val - offsets.val;
  writeEnumMap<SemanticEntityType, MemoryGrdExpLinks>
      (pPtr, pLinksToGrdExpsMapPtr->everythingOrNoEntityTypeToSemExps,
       semanticEntityType_allValues, _writeGrdExpLinks);

  offsets.pint[2] = pPtr.val - offsets.val;
  writeEnumMap<SemanticEntityType, MemoryGrdExpLinks>
      (pPtr, pLinksToGrdExpsMapPtr->genGroundingTypeToSemExps,
       semanticEntityType_allValues, _writeGrdExpLinks);

  offsets.pint[3] = pPtr.val - offsets.val;
  pPtr = radixmap::write<MemoryGrdExpLinks>(pLinksToGrdExpsMapPtr->timeToSemExps,
                                            pPtr, _writeGrdExpLinks);

  offsets.pint[4] = pPtr.val - offsets.val;
  writeEnumMap<SemanticRelativeLocationType, MemoryGrdExpLinks>
      (pPtr, pLinksToGrdExpsMapPtr->relLocationToSemExps,
       semanticRelativeLocationType_allValues, _writeGrdExpLinks);

  offsets.pint[5] = pPtr.val - offsets.val;
  writeEnumMap<SemanticRelativeTimeType, MemoryGrdExpLinks>
      (pPtr, pLinksToGrdExpsMapPtr->relTimeToSemExps,
       semanticRelativeTimeType_allValues, _writeGrdExpLinks);

  offsets.pint[6] = pPtr.val - offsets.val;
  writeEnumMap<SemanticGroundingType, MemoryGrdExpLinks>
      (pPtr, pLinksToGrdExpsMapPtr->grdTypeToSemExps,
       semanticGroundingType_allValues, _writeGrdExpLinks);

  offsets.pint[7] = pPtr.val - offsets.val;
  pPtr = radixmap::write<MemoryGrdExpLinks>(pLinksToGrdExpsMapPtr->userIdToSemExps,
                                            pPtr, _writeGrdExpLinks);

  offsets.pint[8] = pPtr.val - offsets.val;
  writeEnumMap<SemanticLanguageEnum, mystd::radix_map_str<MemoryGrdExpLinks>>
      (pPtr, pLinksToGrdExpsMapPtr->textToSemExps, semanticLanguageEnum_allValues, [&]
       (binarymasks::Ptr& pSubPtr, const mystd::radix_map_str<MemoryGrdExpLinks>* pTextToLinks)
  {
    pSubPtr = radixmap::write<MemoryGrdExpLinks>(*pTextToLinks,
                                                 pSubPtr, _writeGrdExpLinks);
  });

  offsets.pint[9] = pPtr.val - offsets.val;
  writeEnumMap<SemanticLanguageEnum, MemoryGrdExpLinks>
      (pPtr, pLinksToGrdExpsMapPtr->languageToSemExps,
       semanticLanguageEnum_allValues, _writeGrdExpLinks);

  offsets.pint[10] = pPtr.val - offsets.val;
  pPtr = radixmap::write<MemoryGrdExpLinks>(pLinksToGrdExpsMapPtr->resourceToSemExps,
                                            pPtr, _writeGrdExpLinks);
}


void _writeMemoryLinksInBinary(binarymasks::Ptr& pPtr,
                               const SemanticMemoryLinks& pLinks,
                               const MemGrdExpPtrOffsets& pMemGrdExpPtrOffsets,
                               const linguistics::LinguisticDatabase& pLingDb)
{
  writeEnumMap<SemanticRequestType, SemanticLinksToGrdExps>
      (pPtr, pLinks.reqToGrdExps, semanticRequestType_allValues, [&](binarymasks::Ptr& pSubPtr,
       const SemanticLinksToGrdExps* pLinksToGrdExpsMapPtr)
  {
    _writeLinksGrdExps(pSubPtr, pLinksToGrdExpsMapPtr, pMemGrdExpPtrOffsets);
  });
}

template <typename LINKS_TYPE, LinksAccessType access>
bool _getNameWithoutConsideringEquivalences(
    std::string& pRes,
    const UserCharacteristics<LINKS_TYPE, access>& pUserCharacteristics)
{
  if (!pUserCharacteristics.nameLinks.empty())
  {
    pRes = pUserCharacteristics.nameLinks.getValue().getName();
    return true;
  }
  return false;
}

}


template <typename LINKS_TYPE>
void SemanticLinksToGrdExpsTemplate<LINKS_TYPE>::clear()
{
  conceptsToSemExps.clear();
  meaningsToSemExps.clear();
  everythingOrNoEntityTypeToSemExps.clear();
  genGroundingTypeToSemExps.clear();
  timeToSemExps.clear();
  relLocationToSemExps.clear();
  relTimeToSemExps.clear();
  numberToSemExps.clear();
  quantityTypeToSemExps.clear();
  referenceWithoutConceptToSemExps.clear();
  coreferenceWithoutConceptOrReferenceToSemExps.clear();
  statementCoreferenceWithoutConceptToSemExps.clear();
  grdTypeToSemExps.clear();
  userIdToSemExps.clear();
  userIdWithoutContextToSemExps.clear();
  textToSemExps.clear();
  languageToSemExps.clear();
  resourceToSemExps.clear();
}


template <typename LINKS_TYPE>
bool SemanticLinksToGrdExpsTemplate<LINKS_TYPE>::empty() const
{
  return conceptsToSemExps.empty() &&
      meaningsToSemExps.empty() &&
      everythingOrNoEntityTypeToSemExps.empty() &&
      genGroundingTypeToSemExps.empty() &&
      timeToSemExps.empty() &&
      relLocationToSemExps.empty() &&
      numberToSemExps.empty() &&
      quantityTypeToSemExps.empty() &&
      referenceWithoutConceptToSemExps.empty() &&
      coreferenceWithoutConceptOrReferenceToSemExps.empty() &&
      statementCoreferenceWithoutConceptToSemExps.empty() &&
      relTimeToSemExps.empty() &&
      grdTypeToSemExps.empty() &&
      userIdToSemExps.empty() &&
      userIdWithoutContextToSemExps.empty() &&
      textToSemExps.empty() &&
      languageToSemExps.empty() &&
      resourceToSemExps.empty();
}


template <typename LINKS_TYPE, LinksAccessType access1, LinksAccessType access2>
bool SemanticUserCenteredMemoryLinks<LINKS_TYPE, access1, access2>::doesUserIdExist(
    const std::string& pUserId) const
{
  auto* userPtr = userIdToUserCharacteristics.find_ptr(pUserId);
  return userPtr != nullptr && !userPtr->nameLinks.empty();
}


template <typename LINKS_TYPE, LinksAccessType access1, LinksAccessType access2>
SemanticGenderType SemanticUserCenteredMemoryLinks<LINKS_TYPE, access1, access2>::getGender(
    const std::string& pUserId) const
{
  auto* userPtr = userIdToUserCharacteristics.find_ptr(pUserId);
  if (userPtr != nullptr && !userPtr->genderLinks.empty())
    return userPtr->genderLinks.getValue();
  return SemanticGenderType::UNKNOWN;
}


template <typename LINKS_TYPE, LinksAccessType access1, LinksAccessType access2>
std::string SemanticUserCenteredMemoryLinks<LINKS_TYPE, access1, access2>::getName(
    const std::string& pUserId) const
{
  auto* userIdsPtr = userIdToUserCharacteristics.find_ptr(pUserId);
  if (userIdsPtr != nullptr)
  {
    auto& userIds = *userIdsPtr;
    std::string res;
    if (_getNameWithoutConsideringEquivalences(res, userIds))
      return res;

    userIds.equivalentUserIdLinks.iterateOnKeysStoppable([&](const std::string& pSubUserId)
    {
      auto* equUserIdsPtr = userIdToUserCharacteristics.find_ptr(pSubUserId);
      if (equUserIdsPtr != nullptr)
        return !_getNameWithoutConsideringEquivalences(res, *equUserIdsPtr);
      return false;
    });
    return res;
  }
  return "";
}


template <typename LINKS_TYPE, LinksAccessType access1, LinksAccessType access2>
std::unique_ptr<SemanticNameGrounding> SemanticUserCenteredMemoryLinks<LINKS_TYPE, access1, access2>::getNameGrd(
    const std::string& pUserId,
    const SemanticSplitAssertAndInformLinks<LINKS_TYPE>*& pLinks) const
{
  auto* userIdsPtr = userIdToUserCharacteristics.find_ptr(pUserId);
  if (userIdsPtr != nullptr && !userIdsPtr->equivalentUserIdLinks.empty())
  {
    auto& userIds = *userIdsPtr;
    std::string res;
    const std::string& equUserId = userIds.equivalentUserIdLinks.getValue();
    auto* equUserPtr = userIdToUserCharacteristics.find_ptr(equUserId);
    if (_getNameWithoutConsideringEquivalences(res, *equUserPtr))
    {
      pLinks = &userIds.equivalentUserIdLinks.getLinks();
      return std::make_unique<SemanticNameGrounding>(res, equUserId);
    }
  }
  return {};
}


template <typename LINKS_TYPE, LinksAccessType access1, LinksAccessType access2>
bool SemanticUserCenteredMemoryLinks<LINKS_TYPE, access1, access2>::iterateOnUserIdLinkedToAName(
    const std::string& pName,
    const std::function<void(const std::string&)>& pOnUserId) const
{
  const auto* userPtr = nameToUserIds.find_ptr(pName);
  if (userPtr != nullptr)
  {
    userPtr->iterateOnKeys(pOnUserId);
    return true;
  }
  return false;
}


void writeUserCenteredLinksInBinary(
    binarymasks::Ptr& pPtr,
    const SemanticUserCenteredMemoryLinksForMem& pUserCenteredMemoryLinks,
    const MemGrdExpPtrOffsets& pMemGrdExpPtrOffsets,
    const semexpsaver::SemExpPtrOffsets& pSemExpPtrOffsets,
    const linguistics::LinguisticDatabase& pLingDb)
{
  std::map<int32_t, const SemanticStringLinks<MemoryGrdExpLinks, LinksAccessType::MAIN_VALUE>*> semExpOffsetToUserIds;
  for (const auto& currSEToUserId : pUserCenteredMemoryLinks.semExpToUserIds)
    semExpOffsetToUserIds.emplace(pSemExpPtrOffsets.grdExpToOffsetFromBegin(*currSEToUserId.first),
                                  &currSEToUserId.second);
  writeIntMap<const SemanticStringLinks<MemoryGrdExpLinks, LinksAccessType::MAIN_VALUE>*>
      (pPtr, semExpOffsetToUserIds, [&](
       binarymasks::Ptr& pLeafPtr,
       const SemanticStringLinks<MemoryGrdExpLinks, LinksAccessType::MAIN_VALUE>* const& pUserIds)
  {
    binarysaver::writeString(pLeafPtr, pUserIds->getValue());
    _writeLinksFromAssertAndInform(pLeafPtr, pUserIds->getLinks(), pMemGrdExpPtrOffsets);
  });

  auto* nameToUserIdsOffsetPtr = pPtr.pchar;
  pPtr.pchar += 4;

  pPtr = radixmap::write<UserCharacteristics<MemoryGrdExpLinks, LinksAccessType::MAIN_VALUE>>
      (pUserCenteredMemoryLinks.userIdToUserCharacteristics, pPtr, [&](
       binarymasks::Ptr& pLeafPtr,
       const UserCharacteristics<MemoryGrdExpLinks, LinksAccessType::MAIN_VALUE>* pUserCharacteristicsPtr)
  {
    if (pUserCharacteristicsPtr != nullptr)
    {
      auto& userCharacteristics = *pUserCharacteristicsPtr;
      binarysaver::writeChar_0(pLeafPtr.pchar, true);
      if (!userCharacteristics.genderLinks.empty())
      {
        binarysaver::writeChar_1(pLeafPtr.pchar, true);
        binarysaver::writeChar_2To7(pLeafPtr.pchar++, semanticGenderType_toChar(userCharacteristics.genderLinks.getValue()));
        _writeLinksFromAssertAndInform(pLeafPtr, userCharacteristics.genderLinks.getLinks(), pMemGrdExpPtrOffsets);
      }
      else
      {
        binarysaver::writeChar_1(pLeafPtr.pchar++, false);
      }
      if (!userCharacteristics.nameLinks.empty())
      {
        auto& userNames = userCharacteristics.nameLinks.getValue();
        unsigned char nbOfNames = binarysaver::sizet_to_uchar(userNames.names.size());
        binarysaver::writeChar(pLeafPtr.pchar++, nbOfNames);
        for (unsigned char i = 0u; i < nbOfNames; ++i)
          binarysaver::writeString(pLeafPtr, userNames.names[i]);
        if (nbOfNames > 0)
          _writeLinksFromAssertAndInform(pLeafPtr, userCharacteristics.nameLinks.getLinks(), pMemGrdExpPtrOffsets);
      }
      else
      {
        binarysaver::writeChar(pLeafPtr.pchar++, 0);
      }
      if (!userCharacteristics.equivalentUserIdLinks.empty())
      {
        auto* nbOfOtherEquivalentUserIdsPtr = pLeafPtr.pchar;
        pLeafPtr.pchar += 3;
        uint32_t nbOfOtherEquivalentUserIds = 0;
        userCharacteristics.equivalentUserIdLinks.iterateOnKeysValues(
              [&](const std::string& pUserId,
              const SemanticSplitAssertAndInformLinks<MemoryGrdExpLinks>& pLinks)
        {
          binarysaver::writeString(pLeafPtr, pUserId);
          _writeLinksFromAssertAndInform(pLeafPtr, pLinks, pMemGrdExpPtrOffsets);
          ++nbOfOtherEquivalentUserIds;
        });
        binarysaver::writeInThreeBytes(nbOfOtherEquivalentUserIdsPtr, nbOfOtherEquivalentUserIds);
      }
      else
      {
        binarysaver::writeInThreeBytes(pLeafPtr.pchar, 0);
        pLeafPtr.pchar += 3;
      }
      if (!userCharacteristics.semExpLinks.empty())
      {
        auto* mainSemExpLinksPtr = userCharacteristics.semExpLinks.getValue();
        if (mainSemExpLinksPtr != nullptr)
        {
          binarysaver::writeInt(pLeafPtr.pint++,
                                pSemExpPtrOffsets.grdExpToOffsetFromBegin(*mainSemExpLinksPtr));
          _writeLinksFromAssertAndInform(pLeafPtr, userCharacteristics.semExpLinks.getLinks(), pMemGrdExpPtrOffsets);
        }
        else
        {
          binarysaver::writeInt(pLeafPtr.pint++, 0);
        }
      }
      else
      {
        binarysaver::writeInt(pLeafPtr.pint++, 0);
      }
    }
    else
    {
      binarysaver::writeChar_0(pLeafPtr.pchar++, false);
    }
  });

  binarysaver::writeInt(nameToUserIdsOffsetPtr, pPtr.pchar - nameToUserIdsOffsetPtr);

  pPtr = radixmap::write<SemanticStringLinks<MemoryGrdExpLinks, LinksAccessType::MAIN_VALUE_AND_ALL_KEYS>>
      (pUserCenteredMemoryLinks.nameToUserIds, pPtr, [&](
       binarymasks::Ptr& pLeafPtr,
       const SemanticStringLinks<MemoryGrdExpLinks, LinksAccessType::MAIN_VALUE_AND_ALL_KEYS>* pUserIdsPtr)
  {
    if (pUserIdsPtr != nullptr && !pUserIdsPtr->empty())
    {
      auto& userIds = *pUserIdsPtr;
      binarysaver::writeChar(pLeafPtr.pchar++, true);
      auto mainKey = userIds.getValue();
      binarysaver::writeString(pLeafPtr, mainKey);
      _writeLinksFromAssertAndInform(pLeafPtr, userIds.getLinks(), pMemGrdExpPtrOffsets);
      auto* nbOfUserIdsPtr = pLeafPtr.pchar;
      pLeafPtr.pchar += 3;
      uint32_t nbOfUserIds = 0;
      userIds.iterateOnKeys([&](const std::string& pUserId)
      {
        if (pUserId != mainKey)
        {
          binarysaver::writeString(pLeafPtr, pUserId);
          ++nbOfUserIds;
        }
      });
      binarysaver::writeInThreeBytes(nbOfUserIdsPtr, nbOfUserIds);
    }
    else
    {
      binarysaver::writeChar(pLeafPtr.pchar++, false);
    }
  });
}


template <typename LINKS_TYPE, LinksAccessType access1, LinksAccessType access2>
bool SemanticUserCenteredMemoryLinks<LINKS_TYPE, access1, access2>::hasEquivalentUserIds(const std::string& pUserId) const
{
  auto* userIdsPtr = userIdToUserCharacteristics.find_ptr(pUserId);
  return userIdsPtr != nullptr && !userIdsPtr->equivalentUserIdLinks.empty();
}

template <typename LINKS_TYPE, LinksAccessType access1, LinksAccessType access2>
void SemanticUserCenteredMemoryLinks<LINKS_TYPE, access1, access2>::getAllEquivalentUserIds(
    std::list<std::string>& pRes,
    const std::string& pUserId) const
{
  auto* userIdsPtr = userIdToUserCharacteristics.find_ptr(pUserId);
  if (userIdsPtr != nullptr)
  {
    userIdsPtr->equivalentUserIdLinks.iterateOnKeys([&](const std::string& pName)
    {
      pRes.emplace_back(pName);
    });
  }
}


template <typename LINKS_TYPE, LinksAccessType access1, LinksAccessType access2>
bool SemanticUserCenteredMemoryLinks<LINKS_TYPE, access1, access2>::areSameUser(
    const std::string& pUserId1,
    const std::string& pUserId2,
    SemanticSplitAssertAndInformLinks<LINKS_TYPE>*& pLinks)
{
  auto* user1Ptr = userIdToUserCharacteristics.find_ptr(pUserId1);
  if (user1Ptr != nullptr)
  {
    pLinks = user1Ptr->equivalentUserIdLinks.keyToLinks(pUserId2);
    return pLinks != nullptr;
  }
  return false;
}

template <typename LINKS_TYPE, LinksAccessType access1, LinksAccessType access2>
bool SemanticUserCenteredMemoryLinks<LINKS_TYPE, access1, access2>::areSameUser(
    const std::string& pUserId1,
    const std::string& pUserId2,
    const SemanticSplitAssertAndInformLinks<LINKS_TYPE>*& pLinks) const
{
  auto* user1Ptr = userIdToUserCharacteristics.find_ptr(pUserId1);
  if (user1Ptr != nullptr)
  {
    pLinks = user1Ptr->equivalentUserIdLinks.keyToLinks(pUserId2);
    return pLinks != nullptr;
  }
  return false;
}


template <typename LINKS_TYPE, LinksAccessType access1, LinksAccessType access2>
std::unique_ptr<GroundedExpressionContainer> SemanticUserCenteredMemoryLinks<LINKS_TYPE, access1, access2>::getEquivalentGrdExpPtr(
    const std::string& pUserId) const
{
  auto* userPtr = userIdToUserCharacteristics.find_ptr(pUserId);
  if (userPtr != nullptr && !userPtr->semExpLinks.empty())
  {
    auto* grdExpPtr = userPtr->semExpLinks.getValue();
    if (grdExpPtr != nullptr)
      return std::make_unique<GroundedExpressionRef>(*grdExpPtr);
  }
  return {};
}


template <typename LINKS_TYPE, LinksAccessType access1, LinksAccessType access2>
std::string SemanticUserCenteredMemoryLinks<LINKS_TYPE, access1, access2>::getUserIdFromGrdExp(
    const GroundedExpression& pGrdExp,
    const SemanticMemoryBlock& pMemBloc,
    const linguistics::LinguisticDatabase& pLingDb) const
{
  for (const auto& currSemExp : semExpToUserIds)
    if (SemExpComparator::grdExpsAreEqual(pGrdExp, *currSemExp.first, pMemBloc, pLingDb))
      return currSemExp.second.getValue();
  return "";
}




const SemanticMemoryLinksForAnyVerbGoal& SemanticMemoryLinksForAnyVerbTense::getLinksForATense
(SemanticVerbTense pTense) const
{
  switch (pTense)
  {
  case SemanticVerbTense::PUNCTUALPAST:
  case SemanticVerbTense::PAST:
    return pastLinks;
  case SemanticVerbTense::FUTURE:
    return futureLinks;
  case SemanticVerbTense::PRESENT:
  case SemanticVerbTense::PUNCTUALPRESENT:
    return links;
  case SemanticVerbTense::UNKNOWN:
    return infintiveLinks;
  }
  return infintiveLinks;
}

SemanticMemoryLinksForAnyVerbGoal& SemanticMemoryLinksForAnyVerbTense::getLinksForATense
(SemanticVerbTense pTense)
{
  switch (pTense)
  {
  case SemanticVerbTense::PUNCTUALPAST:
  case SemanticVerbTense::PAST:
    return pastLinks;
  case SemanticVerbTense::FUTURE:
    return futureLinks;
  case SemanticVerbTense::PRESENT:
  case SemanticVerbTense::PUNCTUALPRESENT:
    return links;
  case SemanticVerbTense::UNKNOWN:
    return infintiveLinks;
  }
  return infintiveLinks;
}

const SemanticMemoryLinks& SemanticMemoryLinksForAnyVerbTense::getLinks(SemanticVerbTense pTense, VerbGoalEnum pVerbGoal) const
{
  return getLinksForATense(pTense).getLinksForAGoal(pVerbGoal);
}

SemanticMemoryLinks& SemanticMemoryLinksForAnyVerbTense::getLinks(SemanticVerbTense pTense, VerbGoalEnum pVerbGoal)
{
  return getLinksForATense(pTense).getLinksForAGoal(pVerbGoal);
}

void SemanticMemoryLinksForAnyVerbTense::writeInBinary(
    binarymasks::Ptr& pPtr,
    const MemGrdExpPtrOffsets& pMemGrdExpPtrOffsets,
    const linguistics::LinguisticDatabase& pLingDb) const
{
  _writeMemoryLinksInBinary(pPtr, links.notification, pMemGrdExpPtrOffsets, pLingDb);
  _writeMemoryLinksInBinary(pPtr, links.ability, pMemGrdExpPtrOffsets, pLingDb);
  _writeMemoryLinksInBinary(pPtr, pastLinks.notification, pMemGrdExpPtrOffsets, pLingDb);
  _writeMemoryLinksInBinary(pPtr, pastLinks.ability, pMemGrdExpPtrOffsets, pLingDb);
  _writeMemoryLinksInBinary(pPtr, futureLinks.notification, pMemGrdExpPtrOffsets, pLingDb);
  _writeMemoryLinksInBinary(pPtr, futureLinks.ability, pMemGrdExpPtrOffsets, pLingDb);
  _writeMemoryLinksInBinary(pPtr, infintiveLinks.notification, pMemGrdExpPtrOffsets, pLingDb);
  _writeMemoryLinksInBinary(pPtr, infintiveLinks.ability, pMemGrdExpPtrOffsets, pLingDb);
}


const SemanticMemoryLinks& SemanticMemoryLinksForAnyVerbGoal::getLinksForAGoal(VerbGoalEnum pVerbGoal) const
{
  switch (pVerbGoal)
  {
  case VerbGoalEnum::ABILITY:
    return ability;
  default:
    return notification;
  }
}

SemanticMemoryLinks& SemanticMemoryLinksForAnyVerbGoal::getLinksForAGoal(VerbGoalEnum pVerbGoal)
{
  switch (pVerbGoal)
  {
  case VerbGoalEnum::ABILITY:
    return ability;
  default:
    return notification;
  }
}



template struct SemanticLinksToGrdExpsTemplate<MemoryGrdExpLinks>;
template struct SemanticLinksToGrdExpsTemplate<MemoryGrdExpLinksForAMemSentence>;
template struct SemanticUserCenteredMemoryLinks<MemoryGrdExpLinks, LinksAccessType::MAIN_VALUE_AND_ALL_KEYS, LinksAccessType::MAIN_VALUE>;
template struct SemanticUserCenteredMemoryLinks<MemoryGrdExpLinksForAMemSentence, LinksAccessType::ALL, LinksAccessType::ALL>;


} // End of namespace onsem

