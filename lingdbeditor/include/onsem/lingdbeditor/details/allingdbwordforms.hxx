#ifndef ALLINGDBWORDFORMS_HXX
#define ALLINGDBWORDFORMS_HXX

#include <onsem/lingdbeditor/allingdbwordforms.hpp>
#include <cstddef>


namespace onsem
{



inline ALLingdbMeaning* ALLingdbWordForms::getMeaning
() const
{
  return fMeaning;
}



inline const ALLingdbFlexions* ALLingdbWordForms::getFlexions
() const
{
  return fFlexions;
}


inline void ALLingdbWordForms::setFrequency(char pFrequency)
{
  fFrequency = pFrequency;
}


inline char ALLingdbWordForms::getFrequency() const
{
  return fFrequency;
}


inline void ALLingdbWordForms::xGetPointers
(std::vector<const void*>& pRes, void* pVar)
{
  pRes.emplace_back(&reinterpret_cast<ALLingdbWordForms*>
                 (pVar)->fMeaning);
  pRes.emplace_back(&reinterpret_cast<ALLingdbWordForms*>
                 (pVar)->fFlexions);
}


} // End of namespace onsem


#endif // ALLINGDBWORDFORMS_HXX
