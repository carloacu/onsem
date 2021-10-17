#ifndef ONSEM_COMPILERMODEL_LINGDBSTRING_HPP
#define ONSEM_COMPILERMODEL_LINGDBSTRING_HPP

#include <string>
#include <vector>


namespace onsem
{
class CompositePoolAllocator;


/// Class that hold a tag in the database.
class LingdbString
{
public:

  // Getters
  // -------

  /**
   * @brief Get the tag name.
   * @return A string that contains the tag name.
   */
  std::string toStr() const;

  /**
   * @brief Get the tag name in char array.
   * @return A char array that contains the tag name.
   */
  char* toCStr() const;

  /**
   * @brief Get the length of the tag name.
   * @return The length of the tag name.
   */
  unsigned char length() const;



private:
  /// Number of characters of the tag name.
  unsigned char fNbCharacters;
  /// Characters of the tag name.
  char* fCharacters;


private:
  friend class LinguisticIntermediaryDatabase;
  friend class LingdbAnimationsTag;
  friend class LingdbQuestionWords;
  friend class LingdbConcept;

  /**
   * @brief Get the position of the pointers for the allocator.
   * @param pRes The position of the pointers.
   * @param pVar An object of this class.
   */
  static void xGetPointers
  (std::vector<const void*>& pRes, void* pVar);

  /// Constructor.
  LingdbString();

  /**
   * @brief Initialize the tag.
   * @param pFPAlloc The allocator.
   * @param pTagName The tag name.
   */
  void xInit
  (CompositePoolAllocator& pFPAlloc,
   const std::string& pTagName);

  /**
   * @brief Deallocate the tag.
   * @param pFPAlloc The allocator.
   */
  void xDeallocate
  (CompositePoolAllocator& pFPAlloc);
};


} // End of namespace onsem


#endif // ONSEM_COMPILERMODEL_LINGDBSTRING_HPP
