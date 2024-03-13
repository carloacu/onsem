#include "../webtranslator.hpp"
#include "webrequest.hpp"
#include "webresults.hpp"

#include <string>
#include <iostream>
#include <sstream>
#include <fstream>

namespace onsem {
namespace webTranslator {

void run(const LingdbTree& pLingDbTree,
         const std::string& pTmpFolder,
         const std::string& pShareDbFolder,
         const std::string& pInputResourcesDir) {
    // generate php to trig google traduction
    if (false) {
        onsem::process_webrequest(pInputResourcesDir);
    }

    // read google traduction and generate correspondings *.wlks
    onsem::process_webresults(pLingDbTree, pTmpFolder, pShareDbFolder);
}

}    // End of namespace webTranslator
}    // End of namespace onsem
