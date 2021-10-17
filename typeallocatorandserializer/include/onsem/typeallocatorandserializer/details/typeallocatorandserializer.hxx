#ifndef ONSEM_TYPEALLOCATORANDSERIALIZER_DETAILS_TYPEALLOCATORANDSERIALIZER_HXX
#define ONSEM_TYPEALLOCATORANDSERIALIZER_DETAILS_TYPEALLOCATORANDSERIALIZER_HXX

#include <onsem/typeallocatorandserializer/typeallocatorandserializer.hpp>
#include <onsem/typeallocatorandserializer/advanced/compositepoolallocator.hpp>

namespace onsem
{

inline TypeAllocatorAndSerializer::TypeAllocatorAndSerializer
(const std::string& pName)
  : fCompPoolAllocator(new CompositePoolAllocator(pName))
{
}

template<typename T>
inline void TypeAllocatorAndSerializer::declareANewType
(const std::string& pName,
 unsigned char pMemoryAlignment,
 get_pointers pFunc)
{
  fCompPoolAllocator->addANewLeaf<T>(pName, pMemoryAlignment, pFunc);
}

template<typename T>
inline T* TypeAllocatorAndSerializer::allocate
(std::size_t pNbElts)
{
  return fCompPoolAllocator->allocate<T>(pNbElts);
}

template<typename T>
inline void TypeAllocatorAndSerializer::deallocate
(T* pPointer, std::size_t pNbElts)
{
  fCompPoolAllocator->deallocate<T>(pPointer, pNbElts);
}

template<typename T>
bool TypeAllocatorAndSerializer::isAllocated
(T* pPtr) const
{
  return fCompPoolAllocator->isAllocated<T>(pPtr);
}

template<typename T>
inline T* TypeAllocatorAndSerializer::first
() const
{
  return fCompPoolAllocator->first<T>();
}

template<typename T>
inline T* TypeAllocatorAndSerializer::next
(T* pElt) const
{
  return fCompPoolAllocator->next<T>(pElt);
}

inline void TypeAllocatorAndSerializer::serialize
(const std::string& pFilename)
{
  fCompPoolAllocator->serialize(pFilename);
}

inline void TypeAllocatorAndSerializer::serializeAndClear
(const std::string& pFilename)
{
  fCompPoolAllocator->serializeAndClear(pFilename);
}

inline void TypeAllocatorAndSerializer::deserialize
(std::string& pErrorMessage,
 const std::string& pFilename, float pCoef)
{
  fCompPoolAllocator->deserialize
      (pErrorMessage, pFilename, pCoef);
}

inline void TypeAllocatorAndSerializer::clear
()
{
  fCompPoolAllocator->clear();
}

inline const std::string& TypeAllocatorAndSerializer::getName
() const
{
  return fCompPoolAllocator->getName();
}

inline std::size_t TypeAllocatorAndSerializer::getOccupatedSize
() const
{
  return fCompPoolAllocator->getOccupatedSize();
}

inline std::size_t TypeAllocatorAndSerializer::getTotalSize
() const
{
  return fCompPoolAllocator->getTotalSize();
}

inline void TypeAllocatorAndSerializer::setName
(std::string pName)
{
  fCompPoolAllocator->setName(pName);
}

template<typename T>
inline void TypeAllocatorAndSerializer::reserve
(std::size_t pNbElts)
{
  fCompPoolAllocator->reserve<T>(pNbElts);
}

inline const CompositePoolAllocator* TypeAllocatorAndSerializer::getCompositePoolAllocator
() const
{
  return fCompPoolAllocator;
}



} // End of namespace onsem


#endif // ONSEM_TYPEALLOCATORANDSERIALIZER_DETAILS_TYPEALLOCATORANDSERIALIZER_HXX
