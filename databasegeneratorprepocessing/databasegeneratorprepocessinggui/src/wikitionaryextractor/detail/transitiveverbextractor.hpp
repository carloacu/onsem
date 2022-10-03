#ifndef SEMANTICDATABASEVIEWER_WIKITIONARYDATAEXTRACTOR_TRANSITIVEVERBEXTRACTOR_HPP
#define SEMANTICDATABASEVIEWER_WIKITIONARYDATAEXTRACTOR_TRANSITIVEVERBEXTRACTOR_HPP

#include <set>
#include <string>

class PatternRecognizer;
namespace onsem
{
class LingdbMeaning;
class WikiKeyWords;
class LinguisticIntermediaryDatabase;


class WikiTransitiveVerbExtractor
{
public:
  WikiTransitiveVerbExtractor
  (const WikiKeyWords& pWikikeyWords,
   const PatternRecognizer& pPatternRecognizer);

  void extract
  (std::set<LingdbMeaning*>& pTransitiveVerbs,
   std::ifstream& pWikionaryFile,
   const LinguisticIntermediaryDatabase& pLingDatabase);

  void writeNewTransitiveVerbs
  (const std::set<LingdbMeaning*>& pTransitiveVerbs,
   const std::string& poutFile);

private:
  const WikiKeyWords& fWikikeyWords;
  const PatternRecognizer& fPatternReco;
};


} // End of namespace onsem


#endif // SEMANTICDATABASEVIEWER_WIKITIONARYDATAEXTRACTOR_TRANSITIVEVERBEXTRACTOR_HPP
