#ifndef ALLINGDBCONCEPT_H
#define ALLINGDBCONCEPT_H

#include <string>
#include <vector>

namespace onsem
{
class ALCompositePoolAllocator;
class ALLingdbString;
template <typename T>
struct ForwardPtrList;


class ALLingdbConcept
{
public:
  const ALLingdbString* getName() const
  { return fName; }

  bool isAutoFill() const
  { return fAutoFill; }

  const ForwardPtrList<ALLingdbConcept>* getOppositeConcepts() const
  { return fOppositeConcepts; }

  const ForwardPtrList<ALLingdbConcept>* getNearlyEqualConcepts() const
  { return fNearlyEqualConcepts; }

  void addAnOppositeConcept
  (ALCompositePoolAllocator& pAlloc,
   ALLingdbConcept* pOppositeConcept);

  /**
   * @brief addANearlyEqualConcept Notify that if 2 words have these 2 concepts, we can assume the have nearly the same meaning.
   * @param pAlloc The allocator.
   * @param pNearlyEqualConcept The concept that is nearly equal to the one contained in this object.
   */
  void addANearlyEqualConcept
  (ALCompositePoolAllocator& pAlloc,
   ALLingdbConcept* pNearlyEqualConcept);

  static bool conceptNameFinishWithAStar
  (const std::string& pConceptName);

private:
  ALLingdbString* fName;
  ForwardPtrList<ALLingdbConcept>* fOppositeConcepts;
  ForwardPtrList<ALLingdbConcept>* fNearlyEqualConcepts;
  bool fAutoFill;

private:
  friend class LinguisticIntermediaryDatabase;
  friend class ALLingdbLinkToAConcept;

  /**
   * @brief Get the position of the pointers for the allocator.
   * @param pRes The position of the pointers.
   * @param pVar An object of this class.
   */
  static void xGetPointers
  (std::vector<const void*>& pRes, void* pVar);

  /// Constructor.
  ALLingdbConcept();


  void xInit
  (ALCompositePoolAllocator& pAlloc,
   const std::string& pName,
   bool pAutoFill);

  /**
   * @brief Deallocate the concept.
   * @param pFPAlloc The allocator.
   */
  void xDeallocate
  (ALCompositePoolAllocator& pAlloc);

};


} // End of namespace onsem


#endif // ALLINGDBCONCEPT_H
