#ifndef ALLINGDBFLEXIONS_H
#define ALLINGDBFLEXIONS_H

#include <vector>
#include <string>
#include <assert.h>
#include <onsem/common/enum/partofspeech.hpp>


namespace onsem
{
class ALCompositePoolAllocator;
class ALLingdbWordForms;
class ALLingdbDynamicTrieNode;


struct WordLinkForConj
{
  WordLinkForConj()
    : node(nullptr),
      frequency(4)
  {
  }

  void newNodeCandidate
  (ALLingdbDynamicTrieNode const* pNode,
   char pFrequency)
  {
    if (node == nullptr || pFrequency > frequency)
    {
      node = pNode;
      frequency = pFrequency;
    }
  }

  ALLingdbDynamicTrieNode const* node;
  char frequency;
};

struct NounAdjConjugaison
{
  NounAdjConjugaison() = default;

  NounAdjConjugaison(const NounAdjConjugaison&) = delete;
  NounAdjConjugaison& operator=(const NounAdjConjugaison&) = delete;

  WordLinkForConj masculineSingular{};
  WordLinkForConj masculinePlural{};
  WordLinkForConj feminineSingular{};
  WordLinkForConj femininePlural{};
  WordLinkForConj neutralSingular{};
  WordLinkForConj neutralPlural{};
  WordLinkForConj comparative{};
  WordLinkForConj superlative{};
};

struct VerbNumberAndGenderConjugaison
{
  VerbNumberAndGenderConjugaison() = default;

  VerbNumberAndGenderConjugaison(const VerbNumberAndGenderConjugaison&) = delete;
  VerbNumberAndGenderConjugaison& operator=(const VerbNumberAndGenderConjugaison&) = delete;

  WordLinkForConj masculineSingular{};
  WordLinkForConj masculinePlural{};
  WordLinkForConj feminineSingular{};
  WordLinkForConj femininePlural{};
};

struct VerbConjugaison
{
  VerbConjugaison() = default;

  VerbConjugaison(const VerbConjugaison&) = delete;
  VerbConjugaison& operator=(const VerbConjugaison&) = delete;

  WordLinkForConj infinitive{};
  std::vector<WordLinkForConj> imperfect = std::vector<WordLinkForConj>{6};
  std::vector<WordLinkForConj> present = std::vector<WordLinkForConj>{6};
  std::vector<WordLinkForConj> future = std::vector<WordLinkForConj>{6};
  std::vector<WordLinkForConj> imperative = std::vector<WordLinkForConj>{3};
  WordLinkForConj presentParticiple{};
  VerbNumberAndGenderConjugaison pastParticiple{};
  std::vector<WordLinkForConj> conditional = std::vector<WordLinkForConj>{6};
  std::vector<WordLinkForConj> presentSubjunctive = std::vector<WordLinkForConj>{6};
};




/// Class that hold the flexions of a word from.
class ALLingdbFlexions
{
public:

  // Getters
  // -------

  /**
   * @brief Get the number of flexions.
   * @return The number of flexions.
   */
  unsigned char getNbFlexions() const;

  /**
   * @brief Get the memory that store the flexions.
   * @return The memory that store the flexions.
   */
  const unsigned char* getMemory() const;


  bool replaceInfinitiveByImperative
  () const;


  /**
   * @brief Prettry print the flexions in a stream.
   * @param pOs The stream.
   * @param pGram The grammatical type of the flexions.
   */
  void writeInStream
  (std::ostream& pOs,
   PartOfSpeech pGram) const;

  void fillVerbConjugaison
  (VerbConjugaison& pVerbConjugaison,
   const ALLingdbDynamicTrieNode* pTriNode,
   char pFrequency) const;

  void fillNounConjugaison
  (NounAdjConjugaison& pNounConjugaison,
   const ALLingdbDynamicTrieNode* pTriNode,
   char pFrequency) const;

private:
  /// Number of flexions.
  unsigned char fNbFlexions;
  /// All the flexions.
  unsigned char* fFlexions;


private:
  friend class LinguisticIntermediaryDatabase;
  friend class ALLingdbWordForms;

  /**
   * @brief Get the position of the pointers for the allocator.
   * @param pRes The position of the pointers.
   * @param pVar An object of this class.
   */
  static void xGetPointers
  (std::vector<const void*>& pRes, void* pVar);

  /// Constructor.
  ALLingdbFlexions();

  /**
   * @brief Initialize the flexions.
   * @param pFPAlloc The allocator.
   * @param pGram The grammatical type of the flexions.
   * @param pFlexions The flexions.
   * @param pWFForErrorLog Corresponding wordform for the error logs.
   */
  void xInit
  (ALCompositePoolAllocator& pFPAlloc,
   PartOfSpeech pGram,
   const std::vector<std::string>& pFlexions,
   ALLingdbWordForms* pWFForErrorLog);

  void xFillVerbNumberAndGenderConjugaison
  (VerbNumberAndGenderConjugaison& pVerbNumberAndGenderConjugaison,
   char pCurrFlexion,
   const ALLingdbDynamicTrieNode* pTriNode,
   char pFrequency) const;

  /**
   * @brief Merge the existing flexions with new ones.
   * @param pFPAlloc The allocator.
   * @param pGram The grammatical type of the flexions.
   * @param pFlexions The flexions.
   * @param pWFForErrorLog Corresponding wordform for the error logs.
   */
  void xAddNewFlexions
  (ALCompositePoolAllocator& pFPAlloc,
   PartOfSpeech pGram,
   const std::vector<std::string>& pFlexions,
   ALLingdbWordForms* pWFForErrorLog);


  /**
   * @brief Deallocate the flexions.
   * @param pFPAlloc The allocator.
   */
  void xDeallocate
  (ALCompositePoolAllocator& pFPAlloc);

  /**
   * @brief Get a copy of the flexions.
   * @param pFPAlloc The allocator.
   * @return A copy of the flexions.
   */
  ALLingdbFlexions* xClone
  (ALCompositePoolAllocator& pFPAlloc) const;


  /**
   * @brief Convert a string flexion to a char.
   * @param pGram The grammatical type.
   * @param pFlexion The flexion.
   * @param pWFForErrorLog Corresponding wordform for the error logs.
   * @return A char that contain the flexion.
   */
  char xFlexionStringToChar
  (PartOfSpeech pGram,
   const std::string& pFlexion,
   ALLingdbWordForms* pWFForErrorLog) const;

  /**
   * @brief Convert a noun string flexion to a char.
   * @param pFlexion The flexion.
   * @return A char that contain the noun flexion.
   */
  char xFlexionStringToCharForNoun
  (const std::string& pFlexion) const;

  /**
   * @brief Convert a pronoun string flexion to a char.
   * @param pFlexion The flexion.
   * @return A char that contain the pronoun flexion.
   */
  char xFlexionStringToCharForPronoun
  (const std::string& pFlexion) const;

  /**
   * @brief Convert a verb string flexion to a char.
   * @param pFlexion The flexion.
   * @return A char that contain the verb flexion.
   */
  char xFlexionStringToCharForVerb
  (const std::string& pFlexion) const;

  void xFillImperativeVerbTense
  (std::vector<WordLinkForConj>& pVerbTense,
   char pCurrFlexion,
   const ALLingdbDynamicTrieNode* pTriNode,
   char pFrequency) const;

  void xFillVerbTense
  (std::vector<WordLinkForConj>& pVerbTense,
   char pCurrFlexion,
   const ALLingdbDynamicTrieNode* pTriNode,
   char pFrequency) const;
};



} // End of namespace onsem

#endif // ALLINGDBFLEXIONS_H
