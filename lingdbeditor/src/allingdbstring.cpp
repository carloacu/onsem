#include <onsem/lingdbeditor/allingdbstring.hpp>
#include <onsem/typeallocatorandserializer/typeallocatorandserializer.hpp>



namespace onsem
{

ALLingdbString::ALLingdbString()
  : fNbCharacters(0),
    fCharacters(nullptr)
{
}


void ALLingdbString::xInit
(ALCompositePoolAllocator& pFPAlloc,
 const std::string& pTagName)
{
  fNbCharacters = static_cast<unsigned char>(pTagName.size());
  fCharacters = pFPAlloc.allocate<char>(fNbCharacters);
  for (std::size_t i = 0; i < pTagName.size(); ++i)
  {
    fCharacters[i] = pTagName[i];
  }
}

void ALLingdbString::xDeallocate
(ALCompositePoolAllocator& pFPAlloc)
{
  pFPAlloc.deallocate<char>(fCharacters, fNbCharacters);
  pFPAlloc.deallocate<ALLingdbString>(this);
}


std::string ALLingdbString::toStr
() const
{
  return std::string(fCharacters, fNbCharacters);
}

unsigned char ALLingdbString::length
() const
{
  return fNbCharacters;
}

char* ALLingdbString::toCStr
() const
{
  return fCharacters;
}


void ALLingdbString::xGetPointers
(std::vector<const void*>& pRes, void* pVar)
{
  pRes.emplace_back(&reinterpret_cast<ALLingdbString*>
                 (pVar)->fCharacters);
}



} // End of namespace onsem
