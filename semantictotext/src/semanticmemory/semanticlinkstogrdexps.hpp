#ifndef ONSEM_SEMANTICTOTEXT_SRC_SEMANTICMEMORY_SEMANTICLINKSTOGRDEXPS_HPP
#define ONSEM_SEMANTICTOTEXT_SRC_SEMANTICMEMORY_SEMANTICLINKSTOGRDEXPS_HPP

#include <map>
#include <unordered_map>
#include <onsem/common/utility/radix_map_struct.hpp>
#include <onsem/common/utility/radix_map.hpp>
#include <onsem/common/utility/vector_map.hpp>
#include <onsem/common/utility/vector_of_refs.hpp>
#include <onsem/common/utility/unordered_set_of_refs.hpp>
#include <onsem/common/binary/binarymasks.hpp>
#include <onsem/common/enum/semanticlanguagetype.hpp>
#include <onsem/common/enum/semanticentitytype.hpp>
#include <onsem/common/enum/semanticquantitytype.hpp>
#include <onsem/common/enum/semanticrelativelocationtype.hpp>
#include <onsem/common/enum/semanticrelativetimetype.hpp>
#include <onsem/common/enum/semanticrequesttype.hpp>
#include <onsem/common/enum/semanticverbtense.hpp>
#include <onsem/common/enum/semanticgendertype.hpp>
#include <onsem/common/enum/verbgoalenum.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticdurationgrounding.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticresourcegrounding.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemorysentenceid.hpp>
#include "usernames.hpp"
#include "semanticmemorygrdexp.hpp"


namespace onsem
{
namespace linguistics
{
struct LinguisticDatabase;
}
namespace semexpsaver
{
class SemExpPtrOffsets;
}
using MemoryGrdExpLinksForAMemSentence = mystd::vector_of_refs<SemanticMemoryGrdExp>;
using MemoryGrdExpLinks = std::map<intSemId, MemoryGrdExpLinksForAMemSentence>;
class MemGrdExpPtrOffsets;
struct GroundedExpressionContainer;
struct SemanticMemoryBlock;


template <typename LINKS_TYPE>
struct SemanticLinksToGrdExpsTemplate
{
  bool empty() const;

  /// Map of concept name -> part of texts
  mystd::radix_map_str<LINKS_TYPE> conceptsToSemExps{};
  /// Map of language type -> (Map of meaning -> part of texts)
  std::map<SemanticLanguageEnum, std::map<int, LINKS_TYPE>> meaningsToSemExps{};
  /// Map of all or nothing entity type -> part of texts
  std::map<SemanticEntityType, LINKS_TYPE> everythingOrNoEntityTypeToSemExps{};
  /// Map of generic grounding type -> part of texts
  std::map<SemanticEntityType, LINKS_TYPE> genGroundingTypeToSemExps{};
  /// Map of time (duration from 1 january of year 0) -> part of texts
  mystd::radix_map_struct<SemanticDuration, LINKS_TYPE> timeToSemExps{};
  /// Map of relativeLocation -> part of texts
  std::map<SemanticRelativeLocationType, LINKS_TYPE> relLocationToSemExps{};
  /// Map of relativeTime -> part of texts
  std::map<SemanticRelativeTimeType, LINKS_TYPE> relTimeToSemExps{};

  // Meta grounding
  // --------------
  /// Grounding type -> part of texts
  std::map<SemanticGroudingType, LINKS_TYPE> grdTypeToSemExps{};

  // Agent grounding
  // ---------------
  /// Map of user id -> part of texts
  mystd::radix_map_str<LINKS_TYPE> userIdToSemExps{};

  // Text grounding or Generic grounding
  // -----------------------------------
  /// Map of language type -> (Map of text -> part of texts)
  std::map<SemanticLanguageEnum, mystd::radix_map_str<LINKS_TYPE>> textToSemExps{};

  // Language grounding
  // ------------------
  /// Map of language -> part of texts
  std::map<SemanticLanguageEnum, LINKS_TYPE> languageToSemExps{};

