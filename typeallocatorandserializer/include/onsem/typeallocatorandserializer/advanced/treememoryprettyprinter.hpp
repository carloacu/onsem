#ifndef ONSEM_TYPEALLOCATORANDSERIALIZER_ADVANCED_TREEMEMORYPRETTYPRINTER_HPP
#define ONSEM_TYPEALLOCATORANDSERIALIZER_ADVANCED_TREEMEMORYPRETTYPRINTER_HPP

#include <string>
#include <iostream>


namespace onsem
{
class ALCompositePoolAllocator;
template <typename T>
class ALLeafPoolAllocator;


class ALTreeMemoryPrettyPrinter
{
public:
  /**
   * @brief Constructor.
   * @param pOs Stream where we will print the memory.
   */
  ALTreeMemoryPrettyPrinter(std::ostream& pOs);

  /**
   * @brief Print a composite pool allocator object.
   * @param pC The composite pool allocator object to print.
   */
  void operator() (const ALCompositePoolAllocator& pC);


  /**
   * @brief Print a leaf pool allocator object.
   * @param pL The leaf pool allocator object to print.
   */
  template <typename T>
  void operator() (const ALLeafPoolAllocator<T>& pL);


private:
  /// Stream where we print the memory.
  std::ostream& fOs;
};


} // End of namespace onsem

#include "details/treememoryprettyprinter.hxx"

#endif // ONSEM_TYPEALLOCATORANDSERIALIZER_ADVANCED_TREEMEMORYPRETTYPRINTER_HPP
