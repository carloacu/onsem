#ifndef ALLINGDBWORDFORMS_H
#define ALLINGDBWORDFORMS_H

#include <vector>
#include <string>
#include <onsem/common/enum/partofspeech.hpp>


namespace onsem
{
class ALCompositePoolAllocator;
class ALLingdbDynamicTrieNode;
template <typename T>
struct ForwardPtrList;
class ALLingdbMeaning;
class ALLingdbFlexions;
class LinguisticIntermediaryDatabase;

/**
 * @brief Class that hold a Word Form for a word.
 * A Word Form is a meaning and some flexions informations.
 */
class ALLingdbWordForms
{
public:
  // Modifiers
  // ---------

  /**
   * @brief Copy the flexions into this wordform.
   * @param pLingDatabase The main object of the database.
   * @param pReferenceFlexions The flexions.
   */
  void copyFlexions
  (LinguisticIntermediaryDatabase& pLingDatabase,
   const ALLingdbFlexions* pReferenceFlexions);



  // Getters
  // -------

  /**
   * @brief Get the meaning of this wordform.
   * @return The meaning of this wordform.
   */
  ALLingdbMeaning* getMeaning() const;

  /**
   * @brief Get the flexions of this wordform.
   * @return The flexions of this wordform.
   */
  const ALLingdbFlexions* getFlexions() const;

  void setFrequency(char pFrequency);

  char getFrequency() const;


private:
  /// The meaning of this wordform.
  ALLingdbMeaning* fMeaning;
  /// The flexions of this wordform.
  ALLingdbFlexions* fFlexions;
  /// Frequency of this wordform in the language.
  char fFrequency;

private:
  friend class LinguisticIntermediaryDatabase;
  friend class ALLingdbDynamicTrieNode;
  template <typename T>
  friend struct ForwardPtrList;

  /**
   * @brief Get the position of the pointers for the allocator.
   * @param pRes The position of the pointers.
   * @param pVar An object of this class.
   */
  static void xGetPointers
  (std::vector<const void*>& pRes, void* pVar);

  /// Constructor.
  ALLingdbWordForms();

  /**
   * @brief Initialize the wordform.
   * @param pMeaning The meaning of this wordform.
   */
  void xInit
  (ALLingdbMeaning* pMeaning);

  /**
   * @brief Deallocate the wordform.
   * @param pFPAlloc The allocator.
   */
  void xDeallocate
  (ALCompositePoolAllocator& pFPAlloc);

  /**
   * @brief Add flexions to this wordform.
   * @param pFPAlloc The allocator.
   * @param pGram The grammatical type of the flexions.
   * @param pFlexions The flexions.
   */
  void xAddFlexions
  (ALCompositePoolAllocator& pFPAlloc,
   PartOfSpeech pGram,
   const std::vector<std::string>& pFlexions);

};


} // End of namespace onsem

#include "details/allingdbwordforms.hxx"

#endif // ALLINGDBWORDFORMS_H
