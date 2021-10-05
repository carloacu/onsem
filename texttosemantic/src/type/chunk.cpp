#include <onsem/texttosemantic/type/chunk.hpp>
#include <onsem/texttosemantic/type/chunklink.hpp>


namespace onsem
{
namespace linguistics
{

Chunk::Chunk(
    const TokenRange& pTokRange,
    ChunkType pType)
  : tokRange(pTokRange),
    head(pTokRange.getItBegin()),
    hasOnlyOneReference(true),
    positive(true),
    form(LingVerbForm::DECLARATIVE),
    requests(),
    requestCanBeObject(false),
    requestWordTokenPosOpt(),
    isPassive(false),
    type(pType),
    children(),
    introductingWordToSaveForSynthesis()
{
}

PartOfSpeech Chunk::getHeadPartOfSpeech() const
{
  return head->inflWords.front().word.partOfSpeech;
}

const WordAssociatedInfos& Chunk::getHeadAssInfos() const
{
  return head->inflWords.front().infos;
}

const std::map<std::string, char>& Chunk::getHeadConcepts() const
{
  return head->inflWords.front().infos.concepts;
}


} // End of namespace linguistics
} // End of namespace onsem
