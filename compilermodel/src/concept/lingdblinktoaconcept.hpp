#ifndef ONSEM_COMPILERMODEL_SRC_CONCEPT_LINGDBLINKTOACONCEPT_HPP
#define ONSEM_COMPILERMODEL_SRC_CONCEPT_LINGDBLINKTOACONCEPT_HPP

#include <string>
#include <vector>

namespace onsem
{
class CompositePoolAllocator;
class LingdbConcept;


class LingdbLinkToAConcept
{
public:
  const LingdbConcept* getConcept() const
  { return fConcept; }

  char getRelatedToConcept() const
  { return fRelatedToConcept; }

  void setConcept
  (const LingdbConcept* pConcept)
  { fConcept = pConcept; }

  void setRelatedToConcept
  (char pRelatedToConcept)
  { fRelatedToConcept = pRelatedToConcept; }

  bool operator<
  (const LingdbLinkToAConcept& pOther) const;

private:
  const LingdbConcept* fConcept;
  char fRelatedToConcept;

private:
  friend class LinguisticIntermediaryDatabase;
  friend class LingdbMeaning;
  friend class LingdbAnimationsTag;
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
  LingdbLinkToAConcept();

  void xInit
  (const LingdbConcept* pConcept,
   char pRelatedToConcept);

};


} // End of namespace onsem


#endif // ONSEM_COMPILERMODEL_SRC_CONCEPT_LINGDBLINKTOACONCEPT_HPP
