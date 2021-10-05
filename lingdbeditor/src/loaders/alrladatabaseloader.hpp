#ifndef ALRLADATABASELOADER_H
#define ALRLADATABASELOADER_H

#include <string>


namespace onsem
{
class LinguisticIntermediaryDatabase;
class ALLingdbTree;
class ALAnyDatabaseLoader;


class ALRlaDatabaseLoader
{
public:
  static void merge
  (const std::string& pFilename,
   const std::string& pInputResourcesDir,
   LinguisticIntermediaryDatabase& pCurrLingdb,
   const ALLingdbTree& pLingdbTree,
   const ALAnyDatabaseLoader& pAnyLoader,
   std::size_t pImbricationLevel);

private:
  static std::string _getPath
  (const std::string& pLine,
   const std::string& pInstruction,
   const std::string& pHoldingFolder,
   const std::string& pInputResourcesDir);
};



} // End of namespace onsem

#endif // ALRLADATABASELOADER_H
