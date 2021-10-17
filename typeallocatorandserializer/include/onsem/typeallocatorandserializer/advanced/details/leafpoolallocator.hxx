#ifndef ONSEM_TYPEALLOCATORANDSERIALIZER_ADVANCED_DEATILS_LEAFPOOLALLOCATOR_HXX
#define ONSEM_TYPEALLOCATORANDSERIALIZER_ADVANCED_DEATILS_LEAFPOOLALLOCATOR_HXX

#include <onsem/typeallocatorandserializer/advanced/leafpoolallocator.hpp>
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <algorithm>
#include <onsem/typeallocatorandserializer/advanced/treememoryprettyprinter.hpp>
#include <assert.h>

namespace onsem
{

template <typename T>
LeafPoolAllocator<T>::LeafPoolAllocator
(const std::string& pName,
 unsigned char pMemoryAlignment,
 get_pointers pFunc)
  : ComponentPoolAllocator(pName), fMemoryAlignment(pMemoryAlignment),
    fSizeStructAligned(xCalculateSizeStructAligned(pMemoryAlignment)),
    fFunc(pFunc),
    fPools()
{
}



template<typename T>
void LeafPoolAllocator<T>::reserve
(std::size_t pNbElts)
{
  if (pNbElts == 0)
  {
    return;
  }
  // Create a new pool of "pNbElts" elts
  std::size_t memSize = pNbElts * fSizeStructAligned;
  FPAPool newPool;
  newPool.mem.ptr = ::operator new(memSize);
  if (newPool.mem.ptr == nullptr)
  {
    std::cerr << "error: bad alloc of " << memSize
              << " bytes" << std::endl;
  }
  newPool.newAlloc = newPool.mem;
  newPool.maxSize = memSize;
  fPools.emplace_back(newPool);
}



template <typename T>
T* LeafPoolAllocator<T>::allocate(std::size_t pNbElts)
{
  if (pNbElts <= 0)
  {
    return nullptr;
  }
  // if there is no pool for this type we create a new pool that
  // contains 100 elts
  if (fPools.size() == 0)
  {
    reserve(100);
  }

  // if we allocate only one element and
  // if a pool has a deallocated element,
  // we fill the gap
  if (pNbElts == 1)
  {
    for (std::size_t i = 0; i < fPools.size(); ++i)
    {
      if (fPools[i].deallocated.size() > 0)
      {
        std::set<FPAPtr>::iterator it = fPools[i].deallocated.begin();
        FPAPtr res = *it;
        fPools[i].deallocated.erase(it);
        return reinterpret_cast<T*>(res.ptr);
      }
    }
  }

  std::size_t memSize = pNbElts * fSizeStructAligned;

  // if there is not enough remaining space in the current pool,
  // we will reserve space in a new pool
  if (fPools[fPools.size() - 1].maxSize < memSize + (fPools[fPools.size() - 1].newAlloc.val -
                                                     fPools[fPools.size() - 1].mem.val))
  {
    reserve(std::max(fPools[fPools.size() - 1].maxSize / fSizeStructAligned, pNbElts) * 2);
  }

  // allocate at the end of the current pool
  T* res = reinterpret_cast<T*>(fPools[fPools.size() - 1].newAlloc.ptr);
  fPools[fPools.size() - 1].newAlloc.val = fPools[fPools.size() - 1].newAlloc.val + memSize;
  return res;
}


template <typename T>
void LeafPoolAllocator<T>::deallocate
(T* pPointer, std::size_t pNbElts)
{
  if (pPointer == nullptr)
  {
    return;
  }
  FPAPtr ptr = pPointer;
  // get the pool of the pointer
  std::size_t idPool = xGetPool(ptr);
  // until we don't have other elts to deallocate
  while (pNbElts > 0)
  {
    if (fPools[idPool].deallocated.find(ptr) != fPools[idPool].deallocated.end())
    {
      std::cerr << "error: try to deallocate a pointer already deallocated" << std::endl;
    }
    // deallocate = add the pointer to list of deallocated elts
    fPools[idPool].deallocated.insert(ptr);
    ptr.val += fSizeStructAligned;
    --pNbElts;
  }
  return;
}



template<typename T>
void LeafPoolAllocator<T>::clear
()
{
  // remove all pools and put the memory at this initial state
  for (std::size_t i = 0; i < fPools.size(); ++i)
  {
    fPools[i].newAlloc = nullptr;
    ::operator delete(fPools[i].mem.ptr);
    fPools[i].mem = nullptr;
    fPools[i].maxSize = 0;
    fPools[i].deallocated.clear();
  }
  fPools.clear();
}



template<typename T>
T* LeafPoolAllocator<T>::first
() const
{
  return reinterpret_cast<T*>(xGetFirstAlloc());
}



template<typename T>
T* LeafPoolAllocator<T>::next
(T* elt) const
{
  return reinterpret_cast<T*>(xGetNext(elt));
}



template<typename T>
std::size_t LeafPoolAllocator<T>::getOccupatedSize
() const
{
  std::size_t res = 0;
  for (std::size_t i = 0; i < fPools.size(); ++i)
  {
    res += xGetPoolOccupatedSize(fPools[i]);
  }
  return res;
}



template<typename T>
std::size_t LeafPoolAllocator<T>::getTotalSize
() const
{
  std::size_t res = 0;
  for (std::size_t i = 0; i < fPools.size(); ++i)
  {
    res += fPools[i].maxSize;
  }
  return res;
}



template<typename T>
void LeafPoolAllocator<T>::xWriteMemoryInAStream
(std::ostream& pOstr) const
{
  // Give the size occupated
  std::size_t occSize = getOccupatedSize();
  pOstr.write(reinterpret_cast<const char*>(&occSize),
              sizeof(occSize));

  // Indicate the number of pools
  unsigned char nbPools = static_cast<unsigned char>(fPools.size());
  pOstr.write(reinterpret_cast<const char*>(&nbPools),
              sizeof(nbPools));

  // Write the original offset of each pool
  for (std::size_t i = 0; i < fPools.size(); ++i)
  {
    FPASerialPool sp;
    sp.begin = fPools[i].mem;
    sp.end.val = fPools[i].mem.val + xGetPoolOccupatedSize(fPools[i]);
    pOstr.write(reinterpret_cast<const char*>(&sp), sizeof(sp));
  }

  // Write each pool
  for (std::size_t i = 0; i < fPools.size(); ++i)
  {
    pOstr.write(reinterpret_cast<const char*>(fPools[i].mem.ptr),
                xGetPoolOccupatedSize(fPools[i]));
  }
}



template<typename T>
void LeafPoolAllocator<T>::xReadMemoryFromAStream
(std::set<FPAShift>& pShifts,
 std::istream& pIstr,
 float pCoef)
{
  // Read the size occupated in the file
  std::size_t occupatedSize;
  pIstr.read(reinterpret_cast<char*>(&occupatedSize),
             sizeof(occupatedSize));

  // Read the number of pools
  unsigned char nbPools;
  pIstr.read(reinterpret_cast<char*>(&nbPools), sizeof(nbPools));

  // Read the original offset of each pool
  std::vector<FPASerialPool> sp(nbPools);
  for (unsigned char i = 0; i < nbPools; ++i)
  {
    pIstr.read(reinterpret_cast<char*>(&sp[i]), sizeof(sp[i]));
  }

  FPAPool newPool;
  newPool.maxSize = static_cast<std::size_t>(occupatedSize * pCoef);
  newPool.mem = ::operator new(newPool.maxSize);
  if (newPool.mem.ptr == nullptr)
  {
    std::cerr << "error: bad alloc of " << newPool.maxSize
              << " bytes" << std::endl;
  }

  // Read all the previous pools
  // (that is considered like an unique pool now)
  pIstr.read(newPool.mem.pchar, occupatedSize);

  // adjust the list of shifts
  std::size_t newDec = newPool.mem.val;
  for (unsigned char i = 0; i < nbPools; ++i)
  {
    FPAShift dec;
    dec.begin = sp[i].begin;
    dec.end = sp[i].end;
    dec.new_begin.val = newDec;
    newDec += dec.end.val - dec.begin.val;
    pShifts.insert(dec);
  }

  newPool.newAlloc.val = newPool.mem.val + occupatedSize;
  fPools.emplace_back(newPool);
}



template<typename T>
void LeafPoolAllocator<T>::xFindShiftsForDefragmentation
(std::set<FPAShift>& pShifts) const
{
  // The goal of this function is to add a shift for every continuous part of every pool.
  // shift = say that a memory zone (defined with a begin and an end pointer) will,
  //         after the defragmentation, begin at another pointer position.
  // "continous part of a pool" = "a memory zone that only have allocated elements".
  for (std::size_t i = 0; i < fPools.size(); ++i)
  {
    // Add the shift that correspond to the begin of the pool
    if (fPools[i].deallocated.size() == 0 ||
        fPools[i].mem != *fPools[i].deallocated.begin())
    {
      FPAShift dec;
      dec.begin = fPools[i].mem;
      if (fPools[i].deallocated.size() == 0)
      {
        dec.end = fPools[i].newAlloc;
      }
      else
      {
        dec.end = *fPools[i].deallocated.begin();
      }
      dec.new_begin = dec.begin;
      pShifts.insert(dec);
    }
    // Add a shift between each deallocated element
    std::size_t j = 1;
    for (std::set<FPAPtr>::iterator it = fPools[i].deallocated.begin();
         it != fPools[i].deallocated.end(); ++it)
    {
      FPAShift dec;
      dec.begin.val = it->val + fSizeStructAligned;
      if (j == fPools[i].deallocated.size())
      {
        dec.end = fPools[i].newAlloc;
      }
      else
      {
        ++it;
        dec.end = *it;
        --it;
      }
      dec.new_begin.val = dec.begin.val - j * fSizeStructAligned;
      if (dec.end.val > dec.begin.val)
      {
        pShifts.insert(dec);
      }
      ++j;
    }
  }
}



template<typename T>
void LeafPoolAllocator<T>::xChangePointers
(const std::set<FPAShift>& pShifts)
{
  // The goal of this function is to adapt the pointers to the given shifts
  FPAPtr it_ptr = xGetFirstAlloc();
  if (!fFunc || fPools.empty() || it_ptr.ptr == nullptr)
  {
    return;
  }
  std::vector<const void*> class_pointers;
  fFunc(class_pointers, it_ptr.ptr);

  // A vector that hold each pointer decalage inside an elt
  std::vector<std::size_t> decPtrs(class_pointers.size());
  // A vector that keen in mind the previous shift used for each pointer
  // (this is for optimization purpose)
  std::vector<std::set<FPAShift>::const_iterator> refPool(class_pointers.size());
  for (std::size_t j = 0; j < class_pointers.size(); ++j)
  {
    decPtrs[j] = reinterpret_cast<std::size_t>(class_pointers[j]) - it_ptr.val;
    refPool[j] = pShifts.begin();
  }

  // Iterate over all the pointers to adapt their values
  while (it_ptr != nullptr)
  {
    for (std::size_t j = 0; j < decPtrs.size(); ++j)
    {
      FPAPtr* pptr = reinterpret_cast<FPAPtr*>(it_ptr.val + decPtrs[j]);
      if (*pptr != nullptr)
      {
        // If the previous shift used is still valid
        if (!(refPool[j]->begin > *pptr || refPool[j]->end <= *pptr))
        {
          std::size_t decBegin = pptr->val - refPool[j]->begin.val;
          pptr->val = refPool[j]->new_begin.val + decBegin;
        }
        else // find the good shift and adapt the pointer
        {
          pptr->val = xModifyAPtr(refPool[j], *pptr, pShifts);
        }
      }
    }
    it_ptr.ptr = xGetNext(it_ptr.ptr);
  }
}



template<typename T>
std::size_t LeafPoolAllocator<T>::xCalculateSizeStructAligned
(unsigned char pMemoryAlignment)
{
  std::size_t size_of = sizeof(T);
  while (true)
  {
    if (size_of <= pMemoryAlignment)
    {
      if (pMemoryAlignment % size_of == 0)
      {
        return size_of;
      }
    }
    else if (size_of % pMemoryAlignment == 0)
    {
      return size_of;
    }
    ++size_of;
  }
  return 0;
}



template <typename T>
bool LeafPoolAllocator<T>::isAllocated
(T* pPtr) const
{
  // Find the good pool
  for (std::size_t i = 0; i < fPools.size(); ++i)
  {
    if (pPtr >= fPools[i].mem.ptr && pPtr < fPools[i].newAlloc.ptr)
    {
      // Check that is not deallocated
      if (fPools[i].deallocated.find(pPtr) == fPools[i].deallocated.end())
      {
        // Check that is a good memory alignement for a pointer of type T
        return (reinterpret_cast<std::size_t>(pPtr) - fPools[i].mem.val) % fMemoryAlignment == 0;
      }
      return false;
    }
  }
  return false;
}


template <typename T>
void LeafPoolAllocator<T>::accept
(TreeMemoryPrettyPrinter& pV) const
{
  pV (*this);
}


template <typename T>
void* LeafPoolAllocator<T>::xGetFirstAlloc
() const
{
  for (std::size_t i = 0; i < fPools.size(); ++i)
  {
    FPAPtr ptr = fPools[i].mem;
    while (ptr < fPools[i].newAlloc)
    {
      if (fPools[i].deallocated.find(ptr) == fPools[i].deallocated.end())
      {
        return ptr.ptr;
      }
      ptr.val = ptr.val + fSizeStructAligned;
    }
  }
  return nullptr;
}



template <typename T>
void* LeafPoolAllocator<T>::xGetNext
(FPAPtr pPtr) const
{
  std::size_t firstPoolPtr = xGetPool(pPtr);
  for (std::size_t i = firstPoolPtr; i < fPools.size(); ++i)
  {
    if (i == firstPoolPtr)
    {
      pPtr.val = pPtr.val + fSizeStructAligned;
    }
    else
    {
      pPtr = fPools[i].mem;
    }
    while (pPtr < fPools[i].newAlloc)
    {
      if (fPools[i].deallocated.find(pPtr) == fPools[i].deallocated.end())
      {
        return pPtr.ptr;
      }
      pPtr.val = pPtr.val + fSizeStructAligned;
    }
  }
  return nullptr;
}



template <typename T>
inline std::size_t LeafPoolAllocator<T>::xGetPool
(FPAPtr pPtr) const
{
  for (std::size_t i = 0; i < fPools.size(); ++i)
  {
    if (pPtr >= fPools[i].mem && pPtr < fPools[i].newAlloc)
    {
      return i;
    }
  }
  std::cerr << "error: the given pointer is not in the alloc object"
            << std::endl;
  return 0;
}



template<typename T>
inline std::size_t LeafPoolAllocator<T>::xGetPoolOccupatedSize
(const FPAPool& pPool) const
{
  return pPool.newAlloc.val - pPool.mem.val -
      (pPool.deallocated.size() * fSizeStructAligned);
}



template<typename T>
inline std::size_t LeafPoolAllocator<T>::xModifyAPtr
(std::set<FPAShift>::const_iterator& pRefPool,
 FPAPtr pPptr,
 const std::set<FPAShift>& pShifts)
{
  FPAShift prov;
  prov.begin = pPptr;
  pRefPool = pShifts.lower_bound(prov);
  if (pRefPool->begin > pPptr || pRefPool->end <= pPptr)
  {
    std::cerr << "Error: A pointer point out of the pool of allocation." << std::endl;
    std::cerr << "The pointer is in \"" << fName << "\" pool of allocation." << std::endl;
    assert(false);
  }

  std::size_t decBegin = pPptr.val - pRefPool->begin.val;
  return pRefPool->new_begin.val + decBegin;
}



template<typename T>
std::size_t LeafPoolAllocator<T>::getSizeStructAligned
() const
{
  return fSizeStructAligned;
}



} // End of namespace onsem

#endif // ONSEM_TYPEALLOCATORANDSERIALIZER_ADVANCED_DEATILS_LEAFPOOLALLOCATOR_HXX

