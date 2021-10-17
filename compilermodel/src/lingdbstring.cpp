#include <onsem/compilermodel/lingdbstring.hpp>
#include <onsem/typeallocatorandserializer/typeallocatorandserializer.hpp>



namespace onsem
{

LingdbString::LingdbString()
  : fNbCharacters(0),
    fCharacters(nullptr)
{
}


void LingdbString::xInit
(CompositePoolAllocator& pFPAlloc,
 const std::string& pTagName)
{
  fNbCharacters = static_cast<unsigned char>(pTagName.size());
  fCharacters = pFPAlloc.allocate<char>(fNbCharacters);
  for (std::size_t i = 0; i < pTagName.size(); ++i)
  {
    fCharacters[i] = pTagName[i];
  }
}

void LingdbString::xDeallocate
(CompositePoolAllocator& pFPAlloc)
{
  pFPAlloc.deallocate<char>(fCharacters, fNbCharacters);
  pFPAlloc.deallocate<LingdbString>(this);
}


std::string LingdbString::toStr
() const
{
  return std::string(fCharacters, fNbCharacters);
}

unsigned char LingdbString::length
() const
{
  return fNbCharacters;
}

char* LingdbString::toCStr
() const
{
  return fCharacters;
}


void LingdbString::xGetPointers
(std::vector<const void*>& pRes, void* pVar)
{
  pRes.emplace_back(&reinterpret_cast<LingdbString*>
                 (pVar)->fCharacters);
}



} // End of namespace onsem
