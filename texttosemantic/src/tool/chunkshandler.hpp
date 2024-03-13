#ifndef ONSEM_TEXTTOSEMANTIC_TOOL_CHUNKSHANDLER_HPP
#define ONSEM_TEXTTOSEMANTIC_TOOL_CHUNKSHANDLER_HPP

#include <list>
#include <map>
#include <functional>
#include <onsem/common/enum/semanticlanguageenum.hpp>
#include <onsem/texttosemantic/type/syntacticgraph.hpp>
#include <onsem/common/enum/wordcontextualinfos.hpp>

namespace onsem {
struct SemanticFloat;
namespace linguistics {
class InflectionsChecker;
class StaticLinguisticDictionary;
struct ChunkLinkIter;

struct ChunkLinkWorkingZone {
public:
    ChunkLinkWorkingZone(std::list<ChunkLink>& pSyntTree,
                         std::list<ChunkLink>::iterator pBegin,
                         std::list<ChunkLink>::iterator pEnd)
        : _syntTreePtr(&pSyntTree)
        , _end(pEnd)
        , _beginAtBeginOfList(pSyntTree.begin() == pBegin)
        , _beforeBegin(_beginAtBeginOfList ? pBegin : --pBegin) {}

    bool operator==(const ChunkLinkWorkingZone& pOther) const {
        return _syntTreePtr == pOther._syntTreePtr && _end == pOther._end
            && _beginAtBeginOfList == pOther._beginAtBeginOfList && _beforeBegin == pOther._beforeBegin;
    }

    std::list<ChunkLink>::iterator begin() const {
        if (_beginAtBeginOfList) {
            return _syntTreePtr->begin();
        }
        std::list<ChunkLink>::iterator res = _beforeBegin;
        return ++res;
    }

    ChunkLinkIter beginListIter() const;

    std::list<ChunkLink>::iterator end() const { return _end; }

    ChunkLinkIter endListIter() const;

    bool empty() const {
        if (_beginAtBeginOfList) {
            return _syntTreePtr->begin() == _end;
        }
        auto res = _beforeBegin;
        return ++res == _end;
    }

    std::size_t size() const {
        std::size_t res = 0;
        for (auto it = begin(); it != _end; ++it) {
            ++res;
        }
        return res;
    }

    std::size_t sizeUntilNextNotNominal() const {
        std::size_t res = 0;
        for (auto it = begin(); it != _end; ++it) {
            if (it->chunk->type != ChunkType::NOMINAL_CHUNK) {
                break;
            }
            ++res;
        }
        return res;
    }

