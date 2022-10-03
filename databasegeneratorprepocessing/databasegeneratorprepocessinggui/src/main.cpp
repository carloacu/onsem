#include "dbGeneratorpreprocessingwindow.hpp"
#include <QApplication>
#include <iostream>
#include <boost/filesystem.hpp>
#include <onsem/compilermodel/lingdbtree.hpp>
#include <onsem/streamdatabaseaccessor/streamdatabaseaccessor.hpp>

using namespace onsem;


int main(int argc, char *argv[])
{
  std::string buildRootPath;
  std::string shareDbFolder;
  for (int i = 0; i < argc; ++i)
  {
    if ((i + 1) < argc)
    {
      const std::string currAgrv = argv[i];
      if (currAgrv == "--build_root")
        buildRootPath = argv[i + 1];
      if (currAgrv == "--share_db")
        shareDbFolder = argv[i + 1];
    }
  }
  static const std::string exOfCommandLine = "--build_root ../.. --share_db ../../../share";

  const auto inputResourcesFolder(shareDbFolder + "/linguistic/inputresources");
  const auto loadDatabasesFolder(shareDbFolder + "/linguistic/loaddatabases");
  const auto shareSemanticPath(shareDbFolder + "/semantic");
  const auto dynamicDictionaryPath(shareSemanticPath + "/dynamicdictionary");
  std::string lingDbPath = buildRootPath + "/linguistic/databases";

  std::string preprocessingDbPath = buildRootPath + "/preprocessing";
  boost::filesystem::create_directory(preprocessingDbPath);
  onsem::LingdbTree lingDbTree(inputResourcesFolder);
  lingDbTree.update(preprocessingDbPath, loadDatabasesFolder, "", false);

  auto istreams = linguistics::generateIStreams(lingDbPath, dynamicDictionaryPath);

  QApplication a(argc, argv);

  DbGeneratorPreprocessingWindow w(lingDbTree, shareDbFolder, inputResourcesFolder, istreams.linguisticDatabaseStreams);
  w.show();
  istreams.close();

  return a.exec();
}
