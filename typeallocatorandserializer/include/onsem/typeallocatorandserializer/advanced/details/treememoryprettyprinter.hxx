#ifndef ONSEM_TYPEALLOCATORANDSERIALIZER_ADVANCED_DETAILS_TREEMEMORYPRETTYPRINTER_HXX
#define ONSEM_TYPEALLOCATORANDSERIALIZER_ADVANCED_DETAILS_TREEMEMORYPRETTYPRINTER_HXX

#include <onsem/common/utility/sizeprinterinbytes.hpp>
#include <onsem/typeallocatorandserializer/advanced/treememoryprettyprinter.hpp>
#include <onsem/typeallocatorandserializer/advanced/compositepoolallocator.hpp>

namespace onsem
{


inline ALTreeMemoryPrettyPrinter::ALTreeMemoryPrettyPrinter
(std::ostream& pOs)
  : fOs(pOs)
{
}


inline void ALTreeMemoryPrettyPrinter::operator()
(const ALCompositePoolAllocator& pC)
{
  fOs << "\"" << pC.getName() << "\" occSize: ";
  prettyPrintSizeNbInBytes(fOs, pC.getOccupatedSize());
  fOs << std::endl;
}


template <typename T>
inline void ALTreeMemoryPrettyPrinter::operator()
(const ALLeafPoolAllocator<T>& pL)
{
  std::size_t occSize = pL.getOccupatedSize();
  fOs << " -> \"" << pL.getName() << "\" occSize: ";
  prettyPrintSizeNbInBytes(fOs, occSize);
  fOs << " nbElts: " << occSize / pL.getSizeStructAligned()
      << std::endl;
}



} // End of namespace onsem

#endif // ONSEM_TYPEALLOCATORANDSERIALIZER_ADVANCED_DETAILS_TREEMEMORYPRETTYPRINTER_HXX
