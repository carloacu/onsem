#ifndef ALCPTSDATABASELOADER_H
#define ALCPTSDATABASELOADER_H

#include <boost/filesystem/path.hpp>
#include <string>

namespace onsem
{
class LinguisticIntermediaryDatabase;
class ALLingdbConcept;


class ALCptsDatabaseLoader
{
public:
  void merge
  (const boost::filesystem::path& pFilename,
   LinguisticIntermediaryDatabase& pLingdb);

private:
  static void _fillConcept
  (LinguisticIntermediaryDatabase& pLingdb,
   const std::string& pWord,
   const ALLingdbConcept* pNewConcept,
   const std::string& pNewConceptStr,
   char pRelatedToConcept,
   const boost::filesystem::path& pFilename,
   const std::string& pLine);

};



} // End of namespace onsem

#endif // ALCPTSDATABASELOADER_H
