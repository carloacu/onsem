#ifndef ONSEM_COMPILERMODEL_LOADERS_WLKSDATABASELOADER_HPP
#define ONSEM_COMPILERMODEL_LOADERS_WLKSDATABASELOADER_HPP

#include <string>
#include <map>
#include <list>
#include <set>
#include <memory>
#include <filesystem>

namespace onsem {
class LinguisticIntermediaryDatabase;
class LingdbTree;
class LingdbMeaning;
class LingdbConcept;
class LingdbLinkToAConcept;
struct MeaningAndConfidence;

class WlksDatabaseLoader {
public:
    enum class NextLineSpec { TRADUCTION, NOTDEFINED };
    struct WlksDatabaseLoader_LangSpec {
        WlksDatabaseLoader_LangSpec()
            : lingDatabase()
            , conceptToMeanings() {}

        std::shared_ptr<LinguisticIntermediaryDatabase> lingDatabase;
        std::map<const LingdbConcept*, std::set<MeaningAndConfidence> > conceptToMeanings;
    };
    struct WlksDatabaseLoader_TradSpec {
        WlksDatabaseLoader_TradSpec(WlksDatabaseLoader_LangSpec& pInLingDb, WlksDatabaseLoader_LangSpec& pOutLingDb)
            : inLingDb(pInLingDb)
            , outLingDb(pOutLingDb)
            , traductions() {}

        WlksDatabaseLoader_LangSpec& inLingDb;
        WlksDatabaseLoader_LangSpec& outLingDb;
        std::map<LingdbMeaning*, std::set<MeaningAndConfidence> > traductions;
    };
    struct WlksDatabaseLoader_WorkState {
        WlksDatabaseLoader_WorkState(const LingdbTree& pLingbTree)
            : lingbTree(pLingbTree)
            , strToLangSpecs()
            , tradSpecs()
            , nextLinesSpec(NextLineSpec::NOTDEFINED) {}

        std::size_t maxOccupatedSize() const;
        WlksDatabaseLoader_LangSpec* getLangSpec(const std::string& pLanguage);
        std::list<WlksDatabaseLoader_TradSpec*> getTraductionsOfALanguage(const std::string& pLanguage);

        const LingdbTree& lingbTree;
        std::map<std::string, WlksDatabaseLoader_LangSpec> strToLangSpecs;
        std::list<WlksDatabaseLoader_TradSpec> tradSpecs;
        NextLineSpec nextLinesSpec;
    };

    void loadAndSave(const std::filesystem::path& pFilename, const LingdbTree& pLingbTree) const;

    void load(WlksDatabaseLoader_WorkState& pWorkState, const std::filesystem::path& pFilename) const;

private:
    void xLoadDynNewDb(WlksDatabaseLoader_WorkState& pWorkState, const std::string& pLang) const;

    void xGetNextMeaningInLine(LingdbMeaning** pMeaning,
                               char& pConfidence,
                               std::size_t& pCurrPos,
                               const LinguisticIntermediaryDatabase& pLingDb,
                               const std::string& pLine) const;

    bool xHasSlash(const std::string& pStr) const;

    void xRemoveSlashesBeforeSpecifcChars(std::string& pStr) const;
};

}    // End of namespace onsem

#endif    // ONSEM_COMPILERMODEL_LOADERS_WLKSDATABASELOADER_HPP
