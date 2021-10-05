#ifndef ONSEM_TYPEALLOCATORANDSERIALIZER_TYPEALLOCATORANDSERIALIZER_HPP
#define ONSEM_TYPEALLOCATORANDSERIALIZER_TYPEALLOCATORANDSERIALIZER_HPP

#include <string>
#include <vector>
#include <ostream>
#include <onsem/typeallocatorandserializer/advanced/treememoryprettyprinter.hpp>


namespace onsem
{
class ALCompositePoolAllocator;

/**
 * This class allow to allocate/deallocate some datas.\n
 * (This allocator will have a memory pool for each data type used)\n
 * \n
 * Condition for the usage of this allocator:\n
 * ------------------------------------------\n
 * -> Not allocate datas that allocate themselves other datas
 * with another allocator. (like std::string for exemple)\n
 * \n
 * How to use this allocator:\n
 * --------------------------\n
 * -> You have to declare each type you will use for allocate datas
 * with "declareANewType" function.\n
 * -> You have specify (with a static function) where are every pointers
 * (if exist) of every type you declare.\n
 * \n
 * Advantages of this allocator:\n
 * -----------------------------\n
 * -> Allow serialization/deserialization very quickly and in a very
 * compressed way.\n
 * -> Allow to list every datas allocated according to a specific type.\n
 * -> Allow to check if a pointer is valid.\n
 * -> Allow to print the state of the memory.\n
 * (space occupated in total, and for each data type the space occupated
 *  and the number of datas allocated).\n
 * -> Check that all pointers are valid during the serialization and the
 * deserialization.\n
 * -> Very explicit error feedbacks if a pointer is not valid or if you
 * allocate a data without having declared his type previously.
 */
class ALTypeAllocatorAndSerializer
{
public:

  /**
   * @brief Constructor.
   * @param pName Name of this allocator.
   */
  ALTypeAllocatorAndSerializer(const std::string& pName);


  /// Type of the function used to get the pointer(s) inside a type.
  typedef void(*get_pointers)(std::vector<const void*>&, void*);

  /**
   * @brief Declare a new type that will maybe be used to allocate datas.
   * @param pName Name of the new leaf.
   * @param pMemoryAlignment Memory alignement of the elements of this type.
   * @param pFunc Function to get the pointers inside the type.
   */
  template<typename T>
  void declareANewType
  (const std::string& pName,
   unsigned char pMemoryAlignment,
   get_pointers pFunc = nullptr);


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
   * @brief Know if a pointer is allocated.
   * @param pPtr The pointer we want to check.
   * @return True if the pointer is allocated, False otherwise.
   */
  template<typename T>
  bool isAllocated(T* pPtr) const;


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
   * @brief Serialize the memory.
   * @param pFilename The name of the file where we will
   * store the serialization.
   */
  void serialize(const std::string& pFilename);


  /**
   * @brief Serialize the memory and clear it.
   * (we merge this 2 things for optimisation purpose)
   * @param pFilename The name of the file in which we want put our serialization.
   */
  void serializeAndClear(const std::string& pFilename);


  /**
   * @brief Deserialize the input stream in the memory.
   * @param pFilename The name of the serialized file.
   * @param pCoef Coefficient that represent how much memory space
   * we reserve relatively to the size of the stream.
   */
  void deserialize
  (std::string& pErrorMessage,
   const std::string& pFilename, float pCoef = 1.2f);


  /**
   * @brief Remove all allocated elements.
   */
  void clear();


  /**
   * @brief Get the name of the allocator.
   * @return The name of the allocator.
   */
  const std::string& getName() const;


  /**
   * @brief Get the name of the allocator.
   * @return The name of the allocator.
   */
  void setName(std::string pName);


  /**
   * @brief Get the occupated size in memory.
   * @return The occupated size in memory in byte.
   */
  std::size_t getOccupatedSize() const;


  /**
   * @brief Get the reserved size in memory.
   * @return The reserved size in memory in byte.
   */
  std::size_t getTotalSize() const;


  /**
   * @brief Reserve a fixed number of elements in memory.
   * @param pNbElts Number of elements we want to reserve in memory.
   */
  template<typename T>
  void reserve(std::size_t pNbElts);

  /**
   * @brief Get the composite allocator hold by this class.
   * @return The composite allocator hold by this class.
   */
  const ALCompositePoolAllocator* getCompositePoolAllocator() const;

private:
  /// The composite pool allocator.
  ALCompositePoolAllocator* fCompPoolAllocator;
};


/**
 * @brief Pretty print a "ALTypeAllocatorAndSerializer".
 * @param pOs The stream.
 * @param pFPA The "ALTypeAllocatorAndSerializer" we want to print.
 * @return The stream in result.
 */
inline std::ostream& operator<<
(std::ostream& pOs, const ALTypeAllocatorAndSerializer& pFPA)
{
  ALTreeMemoryPrettyPrinter printer(pOs);
  pFPA.getCompositePoolAllocator()->accept(printer);
  return pOs;
}

} // End of namespace onsem

#include "details/typeallocatorandserializer.hxx"


#endif // ONSEM_TYPEALLOCATORANDSERIALIZER_TYPEALLOCATORANDSERIALIZER_HPP