  // Resource grounding
  // ------------------
  /// Map of language -> part of texts
  mystd::radix_map_struct<SemanticResource, LINKS_TYPE> resourceToSemExps{};
};


using SemanticLinksToGrdExps = SemanticLinksToGrdExpsTemplate<MemoryGrdExpLinks>;
using SemanticLinksToGrdExpsForAMemSentence = SemanticLinksToGrdExpsTemplate<MemoryGrdExpLinksForAMemSentence>;


template <typename LINKS_TYPE>
struct SemanticMemoryLinksTemplate
{
  void clear() { reqToGrdExps.clear(); }
  bool empty() const { return reqToGrdExps.empty(); }

  std::map<SemanticRequestType, SemanticLinksToGrdExpsTemplate<LINKS_TYPE>> reqToGrdExps{};
};


using SemanticMemoryLinks = SemanticMemoryLinksTemplate<MemoryGrdExpLinks>;
using SemanticMemoryLinksForAMemSentence = SemanticMemoryLinksTemplate<MemoryGrdExpLinksForAMemSentence>;


template<typename LINKS_TYPE>
struct SemanticSplitAssertAndInformLinks
{
  bool empty() const { return assertions.empty() && informations.empty(); }
  LINKS_TYPE assertions{};
  LINKS_TYPE informations{};
};


enum class LinksAccessType
{
 MAIN_VALUE,
 MAIN_VALUE_AND_ALL_KEYS,
 ALL
};


template<typename TVALUE, typename LINKS_TYPE, typename MAP_TYPE>
void _semanticValueLinks_switchMainValue(
    std::pair<TVALUE, SemanticSplitAssertAndInformLinks<LINKS_TYPE>>& _mainValuesToSemExp,
    MAP_TYPE& _valuesToSemExps,
    const TVALUE& pNewMainValue,
    SemanticSplitAssertAndInformLinks<LINKS_TYPE>& pNewMainLinks)
{
  auto tmp = std::move(_mainValuesToSemExp);
  _mainValuesToSemExp.first = pNewMainValue;
  _mainValuesToSemExp.second = std::move(pNewMainLinks);
  _valuesToSemExps.erase(pNewMainValue);
  _valuesToSemExps.emplace(tmp.first, std::move(tmp.second));
}


template<typename TVALUE, typename LINKS_TYPE, typename MAP_TYPE>
void _semanticValueLinks_addValue(
    bool& _mainValueIsSet,
    std::pair<TVALUE, SemanticSplitAssertAndInformLinks<LINKS_TYPE>>& _mainValuesToSemExp,
    MAP_TYPE& _valuesToSemExps,
    const TVALUE& pValue,
    const std::function<void(SemanticSplitAssertAndInformLinks<LINKS_TYPE>&)>& pUpdateTheLinks,
    const std::function<bool(const SemanticSplitAssertAndInformLinks<LINKS_TYPE>&, const SemanticSplitAssertAndInformLinks<LINKS_TYPE>&)>& pIsMoreRevelant)
{
  if (!_mainValueIsSet)
  {
    _mainValuesToSemExp.first = pValue;
    pUpdateTheLinks(_mainValuesToSemExp.second);
    _mainValueIsSet = true;
  }
  else if (_mainValuesToSemExp.first == pValue)
  {
    pUpdateTheLinks(_mainValuesToSemExp.second);
  }
  else
  {
    auto& valUpdated = _valuesToSemExps[pValue];
    pUpdateTheLinks(valUpdated);
    if (pIsMoreRevelant(valUpdated, _mainValuesToSemExp.second))
      _semanticValueLinks_switchMainValue(_mainValuesToSemExp, _valuesToSemExps, pValue, valUpdated);
  }
}


template<typename TVALUE, typename LINKS_TYPE, typename MAP_TYPE>
void _semanticValueLinks_removeValue(
    bool& _mainValueIsSet,
    std::pair<TVALUE, SemanticSplitAssertAndInformLinks<LINKS_TYPE>>& _mainValuesToSemExp,
    MAP_TYPE& _valuesToSemExps,
    const TVALUE& pValue,
    const std::function<void(SemanticSplitAssertAndInformLinks<LINKS_TYPE>&)>& pUpdateTheLinks,
    const std::function<bool(const SemanticSplitAssertAndInformLinks<LINKS_TYPE>&, const SemanticSplitAssertAndInformLinks<LINKS_TYPE>&)>& pIsMoreRevelant)
{
  if (!_mainValueIsSet)
    return;
  if (_mainValuesToSemExp.first == pValue)
  {
    pUpdateTheLinks(_mainValuesToSemExp.second);
    auto newMainValue = _mainValuesToSemExp.first;
    auto* newMainLinksPtr = &_mainValuesToSemExp.second;
    for (auto& currElt : _valuesToSemExps)
    {
      if (pIsMoreRevelant(currElt.second, *newMainLinksPtr))
      {
        newMainValue = currElt.first;
        newMainLinksPtr = &currElt.second;
      }
    }

    if (_mainValuesToSemExp.second.empty())
    {
      if (newMainValue == _mainValuesToSemExp.first)
      {
        _mainValueIsSet = false;
      }
      else
      {
        _mainValuesToSemExp.first = newMainValue;
        _mainValuesToSemExp.second = std::move(*newMainLinksPtr);
        _valuesToSemExps.erase(newMainValue);
      }
    }
    else if (newMainValue != _mainValuesToSemExp.first)
    {
      _semanticValueLinks_switchMainValue(_mainValuesToSemExp, _valuesToSemExps, newMainValue, *newMainLinksPtr);
    }
  }
  else
  {
    auto& valUpdated = _valuesToSemExps[pValue];
    pUpdateTheLinks(valUpdated);
    if (valUpdated.empty())
      _valuesToSemExps.erase(pValue);
  }
}


#define SemanticValueLinks_LinksAccessType_MAIN_VALUE \
  bool empty() const { return !_mainValueIsSet; }     \
  const TVALUE& getValue() const { assert(_mainValueIsSet); return _mainValuesToSemExp.first; } \
  const SemanticSplitAssertAndInformLinks<LINKS_TYPE>& getLinks() const { assert(_mainValueIsSet); return _mainValuesToSemExp.second; } \
  void addValue( \
      const TVALUE& pValue, \
      const std::function<void(SemanticSplitAssertAndInformLinks<LINKS_TYPE>&)>& pUpdateTheLinks, \
      const std::function<bool(const SemanticSplitAssertAndInformLinks<LINKS_TYPE>&, const SemanticSplitAssertAndInformLinks<LINKS_TYPE>&)>& pIsMoreRevelant) \
  { return _semanticValueLinks_addValue(_mainValueIsSet, _mainValuesToSemExp, _valuesToSemExps, \
                                        pValue, pUpdateTheLinks, pIsMoreRevelant); } \
  void removeValue( \
      const TVALUE& pValue, \
      const std::function<void(SemanticSplitAssertAndInformLinks<LINKS_TYPE>&)>& pUpdateTheLinks, \
      const std::function<bool(const SemanticSplitAssertAndInformLinks<LINKS_TYPE>&, const SemanticSplitAssertAndInformLinks<LINKS_TYPE>&)>& pIsMoreRevelant) \
  { return _semanticValueLinks_removeValue(_mainValueIsSet, _mainValuesToSemExp, _valuesToSemExps, \
                                           pValue, pUpdateTheLinks, pIsMoreRevelant); } \
\
private: \
  bool _mainValueIsSet; \
  std::pair<TVALUE, SemanticSplitAssertAndInformLinks<LINKS_TYPE>> _mainValuesToSemExp{}; \
  MAP_TYPE _valuesToSemExps{};


#define SemanticValueLinks_LinksAccessType_MAIN_VALUE_AND_ALL_KEYS \
  void iterateOnKeys(const std::function<void(const TVALUE&)>& pOnValue) const \
  { \
    if (!_mainValueIsSet) \
      return; \
    pOnValue(_mainValuesToSemExp.first); \
    for (const auto& currElt : _valuesToSemExps) \
      pOnValue(currElt.first); \
  } \
\
  void iterateOnKeysStoppable(const std::function<bool(const TVALUE&)>& pOnValue) const \
  { \
    if (!_mainValueIsSet || \
        !pOnValue(_mainValuesToSemExp.first)) \
      return; \
    for (const auto& currElt : _valuesToSemExps) \
      if (!pOnValue(currElt.first)) \
        return; \
  }


template<typename TVALUE, typename LINKS_TYPE, typename MAP_TYPE, LinksAccessType access>
struct SemanticValueLinks
{
  SemanticValueLinks_LinksAccessType_MAIN_VALUE
};


template<typename TVALUE, typename LINKS_TYPE, typename MAP_TYPE>
struct SemanticValueLinks<TVALUE, LINKS_TYPE, MAP_TYPE, LinksAccessType::MAIN_VALUE_AND_ALL_KEYS>
{
  SemanticValueLinks_LinksAccessType_MAIN_VALUE_AND_ALL_KEYS
  SemanticValueLinks_LinksAccessType_MAIN_VALUE
};

template<typename TVALUE, typename LINKS_TYPE, typename MAP_TYPE>
struct SemanticValueLinks<TVALUE, LINKS_TYPE, MAP_TYPE, LinksAccessType::ALL>
{
  void iterateOnKeysValues(const std::function<void(const TVALUE&, SemanticSplitAssertAndInformLinks<LINKS_TYPE>&)>& pOnValue);
  void iterateOnKeysValues(const std::function<void(const TVALUE&, const SemanticSplitAssertAndInformLinks<LINKS_TYPE>&)>& pOnValue) const;
  const SemanticSplitAssertAndInformLinks<LINKS_TYPE>* keyToLinks(const TVALUE& pKey) const;
  SemanticSplitAssertAndInformLinks<LINKS_TYPE>* keyToLinks(const TVALUE& pKey);

