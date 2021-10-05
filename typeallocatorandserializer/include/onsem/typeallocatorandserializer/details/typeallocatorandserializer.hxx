#ifndef ONSEM_TYPEALLOCATORANDSERIALIZER_DETAILS_TYPEALLOCATORANDSERIALIZER_HXX
#define ONSEM_TYPEALLOCATORANDSERIALIZER_DETAILS_TYPEALLOCATORANDSERIALIZER_HXX

#include <onsem/typeallocatorandserializer/typeallocatorandserializer.hpp>
#include <onsem/typeallocatorandserializer/advanced/compositepoolallocator.hpp>

namespace onsem
{

inline ALTypeAllocatorAndSerializer::ALTypeAllocatorAndSerializer
(const std::string& pName)
  : fCompPoolAllocator(new ALCompositePoolAllocator(pName))
{
}

template<typename T>
inline void ALTypeAllocatorAndSerializer::declareANewType
(const std::string& pName,
 unsigned char pMemoryAlignment,
 get_pointers pFunc)
{
  fCompPoolAllocator->addANewLeaf<T>(pName, pMemoryAlignment, pFunc);
}

template<typename T>
inline T* ALTypeAllocatorAndSerializer::allocate
(std::size_t pNbElts)
{
  return fCompPoolAllocator->allocate<T>(pNbElts);
}

template<typename T>
inline void ALTypeAllocatorAndSerializer::deallocate
(T* pPointer, std::size_t pNbElts)
{
  fCompPoolAllocator->deallocate<T>(pPointer, pNbElts);
}

template<typename T>
bool ALTypeAllocatorAndSerializer::isAllocated
(T* pPtr) const
{
  return fCompPoolAllocator->isAllocated<T>(pPtr);
}

template<typename T>
inline T* ALTypeAllocatorAndSerializer::first
() const
{
  return fCompPoolAllocator->first<T>();
}

template<typename T>
inline T* ALTypeAllocatorAndSerializer::next
(T* pElt) const
{
  return fCompPoolAllocator->next<T>(pElt);
}

inline void ALTypeAllocatorAndSerializer::serialize
(const std::string& pFilename)
{
  fCompPoolAllocator->serialize(pFilename);
}

inline void ALTypeAllocatorAndSerializer::serializeAndClear
(const std::string& pFilename)
{
  fCompPoolAllocator->serializeAndClear(pFilename);
}

inline void ALTypeAllocatorAndSerializer::deserialize
(std::string& pErrorMessage,
 const std::string& pFilename, float pCoef)
{
  fCompPoolAllocator->deserialize
      (pErrorMessage, pFilename, pCoef);
}

inline void ALTypeAllocatorAndSerializer::clear
()
{
  fCompPoolAllocator->clear();
}

inline const std::string& ALTypeAllocatorAndSerializer::getName
() const
{
  return fCompPoolAllocator->getName();
}

inline std::size_t ALTypeAllocatorAndSerializer::getOccupatedSize
() const
{
  return fCompPoolAllocator->getOccupatedSize();
}

inline std::size_t ALTypeAllocatorAndSerializer::getTotalSize
() const
{
  return fCompPoolAllocator->getTotalSize();
}

inline void ALTypeAllocatorAndSerializer::setName
(std::string pName)
{
  fCompPoolAllocator->setName(pName);
}

template<typename T>
inline void ALTypeAllocatorAndSerializer::reserve
(std::size_t pNbElts)
{
  fCompPoolAllocator->reserve<T>(pNbElts);
}

inline const ALCompositePoolAllocator* ALTypeAllocatorAndSerializer::getCompositePoolAllocator
() const
{
  return fCompPoolAllocator;
}



} // End of namespace onsem


#endif // ONSEM_TYPEALLOCATORANDSERIALIZER_DETAILS_TYPEALLOCATORANDSERIALIZER_HXX
