#ifndef ALLINGDBLINKTOACONCEPT_H
#define ALLINGDBLINKTOACONCEPT_H

#include <string>
#include <vector>

namespace onsem
{
class ALCompositePoolAllocator;
class ALLingdbConcept;


class ALLingdbLinkToAConcept
{
public:
  const ALLingdbConcept* getConcept() const
  { return fConcept; }

  char getRelatedToConcept() const
  { return fRelatedToConcept; }

  void setConcept
  (const ALLingdbConcept* pConcept)
  { fConcept = pConcept; }

  void setRelatedToConcept
  (char pRelatedToConcept)
  { fRelatedToConcept = pRelatedToConcept; }

  bool operator<
  (const ALLingdbLinkToAConcept& pOther) const;

private:
  const ALLingdbConcept* fConcept;
  char fRelatedToConcept;

private:
  friend class LinguisticIntermediaryDatabase;
  friend class ALLingdbMeaning;
  friend class ALLingdbAnimationsTag;
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
  ALLingdbLinkToAConcept();

  void xInit
  (const ALLingdbConcept* pConcept,
   char pRelatedToConcept);

};


} // End of namespace onsem


#endif // ALLINGDBLINKTOACONCEPT_H
