#ifndef ALLINGDBEDITOR_MEANINGANDONFIDENCE_H
#define ALLINGDBEDITOR_MEANINGANDONFIDENCE_H

#include <onsem/lingdbeditor/allingdbmeaning.hpp>


namespace onsem
{

struct MeaningAndConfidence
{
  MeaningAndConfidence
  (ALLingdbMeaning* pMeaning,
   char pConfidence)
    : meaning(pMeaning),
      confidence(pConfidence)
  {
  }

  MeaningAndConfidence
  (const MeaningAndConfidence& pObj)
    : meaning(pObj.meaning),
      confidence(pObj.confidence)
  {
  }

  bool operator<
  (const MeaningAndConfidence& pOther) const;

  ALLingdbMeaning* meaning;
  char confidence;
};


} // End of namespace onsem


#endif // ALLINGDBEDITOR_MEANINGANDONFIDENCE_H