  SemanticValueLinks_LinksAccessType_MAIN_VALUE_AND_ALL_KEYS
  SemanticValueLinks_LinksAccessType_MAIN_VALUE
};



template <typename LINKS_TYPE, LinksAccessType access>
using SemanticGenderLinks = SemanticValueLinks<SemanticGenderType, LINKS_TYPE, std::map<SemanticGenderType, SemanticSplitAssertAndInformLinks<LINKS_TYPE>>, access>;
template <typename LINKS_TYPE, LinksAccessType access>
using SemanticUserNamesLinks = SemanticValueLinks<UserNames, LINKS_TYPE, std::map<UserNames, SemanticSplitAssertAndInformLinks<LINKS_TYPE>>, access>;
template <typename LINKS_TYPE, LinksAccessType access>
using SemanticStringLinks = SemanticValueLinks<std::string, LINKS_TYPE, mystd::radix_map_str<SemanticSplitAssertAndInformLinks<LINKS_TYPE>>, access>;
template <typename LINKS_TYPE, LinksAccessType access>
using SemanticGrdExpPtrLinks = SemanticValueLinks<const GroundedExpression*, LINKS_TYPE, std::map<const GroundedExpression*, SemanticSplitAssertAndInformLinks<LINKS_TYPE>>, access>;

template <typename LINKS_TYPE, LinksAccessType access>
struct UserCharacteristics
{
  void clear() { genderLinks.clear(); nameLinks.clear(); equivalentUserIdLinks.clear(); semExpLinks.clear(); }
  bool empty() const { return genderLinks.empty() && nameLinks.empty() && equivalentUserIdLinks.empty() && semExpLinks.empty(); }
  SemanticGenderLinks<LINKS_TYPE, access> genderLinks{};
  SemanticUserNamesLinks<LINKS_TYPE, access> nameLinks{};
  SemanticStringLinks<LINKS_TYPE, LinksAccessType::ALL> equivalentUserIdLinks{};
  SemanticGrdExpPtrLinks<LINKS_TYPE, access> semExpLinks{};
};


template <typename LINKS_TYPE, LinksAccessType access1, LinksAccessType access2>
struct SemanticUserCenteredMemoryLinks
{
  void clear() { userIdToUserCharacteristics.clear(); nameToUserIds.clear(); semExpToUserIds.clear(); }
  bool empty() const { return userIdToUserCharacteristics.empty() && nameToUserIds.empty() && semExpToUserIds.empty(); }
  bool doesUserIdExist(const std::string& pUserId) const;

