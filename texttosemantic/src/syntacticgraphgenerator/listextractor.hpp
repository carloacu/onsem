#ifndef ONSEM_TEXTTOSEMANTIC_SRC_SYNTACTICGRAPHGENERATOR_LISTEXTRACTOR_HPP
#define ONSEM_TEXTTOSEMANTIC_SRC_SYNTACTICGRAPHGENERATOR_LISTEXTRACTOR_HPP

#include <string>
#include <vector>
#include <list>
#include <onsem/common/utility/optional.hpp>
#include <onsem/common/enum/semanticrequesttype.hpp>
#include <onsem/texttosemantic/dbtype/inflection/verbalinflections.hpp>
#include <onsem/texttosemantic/type/enum/chunktype.hpp>
#include "../tool/listiter.hpp"

namespace onsem {
namespace linguistics {
class InflectionsChecker;
class AlgorithmSetForALanguage;
struct ChunkLinkWorkingZone;
struct Chunk;
struct ChunkLink;
struct InflectedWord;

mystd::optional<ChunkType> getListType(const std::map<std::string, char>& pConcepts);

class ListExtractor {
public:
    explicit ListExtractor(const AlgorithmSetForALanguage& pConfiguration);

    bool extractLists(ChunkLinkWorkingZone& pWorkingZone,
                      bool pOneWordList = false,
                      bool pAllowToNotRepeatTheSubject = false,
                      bool pCanCompletePreviousList = false,
                      int pNbMaxOfListElt = -1,
                      const mystd::optional<ChunkType>& pExceptListType = mystd::optional<ChunkType>()) const;

    void extractSubordonates(ChunkLinkWorkingZone& pWorkingZone) const;

private:
    struct ListInfo {
        ListInfo() = default;

        ListInfo(const ListInfo&) = delete;
        ListInfo& operator=(const ListInfo&) = delete;

        bool hasAConceptInCommon(const std::map<std::string, char>& pCpts) const {
            for (const auto& currCpt : concepts)
                if (pCpts.count(currCpt) > 0)
                    return true;
            return false;
        }

        mystd::optional<ChunkType> listType{};
        Chunk* listChunk = nullptr;
        ChunkType eltType = ChunkType::NOMINAL_CHUNK;
        // if the current list have determiner before the elements
        PartOfSpeech partOfSpeechAtBegining = PartOfSpeech::UNKNOWN;
        bool hasASubject = false;
        std::set<std::string> concepts{};
        VerbalInflections commonVerbInflections{};
        SemanticRequests requestsOfElts;
        mystd::optional<ChunkLinkIter> chunkToMoveInNextEltChunkLink{};
        PartOfSpeech firstHeadPartOfSpeech = PartOfSpeech::UNKNOWN;
    };
    struct ListToMove {
        ListToMove(const ChunkLinkIter& pNewParent, const ChunkLinkIter& pItRootList, bool pRemoveIfAnotherChunkFound)
            : newParent(pNewParent)
            , itRootList(pItRootList)
            , removeIfAnotherChunkFound(pRemoveIfAnotherChunkFound) {}

        ChunkLinkIter newParent;
        ChunkLinkIter itRootList;
        bool removeIfAnotherChunkFound;
    };
    const InflectionsChecker& fFlschecker;
    const AlgorithmSetForALanguage& fConf;

    bool _createNewList(std::list<ListToMove>& pListsToMove,
                        ChunkAndTokIt& pChunkEnd,
                        ChunkAndTokIt& pChunkBegin,
                        int& pNbMaxOfListElt,
                        ListInfo& pCurrList,
                        ChunkLinkIter& pPrevIt,
                        ChunkLinkIter& pIt,
                        ChunkLinkIter& pNextIt) const;

    bool _addListElt(Chunk* pListChunk, ChunkLinkIter& pNextIt, int& pNbMaxOfListElt, TokIt pNextChunkBegin) const;

    bool _isNewEltCompatibleWithTheList(ListInfo& pCurrList,
                                        const Chunk& pChunk,
                                        bool pAllowToNotRepeatTheSubject) const;

    void _getVerbAndAuxInflections(VerbalInflections& pVerbFlexions, const Chunk& pVerbChunk) const;

    bool _getBeginNewEltAndEndOldElt(ChunkAndTokIt& pChunkBegin,
                                     ChunkAndTokIt& pChunkEnd,
                                     PartOfSpeech pPartOfSpeechAtBegining) const;

    bool _tryAssociateNewChunkByTruncateTheEnd(TokIt& pLastTokNewChunk,
                                               Chunk& pNewChunk,
                                               const InflectedWord& pIGramLastElt) const;

    static bool _itIsInListToMove(const ChunkLinkIter& pIt, const std::list<ListToMove>& pListsToMove);

    bool _addANewMultiWordsList(ListInfo& pCurrList,
                                std::list<ListToMove>& pListsToMove,
                                int& pNbMaxOfListElt,
                                ChunkLinkIter& pPrevIt,
                                ChunkLinkIter& pIt,
                                ChunkLinkIter& pNextIt,
                                bool pAllowToNotRepeatTheSubject) const;

