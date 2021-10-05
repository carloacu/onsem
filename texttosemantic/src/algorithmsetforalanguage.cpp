#include <onsem/texttosemantic/algorithmsetforalanguage.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/staticlinguisticdictionary.hpp>
#include "syntacticgraphgenerator/interjectinalchunker.hpp"
#include "syntacticgraphgenerator/verbalchunker.hpp"
#include "syntacticgraphgenerator/nominalchunker.hpp"
#include "syntacticgraphgenerator/subordinateextractor.hpp"
#include "syntacticgraphgenerator/errordetector.hpp"
#include "syntacticgraphgenerator/chunkslinker.hpp"
#include "syntacticgraphgenerator/entityrecognizer.hpp"
#include "syntacticgraphgenerator/listextractor.hpp"
#include "syntacticgraphgenerator/verbaltomoninalchunkslinker.hpp"


namespace onsem
{
namespace linguistics
{

AlgorithmSetForALanguage::AlgorithmSetForALanguage(const LinguisticDatabase& pLingDb,
                                                   SemanticLanguageEnum pLanguageType)
  : lingDb(pLingDb),
    fLanguageType(pLanguageType),
    fSpecLingDb(pLingDb.langToSpec[pLanguageType]),
    fListExtractor(),
    fVerbChunker(),
    fIntChunker(),
    fChunker(),
    fVerbToNounLinker(),
    fSubordonatesExtractor(),
    fErrorDetector(),
    fLinker(),
    fEntityRecognizer()
{
  xResetConfiguration();
}



void AlgorithmSetForALanguage::xResetConfiguration()
{
  fVerbChunker = std::shared_ptr<VerbalChunker>(new VerbalChunker(*this));
  fIntChunker = std::shared_ptr<InterjectionalChunker>(new InterjectionalChunker(*this));
  fChunker = std::shared_ptr<NominalChunker>(new NominalChunker(*this));
  fVerbToNounLinker = std::shared_ptr<VerbalToNominalChunksLinker>(new VerbalToNominalChunksLinker(*this));
  fSubordonatesExtractor = std::shared_ptr<SubordinateExtractor>(new SubordinateExtractor(*this));
  fErrorDetector = std::shared_ptr<ErrorDetector>(new ErrorDetector(*this));
  fLinker = std::shared_ptr<ChunksLinker>(new ChunksLinker(*this));
  fEntityRecognizer = std::shared_ptr<EntityRecognizer>(new EntityRecognizer(*this));
  fListExtractor = std::shared_ptr<ListExtractor>(new ListExtractor(*this));
}


const InflectionsChecker& AlgorithmSetForALanguage::getFlsChecker() const
{
  return getSpecifcLingDb().inflectionsChecker();
}

const ListExtractor& AlgorithmSetForALanguage::getListExtractor() const
{
  return *fListExtractor;
}

const VerbalChunker& AlgorithmSetForALanguage::getVerbChunker() const
{
  return *fVerbChunker;
}

const InterjectionalChunker& AlgorithmSetForALanguage::getIntChunker() const
{
  return *fIntChunker;
}

const NominalChunker& AlgorithmSetForALanguage::getChunker() const
{
  return *fChunker;
}

const VerbalToNominalChunksLinker& AlgorithmSetForALanguage::getVerbToNounLinker() const
{
  return *fVerbToNounLinker;
}

const SubordinateExtractor& AlgorithmSetForALanguage::getSubordonatesExtractor() const
{
  return *fSubordonatesExtractor;
}


const ErrorDetector& AlgorithmSetForALanguage::getErrorDetector() const
{
  return *fErrorDetector;
}

const ChunksLinker& AlgorithmSetForALanguage::getLinker() const
{
  return *fLinker;
}

const EntityRecognizer& AlgorithmSetForALanguage::getEntityRecognizer() const
{
  return *fEntityRecognizer;
}

const SpecificLinguisticDatabase& AlgorithmSetForALanguage::getSpecifcLingDb() const
{
  return fSpecLingDb;
}


const LinguisticDictionary& AlgorithmSetForALanguage::getLingDico() const
{
  return fSpecLingDb.lingDico;
}


} // End of namespace linguistics
} // End of namespace onsem
