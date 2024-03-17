#include <iostream>
#include <fstream>
#include <onsem/compilermodel/lingdbtree.hpp>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage databasegenerator <output_dir> <share_folder>" << std::endl;
        return 2;
    }

    std::string outDatabaseDir(argv[1]);
    std::string shareDir(argv[2]);

    const auto inputResourcesDir(shareDir + "/linguistic/inputresources");
    const auto loadDatabasesDir(shareDir + "/linguistic/loaddatabases");
    const auto relationsPath(shareDir + "/semantic/relations");

    onsem::LingdbTree lingdbTree(inputResourcesDir);
    lingdbTree.update(outDatabaseDir, loadDatabasesDir, relationsPath, true);
    return 0;
}
