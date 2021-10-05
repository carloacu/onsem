#include "semanticreasonergtests.hpp"


int main(int argc, char **argv)
{
  std::string shareSemantic;
  for (int i = 0; i < argc; ++i)
  {
    if ((i + 1) < argc)
    {
      const std::string currAgrv = argv[i];
      if (currAgrv == "--databases")
        SemanticReasonerGTests::lingDbPath = argv[i + 1];
      else if (currAgrv == "--share_semantic")
        shareSemantic = argv[i + 1];
    }
  }
  static const std::string exOfCommandLine = "--databases ../../linguistic/databases --share_semantic ../../../share/semantic";
  if (SemanticReasonerGTests::lingDbPath.empty())
    throw std::runtime_error("--databases option is missing (ex: " + exOfCommandLine + ")");
  if (shareSemantic.empty())
    throw std::runtime_error("--share_semantic option is missing (ex: " + exOfCommandLine + ")");

  SemanticReasonerGTests::corpusPath = shareSemantic + "/corpus";
  SemanticReasonerGTests::scenariosPath = shareSemantic + "/scenarios";
  SemanticReasonerGTests::dynamicdictionaryPath = shareSemantic + "/dynamicdictionary";

  ::testing::InitGoogleTest(&argc, argv);
  int res = RUN_ALL_TESTS();
  return res;
}


