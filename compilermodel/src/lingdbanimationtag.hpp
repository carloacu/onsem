#ifndef ONSEM_COMPILERMODEL_SRC_LINGDBANIMATIONTAG_HPP
#define ONSEM_COMPILERMODEL_SRC_LINGDBANIMATIONTAG_HPP

#include <string>
#include <vector>
#include "concept/lingdblinktoaconcept.hpp"

namespace onsem
{
template <typename T>
struct ForwardPtrList;
class CompositePoolAllocator;
class LingdbString;
class LingdbMeaning;


class PonderatedMeaning
{
public:
  LingdbMeaning* meaning;
  char relation;

  void init(LingdbMeaning* pMeaning,
            char pRelation);


private:
  friend class LinguisticIntermediaryDatabase;

  static void xGetPointers
  (std::vector<const void*>& pRes, void* pVar)
  {
    pRes.emplace_back(&reinterpret_cast<PonderatedMeaning*>
                   (pVar)->meaning);
  }
};


class LingdbAnimationsTag
{
public:
  const LingdbString* getTag() const
  { return fTag; }

  const ForwardPtrList<LingdbLinkToAConcept>* getLinksToConcept() const
  { return fLinksToConcept; }

  const ForwardPtrList<PonderatedMeaning>* getMeanings() const
  { return fMeanings; }

  bool isEmpty() const
  { return fLinksToConcept == nullptr && fMeanings == nullptr; }

  void addConcept
  (CompositePoolAllocator& pAlloc,
   LingdbConcept* pConcept,
   char pMinValue);

  void addMeaning
  (CompositePoolAllocator& pAlloc,
   LingdbMeaning* pMeaning,
   char pRelation);


private:
  LingdbString* fTag;
  ForwardPtrList<PonderatedMeaning>* fMeanings;
  ForwardPtrList<LingdbLinkToAConcept>* fLinksToConcept;


private:
  friend class LinguisticIntermediaryDatabase;
  friend class LingdbMeaning;

  /**
   * @brief Get the position of the pointers for the allocator.
   * @param pRes The position of the pointers.
   * @param pVar An object of this class.
   */
  static void xGetPointers
  (std::vector<const void*>& pRes, void* pVar);

  /// Constructor.
  LingdbAnimationsTag();

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


#endif // ONSEM_COMPILERMODEL_SRC_LINGDBANIMATIONTAG_HPP
