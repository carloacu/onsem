#ifndef ONSEM_TEXTTOSEMANTIC_TYPES_LINGUISTICANALYSISCONFIG_HPP
#define ONSEM_TEXTTOSEMANTIC_TYPES_LINGUISTICANALYSISCONFIG_HPP

#include <onsem/texttosemantic/dbtype/misc/spellingmistaketype.hpp>
#include <onsem/texttosemantic/dbtype/resourcegroundingextractor.hpp>
#include "../api.hpp"

namespace onsem {

struct ONSEM_TEXTTOSEMANTIC_API LinguisticAnalysisConfig {
    std::shared_ptr<ResourceGroundingExtractor> cmdGrdExtractorPtr;
    std::set<SpellingMistakeType> spellingMistakeTypesPossible;
    bool tryToResolveCoreferences = true;
    bool canOnlyBeANominalGroup = false;
};

}    // End of namespace onsem

#endif    // ONSEM_TEXTTOSEMANTIC_TYPES_LINGUISTICANALYSISCONFIG_HPP
