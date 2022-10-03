#include "conceptsextractor.hpp"
#include <fstream>
#include <onsem/compilermodel/lingdbdynamictrienode.hpp>
#include "frwiki/frpatternrecognizer.hpp"
#include "frwiki/frwikikeywords.hpp"
#include "wikiutils.hpp"


namespace onsem
{

ConceptsExtractor::ConceptsExtractor
(const WikiKeyWords& pWikikeyWords,
 const PatternRecognizer& pPatternRecognizer)
  : _wikikeyWords(pWikikeyWords),
    _patternReco(pPatternRecognizer)
{
}



void ConceptsExtractor::extractDatasFromFile
(std::map<std::string, std::set<LingdbMeaning*>>& pConceptToMeanings,
 std::ifstream& pWikionaryFile,
 const LinguisticIntermediaryDatabase& pLingDatabase)
{
  std::string currWord;
  LingdbMeaning* nounMeaning = nullptr;
  std::string line;
  long numLine = 0;
  bool isInGramSection = false;
  while (getline(pWikionaryFile, line))
  {
    if (!line.empty())
    {
      if (_patternReco.isTitle(line))
      {
        currWord.clear();
        isInGramSection = false;
        wikiutil::extractWordOfTitleLine(currWord, line);
        auto* nodePtr = pLingDatabase.getPointerToEndOfWord(currWord);
        nounMeaning = nodePtr != nullptr ?
            nodePtr->getMeaning(PartOfSpeech::NOUN) : nullptr;
      }
      else if (_patternReco.isSeparator(line))
      {
        isInGramSection = _patternReco.isGramInLineType(line);
      }
      else if (isInGramSection && nounMeaning != nullptr)
      {
        static const std::string psycholPattern = "# {{psychol|";
        static const std::size_t psycholPatternSize = psycholPattern.size();
        if (line.compare(0, psycholPatternSize, psycholPattern) == 0)
        {
          pConceptToMeanings["abstract_psychology_*"].insert(nounMeaning);
        }
      }
    }

    ++numLine;
    if (numLine % 10000000 == 0)
    {
      std::cout << "numLine: " << numLine / 10000000 << "0 000 000" << std::endl;
    }
  }
  pWikionaryFile.close();
}



void ConceptsExtractor::writeConcepts
(const std::string& pOutFilename,
 const std::map<std::string, std::set<LingdbMeaning*>>& pConceptToMeanings)
{
  std::ofstream outfile(pOutFilename);
  outfile << "<linguisticdatabase>\n";
  for (const auto& currCptToMeaning : pConceptToMeanings)
  {
    outfile << " <concept name=\"" << currCptToMeaning.first << "\">\n";
    for (const auto& currMeaning : currCptToMeaning.second)
    {
      outfile << "  <linkedMeaning lemme=\"" << currMeaning->getLemma()->getWord()
              << "\" gram=\"" << partOfSpeech_toStr(currMeaning->getPartOfSpeech())
              << "\" relationToConcept=\"4\"/>\n";
    }
    outfile << " </concept>\n";
  }
  outfile << "</linguisticdatabase>";
  outfile.close();
}


} // End of namespace onsem