  SemanticGenderType getGender(const std::string& pUserId) const;
  std::string getName(const std::string& pUserId) const;
  std::unique_ptr<SemanticNameGrounding> getNameGrd(const std::string& pUserId,
                                                    const SemanticSplitAssertAndInformLinks<LINKS_TYPE>*& pLinks) const;
  bool iterateOnUserIdLinkedToAName(const std::string& pName,
                                    const std::function<void(const std::string&)>& pOnUserId) const;
  bool hasEquivalentUserIds(const std::string& pUserId) const;
  void getAllEquivalentUserIds(std::list<std::string>& pRes,
                               const std::string& pUserId) const;
  bool areSameUser(const std::string& pUserId1,
                   const std::string& pUserId2,
                   SemanticSplitAssertAndInformLinks<LINKS_TYPE>*& pLinks);
  bool areSameUser(const std::string& pUserId1,
                   const std::string& pUserId2,
                   const SemanticSplitAssertAndInformLinks<LINKS_TYPE>*& pLinks) const;

  //! User identification to a grounded expression that describes the user and it's not his name, it's to avoid sentences like "Eva is Eva".
  std::unique_ptr<GroundedExpressionContainer> getEquivalentGrdExpPtr(const std::string& pUserId) const;

  std::string getUserIdFromGrdExp(const GroundedExpression& pGrdExp,
                                  const SemanticMemoryBlock& pMemBloc,
                                  const linguistics::LinguisticDatabase& pLingDb) const;

