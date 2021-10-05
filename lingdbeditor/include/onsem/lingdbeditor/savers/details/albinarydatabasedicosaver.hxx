#ifndef ALBINARYDATABASEDICOSAVER_HXX
#define ALBINARYDATABASEDICOSAVER_HXX

#include <onsem/lingdbeditor/savers/albinarydatabasedicosaver.hpp>

namespace onsem
{


template<typename TRULE>
signed char* ALBinaryDatabaseDicoSaver::xWriteRuleWithMeanings
(signed char* pEndMemory,
 const std::map<const ALLingdbMeaning*, int>& pMeaningsPtr,
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



inline signed char* ALBinaryDatabaseDicoSaver::xGetLemme
(signed char* pMeaning)
{
  return pMeaning + sizeof(int);
}


} // End of namespace onsem


#endif // ALBINARYDATABASEDICOSAVER_HXX
