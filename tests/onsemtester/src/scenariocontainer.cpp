#include <onsem/tester/scenariocontainer.hpp>
#include <sstream>
#include <boost/filesystem.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemory.hpp>

namespace onsem
{


ScenarioContainer::ScenarioContainer()
  : _scenarios(),
    _itCurrScenario(_scenarios.begin())
{
}


void ScenarioContainer::clear()
{
  _scenarios.clear();
  _itCurrScenario = _scenarios.begin();
}


bool ScenarioContainer::isEmpty() const
{
  return _scenarios.empty();
}


void _getLocalPath(boost::filesystem::path& pLocalpath,
                   const boost::filesystem::path& pRoot,
                   const boost::filesystem::path& pFullPath)
{
  boost::filesystem::path tmppath = pFullPath;
  while (tmppath != pRoot)
  {
    pLocalpath = tmppath.filename() / pLocalpath;
    tmppath = tmppath.parent_path();
  }
}


void ScenarioContainer::getAllScenarios
(std::set<std::string>& pScenarios,
 const std::string& pScenariosSpecFolder)
{
  boost::filesystem::path root(pScenariosSpecFolder);
  if (boost::filesystem::exists(root) &&
      boost::filesystem::is_directory(root))
  {
    boost::filesystem::recursive_directory_iterator it(root);
    boost::filesystem::recursive_directory_iterator endit;
    while (it != endit)
    {
      if (boost::filesystem::is_regular_file(*it))
      {
        boost::filesystem::path localpath;
        _getLocalPath(localpath, root, it->path());
        pScenarios.insert(localpath.string());
      }
      ++it;
    }
  }
}


bool ScenarioContainer::compareScenariosToReferenceResults(
    std::string& pBilan,
    const std::string& pInputScenariosFolder,
    const std::string& pOutputScenariosFolder,
    std::function<void(
      std::list<std::string>&,
      const std::string&,
      SemanticMemory&,
      const linguistics::LinguisticDatabase&)> pGetResultOfAScenario,
    const linguistics::LinguisticDatabase& pLingDb)
{
  std::stringstream ssRefResultNotFound;
  std::stringstream ssCompariasonResult;

  bool someScenariosHaveChanged = false;
  std::set<std::string> scenarios;
  ScenarioContainer::getAllScenarios(scenarios, pInputScenariosFolder);
  std::size_t nbBadScenarios = 0;

  for (const auto& currScenarioName : scenarios)
  {
    SemanticMemory semanticMemory;
    std::list<std::string> newResult;
    pGetResultOfAScenario(newResult,
                          pInputScenariosFolder + "/" + currScenarioName,
                          semanticMemory, pLingDb);

    const auto scenarioResultFilename = pOutputScenariosFolder + "/" + currScenarioName;
    std::ifstream refResultFile(scenarioResultFilename);
    if (!refResultFile.is_open())
    {
      ssRefResultNotFound << currScenarioName << "\n";
      refResultFile.close();
    }
    else
    {
      std::string line;
      std::string beforeLine;
      std::string errorStr;
      std::list<std::string>::const_iterator itScenarioNewResult = newResult.begin();
      while (getline(refResultFile, line))
      {
        if (line.empty())
          continue;

        static const std::string autoStr = "auto_";
        static const std::size_t autoStrSize = autoStr.size();
        char lastCharOfLine = line[line.size() - 1];
        if (lastCharOfLine != '"' && lastCharOfLine != '>' &&
            lastCharOfLine != ')' && lastCharOfLine != ']' &&
            line.compare(0, autoStrSize, autoStr) != 0)
        {
          beforeLine += line + "\n";
          continue;
        }
        if (!beforeLine.empty())
        {
          line = beforeLine + line;
          beforeLine = "";
        }

        if (itScenarioNewResult == newResult.end())
        {
          errorStr = "ref is shorter\n";
          break;
        }
        if (line != *itScenarioNewResult)
        {
          errorStr = "\tcurr: " + *itScenarioNewResult + "\n";
          errorStr += "\tref:  " + line + "\n";
          break;
        }
        ++itScenarioNewResult;
      }
      refResultFile.close();


      if (errorStr.empty() &&
          itScenarioNewResult != newResult.end())
      {
        errorStr = "ref is longer\n";
      }

      if (!errorStr.empty())
      {
        ++nbBadScenarios;
        ssCompariasonResult << currScenarioName << " NOK\n";
        ssCompariasonResult << errorStr;
        if (!someScenariosHaveChanged)
        {
          clear();
          someScenariosHaveChanged = true;
        }
        std::list<std::string> referenceResult;
        ScenarioContainer::getFileContent(referenceResult, scenarioResultFilename);
        _addNewChangedScenario(currScenarioName, referenceResult);
      }
    }
  }

  std::string strRefResultNotFound = ssRefResultNotFound.str();
  pBilan.clear();
  if (!strRefResultNotFound.empty())
  {
    pBilan = "Not found results:\n";
    pBilan += "------------------\n";
    pBilan += strRefResultNotFound + "\n";
    someScenariosHaveChanged = true;
  }
  std::string strCompariasonResult = ssCompariasonResult.str();
  if (!strCompariasonResult.empty())
  {
    pBilan += "Results:\n";
    pBilan += "--------\n";
    pBilan += strCompariasonResult + "\n";
  }
  pBilan += "Bilan:\n";
  pBilan += "---------\n";
  std::stringstream ssBilan;
  ssBilan << nbBadScenarios << " / " << scenarios.size();
  pBilan += ssBilan.str();

  return someScenariosHaveChanged;
}


void ScenarioContainer::updateScenariosResults(
    const std::string& pInputScenariosFolder,
    const std::string& pOutputScenariosFolder,
    std::function<void(
      std::list<std::string>&,
      const std::string&,
      SemanticMemory&,
      const linguistics::LinguisticDatabase&)> pGetResultOfAScenario,
    const linguistics::LinguisticDatabase& pLingDb)
{
  std::set<std::string> scenarios;
  getAllScenarios(scenarios, pInputScenariosFolder);
  for (const auto& currScenario : scenarios)
  {
    SemanticMemory semMemory;
    std::list<std::string> scenarioResult;
    pGetResultOfAScenario(scenarioResult, pInputScenariosFolder +  "/" + currScenario,
                          semMemory, pLingDb);

    std::ofstream resultFile(pOutputScenariosFolder + "/" + currScenario);
    for (const auto& currLine : scenarioResult)
      resultFile << currLine << "\n";
    resultFile.close();
  }
}


bool ScenarioContainer::doesFileExist(const std::string& pFilename)
{
  std::ifstream readFile(pFilename);
  if (readFile.good())
  {
    readFile.close();
    return true;
  }
  else
  {
    readFile.close();
    return false;
  }
}


void ScenarioContainer::getFileContent(std::list<std::string>& pFileContent,
                                       const std::string& pFilename)
{
  std::ifstream fileStream(pFilename, std::ifstream::in);
  if (!fileStream.is_open())
    throw std::runtime_error("Can't open " + pFilename + " file !");
  std::string line;
  while (getline(fileStream, line))
    if (!line.empty())
      pFileContent.push_back(line);
  fileStream.close();
}


void ScenarioContainer::load
(const std::string& pScenariosSpecFolder)
{
  clear();
  std::set<std::string> scenarioFilenames;
  getAllScenarios(scenarioFilenames, pScenariosSpecFolder);
  for (const auto& currElt : scenarioFilenames)
    _scenarios.emplace_back(currElt, std::list<std::string>());
  _itCurrScenario = _scenarios.begin();
}


void ScenarioContainer::_addNewChangedScenario
(const std::string& pFilename,
 const std::list<std::string>& pOldCntent)
{
  _scenarios.emplace_back(pFilename, pOldCntent);
  _itCurrScenario = --_scenarios.end();
}


void ScenarioContainer::getCurrScenarioFilename
(std::string& pScenarioFilename) const
{
  if (!_scenarios.empty())
    pScenarioFilename = _itCurrScenario->filename;
}


void ScenarioContainer::getOldContent
(std::list<std::string>& pOldContent) const
{
  if (!_scenarios.empty())
    pOldContent = _itCurrScenario->oldContent;
}


void ScenarioContainer::getMoveToPrevScenario()
{
  if (!_scenarios.empty())
  {
    if (_itCurrScenario == _scenarios.begin())
      _itCurrScenario = --_scenarios.end();
    else
      --_itCurrScenario;
  }
}


void ScenarioContainer::getMoveToNextScenario()
{
  if (!_scenarios.empty())
  {
    if (_itCurrScenario == --_scenarios.end())
      _itCurrScenario = _scenarios.begin();
    else
      ++_itCurrScenario;
  }
}


void ScenarioContainer::writeScenarioToFile(
    const std::string& pFilename,
    const std::string& pText,
    const std::list<std::string>& pInputLabels)
{
  auto _doesLineAsToBeSaveInAScenario = [&pText, &pInputLabels]
  (std::size_t pBeginLine)
  {
    for (const auto& currInputLabel : pInputLabels)
      if (pText.compare(pBeginLine, currInputLabel.size(), currInputLabel) == 0)
        return true;
    return false;
  };

  std::ofstream outfile(pFilename);
  std::size_t beginPos = 0;
  std::size_t currPos = pText.find_first_of('\n');
  while (currPos != std::string::npos)
  {
    if (_doesLineAsToBeSaveInAScenario(beginPos) &&
        currPos > beginPos)
    {
      outfile << pText.substr(beginPos, currPos - beginPos) << "\n";
    }
    beginPos = currPos + 1;
    currPos = pText.find_first_of('\n', beginPos);
  }

  if (_doesLineAsToBeSaveInAScenario(beginPos) &&
      pText.size() > beginPos)
  {
    outfile << pText.substr(beginPos, pText.size() - beginPos);
  }
  outfile.close();
}


} // End of namespace onsem