  void writeInBinary(binarymasks::Ptr& pPtr,
                     const MemGrdExpPtrOffsets& pMemGrdExpPtrOffsets,
                     const linguistics::LinguisticDatabase& pLingDb) const;

  mystd::radix_map_str<UserCharacteristics<LINKS_TYPE, access2>> userIdToUserCharacteristics{};

  mystd::radix_map_str<SemanticStringLinks<LINKS_TYPE, access1>> nameToUserIds{};

  // semexp to userIds mapping
  std::unordered_map<const GroundedExpression*, SemanticStringLinks<LINKS_TYPE, access2>> semExpToUserIds{};
};

using SemanticUserCenteredMemoryLinksForMem = SemanticUserCenteredMemoryLinks<MemoryGrdExpLinks, LinksAccessType::MAIN_VALUE_AND_ALL_KEYS, LinksAccessType::MAIN_VALUE>;
using SemanticUserCenteredMemoryLinksForAMemSentence = SemanticUserCenteredMemoryLinks<MemoryGrdExpLinksForAMemSentence, LinksAccessType::ALL, LinksAccessType::ALL>;

struct SemanticMemoryLinksForAnyVerbGoal
{
public:
  const SemanticMemoryLinks& getLinksForAGoal(VerbGoalEnum pVerbGoal) const;
  SemanticMemoryLinks& getLinksForAGoal(VerbGoalEnum pVerbGoal);

  void clear() { notification.clear(); ability.clear(); }
  bool empty() const { return notification.empty() && ability.empty(); }

  SemanticMemoryLinks notification;
  SemanticMemoryLinks ability;
};


struct SemanticMemoryLinksForAnyVerbTense
{
public:
  const SemanticMemoryLinksForAnyVerbGoal& getLinksForATense(SemanticVerbTense pTense) const;
  SemanticMemoryLinksForAnyVerbGoal& getLinksForATense(SemanticVerbTense pTense);
  const SemanticMemoryLinks& getLinks(SemanticVerbTense pTense, VerbGoalEnum pVerbGoal) const;
  SemanticMemoryLinks& getLinks(SemanticVerbTense pTense, VerbGoalEnum pVerbGoal);
  void writeInBinary(binarymasks::Ptr& pPtr,
                     const MemGrdExpPtrOffsets& pMemGrdExpPtrOffsets,
                     const linguistics::LinguisticDatabase& pLingDb) const;

