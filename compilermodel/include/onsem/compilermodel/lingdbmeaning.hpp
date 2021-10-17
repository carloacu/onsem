#ifndef ONSEM_COMPILERMODEL_LINGDBMEANING_HPP
#define ONSEM_COMPILERMODEL_LINGDBMEANING_HPP

#include <vector>
#include <list>
#include <string>
#include <onsem/common/enum/wordcontextualinfos.hpp>
#include <onsem/common/enum/partofspeech.hpp>

namespace onsem
{
template <typename T>
struct ForwardPtrList;
class LingdbDynamicTrieNode;
class LingdbWordForms;
class CompositePoolAllocator;
class LinguisticIntermediaryDatabase;
class LingdbLinkToAConcept;
class LingdbConcept;


/**
 * @brief Class that hold a meaning of the database.
 * A meaning is a pair of a lemma and a grammatical type.
 */
class LingdbMeaning
{
public:

  // Modifiers
  // ---------

  void addLinkToConcept
  (LinguisticIntermediaryDatabase& pLingDatabase,
   const LingdbConcept* pNewConcept,
   const std::string& pNewConceptStr,
   char pRelatedToConcept,
   bool pReplaceIfAlreadyExist);


  // Getters
  // -------

  const ForwardPtrList<LingdbLinkToAConcept>* getLinkToConcepts() const;

  void getLinkToConceptsWithoutNullRelation
  (std::vector<const LingdbLinkToAConcept*>& pLinkToConcepts) const;

  /**
   * @brief Get the lemme of this meaning.
   * @return The lemme of this meaning.
   */
  LingdbDynamicTrieNode* getLemma() const;

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
  LingdbDynamicTrieNode* fLemma;
  /// The number of wordforms that point to this meaning.
  unsigned int fPtrCount;
  /// The associated link to concepts.
  ForwardPtrList<LingdbLinkToAConcept>* fLinkToConcepts;
  ForwardPtrList<char>* fContextInfos;


private:
  friend class LinguisticIntermediaryDatabase;
  friend class LingdbDynamicTrieNode;
  friend class LingdbWordForms;
  friend class PonderatedMeaning;

  /**
   * @brief Get the position of the pointers for the allocator.
   * @param pRes The position of the pointers.
   * @param pVar An object of this class.
   */
  static void xGetPointers
  (std::vector<const void*>& pRes, void* pVar);

  /// Constructor.
  LingdbMeaning();

  /**
   * @brief Initialize the meaning.
   * @param pLemme The lemme of this meaning.
   * @param pGram The grammatical type of this meaning.
   */
  void xInit
  (LingdbDynamicTrieNode* pLemme,
   char pGram);

  /**
   * @brief Deallocate the meaning.
   * @param pFPAlloc The allocator.
   */
  void xDeallocate(CompositePoolAllocator& pFPAlloc);


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

#include "details/lingdbmeaning.hxx"

#endif // ONSEM_COMPILERMODEL_LINGDBMEANING_HPP
