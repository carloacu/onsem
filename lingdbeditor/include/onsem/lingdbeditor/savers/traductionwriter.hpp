#ifndef ALLINGDBEDITOR_TRADUCTIONWRITER_H
#define ALLINGDBEDITOR_TRADUCTIONWRITER_H

#include <string>
#include <onsem/lingdbeditor/linguisticintermediarydatabase.hpp>
#include <onsem/lingdbeditor/allingdbmeaning.hpp>
#include <onsem/lingdbeditor/allingdbdynamictrienode.hpp>
#include <onsem/lingdbeditor/allingdbwordforms.hpp>
#include <onsem/lingdbeditor/meaningandconfidence.hpp>


namespace onsem
{

struct LingdbSaverOutLinks
{
  LingdbSaverOutLinks()
    : linkedWithInMeaningGram(),
      notLinkedWithInMeaningGram()
  {
  }

  std::set<ALLingdbMeaning*> linkedWithInMeaningGram;
  std::set<ALLingdbMeaning*> notLinkedWithInMeaningGram;
};



class LingdbSaverTraductionWriter
{
public:
  void writeTranslations
  (const boost::filesystem::path& pFilnename,
   const std::map<ALLingdbMeaning*, LingdbSaverOutLinks>& pTrads1,
   const std::map<ALLingdbMeaning*, LingdbSaverOutLinks>& pTrads2,
   const LinguisticIntermediaryDatabase& pInLingDatabase) const;

  void writeTraductionsForOneWiki
  (const std::string& pFilnename,
   const std::map<ALLingdbMeaning*, LingdbSaverOutLinks>& pTrads);

  void writeTraductionsForOneWiki_fromReverseTraductions
  (const std::string& pFilnename,
   const std::map<ALLingdbMeaning*, LingdbSaverOutLinks>& pTrads);

  void writeSummaryTraductions
  (const std::string& pFilnename,
   const std::map<ALLingdbMeaning*, std::set<MeaningAndConfidence> >& pTrads);

private:
  void xWriteTrads
  (std::ofstream& pOutfile,
   const std::string& pSep,
   const std::set<MeaningAndConfidence>& pTradWithConfidences) const;

  void xWriteMeaning
  (std::ofstream& pOutfile,
  const ALLingdbMeaning* pMeaning) const;

  void xWriteMeaningWithConfidence
  (std::ofstream& pOutfile,
   const ALLingdbMeaning* pMeaning,
   const std::string& pSep,
   char pConfidence) const;

  void xWriteStr
  (std::ofstream& pOutfile,
   const std::string& pStr) const;

  std::set<MeaningAndConfidence>::const_iterator xGetMeaningOfTradWC
  (const std::set<MeaningAndConfidence>& pTradWithConfidences,
   const ALLingdbMeaning* pMeaning) const;

  void xGetTradsWithConfidence
  (std::set<MeaningAndConfidence>& pTradWithConfidences,
   PartOfSpeech pInPartOfSpeech,
   const LingdbSaverOutLinks& pOutLinks) const;

  void xMerge2Confidences_withSecondConfLessImportant
  (std::set<MeaningAndConfidence>& pTradWithConfidences,
   const std::set<MeaningAndConfidence>& pTradWithConfidences2) const;

  const LingdbSaverOutLinks* xGetOutLkForAMeaning
  (const std::map<ALLingdbMeaning*, LingdbSaverOutLinks>& pTrads,
   ALLingdbMeaning* pMeaning) const;
};

} // End of namespace onsem

#endif // ALLINGDBEDITOR_TRADUCTIONWRITER_H
