#ifndef ONSEM_TYPEALLOCATORANDSERIALIZER_ADVANCED_COMPONENTPOOLALLOCATOR_HPP
#define	ONSEM_TYPEALLOCATORANDSERIALIZER_ADVANCED_COMPONENTPOOLALLOCATOR_HPP

#include <string>
#include <set>
#include <boost/filesystem/path.hpp>


namespace onsem
{
class ALTreeMemoryPrettyPrinter;
class ALCompositePoolAllocator;

/// Allocator for pooled memory allocation
class ALComponentPoolAllocator
{
public:
  /**
   * @brief Constructor.
   * @param name Name of the pool allocator.
   */
  ALComponentPoolAllocator(const boost::filesystem::path& name);


  /**
   * @brief Virtual destructor.
   * (because this is a virtual class)
   */
  virtual ~ALComponentPoolAllocator()
  {}


  /**
   * @brief Get the name of the pool allocator.
   * @return The name of the pool allocator.
   */
  const std::string& getName() const
  { return fName; }


  /**
   * @brief Set the name of the pool allocator.
   * @param pName The new name of the pool allocator.
   */
  void setName(std::string pName)
  { fName = pName; }


  /**
   * @brief Serialize the memory.
   * @param pFilename The name of the file where we will
   * store the serialization.
   */
  void serialize(const std::string& pFilename);


  /**
   * @brief Serialize the memory and clear it.
   * (we merge this 2 thing for optimisation purpose)
   * @param pFilename The name of the file in which we want to serialize.
   */
  void serializeAndClear(const std::string& pFilename);


  /**
   * @brief Deserialize the input stream in the memory.
   * @param pFilename The name of the serialized file.
   * @param pCoef Coefficient that represent how much memory space
   * we reserve relatively to the size of the stream.
   * @return If the deserialization has succeeded.
   */
  void deserialize
  (std::string& pErrorMessage,
   const std::string& pFilename, float pCoef = 1.2f);


  /**
   * @brief Remove all allocated elements.
   */
  virtual void clear() = 0;


  /**
   * @brief Get the occupated size in memory.
   * @return The occupated size in memory in byte.
   */
  virtual std::size_t getOccupatedSize() const = 0;


  /**
   * @brief Get the reserved size in memory.
   * @return The reserved size in memory in byte.
   */
  virtual std::size_t getTotalSize() const = 0;


  /**
   * @brief Accept the pretty printer visitor.
   * @param pV The pretty printer visitor.
   */
  virtual void accept(ALTreeMemoryPrettyPrinter& pV) const = 0;


  friend class ALCompositePoolAllocator;
protected:
  /// Union that hold a pointer
  union FPAPtr
  {
    void* ptr;
    char* pchar;
    std::size_t val;
    /// Constructor
    FPAPtr()
    {
      ptr = nullptr;
    }
    /**
     * @brief Constructor
     * @param p The pointer to initialize the union.
     */
    FPAPtr(void* p)
    {
      ptr = p;
    }
    /**
     * @brief If the num is different to another.
     * @param b The other enum.
     * @return True if the 2 nums are different, False otherwise.
     */
    bool operator!=(const FPAPtr& b) const
    {
      return ptr != b.ptr;
    }
    /**
     * @brief If the num is lesser than another.
     * @param b The other enum.
     * @return True if the enum is lesser than another, False otherwise.
     */
    bool operator<(const FPAPtr& b) const
    {
      return ptr < b.ptr;
    }
    /**
     * @brief If the num is greater than another.
     * @param b The other enum.
     * @return True if the enum is greater than another, False otherwise.
     */
    bool operator>(const FPAPtr& b) const
    {
      return ptr > b.ptr;
    }
    /**
     * @brief If the num is lesser or equal than another.
     * @param b The other enum.
     * @return True if the enum is lesser or equal than another,
     * False otherwise.
     */
    bool operator<=(const FPAPtr& b) const
    {
      return ptr <= b.ptr;
    }
    /**
     * @brief If the num is greater or equal than another.
     * @param b The other enum.
     * @return True if the enum is greater or equal than another,
     * False otherwise.
     */
    bool operator>=(const FPAPtr& b) const
    {
      return ptr >= b.ptr;
    }
  };

  /// Struct that map a memory zone to another memory zone.
  struct FPAShift
  {
    /// Constructor.
    FPAShift()
      : begin(),
        end(),
        new_begin()
    {}
    /**
     * @brief Comparaison with another FPAShift.
     * (The order is the inverse of the pointer order
     *  because this is the only way get the zone of a
     *  pointer using lower_bound standard function)
     * @param b The other FPAShift.
     * @return True if the second FPAShift is lesser.
     */
    bool operator< (const FPAShift& b) const {
      return begin > b.begin;
    }
    /// Begin of the initial memory zone. (in the memory zone)
    FPAPtr begin;
    /// End of the initial memory zone. (out of the memory zone)
    FPAPtr end;
    /// New begin of the final memory zone. (in the memory zone)
    FPAPtr new_begin;
  };

  /// The name of the pool allocator.
  std::string fName;


  /**
   * @brief Serialize the pool allocator.
   * @param[out] The shifts used to fragment the memory before
   * the serialization.
   * @param pFilename The name of the file where we will
   * store the serialization.
   */
  void xSerialize
  (std::set<FPAShift>& pShifts,
   const std::string& pFilename);

  /**
   * @brief Write the memory in a stream.
   * @param pOstr The stream.
   */
  virtual void xWriteMemoryInAStream
  (std::ostream& pOstr) const = 0;

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
   float pCoef) = 0;

  /**
   * @brief Find shifts in order defragment the memory.
   * @param[out] pShifts Shifts in order defragment the memory.
   */
  virtual void xFindShiftsForDefragmentation
  (std::set<FPAShift>& pShifts) const = 0;

  /**
   * @brief Change the pointers according to the given shifts.
   * @param pShifts Memory pool shifts.
   */
  virtual void xChangePointers
  (const std::set<FPAShift>& pShifts) = 0;

  /**
   * @brief Defragment the memory according to the given shifts.
   * @param pShifts Memory pool shifts.
   */
  void xDefragmentMemory
  (const std::set<FPAShift>& pShifts);

  /**
   * @brief Fragment the memory according to the given shifts.
   * @param pShifts Memory pool shifts.
   */
  void xFragmentMemory
  (const std::set<FPAShift>& pShifts);


private:
  /**
   * @brief To invert the shifts.
   * (initial memory zone become final memory zone and
   *  final memory zone become initial memory zone)
   * @param[out] pNewShifts Inverted shifts.
   * @param pPrevShifts Initial shifts.
   */
  void xInvertShifts
  (std::set<FPAShift>& pNewShifts,
   const std::set<FPAShift>& pPrevShifts) const;
};


} // End of namespace onsem

#endif // ONSEM_TYPEALLOCATORANDSERIALIZER_ADVANCED_COMPONENTPOOLALLOCATOR_HPP
