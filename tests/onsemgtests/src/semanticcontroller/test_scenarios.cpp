#include <gtest/gtest.h>
#include <onsem/tester/scenariocontainer.hpp>
#include <onsem/tester/reactOnTexts.hpp>
#include "../semanticreasonergtests.hpp"


using namespace onsem;


TEST_F(SemanticReasonerGTests, test_scenarios)
{
  ScenarioContainer scenarioContainer;
  const auto inputScenariosFolder(scenariosPath + "/input");
  const auto outputScenariosFolder(scenariosPath + "/output");

  std::string bilan;
  bool someScenariosHaveChanged =
      scenarioContainer.compareScenariosToReferenceResults
      (bilan, inputScenariosFolder, outputScenariosFolder,
       &getResultOfAScenario, *lingDbPtr);
  std::cout << "\n" << bilan << std::endl;
  ASSERT_FALSE(someScenariosHaveChanged);
}

