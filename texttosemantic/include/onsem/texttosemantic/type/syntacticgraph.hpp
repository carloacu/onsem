#ifndef ONSEM_TEXTTOSEMANTIC_TYPE_SYNTACTICGRAPH_HPP
#define ONSEM_TEXTTOSEMANTIC_TYPE_SYNTACTICGRAPH_HPP

#include <list>
#include "chunklink.hpp"
#include "../api.hpp"
#include <onsem/texttosemantic/algorithmsetforalanguage.hpp>

namespace onsem
{
namespace linguistics
{


struct ONSEM_TEXTTOSEMANTIC_API SyntacticGraph
{
  SyntacticGraph(const LinguisticDatabase& pLingDb,
                 SemanticLanguageEnum pLanguageType)
    : langConfig(pLingDb, pLanguageType),
      tokensTree(),
      firstChildren()
  {
  }

  SyntacticGraph(const SyntacticGraph&) = delete;
  SyntacticGraph& operator=(const SyntacticGraph&) = delete;

  AlgorithmSetForALanguage langConfig;

  TokensTree tokensTree;

  // first children of the syntatic graph
  std::list<ChunkLink> firstChildren;
};


} // End of namespace linguistics
} // End of namespace onsem


#endif // ONSEM_TEXTTOSEMANTIC_TYPE_SYNTACTICGRAPH_HPP
