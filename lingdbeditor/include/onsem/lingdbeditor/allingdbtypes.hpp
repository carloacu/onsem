#ifndef ALLINGDBTYPES_H
#define ALLINGDBTYPES_H

#include <onsem/typeallocatorandserializer/typeallocatorandserializer.hpp>


namespace onsem
{

/**
 * A list where each element only store a pointer to the next element.
 * So, not double linked.
 */
template <typename ELT>
struct ForwardPtrList
{
  /// The element stored in this node of the list.
  ELT* elt;
  /// The next node of the list.
  ForwardPtrList<ELT>* next;

  /**
   * @brief Initialize a new element.
   * @param pElt The new element.
   */
  void init(ELT* pElt)
  {
    elt = pElt;
    next = nullptr;
  }

  /**
   * @brief Get the last list elt.
   * @return The last list elt.
   */
  ForwardPtrList<ELT>* back()
  {
    ForwardPtrList<ELT>* it = this;
    while (it->next != nullptr)
    {
      it = it->next;
    }
    return it;
  }

  ForwardPtrList<ELT>* find
  (const ELT& pElt)
  {
    ForwardPtrList<ELT>* it = this;
    while (it != nullptr)
    {
      if (*it->elt == pElt)
      {
        return it;
      }
      it = it->next;
    }
    return nullptr;
  }

  unsigned char length() const
  {
    unsigned char res = 1;
    const ForwardPtrList<ELT>* it = this;
    while (it->next != nullptr)
    {
      assert(res < 254);
      ++res;
      it = it->next;
    }
    return res;
  }

  ForwardPtrList<ELT>* remove(const ELT& pElt,
                              ALCompositePoolAllocator& pAlloc)
  {
    ForwardPtrList<ELT>* prev = nullptr;
    ForwardPtrList<ELT>* it = this;
    ForwardPtrList<ELT>* newThis = this;
    while (it != nullptr)
    {
      if (*it->elt == pElt)
      {
        if (prev == nullptr)
          clearNextElt(newThis, pAlloc);
        else
          clearNextElt(prev->next, pAlloc);
        return newThis;
      }
      prev = it;
      it = it->next;
    }
    return newThis;
  }

  static void clearNextElt
  (ForwardPtrList<ELT>*& pListToModify,
   ALCompositePoolAllocator& pAlloc)
  {
    assert(pListToModify != nullptr);
    ForwardPtrList<ELT>* eltToDel = pListToModify;
    pListToModify = pListToModify->next;
    pAlloc.deallocate<ELT>(eltToDel->elt);
    pAlloc.deallocate<ForwardPtrList<ELT> >(eltToDel);
  }

  static void clearNextComposedElt
  (ForwardPtrList<ELT>*& pListToModify,
   ALCompositePoolAllocator& pAlloc)
  {
    assert(pListToModify != nullptr);
    ForwardPtrList<ELT>* eltToDel = pListToModify;
    pListToModify = pListToModify->next;
    eltToDel->elt->xDeallocate(pAlloc);
    pAlloc.deallocate<ForwardPtrList<ELT> >(eltToDel);
  }

  void clear
  (ALCompositePoolAllocator& pFPAlloc)
  {
    ForwardPtrList<ELT>* it = this;
    do
    {
      pFPAlloc.deallocate<ELT>(it->elt);
      ForwardPtrList<ELT>* toDel = it;
      it = it->next;
      pFPAlloc.deallocate<ForwardPtrList<ELT> >(toDel);
    }
    while (it != nullptr);
    next = nullptr;
  }


  void clearComposedElts
  (ALCompositePoolAllocator& pFPAlloc)
  {
    ForwardPtrList<ELT>* it = this;
    do
    {
      it->elt->xDeallocate(pFPAlloc);
      ForwardPtrList<ELT>* toDel = it;
      it = it->next;
      pFPAlloc.deallocate<ForwardPtrList<ELT> >(toDel);
    }
    while (it != nullptr);
    next = nullptr;
  }


  void clearWithoutDesallocateElts
  (ALCompositePoolAllocator& pFPAlloc)
  {
    ForwardPtrList<ELT>* it = this;
    do
    {
      ForwardPtrList<ELT>* toDel = it;
      it = it->next;
      pFPAlloc.deallocate<ForwardPtrList<ELT> >(toDel);
    }
    while (it != nullptr);
    next = nullptr;
  }


  /**
   * @brief Get the position of the pointers for the allocator.
   * @param pRes The position of the pointers.
   * @param pVar An object of this class.
   */
  static void getPointers
  (std::vector<const void*>& pRes, void* pVar)
  {
    pRes.emplace_back(&reinterpret_cast<ForwardPtrList<ELT>*>
                   (pVar)->elt);
    pRes.emplace_back(&reinterpret_cast<ForwardPtrList<ELT>*>
                   (pVar)->next);
  }
};


template <typename ELT>
void forwardListPushFront
(ALCompositePoolAllocator& pAlloc,
 ForwardPtrList<ELT>** pList,
 ELT* pNewElt)
{
  assert(pList != 0);
  ForwardPtrList<ELT>* newNodeElt = pAlloc.allocate<ForwardPtrList<ELT> >(1);
  newNodeElt->init(pNewElt);
  newNodeElt->next = *pList;
  *pList = newNodeElt;
}



} // End of namespace onsem

#endif // ALLINGDBTYPES_H
