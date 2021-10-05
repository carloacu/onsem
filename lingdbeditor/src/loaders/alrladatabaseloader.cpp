#include "alrladatabaseloader.hpp"
#include <iostream>
#include <boost/filesystem/fstream.hpp>
#include <onsem/lingdbeditor/allingdbtree.hpp>
#include <onsem/lingdbeditor/loaders/alanydatabaseloader.hpp>
#include <onsem/lingdbeditor/linguisticintermediarydatabase.hpp>


namespace onsem
{

void ALRlaDatabaseLoader::merge
(const boost::filesystem::path& pFilename,
 const boost::filesystem::path& pInputResourcesDir,
 LinguisticIntermediaryDatabase& pCurrLingdb,
 const ALLingdbTree& pLingdbTree,
 const ALAnyDatabaseLoader& pAnyLoader,
 std::size_t pImbricationLevel)
{
  boost::filesystem::ifstream infile(pFilename, boost::filesystem::ifstream::in);
  if (!infile.is_open())
  {
    std::cerr << "Error: Can't open " << pFilename.string() << " file !" << std::endl;
    return;
  }

  boost::filesystem::path holdingFolder;
  pLingdbTree.getHoldingFolder(holdingFolder, pFilename);

  std::string line;
  while (getline(infile, line))
  {
    if (line.empty() || line[0] == '%')
    {
      continue;
    }

    for (std::size_t i = 0; i < pImbricationLevel; ++i)
    {
      std::cout << "  ";
    }
    std::cout << line << std::endl;
    {
      std::string openInstruction = "open ";
      if (line.size() > openInstruction.size() &&
          line.compare(0, openInstruction.size(), openInstruction) == 0)
      {
        pAnyLoader.open(_getPath(line, openInstruction, holdingFolder, pInputResourcesDir),
                        pInputResourcesDir, pCurrLingdb, pLingdbTree, pImbricationLevel + 1);
        continue;
      }
    }
    {
      std::string mergeInstruction = "merge ";
      if (line.size() > mergeInstruction.size() &&
          line.compare(0, mergeInstruction.size(), mergeInstruction) == 0)
      {
        pAnyLoader.mergeWith(_getPath(line, mergeInstruction, holdingFolder, pInputResourcesDir),
                             pInputResourcesDir,
                             pCurrLingdb, pLingdbTree, pImbricationLevel + 1);
        continue;
      }
    }
    {
      std::string execInstruction = "exec ";
      if (line.size() > execInstruction.size() &&
          line.compare(0, execInstruction.size(), execInstruction) == 0)
      {
        pAnyLoader.exec(_getPath(line, execInstruction, holdingFolder, pInputResourcesDir),
                        pLingdbTree);
        continue;
      }
    }
    if (line == "clear alldatabases")
    {
      pLingdbTree.clearAll();
      continue;
    }
    if (line == "save")
    {
      if (pCurrLingdb.getLanguage() == nullptr ||
          pCurrLingdb.getLanguage()->toStr().empty())
      {
        throw std::runtime_error("The current database has no language associated");
      }
      pCurrLingdb.save(pLingdbTree.getDynamicDatabasesFolder() /
                       boost::filesystem::path(pCurrLingdb.getLanguage()->toStr() + "." +
                                pLingdbTree.getExtDynDatabase()));
      continue;
    }
    throw std::runtime_error("instruction unknown: " + line);
  }

  infile.close();
}


boost::filesystem::path ALRlaDatabaseLoader::_getPath
(const std::string& pLine,
 const std::string& pInstruction,
 const boost::filesystem::path& pHoldingFolder,
 const boost::filesystem::path& pInputResourcesDir)
{
  std::string pathStr = pLine.substr(pInstruction.size(), pLine.size() - pInstruction.size());
  {
    const std::string inputResourceLabel = "[inputresources]";
    std::size_t posInputResource = pathStr.find(inputResourceLabel);
    if (posInputResource != std::string::npos)
    {
      pathStr.replace(posInputResource, inputResourceLabel.size(), pInputResourcesDir.string());
      return boost::filesystem::path(pathStr);
    }
  }

  return pHoldingFolder / boost::filesystem::path(pathStr);
}



} // End of namespace onsem
