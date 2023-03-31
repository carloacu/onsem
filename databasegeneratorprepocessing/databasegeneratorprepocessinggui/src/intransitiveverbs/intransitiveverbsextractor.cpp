#include "intransitiveverbsextractor.hpp"
#include <fstream>
#include <sstream>
#include <onsem/texttosemantic/dbtype/linguisticdatabase.hpp>

namespace onsem
{
namespace intransitiveVerbsExtractor
{
namespace
{

void _run(std::map<std::string, std::set<PartOfSpeech>>& pLemmasToPos,
          const std::string& pInFilename,
          const linguistics::LinguisticDatabase& pLingDb)
{
  std::ifstream inFile(pInFilename, std::ifstream::in);
  if (!inFile.is_open())
    throw std::runtime_error("Error: Can't open " + pInFilename + " file !");

  const linguistics::StaticLinguisticDictionary& staticBinDico =
      pLingDb.langToSpec[SemanticLanguageEnum::FRENCH].lingDico.statDb;

  std::string line;
  while (getline(inFile, line))
  {
    std::list<linguistics::InflectedWord> inflectedWords;
    staticBinDico.getGramPossibilities(inflectedWords, line, 0, line.size());
    for (const auto& currInflectedWord : inflectedWords)
      pLemmasToPos[currInflectedWord.word.lemma].insert(currInflectedWord.word.partOfSpeech);
  }
  inFile.close();
}

}

void run(const std::string& pMyDataMiningPath,
         const std::string& pInputResourcesFolder,
         const linguistics::LinguisticDatabase& pLingDb)
{
  std::map<std::string, std::set<PartOfSpeech>> lemmasToPos;
  _run(lemmasToPos,
       pMyDataMiningPath + "/resources/french/verb_intransitifs/verb_intransitifs.txt",
       pLingDb);
  _run(lemmasToPos,
       pMyDataMiningPath + "/resources/french/verb_intransitifs/verb_transitifs_dont_ceux_qui_peuvent_etre_transitifs_aussi.txt",
       pLingDb);

  std::ofstream outFile(pInputResourcesFolder + "/french/readonly/french_contextinfos_intransitiveverb.xml");
  outFile << "<linguisticdatabase>\n";
  for (const auto& currLemmasToPos : lemmasToPos)
  {
    for (auto currPos : currLemmasToPos.second)
    {
      outFile << "<existingMeaning lemme=\"" << currLemmasToPos.first << "\" gram=\""
              << partOfSpeech_toStr(currPos) << "\">\n";
      outFile << "  <contextInfo val=\"intransitiveVerb\"/>\n";
      outFile << "</existingMeaning>\n";
    }
  }
  outFile << "</linguisticdatabase>" << std::endl;
  outFile.close();
}


} // End of intransitiveVerbsExtractor intransitiveVerbsExtractor
} // End of namespace onsem
