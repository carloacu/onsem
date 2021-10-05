#ifndef ONSEM_SEMANTICTOTEXT_SRC_SEMANTICMEMORY_SEMANTICMEMORYBLOCKBINARYREADER_HPP
#define ONSEM_SEMANTICTOTEXT_SRC_SEMANTICMEMORY_SEMANTICMEMORYBLOCKBINARYREADER_HPP

#include <onsem/common/binary/binarymasks.hpp>
#include <onsem/common/enum/semanticverbtense.hpp>
#include <onsem/common/enum/verbgoalenum.hpp>
#include "sentenceslinks.hpp"

namespace onsem
{

struct SemanticMemoryBlockBinaryReader
{
  static std::shared_ptr<SemanticMemoryBlockBinaryReader> getInstance(const std::string& pPath);
  ~SemanticMemoryBlockBinaryReader();

  intMemBlockId getId() const { return _id; }

  const unsigned char* getLinks(SemanticTypeOfLinks pTypeOfLinks,
                                SemanticVerbTense pTense,
                                VerbGoalEnum pVerbGoal) const;

  static const unsigned char* getLinksForARequest(const unsigned char* pPtr,
                                                  SemanticRequestType pRequest);

  static const unsigned char* moveToConceptsPtr(const unsigned char* pPtr);
  static const unsigned char* moveToMeaningsPtr(const unsigned char* pPtr);
  static const unsigned char* moveToEverythingOrNothingEntityPtr(const unsigned char* pPtr);
  static const unsigned char* moveToGenGrdTypePtr(const unsigned char* pPtr);
  static const unsigned char* moveToTimePtr(const unsigned char* pPtr);
  static const unsigned char* moveToRelLocationPtr(const unsigned char* pPtr);
  static const unsigned char* moveToRelTimePtr(const unsigned char* pPtr);
  static const unsigned char* moveToGrdTypePtr(const unsigned char* pPtr);
  static const unsigned char* moveToUserIdPtr(const unsigned char* pPtr);
  static const unsigned char* moveToTextPtr(const unsigned char* pPtr);
  static const unsigned char* moveToLanguagePtr(const unsigned char* pPtr);
  static const unsigned char* moveToResourcePtr(const unsigned char* pPtr);

  static const unsigned char* memoryGrdExpToMemorySentencePtr(const unsigned char* pPtr);
  static const unsigned char* memoryGrdExpToGrdExpPtr(const unsigned char* pPtr);

  const unsigned char* getSemanticExpressionsBlocPtr() const { return _semanticExpressionsPtr; }

  static intSemId memorySentenceToId(const unsigned char* pPtr);
  static const unsigned char* memorySentenceToGrdExpPtr(const unsigned char* pPtr);
  static const unsigned char* memorySentenceToAnnotationsPtr(const unsigned char* pPtr);
  static const unsigned char* readExpPtr(const unsigned char* pPtr);

  const unsigned char* userIdToUserCharacteristicsPtr(const std::string& pUserId) const;
  const unsigned char* userIdToNameLinksPtr(const std::string& pUserId) const;
  const unsigned char* userIdToEquivalentUserIdsPtr(const std::string& pUserId) const;
  static const unsigned char* userCharacteristicsToGenderLinksPtr(const unsigned char* pPtr);
  static const unsigned char* userCharacteristicsToNameLinksPtr(const unsigned char* pPtr);
  bool doesUserIdExist(const std::string& pUserId) const;
  SemanticGenderType getGender(const std::string& pUserId) const;
  std::string getName(const std::string& pUserId) const;
  std::unique_ptr<SemanticNameGrounding> getNameGrd(const std::string& pUserId) const;

  void getAllEquivalentUserIds(std::list<std::string>& pRes,
                               const std::string& pUserId) const;
  std::unique_ptr<GroundedExpressionContainer> getEquivalentGrdExpPtr(const std::string& pUserId,
                                                                      const linguistics::LinguisticDatabase& pLingDb) const;
  bool areSameUser(const std::string& pUserId1,
                   const std::string& pUserId2) const;


  const unsigned char* nameToUserIdPtr(const std::string& pName) const;
  bool iterateOnUserIdLinkedToAName(const std::string& pName,
                                    const std::function<void(const std::string&)>& pOnUserId) const;
  std::string getUserIdFromGrdExp(const GroundedExpression& pGrdExp,
                                  const SemanticMemoryBlock& pMemBloc,
                                  const linguistics::LinguisticDatabase& pLingDb);


  std::string userIdToHardCodedUserIdsPtr(const std::string& pUserId) const;


private:
  intMemBlockId _id;
  unsigned char* _semanticExpressionsPtr;
  unsigned char* _memoryLinksPtr;
  unsigned char* _userLinksPtr;
  unsigned char* _hardCodedUserIdsPtr;

  SemanticMemoryBlockBinaryReader(const std::string& pMemoryBlockFilename);
  void _load(const std::string& pMemoryBlockFilename);
  void _unload();
};



} // End of namespace onsem


#endif // ONSEM_SEMANTICTOTEXT_SRC_SEMANTICMEMORY_SEMANTICMEMORYBLOCKBINARYREADER_HPP
