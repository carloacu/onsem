#include <onsem/lingdbeditor/meaningandconfidence.hpp>
#include <onsem/lingdbeditor/allingdbdynamictrienode.hpp>
#include <string>


namespace onsem
{


bool MeaningAndConfidence::operator<
(const MeaningAndConfidence& pOther) const
{
  if (confidence != pOther.confidence)
  {
    return confidence > pOther.confidence;
  }
  std::string currLemme = meaning->getLemma()->getWord();
  std::string otherLemme = pOther.meaning->getLemma()->getWord();
  if (currLemme != otherLemme)
  {
    return currLemme < otherLemme;
  }
  return meaning->getPartOfSpeech() < pOther.meaning->getPartOfSpeech();
}



} // End of namespace onsem

