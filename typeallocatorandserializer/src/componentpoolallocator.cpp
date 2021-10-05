#include <onsem/typeallocatorandserializer/advanced/componentpoolallocator.hpp>
#include <iostream>
#include <sstream>
#include <cstring>
#include <assert.h>
#include <boost/filesystem/fstream.hpp>

namespace onsem
{

  ALComponentPoolAllocator::ALComponentPoolAllocator
  (const boost::filesystem::path& name)
    : fName(name.string())
  {
  }


  void ALComponentPoolAllocator::serialize
  (const boost::filesystem::path& pFilename)
  {
    std::set<FPAShift> shifts;
    // Do the serialization task.
    // This function is private because it modify the memory.
    // If we want to keep using the memory after, we have to retore it.
    xSerialize(shifts, pFilename);

    // re put the memory has we have found it
    std::set<FPAShift> invertShifts;
    xInvertShifts(invertShifts, shifts);
    xFragmentMemory(invertShifts);
    xChangePointers(invertShifts);
  }


  void ALComponentPoolAllocator::serializeAndClear
  (const std::string& pFilename)
  {
    std::set<FPAShift> shifts;
    xSerialize(shifts, pFilename);
    clear();
  }


  void ALComponentPoolAllocator::deserialize
  (std::string& pErrorMessage,
   const boost::filesystem::path& pFilename, float pCoef)
  {
    clear();

    boost::filesystem::ifstream infile(pFilename, boost::filesystem::ifstream::binary);
    if (!infile.is_open())
    {
      pErrorMessage = "Error: Can't open " + pFilename.string() + " file !";
      return;
    }
    std::set<FPAShift> shifts;
    xReadMemoryFromAStream(shifts, infile, pCoef);
    xChangePointers(shifts);
    infile.close();
  }


  void ALComponentPoolAllocator::xSerialize
  (std::set<FPAShift>& pShifts,
   const boost::filesystem::path& pFilename)
  {
    boost::filesystem::ofstream outfile(pFilename,
                                        boost::filesystem::ofstream::binary);
    xFindShiftsForDefragmentation(pShifts);
    xChangePointers(pShifts);
    xDefragmentMemory(pShifts);
    xWriteMemoryInAStream(outfile);
    outfile.close();
  }


  void ALComponentPoolAllocator::xInvertShifts
  (std::set<FPAShift>& pNewShifts,
   const std::set<FPAShift>& pPrevShifts) const
  {
    for (std::set<FPAShift>::iterator it = pPrevShifts.begin();
         it != pPrevShifts.end(); ++it)
    {
      FPAShift newDec;
      newDec.begin = it->new_begin;
      newDec.end.val = it->new_begin.val + (it->end.val - it->begin.val);
      newDec.new_begin = it->begin;
      pNewShifts.insert(newDec);
    }
  }


  void ALComponentPoolAllocator::xDefragmentMemory
  (const std::set<FPAShift>& pShifts)
  {
    for (std::set<FPAShift>::const_reverse_iterator it = pShifts.rbegin();
         it != pShifts.rend(); ++it)
    {
      if (it->begin.val == it->new_begin.val)
      {
        continue;
      }
      assert(it->begin.val > it->new_begin.val);
      std::size_t size_to_move = it->end.val - it->begin.val;
      if (size_to_move < it->begin.val - it->new_begin.val)
      {
        memcpy(it->new_begin.ptr, it->begin.ptr, size_to_move);
      }
      else
      {
        memmove(it->new_begin.ptr, it->begin.ptr, size_to_move);
      }
    }
  }


  void ALComponentPoolAllocator::xFragmentMemory
  (const std::set<FPAShift>& pShifts)
  {
    for (std::set<FPAShift>::const_iterator it = pShifts.begin();
         it != pShifts.end(); ++it)
    {
      if (it->new_begin.val == it->begin.val)
      {
        continue;
      }
      assert(it->new_begin.val > it->begin.val);
      std::size_t size_to_move = it->end.val - it->begin.val;
      if (size_to_move < it->new_begin.val - it->begin.val)
      {
        memcpy(it->new_begin.ptr, it->begin.ptr, size_to_move);
      }
      else
      {
        memmove(it->new_begin.ptr, it->begin.ptr, size_to_move);
      }
    }
  }


} // End of namespace onsem
