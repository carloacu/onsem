#ifndef ONSEM_COMPILERMODEL_LINGDBWORDFORMS_HXX
#define ONSEM_COMPILERMODEL_LINGDBWORDFORMS_HXX

#include <onsem/compilermodel/lingdbwordforms.hpp>
#include <cstddef>


namespace onsem
{



inline LingdbMeaning* LingdbWordForms::getMeaning
() const
{
  return fMeaning;
}



inline const LingdbFlexions* LingdbWordForms::getFlexions
() const
{
  return fFlexions;
}


inline void LingdbWordForms::setFrequency(char pFrequency)
{
  fFrequency = pFrequency;
}


inline char LingdbWordForms::getFrequency() const
{
  return fFrequency;
}


inline void LingdbWordForms::xGetPointers
(std::vector<const void*>& pRes, void* pVar)
{
  pRes.emplace_back(&reinterpret_cast<LingdbWordForms*>
                 (pVar)->fMeaning);
  pRes.emplace_back(&reinterpret_cast<LingdbWordForms*>
                 (pVar)->fFlexions);
}


} // End of namespace onsem


#endif // ONSEM_COMPILERMODEL_LINGDBWORDFORMS_HXX
