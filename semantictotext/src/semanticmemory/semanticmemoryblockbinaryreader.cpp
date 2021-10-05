#include "semanticmemoryblockbinaryreader.hpp"
#include <mutex>
#include <fstream>
#include <onsem/common/binary/binaryloader.hpp>
#include <onsem/common/binary/enummapreader.hpp>
#include <onsem/common/binary/radixmapreader.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/groundedexpression.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticnamegrounding.hpp>
#include <onsem/texttosemantic/dbtype/binary/semexploader.hpp>
#include <onsem/semantictotext/tool/semexpcomparator.hpp>


namespace onsem
{
namespace
{
std::mutex _pathToInstanceMutex;
std::map<std::string, std::weak_ptr<SemanticMemoryBlockBinaryReader>> _pathToInstance;

#define NB_MEMBLOCK_SIZE 24
union MemoryBlockHeader
{
  int32_t intValues[6];
  char charValues[NB_MEMBLOCK_SIZE];
  uint64_t int64Values[2];
};

const unsigned char* _skipGrdExpsVector(const unsigned char* pPtr)
{
  return pPtr + 3 + (binaryloader::loadIntInThreeBytes(pPtr) << 2); // * 4
}

const unsigned char* _jumpAfterLinksForAVerbTense(const unsigned char* pPtr)
{
  for (std::size_t i = 0; i < 8; ++i)
    pPtr = jumpAfterEnumMap(pPtr);
  return pPtr;
}

const unsigned char* _getTenseLinks(const unsigned char* pPtr,
                                    SemanticVerbTense pTense)
{
  std::size_t nbOfMemoryLinksToSkip = 6;
  switch (pTense)
  {
  case SemanticVerbTense::PUNCTUALPAST:
  case SemanticVerbTense::PAST:
    nbOfMemoryLinksToSkip = 2;
    break;
  case SemanticVerbTense::FUTURE:
    nbOfMemoryLinksToSkip = 4;
    break;
  case SemanticVerbTense::PRESENT:
  case SemanticVerbTense::PUNCTUALPRESENT:
    return pPtr;
  case SemanticVerbTense::UNKNOWN:
    break;
  }
  for (std::size_t i = 0; i < nbOfMemoryLinksToSkip; ++i)
    pPtr = jumpAfterEnumMap(pPtr);
  return pPtr;
}

const unsigned char* _getVerbGoalLinks(const unsigned char* pPtr,
                                       VerbGoalEnum pVerbGoal)
{
  if (pVerbGoal == VerbGoalEnum::ABILITY)
    return jumpAfterEnumMap(pPtr);
  return pPtr;
}


bool _getNameWithoutConsideringEquivalences(std::string& pRes,
                                            const unsigned char*& pNameLinksPtr)
{
  unsigned char nbOfNames = *(pNameLinksPtr++);
  if (nbOfNames > 0)
  {
    pRes = SemanticNameGrounding::namesToStrFromBinary(nbOfNames, pNameLinksPtr);
    return true;
  }
  return false;
}

}




std::shared_ptr<SemanticMemoryBlockBinaryReader> SemanticMemoryBlockBinaryReader::getInstance(
    const std::string& pPath)
{
  std::lock_guard<std::mutex> lock(_pathToInstanceMutex);
  auto itElt = _pathToInstance.find(pPath);
  std::shared_ptr<SemanticMemoryBlockBinaryReader> res;
  if (itElt != _pathToInstance.end())
    res = itElt->second.lock();
  if (!res)
  {
    auto res = std::shared_ptr<SemanticMemoryBlockBinaryReader>(new SemanticMemoryBlockBinaryReader(pPath));
    _pathToInstance.emplace(pPath, res);
    return res;
  }
  return res;
}


SemanticMemoryBlockBinaryReader::SemanticMemoryBlockBinaryReader(const std::string& pMemoryBlockFilename)
 : _id(0),
   _semanticExpressionsPtr(nullptr),
   _memoryLinksPtr(nullptr),
   _userLinksPtr(nullptr),
   _hardCodedUserIdsPtr(nullptr)
{
  _load(pMemoryBlockFilename);
}


SemanticMemoryBlockBinaryReader::~SemanticMemoryBlockBinaryReader()
{
  _unload();
}


void SemanticMemoryBlockBinaryReader::_load(const std::string& pMemoryBlockFilename)
{
  std::ifstream memoryBlockFile(pMemoryBlockFilename, std::ifstream::binary);
  if (!memoryBlockFile.is_open())
    throw std::runtime_error("NOT_FOUND (" + pMemoryBlockFilename + ")");

  MemoryBlockHeader header;
  memoryBlockFile.read(header.charValues, NB_MEMBLOCK_SIZE);
#undef NB_MEMBLOCK_SIZE
  _id = header.int64Values[0];
  int32_t semanticexpressionsSize = header.intValues[2];
  int32_t memoryLinksSize = header.intValues[3];
  int32_t userLinksSize = header.intValues[4];
  int32_t hardCodedUserIdsSize = header.intValues[5];

  if (!binaryloader::allocMemZoneU(&_semanticExpressionsPtr, memoryBlockFile, semanticexpressionsSize) ||
      !binaryloader::allocMemZoneU(&_memoryLinksPtr, memoryBlockFile, memoryLinksSize) ||
      !binaryloader::allocMemZoneU(&_userLinksPtr, memoryBlockFile, userLinksSize) ||
      !binaryloader::allocMemZoneU(&_hardCodedUserIdsPtr, memoryBlockFile, hardCodedUserIdsSize))
  {
    _unload();
  }

  memoryBlockFile.close();
}


void SemanticMemoryBlockBinaryReader::_unload()
{
  binaryloader::deallocMemZoneU(&_semanticExpressionsPtr);
  binaryloader::deallocMemZoneU(&_memoryLinksPtr);
  binaryloader::deallocMemZoneU(&_userLinksPtr);
  binaryloader::deallocMemZoneU(&_hardCodedUserIdsPtr);
}


const unsigned char* SemanticMemoryBlockBinaryReader::getLinks(
    SemanticTypeOfLinks pTypeOfLinks,
    SemanticVerbTense pTense,
    VerbGoalEnum pVerbGoal) const
{
  std::size_t linksId = 0;
  switch (pTypeOfLinks)
  {
  case SemanticTypeOfLinks::ANSWER:
    break;
  case SemanticTypeOfLinks::CONDITION_INFORMATION:
    linksId = 1;
    break;
  case SemanticTypeOfLinks::SENT_WITH_ACTION:
    linksId = 2;
    break;
  }
  const unsigned char* res = _memoryLinksPtr;
  for (std::size_t i = 0; i < linksId; ++i)
    res = _jumpAfterLinksForAVerbTense(res);
  res = _getTenseLinks(res, pTense);
  return _getVerbGoalLinks(res, pVerbGoal);
}


const unsigned char* SemanticMemoryBlockBinaryReader::getLinksForARequest(
    const unsigned char* pPtr,
    SemanticRequestType pRequest)
{
  if (pPtr == nullptr)
    return nullptr;
  return readEnumMap(pPtr, semanticRequestType_toChar(pRequest), semanticRequestType_size);
}


const unsigned char* SemanticMemoryBlockBinaryReader::moveToConceptsPtr(const unsigned char* pPtr)
{
  return pPtr + (11 * sizeof(int32_t));
}

const unsigned char* SemanticMemoryBlockBinaryReader::moveToMeaningsPtr(const unsigned char* pPtr)
{
  return pPtr + binaryloader::loadInt(pPtr);
}

const unsigned char* SemanticMemoryBlockBinaryReader::moveToEverythingOrNothingEntityPtr(const unsigned char* pPtr)
{
  return pPtr + binaryloader::loadInt(pPtr + sizeof(int32_t));
}

const unsigned char* SemanticMemoryBlockBinaryReader::moveToGenGrdTypePtr(const unsigned char* pPtr)
{
  return pPtr + binaryloader::loadInt(pPtr + (2 * sizeof(int32_t)));
}

const unsigned char* SemanticMemoryBlockBinaryReader::moveToTimePtr(const unsigned char* pPtr)
{
  return pPtr + binaryloader::loadInt(pPtr + (3 * sizeof(int32_t)));
}

const unsigned char* SemanticMemoryBlockBinaryReader::moveToRelLocationPtr(const unsigned char* pPtr)
{
  return pPtr + binaryloader::loadInt(pPtr + (4 * sizeof(int32_t)));
}

const unsigned char* SemanticMemoryBlockBinaryReader::moveToRelTimePtr(const unsigned char* pPtr)
{
  return pPtr + binaryloader::loadInt(pPtr + (5 * sizeof(int32_t)));
}

const unsigned char* SemanticMemoryBlockBinaryReader::moveToGrdTypePtr(const unsigned char* pPtr)
{
  return pPtr + binaryloader::loadInt(pPtr + (6 * sizeof(int32_t)));
}

const unsigned char* SemanticMemoryBlockBinaryReader::moveToUserIdPtr(const unsigned char* pPtr)
{
  return pPtr + binaryloader::loadInt(pPtr + (7 * sizeof(int32_t)));
}

const unsigned char* SemanticMemoryBlockBinaryReader::moveToTextPtr(const unsigned char* pPtr)
{
  return pPtr + binaryloader::loadInt(pPtr + (8 * sizeof(int32_t)));
}

const unsigned char* SemanticMemoryBlockBinaryReader::moveToLanguagePtr(const unsigned char* pPtr)
{
  return pPtr + binaryloader::loadInt(pPtr + (9 * sizeof(int32_t)));
}

const unsigned char* SemanticMemoryBlockBinaryReader::moveToResourcePtr(const unsigned char* pPtr)
{
  return pPtr + binaryloader::loadInt(pPtr + (10 * sizeof(int32_t)));
}


const unsigned char* SemanticMemoryBlockBinaryReader::memoryGrdExpToMemorySentencePtr(const unsigned char* pPtr)
{
  const unsigned char* ptr = pPtr;
  ptr += 3;
  unsigned char index = *ptr;
  return pPtr - (index * 4) - 1 /* nb of elts */ - 3 /* grd exp ptr */ - 4 /* memory sentence id */;
}

const unsigned char* SemanticMemoryBlockBinaryReader::memoryGrdExpToGrdExpPtr(const unsigned char* pPtr)
{
  return pPtr - binaryloader::loadIntInThreeBytes(pPtr);
}


intSemId SemanticMemoryBlockBinaryReader::memorySentenceToId(const unsigned char* pPtr)
{
  return binaryloader::loadInt(pPtr);
}

const unsigned char* SemanticMemoryBlockBinaryReader::memorySentenceToGrdExpPtr(const unsigned char* pPtr)
{
  pPtr += 4; /* memory sentence id */
  return pPtr - binaryloader::loadIntInThreeBytes(pPtr);
}

const unsigned char* SemanticMemoryBlockBinaryReader::memorySentenceToAnnotationsPtr(const unsigned char* pPtr)
{
  pPtr += 4 /* memory sentence id */ + 3 /* grd exp ptr */;
  return pPtr + 1 /* nb of memory grd exp offset */ +
      ((*pPtr) /* nb of memory grd exp */ * 4);
}

const unsigned char* SemanticMemoryBlockBinaryReader::readExpPtr(const unsigned char* pPtr)
{
  return pPtr - binaryloader::loadIntInThreeBytes(pPtr);
}



const unsigned char* SemanticMemoryBlockBinaryReader::userIdToUserCharacteristicsPtr(const std::string& pUserId) const
{
  const unsigned char* ptr =  _userLinksPtr;
  ptr = jumpAfterIntMap(ptr) + 4;
  return radixmap::read(ptr, pUserId, [](const unsigned char* pEndOfNodePtr) { return binaryloader::loadChar_0(pEndOfNodePtr); });
}

const unsigned char* SemanticMemoryBlockBinaryReader::userIdToNameLinksPtr(const std::string& pUserId) const
{
  const unsigned char* ptr = userIdToUserCharacteristicsPtr(pUserId);
  return ptr != nullptr ? userCharacteristicsToNameLinksPtr(ptr) : nullptr;
}

const unsigned char* SemanticMemoryBlockBinaryReader::userIdToEquivalentUserIdsPtr(const std::string& pUserId) const
{
  const unsigned char* ptr = userIdToUserCharacteristicsPtr(pUserId);
  if (ptr != nullptr)
  {
    ptr = userCharacteristicsToNameLinksPtr(ptr);
    unsigned char nbOfNames = *(ptr++);
    if (nbOfNames > 0)
    {
      for (unsigned char i = 0u; i < nbOfNames; ++i)
        binaryloader::skipString(ptr);
      return _skipGrdExpsVector(ptr);
    }
  }
  return ptr;
}


const unsigned char* SemanticMemoryBlockBinaryReader::userCharacteristicsToGenderLinksPtr(const unsigned char* pPtr)
{
  if (binaryloader::loadChar_0(pPtr))
    return pPtr;
  return nullptr;
}

const unsigned char* SemanticMemoryBlockBinaryReader::userCharacteristicsToNameLinksPtr(const unsigned char* pPtr)
{
  if (binaryloader::loadChar_1(pPtr++))
    return _skipGrdExpsVector(pPtr);
  return pPtr;
}



bool SemanticMemoryBlockBinaryReader::doesUserIdExist(const std::string& pUserId) const
{
  const unsigned char* ptr = userIdToNameLinksPtr(pUserId);
  return ptr != nullptr && *ptr != 0;
}


SemanticGenderType SemanticMemoryBlockBinaryReader::getGender(const std::string& pUserId) const
{
  const unsigned char* ptr = userIdToUserCharacteristicsPtr(pUserId);
  if (ptr != nullptr &&
      binaryloader::loadChar_1(ptr++))
    return semanticGenderType_fromChar(binaryloader::loadChar_2To7(ptr));
  return SemanticGenderType::UNKNOWN;
}

std::string SemanticMemoryBlockBinaryReader::getName(const std::string& pUserId) const
{
  const unsigned char* ptr = userIdToNameLinksPtr(pUserId);
  if (ptr != nullptr)
  {
    std::string res;
    if (_getNameWithoutConsideringEquivalences(res, ptr))
      return res;

    uint32_t nbOfOtherEquivalentUserIds = binaryloader::loadIntInThreeBytes(ptr);
    ptr += 3;
    for (uint32_t i = 0; i < nbOfOtherEquivalentUserIds; ++i)
    {
      auto* subPtr = userIdToNameLinksPtr(binaryloader::loadString(ptr));
      if (subPtr != nullptr &&
          _getNameWithoutConsideringEquivalences(res, subPtr))
        return res;
      ptr = _skipGrdExpsVector(ptr);
    }
  }
  return "";
}


std::unique_ptr<SemanticNameGrounding> SemanticMemoryBlockBinaryReader::getNameGrd
(const std::string& pUserId) const
{
  const unsigned char* ptr = userIdToEquivalentUserIdsPtr(pUserId);
  if (ptr != nullptr)
  {
    uint32_t nbOfOtherEquivalentUserIds = binaryloader::loadIntInThreeBytes(ptr);
    ptr += 3;
    if (nbOfOtherEquivalentUserIds > 0)
    {
      std::string res;
      auto equUserId = binaryloader::loadString(ptr);
      auto* subPtr = userIdToNameLinksPtr(equUserId);
      if (subPtr != nullptr &&
          _getNameWithoutConsideringEquivalences(res, subPtr))
        return mystd::make_unique<SemanticNameGrounding>(res, equUserId);
    }
  }
  return {};
}


void SemanticMemoryBlockBinaryReader::getAllEquivalentUserIds(
    std::list<std::string>& pRes,
    const std::string& pUserId) const
{
  const unsigned char* ptr = userIdToEquivalentUserIdsPtr(pUserId);
  if (ptr != nullptr)
  {
    uint32_t nbOfOtherEquivalentUserIds = binaryloader::loadIntInThreeBytes(ptr);
    ptr += 3;
    for (uint32_t i = 0; i < nbOfOtherEquivalentUserIds; ++i)
    {
      pRes.emplace_back(binaryloader::loadString(ptr));
      ptr = _skipGrdExpsVector(ptr);
    }
  }
}


std::unique_ptr<GroundedExpressionContainer> SemanticMemoryBlockBinaryReader::getEquivalentGrdExpPtr(
    const std::string& pUserId,
    const linguistics::LinguisticDatabase& pLingDb) const
{
  const unsigned char* ptr = userIdToEquivalentUserIdsPtr(pUserId);
  if (ptr != nullptr)
  {
    uint32_t nbOfOtherEquivalentUserIds = binaryloader::loadIntInThreeBytes(ptr);
    ptr += 3;
    for (uint32_t i = 0; i < nbOfOtherEquivalentUserIds; ++i)
    {
      binaryloader::skipString(ptr);
      ptr = _skipGrdExpsVector(ptr);
    }
    auto semExpOffset = binaryloader::loadInt(ptr);
    if (semExpOffset != 0)
    {
      ptr = _semanticExpressionsPtr + semExpOffset;
      return semexploader::loadGrdExp(ptr, pLingDb);
    }
  }
  return nullptr;
}


bool SemanticMemoryBlockBinaryReader::areSameUser(const std::string& pUserId1,
                                                  const std::string& pUserId2) const
{
  const unsigned char* ptr = userIdToEquivalentUserIdsPtr(pUserId1);
  if (ptr != nullptr)
  {
    uint32_t nbOfOtherEquivalentUserIds = binaryloader::loadIntInThreeBytes(ptr);
    ptr += 3;
    for (uint32_t i = 0; i < nbOfOtherEquivalentUserIds; ++i)
    {
      if (binaryloader::loadString(ptr) == pUserId2)
        return true;
      ptr = _skipGrdExpsVector(ptr);
    }
  }
  return false;
}



// -----------------------------------------------------------------

const unsigned char* SemanticMemoryBlockBinaryReader::nameToUserIdPtr(const std::string& pName) const
{
  const unsigned char* ptr =  _userLinksPtr;
  ptr = jumpAfterIntMap(ptr);
  ptr = ptr + binaryloader::loadInt(ptr);
  return radixmap::read(ptr, pName, [](const unsigned char* pEndOfNodePtr) { return *pEndOfNodePtr == true; });
}


bool SemanticMemoryBlockBinaryReader::iterateOnUserIdLinkedToAName(
    const std::string& pName,
    const std::function<void(const std::string&)>& pOnUserId) const
{
  auto* ptr = nameToUserIdPtr(pName);
  if (ptr != nullptr)
  {
    pOnUserId(binaryloader::loadString(++ptr));
    ptr = _skipGrdExpsVector(ptr);
    uint32_t nbOfOtherEquivalentUserIds = binaryloader::loadIntInThreeBytes(ptr);
    ptr += 3;
    for (uint32_t i = 0; i < nbOfOtherEquivalentUserIds; ++i)
      pOnUserId(binaryloader::loadString(ptr));
    return true;
  }
  return false;
}



// ---------------------------------------------------

std::string SemanticMemoryBlockBinaryReader::getUserIdFromGrdExp(
    const GroundedExpression& pGrdExp,
    const SemanticMemoryBlock& pMemBloc,
    const linguistics::LinguisticDatabase& pLingDb)
{
  std::string res;
  readAllIntMapValues(_userLinksPtr, [&](int pGrdExpOffset,
                      const unsigned char* pStrPtr)
  {
    if (res.empty())
    {
      const auto* grdExpPtr = _semanticExpressionsPtr + pGrdExpOffset;
      if (SemExpComparator::grdExpsAreEqual
          (pGrdExp, *semexploader::loadGrdExp(grdExpPtr, pLingDb), pMemBloc, pLingDb))
        res = binaryloader::loadString(pStrPtr);
    }
  });
  return res;
}


// --------------------------------------------------------



std::string SemanticMemoryBlockBinaryReader::userIdToHardCodedUserIdsPtr(const std::string& pUserId) const
{
  const unsigned char* strPtr = radixmap::read(_hardCodedUserIdsPtr, pUserId,
                                               [](const unsigned char* pEndOfNodePtr) { return *pEndOfNodePtr == true; });
  if (strPtr != nullptr)
  {
    ++strPtr;
    return binaryloader::loadString(strPtr);
  }
  return "";
}


} // End of namespace onsem

