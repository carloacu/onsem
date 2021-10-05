#include <onsem/tester/sentencesloader.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/property_tree/xml_parser.hpp>


namespace onsem
{
// this "if" is needed otherwise we have a crash on mac if we try to iterate on an empty tree
#define childLoop(TREE, ELT, LABEL)                   \
  auto optChildren = TREE.get_child_optional(LABEL);  \
  if (optChildren)                                    \
    for (const auto& ELT : *optChildren)


SentencesLoader::SentencesLoader()
  : fResults(),
    fCurrIndex(0)
{
}


void SentencesLoader::loadFolder
(const std::string& pFoldername)
{
  loadFile(pFoldername + "/registration_corpus.xml");
}


void SentencesLoader::loadFile
(const std::string& pFilename)
{
  fResults = std::make_shared<syntacticAnalysisXmlLoader::DeserializedTextResults>();
  xLoadFile(pFilename);
  fCurrIndex = fResults->diffsInputSentences.size() - 1;
}


void SentencesLoader::loadSentencesWithOldResults
(const std::shared_ptr<syntacticAnalysisXmlLoader::DeserializedTextResults>& pOldResults)
{
  fResults = pOldResults;
  fCurrIndex = fResults->diffsInputSentences.size() - 1;
}


void SentencesLoader::xLoadFile
(const std::string& pFilename)
{
  const std::string filenameStr = pFilename;
  std::string suffixStr;
  if (filenameStr.size() > 3)
    suffixStr = filenameStr.substr(filenameStr.size() - 3, 3);

  if (suffixStr == "txt")
    xLoadTxt(pFilename);
  else if (suffixStr == "xml")
    xLoadXml(pFilename);
  else
    throw std::runtime_error("Cannot find " + pFilename + " file !");
}


void SentencesLoader::xLoadTxt
(const std::string& pFilename)
{
  std::ifstream sentencesFile(pFilename, std::ifstream::in);
  if (!sentencesFile.is_open())
    throw std::runtime_error("Can't open " + pFilename + " file !");

  std::string line;
  while (getline(sentencesFile, line))
    if (!line.empty())
      fResults->diffsInputSentences.push_back(line);
  sentencesFile.close();
}


void SentencesLoader::xLoadXml
(const std::string& pFilename)
{
  boost::filesystem::path folder = boost::filesystem::path(pFilename).parent_path();
  boost::property_tree::ptree tree;
  boost::property_tree::read_xml(pFilename, tree);
  childLoop(tree, currSubTree, "sentencesTester")
  {
    if (currSubTree.first == "include")
      xLoadFile((folder / currSubTree.second.get<std::string>("<xmlattr>.file")).string());
    else
      throw std::runtime_error("Unknown beacon : '" + currSubTree.first + "'.");
  }
}




void SentencesLoader::getPrevSentence
(std::string& pPrevString)
{
  if (fResults->diffsInputSentences.empty())
  {
    return;
  }
  if (fCurrIndex > 0)
  {
    --fCurrIndex;
  }
  else
  {
    fCurrIndex = fResults->diffsInputSentences.size() - 1;
  }
  pPrevString = fResults->diffsInputSentences[fCurrIndex];
}


void SentencesLoader::getCurrSentence
(std::string& pNextString) const
{
  pNextString = fResults->diffsInputSentences[fCurrIndex];
}


void SentencesLoader::getNextSentence
(std::string& pNextString)
{
  if (fResults->diffsInputSentences.empty())
  {
    return;
  }
  if (fCurrIndex >= fResults->diffsInputSentences.size() - 1)
  {
    fCurrIndex = 0;
  }
  else
  {
    ++fCurrIndex;
  }
  pNextString = fResults->diffsInputSentences[fCurrIndex];
}

} // End of namespace onsem
