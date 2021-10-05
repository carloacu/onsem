#include <onsem/lingdbeditor/loaders/alanydatabaseloader.hpp>
#include <onsem/common/utility/string.hpp>
#include <onsem/lingdbeditor/linguisticintermediarydatabase.hpp>
#include <onsem/lingdbeditor/allingdbtree.hpp>
#include <onsem/lingdbeditor/loaders/aldeladatabaseloader.hpp>
#include <onsem/lingdbeditor/loaders/alwlksdatabaseloader.hpp>
#include "alxmldatabaseloader.hpp"
#include "alrladatabaseloader.hpp"
#include "algfsdatabaseloader.hpp"
#include "alcptsdatabaseloader.hpp"


namespace onsem
{


void ALAnyDatabaseLoader::open
(const boost::filesystem::path& pFilename,
 const boost::filesystem::path& pInputResourcesDir,
 LinguisticIntermediaryDatabase& pCurrLingdb,
 const ALLingdbTree& pLingdbTree,
 std::size_t pImbricationLevel) const
{
  std::string suffixStr = mystd::filenameToSuffix(pFilename.string());

  if (suffixStr == pLingdbTree.getExtDynDatabase())
  {
    pCurrLingdb.load(pFilename);
  }
  else if (suffixStr == pLingdbTree.getExtXmlDatabase())
  {
    pCurrLingdb.clear();
    ALXmlDatabaseLoader::merge(pFilename, pCurrLingdb, pLingdbTree);
  }
  else if (suffixStr == pLingdbTree.getExtDelaDatabase())
  {
    pCurrLingdb.clear();
    ALDelaDatabaseLoader ddl;
    ddl.merge(pFilename, pCurrLingdb);
  }
  else if (suffixStr == pLingdbTree.getRlaDatabase())
  {
    pCurrLingdb.clear();
    ALRlaDatabaseLoader::merge(pFilename, pInputResourcesDir, pCurrLingdb,
                               pLingdbTree, *this, pImbricationLevel);
  }
  else if (suffixStr == pLingdbTree.getCptsDatabase())
  {
    pCurrLingdb.clear();
    ALCptsDatabaseLoader cptsLoader;
    cptsLoader.merge(pFilename, pCurrLingdb);
  }
}


void ALAnyDatabaseLoader::mergeWith
(const boost::filesystem::path& pFilename,
 const boost::filesystem::path& pInputResourcesDir,
 LinguisticIntermediaryDatabase& pCurrLingdb,
 const ALLingdbTree& pLingdbTree,
 std::size_t pImbricationLevel) const
{
  std::string suffixStr = mystd::filenameToSuffix(pFilename.string());

  if (suffixStr == pLingdbTree.getExtXmlDatabase())
  {
    ALXmlDatabaseLoader::merge(pFilename, pCurrLingdb, pLingdbTree);
  }
  else if (suffixStr == pLingdbTree.getExtDelaDatabase())
  {
    ALDelaDatabaseLoader ddl;
    ddl.merge(pFilename, pCurrLingdb);
  }
  else if (suffixStr == pLingdbTree.getExtGfsDatabase())
  {
    ALGFSDatabaseLoader::merge(pFilename, pCurrLingdb);
  }
  else if (suffixStr == pLingdbTree.getRlaDatabase())
  {
    ALRlaDatabaseLoader::merge(pFilename, pInputResourcesDir, pCurrLingdb,
                               pLingdbTree, *this, pImbricationLevel);
  }
  else if (suffixStr == pLingdbTree.getCptsDatabase())
  {
    ALCptsDatabaseLoader cptsLoader;
    cptsLoader.merge(pFilename, pCurrLingdb);
  }
}


void ALAnyDatabaseLoader::exec
(const boost::filesystem::path& pFilename,
 const ALLingdbTree& pLingdbTree) const
{
  std::string suffixStr = mystd::filenameToSuffix(pFilename.string());

  if (suffixStr == pLingdbTree.getWlksDatabase())
  {
    ALWlksDatabaseLoader wlksLoader;
    wlksLoader.loadAndSave(pFilename, pLingdbTree);
    return;
  }
  throw std::runtime_error("Cannot exec file: \"" + pFilename.string() + "\".");
}


} // End of namespace onsem
