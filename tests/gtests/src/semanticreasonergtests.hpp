#ifndef ONSEM_GTESTS_SEMANTICREASONNERGTESTS_HPP
#define ONSEM_GTESTS_SEMANTICREASONNERGTESTS_HPP

#include <memory>
#include <gtest/gtest.h>
#include <onsem/streamdatabaseaccessor/streamdatabaseaccessor.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semantictimegrounding.hpp>
#include <onsem/texttosemantic/dbtype/textprocessingcontext.hpp>
#include <onsem/common/utility/make_unique.hpp>
#include <onsem/tester/resourcelabelfortests.hpp>
#include "util/util.hpp"


class SemanticReasonerGTests : public testing::Test
{
public:

  // If the constructor and destructor are not enough for setting up
  // and cleaning up each test, you can define the following methods:

  /**
   * @brief This function will be called before all tests.
   */
  static void SetUpTestCase()
  {
    auto iStreams = onsem::linguistics::generateIStreams(lingDbPath, dynamicdictionaryPath);
    lingDbPtr = onsem::mystd::make_unique<onsem::linguistics::LinguisticDatabase>(iStreams.linguisticDatabaseStreams);
    iStreams.close();

    _corpusInputFolder = corpusPath + "/input";
    _corpusResultsFolder = corpusPath + "/results";
  }

  /**
   * @brief This function will be called immediately after the constructor
   * (right before each test).
   */
  void SetUp()
  {
    onsem::SemanticTimeGrounding::setAnHardCodedTimeElts(true, false);
  }

  /**
   * @brief This function will be called immediately after each test
   * (right before the destructor).
   */
  void TearDown()
  {
  }

  /**
   * @brief This function will be called after all tests.
   */
  static void TearDownTestCase()
  {
    lingDbPtr.reset();
  }

public:
  static std::string lingDbPath;
  static std::string dynamicdictionaryPath;
  static std::string corpusPath;
  static std::string scenariosPath;
  static std::unique_ptr<onsem::linguistics::LinguisticDatabase> lingDbPtr;
  static const onsem::TextProcessingContext fromRobot;
  static const onsem::TextProcessingContext fromUser;
protected:
  static std::string _corpusInputFolder;
  static std::string _corpusResultsFolder;
};


#endif // ONSEM_GTESTS_SEMANTICREASONNERGTESTS_HPP
