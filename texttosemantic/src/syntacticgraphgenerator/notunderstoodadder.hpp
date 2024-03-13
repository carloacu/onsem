#ifndef ONSEM_TEXTTOSEMANTIC_SRC_SYNTACTICGRAPHGENERATOR_NOTUNDERSTOODADDER_HPP
#define ONSEM_TEXTTOSEMANTIC_SRC_SYNTACTICGRAPHGENERATOR_NOTUNDERSTOODADDER_HPP

#include <list>
#include <set>
#include <onsem/texttosemantic/dbtype/misc/spellingmistaketype.hpp>

namespace onsem {
namespace linguistics {
struct ChunkLink;
class InflectionsChecker;
class LinguisticDictionary;

bool addNotUnderstood(std::list<ChunkLink>& pChunkList,
                      std::size_t& pNbOfNotUnderstood,
                      std::size_t& pNbOfSuspiciousChunks,
                      const std::set<SpellingMistakeType>& pSpellingMistakeTypesPossible,
                      const InflectionsChecker& pInlfChecker,
                      const linguistics::LinguisticDictionary& pLingDico);

}    // End of namespace linguistics
}    // End of namespace onsem

#endif    // ONSEM_TEXTTOSEMANTIC_SRC_SYNTACTICGRAPHGENERATOR_NOTUNDERSTOODADDER_HPP
