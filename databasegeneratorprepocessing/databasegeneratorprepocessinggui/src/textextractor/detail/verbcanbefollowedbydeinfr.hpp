#ifndef TEXTEXTRACTOR_VERBCANBEFOLLOWEDBYDEINFR_HPP
#define TEXTEXTRACTOR_VERBCANBEFOLLOWEDBYDEINFR_HPP

#include <map>
#include <string>
#include <vector>
#include <onsem/typeallocatorandserializer/typeallocatorandserializer.hpp>
#include "metaextractor.hpp"

namespace onsem
{
namespace linguistics
{
struct SyntacticGraph;
struct ChunkLink;
}


class VerbCanBeFollowedByDeInFr : public MetaExtractor
{
public:
  VerbCanBeFollowedByDeInFr(const CompositePoolAllocator& pAlloc);
  VerbCanBeFollowedByDeInFr(const std::vector<VerbCanBeFollowedByDeInFr>& pOthers);

  void processText
  (const linguistics::SyntacticGraph& pSyntGraph,
   const std::string& pText);

  void writeResults
  (const std::string& pResultFilename) const;

  static void writeXml
  (const std::string& pResultFilename,
   const std::string& pResultFilenameXml);

private:
  struct FollwoedByDeStats
  {
    FollwoedByDeStats()
      : nbOccs(0),
        nbFollowedByDe(0)
    {
    }

    void add(const FollwoedByDeStats& pOtherAuxStats)
    {
      nbOccs += pOtherAuxStats.nbOccs;
      nbFollowedByDe += pOtherAuxStats.nbFollowedByDe;
    }

    int nbOccs;
    int nbFollowedByDe;
  };
  std::map<std::string, FollwoedByDeStats> _verbStats;

  void xProcessChunkLinkList
  (const std::list<linguistics::ChunkLink>& pChunkLinks,
   const std::string& pText);
};

} // End of namespace onsem

#endif // TEXTEXTRACTOR_VERBCANBEFOLLOWEDBYDEINFR_HPP
