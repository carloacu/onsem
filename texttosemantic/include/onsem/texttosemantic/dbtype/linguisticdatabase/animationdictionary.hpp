#ifndef ONSEM_TEXTTOSEMANTIC_TYPE_LINGUISTICDATABASE_ANIMATIONDICTIONARY_HPP
#define ONSEM_TEXTTOSEMANTIC_TYPE_LINGUISTICDATABASE_ANIMATIONDICTIONARY_HPP

#include <mutex>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/staticanimationdictionary.hpp>
#include "../../api.hpp"

namespace onsem
{
namespace linguistics
{

class ONSEM_TEXTTOSEMANTIC_API AnimationDictionary
{
public:
  AnimationDictionary(std::istream* pIstreamPtr,
                      const StaticConceptSet& pConceptsDb,
                      SemanticLanguageEnum pLangEnum);

  const StaticAnimationDictionary& statDb;

private:
  static std::mutex _pathToStatDbsMutex;
  static std::map<SemanticLanguageEnum, std::unique_ptr<StaticAnimationDictionary>> _statDbs;
  static const StaticAnimationDictionary& _getStatDbInstance(std::istream* pIstreamPtr,
                                                             const StaticConceptSet& pConceptsDb,
                                                             SemanticLanguageEnum pLangEnum);


};


} // End of namespace linguistics
} // End of namespace onsem

#endif // ONSEM_TEXTTOSEMANTIC_TYPE_LINGUISTICDATABASE_ANIMATIONDICTIONARY_HPP

