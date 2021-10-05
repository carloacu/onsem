#ifndef ALLINGDBMEANING_H
#define ALLINGDBMEANING_H

#include <vector>
#include <list>
#include <string>
#include <onsem/common/enum/wordcontextualinfos.hpp>
#include <onsem/common/enum/partofspeech.hpp>

namespace onsem
{
template <typename T>
struct ForwardPtrList;
class ALLingdbDynamicTrieNode;
class ALLingdbWordForms;
class ALCompositePoolAllocator;
class LinguisticIntermediaryDatabase;
class ALLingdbLinkToAConcept;
class ALLingdbConcept;


/**
 * @brief Class that hold a meaning of the database.
 * A meaning is a pair of a lemma and a grammatical type.
 */
class ALLingdbMeaning
{
public:

  // Modifiers
  // ---------

  void addLinkToConcept
  (LinguisticIntermediaryDatabase& pLingDatabase,
   const ALLingdbConcept* pNewConcept,
   const std::string& pNewConceptStr,
   char pRelatedToConcept,
   bool pReplaceIfAlreadyExist);


  // Getters
  // -------

  const ForwardPtrList<ALLingdbLinkToAConcept>* getLinkToConcepts() const;

  void getLinkToConceptsWithoutNullRelation
  (std::vector<const ALLingdbLinkToAConcept*>& pLinkToConcepts) const;

  /**
   * @brief Get the lemme of this meaning.
   * @return The lemme of this meaning.
   */
  ALLingdbDynamicTrieNode* getLemma() const;

  /**
   * @brief Get the grammatical type of this meaning.
   * @return The grammatical type of this meaning.
   */
  PartOfSpeech getPartOfSpeech() const;


  void getLinkToConceptsStr
  (std::list<std::string>& pConceptsStr) const;

  const ForwardPtrList<char>* getContextInfos() const;

  void getAllContextInfos
  (std::list<char>& pAllContextInfos) const;


  ForwardPtrList<char>* pushBackContextInfo(LinguisticIntermediaryDatabase& pLingDatabase,
                                            WordContextualInfos pContextInfo);
  void removeContextInfo(LinguisticIntermediaryDatabase& pLingDatabase,
                         WordContextualInfos pContextInfo);


private:
  /// The grammatical type of this meaning.
  char fPartOfSpeech;
  /// The lemme of this meaning.
  ALLingdbDynamicTrieNode* fLemma;
  /// The number of wordforms that point to this meaning.
  unsigned int fPtrCount;
  /// The associated link to concepts.
  ForwardPtrList<ALLingdbLinkToAConcept>* fLinkToConcepts;
  ForwardPtrList<char>* fContextInfos;


private:
  friend class LinguisticIntermediaryDatabase;
  friend class ALLingdbDynamicTrieNode;
  friend class ALLingdbWordForms;
  friend class PonderatedMeaning;

  /**
   * @brief Get the position of the pointers for the allocator.
   * @param pRes The position of the pointers.
   * @param pVar An object of this class.
   */
  static void xGetPointers
  (std::vector<const void*>& pRes, void* pVar);

  /// Constructor.
  ALLingdbMeaning();

  /**
   * @brief Initialize the meaning.
   * @param pLemme The lemme of this meaning.
   * @param pGram The grammatical type of this meaning.
   */
  void xInit
  (ALLingdbDynamicTrieNode* pLemme,
   char pGram);

  /**
   * @brief Deallocate the meaning.
   * @param pFPAlloc The allocator.
   */
  void xDeallocate(ALCompositePoolAllocator& pFPAlloc);


  bool xIsSecondConceptMorePrecise
  (const std::string& pConceptName1,
   const std::string& pConceptName2) const;


  /// Say that a new wordform point to this meaning.
  void xAddAPtrToThisMeaning();

  /// Say that we have removed an existing wordform pointer to this meaning.
  unsigned int xRemoveAPtrToThisMeaning();

  ForwardPtrList<char>* xPushBackContextInfo
  (LinguisticIntermediaryDatabase& pLingDatabase,
   ForwardPtrList<char>*& pContInfos,
   char pContextInfo);
};


} // End of namespace onsem

#include "details/allingdbmeaning.hxx"

#endif // ALLINGDBMEANING_H
