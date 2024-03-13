#include "questionwords.hpp"
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticgenericgrounding.hpp>
#include <onsem/texttosemantic/tool/syntacticanalyzertokenshandler.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/semanticframedictionary.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/staticlinguisticdictionary.hpp>
#include <onsem/texttosemantic/type/enumsconvertions.hpp>
#include <onsem/texttosemantic/type/syntacticgraph.hpp>
#include "../tool/chunkshandler.hpp"
#include "../syntacticgraphgenerator/entityrecognizer.hpp"

namespace onsem {
namespace linguistics {
namespace questionWords {

SemanticRequestType _getAppropriateRequestAccordingToTheContext(
    const StaticLinguisticDictionary::QuestionWords& pQuestionWord,
    Chunk& pVerbChunk,
    std::vector<Token>::iterator pItQWord,
    bool pAfterVerb,
    bool pIsLatestQuestionWord,
    const ChunkLink& pCurrChunkLink,
    const SemanticFrameDictionary& pFrameDic) {
    SemanticRequestType res = pQuestionWord.request;
    if (pQuestionWord.followedByRequestedWord) {
        if (pIsLatestQuestionWord) {
            auto holdingChunkItEnd = pCurrChunkLink.chunk->tokRange.getItEnd();
            if (pItQWord != holdingChunkItEnd) {
                auto itNext = getNextToken(pItQWord, holdingChunkItEnd);
                if (itNext != holdingChunkItEnd)
                    pVerbChunk.requestWordTokenPosOpt = itNext->tokenPos;
            }
        }
    } else if (pQuestionWord.request == SemanticRequestType::SUBJECT && pQuestionWord.canBeAlone) {
        if (pAfterVerb || haveASubject(pVerbChunk))
            res = SemanticRequestType::OBJECT;
        else
            pVerbChunk.requestCanBeObject = true;
    }
    if (!pCurrChunkLink.tokRange.isEmpty()) {
        InflectedWord& verbInflWord = pVerbChunk.head->inflWords.front();
        bool willBeAbleToSynthesizeIt = true;
        auto chunkLinkTypeOpt = pFrameDic.getChunkLinkFromContext(
            &verbInflWord, willBeAbleToSynthesizeIt, &pCurrChunkLink.tokRange.getItBegin()->inflWords.front(), nullptr);
        if (chunkLinkTypeOpt) {
            auto requestOpt = linguistics::chunkTypeToRequestType(*chunkLinkTypeOpt);
            if (requestOpt)
                return *requestOpt;
        }
    }
    return res;
}

bool _checkIfCloseToVerb(bool pAfterVerb, std::vector<Token>::iterator pItQWord, const Chunk& pVerbChunk) {
    if (pAfterVerb)
        return getPrevToken(pItQWord, pVerbChunk.head, pVerbChunk.tokRange.getItEnd()) == pVerbChunk.head;
    auto itNext = getNextToken(pItQWord, pVerbChunk.head);
    while (itNext != pVerbChunk.head && itNext->inflWords.front().word.partOfSpeech == PartOfSpeech::PRONOUN_COMPLEMENT)
        itNext = getNextToken(itNext, pVerbChunk.head);
    return itNext == pVerbChunk.head;
}

struct PotentialQWordChunkLink {
    PotentialQWordChunkLink(ChunkLink& pChunkLink, const StaticLinguisticDictionary::QuestionWords& pQuestionWord)
        : chunkLink(pChunkLink)
        , questionWord(pQuestionWord) {}
    ChunkLink& chunkLink;
    const StaticLinguisticDictionary::QuestionWords& questionWord;
};

bool _addIfItIsAListOfQWords(ChunkLinkWorkingZone& pWorkingZone,
                             std::list<ChunkLink>::iterator& pListChunkLink,    // iterator that point to the chunk that
                                                                                // contains the potential question word
                             Chunk& pVerbChunk,    // verb chunk maybe related to the question word
                             bool pAfterVerb,      // if the potential question is after the verb or not
                             const SpecificLinguisticDatabase& pSpecLingDb) {
    auto& rootQWordsChunk = *pListChunkLink->chunk;
    std::list<PotentialQWordChunkLink> potentialQWOrdChunks;
    for (auto& currChild : rootQWordsChunk.children) {
        if (currChild.type != ChunkLinkType::SIMPLE)
            continue;
        auto itHead = currChild.chunk->head;
        bool isCloseToTheVerb = _checkIfCloseToVerb(pAfterVerb, itHead, pVerbChunk);
        const InflectedWord& mainInflWord = itHead->inflWords.front();
        const auto* qWordPtr =
            pSpecLingDb.lingDico.statDb.wordToQuestionWord(mainInflWord.word, pAfterVerb, isCloseToTheVerb);
        if (qWordPtr != nullptr) {
            potentialQWOrdChunks.emplace_back(currChild, *qWordPtr);
        }
    }
    if (potentialQWOrdChunks.empty())
        return false;
    pVerbChunk.form = LingVerbForm::INTERROGATIVE;
    const auto& frameDic = pSpecLingDb.getSemFrameDict();
    std::size_t index = 0;
    std::size_t lastIndex = potentialQWOrdChunks.size() - 1;
    for (auto& currQWordChunkLink : potentialQWOrdChunks) {
        bool isLatestQuestionWord = index == lastIndex;
        SemanticRequestType requestType =
            _getAppropriateRequestAccordingToTheContext(currQWordChunkLink.questionWord,
                                                        pVerbChunk,
                                                        currQWordChunkLink.chunkLink.chunk->head,
                                                        pAfterVerb,
                                                        isLatestQuestionWord,
                                                        currQWordChunkLink.chunkLink,
                                                        frameDic);
        pVerbChunk.requests.add(requestType);
        ++index;
    }

    auto delCurrChunkLink = pListChunkLink;
    ++pListChunkLink;
    delCurrChunkLink->type = ChunkLinkType::QUESTIONWORD;
    addChildInGoodOrder(pVerbChunk.children, pWorkingZone.syntTree(), delCurrChunkLink);
    return true;
}

bool _addIfIsAQWord(ChunkLinkWorkingZone& pWorkingZone,
                    std::list<ChunkLink>::iterator&
                        pCurrChunkLink,    // iterator that point to the chunk that contains the potential question word
                    Chunk& pVerbChunk,     // verb chunk maybe related to the question word
                    std::vector<Token>::iterator pItQWord,    // iterator point to the potential question word
                    bool pAfterVerb,                          // if the potential question is after the verb or not
                    const SpecificLinguisticDatabase& pSpecLingDb) {
    bool isCloseToTheVerb = _checkIfCloseToVerb(pAfterVerb, pItQWord, pVerbChunk);
    const InflectedWord& mainInflWord = pItQWord->inflWords.front();
    const auto* qWordPtr =
        pSpecLingDb.lingDico.statDb.wordToQuestionWord(mainInflWord.word, pAfterVerb, isCloseToTheVerb);
    if (qWordPtr != nullptr) {
        const auto& qWord = *qWordPtr;
        const auto& holdingChunkTokRange = pCurrChunkLink->chunk->tokRange;
        TokenRange qWordTokenRange = tokenToTokenRange(pItQWord, holdingChunkTokRange);
        bool hasOnlyPrepositionsBefore = true;
        for (auto itPrepToken = holdingChunkTokRange.getItBegin(); itPrepToken != qWordTokenRange.getItBegin();
             itPrepToken = getNextToken(itPrepToken, qWordTokenRange.getItBegin())) {
            if (itPrepToken->inflWords.front().word.partOfSpeech != PartOfSpeech::PREPOSITION) {
                hasOnlyPrepositionsBefore = false;
                break;
            }
        }

        auto holdingChunkTokRangeItEnd = holdingChunkTokRange.getItEnd();
        // handle the tokens before the question word but that are in the question word chunk
        if (qWordTokenRange.getItBegin() != holdingChunkTokRange.getItBegin()) {
            auto language = pSpecLingDb.lingDico.statDb.getLanguageType();
            if (hasOnlyPrepositionsBefore) {
                putBeginOfAChunkInTheChunkLink(
                    pWorkingZone.syntTree(), pCurrChunkLink, qWordTokenRange.getItBegin(), language);
            } else {
                separateEndOfAChunk(pWorkingZone.syntTree(),
                                    pCurrChunkLink,
                                    qWordTokenRange.getItBegin(),
                                    ChunkType::NOMINAL_CHUNK,
                                    language);
                ++pCurrChunkLink;
            }
        }
        if (pVerbChunk.form != LingVerbForm::INTERROGATIVE || qWord.request != SemanticRequestType::YESORNO) {
            pVerbChunk.form = LingVerbForm::INTERROGATIVE;
            const auto& frameDic = pSpecLingDb.getSemFrameDict();
            // if we have specific request for the couple "verb" - "question word"
            auto requestType = _getAppropriateRequestAccordingToTheContext(
                qWord, pVerbChunk, pItQWord, pAfterVerb, true, *pCurrChunkLink, frameDic);
            pVerbChunk.requests.set(requestType);
        }
        if (qWordTokenRange.getItEnd() != holdingChunkTokRangeItEnd) {
            separateBeginOfAChunk(pWorkingZone.syntTree(),
                                  pCurrChunkLink,
                                  qWordTokenRange.getItEnd(),
                                  ChunkType::NOMINAL_CHUNK,
                                  pItQWord,
                                  pSpecLingDb.lingDico.statDb.getLanguageType());
            --pCurrChunkLink;
        }

        auto delCurrChunkLink = pCurrChunkLink;
        ++pCurrChunkLink;
        delCurrChunkLink->type = ChunkLinkType::QUESTIONWORD;
        addChildInGoodOrder(pVerbChunk.children, pWorkingZone.syntTree(), delCurrChunkLink);
        return true;
    }
    return false;
}

void _addQuestionWords(ChunkLinkWorkingZone& pWorkingZone,
                       std::list<ChunkLink>::iterator& pCurrChunkLink,
                       Chunk& pVerbChunk,
                       bool pAfterVerb,
                       const SpecificLinguisticDatabase& pSpecLingDb) {
    // iterate over all chunks
    for (; pCurrChunkLink != pWorkingZone.end(); ++pCurrChunkLink) {
        // stop the search at the first separator found
        if (pCurrChunkLink->chunk->type == ChunkType::SEPARATOR_CHUNK) {
            ++pCurrChunkLink;
            return;
        }
        if (chunkTypeIsAList(pCurrChunkLink->chunk->type)) {
            if (_addIfItIsAListOfQWords(pWorkingZone, pCurrChunkLink, pVerbChunk, pAfterVerb, pSpecLingDb))
                return;
        } else {
            // iterate over all the words of the chunk
            bool qWordHasBeenLinked = false;
            do {
                qWordHasBeenLinked = false;

                for (auto itWord = getTheNextestToken(pCurrChunkLink->chunk->tokRange.getItBegin(),
                                                      pCurrChunkLink->chunk->tokRange.getItEnd(),
                                                      SkipPartOfWord::YES);
                     itWord != pCurrChunkLink->chunk->tokRange.getItEnd();
                     itWord = getNextToken(itWord, pCurrChunkLink->chunk->tokRange.getItEnd(), SkipPartOfWord::YES)) {
                    if (_addIfIsAQWord(pWorkingZone, pCurrChunkLink, pVerbChunk, itWord, pAfterVerb, pSpecLingDb)) {
                        qWordHasBeenLinked = true;
                        break;
                    }
                }

                if (pCurrChunkLink == pWorkingZone.end())
                    return;
            } while (qWordHasBeenLinked);
        }
    }
}

void addQuestionWords(ChunkLinkWorkingZone& pWorkingZone,
                      Chunk* pFirstVerbChunk,
                      Chunk* pSecondVerbChunk,
                      const SpecificLinguisticDatabase& pSpecLingDb) {
    // try to add question words to the second verb chunk
    if (pSecondVerbChunk != nullptr) {
        auto currChunkLink = pWorkingZone.begin();
        advanceBeforeLastSeparator(currChunkLink, pWorkingZone);
        _addQuestionWords(pWorkingZone, currChunkLink, *pSecondVerbChunk, false, pSpecLingDb);
    }

    // try to add question words to the first verb chunk
    if (pFirstVerbChunk != nullptr) {
        auto currChunkLink = pWorkingZone.begin();
        _addQuestionWords(pWorkingZone, currChunkLink, *pFirstVerbChunk, true, pSpecLingDb);
    }
}

void extractQuestionWordsInsideAVerbGroup(std::list<ChunkLink>& pSyntTree,
                                          std::list<ChunkLink>::iterator pVerbChunkLink,
                                          const SpecificLinguisticDatabase& pSpecLingDb) {
    std::list<ChunkLink>::iterator ChLend = pVerbChunkLink;
    ++ChLend;
    ChunkLinkWorkingZone workingZone(pSyntTree, pVerbChunkLink, ChLend);

    TokIt itFirstToken = pVerbChunkLink->chunk->tokRange.getItBegin();
    if (itFirstToken != pVerbChunkLink->chunk->head) {
        std::list<ChunkLink>::iterator chLkVerb = pVerbChunkLink;
        _addIfIsAQWord(workingZone, chLkVerb, *pVerbChunkLink->chunk, itFirstToken, false, pSpecLingDb);
    }

    TokIt lastTok = getPrevToken(pVerbChunkLink->chunk->tokRange.getItEnd(),
                                 pVerbChunkLink->chunk->head,
                                 pVerbChunkLink->chunk->tokRange.getItEnd());
    if (lastTok != pVerbChunkLink->chunk->tokRange.getItEnd()) {
        std::list<ChunkLink>::iterator chLkVerb = pVerbChunkLink;
        _addIfIsAQWord(workingZone, chLkVerb, *pVerbChunkLink->chunk, lastTok, true, pSpecLingDb);
    }
}

}    // End of namespace questionWords
}    // End of namespace linguistics
}    // End of namespace onsem
