#ifndef ONSEM_TYPEALLOCATORANDSERIALIZER_ADVANCED_LEAFPOOLALLOCATOR_HPP
#define ONSEM_TYPEALLOCATORANDSERIALIZER_ADVANCED_LEAFPOOLALLOCATOR_HPP

#include <vector>
#include <onsem/typeallocatorandserializer/advanced/componentpoolallocator.hpp>

namespace onsem
{
class TreeMemoryPrettyPrinter;

/// Pool allocator for a single data type.
template <typename T>
class LeafPoolAllocator : public ComponentPoolAllocator
{
public:

  /// Type of the function used to get the pointer(s) inside a type.
  typedef void(*get_pointers)(std::vector<const void*>&, void*);


  /**
   * @brief Construct a leaf pool allocator.
   * @param pName Name of the leaf pool allocator.
   * @param pMemoryAlignment Memory alignement of the elements.
   * @param pFunc Function to get the pointers inside the type of the elements.
   */
  LeafPoolAllocator
  (const std::string& pName,
   unsigned char pMemoryAlignment,
   get_pointers pFunc = nullptr);


  /**
   * @brief Reserve a fixed number of elements in memory.
   * @param pNbElts Nb elts we want to reserve in memory.
   */
  virtual void reserve(std::size_t pNbElts);


  /**
   * @brief Allocate a fixed number of elements.
   * @param pNbElts Nb elts we want to allocate.
   */
  virtual T* allocate(std::size_t pNbElts);

  /**
   * @brief Deallocate a fixed number of elements.
   * @param pPointer Pointer of the first elt to deallocate.
   * @param pNbElts Nb elts we want to deallocate.
   */
  virtual void deallocate(T* pPointer, std::size_t pNbElts = 1);


  /**
   * @brief Remove all allocated elements.
   */
  virtual void clear();


  /**
   * @brief Access to the first element stored in memory.
   */
  virtual T* first() const;


  /**
   * @brief Access to the next element stored in memory.
   * @param pElt Current element.
   */
  virtual T* next(T* pElt) const;


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
   * @brief Get the size of an element in memory (with the memory alignement at it's end).
   * @return The size of an element in memory.
   */
  std::size_t getSizeStructAligned() const;


  /**
   * @brief Know if a pointer is allocated.
   * @param pPtr The pointer we want to check.
   * @return True if the pointer is allocated, False otherwise.
   */
  bool isAllocated(T* pPtr) const;


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
   * @param pCoef Coefficient that represent how much
   * memory space we reserve relatively to the size
   * of the stream.
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
  /// Struct of a memory pool.
  struct FPAPool
  {
    /// Constructor.
    FPAPool()
      : mem(),
        newAlloc(),
        maxSize(0),
        deallocated()
    {}
    /// Pointer at the begin of the pool. (in the memory zone)
    FPAPtr mem;
    /// Pointer at the end of the pool. (out of the memory zone)
    FPAPtr newAlloc;
    /// Allocated size for this pool. (in bytes)
    std::size_t maxSize;
    /// All deallocated elements of the pool.
    std::set<FPAPtr> deallocated;
  };

  /**
   * @brief Structure use in the serialization step
   * in order to indicate where was allocated a
   * memory pool.\n
   * Because during the deserialization step we have
   * to know where was allocated the memory pools in
   * order to adapt the pointers to the new allocations.
   */
  struct FPASerialPool
  {
    /// Constructor.
    FPASerialPool()
      : begin(),
        end()
    {}
    /// Pointer at the begin of the pool. (in the memory zone)
    FPAPtr begin;
    /// Pointer at the end of the pool. (out of the memory zone)
    FPAPtr end;
  };

  /// Memory alignement of the element in memory.
  const unsigned char fMemoryAlignment;
  /// Size occupated by each element according to the memory alignement.
  const std::size_t fSizeStructAligned;
  /// Function to get the pointers in an element.
  get_pointers fFunc;
  /// Memory pools of elements.
  std::vector<FPAPool> fPools;


  /**
   * @brief Calculate the Size occupated by each element
   * according to the memory alignement.
   * @param pMemoryAlignment The memory alignement.
   * @return The Size occupated by each element.
   */
  static std::size_t xCalculateSizeStructAligned
  (unsigned char pMemoryAlignment);

  /**
   * @brief Get the first element allocated in the memory.
   * @return A pointer to the first element allocated in the memory.
   */
  void* xGetFirstAlloc() const;

  /**
   * @brief Get the next element allocated in the memory.
   * @param pPtr The current element.
   * @return A pointer to the next element allocated in the memory.
   */
  void* xGetNext(FPAPtr pPtr) const;

  /**
   * @brief Get the pool of a pointer.
   * @param pPtr The pointer.
   * @return The id of the pool of the pointer.
   */
  std::size_t xGetPool(FPAPtr pPtr) const;

  /**
   * @brief Get the occupated size of a memory pool.
   * @param pPool The memory pool.
   * @return The occupated size of a memory pool in byte.
   */
  std::size_t xGetPoolOccupatedSize(const FPAPool& pPool) const;

  /**
   * @brief Modify a pointer according to the given shifts.
   * @param[out] pRefPool Keep an iterator to the pool memory used for this pointer.
   * (This is for optimisation because a each pointer that has the same position
   *  usaly point to the same pool memory)
   * @param pPtr The pointer that we want to modify.
   * @param pShifts The shifts.
   * @return The new value of the pointer.
   */
  std::size_t xModifyAPtr
  (std::set<FPAShift>::const_iterator& pRefPool,
   FPAPtr pPtr,
   const std::set<FPAShift>& pShifts);

};


} // End of namespace onsem

#include "details/leafpoolallocator.hxx"

#endif // ONSEM_TYPEALLOCATORANDSERIALIZER_ADVANCED_LEAFPOOLALLOCATOR_HPP
