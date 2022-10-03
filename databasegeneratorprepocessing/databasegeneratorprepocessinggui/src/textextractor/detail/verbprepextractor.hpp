#ifndef TEXTEXTRACTOR_VERBPREPEXTRACTOR_H
#define TEXTEXTRACTOR_VERBPREPEXTRACTOR_H

#include <map>
#include <string>
#include <onsem/typeallocatorandserializer/typeallocatorandserializer.hpp>
#include <onsem/texttosemantic/dbtype/semanticword.hpp>
#include "metaextractor.hpp"


namespace onsem
{
namespace linguistics
{
struct SyntacticGraph;
struct Chunk;
struct ChunkLink;
}


class VerbPrepExtractor : public MetaExtractor
{
public:
  VerbPrepExtractor();
  VerbPrepExtractor(const std::vector<VerbPrepExtractor>& pOtherVerbPrepExtractors);

  void processText
  (const linguistics::SyntacticGraph& pSyntGraph,
   const std::string& pText);

  void writeResults
  (const std::string& pResultFilename) const;

private:
  SemanticWord fAQuiPronWord;
  SemanticWord fAQuiConjSWord;
  std::map<std::string, int> fVerbsToFreq;

  void xInit();

  void xProcessChunkLinkList
  (const std::list<linguistics::ChunkLink>& pChunkLinks,
   const std::string& pText);

  void xAddSubVerbIfInterestingChunkLink
  (const linguistics::ChunkLink& pChunkLink);

  void xAddChunkHead
  (const linguistics::Chunk& pChunk);
};

} // End of namespace onsem

#endif // TEXTEXTRACTOR_VERBPREPEXTRACTOR_H
