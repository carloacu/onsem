#ifndef ONSEM_TYPEALLOCATORANDSERIALIZER_ADVANCED_DETAILS_COMPOSITEPOOLALLOCATOR_HXX
#define ONSEM_TYPEALLOCATORANDSERIALIZER_ADVANCED_DETAILS_COMPOSITEPOOLALLOCATOR_HXX

#include <onsem/typeallocatorandserializer/advanced/compositepoolallocator.hpp>
#include <cstdlib>
#include <onsem/typeallocatorandserializer/advanced/leafpoolallocator.hpp>


namespace onsem
{


template<typename T>
inline bool ALCompositePoolAllocator::isAllocated
(T* pPtr) const
{
  return xGetLeaf<T>()->isAllocated(pPtr);
}


template<typename T>
inline void ALCompositePoolAllocator::reserve
(std::size_t pNbElts)
{
  return xGetLeaf<T>()->reserve(pNbElts);
}


template <typename T>
inline T* ALCompositePoolAllocator::allocate
(std::size_t pNbElts)
{
  return xGetLeaf<T>()->allocate(pNbElts);
}

template <typename T>
inline void ALCompositePoolAllocator::deallocate
(T* pPointer, std::size_t pNbElts)
{
  return xGetLeaf<T>()->deallocate(pPointer, pNbElts);
}


template<typename T>
inline T* ALCompositePoolAllocator::first
() const
{
  return xGetLeaf<T>()->first();
}


template<typename T>
inline T* ALCompositePoolAllocator::next
(T* pElt) const
{
  return xGetLeaf<T>()->next(pElt);
}


template<typename T>
inline void
ALCompositePoolAllocator::addANewLeaf
(const std::string& pName,
 unsigned char pMemoryAlignment,
 get_pointers pFunc)
{
  fPoolAllocators.emplace_back(new ALLeafPoolAllocator<T>(pName, pMemoryAlignment, pFunc));
}





template<typename T>
inline ALLeafPoolAllocator<T>* ALCompositePoolAllocator::xGetLeaf
() const
{
  for (std::size_t i = 0; i < fPoolAllocators.size(); ++i)
  {
    ALLeafPoolAllocator<T>* leafPool =
        dynamic_cast<ALLeafPoolAllocator<T>* > (fPoolAllocators[i]);
    if (leafPool)
    {
      return leafPool;
    }
  }

  std::cerr << "error: Try to get an undefined leafPoolAllocator. "
            << "Call addANewLeaf to define this leaf" << std::endl;
  return nullptr;
}


} // End of namespace onsem

#endif // ONSEM_TYPEALLOCATORANDSERIALIZER_ADVANCED_DETAILS_COMPOSITEPOOLALLOCATOR_HXX

