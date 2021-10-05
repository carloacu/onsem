#ifndef ONSEM_COMMON_KEYTOSTREAMS_HPP
#define ONSEM_COMMON_KEYTOSTREAMS_HPP

#include <list>
#include <map>
#include <fstream>
#include "api.hpp"
#include <onsem/common/enum/semanticlanguagetype.hpp>

namespace onsem
{
namespace linguistics
{

struct ONSEM_COMMON_API KeyToStreams
{
  std::istream* mainDicToStream = nullptr;
  std::istream* animationsToStream = nullptr;
  std::istream* synthesizerToStream = nullptr;
  std::map<SemanticLanguageEnum, std::istream*> translationStreams;
  std::map<std::string, std::istream*> conversionsStreams;
  void clear();
};

struct ONSEM_COMMON_API LinguisticDatabaseStreams
{
  std::istream* concepts = nullptr;
  std::map<SemanticLanguageEnum, KeyToStreams> languageToStreams{};
  std::list<std::istream*> dynamicContentStreams;
  void clear();
};

struct ONSEM_COMMON_API KeyToFStreams
{
  std::list<std::ifstream> keyToIfStream;
  LinguisticDatabaseStreams linguisticDatabaseStreams;

  void addMainDicFile(SemanticLanguageEnum pLanguage,
                      const std::string& pFilename);
  void addAnimationsFile(SemanticLanguageEnum pLanguage,
                         const std::string& pFilename);
  void addSynthesizerFile(SemanticLanguageEnum pLanguage,
                          const std::string& pFilename);
  void addFile(SemanticLanguageEnum pInLanguage,
               SemanticLanguageEnum pOutLanguage,
               const std::string& pFilename);
  void addConceptFile(const std::string& pFilename);
  void addDynamicContentFile(const std::string& pFilename);
  void addConversionFile(SemanticLanguageEnum pLanguage,
                         const std::string& pLocalPath,
                         const std::string& pFilename);
  void close();

private:
  std::ifstream& _addKeyToIfStreamFile(
      SemanticLanguageEnum pLanguage,
      const std::string& pFilename);
};


} // End of namespace linguistics
} // End of namespace onsem



#endif // ONSEM_COMMON_KEYTOSTREAMS_HPP