    bool _addANewOneWordList(ListInfo& pCurrList,
                             std::list<ListToMove>& pListsToMove,
                             int& pNbMaxOfListElt,
                             ChunkLinkIter& pPrevIt,
                             ChunkLinkIter& pIt,
                             ChunkLinkIter& pNextIt) const;

    static void _addAListToMove(std::list<ListToMove>& pListsToMove, const ListToMove& pNewListToMove);

    // Subordonates extractors
    // ---------------------------------------------------------------------

    struct SubordinateWithSeparators {
        SubordinateWithSeparators(std::list<ChunkLink>::iterator pSub)
            : sub(pSub)
            , separators() {}

        SubordinateWithSeparators(std::list<ChunkLink>::iterator pSub, std::list<ChunkLink>::iterator pSep)
            : sub(pSub)
            , separators() {
            separators.emplace_back(pSep);
        }

        std::list<ChunkLink>::iterator sub;
        std::list<std::list<ChunkLink>::iterator> separators;
    };

    struct ListOfSubordinates {
        void addSub(std::list<ChunkLink>::iterator pItChkLk) {
            subWithSeps.emplace_front(SubordinateWithSeparators(pItChkLk));
        }

        void addSub(std::list<ChunkLink>::iterator pItChkLk, std::list<ChunkLink>::iterator pSep) {
            subWithSeps.emplace_front(SubordinateWithSeparators(pItChkLk, pSep));
        }

        bool endOfList = true;
        std::list<SubordinateWithSeparators> subWithSeps{};
    };

    bool _addSubordonates(std::list<ChunkLink>::iterator& pParentChunkLk,
                          std::list<SubordinateWithSeparators>& pSubOfChunk,
                          std::list<ChunkLink>::iterator& pEndEltOfList,
                          mystd::optional<ChunkType> pListType) const;

    static void _clear(ChunkLinkWorkingZone& pWorkingZone, const std::list<SubordinateWithSeparators>& pSubOfChunk);

    mystd::optional<ChunkLinkType> _getAppropriateChunkLinkFromTokens(
        InflectedWord* pVerbInflectedWord,
        mystd::optional<const SemanticWord*>& pIntroductingWord,
        const TokenRange& pTokRange,
        ChunkType pTokensChunkType) const;

    bool _linkVerbChunk_VerbChunk(Chunk& pFirstVerb, ChunkLink& pSecondVerb, const TokenRange& pTokRange) const;

    bool _linkChunk_VerbChunk(Chunk& pFirstChunk, ChunkLink& pSecondVerb, const TokenRange& pTokRange) const;

    void _refactorANewSubordinate(Chunk& pFirstVerb, Chunk& pSecondVerb, const TokenRange& pTokRange) const;

    bool _canLinkDO_Subject(Chunk& pFirstVerb, Chunk& pSecondVerb) const;

    // complete an existing list
    // =========================

    std::list<ChunkLink>::iterator _tryToCompleteAnExistingList(std::list<ListToMove>& pListsToMove,
                                                                ChunkLinkWorkingZone& pWorkingZone,
                                                                std::list<ChunkLink>::iterator pItChunkLink) const;

    void _initNewListInfo(ListInfo& pCurrList, const Chunk& pFirstListEltChunk) const;

    bool _updateExistingListInfoAccordingToTheNewElt(ListInfo& pCurrList,
                                                     const Chunk& pNewListEltChunk,
                                                     bool pDoesBecomeAListOfInfinitiveVerbs) const;

    bool _doesBecomeAListOfInfinitiveVerbs(const ListInfo& pCurrList, const ChunkType& pNewChunkType) const;

    bool _tryToCompleteListInfoWithAChunk(ChunkLinkIter& pCurrIt,
                                          ListInfo& pCurrList,
                                          std::list<ListToMove>& pListsToMove,
                                          int& pNbMaxOfListElt,
                                          const ChunkLinkWorkingZone& pWorkingZone,
                                          bool pOneWordList,
                                          bool pAllowToNotRepeatTheSubject) const;

    static void _moveSomeChunksAfterMAinAlgo(std::list<ListToMove>& pListsToMove);

    bool _tryToExtractANewListAroundASeparator(ChunkLinkIter& pCurrIt,
                                               ListInfo& pCurrList,
                                               std::list<ListToMove>& pListsToMove,
                                               bool& pCanHaveAnotherList,
                                               int& pNbMaxOfListElt,
                                               const ChunkLinkWorkingZone& pWorkingZone,
                                               bool pOneWordList,
                                               bool pAllowToNotRepeatTheSubject,
                                               const mystd::optional<ChunkType>& pExceptListType) const;
};

}    // End of namespace linguistics
}    // End of namespace onsem

#endif    // ONSEM_TEXTTOSEMANTIC_SRC_SYNTACTICGRAPHGENERATOR_LISTEXTRACTOR_HPP
