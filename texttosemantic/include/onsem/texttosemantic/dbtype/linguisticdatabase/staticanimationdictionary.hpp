#ifndef ONSEM_TEXTTOSEMANTIC_TYPE_LINGUISTICDATABASE_STATICANIMATIONDICTIONARY_HPP
#define ONSEM_TEXTTOSEMANTIC_TYPE_LINGUISTICDATABASE_STATICANIMATIONDICTIONARY_HPP

#include <map>
#include <set>
#include <vector>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/detail/virtualsembinarydatabase.hpp>
#include <onsem/common/enum/semanticlanguagetype.hpp>
#include <onsem/texttosemantic/dbtype/linguisticmeaning.hpp>
#include "../../api.hpp"

namespace onsem
{
class StaticConceptSet;
namespace linguistics
{

class ONSEM_TEXTTOSEMANTIC_API StaticAnimationDictionary : public VirtualSemBinaryDatabase
{
public:
  StaticAnimationDictionary(std::istream& pIstream,
                            const StaticConceptSet& pConceptsDb,
                            SemanticLanguageEnum pLangEnum);

  ~StaticAnimationDictionary();

  SemanticLanguageEnum getLanguageEnum() const { return fLangEnum; }

  void getAnimTagOfAMeaning
  (std::string& pAnimTag,
   int& pRela,
   const StaticLinguisticMeaning& pMeaning) const;


  void getAnimTagOfConcepts
  (std::string& pAnimTag,
   int& pRela,
   const std::map<char, std::map<std::string, char>>& pConcepts) const;



private:
  union AnimDatabaseHeader
  {
    int intValues[4];
    char charValues[16];
  };

  const SemanticLanguageEnum fLangEnum;
  const StaticConceptSet& fStaticConceptSet;
  std::map<std::string, std::map<char, std::string> > fConceptsToMinCptValueToAnimTag;

  struct AnimConcept
  {
    std::string tag;
    char rela;
  };

  std::map<int, AnimConcept> fMeaningsToAnimTag;


  void xLoad(std::istream& pIstream);

  void xUnload();

  virtual bool xIsLoaded() const;

  signed char* fDataPtr;
};



} // End of namespace linguistics
} // End of namespace onsem

#endif // ONSEM_TEXTTOSEMANTIC_TYPE_LINGUISTICDATABASE_STATICANIMATIONDICTIONARY_HPP

