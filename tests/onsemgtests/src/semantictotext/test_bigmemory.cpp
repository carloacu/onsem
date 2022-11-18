#include "../semanticreasonergtests.hpp"
#include <gtest/gtest.h>
#include <onsem/texttosemantic/dbtype/linguisticdatabase.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemory.hpp>


using namespace onsem;



TEST_F(SemanticReasonerGTests, test_bigMemory)
{
  auto iStreams = linguistics::generateIStreams(lingDbPath, dynamicdictionaryPath);
  linguistics::LinguisticDatabase lingDb(iStreams.linguisticDatabaseStreams);
  iStreams.close();
  SemanticMemory semMem;


  const std::string textFilename = corpusPath + "/triggerAndTexts/triggersAndTextsCorpus.txt";
  std::ifstream triggersAndTextsCorpusFile(textFilename, std::ifstream::in);
  if (!triggersAndTextsCorpusFile.is_open())
    throw std::runtime_error("Can't open " + textFilename + " file !");

  std::string currentLabel;
  std::string currentText;

  std::string line;
  while (getline(triggersAndTextsCorpusFile, line))
  {
    if (line.empty())
      continue;
    if (line[0] == '#')
    {
      auto endOfLabel = line.find_first_of('#', 1);
      auto label = line.substr(1, endOfLabel - 1);
      if (label == "trigger" || label == "message")
      {
        std::cout << "~" << currentLabel << " -> " << currentText << std::endl;
        currentLabel = label;
        currentText = line.substr(endOfLabel + 1, line.size() - 1);
        continue;
      }
    }

    currentText += "\n" + line;
  }
  triggersAndTextsCorpusFile.close();

}

