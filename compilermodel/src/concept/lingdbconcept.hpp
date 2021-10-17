#ifndef ONSEM_COMPILERMODEL_SRC_CONCEPT_LINGDBCONCEPT_HPP
#define ONSEM_COMPILERMODEL_SRC_CONCEPT_LINGDBCONCEPT_HPP

#include <string>
#include <vector>

namespace onsem
{
class CompositePoolAllocator;
class LingdbString;
template <typename T>
struct ForwardPtrList;


class LingdbConcept
{
public:
  const LingdbString* getName() const
  { return fName; }

  bool isAutoFill() const
  { return fAutoFill; }

  const ForwardPtrList<LingdbConcept>* getOppositeConcepts() const
  { return fOppositeConcepts; }

  const ForwardPtrList<LingdbConcept>* getNearlyEqualConcepts() const
  { return fNearlyEqualConcepts; }

  void addAnOppositeConcept
  (CompositePoolAllocator& pAlloc,
   LingdbConcept* pOppositeConcept);

  /**
   * @brief addANearlyEqualConcept Notify that if 2 words have these 2 concepts, we can assume the have nearly the same meaning.
   * @param pAlloc The allocator.
   * @param pNearlyEqualConcept The concept that is nearly equal to the one contained in this object.
   */
  void addANearlyEqualConcept
  (CompositePoolAllocator& pAlloc,
   LingdbConcept* pNearlyEqualConcept);

  static bool conceptNameFinishWithAStar
  (const std::string& pConceptName);

private:
  LingdbString* fName;
  ForwardPtrList<LingdbConcept>* fOppositeConcepts;
  ForwardPtrList<LingdbConcept>* fNearlyEqualConcepts;
  bool fAutoFill;

private:
  friend class LinguisticIntermediaryDatabase;
  //friend class ALLingdbLinkToAConcept;

  /**
   * @brief Get the position of the pointers for the allocator.
   * @param pRes The position of the pointers.
   * @param pVar An object of this class.
   */
  static void xGetPointers
  (std::vector<const void*>& pRes, void* pVar);

  /// Constructor.
  LingdbConcept();


  void xInit
  (CompositePoolAllocator& pAlloc,
   const std::string& pName,
   bool pAutoFill);

  /**
   * @brief Deallocate the concept.
   * @param pFPAlloc The allocator.
   */
  void xDeallocate
  (CompositePoolAllocator& pAlloc);

};


} // End of namespace onsem


#endif // ONSEM_COMPILERMODEL_SRC_CONCEPT_LINGDBCONCEPT_HPP
