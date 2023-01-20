#ifndef TEXTEXTRACTOR_ADJBEFORENOUNEXTRACTOR_H
#define TEXTEXTRACTOR_ADJBEFORENOUNEXTRACTOR_H

#include <list>
#include <map>
#include <string>
#include <onsem/typeallocatorandserializer/typeallocatorandserializer.hpp>
#include "metaextractor.hpp"


namespace onsem
{
namespace linguistics
{
struct InflectedWord;
struct TokensTree;
}


class AdjBeforeNounExtractor : public MetaExtractor
{
public:
  AdjBeforeNounExtractor(const CompositePoolAllocator& pAlloc);
  AdjBeforeNounExtractor(const std::vector<AdjBeforeNounExtractor>& pOtherAdjBeforeNounExtractors);

  void processText
  (const linguistics::TokensTree& pTokensTree);

  void writeResults
  (const std::string& pResultFilename) const;

  static void writeXml
  (const std::string& pResultFilename,
   const std::string& pResultFilenameXml);

private:
  struct AdjStats
  {
    AdjStats()
      : nbBeforeNoun(0),
        nbAfterNoun(0)
    {
    }

    void add(const AdjStats& pOtherAdjStats)
    {
      nbBeforeNoun += pOtherAdjStats.nbBeforeNoun;
      nbAfterNoun += pOtherAdjStats.nbAfterNoun;
    }

    int nbBeforeNoun;
    int nbAfterNoun;
  };
  std::map<std::string, AdjStats> fAdjStats;

  void xRefreshAdjPositionInfo
  (const std::list<linguistics::InflectedWord>& pIGrams,
   bool pBeforeAfterNoun);
};

} // End of namespace onsem


#endif // TEXTEXTRACTOR_ADJBEFORENOUNEXTRACTOR_H
