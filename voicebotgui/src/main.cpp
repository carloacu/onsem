#include "mainwindow.h"
#include <QtGlobal>
#include <QApplication>
#include <onsem/streamdatabaseaccessor/streamdatabaseaccessor.hpp>

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);

  std::string lingDbPath;
  std::string shareSemantic;
  for (int i = 0; i < argc; ++i)
  {
    if ((i + 1) < argc)
    {
      const std::string currAgrv = argv[i];
      if (currAgrv == "--databases")
        lingDbPath = argv[i + 1];
      else if (currAgrv == "--share_semantic")
        shareSemantic = argv[i + 1];
    }
  }
  static const std::string exOfCommandLine = "--databases ../linguistic/databases --share_semantic ../../share/semantic";
  if (lingDbPath.empty())
    throw std::runtime_error("--databases option is missing (ex: " + exOfCommandLine + ")");
  if (shareSemantic.empty())
    throw std::runtime_error("--share_semantic option is missing (ex: " + exOfCommandLine + ")");

  const auto corpusFolder(shareSemantic + "/corpus");
  const auto scenariosFolder(shareSemantic + "/scenarios");
  const auto dynamicDictionaryPath(shareSemantic + "/dynamicdictionary");

  const auto corpusEquivalencesFolder(corpusFolder + "/equivalences");
  const auto corpusResultsFolder(corpusFolder + "/results");
  const auto inputScenariosFolder(scenariosFolder + "/input");
  const auto outputScenariosFolder(scenariosFolder + "/output");

  auto istreams = linguistics::generateIStreams(lingDbPath, dynamicDictionaryPath);
  MainWindow w(corpusEquivalencesFolder, corpusResultsFolder,
               inputScenariosFolder, outputScenariosFolder, corpusFolder,
               istreams.linguisticDatabaseStreams);
  istreams.close();
  w.show();

  return a.exec();
}
