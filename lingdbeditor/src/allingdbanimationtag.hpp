#ifndef ALLINGDBANIMATIONTAG_H
#define ALLINGDBANIMATIONTAG_H

#include <string>
#include <vector>
#include "concept/allingdblinktoaconcept.hpp"

namespace onsem
{
template <typename T>
struct ForwardPtrList;
class ALCompositePoolAllocator;
class ALLingdbString;
class ALLingdbMeaning;


class PonderatedMeaning
{
public:
  ALLingdbMeaning* meaning;
  char relation;

  void init(ALLingdbMeaning* pMeaning,
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


class ALLingdbAnimationsTag
{
public:
  const ALLingdbString* getTag() const
  { return fTag; }

  const ForwardPtrList<ALLingdbLinkToAConcept>* getLinksToConcept() const
  { return fLinksToConcept; }

  const ForwardPtrList<PonderatedMeaning>* getMeanings() const
  { return fMeanings; }

  bool isEmpty() const
  { return fLinksToConcept == nullptr && fMeanings == nullptr; }

  void addConcept
  (ALCompositePoolAllocator& pAlloc,
   ALLingdbConcept* pConcept,
   char pMinValue);

  void addMeaning
  (ALCompositePoolAllocator& pAlloc,
   ALLingdbMeaning* pMeaning,
   char pRelation);


private:
  ALLingdbString* fTag;
  ForwardPtrList<PonderatedMeaning>* fMeanings;
  ForwardPtrList<ALLingdbLinkToAConcept>* fLinksToConcept;


private:
  friend class LinguisticIntermediaryDatabase;
  friend class ALLingdbMeaning;

  /**
   * @brief Get the position of the pointers for the allocator.
   * @param pRes The position of the pointers.
   * @param pVar An object of this class.
   */
  static void xGetPointers
  (std::vector<const void*>& pRes, void* pVar);

  /// Constructor.
  ALLingdbAnimationsTag();

  /**
   * @brief Initialize the tag.
   * @param pFPAlloc The allocator.
   * @param pTagName The tag name.
   */
  void xInit
  (ALCompositePoolAllocator& pFPAlloc,
   const std::string& pTagName);

  /**
   * @brief Deallocate the tag.
   * @param pFPAlloc The allocator.
   */
  void xDeallocate
  (ALCompositePoolAllocator& pFPAlloc);
};


} // End of namespace onsem


#endif // ALLINGDBANIMATIONTAG_H