    std::list<ChunkLink>& syntTree() { return *_syntTreePtr; }
    const std::list<ChunkLink>& syntTree() const { return *_syntTreePtr; }
    std::list<ChunkLink>* syntTreePtr() { return _syntTreePtr; }
    const std::list<ChunkLink>* syntTreePtr() const { return _syntTreePtr; }

private:
    std::list<ChunkLink>* _syntTreePtr;    // it cannot be empty (it's not a ref to allow copy of this class)
    std::list<ChunkLink>::iterator _end;
    bool _beginAtBeginOfList;
    std::list<ChunkLink>::iterator _beforeBegin;
};

TokIt goAtBeginOfWordGroup(TokIt pTokenIt, const TokIt& pTokenBeginIt);
TokIt goAtEndOfWordGroup(TokIt pTokenIt, const TokenRange& pTokRangeContext);

TokenRange tokenToTokenRange(TokIt pTokenIt, const TokenRange& pTokRangeContext);

void advanceBeforeLastSeparator(std::list<ChunkLink>::iterator& pPos, const ChunkLinkWorkingZone& pWorkingZone);

void addNominalGroup(std::list<ChunkLink>& pSyntTree, const TokenRange& pTokRange, SemanticLanguageEnum pLangType);

void separateBeginOfAChunk(ChunkLinkIter& pChunkIt,
                           TokIt pBeginNewChunk,
                           ChunkType pChunkType,
                           SemanticLanguageEnum pLanguage);

//! Does the token range contains at least one token that have a specific "part of speech".
bool doesHaveAPos(const TokenRange& pTokRange, PartOfSpeech pPartOfSpeech);
void separateBeginOfAChunk(std::list<ChunkLink>& pSyntTree,
                           std::list<ChunkLink>::iterator pChunkIt,
                           TokIt pBeginNewChunk,
                           ChunkType pChunkType,
                           SemanticLanguageEnum pLanguage);

void separateBeginOfAChunk(std::list<ChunkLink>& pSyntTree,
                           std::list<ChunkLink>::iterator pChunkIt,
                           TokIt pBeginNewChunk,
                           ChunkType pChunkType,
                           TokIt pHeadFirstChunk,
                           SemanticLanguageEnum pLanguage);

void separateEndOfAChunk(ChunkLinkIter& pChunkIt,
                         TokIt pBeginNewChunk,
                         ChunkType pChunkType,
                         SemanticLanguageEnum pLanguage);

void separateEndOfAChunk(std::list<ChunkLink>& pSyntTree,
                         std::list<ChunkLink>::iterator pChunkIt,
                         TokIt pBeginNewChunk,
                         ChunkType pChunkType,
                         SemanticLanguageEnum pLanguage);

void moveEndOfAChunk(Chunk& pChunk,
                     TokIt pBeginNewChunk,
                     ChunkLinkType pChunkLinkType,
                     ChunkType pChunkType,
                     std::list<ChunkLink>& pDestinationList,
                     SemanticLanguageEnum pLanguage);

void putBeginOfAChunkInTheChunkLink(std::list<ChunkLink>& pSyntTree,
                                    std::list<ChunkLink>::iterator pChunkIt,
                                    TokIt pBeginNewChunk,
                                    SemanticLanguageEnum pLanguage);

void putEndOfAChunkToHisChildren(Chunk& pChunk, TokIt pBeginNewChunk, SemanticLanguageEnum pLanguage);

void putPrepositionInCunkLkTokenRange(ChunkLink& pChunkLink, SemanticLanguageEnum pLanguage);

TokIt getHeadOfNominalGroup(const TokenRange& pTokRange, SemanticLanguageEnum pLanguage);

TokIt getEndOfNominalGroup(const TokenRange& pTokRange, TokIt pTokIt);

bool ifContainToken(TokIt pTokenToSearch, const TokenRange& pTokRange);

void setChunkType(Chunk& pChunk);

bool hasAChild(const Chunk& pChunk, const Chunk& pPossibleChild);

bool haveAnAuxiliary(const Chunk& pVerbChunk);

const Chunk* getAuxiliaryChunk(const Chunk& pVerbChunk);

Chunk* getAuxiliaryChunk(Chunk& pVerbChunk);

Chunk* getChunkWithAQuestionWordChild(Chunk& pVerbChunk);

bool haveAQuestionWordChildAfter(const Chunk& pVerbChunk);

void forEachAuxiliaryChunks(const Chunk& pVerbChunk, const std::function<void(const Chunk&)>& pCallback);

std::list<ChunkLink>::iterator getChunkLink(Chunk& pVerbChunk, ChunkLinkType pLinkType);

std::list<ChunkLink>::const_iterator getChunkLink(const Chunk& pVerbChunk, ChunkLinkType pLinkType);

bool haveChild(const Chunk& pChunk, ChunkLinkType pLinkType);

bool haveChildWithAuxSkip(const Chunk& pVerbChunk, const std::set<ChunkLinkType>& pChildChunkLinks);

const Chunk* getChildChunkPtr(const Chunk& pChunk, ChunkLinkType pLinkType);

Chunk* getChildChunkPtr(Chunk& pChunk, ChunkLinkType pLinkType);

bool haveAQuestionInDirectObject(const Chunk& pChunk);

const Chunk& getFirstListEltChunk(const Chunk& pChunk);

bool haveASubject(const Chunk& pVerbChunk);
bool haveASubjectExceptQuestionWord(const Chunk& pVerbChunk);

bool haveAChildBefore(const Chunk& pChunk);

bool conditionIsTheFirstChildAndThereIsManyChildren(const Chunk& pVerbChunk);

bool doesChunkHaveDeterminerBeforeHead(const Chunk& pChunk);

bool haveADO(const Chunk& pVerbChunk);

bool haveASubordonateClause(const Chunk& pVerbChunk);

bool haveAChildWithChunkLink(const Chunk& pVerbChunk, ChunkLinkType pChildChunkLink);

Chunk& whereToLinkSubject(Chunk& pVerbChunk);

bool haveOtherEltsBetterToLinkInAList(ChunkLinkIter& pBeforeLastElt, ChunkLinkIter& pLastElt);

const ChunkLink* getChunkLinkWithAuxSkip(const Chunk& pVerbChunk, ChunkLinkType pChildChunkLink);

ChunkLink* getSubjectChunkLink(Chunk& pVerbChunk);

ChunkLinkIter getSubjectChunkLkIterator(Chunk& pVerbChunk);

Chunk* getSubjectChunk(const Chunk& pVerbChunk);

ChunkLink* getChunkLinkWhereWeCanLinkASubodinate(Chunk& pVerbChunk);
ChunkLink* getSubjectChunkLinkWhereWeCanLinkASubodinate(Chunk& pVerbChunk);

ChunkLink* getDOChunkLink(Chunk& pVerbChunk);

const ChunkLink* getDOChunkLink(const Chunk& pVerbChunk);

Chunk* getlatestVerbChunk(Chunk& pChunk);

Chunk* getDOComplOrSubjClauseChunk(const Chunk& pVerbChunk);
Chunk* getNoneVerbalDOComplOrSubjClauseChunk(Chunk& pVerbChunk);

Chunk* getReflexiveChunk(const Chunk& pVerbChunk);

Chunk* whereToLinkTheSubjectPtr(Chunk* pVerbChunk, const InflectionsChecker& pFls);
Chunk& whereToLinkTheSubject(Chunk& pVerbChunk);
const Chunk& whereToLinkTheSubject(const Chunk& pVerbChunk);

bool isAPrepositionnalChunkLink(const ChunkLink& pChunkLink);

void setSubjectPronounInflectionsAccordingToTheVerb(InflectedWord& pSubjectPronounInfls,
                                                    const InflectedWord& pVerbInfls,
                                                    const InflectionsChecker& pFls);

bool canLinkVerbToASubject(const Chunk& pVerbChunk,
                           const Chunk& pSubjectChunk,
                           const InflectionsChecker& pFls,
                           bool pCanBeAtPastOrPresentParticiple);

int nbOfGNs(const ChunkLinkWorkingZone& pWorkingZone);

bool hasAChunkTypeOrAListOfChunkTypes(const Chunk& pChunk, ChunkType pChunkType);

bool isACommandOrAListOfCommands(const Chunk& pChunk);
bool doesChunkIsOrCanBeAtImperative(const Chunk& pChunk);
bool chunkIsVerbal(const Chunk& pChunk);
bool chunkIsVerbalAndNotAYesOrNo(const Chunk& pChunk);
bool chunkIsNominal(const Chunk& pChunk);
bool chunkIsAtInfinitive(const Chunk& pChunk);
bool chunkDontHaveASubjectAndCannotHaveNoSubject(const Chunk& pChunk);
bool chunkCanBeAtImperative(const Chunk& pChunk);
bool chunkIsAtPresentParticiple(const Chunk& pChunk);
bool chunkIsAtPastParticiple(const Chunk& pChunk);

bool recInListConst(std::function<bool(const Chunk& pChunk)> pPredicate, const Chunk& pChunk);
void recInList(std::function<void(Chunk& pChunk)> pPredicate, Chunk& pChunk);

bool chunkCanHaveSubordinates(const Chunk& pChunk);

bool chunkCanBeAnObject(const Chunk& pChunk);

bool canBeTheHeadOfASubordinate(const InflectedWord& pInflWord);

PartOfSpeech getFirstPartOfSpeechOfAChunk(const Chunk& pChunk);

bool doesBeginWithAnIndefiniteDeterminer(const TokenRange& pTokRange);

bool doesTokRangeBeginsWithARequestSubordinate(const TokenRange& pTokRange, const StaticLinguisticDictionary& pBinDico);

bool isChunkAtPassiveForm(const Chunk& pVerbChunk,
                          const LinguisticDictionary& pLingDico,
                          const InflectionsChecker& pInflChecker);

bool checkOrder(const Chunk& pChunk1, const Chunk& pChunk2);

void setChunkAtInterrogativeForm(Chunk& pChunk);

bool isTokenAtEndOfASentence(TokIt pTokenIt, const TokenRange& pTokRangeContext);

void addChildInGoodOrder(std::list<ChunkLink>& pChildren,
                         std::list<ChunkLink>& pListOfTheChildToMove,
                         std::list<ChunkLink>::iterator pItChildToMove);

void linkPartitives(Chunk& rootChunk,
                    ChunkLinkIter& pItToPotentialPartitiveChunk,
                    const EntityRecognizer& pEntityRecognizer);

bool filterIncompatibleInflectionsInTokenList(std::vector<Token>& pTokens, const InflectionsChecker& pFls);

void moveChildren(std::list<ChunkLink>& pRootChildren, ChunkLinkIter& pIteratorToMove, ChunkLinkType pNewChkLkType);

bool getNumberBeforeHead(SemanticFloat& pNumber, const Chunk& pChunk);

}    // End of namespace linguistics
}    // End of namespace onsem

#endif    // ONSEM_TEXTTOSEMANTIC_TOOL_CHUNKSHANDLER_HPP
