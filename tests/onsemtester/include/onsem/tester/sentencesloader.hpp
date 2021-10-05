#ifndef ONSEM_TESTER_SENTENCESLOADER_HPP
#define ONSEM_TESTER_SENTENCESLOADER_HPP

#include <vector>
#include <string>
#include <onsem/tester/syntacticanalysisxmlloader.hpp>
#include "api.hpp"


class QWidget;

namespace onsem
{

class ONSEMTESTER_API SentencesLoader
{
public:
  SentencesLoader();

  void loadFolder
  (const std::string& pFoldername);

  void loadFile
  (const std::string& pFilename);

  void loadSentencesWithOldResults
  (const std::shared_ptr<syntacticAnalysisXmlLoader::DeserializedTextResults>& pOldResults);


  void getPrevSentence(std::string& pPrevString);

  void getCurrSentence(std::string& pNextString) const;

  void getNextSentence(std::string& pNextString);

  const std::vector<std::string>& getSentences() const
  { return fResults->diffsInputSentences; }

  const syntacticAnalysisXmlLoader::DeserializedTextResults& getOldResults()
  { return *fResults; }

  std::size_t getCurrIndex()
  { return fCurrIndex; }

private:
  std::shared_ptr<syntacticAnalysisXmlLoader::DeserializedTextResults> fResults;
  std::size_t fCurrIndex;

  void xLoadFile
  (const std::string& pFilename);

  void xLoadXml
  (const std::string& pFilename);

  void xLoadTxt
  (const std::string& pFilename);
};


} // End of namespace onsem


#endif // ONSEM_TESTER_SENTENCESLOADER_HPP
