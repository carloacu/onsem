#ifndef ONSEM_TEXTTOSEMANTIC_TYPE_LINGUISTICDATABASE_STATICSYNTHESIZERDICTIONARY_HXX
#define ONSEM_TEXTTOSEMANTIC_TYPE_LINGUISTICDATABASE_STATICSYNTHESIZERDICTIONARY_HXX

#include "../staticsynthesizerdictionary.hpp"

namespace onsem
{
namespace linguistics
{

inline bool StaticSynthesizerDictionary::xIsLoaded() const
{
  return fPtrConjugaisons != nullptr;
}


inline int StaticSynthesizerDictionary::xGetMascSingularPtr
(const signed char* pConjPtr) const
{
  return *(reinterpret_cast<const int*>(pConjPtr));
}

inline int StaticSynthesizerDictionary::xGetMascPluralPtr
(const signed char* pConjPtr) const
{
  return *(reinterpret_cast<const int*>(pConjPtr) + 1);
}

inline int StaticSynthesizerDictionary::xGetFemSingularlPtr
(const signed char* pConjPtr) const
{
  return *(reinterpret_cast<const int*>(pConjPtr) + 2);
}

inline int StaticSynthesizerDictionary::xGetFemPluralPtr
(const signed char* pConjPtr) const
{
  return *(reinterpret_cast<const int*>(pConjPtr) + 3);
}

inline int StaticSynthesizerDictionary::xGetNeutralSingularPtr
(const signed char* pConjPtr) const
{
  return *(reinterpret_cast<const int*>(pConjPtr) + 4);
}

inline int StaticSynthesizerDictionary::xGetneutralPluralPtr
(const signed char* pConjPtr) const
{
  return *(reinterpret_cast<const int*>(pConjPtr) + 5);
}

inline int StaticSynthesizerDictionary::xGetEnglishComparativePtr
(const signed char* pConjPtr) const
{
  return *(reinterpret_cast<const int*>(pConjPtr) + 6);
}

} // End of namespace linguistics
} // End of namespace onsem

#endif // ONSEM_TEXTTOSEMANTIC_TYPE_LINGUISTICDATABASE_STATICSYNTHESIZERDICTIONARY_HXX
