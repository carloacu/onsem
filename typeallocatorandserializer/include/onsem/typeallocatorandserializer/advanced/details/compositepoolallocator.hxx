#ifndef ONSEM_TYPEALLOCATORANDSERIALIZER_ADVANCED_DETAILS_COMPOSITEPOOLALLOCATOR_HXX
#define ONSEM_TYPEALLOCATORANDSERIALIZER_ADVANCED_DETAILS_COMPOSITEPOOLALLOCATOR_HXX

#include <onsem/typeallocatorandserializer/advanced/compositepoolallocator.hpp>
#include <cstdlib>
#include <onsem/typeallocatorandserializer/advanced/leafpoolallocator.hpp>


namespace onsem
{


template<typename T>
inline bool CompositePoolAllocator::isAllocated
(T* pPtr) const
{
  return xGetLeaf<T>()->isAllocated(pPtr);
}


template<typename T>
inline void CompositePoolAllocator::reserve
(std::size_t pNbElts)
{
  return xGetLeaf<T>()->reserve(pNbElts);
}


template <typename T>
inline T* CompositePoolAllocator::allocate
(std::size_t pNbElts)
{
  return xGetLeaf<T>()->allocate(pNbElts);
}

template <typename T>
inline void CompositePoolAllocator::deallocate
(T* pPointer, std::size_t pNbElts)
{
  return xGetLeaf<T>()->deallocate(pPointer, pNbElts);
}


template<typename T>
inline T* CompositePoolAllocator::first
() const
{
  return xGetLeaf<T>()->first();
}


template<typename T>
inline T* CompositePoolAllocator::next
(T* pElt) const
{
  return xGetLeaf<T>()->next(pElt);
}


template<typename T>
inline void
CompositePoolAllocator::addANewLeaf
(const std::string& pName,
 unsigned char pMemoryAlignment,
 get_pointers pFunc)
{
  fPoolAllocators.emplace_back(new LeafPoolAllocator<T>(pName, pMemoryAlignment, pFunc));
}





template<typename T>
inline LeafPoolAllocator<T>* CompositePoolAllocator::xGetLeaf
() const
{
  for (std::size_t i = 0; i < fPoolAllocators.size(); ++i)
  {
    LeafPoolAllocator<T>* leafPool =
        dynamic_cast<LeafPoolAllocator<T>* > (fPoolAllocators[i]);
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

