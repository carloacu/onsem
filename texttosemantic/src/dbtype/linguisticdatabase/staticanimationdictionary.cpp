#include <onsem/texttosemantic/dbtype/linguisticdatabase/staticanimationdictionary.hpp>
#include <onsem/common/binary/binaryloader.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/staticlinguisticdictionary.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/staticconceptset.hpp>


namespace onsem
{
namespace linguistics
{

StaticAnimationDictionary::StaticAnimationDictionary(std::istream& pIstream,
                                                     const StaticConceptSet& pConceptsDb,
                                                     SemanticLanguageEnum pLangEnum)
  : VirtualSemBinaryDatabase(),
    fLangEnum(pLangEnum),
    fStaticConceptSet(pConceptsDb),
    fConceptsToMinCptValueToAnimTag(),
    fMeaningsToAnimTag(),
    fDataPtr(nullptr)
{
  xLoad(pIstream);
}

StaticAnimationDictionary::~StaticAnimationDictionary()
{
  xUnload();
}


bool StaticAnimationDictionary::xIsLoaded() const
{
  return !fConceptsToMinCptValueToAnimTag.empty() ||
      !fMeaningsToAnimTag.empty();
}


void StaticAnimationDictionary::xUnload()
{
  fConceptsToMinCptValueToAnimTag.clear();
  fMeaningsToAnimTag.clear();
  fTotalSize = 0;
  fErrorMessage = "NOT_LOADED";
  binaryloader::deallocMemZone(&fDataPtr);
}


void StaticAnimationDictionary::xLoad(std::istream& pIstream)
{
  assert(!xIsLoaded());
  AnimDatabaseHeader header;
  pIstream.read(header.charValues, sizeof(AnimDatabaseHeader));
  if (header.intValues[0] != fFormalism)
  {
    fErrorMessage = "BAD_FORMALISM";
    return;
  }

  int nbTagAnims = header.intValues[2];
  fTotalSize = static_cast<std::size_t>(header.intValues[3]);
  if (!binaryloader::allocMemZone(&fDataPtr, pIstream, fTotalSize))
  {
    xUnload();
    fErrorMessage = "BAD_ALLOC";
    return;
  }

  // Close database file
  fErrorMessage = "";

  signed char const* currPtr = fDataPtr;
  for (int i = 0; i < nbTagAnims; ++i)
  {
    std::string animTag;
    currPtr = binaryloader::getStrWithOffset(animTag, currPtr);
    char nbOfLksToConcept = *(currPtr++);
    char nbOfMeanings = *(currPtr++);
    currPtr = binaryloader::alignMemory(currPtr);

    // read concepts for the current animation tag
    for (char lkToCpt = 0; lkToCpt < nbOfLksToConcept; ++lkToCpt)
    {
      const int* currIntPtr = reinterpret_cast<const int*>(currPtr);
      std::string cptName = fStaticConceptSet.conceptName(binaryloader::alignedDecToInt(*currIntPtr));
      currPtr += 3;
      char minCptValue = *(currPtr++);
      fConceptsToMinCptValueToAnimTag[cptName][minCptValue] = animTag;
    }

    // read meanings for the current animation tag
    for (char iMean = 0; iMean < nbOfMeanings; ++iMean)
    {
      const int* currIntPtr = reinterpret_cast<const int*>(currPtr);
      int meanId = binaryloader::alignedDecToInt(*currIntPtr);
      currPtr += 3;
      char rela = *(currPtr++);
      AnimConcept tempConcept= {animTag, rela};

      fMeaningsToAnimTag[meanId] = tempConcept;
    }
  }
}


void StaticAnimationDictionary::getAnimTagOfAMeaning
(std::string& pAnimTag,
 int& pRela,
 const StaticLinguisticMeaning& pMeaning) const
{
  if (!xIsLoaded())
    return;
  assert(pMeaning.language == fLangEnum);

  auto it = fMeaningsToAnimTag.find(pMeaning.meaningId);
  if (it != fMeaningsToAnimTag.end())
  {
    pAnimTag = it->second.tag;
    pRela = it->second.rela;
  }
}



void StaticAnimationDictionary::getAnimTagOfConcepts
(std::string& pAnimTag,
 int& pRela,
 const std::map<char,std::map<std::string, char>>& pConcepts) const
{
  if (!xIsLoaded())
    return;
  for (std::map<char, std::map<std::string, char>>::const_reverse_iterator
       itCptsForAConf = pConcepts.rbegin(); itCptsForAConf != pConcepts.rend(); ++itCptsForAConf)
  {
    for (const auto& currCpt : itCptsForAConf->second)
    {
      auto itCptToTag = fConceptsToMinCptValueToAnimTag.find(currCpt.first);
      if (itCptToTag != fConceptsToMinCptValueToAnimTag.end())
      {
        for (const auto& currConfToAnimTag : itCptToTag->second)
        {
          bool valueIsOk = false;
          char minValue = currConfToAnimTag.first;
          if (minValue > 0)
          {
            valueIsOk = currCpt.second >= minValue;
          }
          else
          {
            valueIsOk = currCpt.second <= minValue;
          }
          if (valueIsOk)
          {
            pAnimTag = currConfToAnimTag.second;
            pRela = currCpt.second;

            return;
          }
        }
      }
    }
  }
}


} // End of namespace linguistics
} // End of namespace onsem
