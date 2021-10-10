#include <iostream>
#include <fstream>
#include <onsem/compilermodel/lingdbtree.hpp>



int main(int argc, char* argv[])
{
  if (argc < 2)
  {
    std::cerr << "Usage semanticdbgenerator <output_dir> <share_folder>" << std::endl;
    return 2;
  }

  std::string ontDatabaseDir(argv[1]);
  std::string shareDir(argv[2]);

  const auto inputResourcesDir(shareDir + "/linguistic/inputresources");
  const auto loadDatabasesDir(shareDir + "/linguistic/loaddatabases");
  const auto dynamicdictionaryPath(shareDir + "/semantic/dynamicdictionary");

  onsem::LingdbTree lingdbTree(inputResourcesDir);
  lingdbTree.update(ontDatabaseDir, loadDatabasesDir, dynamicdictionaryPath, true);
  return 0;
}
