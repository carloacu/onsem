#ifndef ONSEM_COMPILERMODEL_LINGDBWORDFORMS_HPP
#define ONSEM_COMPILERMODEL_LINGDBWORDFORMS_HPP

#include <vector>
#include <string>
#include <onsem/common/enum/partofspeech.hpp>


namespace onsem
{
class CompositePoolAllocator;
class LingdbDynamicTrieNode;
template <typename T>
struct ForwardPtrList;
class LingdbMeaning;
class LingdbFlexions;
class LinguisticIntermediaryDatabase;

/**
 * @brief Class that hold a Word Form for a word.
 * A Word Form is a meaning and some flexions informations.
 */
class LingdbWordForms
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
   const LingdbFlexions* pReferenceFlexions);



  // Getters
  // -------

  /**
   * @brief Get the meaning of this wordform.
   * @return The meaning of this wordform.
   */
  LingdbMeaning* getMeaning() const;

  /**
   * @brief Get the flexions of this wordform.
   * @return The flexions of this wordform.
   */
  const LingdbFlexions* getFlexions() const;

  void setFrequency(char pFrequency);

  char getFrequency() const;


private:
  /// The meaning of this wordform.
  LingdbMeaning* fMeaning;
  /// The flexions of this wordform.
  LingdbFlexions* fFlexions;
  /// Frequency of this wordform in the language.
  char fFrequency;

private:
  friend class LinguisticIntermediaryDatabase;
  friend class LingdbDynamicTrieNode;
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
  LingdbWordForms();

  /**
   * @brief Initialize the wordform.
   * @param pMeaning The meaning of this wordform.
   */
  void xInit
  (LingdbMeaning* pMeaning);

  /**
   * @brief Deallocate the wordform.
   * @param pFPAlloc The allocator.
   */
  void xDeallocate
  (CompositePoolAllocator& pFPAlloc);

  /**
   * @brief Add flexions to this wordform.
   * @param pFPAlloc The allocator.
   * @param pGram The grammatical type of the flexions.
   * @param pFlexions The flexions.
   */
  void xAddFlexions
  (CompositePoolAllocator& pFPAlloc,
   PartOfSpeech pGram,
   const std::vector<std::string>& pFlexions);

};


} // End of namespace onsem

#include "details/lingdbwordforms.hxx"

#endif // ONSEM_COMPILERMODEL_LINGDBWORDFORMS_HPP
