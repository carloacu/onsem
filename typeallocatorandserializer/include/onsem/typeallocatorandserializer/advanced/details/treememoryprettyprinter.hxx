#ifndef ONSEM_TYPEALLOCATORANDSERIALIZER_ADVANCED_DETAILS_TREEMEMORYPRETTYPRINTER_HXX
#define ONSEM_TYPEALLOCATORANDSERIALIZER_ADVANCED_DETAILS_TREEMEMORYPRETTYPRINTER_HXX

#include <onsem/common/utility/sizeprinterinbytes.hpp>
#include <onsem/typeallocatorandserializer/advanced/treememoryprettyprinter.hpp>
#include <onsem/typeallocatorandserializer/advanced/compositepoolallocator.hpp>

namespace onsem
{


inline TreeMemoryPrettyPrinter::TreeMemoryPrettyPrinter
(std::ostream& pOs)
  : fOs(pOs)
{
}


inline void TreeMemoryPrettyPrinter::operator()
(const CompositePoolAllocator& pC)
{
  fOs << "\"" << pC.getName() << "\" occSize: ";
  prettyPrintSizeNbInBytes(fOs, pC.getOccupatedSize());
  fOs << std::endl;
}


template <typename T>
inline void TreeMemoryPrettyPrinter::operator()
(const LeafPoolAllocator<T>& pL)
{
  std::size_t occSize = pL.getOccupatedSize();
  fOs << " -> \"" << pL.getName() << "\" occSize: ";
  prettyPrintSizeNbInBytes(fOs, occSize);
  fOs << " nbElts: " << occSize / pL.getSizeStructAligned()
      << std::endl;
}



} // End of namespace onsem

#endif // ONSEM_TYPEALLOCATORANDSERIALIZER_ADVANCED_DETAILS_TREEMEMORYPRETTYPRINTER_HXX
