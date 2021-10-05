#ifndef ALANYDATABASELOADER_H
#define ALANYDATABASELOADER_H

#include <string>


namespace onsem
{
class LinguisticIntermediaryDatabase;
class ALLingdbTree;


class ALAnyDatabaseLoader
{
public:
  void open
  (const std::string& pFilename,
   const std::string& pInputResourcesDir,
   LinguisticIntermediaryDatabase& pLingdb,
   const ALLingdbTree& pLingdbTree,
   std::size_t pImbricationLevel = 0) const;

  void mergeWith
  (const std::string& pFilename,
   const std::string& pInputResourcesDir,
   LinguisticIntermediaryDatabase& pLingdb,
   const ALLingdbTree& pLingdbTree,
   std::size_t pImbricationLevel = 0) const;

  void exec
  (const std::string& pFilename,
   const ALLingdbTree& pLingdbTree) const;
};



} // End of namespace onsem

#endif // ALANYDATABASELOADER_H
