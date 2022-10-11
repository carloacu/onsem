#include <onsem/texttosemantic/dbtype/linguisticdatabase/animationdictionary.hpp>

namespace onsem
{
namespace linguistics
{
std::mutex AnimationDictionary::_pathToStatDbsMutex{};
std::map<SemanticLanguageEnum, std::unique_ptr<StaticAnimationDictionary>> AnimationDictionary::_statDbs{};


const StaticAnimationDictionary& AnimationDictionary::_getStatDbInstance(std::istream& pIstream,
                                                                         const StaticConceptSet& pConceptsDb,
                                                                         SemanticLanguageEnum pLangEnum)
{
  std::lock_guard<std::mutex> lock(_pathToStatDbsMutex);
  auto it = _statDbs.find(pLangEnum);
  if (it == _statDbs.end())
  {
    auto& res = _statDbs[pLangEnum];
    res = std::make_unique<StaticAnimationDictionary>(pIstream, pConceptsDb, pLangEnum);
    return *res;
  }
  return *it->second;
}


AnimationDictionary::AnimationDictionary(std::istream& pIstream,
                                         const StaticConceptSet& pConceptsDb,
                                         SemanticLanguageEnum pLangEnum)
  : statDb(_getStatDbInstance(pIstream, pConceptsDb, pLangEnum))
{
}



} // End of namespace linguistics
} // End of namespace onsem
