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

TokenRange Chunk::getTokRangeWrappingChildren(const std::set<ChunkLinkType>* pExceptChildrenPtr) const {
    auto res = tokRange;
    _increaseTokRangeWrappingOnlyChildren(res, pExceptChildrenPtr);
    return res;
}


void Chunk::_increaseTokRangeWrappingChildren(TokenRange& pTokenRange, const std::set<ChunkLinkType>* pExceptChildrenPtr) const
{
    pTokenRange.mergeWith(tokRange);
    _increaseTokRangeWrappingOnlyChildren(pTokenRange, pExceptChildrenPtr);
}


void Chunk::_increaseTokRangeWrappingOnlyChildren(TokenRange& pTokenRange, const std::set<ChunkLinkType>* pExceptChildrenPtr) const
{
    for (auto& currChildLk : children) {
        if (pExceptChildrenPtr != nullptr && pExceptChildrenPtr->count(currChildLk.type) > 0) {
            continue;
        }
        const Chunk& currChild = *currChildLk.chunk;
        currChild._increaseTokRangeWrappingChildren(pTokenRange, pExceptChildrenPtr);
    }
}



}    // End of namespace linguistics
}    // End of namespace onsem
