#include "chunkslinker.hpp"
#include <algorithm>
#include <onsem/texttosemantic/dbtype/inflection/nominalinflections.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/conceptset.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/staticlinguisticdictionary.hpp>
#include <onsem/texttosemantic/tool/inflectionschecker.hpp>
#include <onsem/texttosemantic/algorithmsetforalanguage.hpp>
#include <onsem/texttosemantic/type/syntacticgraph.hpp>
#include "../tool/chunkshandler.hpp"

namespace onsem {
namespace linguistics {

ChunksLinker::ChunksLinker(const AlgorithmSetForALanguage& pConfiguration)
    : fConfiguration(pConfiguration)
    , fLingDico(fConfiguration.getLingDico()) {}

void ChunksLinker::process(std::list<ChunkLink>& pFirstChildren) const {
    History history;
    for (auto it = pFirstChildren.begin(); it != pFirstChildren.end(); ++it) {
        std::list<Chunk*> stack;
        _linkPronounToMorePreciseSubject(*it, history, stack, pFirstChildren);

        PartOfSpeech headPartOfSpeech = it->chunk->head->inflWords.front().word.partOfSpeech;
        ChunkType chunkType = it->chunk->type;
        switch (chunkType) {
            case ChunkType::NOMINAL_CHUNK: {
                if (headPartOfSpeech != PartOfSpeech::PRONOUN_SUBJECT && headPartOfSpeech != PartOfSpeech::ADVERB
                    && headPartOfSpeech != PartOfSpeech::INTERJECTION)
                    history.prevNominal[0].chLk = std::make_unique<IterToChkLink>(pFirstChildren, it);
                break;
            }
            case ChunkType::INFINITVE_VERB_CHUNK: {
                history.prevNominal[0].chLk = std::make_unique<IterToChkLink>(pFirstChildren, it);
                break;
            }
            case ChunkType::SEPARATOR_CHUNK: {
                if (headPartOfSpeech == PartOfSpeech::LINKBETWEENWORDS && history.prevNominal[0].chLk)
                    history.prevNominal[0].sepToDelete = std::make_unique<IterToChkLink>(pFirstChildren, it);
                break;
            }
            default: break;
        }
    }
}

void ChunksLinker::_linkPronounToMorePreciseSubject(ChunkLink& pChunkLink,
                                                    History& pHistory,
                                                    std::list<Chunk*>& pStack,
                                                    std::list<ChunkLink>& pFirstChildren) const {
    if (pChunkLink.chunk->type == ChunkType::VERB_CHUNK || pChunkLink.chunk->type == ChunkType::INFINITVE_VERB_CHUNK)
        _newSentence(pHistory);

    for (ChunkLink& currChkLk : pChunkLink.chunk->children) {
        if (!currChkLk.chunk->children.empty()) {
            pStack.emplace_back(&*currChkLk.chunk);
            _linkPronounToMorePreciseSubject(currChkLk, pHistory, pStack, pFirstChildren);
            pStack.pop_back();
        }

        PartOfSpeech headPartOfSpeech = currChkLk.chunk->head->inflWords.front().word.partOfSpeech;
        if (currChkLk.type == ChunkLinkType::SUBJECT) {
            if (!pChunkLink.chunk->head->inflWords.front().infos.hasContextualInfo(
                    WordContextualInfos::CANNOTLINKSUBJECT)) {
                if (headPartOfSpeech == PartOfSpeech::PRONOUN || headPartOfSpeech == PartOfSpeech::PRONOUN_SUBJECT)
                    _tryToLinkToAPrevWord(currChkLk, pHistory, pStack);
                else
                    _saveTheLink(pHistory.prevSubject[0], currChkLk);
            }
        } else if (currChkLk.type == ChunkLinkType::DIRECTOBJECT || currChkLk.type == ChunkLinkType::INDIRECTOBJECT) {
            if (headPartOfSpeech == PartOfSpeech::PRONOUN || headPartOfSpeech == PartOfSpeech::PRONOUN_COMPLEMENT)
                _tryToLinkToAPrevWord(currChkLk, pHistory, pStack);
            else
                _saveTheLink(pHistory.prevDO[0], currChkLk);
        }
    }
}

void ChunksLinker::_tryToLinkToAPrevWord(ChunkLink& pPronounChkLk, History& pHistory, std::list<Chunk*>& pStack) const {
    auto _canLinkWith = [this, &pPronounChkLk, &pStack](ChunkLink& pPrevElt, bool pRefChunkIsAtRoot) {
        return canSubstituteAChunkByAnother(pPronounChkLk, *pPrevElt.chunk, pRefChunkIsAtRoot)
            && std::find(pStack.begin(), pStack.end(), &*pPrevElt.chunk) == pStack.end()
            && !hasAChild(*pPronounChkLk.chunk, *pPrevElt.chunk);
    };

    if (pHistory.prevNominal[1].chLk && _canLinkWith(*pHistory.prevNominal[1].chLk->it, true)) {
        _linkPronounToReferentChunk(pPronounChkLk, *pHistory.prevNominal[1].chLk->it);
        pHistory.prevNominal[1].chLk->list.erase(pHistory.prevNominal[1].chLk->it);
        if (pHistory.prevNominal[1].sepToDelete)
            pHistory.prevNominal[1].sepToDelete->list.erase(pHistory.prevNominal[1].sepToDelete->it);
        pHistory.prevNominal[1].reset();
    } else if (pHistory.prevSubject[1] != nullptr && _canLinkWith(*pHistory.prevSubject[1], false))
        _linkPronounToReferentChunk(pPronounChkLk, *pHistory.prevSubject[1]);
    else if (pHistory.prevDO[1] != nullptr && _canLinkWith(*pHistory.prevDO[1], false))
        _linkPronounToReferentChunk(pPronounChkLk, *pHistory.prevDO[1]);
    else if (pHistory.prevDO[0] != nullptr && _canLinkWith(*pHistory.prevDO[0], false))
        _linkPronounToReferentChunk(pPronounChkLk, *pHistory.prevDO[0]);
}

bool _pronCanSubstituteAInflWord(const InflectedWord& pPronToSubstitute, const InflectedWord& pInflWord) {
    if (pInflWord.word.partOfSpeech != PartOfSpeech::PROPER_NOUN) {
        if (!InflectionsChecker::areInflectionsCompatibles(pPronToSubstitute.inflections(), pInflWord.inflections()))
            return false;
        if (ConceptSet::haveAConceptThatBeginWith(pInflWord.infos.concepts, "agent_profession_"))
            return false;
        if (pPronToSubstitute.word == SemanticWord(SemanticLanguageEnum::FRENCH, "on", PartOfSpeech::PRONOUN_SUBJECT))
            return false;

        if (pPronToSubstitute.infos.hasContextualInfo(WordContextualInfos::REFTOAPERSON)) {
            if (pInflWord.word.partOfSpeech == PartOfSpeech::VERB)
                return false;
            if (partOfSpeech_isPronominal(pInflWord.word.partOfSpeech)) {
                if (!pInflWord.infos.hasContextualInfo(WordContextualInfos::REFTOAPERSON))
                    return false;
            } else if (pPronToSubstitute.word.language == SemanticLanguageEnum::ENGLISH
                       && partOfSpeech_isNominal(pInflWord.word.partOfSpeech)
                       && !ConceptSet::haveAConceptThatBeginWith(pInflWord.infos.concepts, "agent_"))
                return false;
        }
    }
    return true;
}

bool ChunksLinker::canSubstituteAChunkByAnother(const ChunkLink& pPronounChkLk,
                                                const Chunk& pRefChunk,
                                                bool pRefChunkIsAtRoot) const {
    const InflectionsChecker& flsChecker = fConfiguration.getFlsChecker();
    const InflectedWord& pronToSubstitute = pPronounChkLk.chunk->head->inflWords.front();

    if (!pRefChunkIsAtRoot && pronToSubstitute.infos.hasContextualInfo(WordContextualInfos::REFTOASENTENCE))
        return false;

    if (chunkTypeIsAList(pRefChunk.type)) {
        if (flsChecker.pronounSetAt3ePers(pronToSubstitute) && flsChecker.pronounCanBePlural(pronToSubstitute)) {
            for (const auto& currElt : pRefChunk.children) {
                if (currElt.type != ChunkLinkType::SIMPLE)
                    continue;
                const InflectedWord& eltIWord = currElt.chunk->head->inflWords.front();
                if (!_pronCanSubstituteAInflWord(pronToSubstitute, eltIWord))
                    return false;
            }
            return true;
        }
        return false;
    }
    if (pRefChunk.type == ChunkType::SEPARATOR_CHUNK)
        return false;

    const InflectedWord& refIWord = pRefChunk.head->inflWords.front();
    if (!_pronCanSubstituteAInflWord(pronToSubstitute, refIWord))
        return false;

    if (pronToSubstitute.word.partOfSpeech == PartOfSpeech::PRONOUN_COMPLEMENT
        && refIWord.word.partOfSpeech == PartOfSpeech::PRONOUN)
        return pronToSubstitute.inflections() == refIWord.inflections();

    if (!flsChecker.pronounSetAt3ePers(pronToSubstitute))
        return false;

    switch (refIWord.word.partOfSpeech) {
        case PartOfSpeech::NOUN:
            return (pRefChunkIsAtRoot || pPronounChkLk.type != ChunkLinkType::SUBJECT
                    || pronToSubstitute.word.partOfSpeech != PartOfSpeech::PRONOUN)
                && flsChecker.pronounCanBePlural(pronToSubstitute) == flsChecker.nounCanBePlural(refIWord);
        case PartOfSpeech::PRONOUN:
            return !fConfiguration.getFlsChecker().pronounCanReferToA3ePers(refIWord)
                && flsChecker.pronounCanBePlural(pronToSubstitute) == flsChecker.pronounCanBePlural(refIWord);
        case PartOfSpeech::INTERJECTION: return false;
        default: return true;
    }
}

void ChunksLinker::_linkPronounToReferentChunk(ChunkLink& pPronounToLink, ChunkLink& pReferentChunkLink) {
    // put the pronoun in the link label
    pPronounToLink.tokRange = pPronounToLink.chunk->tokRange;
    auto& inflRefWord = pReferentChunkLink.chunk->head->inflWords.front();
    if (inflRefWord.word.partOfSpeech == PartOfSpeech::PROPER_NOUN) {
        auto* properNounInflections = inflRefWord.inflections().getNominalIPtr();
        if (properNounInflections == nullptr || properNounInflections->inflections.empty()) {
            const auto& pronInflWord = pPronounToLink.chunk->head->inflWords.front();
            auto* inflsPronounPtr = pronInflWord.inflections().getPronominalIPtr();
            if (inflsPronounPtr != nullptr)
                inflRefWord.moveInflections(
                    pronInflWord.inflections().getOtherInflectionsType(InflectionType::NOMINAL));
        }
    }
    // put the pronoun children in the referent children
    pReferentChunkLink.chunk->children.insert(pReferentChunkLink.chunk->children.end(),
                                              pPronounToLink.chunk->children.begin(),
                                              pPronounToLink.chunk->children.end());
    // replace the pronoun chunk by the referent chunk
    pPronounToLink.chunk = pReferentChunkLink.chunk;
    // say that the chunk has not only one father
    pReferentChunkLink.chunk->hasOnlyOneReference = false;
}

void ChunksLinker::_saveTheLink(ChunkLink*& pSaveLink, ChunkLink& pSubject) const {
    const InflectedWord& subjectHeadIGram = pSubject.chunk->head->inflWords.front();
    if (chunkTypeIsAList(pSubject.chunk->type) || subjectHeadIGram.word.partOfSpeech == PartOfSpeech::NOUN
        || subjectHeadIGram.word.partOfSpeech == PartOfSpeech::PROPER_NOUN
        || ((subjectHeadIGram.word.partOfSpeech == PartOfSpeech::PRONOUN
             || subjectHeadIGram.word.partOfSpeech == PartOfSpeech::PRONOUN_SUBJECT)
            && fConfiguration.getFlsChecker().pronounSetAt3ePers(subjectHeadIGram)))
        pSaveLink = &pSubject;
    else
        pSaveLink = nullptr;
}

void ChunksLinker::_newSentence(History& pHistory) {
    pHistory.prevNominal[1] = std::move(pHistory.prevNominal[0]);
    pHistory.prevNominal[0].reset();
    if (pHistory.prevSubject[0] != nullptr) {
        pHistory.prevSubject[1] = pHistory.prevSubject[0];
        pHistory.prevSubject[0] = nullptr;
    }
    if (pHistory.prevDO[0] != nullptr) {
        pHistory.prevDO[1] = pHistory.prevDO[0];
        pHistory.prevDO[0] = nullptr;
    }
}

}    // End of namespace linguistics
}    // End of namespace onsem