  void clear() { links.clear(); pastLinks.clear(); futureLinks.clear(); infintiveLinks.clear(); }
  bool empty() const { return links.empty() && pastLinks.empty() && futureLinks.empty() && infintiveLinks.empty(); }

  // verb at present tense
  SemanticMemoryLinksForAnyVerbGoal links;
  // verb at past tense
  SemanticMemoryLinksForAnyVerbGoal pastLinks;
  // verb at future tense
  SemanticMemoryLinksForAnyVerbGoal futureLinks;
  // verb at infintive tense
  SemanticMemoryLinksForAnyVerbGoal infintiveLinks;
};


void writeUserCenteredLinksInBinary(
    binarymasks::Ptr& pPtr,
    const SemanticUserCenteredMemoryLinksForMem& pUserCenteredMemoryLinks,
    const MemGrdExpPtrOffsets& pMemGrdExpPtrOffsets,
    const semexpsaver::SemExpPtrOffsets& pSemExpPtrOffsets,
    const linguistics::LinguisticDatabase& pLingDb);



template<typename TVALUE, typename LINKS_TYPE, typename MAP_TYPE>
void SemanticValueLinks<TVALUE, LINKS_TYPE, MAP_TYPE, LinksAccessType::ALL>::iterateOnKeysValues(
    const std::function<void(const TVALUE&, SemanticSplitAssertAndInformLinks<LINKS_TYPE>&)>& pOnValue)
{
  if (!_mainValueIsSet)
    return;
  pOnValue(_mainValuesToSemExp.first, _mainValuesToSemExp.second);
  for (auto& currElt : _valuesToSemExps)
    pOnValue(currElt.first, currElt.second);
}

template<typename TVALUE, typename LINKS_TYPE, typename MAP_TYPE>
void SemanticValueLinks<TVALUE, LINKS_TYPE, MAP_TYPE, LinksAccessType::ALL>::iterateOnKeysValues(
    const std::function<void(const TVALUE&, const SemanticSplitAssertAndInformLinks<LINKS_TYPE>&)>& pOnValue) const
{
  if (!_mainValueIsSet)
    return;
  pOnValue(_mainValuesToSemExp.first, _mainValuesToSemExp.second);
  for (auto& currElt : _valuesToSemExps)
    pOnValue(currElt.first, currElt.second);
}

template<typename TVALUE, typename LINKS_TYPE, typename MAP_TYPE>
const SemanticSplitAssertAndInformLinks<LINKS_TYPE>* SemanticValueLinks<TVALUE, LINKS_TYPE, MAP_TYPE, LinksAccessType::ALL>::keyToLinks(
    const TVALUE& pKey) const
{
  if (!_mainValueIsSet)
    return nullptr;
  if (_mainValuesToSemExp.first == pKey)
    return &_mainValuesToSemExp.second;
  for (auto& currElt : _valuesToSemExps)
    if (currElt.first == pKey)
      return &currElt.second;
  return nullptr;
}

template<typename TVALUE, typename LINKS_TYPE, typename MAP_TYPE>
SemanticSplitAssertAndInformLinks<LINKS_TYPE>* SemanticValueLinks<TVALUE, LINKS_TYPE, MAP_TYPE, LinksAccessType::ALL>::keyToLinks(
    const TVALUE& pKey)
{
  if (!_mainValueIsSet)
    return nullptr;
  if (_mainValuesToSemExp.first == pKey)
    return &_mainValuesToSemExp.second;
  for (auto& currElt : _valuesToSemExps)
    if (currElt.first == pKey)
      return &currElt.second;
  return nullptr;
}


} // End of namespace onsem


#endif // ONSEM_SEMANTICTOTEXT_SRC_SEMANTICMEMORY_SEMANTICLINKSTOGRDEXPS_HPP
