#ifndef ONSEM_COMPILERMODEL_SAVERS_BINARYDATABASEDICOSAVER_HXX
#define ONSEM_COMPILERMODEL_SAVERS_BINARYDATABASEDICOSAVER_HXX

#include <onsem/compilermodel/savers/binarydatabasedicosaver.hpp>

namespace onsem
{


template<typename TRULE>
signed char* BinaryDatabaseDicoSaver::xWriteRuleWithMeanings
(signed char* pEndMemory,
 const std::map<const LingdbMeaning*, int>& pMeaningsPtr,
 const LinguisticIntermediaryDatabase& pLingDatabase,
 TRULE* pRule,
 int pNbCharToPutZeroIfEmpty) const
{
  if (pRule != nullptr)
  {
    pEndMemory = pRule->exportToBin(pEndMemory,
                                    pMeaningsPtr,
                                    pLingDatabase);
  }
  else
  {
    for (int i = 0; i < pNbCharToPutZeroIfEmpty; ++i)
    {
      binarysaver::writeChar(pEndMemory++, 0);
    }
  }
  return pEndMemory;
}



inline signed char* BinaryDatabaseDicoSaver::xGetLemme
(signed char* pMeaning)
{
  return pMeaning + sizeof(int);
}


} // End of namespace onsem


#endif // ONSEM_COMPILERMODEL_SAVERS_BINARYDATABASEDICOSAVER_HXX
