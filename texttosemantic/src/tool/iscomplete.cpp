#include <onsem/texttosemantic/tool/iscomplete.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase.hpp>
#include <onsem/texttosemantic/linguisticanalyzer.hpp>
#include "chunkshandler.hpp"

namespace onsem {
namespace linguistics {

mystd::optional<bool> _skipSeparators(std::list<ChunkLink>::const_reverse_iterator& pItChild,
                                      const std::list<ChunkLink>& pChildren) {
    mystd::optional<bool> res;
    PartOfSpeech lextChunkHeadPartOfSpeech = pItChild->chunk->head->inflWords.front().word.partOfSpeech;
    if (lextChunkHeadPartOfSpeech == PartOfSpeech::SUBORDINATING_CONJONCTION) {
        res.emplace(false);
    } else if (lextChunkHeadPartOfSpeech == PartOfSpeech::LINKBETWEENWORDS) {
        ++pItChild;
        if (pItChild == pChildren.rend())
            res.emplace(true);
    }
    return res;
}

bool isComplete_fromSyntGraph(const SyntacticGraph& pSyntGraph) {
    for (auto itChild = pSyntGraph.firstChildren.rbegin(); itChild != pSyntGraph.firstChildren.rend(); ++itChild) {
        const Chunk& currChunk = *itChild->chunk;

        switch (currChunk.type) {
            case ChunkType::VERB_CHUNK: {
                if (currChunk.requests.has(SemanticRequestType::TIME)) {
                    auto itSubject = getChunkLink(currChunk, ChunkLinkType::SUBJECT);
                    if (itSubject != currChunk.children.end() && checkOrder(*itSubject->chunk, currChunk))
                        return false;
                }

                for (auto& currVerbChild : currChunk.children)
                    if (currVerbChild.chunk->getHeadPartOfSpeech() == PartOfSpeech::DETERMINER
                        && currVerbChild.chunk->children.empty())
                        return false;

                ++itChild;
                if (itChild != pSyntGraph.firstChildren.rend()) {
                    if (itChild->chunk->type == ChunkType::SEPARATOR_CHUNK) {
                        auto subRes = _skipSeparators(itChild, pSyntGraph.firstChildren);
                        if (subRes)
                            return *subRes;
                    }

                    if (currChunk.requests.has(SemanticRequestType::ACTION)
                        && itChild->chunk->requests.has(SemanticRequestType::ACTION))
                        return false;
                }
                return true;
            }
            case ChunkType::NOMINAL_CHUNK: {
                ++itChild;
                if (itChild != pSyntGraph.firstChildren.rend()) {
                    if (itChild->chunk->type == ChunkType::SEPARATOR_CHUNK) {
                        auto subRes = _skipSeparators(itChild, pSyntGraph.firstChildren);
                        if (subRes)
                            return *subRes;
                    }

                    if (itChild->chunk->type == ChunkType::NOMINAL_CHUNK)
                        return false;
                }
                return true;
            }
            case ChunkType::AND_CHUNK:
            case ChunkType::OR_CHUNK:
            case ChunkType::THEN_CHUNK:
            case ChunkType::THEN_REVERSED_CHUNK: {
                if (currChunk.children.empty() || !checkOrder(currChunk, *currChunk.children.back().chunk))
                    return false;
                return true;
            }
            case ChunkType::PREPOSITIONAL_CHUNK: return false;
            default: return true;
        }
    }

    return true;
}

bool isComplete(const std::string& pText, const LinguisticDatabase& pLingDb, SemanticLanguageEnum pLanguageType) {
    SyntacticGraph syntGraph(pLingDb, pLanguageType);
    std::shared_ptr<ResourceGroundingExtractor> cmdGrdExtractorPtr;
    const std::set<SpellingMistakeType> spellingMistakeTypesPossible;
    tokenizationAndSyntacticalAnalysis(syntGraph, pText, spellingMistakeTypesPossible, cmdGrdExtractorPtr);
    return isComplete_fromSyntGraph(syntGraph);
}

}    // End of namespace linguistics
}    // End of namespace onsem
