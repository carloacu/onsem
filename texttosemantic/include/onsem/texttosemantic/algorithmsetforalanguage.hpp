#ifndef ONSEM_TEXTTOSEMANTIC_ALGORITHMSETFORALANGUAGE_HPP
#define ONSEM_TEXTTOSEMANTIC_ALGORITHMSETFORALANGUAGE_HPP

#include <vector>
#include <memory>
#include <onsem/common/enum/semanticlanguagetype.hpp>
#include "api.hpp"


namespace onsem
{
class ALLingSyntacticAnalyzerSyntAnalysis;
namespace linguistics
{
struct LinguisticDatabase;
struct SpecificLinguisticDatabase;
class InflectionsChecker;
class EntityRecognizer;
class ListExtractor;
class ChunksLinker;
class InterjectionalChunker;
class VerbalChunker;
class NominalChunker;
class VerbalToNominalChunksLinker;
class SubordinateExtractor;
class ErrorDetector;
class LinguisticDictionary;


class ONSEM_TEXTTOSEMANTIC_API AlgorithmSetForALanguage
{
public:
  AlgorithmSetForALanguage(const LinguisticDatabase& pLingDb,
                           SemanticLanguageEnum pLanguageType);

  const LinguisticDictionary& getLingDico() const;
  const InflectionsChecker& getFlsChecker() const;
  const ListExtractor& getListExtractor() const;
  const VerbalChunker& getVerbChunker() const;
  const InterjectionalChunker& getIntChunker() const;
  const NominalChunker& getChunker() const;
  const VerbalToNominalChunksLinker& getVerbToNounLinker() const;
  const SubordinateExtractor& getSubordonatesExtractor() const;
  const ErrorDetector& getErrorDetector() const;
  const ChunksLinker& getLinker() const;
  const EntityRecognizer& getEntityRecognizer() const;

  SemanticLanguageEnum getLanguageType() const { return fLanguageType; }
  const SpecificLinguisticDatabase& getSpecifcLingDb() const;


  const LinguisticDatabase& lingDb;

private:
  SemanticLanguageEnum fLanguageType;
  const SpecificLinguisticDatabase& fSpecLingDb;

  std::shared_ptr<ListExtractor> fListExtractor;

  std::shared_ptr<VerbalChunker> fVerbChunker;
  std::shared_ptr<InterjectionalChunker> fIntChunker;
  std::shared_ptr<NominalChunker> fChunker;
  std::shared_ptr<VerbalToNominalChunksLinker> fVerbToNounLinker;
  std::shared_ptr<SubordinateExtractor> fSubordonatesExtractor;
  std::shared_ptr<ErrorDetector> fErrorDetector;
  std::shared_ptr<ChunksLinker> fLinker;
  std::shared_ptr<EntityRecognizer> fEntityRecognizer;

  void xResetConfiguration();
};


} // End of namespace linguistics
} // End of namespace onsem

#endif // ONSEM_TEXTTOSEMANTIC_ALGORITHMSETFORALANGUAGE_HPP
