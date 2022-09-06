#include <onsem/tester/memorybinarization.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/semanticexpression.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemorybloc.hpp>
#include <onsem/texttosemantic/dbtype/binary/semexploader.hpp>
#include <onsem/texttosemantic/dbtype/binary/semexpsaver.hpp>


namespace onsem
{

bool checkMemBlocBinarization(const SemanticMemoryBlock& pMemBloc,
                              const linguistics::LinguisticDatabase& pLingDb)
{
  bool res = true;
  auto semExps = pMemBloc.getSemExps();
  const std::size_t maxSize = 3000000;
  binarymasks::Ptr mem = ::operator new(maxSize);
  binarymasks::Ptr beginPtr = mem;

  for (const auto& currSemExp : semExps)
  {
    mem = beginPtr;
    semexpsaver::writeSemExp(mem, *currSemExp, pLingDb, nullptr);

    binarymasks::Ptr loaderPtr = beginPtr;
    auto semExpLoaded = semexploader::loadSemExp(loaderPtr.pcuchar, pLingDb);
    if (*currSemExp != *semExpLoaded)
    {
      res = false;
      currSemExp->assertEqual(*semExpLoaded);
    }
  }
  ::operator delete(beginPtr.pchar);
  return res;
}


}

