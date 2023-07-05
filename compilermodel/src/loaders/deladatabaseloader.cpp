#include <onsem/compilermodel/loaders/deladatabaseloader.hpp>
#include <sstream>
#include <fstream>
#include <onsem/common/utility/uppercasehandler.hpp>
#include <onsem/common/utility/string.hpp>
#include <onsem/compilermodel/linguisticintermediarydatabase.hpp>
#include <onsem/compilermodel/lingdbmeaning.hpp>
#include <onsem/compilermodel/lingdbmodifier.hpp>

namespace
{
std::vector<std::string> _splitInflectionsWithoutComma
(const std::string& pInflectionalCodes)
{
  auto sizeOfCode = pInflectionalCodes.size();
  if (sizeOfCode > 0)
  {
    if (sizeOfCode > 1 &&
        pInflectionalCodes.compare(pInflectionalCodes.size() - 2, 2, "sp") == 0)
    {
      std::vector<std::string> vecOfInflectionalCodes(2);
      auto inflectionWithoutNumber = pInflectionalCodes.substr(0, pInflectionalCodes.size() - 2);
      vecOfInflectionalCodes[0] = inflectionWithoutNumber + "s";
      vecOfInflectionalCodes[1] = inflectionWithoutNumber + "p";
      return vecOfInflectionalCodes;
    }
    return std::vector<std::string>(1, pInflectionalCodes);
  }
  return {};
}
}

