#ifndef ONSEM_COMPILERMODEL_SAVERS_TRADUCTIONWRITER_HPP
#define ONSEM_COMPILERMODEL_SAVERS_TRADUCTIONWRITER_HPP

#include <filesystem>
#include <string>
#include <onsem/compilermodel/linguisticintermediarydatabase.hpp>
#include <onsem/compilermodel/lingdbmeaning.hpp>
#include <onsem/compilermodel/lingdbdynamictrienode.hpp>
#include <onsem/compilermodel/lingdbwordforms.hpp>
#include <onsem/compilermodel/meaningandconfidence.hpp>

namespace onsem {

struct LingdbSaverOutLinks {
    LingdbSaverOutLinks()
        : linkedWithInMeaningGram()
        , notLinkedWithInMeaningGram() {}

    std::set<LingdbMeaning*> linkedWithInMeaningGram;
    std::set<LingdbMeaning*> notLinkedWithInMeaningGram;
};

class LingdbSaverTraductionWriter {
public:
    void writeTranslations(const std::filesystem::path& pFilnename,
                           const std::map<LingdbMeaning*, LingdbSaverOutLinks>& pTrads1,
                           const std::map<LingdbMeaning*, LingdbSaverOutLinks>& pTrads2,
                           const LinguisticIntermediaryDatabase& pInLingDatabase) const;

    void writeTraductionsForOneWiki(const std::string& pFilnename,
                                    const std::map<LingdbMeaning*, LingdbSaverOutLinks>& pTrads);

    void writeTraductionsForOneWiki_fromReverseTraductions(const std::string& pFilnename,
                                                           const std::map<LingdbMeaning*, LingdbSaverOutLinks>& pTrads);

    void writeSummaryTraductions(const std::string& pFilnename,
                                 const std::map<LingdbMeaning*, std::set<MeaningAndConfidence> >& pTrads);

private:
    void xWriteTrads(std::ofstream& pOutfile,
                     const std::string& pSep,
                     const std::set<MeaningAndConfidence>& pTradWithConfidences) const;

    void xWriteMeaning(std::ofstream& pOutfile, const LingdbMeaning* pMeaning) const;

    void xWriteMeaningWithConfidence(std::ofstream& pOutfile,
                                     const LingdbMeaning* pMeaning,
                                     const std::string& pSep,
                                     char pConfidence) const;

    void xWriteStr(std::ofstream& pOutfile, const std::string& pStr) const;

    std::set<MeaningAndConfidence>::const_iterator xGetMeaningOfTradWC(
        const std::set<MeaningAndConfidence>& pTradWithConfidences,
        const LingdbMeaning* pMeaning) const;

    void xGetTradsWithConfidence(std::set<MeaningAndConfidence>& pTradWithConfidences,
                                 PartOfSpeech pInPartOfSpeech,
                                 const LingdbSaverOutLinks& pOutLinks) const;

    void xMerge2Confidences_withSecondConfLessImportant(
        std::set<MeaningAndConfidence>& pTradWithConfidences,
        const std::set<MeaningAndConfidence>& pTradWithConfidences2) const;

    const LingdbSaverOutLinks* xGetOutLkForAMeaning(const std::map<LingdbMeaning*, LingdbSaverOutLinks>& pTrads,
                                                    LingdbMeaning* pMeaning) const;
};

}    // End of namespace onsem

#endif    // ONSEM_COMPILERMODEL_SAVERS_TRADUCTIONWRITER_HPP
