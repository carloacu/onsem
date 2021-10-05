#include <onsem/typeallocatorandserializer/advanced/compositepoolallocator.hpp>
#include <onsem/typeallocatorandserializer/advanced/treememoryprettyprinter.hpp>
#include <string>
#include <onsem/typeallocatorandserializer/advanced/treememoryprettyprinter.hpp>

namespace onsem
{

std::ostream& operator<<
(std::ostream& pOs, const ALCompositePoolAllocator& pFPA)
{
  ALTreeMemoryPrettyPrinter printer(pOs);
  pFPA.accept(printer);
  return pOs;
}


  ALCompositePoolAllocator::ALCompositePoolAllocator
  (const std::string& pName)
    : ALComponentPoolAllocator(pName),
      fPoolAllocators()
  {
  }


  void ALCompositePoolAllocator::clear
  ()
  {
    for (std::size_t i = 0; i < fPoolAllocators.size(); ++i)
    {
      fPoolAllocators[i]->clear();
    }
  }


  std::size_t ALCompositePoolAllocator::getOccupatedSize
  () const
  {
    std::size_t res = 0;
    for (std::size_t i = 0; i < fPoolAllocators.size(); ++i)
    {
      res += fPoolAllocators[i]->getOccupatedSize();
    }
    return res;
  }


  std::size_t ALCompositePoolAllocator::getTotalSize
  () const
  {
    std::size_t res = 0;
    for (std::size_t i = 0; i < fPoolAllocators.size(); ++i)
    {
      res += fPoolAllocators[i]->getTotalSize();
    }
    return res;
  }



  void ALCompositePoolAllocator::addANewComposite
  (const std::string& pName)
  {
    fPoolAllocators.emplace_back(new ALCompositePoolAllocator(pName));
  }


  void ALCompositePoolAllocator::accept
  (ALTreeMemoryPrettyPrinter& pV) const
  {
    pV (*this);
    for (std::size_t i = 0; i < fPoolAllocators.size(); ++i)
    {
      fPoolAllocators[i]->accept(pV);
    }
  }


  void ALCompositePoolAllocator::xWriteMemoryInAStream
  (std::ostream& pOstr) const
  {
    for (std::size_t i = 0; i < fPoolAllocators.size(); ++i)
    {
      fPoolAllocators[i]->xWriteMemoryInAStream(pOstr);
    }
  }


  void ALCompositePoolAllocator::xReadMemoryFromAStream
  (std::set<FPAShift>& pShifts,
   std::istream& pIstr, float pCoef)
  {
    for (std::size_t i = 0; i < fPoolAllocators.size(); ++i)
    {
      fPoolAllocators[i]->xReadMemoryFromAStream(pShifts, pIstr, pCoef);
    }
  }


  void ALCompositePoolAllocator::xFindShiftsForDefragmentation
  (std::set<FPAShift>& pShifts) const
  {
    for (std::size_t i = 0; i < fPoolAllocators.size(); ++i)
    {
      fPoolAllocators[i]->xFindShiftsForDefragmentation(pShifts);
    }
  }


  void ALCompositePoolAllocator::xChangePointers
  (const std::set<FPAShift>& pShifts)
  {
    for (std::size_t i = 0; i < fPoolAllocators.size(); ++i)
    {
      fPoolAllocators[i]->xChangePointers(pShifts);
    }
  }


  ALCompositePoolAllocator* ALCompositePoolAllocator::getComposite
  (const std::string& pName) const
  {
    for (std::size_t i = 0; i < fPoolAllocators.size(); ++i)
    {
      ALCompositePoolAllocator* leafPool =
          dynamic_cast<ALCompositePoolAllocator*> (fPoolAllocators[i]);
      if (leafPool && leafPool->getName() == pName)
      {
        return leafPool;
      }
    }
    return nullptr;
  }


  void ALCompositePoolAllocator::getComposites
  (std::vector<ALCompositePoolAllocator*>& pComposites)
  {
    for (std::size_t i = 0; i < fPoolAllocators.size(); ++i)
    {
      ALCompositePoolAllocator* leafPool =
          dynamic_cast<ALCompositePoolAllocator*> (fPoolAllocators[i]);
      if (leafPool)
      {
        pComposites.emplace_back(leafPool);
      }
    }
  }



} // End of namespace onsem
