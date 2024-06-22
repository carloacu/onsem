#include <onsem/texttosemantic/type/chunk.hpp>
#include <onsem/texttosemantic/type/chunklink.hpp>

namespace onsem {
namespace linguistics {

Chunk::Chunk(const TokenRange& pTokRange, ChunkType pType)
    : tokRange(pTokRange)
    , head(pTokRange.getItBegin())
    , hasOnlyOneReference(true)
    , positive(true)
    , form(LingVerbForm::DECLARATIVE)
    , requests()
    , requestCanBeObject(false)
    , requestWordTokenPosOpt()
    , isPassive(false)
    , type(pType)
    , children()
    , introductingWordToSaveForSynthesis() {}

const SemanticWord& Chunk::getHeadWord() const {
    return head->inflWords.front().word;
}

PartOfSpeech Chunk::getHeadPartOfSpeech() const {
    return head->inflWords.front().word.partOfSpeech;
}

const WordAssociatedInfos& Chunk::getHeadAssInfos() const {
    return head->inflWords.front().infos;
}

const std::map<std::string, char>& Chunk::getHeadConcepts() const {
    return head->inflWords.front().infos.concepts;
}

TokenRange Chunk::getTokRangeWrappingChildren() const {
    auto res = tokRange;
    for (auto& currChildLk : children) {
        const Chunk& currChild = *currChildLk.chunk;
        currChild._increaseTokRangeWrappingChildren(res);
    }
    return res;
}


void Chunk::_increaseTokRangeWrappingChildren(TokenRange& pTokenRange) const
{
    pTokenRange.mergeWith(tokRange);
    for (auto& currChildLk : children) {
        const Chunk& currChild = *currChildLk.chunk;
        currChild._increaseTokRangeWrappingChildren(pTokenRange);
    }
}


}    // End of namespace linguistics
}    // End of namespace onsem
