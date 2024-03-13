#include "incompletelistsresolver.hpp"
#include <set>
#include <onsem/common/utility/optional.hpp>
#include <onsem/texttosemantic/type/chunklink.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/conceptset.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/linguisticdictionary.hpp>
#include "listextractor.hpp"

namespace onsem {
namespace linguistics {
namespace {
bool _hasOnlyInterjection(std::list<ChunkLink>::iterator pItBegin, std::list<ChunkLink>::iterator pItEnd) {
    for (auto it = pItBegin; it != pItEnd; ++it)
        if (it->chunk->type != ChunkType::INTERJECTION_CHUNK)
            return false;
    return true;
}

bool _hasOnlyInterjectionOrInterrogationMark(std::list<ChunkLink>::iterator pItBegin,
                                             std::list<ChunkLink>::iterator pItEnd) {
    for (auto it = pItBegin; it != pItEnd; ++it)
        if (it->chunk->type != ChunkType::INTERJECTION_CHUNK
            && (it->chunk->type != ChunkType::SEPARATOR_CHUNK || it->chunk->head->inflWords.front().word.lemma != "?"))
            return false;
    return true;
}
}

void resolveIncompleteLists(std::list<ChunkLink>& pChunkList, const LinguisticDictionary& pLingDico) {
    static const std::set<ChunkType> chunkTypeToAddInList = {
        ChunkType::VERB_CHUNK, ChunkType::INFINITVE_VERB_CHUNK, ChunkType::NOMINAL_CHUNK};

    for (auto it = pChunkList.begin(); it != pChunkList.end(); ++it) {
        Chunk& currChunk = *it->chunk;
        if (currChunk.type == ChunkType::SEPARATOR_CHUNK) {
            mystd::optional<ChunkType> chunkTypeOpt = getListType(currChunk.getHeadConcepts());
            if (chunkTypeOpt) {
                auto nextIt = it;
                ++nextIt;

                auto itLinkTowardLastListElt = pChunkList.end();
                if (nextIt != pChunkList.end()
                    && (nextIt->chunk->type == ChunkType::SEPARATOR_CHUNK
                        || (nextIt->chunk->type == ChunkType::NOMINAL_CHUNK
                            && nextIt->chunk->getHeadPartOfSpeech() == PartOfSpeech::ADVERB))) {
                    const std::map<std::string, char>& nextChunkHeadConcepts = nextIt->chunk->getHeadConcepts();
                    auto newChunkTypeOpt = getListType(nextChunkHeadConcepts);
                    if (newChunkTypeOpt) {
                        chunkTypeOpt = newChunkTypeOpt;
                        nextIt = pChunkList.erase(nextIt);    // TODO: here we can remove some words from the original
                                                              // text in the syntactic tree
                    } else if (ConceptSet::haveAConcept(nextChunkHeadConcepts, "time_relative_after")) {
                        chunkTypeOpt.emplace(ChunkType::THEN_CHUNK);
                        itLinkTowardLastListElt = nextIt;
                        ++nextIt;
                    } else if (ConceptSet::haveAConcept(nextChunkHeadConcepts, "time_relative_before")) {
                        chunkTypeOpt.emplace(ChunkType::THEN_REVERSED_CHUNK);
                        itLinkTowardLastListElt = nextIt;
                        ++nextIt;
                    }
                }

                bool hasAListEltBefore = !_hasOnlyInterjection(pChunkList.begin(), it);
                bool hasAListEltAfter = !_hasOnlyInterjectionOrInterrogationMark(nextIt, pChunkList.end());
                if (!hasAListEltAfter && !hasAListEltBefore) {
                    currChunk.type = *chunkTypeOpt;
                    if (itLinkTowardLastListElt != pChunkList.end()) {
                        pChunkList.erase(itLinkTowardLastListElt);
                        itLinkTowardLastListElt = pChunkList.end();
                    }
                } else if (hasAListEltAfter && !hasAListEltBefore) {
                    Chunk& nextChunk = *nextIt->chunk;
                    if (chunkTypeToAddInList.count(nextChunk.type)) {
                        currChunk.type = *chunkTypeOpt;
                        if (itLinkTowardLastListElt != pChunkList.end()) {
                            nextIt->tokRange = itLinkTowardLastListElt->chunk->tokRange;
                            pChunkList.erase(itLinkTowardLastListElt);
                            itLinkTowardLastListElt = pChunkList.end();
                        }
                        currChunk.children.splice(currChunk.children.end(), pChunkList, nextIt);
                    }
                } else if (hasAListEltBefore && !hasAListEltAfter) {
                    auto prevIt = it;
                    --prevIt;

                    mystd::optional<ChunkType> currListChunkType;
                    while (true) {
                        Chunk& prevChunk = *prevIt->chunk;
                        bool addToTheList = false;
                        if (currListChunkType) {
                            addToTheList = *currListChunkType == prevChunk.type;
                        } else if (chunkTypeToAddInList.count(prevChunk.type)) {
                            currListChunkType.emplace(prevChunk.type);
                            addToTheList = true;
                        }

                        if (addToTheList) {
                            currChunk.type = *chunkTypeOpt;
                            auto itToDel = prevIt;
                            ++prevIt;
                            currChunk.children.splice(currChunk.children.begin(), pChunkList, itToDel);
                            if (prevIt == pChunkList.begin())
                                break;
                            --prevIt;
                        } else {
                            break;
                        }
                    }
                } else if (hasAListEltBefore && hasAListEltAfter && it != pChunkList.begin()) {
                    Chunk& nextChunk = *nextIt->chunk;
                    if (chunkTypeIsVerbal(nextChunk.type)) {
                        auto prevIt = it;
                        --prevIt;
                        Chunk& prevChunk = *prevIt->chunk;
                        if (prevChunk.type == ChunkType::NOMINAL_CHUNK) {
                            auto* qWordPtr = pLingDico.statDb.wordToQuestionWord(
                                prevChunk.head->inflWords.front().word, false, false);
                            if (qWordPtr != nullptr) {
                                auto* verbalChunkWithQuestionWord = getChunkWithAQuestionWordChild(nextChunk);
                                if (verbalChunkWithQuestionWord != nullptr) {
                                    nextChunk.requests.addFront(qWordPtr->request);
                                    prevIt->type = ChunkLinkType::QUESTIONWORD;
                                    verbalChunkWithQuestionWord->children.splice(
                                        verbalChunkWithQuestionWord->children.begin(), pChunkList, prevIt);
                                    it = pChunkList.erase(it);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

}    // End of namespace linguistics
}    // End of namespace onsem