namespace onsem
{

void _fillWordsWithAConcept(
    LinguisticIntermediaryDatabase& pWords,
    const std::map<std::string, std::set<PartOfSpeech> >& pLemmaAndGramThatPointsToTheConcept,
    const std::string& pConcept)
{
  LingdbConcept* conceptPtr = pWords.getConcept(pConcept);
  if (conceptPtr == nullptr)
  {
    std::stringstream ss;
    ss << "\"" << pConcept << "\" concept is not found !";
    throw std::runtime_error(ss.str());
  }

  for (const auto& eltForALemma : pLemmaAndGramThatPointsToTheConcept)
  {
    for (const auto& eltForAGram : eltForALemma.second)
    {
      LingdbMeaning* meaning = pWords.getMeaning(eltForALemma.first, eltForAGram);
      if (meaning != nullptr)
        meaning->addLinkToConcept(pWords, conceptPtr, pConcept, 4, false);
    }
  }
}



DelaDatabaseLoader::DelaDatabaseLoader
(bool pSaveWords)
  : fSaveWords(pSaveWords),
    fWords()
{
}


void DelaDatabaseLoader::simplifyDelaFile
(const std::string& pInFilename,
 const std::string& pOutFilename,
 const std::set<std::string>& pLemmaToKeep,
 bool pRemoveHum,
 bool pRemoveDnum,
 bool pWordsThatBeginsWithCapitalLetter)
{
  std::ifstream infile(pInFilename, std::ifstream::in);
  if (!infile.is_open())
  {
    throw std::runtime_error("Can't open " + pInFilename + " file !");
  }
  std::ofstream outfile(pOutFilename);

  std::vector<std::string> beginsOfWordsToSkip =
  {"pour ", "cet "};

  std::string line;
  while (getline(infile, line))
  {
    if (line.empty())
      continue;

    bool removeLine = false;
    if (line[0] == '#')
    {
      NewWordInfos newWord;
      xFillNewWordInfos(newWord, line);

      if (newWord.gram == PartOfSpeech::UNKNOWN)
      {
        continue;
      }
      if (pWordsThatBeginsWithCapitalLetter && beginWithUppercase(newWord.lemma))
        continue;

      if (pLemmaToKeep.count(newWord.lemma) == 0)
      {
        for (const auto& currBegOfWord : beginsOfWordsToSkip)
        {
          if (newWord.word.compare(0, currBegOfWord.size(), currBegOfWord) == 0)
          {
            removeLine = true;
            break;
          }
        }

        if (removeLine ||
            newWord.word == "à midi" ||
            newWord.word == "la première fois" ||
            newWord.word == "première fois" ||
            newWord.word == "la dernière fois")
        {
          removeLine = true;
        }
        else
        {
          for (const auto& currOthInf : newWord.otherInfos)
          {
            if (currOthInf == "Hum")
            {
              if (pRemoveHum)
              {
                removeLine = true;
                break;
              }
              else
              {
                continue;
              }
            }
            if (currOthInf == "Dnum")
            {
              if (pRemoveDnum)
              {
                removeLine = true;
                break;
              }
              else
              {
                continue;
              }
            }
            else if (currOthInf == "z1" || currOthInf == "z2" || currOthInf == "z3" || currOthInf == "OLD" ||
                     currOthInf == "Ntime" || currOthInf == "Anl" || currOthInf == "Profession" ||
                     currOthInf == "PAC" || currOthInf == "PCDC" || currOthInf == "PC" ||
                     currOthInf == "Dind" || currOthInf == "RelQ" || currOthInf == "Pdem" || currOthInf == "Ddem" ||
                     currOthInf == "Conc" || currOthInf == "Accus" || currOthInf == "Refl" || currOthInf == "Ddéf" ||
                     currOthInf == "Dadj" || currOthInf == "aux" || currOthInf == "PCPN" ||
                     currOthInf == "Advconjs" || currOthInf == "Unit" || currOthInf == "Abst" || currOthInf == "Nomin" ||
                     currOthInf == "DA" || currOthInf == "VN" || currOthInf == "PN")
            {
              continue;
            }
            else if (currOthInf.size() > 4 && currOthInf.substr(0, 4) == "Poss")
            {
              continue;
            }
            else if (currOthInf.size() == 1 &&
                     (currOthInf == "0" || currOthInf == "1" || currOthInf == "2" || currOthInf == "3" ||
                      currOthInf == "4" || currOthInf == "5" || currOthInf == "6" || currOthInf == "7" ||
                      currOthInf == "8" || currOthInf == "9" || currOthInf == "A" || currOthInf == "t" ||
                      currOthInf == "i"))
            {
              continue;
            }
            else
            {
              removeLine = true;
              break;
            }
          }
        }
      }
    }
    if (!removeLine)
    {
      outfile << line << std::endl;
    }
  }
  outfile.close();
  infile.close();
}


void DelaDatabaseLoader::toXml
(const std::vector<std::string>& pInFilenames,
 const std::string& pOutDir)
{
  std::map<std::string, std::set<PartOfSpeech> > lemmaAndGramThatPointsToConcrete;
  std::map<std::string, std::set<PartOfSpeech> > lemmaAndGramThatPointsToACountry;
  std::map<std::string, std::set<PartOfSpeech> > lemmaAndGramThatPointsToAProfesion;
  std::map<std::string, std::set<PartOfSpeech> > lemmaAndGramThatPointsToAHuman;
  std::map<std::string, std::set<PartOfSpeech> > lemmaAndGramThatPointsToATime;
  std::map<std::string, std::set<PartOfSpeech> > lemmaAndGramThatPointsToAnimal;
  std::map<std::string, WordModif> lemmaToWordModifs;

  auto removeInfl = [&](const std::string& pWordToRemove,
                        PartOfSpeech pPosToRemove) {
    bool res = false;
    for (auto itLemmaToNewWords = lemmaToWordModifs.begin(); itLemmaToNewWords != lemmaToWordModifs.end(); )
    {
      // Remove words to remove
      for (auto itWord = itLemmaToNewWords->second.newWords.begin(); itWord != itLemmaToNewWords->second.newWords.end(); )
      {
        if (itWord->word == pWordToRemove && itWord->gram == pPosToRemove)
        {
          res = true;
          itWord = itLemmaToNewWords->second.newWords.erase(itWord);
        }
        else
        {
          ++itWord;
        }
      }

      if (itLemmaToNewWords->second.empty())
        itLemmaToNewWords = lemmaToWordModifs.erase(itLemmaToNewWords);
      else
        ++itLemmaToNewWords;
    }
    return res;
  };

  for (const auto& currInFilename : pInFilenames)
  {
    std::ifstream infile(currInFilename, std::ifstream::in);
    if (!infile.is_open())
      throw std::runtime_error("Can't open " + currInFilename + " file !");
    std::string line;
    while (getline(infile, line))
    {
      if (line.empty())
        continue;

      bool removeLine = false;
      if (line[0] == '#')
      {
        NewWordInfos newWord;
        xFillNewWordInfos(newWord, line);
        lemmaToWordModifs[newWord.lemma].newWords.push_back(newWord);
      }
      else if (line[0] == '-') // delete a wordform
      {
        std::string lemma;
        std::size_t endOfLemma = xGetStringUntilAChar(lemma, 1, '.', line);
        std::size_t beginOfGram = endOfLemma + 1;
        PartOfSpeech gram = xReadGram(line.substr(beginOfGram,  line.size() - beginOfGram));

        if (!removeInfl(lemma, gram))
          lemmaToWordModifs[lemma].partOfSpeechesToDel.insert(gram);
      }
    }
    infile.close();
  }

  std::ofstream outfile(pOutDir + "/french.omld");
  outfile << "<omld>" << std::endl;
  std::string line;
  for (auto& currLemmaToNewWords : lemmaToWordModifs)
  {
    PartOfSpeech lemmaGram = PartOfSpeech::UNKNOWN;
    if (!currLemmaToNewWords.second.newWords.empty())
      lemmaGram = currLemmaToNewWords.second.newWords.front().gram;
    else if (!currLemmaToNewWords.second.partOfSpeechesToDel.empty())
      lemmaGram = *currLemmaToNewWords.second.partOfSpeechesToDel.begin();

    outfile << " <w l=\"" << currLemmaToNewWords.first << "\" p=\"" << partOfSpeech_toStr(lemmaGram) << "\">" << std::endl;

    for (auto& posToRemove : currLemmaToNewWords.second.partOfSpeechesToDel)
    {
      outfile << "  <r";
      if (posToRemove != lemmaGram)
        outfile << " p=\"" << partOfSpeech_toStr(posToRemove) << "\"";
      outfile << " />" << std::endl;
    }
    for (auto& newWord : currLemmaToNewWords.second.newWords)
    {
      outfile << "  <i";
      if (newWord.word != currLemmaToNewWords.first)
        outfile << " i=\"" << newWord.word << "\"";
      if (newWord.gram != lemmaGram)
        outfile << " p=\"" << partOfSpeech_toStr(newWord.gram) << "\"";
      if (!newWord.flexions.empty())
        outfile << " f=\"" << mystd::join(newWord.flexions, "|") << "\"";

      char frequency = 4;
      for (const auto& currOthInf : newWord.otherInfos)
      {
        if (currOthInf == "z2")
        {
          frequency = 3;
        }
        else if (currOthInf == "z3")
        {
          frequency = 2;
        }
        else if (currOthInf == "OLD")
        {
          frequency = 1;
        }
        else if (currOthInf == "Ntime")
        {
          lemmaAndGramThatPointsToATime[newWord.lemma].insert(newWord.gram);
        }
        else if (currOthInf == "Conc")
        {
          lemmaAndGramThatPointsToConcrete[newWord.lemma].insert(newWord.gram);
        }
        else if (currOthInf == "Country")
        {
          lemmaAndGramThatPointsToACountry[newWord.lemma].insert(newWord.gram);
        }
        else if (currOthInf == "Hum")
        {
          lemmaAndGramThatPointsToAHuman[newWord.lemma].insert(newWord.gram);
        }
        else if (currOthInf == "Anl")
        {
          lemmaAndGramThatPointsToAnimal[newWord.lemma].insert(newWord.gram);
        }
        else if (currOthInf == "Profession")
        {
          lemmaAndGramThatPointsToAProfesion[newWord.lemma].insert(newWord.gram);
          break;
        }
      }
      if (frequency != 4)
        outfile << " y=\"" << static_cast<int>(frequency) << "\"";
      outfile << " />" << std::endl;
    }
    outfile << " </w>" << std::endl;
  }
  outfile << "</omld>" << std::endl;
  outfile.close();

  /*
  auto _addCptFile = [&](const std::string& pFilename,
                         const std::string& pConcept,
                         const std::map<std::string, std::set<PartOfSpeech>>& pWords) {
    std::ofstream outfile(pOutDir + "/cpts/" + pFilename);

    outfile << "<linguisticdatabase>" << std::endl;
    outfile << " <concept name=\"" << pConcept << "\">" << std::endl;
    for (const auto& currElt : pWords)
    {
      for (const auto& currPos : currElt.second)
        outfile << "  <linkedMeaning lemme=\"" << currElt.first <<
                   "\" gram=\"" << partOfSpeech_toStr(currPos) << "\" relationToConcept=\"4\" />" << std::endl;
    }
    outfile << " </concept>" << std::endl;
    outfile << "</linguisticdatabase>" << std::endl;
    outfile.close();
  };

  _addCptFile("concret_concepts.xml", "concrete_*", lemmaAndGramThatPointsToConcrete);
  _addCptFile("country_concepts.xml", "country_*", lemmaAndGramThatPointsToACountry);
  _addCptFile("agent_profession_concepts.xml", "agent_profession_*", lemmaAndGramThatPointsToAProfesion);
  _addCptFile("agent_concepts.xml", "agent_*", lemmaAndGramThatPointsToAHuman);
  _addCptFile("time_concepts.xml", "time_*", lemmaAndGramThatPointsToATime);
  _addCptFile("agent_animal_concepts.xml", "agent_animal_*", lemmaAndGramThatPointsToAnimal);
  */
}



void DelaDatabaseLoader::merge
(const std::string& pFilename,
 LinguisticIntermediaryDatabase& pWords)
{
  std::ifstream infile(pFilename, std::ifstream::in);
  if (!infile.is_open())
  {
    throw std::runtime_error("Can't open " + pFilename + " file !");
  }

  std::map<std::string, std::set<PartOfSpeech> > lemmaAndGramThatPointsToConcrete;
  std::map<std::string, std::set<PartOfSpeech> > lemmaAndGramThatPointsToACountry;
  std::map<std::string, std::set<PartOfSpeech> > lemmaAndGramThatPointsToAProfesion;
  std::map<std::string, std::set<PartOfSpeech> > lemmaAndGramThatPointsToAHuman;
  std::map<std::string, std::set<PartOfSpeech> > lemmaAndGramThatPointsToATime;
  std::map<std::string, std::set<PartOfSpeech> > lemmaAndGramThatPointsToAnimal;

  std::string line;
  while (getline(infile, line))
  {
    if (line.empty())
    {
      continue;
    }

    if (line[0] == '#')
    {
      NewWordInfos newWord;
      xFillNewWordInfos(newWord, line);

      if (newWord.gram == PartOfSpeech::UNKNOWN)
      {
        std::cerr << "In line: " << line <<"\" the word has no known grammatical type"
                  << std::endl;
        continue;
      }

      char frequency = 4;
      for (const auto& currOthInf : newWord.otherInfos)
      {
        if (currOthInf == "z2")
        {
          frequency = 3;
        }
        else if (currOthInf == "z3")
        {
          frequency = 2;
        }
        else if (currOthInf == "OLD")
        {
          frequency = 1;
        }
        else if (currOthInf == "Ntime")
        {
          lemmaAndGramThatPointsToATime[newWord.lemma].insert(newWord.gram);
        }
        else if (currOthInf == "Conc")
        {
          lemmaAndGramThatPointsToConcrete[newWord.lemma].insert(newWord.gram);
        }
        else if (currOthInf == "Country")
        {
          lemmaAndGramThatPointsToACountry[newWord.lemma].insert(newWord.gram);
        }
        else if (currOthInf == "Hum")
        {
          lemmaAndGramThatPointsToAHuman[newWord.lemma].insert(newWord.gram);
        }
        else if (currOthInf == "Anl")
        {
          lemmaAndGramThatPointsToAnimal[newWord.lemma].insert(newWord.gram);
        }
        else if (currOthInf == "Profession")
        {
          lemmaAndGramThatPointsToAProfesion[newWord.lemma].insert(newWord.gram);
          break;
        }
      }
      pWords.addWord(newWord.word, newWord.lemma, newWord.gram,
                     newWord.flexions, frequency);
      if (fSaveWords)
        fWords.push_back(newWord.word);
    }
    else if (line[0] == '-') // delete a wordform
    {
      // get lemme
      std::string lemme;
      std::size_t endOfLemme = xGetStringUntilAChar(lemme, 1, '.', line);
      std::size_t beginOfGram = endOfLemme + 1;
      PartOfSpeech gram = xReadGram(line.substr(beginOfGram,  line.size() - beginOfGram));
      pWords.removeWordForm(lemme, gram);
    }
    else if (line[0] == '%') // associate a new gram for the wordforms that have a specific meaning
    {
      // get lemme
      std::string lemme;
      std::size_t endOfLemme = xGetStringUntilAChar(lemme, 1, '.', line);
      std::size_t beginOfFirstGram = endOfLemme + 1;
      std::size_t endOfFirstGram = line.find_first_of('.', beginOfFirstGram);
      if (endOfFirstGram == std::string::npos)
      {
        throw std::runtime_error(". between 2 grammatical types, not found in: " + line);
      }
      PartOfSpeech oldGram = xReadGram(line.substr(beginOfFirstGram,  endOfFirstGram - beginOfFirstGram));
      std::size_t beginOfSecondGram = endOfFirstGram + 1;
      PartOfSpeech newGram = xReadGram(line.substr(beginOfSecondGram,  line.size() - beginOfSecondGram));

      LingdbModifier modifier;
      modifier.associateANewGramForAMeaning(pWords, lemme,
                                            oldGram, newGram);
    }
    else if (line[0] == '>')
    {
      LingdbModifier modifier;
      if (line == ">delExprs")
      {
        modifier.delExprs(pWords);
      }
      else if (line == ">delAllWords")
      {
        modifier.delAllWords(pWords);
      }
      else if (line == ">addToAtBeginOfVerbsForEnglish")
      {
        modifier.addToAtBeginOfVerbsForEnglish(pWords);
      }
      else if (line == ">findQueAddQu")
      {
        modifier.findQueAddQu(pWords);
      }
      else
      {
        std::cerr << "invalid line: " << line << std::endl;
      }
    }
    else
    {
      std::cerr << "invalid line: " << line << std::endl;
    }
  }
  infile.close();

  _fillWordsWithAConcept(pWords, lemmaAndGramThatPointsToAHuman, "agent_*");
  _fillWordsWithAConcept(pWords, lemmaAndGramThatPointsToAnimal, "agent_animal_*");
  _fillWordsWithAConcept(pWords, lemmaAndGramThatPointsToACountry, "country_*");
  _fillWordsWithAConcept(pWords, lemmaAndGramThatPointsToAProfesion, "agent_profession_*");
  _fillWordsWithAConcept(pWords, lemmaAndGramThatPointsToConcrete, "concrete_*");
  _fillWordsWithAConcept(pWords, lemmaAndGramThatPointsToATime, "time_*");
}



void DelaDatabaseLoader::xFillNewWordInfos
(NewWordInfos& pNewWord,
 const std::string& pLine) const
{
  // get word
  std::size_t endOfWord = xGetStringUntilAChar(pNewWord.word, 1, ',', pLine);

  // get lemme
  std::size_t endOfLemme = xGetStringUntilAChar(pNewWord.lemma,
                                                endOfWord + 1, '.', pLine);
  if (pNewWord.lemma.empty())
    pNewWord.lemma = pNewWord.word;

  // get complementary infos
  std::size_t beforeInfo = endOfLemme;
  std::size_t beginOfInfo = beforeInfo + 1;
  std::size_t endOfInfo = pLine.find_first_of("+:\n\r", beginOfInfo);
  bool flexionsAreSet = false;
  while (endOfInfo != std::string::npos)
  {
    xReadWordInfo(pNewWord, beforeInfo, beginOfInfo, endOfInfo, flexionsAreSet, pLine);
  }
  if (beginOfInfo < pLine.size())
  {
    xReadWordInfo(pNewWord, beforeInfo, beginOfInfo, endOfInfo, flexionsAreSet, pLine);
  }
}


std::size_t DelaDatabaseLoader::xGetStringUntilAChar
(std::string& pStrResult,
 std::size_t pBeginOfSearch,
 char pEndingChar,
 const std::string& pLine) const
{
  std::size_t endOfStr = pLine.find_first_of(pEndingChar, pBeginOfSearch);
  if (endOfStr == std::string::npos)
  {
    std::stringstream ss;
    ss << pEndingChar << " not found in: " << pLine;
    throw std::runtime_error(ss.str());
  }
  pStrResult = pLine.substr(pBeginOfSearch,  endOfStr - pBeginOfSearch);

  std::size_t escapeCharPos = pStrResult.find_first_of('\\');
  if (escapeCharPos != std::string::npos)
  {
    std::size_t beforeEscape = 0;
    std::stringstream ss;
    while (escapeCharPos != std::string::npos)
    {
      ss << pStrResult.substr(beforeEscape, escapeCharPos - beforeEscape);
      beforeEscape = escapeCharPos + 1;
      escapeCharPos = pStrResult.find_first_of('\\', beforeEscape + 1);
    }
    if (beforeEscape < pStrResult.size())
    {
      ss << pStrResult.substr(beforeEscape, pStrResult.size());
    }
    pStrResult = ss.str();
  }
  return endOfStr;
}



void DelaDatabaseLoader::xReadWordInfo
(NewWordInfos& pNewWord,
 std::size_t& pBeforeInfo,
 std::size_t& pBeginOfInfo,
 std::size_t& pEndOfInfo,
 bool& pFlexionsAreSet,
 const std::string& pLine) const
{
  if (pLine[pBeforeInfo] == '.')
  {
    pNewWord.gram = xReadGram(pLine.substr(pBeginOfInfo,  pEndOfInfo - pBeginOfInfo));
  }
  else if (pLine[pBeforeInfo] == ':')
  {
    if (!pFlexionsAreSet)
      pNewWord.flexions.emplace_back(pLine.substr(pBeginOfInfo,  pEndOfInfo - pBeginOfInfo));
  }
  else
  {
    auto info = pLine.substr(pBeginOfInfo,  pEndOfInfo - pBeginOfInfo);
    if (pNewWord.gram != PartOfSpeech::DETERMINER &&
        info.size() > 4 &&
        info.compare(0, 4, "Poss") == 0)
    {
      auto inflections = info.substr(4, info.size() - 4);
      pNewWord.flexions = _splitInflectionsWithoutComma(inflections);
      pFlexionsAreSet = true;
    }
    pNewWord.otherInfos.emplace_back(info);
  }

  pBeforeInfo = pEndOfInfo;
  pBeginOfInfo = pBeforeInfo + 1;
  pEndOfInfo = pLine.find_first_of("+:\n\r", pBeginOfInfo);
}



PartOfSpeech DelaDatabaseLoader::xReadGram
(const std::string& pStr) const
{
  if (pStr == "SPACE")
  {
    return PartOfSpeech::INTERSPACE;
  }
  if (pStr == "LINK")
  {
    return PartOfSpeech::LINKBETWEENWORDS;
  }
  if (pStr == "PUNCT")
  {
    return PartOfSpeech::PUNCTUATION;
  }
  if (pStr == "A" || pStr == "PREPADJ")
  {
    return PartOfSpeech::ADJECTIVE;
  }
  if (pStr == "ADV" || pStr == "PRED" ||
      // only english
      pStr == "ADVA")
  {
    return PartOfSpeech::ADVERB;
  }
  if (pStr == "CONJC" || pStr == "CONJ")
  {
    return PartOfSpeech::CONJUNCTIVE;
  }
  if (pStr == "CONJS")
  {
    return PartOfSpeech::SUBORDINATING_CONJONCTION;
  }
  if (pStr == "DET")
  {
    return PartOfSpeech::DETERMINER;
  }
  if (pStr == "PARTITIVE")
  {
    return PartOfSpeech::PARTITIVE;
  }
  if (pStr == "INTJ")
  {
    return PartOfSpeech::INTERJECTION;
  }
  if (pStr == "N" || pStr == "PRED" || pStr == "PFX" ||
      // only english
      pStr == "NES")
  {
    return PartOfSpeech::NOUN;
  }
  if (pStr == "PREP" || pStr == "PREPDET")
  {
    return PartOfSpeech::PREPOSITION;
  }
  if (pStr == "PRO" || pStr == "PRON"  || pStr == "PREPPRO")
  {
    return PartOfSpeech::PRONOUN;
  }
  if (pStr == "V")
  {
    return PartOfSpeech::VERB;
  }
  if (pStr == "AUX")
  {
    return PartOfSpeech::AUX;
  }
  if (pStr == "PN")
  {
    return PartOfSpeech::PROPER_NOUN;
  }
  if (pStr == "PROC")
  {
    return PartOfSpeech::PRONOUN_COMPLEMENT;
  }
  if (pStr == "PROS")
  {
    return PartOfSpeech::PRONOUN_SUBJECT;
  }
  if (pStr == "NDET" || // noun + determiner
      pStr == "PCDN3" || pStr == "X" || pStr == "XI" ||
      pStr == "GN" || pStr == "GNP" || pStr == "GNPX" ||
      // only english
      pStr == "PART" || pStr == "VA" || pStr == "NA")

  {
    return PartOfSpeech::UNKNOWN;
  }
  throw std::runtime_error("Unkown tag grammatical: " + pStr);
}

} // End of namespace onsem
