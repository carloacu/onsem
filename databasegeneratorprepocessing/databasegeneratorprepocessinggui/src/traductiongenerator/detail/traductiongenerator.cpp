#include "../traductiongenerator.hpp"
#include <fstream>
#include <filesystem>
#include <onsem/common/enum/semanticlanguageenum.hpp>
#include <onsem/compilermodel/loaders/wlksdatabaseloader.hpp>
#include <onsem/compilermodel/savers/traductionwriter.hpp>

namespace onsem
{

namespace traductiongenerator
{

void run(const LingdbTree& pLingDbTree,
         const std::string& pTmpFolder,
         const std::string& pInputResourcePath)
{
  auto resultFolderPath = pTmpFolder + "/traductiongenerator";
  std::filesystem::create_directory(resultFolderPath);

  const std::string tradComponantsFilename = resultFolderPath + "/traductionsComponants.wlks";
  // write request file
  {
    std::ofstream traductionsComponants(tradComponantsFilename.c_str());
    traductionsComponants << "> nextswillbe traduction english to french" << std::endl;
    traductionsComponants << "> include ../wikitionaryExtractor_traductions/wikionary_en_to_fr.wlks" << std::endl;
    traductionsComponants << "> include ../webtranslator/web_en_to_fr.wlks" << std::endl;
    traductionsComponants << "> include ../webtranslator/web_en_to_fr_fromReverse.wlks" << std::endl;
    traductionsComponants << "> nextswillbe traduction french to english" << std::endl;
    traductionsComponants << "> include ../wikitionaryExtractor_traductions/wikionary_fr_to_en.wlks" << std::endl;
    traductionsComponants << "> include ../webtranslator/web_fr_to_en.wlks" << std::endl;
    traductionsComponants << "> include ../webtranslator/web_fr_to_en_fromReverse.wlks" << std::endl;
    traductionsComponants.close();
  }

  WlksDatabaseLoader tradLoader;
  WlksDatabaseLoader::WlksDatabaseLoader_WorkState tradSummary(pLingDbTree);
  tradLoader.load(tradSummary, tradComponantsFilename);

  LingdbSaverTraductionWriter writer;
  for (std::list<WlksDatabaseLoader::WlksDatabaseLoader_TradSpec>::iterator
       itTradSpecs = tradSummary.tradSpecs.begin();
       itTradSpecs != tradSummary.tradSpecs.end(); ++itTradSpecs)
  {
    LinguisticIntermediaryDatabase& inLingDb = *itTradSpecs->inLingDb.lingDatabase;
    SemanticLanguageEnum inLang = semanticLanguageTypeGroundingEnumFromStr
        (inLingDb.getLanguage()->toStr());
    SemanticLanguageEnum outLang = semanticLanguageTypeGroundingEnumFromStr
        (itTradSpecs->outLingDb.lingDatabase->getLanguage()->toStr());

    if (inLang == SemanticLanguageEnum::ENGLISH &&
        outLang == SemanticLanguageEnum::FRENCH)
    {
      std::map<LingdbMeaning*, std::set<MeaningAndConfidence>>&
          trads = itTradSpecs->traductions;
      for (auto& currTrad : trads)
      {
        LingdbMeaning& inMeaning = *currTrad.first;
        if (inMeaning.getPartOfSpeech() == PartOfSpeech::ADJECTIVE)
        {
          std::string adjWordStr = inMeaning.getLemma()->getWord();
          static const std::string ingEndingStr = "ing";
          static const std::size_t ingEndingStr_size = ingEndingStr.size();
          std::size_t adjWordStr_size = adjWordStr.size();
          if (adjWordStr_size > ingEndingStr_size &&
              adjWordStr.compare(adjWordStr_size - ingEndingStr_size,
                                 ingEndingStr_size, ingEndingStr) == 0)
          {
            LingdbDynamicTrieNode* adjWordNode = inLingDb.getPointerToEndOfWord(adjWordStr);
            if (adjWordNode != nullptr)
            {
              const ForwardPtrList<LingdbWordForms>* wordsFromsForThisWord = adjWordNode->getWordForms();
              while (wordsFromsForThisWord != nullptr)
              {
                LingdbMeaning* correspondingVerbMeaning = wordsFromsForThisWord->elt->getMeaning();
                if (correspondingVerbMeaning->getPartOfSpeech() == PartOfSpeech::VERB)
                {
                  auto itVerbTrad = trads.find(correspondingVerbMeaning);
                  if (itVerbTrad != trads.end() &&
                      !itVerbTrad->second.empty())
                  {
                    MeaningAndConfidence meaningToAdd = *itVerbTrad->second.begin();
                    currTrad.second.insert(meaningToAdd);
                  }
                }
                wordsFromsForThisWord = wordsFromsForThisWord->next;
              }
            }
          }
        }
      }

      const std::string outputFilename = pInputResourcePath + "/common/readonly/english_to_french.wlks";
      std::cout << "writing: " << outputFilename << std::endl;
      writer.writeSummaryTraductions(outputFilename, trads);
    }
    else if (inLang == SemanticLanguageEnum::FRENCH &&
             outLang == SemanticLanguageEnum::ENGLISH)
    {
      const std::string outputFilename = pInputResourcePath + "/common/readonly/french_to_english.wlks";
      std::cout << "writing: " << outputFilename << std::endl;
      writer.writeSummaryTraductions(outputFilename,
                                     itTradSpecs->traductions);
    }
  }
}


} // End of namespace traductiongenerator

} // End of namespace onsem
