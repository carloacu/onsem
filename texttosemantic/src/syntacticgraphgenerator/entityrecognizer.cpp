#include "entityrecognizer.hpp"
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticgenericgrounding.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticmetagrounding.hpp>
#include <onsem/texttosemantic/tool/semexpgetter.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/conceptset.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/semanticframedictionary.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/staticlinguisticdictionary.hpp>
#include <onsem/texttosemantic/dbtype/inflection/verbalinflections.hpp>
#include <onsem/texttosemantic/tool/inflectionschecker.hpp>
#include <onsem/texttosemantic/algorithmsetforalanguage.hpp>
#include <onsem/texttosemantic/tool/syntacticanalyzertokenshandler.hpp>
#include <onsem/texttosemantic/type/enumsconvertions.hpp>
#include "../tool/chunkshandler.hpp"

namespace onsem {
namespace linguistics {
namespace {

bool _getNextWordWithoutIntro(ConstTokenIterator& pNextToken, const TokenRange& pTokRange) {
    const TokIt& tokRangeEnd = pTokRange.getItEnd();
    for (TokIt itTok = pTokRange.getItBegin(); itTok != tokRangeEnd; itTok = getNextToken(itTok, tokRangeEnd)) {
        const InflectedWord& linkTokenInfoGram = itTok->inflWords.front();
        if (linkTokenInfoGram.word.partOfSpeech == PartOfSpeech::LINKBETWEENWORDS) {
            continue;
        }
        if (linkTokenInfoGram.word.partOfSpeech == PartOfSpeech::PREPOSITION
            || linkTokenInfoGram.word.partOfSpeech == PartOfSpeech::PRONOUN_COMPLEMENT
            || linkTokenInfoGram.word.partOfSpeech == PartOfSpeech::SUBORDINATING_CONJONCTION
            || linkTokenInfoGram.word.partOfSpeech == PartOfSpeech::PARTITIVE) {
            continue;
        }
        pNextToken.setTokenIt(itTok);
        return true;
    }
    return false;
}

InflectedWord* _getIntroWord(const TokenRange& pTokRange) {
    const TokIt& tokRangeEnd = pTokRange.getItEnd();
    for (TokIt itTok = pTokRange.getItBegin(); itTok != tokRangeEnd; itTok = getNextToken(itTok, tokRangeEnd)) {
        const InflectedWord& linkTokenInfoGram = itTok->inflWords.front();
        if (linkTokenInfoGram.word.partOfSpeech == PartOfSpeech::LINKBETWEENWORDS
            || linkTokenInfoGram.word.partOfSpeech == PartOfSpeech::ADVERB
            || linkTokenInfoGram.word.partOfSpeech == PartOfSpeech::DETERMINER) {
            continue;
        }
        if (linkTokenInfoGram.word.partOfSpeech == PartOfSpeech::PREPOSITION
            || linkTokenInfoGram.word.partOfSpeech == PartOfSpeech::PRONOUN_COMPLEMENT
            || linkTokenInfoGram.word.partOfSpeech == PartOfSpeech::SUBORDINATING_CONJONCTION
            || linkTokenInfoGram.word.partOfSpeech == PartOfSpeech::PARTITIVE) {
            return &itTok->inflWords.front();
        }
        return nullptr;
    }
    return nullptr;
}

}

EntityRecognizer::EntityRecognizer(const AlgorithmSetForALanguage& pConfiguration)
    : fConfiguration(pConfiguration)
    , fSpecLingDb(fConfiguration.getSpecifcLingDb())
    , fLingDico(fConfiguration.getLingDico()) {}

bool _canBeAnIndirectObject(const InflectedWord& pInflWord, SemanticLanguageEnum pLanguage) {
    PartOfSpeech objGram = pInflWord.word.partOfSpeech;
    return ((partOfSpeech_isPronominal(objGram))
            && pInflWord.infos.hasContextualInfo(WordContextualInfos::REFTOAPERSON))
        || (objGram == PartOfSpeech::NOUN && ConceptSet::haveAConceptThatBeginWith(pInflWord.infos.concepts, "agent_"))
        || (objGram == PartOfSpeech::PROPER_NOUN && pLanguage == SemanticLanguageEnum::ENGLISH);
}

void EntityRecognizer::process(std::list<ChunkLink>& pChunkList) const {
    SemanticLanguageEnum language = fConfiguration.getLanguageType();

    for (auto& currChkLk : pChunkList) {
        ChunkType currChunkType = currChkLk.chunk->type;
        if (chunkTypeIsVerbal(currChunkType)) {
            const InflectedWord& verbIGram = currChkLk.chunk->head->inflWords.front();
            ChunkLinkType verbFollowedBy = ChunkLinkType::SIMPLE;

            StaticLinguisticMeaning verbLingMeaning;
            SemExpGetter::wordToAStaticMeaning(verbLingMeaning, verbIGram.word, language, fConfiguration.lingDb);
            if (!verbLingMeaning.isEmpty()) {
                const auto& statLingDico = fConfiguration.lingDb.langToSpec[verbLingMeaning.language].lingDico.statDb;
                verbFollowedBy = xVerbFollowedBy(verbLingMeaning, statLingDico.getRootMeaning(verbLingMeaning));
            }

            Chunk& auxChunk = whereToLinkSubject(*currChkLk.chunk);
            ChunkLink* subjectChunkLink = getSubjectChunkLink(auxChunk);
            xAddComplementsOfVerbFromBegin(
                *currChkLk.chunk, subjectChunkLink, auxChunk, auxChunk, auxChunk.head, auxChunk.tokRange.getItEnd());
            if (language == SemanticLanguageEnum::ENGLISH) {
                if (&auxChunk != &*currChkLk.chunk)
                    xAddComplementsOfVerbFromBegin(*currChkLk.chunk,
                                                   subjectChunkLink,
                                                   *currChkLk.chunk,
                                                   *currChkLk.chunk,
                                                   currChkLk.chunk->head,
                                                   currChkLk.chunk->tokRange.getItEnd());
            } else    // for french
            {
                xAddComplementsOfVerbFromEnd(*currChkLk.chunk,
                                             subjectChunkLink,
                                             auxChunk,
                                             auxChunk,
                                             auxChunk.tokRange.getItBegin(),
                                             auxChunk.head,
                                             verbFollowedBy);
            }

            // move directObject -> indirectObject for some verbs
            if (verbFollowedBy == ChunkLinkType::INDIRECTOBJECT
                && !haveChild(*currChkLk.chunk, ChunkLinkType::INDIRECTOBJECT)) {
                for (auto& currChild : currChkLk.chunk->children) {
                    if (currChild.type == ChunkLinkType::DIRECTOBJECT) {
                        const InflectedWord& objIGram = currChild.chunk->head->inflWords.front();
                        if (_canBeAnIndirectObject(objIGram, language)
                            && (objIGram.word.partOfSpeech == PartOfSpeech::PROPER_NOUN
                                || objIGram.infos.hasContextualInfo(WordContextualInfos::REFTOAPERSON))) {
                            currChild.type = ChunkLinkType::INDIRECTOBJECT;
                            break;
                        }
                    }
                }
            }
        } else if (currChunkType == ChunkType::PREPOSITIONAL_CHUNK && currChkLk.type != ChunkLinkType::SIMPLE) {
            putPrepositionInCunkLkTokenRange(currChkLk, language);
        }
        process(currChkLk.chunk->children);
    }
}

ChunkLinkType EntityRecognizer::xGetChkLinkTypeFromChunkLink(const ChunkLink& pChunkLink,
                                                             Chunk* pFirstVerbChunk) const {
    for (TokIt itTok = pChunkLink.tokRange.getItBegin(); itTok != pChunkLink.tokRange.getItEnd();
         itTok = getNextToken(itTok, pChunkLink.tokRange.getItEnd())) {
        InflectedWord& linkTokenInfoGram = itTok->inflWords.front();
        if (linkTokenInfoGram.word.partOfSpeech == PartOfSpeech::PREPOSITION
            && ConceptSet::haveAConceptThatBeginWith(linkTokenInfoGram.infos.concepts, "location_relative_")) {
            return ChunkLinkType::LOCATION;
        }

        if (pFirstVerbChunk != nullptr) {
            auto* verbInflWordPtr = &pFirstVerbChunk->head->inflWords.front();
            auto linkType = getAppropriateChunkLink(verbInflWordPtr,
                                                    pChunkLink.chunk->introductingWordToSaveForSynthesis,
                                                    &linkTokenInfoGram,
                                                    pChunkLink.chunk->type,
                                                    nullptr);
            if (linkType)
                return *linkType;
        }
    }
    return ChunkLinkType::SIMPLE;
}

ChunkLinkType EntityRecognizer::findNatureOfAChunkLink(ChunkLink& pChunkLink,
                                                       Chunk* pFirstVerbChunk,
                                                       bool pFromEnglishPossessive) const {
    if (chunkTypeIsAList(pChunkLink.chunk->type) && pChunkLink.chunk->children.size() >= 1) {
        ChunkLinkType linkType = xGetChkLinkTypeFromChunkLink(pChunkLink, pFirstVerbChunk);
        if (linkType != ChunkLinkType::SIMPLE)
            return linkType;
        return xFindNatureOfANominalGroup(pChunkLink.chunk->children.front(), pFirstVerbChunk, pFromEnglishPossessive);
    }

    return xFindNatureOfANominalGroup(pChunkLink, pFirstVerbChunk, pFromEnglishPossessive);
}

bool _referToOneOrManyAgent(const Chunk& pChunk) {
    if (doesChunkHaveDeterminerBeforeHead(pChunk))
        return false;
    const InflectedWord& headIGram = pChunk.head->inflWords.front();
    PartOfSpeech headGram = headIGram.word.partOfSpeech;
    if (headGram == PartOfSpeech::PROPER_NOUN)
        return true;
    if (headGram == PartOfSpeech::NOUN)
        return ConceptSet::haveAConceptThatBeginWith(headIGram.infos.concepts, "agent_");

    if (headGram == PartOfSpeech::PARTITIVE && !pChunk.children.empty()
        && pChunk.children.front().chunk->type == ChunkType::AND_CHUNK
        && !pChunk.children.front().chunk->children.empty()) {
        return _referToOneOrManyAgent(*pChunk.children.front().chunk->children.front().chunk);
    }
    return false;
}

bool EntityRecognizer::isTheBeginOfAPartitive(const Chunk& pChunk) {
    return pChunk.tokRange.getItBegin()->inflWords.front().word.partOfSpeech == PartOfSpeech::PARTITIVE
        && (pChunk.head->inflWords.front().word.partOfSpeech != PartOfSpeech::PARTITIVE || !pChunk.children.empty());
}

bool _isMetaTimeChunk(const Chunk& pChunk) {
    if (!pChunk.head->str.empty() && pChunk.head->str[0] == SemanticMetaGrounding::firstCharOfStrOld
        && SemanticMetaGrounding::isTheBeginOfAParamOld(pChunk.head->str)) {
        SemanticGroundingType refToType = SemanticGroundingType::GENERIC;
        return SemanticMetaGrounding::groundingTypeFromStr(refToType, pChunk.head->str)
            && refToType == SemanticGroundingType::TIME;
    }
    return false;
}

ChunkLinkType EntityRecognizer::xFindNatureOfANominalGroup(ChunkLink& pChunkLink,
                                                           Chunk* pFirstVerbChunk,
                                                           bool pFromEnglishPossessive) const {
    Chunk& chunkToProcess = *pChunkLink.chunk;
    SemanticLanguageEnum language = fConfiguration.getLanguageType();

    if (_isMetaTimeChunk(chunkToProcess))
        return ChunkLinkType::TIME;

    auto* prepInflWordPtr = _getIntroWord(pChunkLink.tokRange);
    if (prepInflWordPtr == nullptr)
        prepInflWordPtr = _getIntroWord(chunkToProcess.tokRange);

    ConstTokenIterator nextToken(chunkToProcess.tokRange.getTokList(), 0);
    if (!_getNextWordWithoutIntro(nextToken, chunkToProcess.tokRange) && !chunkToProcess.tokRange.isEmpty()) {
        nextToken.setTokenIt(chunkToProcess.tokRange.getItBegin());
    }

    auto doesBeginWith = [&](PartOfSpeech pPartOfSpeech) {
        if (prepInflWordPtr != nullptr && prepInflWordPtr->word.partOfSpeech == pPartOfSpeech)
            return true;
        if (!chunkToProcess.tokRange.isEmpty()) {
            TokIt itBeginWordOfChunk = chunkToProcess.tokRange.getItBegin();
            if (itBeginWordOfChunk->inflWords.front().word.partOfSpeech == pPartOfSpeech)
                return true;
        }
        return false;
    };
    bool beginWithAPrep = doesBeginWith(PartOfSpeech::PREPOSITION);

    const InflectedWord& inflWordHead = chunkToProcess.head->inflWords.front();
    {
        if (pFirstVerbChunk != nullptr) {
            auto* verbalInflectionsPtr = pFirstVerbChunk->head->inflWords.front().inflections().getVerbalIPtr();
            if (verbalInflectionsPtr != nullptr && !verbalInflectionsPtr->inflections.empty()
                && verbalInflectionsPtr->inflections.front().tense == LinguisticVerbTense::PRESENT_INDICATIVE
                && prepInflWordPtr == nullptr && doesBeginWithAnIndefiniteDeterminer(chunkToProcess.tokRange))
                return ChunkLinkType::DIRECTOBJECT;
        }
        if (pFirstVerbChunk != nullptr
            && ConceptSet::haveAConceptThatBeginWith(pFirstVerbChunk->head->inflWords.front().infos.concepts,
                                                     "verb_action")
            && ConceptSet::haveAConceptThatBeginWith(inflWordHead.infos.concepts, "sequence_relative_")) {
            return ChunkLinkType::TIME;
        }
        if (!doesBeginWith(PartOfSpeech::PARTITIVE)
            && ConceptSet::haveAConceptThatBeginWith(inflWordHead.infos.concepts, "manner_")) {
            return ChunkLinkType::MANNER;
        }
        if (!beginWithAPrep && inflWordHead.word.partOfSpeech == PartOfSpeech::ADVERB
            && chunkToProcess.children.empty()) {
            if (ConceptSet::haveAConceptThatBeginWith(inflWordHead.infos.concepts, "accordance_disagreement_"))
                return ChunkLinkType::QUESTIONWORD;
            if (ConceptSet::haveAConceptThatBeginWith(inflWordHead.infos.concepts, "still_"))
                return ChunkLinkType::SIMPLE;
        }
        if (ConceptSet::haveAConcept(inflWordHead.infos.concepts, "times")) {
            TokIt itBeginWordOfChunk = chunkToProcess.tokRange.getItBegin();
            if (ConceptSet::haveAConceptThatBeginWith(itBeginWordOfChunk->inflWords.front().infos.concepts, "number_"))
                return ChunkLinkType::REPETITION;
        }
        if (inflWordHead.word.partOfSpeech == PartOfSpeech::INTERJECTION
            && (pFirstVerbChunk == nullptr
                || !ConceptSet::haveAConcept(pFirstVerbChunk->head->inflWords.front().infos.concepts,
                                             "verb_action_say"))) {
            return ChunkLinkType::INTERJECTION;
        }
    }

    if (pChunkLink.chunk->type == ChunkType::VERB_CHUNK)
        return ChunkLinkType::SUBORDINATE_CLAUSE;

    {
        InflectedWord* firstVerbInflWordPtr =
            pFirstVerbChunk == nullptr ? nullptr : &pFirstVerbChunk->head->inflWords.front();
        mystd::optional<ChunkLinkType> linkType;
        linkType = getAppropriateChunkLink(firstVerbInflWordPtr,
                                           chunkToProcess.introductingWordToSaveForSynthesis,
                                           prepInflWordPtr,
                                           chunkToProcess.type,
                                           &nextToken);
        if (linkType)
            return *linkType;

        if (pFromEnglishPossessive)
            return ChunkLinkType::OWNER;

        if (isTheBeginOfAPartitive(chunkToProcess)) {
            if (pFirstVerbChunk == nullptr) {
                if (_referToOneOrManyAgent(chunkToProcess))
                    return ChunkLinkType::OWNER;
                return ChunkLinkType::SPECIFICATION;
            }
            return ChunkLinkType::DIRECTOBJECT;
        }

        if (!beginWithAPrep && inflWordHead.word.partOfSpeech == PartOfSpeech::ADVERB
            && !doesBeginWith(PartOfSpeech::DETERMINER))
            return ChunkLinkType::SIMPLE;
    }

    if (!chunkToProcess.tokRange.isEmpty()) {
        TokIt itBeginWordOfChunk = chunkToProcess.tokRange.getItBegin();
        if (itBeginWordOfChunk->inflWords.front().word.partOfSpeech == PartOfSpeech::PREPOSITION) {
            const auto& conceptsBeginOfChunk = itBeginWordOfChunk->inflWords.front().infos.concepts;
            if (ConceptSet::haveAConceptThatBeginWith(conceptsBeginOfChunk, "location_relative_"))
                return ChunkLinkType::LOCATION;
        }
    }

    if (pChunkLink.chunk->type == ChunkType::VERB_CHUNK)
        return ChunkLinkType::SUBORDINATE_CLAUSE;
    if (pChunkLink.chunk->type == ChunkType::INFINITVE_VERB_CHUNK)
        return ChunkLinkType::DIRECTOBJECT;
    if (pFirstVerbChunk != nullptr && !beginWithAPrep && !haveADO(*pFirstVerbChunk)) {
        const SemanticWord& verbWord = pFirstVerbChunk->head->inflWords.front().word;
        if (verbWord.language != SemanticLanguageEnum::UNKNOWN) {
            StaticLinguisticMeaning verbLingMeaning;
            SemExpGetter::wordToAStaticMeaningInLanguage(
                verbLingMeaning, verbWord, verbWord.language, fConfiguration.lingDb);
            if (!verbLingMeaning.isEmpty()
                && fLingDico.hasContextualInfo(WordContextualInfos::CANBEFOLLOWEDBYINDIRECTOBJECT, verbLingMeaning)
                &&    // TODO: factorise the 2 code that use WordContextualInfos::CANBEFOLLOWEDBYINDIRECTOBJECT
                !haveAChildWithChunkLink(*pFirstVerbChunk, ChunkLinkType::INDIRECTOBJECT)
                && _canBeAnIndirectObject(chunkToProcess.head->inflWords.front(), language))
                return ChunkLinkType::INDIRECTOBJECT;
        }
        return ChunkLinkType::DIRECTOBJECT;
    }
    return ChunkLinkType::SUBORDINATE;
}

mystd::optional<ChunkLinkType> EntityRecognizer::getAppropriateChunkLink(
    InflectedWord* pVerbInflectedWord,
    mystd::optional<const SemanticWord*>& pIntroductingWord,
    InflectedWord* pPrepInflWordPtr,
    ChunkType pChunkType,
    const ConstTokenIterator* pNextToken) const {
    bool willBeAbleToSynthesizeIt = true;
    auto res = fSpecLingDb.getSemFrameDict().getChunkLinkFromContext(
        pVerbInflectedWord, willBeAbleToSynthesizeIt, pPrepInflWordPtr, pNextToken);
    if (res && (*res == ChunkLinkType::LOCATION || *res == ChunkLinkType::SUBORDINATE_CLAUSE)
        && (pChunkType == ChunkType::INFINITVE_VERB_CHUNK || pChunkType == ChunkType::VERB_CHUNK
            || pChunkType == ChunkType::AUX_CHUNK))
        res.reset();
    if (!willBeAbleToSynthesizeIt) {
        if (pPrepInflWordPtr != nullptr)
            pIntroductingWord = &pPrepInflWordPtr->word;
        else
            pIntroductingWord = nullptr;
    }
    return res;
}

bool EntityRecognizer::addSubordonatesToAVerb(Chunk& pVerbRoot, ChunkLinkWorkingZone& pWorkingZone) const {
    bool res = false;
    ChunkLink* prevSubordonate = nullptr;
    ChunkLink* currSubordonate = &*pWorkingZone.begin();
    while (true) {
        auto newChkType = findNatureOfAChunkLink(*currSubordonate, &pVerbRoot);
        bool needToAddToVerbRoot = true;
        if (currSubordonate->chunk->type == ChunkType::PREPOSITIONAL_CHUNK || newChkType == ChunkLinkType::INTERVAL
            || newChkType == ChunkLinkType::TIME || newChkType == ChunkLinkType::LENGTH
            || newChkType == ChunkLinkType::DURATION || newChkType == ChunkLinkType::LOCATION
            || newChkType == ChunkLinkType::REPETITION || newChkType == ChunkLinkType::MANNER
            || newChkType == ChunkLinkType::LANGUAGE || newChkType == ChunkLinkType::INDIRECTOBJECT
            || newChkType == ChunkLinkType::PURPOSE || newChkType == ChunkLinkType::WITH
            || newChkType == ChunkLinkType::ACCORDINGTO || newChkType == ChunkLinkType::AGAINST
            || newChkType == ChunkLinkType::BETWEEN || newChkType == ChunkLinkType::STARTING_POINT
            || newChkType == ChunkLinkType::INTERJECTION || newChkType == ChunkLinkType::UNITY
            || newChkType == ChunkLinkType::SIMPLE)
            currSubordonate->type = newChkType;
        else if (newChkType == ChunkLinkType::QUESTIONWORD) {
            currSubordonate->type = ChunkLinkType::SIMPLE;
            pVerbRoot.positive = !pVerbRoot.positive;
        } else if (prevSubordonate == nullptr)
            currSubordonate->type = ChunkLinkType::DIRECTOBJECT;
        else
            needToAddToVerbRoot = false;

        // if the verb already have a direct object maybe it should be a indirect object
        if (currSubordonate->type == ChunkLinkType::DIRECTOBJECT) {
            ChunkLink* doChunkLink = getDOChunkLink(pVerbRoot);
            if (doChunkLink != nullptr && partOfSpeech_isPronominal(doChunkLink->chunk->getHeadPartOfSpeech()))
                doChunkLink->type = ChunkLinkType::INDIRECTOBJECT;

            if (pVerbRoot.requests.has(SemanticRequestType::YESORNO)
                && currSubordonate->chunk->type == ChunkType::OR_CHUNK)
                pVerbRoot.requests.set(SemanticRequestType::OBJECT);
        }

        if (needToAddToVerbRoot) {
            if (prevSubordonate == nullptr)
                pVerbRoot.children.splice(pVerbRoot.children.end(), pWorkingZone.syntTree(), pWorkingZone.begin());
            else
                pVerbRoot.children.splice(pVerbRoot.children.end(),
                                          prevSubordonate->chunk->children,
                                          prevSubordonate->chunk->children.begin());
            res = true;
        }

        bool isAList = chunkTypeIsAList(currSubordonate->chunk->type);
        if (isAList || currSubordonate->chunk->children.empty())
            break;
        if (!currSubordonate->chunk->tokRange.isEmpty()
            && currSubordonate->chunk->tokRange.getItBegin()->inflWords.front().word.partOfSpeech
                   == PartOfSpeech::PARTITIVE) {
            if (currSubordonate->chunk->children.empty())
                break;
            prevSubordonate = currSubordonate;
            currSubordonate = &currSubordonate->chunk->children.front();
            continue;
        }
        break;
    }
    return res;
}

ChunkLinkType EntityRecognizer::xVerbFollowedBy(const StaticLinguisticMeaning& pVerMeaning,
                                                const StaticLinguisticMeaning& pVerRootbMeaning) const {
    if (fLingDico.hasContextualInfo(WordContextualInfos::CANBEFOLLOWEDBYINDIRECTOBJECT, pVerRootbMeaning)) {
        return ChunkLinkType::INDIRECTOBJECT;
    }
    if (fLingDico.hasContextualInfo(WordContextualInfos::CANBEFOLLOWEDBYLOCATION, pVerMeaning)) {
        return ChunkLinkType::LOCATION;
    }
    return ChunkLinkType::DIRECTOBJECT;
}

void EntityRecognizer::xAddComplementsOfVerbFromBegin(const Chunk& pRootVerb,
                                                      const ChunkLink* pSubjectChunkLink,
                                                      Chunk& pCurrentChunk,
                                                      Chunk& pChunkToSplit,
                                                      TokIt pItBeforeBegin,
                                                      TokIt pItEnd) const {
    for (TokIt it = getNextToken(pItBeforeBegin, pItEnd); it != pItEnd; it = getNextToken(it, pItEnd)) {
        if (it->inflWords.front().word.partOfSpeech == PartOfSpeech::PRONOUN_COMPLEMENT) {
            xLinkPronounComplementAfterVerb(pRootVerb, pSubjectChunkLink, pCurrentChunk, pChunkToSplit, it);
            break;
        }
    }
}

void EntityRecognizer::xAddComplementsOfVerbFromEnd(Chunk& pRootVerb,
                                                    ChunkLink* pSubjectChunkLink,
                                                    Chunk& pCurrentChunk,
                                                    Chunk& pChunkToSplit,
                                                    TokIt pItBegin,
                                                    TokIt pItEnd,
                                                    ChunkLinkType pVerbCanBeFollowedBy) const {
    for (TokIt it = getPrevToken(pItEnd, pItBegin, pItEnd); it != pItEnd; it = getPrevToken(it, pItBegin, pItEnd)) {
        if (it->inflWords.front().word.partOfSpeech == PartOfSpeech::PRONOUN_COMPLEMENT) {
            xLinkPronounComplementBeforeVerb(
                pRootVerb, pSubjectChunkLink, pCurrentChunk, pChunkToSplit, it, pVerbCanBeFollowedBy);
            break;
        }
    }
}

bool EntityRecognizer::pronounCompIsReflexiveOfASubject(const InflectedWord& pSubjectInflWord,
                                                        const InflectedWord& pPronounCompInflWord) const {
    return pSubjectInflWord.word.partOfSpeech == PartOfSpeech::PRONOUN_SUBJECT
        && fConfiguration.getFlsChecker().pronounSetAtSamePers(pSubjectInflWord, pPronounCompInflWord);
}

void EntityRecognizer::xLinkPronounComplementBeforeVerb(Chunk& pRootVerb,
                                                        ChunkLink* pSubjectChunkLink,
                                                        Chunk& pCurrentChunk,
                                                        Chunk& pChunkToSplit,
                                                        TokIt pPronounComplement,
                                                        ChunkLinkType pVerbCanBeFollowedBy) const {
    TokIt sepToken = getNextToken(pPronounComplement, pChunkToSplit.tokRange.getItEnd());

    auto itWhereToInsertSubChunk = pCurrentChunk.children.begin();
    if (!pCurrentChunk.children.empty() && pCurrentChunk.children.begin()->type == ChunkLinkType::SUBJECT) {
        ++itWhereToInsertSubChunk;
    }

    ChunkLinkType linkType = ChunkLinkType::DIRECTOBJECT;
    // TODO: use the underlying lemma to detect reflexive pronouns.
    if (pPronounComplement->str == "s" || pPronounComplement->str == "se" || pPronounComplement->str == "S"
        || pPronounComplement->str == "Se") {
        linkType = ChunkLinkType::REFLEXIVE;
    } else if (pPronounComplement->str == "y" || pPronounComplement->str == "Y") {
        const std::string& verbLemma = pRootVerb.head->inflWords.front().word.lemma;
        if (verbLemma == "avoir" || verbLemma == "être") {
            linkType = ChunkLinkType::REFLEXIVE;
        } else {
            linkType = ChunkLinkType::TOPIC;
        }
    } else if (pPronounComplement->str == "le" || pPronounComplement->str == "la" || pPronounComplement->str == "en"
               || pPronounComplement->str == "les" || pPronounComplement->str == "l") {
        linkType = ChunkLinkType::DIRECTOBJECT;
    } else if (pPronounComplement->str == "lui" || pPronounComplement->str == "leur") {
        linkType = ChunkLinkType::INDIRECTOBJECT;
    } else if (pSubjectChunkLink != nullptr
               && pronounCompIsReflexiveOfASubject(pSubjectChunkLink->chunk->head->inflWords.front(),
                                                   pPronounComplement->inflWords.front())) {
        linkType = ChunkLinkType::REFLEXIVE;
    } else if (pVerbCanBeFollowedBy != ChunkLinkType::DIRECTOBJECT) {
        linkType = pVerbCanBeFollowedBy;
    } else if (haveADO(pRootVerb)) {
        linkType = ChunkLinkType::INDIRECTOBJECT;
    }

    std::list<ChunkLink>::iterator complChunk = pCurrentChunk.children.insert(
        itWhereToInsertSubChunk,
        ChunkLink(linkType,
                  Chunk(TokenRange(pChunkToSplit.tokRange.getTokList(), pChunkToSplit.tokRange.getItBegin(), sepToken),
                        ChunkType::NOMINAL_CHUNK)));
    complChunk->chunk->head = pPronounComplement;
    pChunkToSplit.tokRange.setItBegin(sepToken);
    if (complChunk->chunk->tokRange.getItBegin() != pPronounComplement) {
        xAddComplementsOfVerbFromEnd(pRootVerb,
                                     pSubjectChunkLink,
                                     pCurrentChunk,
                                     *complChunk->chunk,
                                     complChunk->chunk->tokRange.getItBegin(),
                                     pPronounComplement,
                                     pVerbCanBeFollowedBy);
    }
}

void EntityRecognizer::xLinkPronounComplementAfterVerb(const Chunk& pRootVerb,
                                                       const ChunkLink* pSubjectChunkLink,
                                                       Chunk& pCurrentChunk,
                                                       Chunk& pChunkToSplit,
                                                       TokIt pPronounComplement) const {
    ChunkLinkType linkType = [&] {
        if (pSubjectChunkLink != nullptr) {
            const InflectedWord& subjInflWord = pSubjectChunkLink->chunk->head->inflWords.front();
            if ((subjInflWord.word.partOfSpeech == PartOfSpeech::PRONOUN_SUBJECT
                 || subjInflWord.word.partOfSpeech == PartOfSpeech::PRONOUN_COMPLEMENT)
                && fConfiguration.getFlsChecker().pronounSetAtSamePers(subjInflWord,
                                                                       pPronounComplement->inflWords.front()))
                return ChunkLinkType::REFLEXIVE;
        } else if (pPronounComplement->getHeadToken() == &*pCurrentChunk.head) {
            return ChunkLinkType::REFLEXIVE;
        }
        if (getDOComplOrSubjClauseChunk(pRootVerb) == nullptr)
            return ChunkLinkType::DIRECTOBJECT;
        return ChunkLinkType::INDIRECTOBJECT;
    }();

    std::list<ChunkLink>::iterator complChunk = pCurrentChunk.children.insert(
        pCurrentChunk.children.end(),
        ChunkLink(linkType,
                  Chunk(TokenRange(
                            pChunkToSplit.tokRange.getTokList(), pPronounComplement, pChunkToSplit.tokRange.getItEnd()),
                        ChunkType::NOMINAL_CHUNK)));
    complChunk->chunk->head = pPronounComplement;
    pChunkToSplit.tokRange.setItEnd(pPronounComplement);

    auto& subChunk = *complChunk->chunk;
    TokIt afterPronounComplement = getNextToken(pPronounComplement, subChunk.tokRange.getItEnd());
    if (afterPronounComplement != subChunk.tokRange.getItEnd()) {
        auto itEnd = subChunk.tokRange.getItEnd();
        for (TokIt it = getNextToken(pPronounComplement, itEnd); it != itEnd; it = getNextToken(it, itEnd)) {
            if (it->inflWords.front().word.partOfSpeech == PartOfSpeech::ADVERB) {
                std::list<ChunkLink>::iterator advChunk = pCurrentChunk.children.insert(
                    pCurrentChunk.children.end(),
                    ChunkLink(ChunkLinkType::SIMPLE,
                              Chunk(TokenRange(subChunk.tokRange.getTokList(), it, subChunk.tokRange.getItEnd()),
                                    ChunkType::NOMINAL_CHUNK)));
                advChunk->chunk->head = it;
                subChunk.tokRange.setItEnd(it);
                break;
            }
        }
    }
}

}    // End of namespace linguistics
}    // End of namespace onsem
