#include "aspiredhextractor.hpp"
#include <fstream>
#include <sstream>
#include <onsem/texttosemantic/dbtype/linguisticdatabase.hpp>

namespace onsem
{
namespace aspiredHExtractor
{


void run(const std::string& pMyDataMiningPath,
         const std::string& pInputResourcesFolder,
         const linguistics::LinguisticDatabase& pLingDb)
{
  const std::string inFilename = pMyDataMiningPath + "/resources/french/french_aspiredh.txt";

  std::ifstream inFile(inFilename, std::ifstream::in);
  if (!inFile.is_open())
    throw std::runtime_error("Error: Can't open " + inFilename + " file !");

  const linguistics::StaticLinguisticDictionary& staticBinDico =
      pLingDb.langToSpec[SemanticLanguageEnum::FRENCH].lingDico.statDb;

  std::ofstream outFile(pInputResourcesFolder + "/french/readonly/french_contextinfos_aspiredh.xml");
  outFile << "<linguisticdatabase>\n";
  std::string line;
  while (getline(inFile, line))
  {
    std::size_t endOfWordPos = std::string::npos;
    std::size_t beginOfParenthesis = line.find('(');

    if (beginOfParenthesis != std::string::npos)
      endOfWordPos = beginOfParenthesis;

    std::size_t beginOfComma = line.find(',');
    if (beginOfComma != std::string::npos && (endOfWordPos == std::string::npos || beginOfComma < endOfWordPos))
      endOfWordPos = beginOfComma;

    if (endOfWordPos != std::string::npos && endOfWordPos > 1)
    {
      std::list<linguistics::InflectedWord> inflectedWords;
      staticBinDico.getGramPossibilities(inflectedWords, line, 0, endOfWordPos - 1);
      for (const auto& currInflectedWord : inflectedWords)
      {
        outFile << "<existingMeaning lemme=\"" << currInflectedWord.word.lemma << "\" gram=\""
                << partOfSpeech_toStr(currInflectedWord.word.partOfSpeech) << "\">\n";
        outFile << "  <contextInfo val=\"fr_aspiredH\"/>\n";
        outFile << "</existingMeaning>\n";
      }
    }
  }
  outFile << "</linguisticdatabase>" << std::endl;
  outFile.close();
  inFile.close();
}


} // End of namespace aspiredHExtractor
} // End of namespace onsem
