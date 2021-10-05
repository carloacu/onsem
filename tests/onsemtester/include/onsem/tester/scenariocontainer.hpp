#ifndef ONSEM_TESTER_SCENARIOCONTAINER_HPP
#define ONSEM_TESTER_SCENARIOCONTAINER_HPP

#include <set>
#include <list>
#include <string>
#include <functional>
#include "api.hpp"


namespace onsem
{
struct SemanticMemory;
namespace linguistics
{
struct LinguisticDatabase;
}


class ONSEMTESTER_API ScenarioContainer
{
public:
  ScenarioContainer();

  void clear();

  bool isEmpty() const;

  static void getAllScenarios(std::set<std::string>& pScenarios,
                              const std::string& pScenariosSpecFolder);
  static void writeScenarioToFile(const std::string& pFilename,
                                  const std::string& pText,
                                  const std::list<std::string>& pInputLabels);
  static bool doesFileExist(const std::string& pFilename);
  static void getFileContent(std::list<std::string>& pFileContent,
                             const std::string& pFilename);

  void load(const std::string& pScenariosSpecFolder);

  void getCurrScenarioFilename(std::string& pScenario) const;

  void getOldContent(std::list<std::string>& pOldContent) const;

  void getMoveToPrevScenario();
  void getMoveToNextScenario();

  bool compareScenariosToReferenceResults(
      std::string& pBilan,
      const std::string& pInputScenariosFolder,
      const std::string& pOutputScenariosFolder,
      std::function<void(
        std::list<std::string>&,
        const std::string&,
        SemanticMemory&,
        const linguistics::LinguisticDatabase&)> pGetResultOfAScenario,
      const linguistics::LinguisticDatabase& pLingDb);

  void updateScenariosResults(
      const std::string& pInputScenariosFolder,
      const std::string& pOutputScenariosFolder,
      std::function<void(
        std::list<std::string>&,
        const std::string&,
        SemanticMemory&,
        const linguistics::LinguisticDatabase&)> pGetResultOfAScenario,
      const linguistics::LinguisticDatabase& pLingDb);


private:
  struct ScenarioSpecification
  {
    ScenarioSpecification
    (const std::string& pFilename,
     const std::list<std::string>& pOldContent)
      : filename(pFilename),
        oldContent(pOldContent)
    {
    }

    std::string filename;
    const std::list<std::string> oldContent;
  };

  std::list<ScenarioSpecification> _scenarios;
  std::list<ScenarioSpecification>::iterator _itCurrScenario;

  void _addNewChangedScenario(const std::string& pFilename,
                              const std::list<std::string>& pOldCntent);
};


} // End of namespace onsem


#endif // ONSEM_TESTER_SCENARIOCONTAINER_HPP
