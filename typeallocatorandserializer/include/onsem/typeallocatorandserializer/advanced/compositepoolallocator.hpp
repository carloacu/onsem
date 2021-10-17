#ifndef ONSEM_TYPEALLOCATORANDSERIALIZER_ADVANCED_COMPOSITEPOOLALLOCATOR_HPP
#define ONSEM_TYPEALLOCATORANDSERIALIZER_ADVANCED_COMPOSITEPOOLALLOCATOR_HPP

#include <vector>
#include <string>
#include <set>
#include <onsem/typeallocatorandserializer/advanced/componentpoolallocator.hpp>

namespace onsem
{
class TreeMemoryPrettyPrinter;
template<typename T>
class LeafPoolAllocator;

/// Class that hold 0 or many pool allocators
class CompositePoolAllocator : public ComponentPoolAllocator
{
public:

  /**
   * @brief Constructor.
   * @param name Name of this composite pool allocator.
   */
  CompositePoolAllocator(const std::string& pName);


  /**
   * @brief Reserve a fixed number of elements in memory.
   * @param pNbElts Nb elts we want to reserve in memory.
   */
  template<typename T>
  void reserve(std::size_t pNbElts);


  /**
   * @brief Allocate a fixed number of elements.
   * @param pNbElts Number of elements we want to allocate.
   */
  template<typename T>
  T* allocate(std::size_t pNbElts);


  /**
   * @brief Deallocate a fixed number of elements.
   * @param pPointer Pointer of the first elt to deallocate.
   * @param pNbElts Nb elts we want to deallocate.
   */
  template<typename T>
  void deallocate(T* pPointer, std::size_t pNbElts = 1);


  /**
   * @brief Remove all allocated elements.
   */
  virtual void clear();


  /**
   * @brief Access to the first element
   * of a specific type stored in memory.
   */
  template<typename T>
  T* first() const;


  /**
   * @brief Access to the next element
   * of a specific type stored in memory.
   * @param pElt Current element.
   */
  template<typename T>
  T* next(T* pElt) const;


  /**
   * @brief Get the occupated size in memory.
   * @return The occupated size in memory in byte.
   */
  virtual std::size_t getOccupatedSize() const;


  /**
   * @brief Get the reserved size in memory.
   * @return The reserved size in memory in byte.
   */
  virtual std::size_t getTotalSize() const;


  /**
   * @brief Know if a pointer is allocated.
   * @param pPtr The pointer we want to check.
   * @return True if the pointer is allocated, False otherwise.
   */
  template<typename T>
  bool isAllocated(T* pPtr) const;


  /**
   * @brief Add a new sub-composite.
   * @param pName Name of the new subcomposite.
   */
  void addANewComposite(const std::string& pName);


  CompositePoolAllocator* getComposite(const std::string& pName) const;


  void getComposites
  (std::vector<CompositePoolAllocator*>& pComposites);


  /// Type of the function used to get the pointer(s) inside a type.
  typedef void(*get_pointers)(std::vector<const void*>&, void*);


  /**
   * @brief Add a new leaf pool allocator in this composite.
   * @param pName Name of the new leaf.
   * @param pMemoryAlignment Memory alignement of the elements in the new leaf.
   * @param pFunc Function to get the pointers inside the type.
   */
  template<typename T>
  void addANewLeaf
  (const std::string& pName,
   unsigned char pMemoryAlignment,
   get_pointers pFunc = nullptr);


  /**
   * @brief Accept the pretty printer visitor.
   * @param pV The pretty printer visitor.
   */
  virtual void accept
  (TreeMemoryPrettyPrinter& pV) const;





protected:
  /**
   * @brief Write the memory in a stream.
   * @param pOstr The stream.
   */
  virtual void xWriteMemoryInAStream
  (std::ostream& pOstr) const;

  /**
   * @brief Read the memory from a stream.
   * @param[out] pShifts Shifts of the memory pools between
   * the precedent serialization and the new allocation.
   * @param pOstr The stream.
   * @param pCoef Coefficient that represent how much memory space
   * we reserve relatively to the size of the stream.
   */
  virtual void xReadMemoryFromAStream
  (std::set<FPAShift>& pShifts,
   std::istream& pIstr,
   float pCoef);

  /**
   * @brief Find shifts in order defragment the memory.
   * @param[out] pShifts Shifts in order defragment the memory.
   */
  virtual void xFindShiftsForDefragmentation
  (std::set<FPAShift>& pShifts) const;

  /**
   * @brief Change the pointers according to the given shifts.
   * @param pShifts Memory pool shifts.
   */
  virtual void xChangePointers
  (const std::set<FPAShift>& pShifts);


private:
  /// Every pool allocators hold in this composite.
  std::vector<ComponentPoolAllocator*> fPoolAllocators;

  /**
   * @brief Get the leaf corresponding of a type.
   * @return The leaf corresponding of a type.
   */
  template<typename T>
  LeafPoolAllocator<T>* xGetLeaf() const;
};


std::ostream& operator<<(std::ostream& pOs, const CompositePoolAllocator& pFPA);


} // End of namespace onsem

#include "details/compositepoolallocator.hxx"

#endif // ONSEM_TYPEALLOCATORANDSERIALIZER_ADVANCED_COMPOSITEPOOLALLOCATOR_HPP
