#ifndef DICTIONARYEXTRACTOR_AUXILIARIESEXTRACTOR_HPP
#define DICTIONARYEXTRACTOR_AUXILIARIESEXTRACTOR_HPP

#include "api.hpp"
#include <map>
#include <list>
#include <mutex>
#include <set>
#include <onsem/common/enum/semanticlanguageenum.hpp>
#include <onsem/texttosemantic/dbtype/semanticword.hpp>

namespace onsem {
namespace linguistics {
struct LinguisticDatabase;
}

class DICTIONARYEXTRACTOR_API AuxiliariesExtractor {
public:
    AuxiliariesExtractor(const linguistics::LinguisticDatabase& pLingDb);

    void processFile(const std::string& pFilename);

private:
    struct AuxiliariesLinksResult {
        int verbMeaningId;
        std::size_t nbFound;
        std::size_t nbFoundWithBeAux;
        std::size_t nbFoundWithHaveAux;
    };
    const linguistics::LinguisticDatabase& _lingDb;
    std::size_t _nbOfThreads;
    std::mutex _mutex;
    std::map<SemanticWord, AuxiliariesLinksResult> _vertoToAuxMeaningRes;
    std::set<std::string> _verbsWithBeAux;
    std::set<std::string> _verbsWithHaveAux;

    void _processTexts(const std::list<std::string>& pTexts, SemanticLanguageEnum pLanguage, bool pPrintLineNumber);
};

}

#endif    // DICTIONARYEXTRACTOR_AUXILIARIESEXTRACTOR_HPP
