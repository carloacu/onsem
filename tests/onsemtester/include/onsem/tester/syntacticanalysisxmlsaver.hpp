#ifndef ONSEM_TESTER_SYNTACTICANALYSISXMLSAVER_HPP
#define ONSEM_TESTER_SYNTACTICANALYSISXMLSAVER_HPP

#include <map>
#include <vector>
#include <memory>
#include <onsem/tester/syntacticanalysisxmlloader.hpp>
#include "api.hpp"


namespace onsem
{
namespace syntacticAnalysisXmlSaver
{

ONSEMTESTER_API
void compareResults(std::shared_ptr<syntacticAnalysisXmlLoader::DeserializedTextResults>& pDiffResults,
                    const std::string& pLanguageStr,
                    const linguistics::LinguisticDatabase& pLingDb,
                    const std::string& pCorpusResultsFolder,
                    const std::map<std::string, std::string>* pEquivalencesPtr = nullptr,
                    std::string* pPerformancesPtr = nullptr);


ONSEMTESTER_API
void save(const std::string& pFilename,
          SemanticLanguageEnum pLanguageType,
          const std::vector<std::string>& pInputSentences,
          const linguistics::LinguisticDatabase& pLingDb);


} // End of namespace syntacticAnalysisXmlSaver
} // End of namespace onsem

#endif // ONSEM_TESTER_SYNTACTICANALYSISXMLSAVER_HPP
