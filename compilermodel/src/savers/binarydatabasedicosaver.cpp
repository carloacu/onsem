#include <onsem/compilermodel/savers/binarydatabasedicosaver.hpp>
#include <climits>
#include <iomanip>
#include <fstream>
#include <onsem/common/enum/semanticlanguageenum.hpp>
#include <onsem/compilermodel/savers/binarydatabaseconceptssaver.hpp>
#include <onsem/compilermodel/lingdbmeaning.hpp>
#include <onsem/compilermodel/lingdbmeaningtowords.hpp>
#include <onsem/compilermodel/lingdbdynamictrienode.hpp>
#include <onsem/compilermodel/lingdbtypes.hpp>
#include <onsem/compilermodel/lingdbflexions.hpp>
#include <onsem/compilermodel/lingdbwordforms.hpp>
#include <onsem/compilermodel/linguisticintermediarydatabase.hpp>
#include <onsem/compilermodel/lingdbstring.hpp>
#include "../concept/lingdbconcept.hpp"
#include "../concept/lingdblinktoaconcept.hpp"

namespace onsem {

void BinaryDatabaseDicoSaver::save(std::map<const LingdbMeaning*, int>& pMeaningsPtr,
                                   const std::map<std::string, ConceptsBinMem>& pConceptsOffsets,
                                   const std::filesystem::path& pFilenameDatabase,
                                   const std::filesystem::path& pFilenameSynthesizerDatabase,
                                   const LinguisticIntermediaryDatabase& pLingDatabase) const {
    std::string languageStr = pLingDatabase.getLanguage()->toStr();
    SemanticLanguageEnum langType = semanticLanguageTypeGroundingEnumFromStr(languageStr);

    std::ofstream debufInfosFile(std::filesystem::path(languageStr + "_db.txt"));

    const CompositePoolAllocator& alloc = pLingDatabase.getFPAlloc();
    std::size_t maxSize = alloc.getOccupatedSize() + (4 * pConceptsOffsets.size());
    binarymasks::Ptr mem = ::operator new(maxSize);

    assert(mem.val == binarysaver::alignMemory(mem).val);

    std::map<const LingdbConcept*, std::set<MeaningAndConfidence> > conceptToMeanings;
    binarymasks::Ptr beginMeanings = mem;
    binarymasks::Ptr endMemory = xAddMeanings(pMeaningsPtr, conceptToMeanings, pConceptsOffsets, alloc, mem, mem);
    std::size_t meaningSize = endMemory.val - beginMeanings.val;
    debufInfosFile << "meanings: (size: ";
    xWriteNbWithSpaces(debufInfosFile, meaningSize);
    debufInfosFile << ", limit: ";
    xWriteNbWithSpaces(debufInfosFile, 0x00FFFFFF * 4);
    debufInfosFile << ")" << std::endl;

    std::map<std::string, unsigned char> flexionsPtr;
    binarymasks::Ptr beginFlexions = endMemory;
    endMemory = xAddSomeFlexions(flexionsPtr, alloc, endMemory, endMemory);
    std::size_t flexionsSize = endMemory.val - beginFlexions.val;
    debufInfosFile << "flexions: (size: ";
    xWriteNbWithSpaces(debufInfosFile, flexionsSize);
    debufInfosFile << ", approxLimitToStoreInPatriciaTrie: ";
    xWriteNbWithSpaces(debufInfosFile, 0x0000007F * 4);
    debufInfosFile << ")" << std::endl;

    std::map<LingdbDynamicTrieNode const*, int> nodesPtr;
    binarymasks::Ptr beginTrie = endMemory;
    TreeCreationWorkState treeCrWorkState(nodesPtr, beginMeanings, pMeaningsPtr, flexionsPtr);
    endMemory = xCreateRootNode(treeCrWorkState, alloc.first<LingdbDynamicTrieNode>(), endMemory, endMemory);
    std::size_t patriciaTrieSize = endMemory.val - beginTrie.val;
    debufInfosFile << "patricia trie: (size: ";
    xWriteNbWithSpaces(debufInfosFile, patriciaTrieSize);
    debufInfosFile << ", limit: ";
    xWriteNbWithSpaces(debufInfosFile, 0x00FFFFFF * 4);
    debufInfosFile << ")" << std::endl;

    std::size_t sizeMemory = endMemory.val - mem.val;
    xWriteLemmeOfMeanings(beginMeanings, pMeaningsPtr, nodesPtr);

    // write synthetizer database
    if (!pFilenameSynthesizerDatabase.string().empty()) {
        xWriteSynthesizerDb(maxSize,
                            beginMeanings,
                            conceptToMeanings,
                            pConceptsOffsets,
                            pMeaningsPtr,
                            nodesPtr,
                            pFilenameSynthesizerDatabase,
                            pLingDatabase,
                            langType);
    }

    std::ofstream outfile(pFilenameDatabase, std::ofstream::binary);
    outfile.write(reinterpret_cast<const char*>(&fFormalism), sizeof(fFormalism));

    // write if the version of the database
    {
        unsigned int version = pLingDatabase.getVersion();
        outfile.write(reinterpret_cast<const char*>(&version), sizeof(unsigned int));
    }

    // write meanings
    binarysaver::writePtr(outfile, meaningSize);
    // write some flexions
    binarysaver::writePtr(outfile, flexionsSize);
    // write patricia trie
    binarysaver::writePtr(outfile, patriciaTrieSize);

    // write if the language has separators between each words
    {
        bool separatorNeeded = pLingDatabase.isSeparatorNeeded();
        outfile.write(reinterpret_cast<const char*>(&separatorNeeded), sizeof(bool));
    }

    // Write all the memory
    outfile.write(reinterpret_cast<const char*>(mem.ptr), sizeMemory);

    binarymasks::Ptr beginRules = endMemory;

    // Write the question words
    // ------------------------
    endMemory.pchar =
        xWriteRuleWithMeanings(endMemory.pchar, pMeaningsPtr, pLingDatabase, pLingDatabase.getQuestionWords(), 1);

    // fash the rules memory in the file
    // ---------------------------------
    std::size_t sizeRules = endMemory.val - beginRules.val;
    int printSizeRules = static_cast<int>(sizeRules);
    outfile.write(reinterpret_cast<const char*>(&printSizeRules), sizeof(int));
    outfile.write(reinterpret_cast<const char*>(beginRules.ptr), sizeRules);

    outfile.close();

    ::operator delete(mem.ptr);
    debufInfosFile.close();
}

void BinaryDatabaseDicoSaver::xWriteLemmeOfMeanings(
    binarymasks::Ptr pBeginMeaning,
    const std::map<const LingdbMeaning*, int>& pMeaningsPtr,
    const std::map<LingdbDynamicTrieNode const*, int>& pNodesPtr) const {
    for (const auto& currMeaning : pMeaningsPtr) {
        LingdbDynamicTrieNode* lemmaNode = xGetMeaningLemma(*currMeaning.first);
        auto* meaningPtr = pBeginMeaning.pchar + currMeaning.second * 4;
        std::map<LingdbDynamicTrieNode const*, int>::const_iterator itNode = pNodesPtr.find(lemmaNode);
        if (itNode != pNodesPtr.end()) {
            binarysaver::writeInThreeBytes(xGetLemme(meaningPtr), itNode->second);
        }
    }
}

LingdbDynamicTrieNode* BinaryDatabaseDicoSaver::xGetMeaningLemma(const LingdbMeaning& pMeaning) const {
    LingdbDynamicTrieNode* lemmaNode = pMeaning.getLemma();
    if (lemmaNode->getDatasIfItsAMultiMeaningNode() != nullptr) {
        return lemmaNode->getFather();
    }
    return lemmaNode;
}

void BinaryDatabaseDicoSaver::xWriteSynthesizerDb(
    std::size_t pMaxSize,
    binarymasks::Ptr pBeginMeaning,
    const std::map<const LingdbConcept*, std::set<MeaningAndConfidence> >& pConceptToMeanings,
    const std::map<std::string, ConceptsBinMem>& pConceptsOffsets,
    const std::map<const LingdbMeaning*, int>& pMeaningsPtr,
    const std::map<LingdbDynamicTrieNode const*, int>& pNodesPtr,
    const std::filesystem::path& pFilename,
    const LinguisticIntermediaryDatabase& pLingDatabase,
    SemanticLanguageEnum pLangType) const {
    binarymasks::Ptr mem = ::operator new(pMaxSize);

    LingdbMeaningToWords meaningToWords;
    std::map<const LingdbMeaning*, VerbConjugaison> verbConjugaison;
    std::map<const LingdbMeaning*, NounAdjConjugaison> nounConjugaison;
    meaningToWords.findWordsConjugaisons(verbConjugaison, nounConjugaison, pLingDatabase);

    binarymasks::Ptr endMemory = mem;
    binarysaver::writeInt(endMemory.pchar, 0);
    endMemory.val += 4;

    std::map<const LingdbMeaning*, int> meaningToConj;
    for (const auto& currMeaning : pMeaningsPtr) {
        // write the conjugaisons
        if (currMeaning.first->getLemma()->getDatasIfItsAMultiMeaningNode() == nullptr) {
            meaningToConj[currMeaning.first] = binarysaver::alignedDecToSave(endMemory.val - mem.val);

            PartOfSpeech currPartOfSpeech = currMeaning.first->getPartOfSpeech();

            if (currPartOfSpeech == PartOfSpeech::VERB || currPartOfSpeech == PartOfSpeech::AUX) {
                auto itConj = verbConjugaison.find(currMeaning.first);
                assert(itConj != verbConjugaison.end());
                VerbConjugaison& conj = itConj->second;
                endMemory = xWriteVerbConj(endMemory, pNodesPtr, conj.imperfect);              // 0        (+6)
                endMemory = xWriteVerbConj(endMemory, pNodesPtr, conj.present);                // 6        (+6)
                endMemory = xWriteVerbConj(endMemory, pNodesPtr, conj.future);                 // 12        (+6)
                endMemory = xWriteVerbConj(endMemory, pNodesPtr, conj.imperative);             // 18        (+3)
                endMemory = xWriteNodeRef(endMemory, pNodesPtr, conj.presentParticiple.node);  // 21
                if (pLangType == SemanticLanguageEnum::FRENCH) {
                    endMemory =
                        xWriteNodeRef(endMemory, pNodesPtr, conj.pastParticiple.masculineSingular.node);          // 22
                    endMemory = xWriteNodeRef(endMemory, pNodesPtr, conj.pastParticiple.masculinePlural.node);    // 23
                    endMemory = xWriteNodeRef(endMemory, pNodesPtr, conj.pastParticiple.feminineSingular.node);   // 24
                    endMemory = xWriteNodeRef(endMemory, pNodesPtr, conj.pastParticiple.femininePlural.node);     // 25
                    endMemory = xWriteVerbConj(endMemory, pNodesPtr, conj.conditional);                           // 26   (+6)
                    endMemory = xWriteVerbConj(endMemory, pNodesPtr, conj.presentSubjunctive);                    // 32   (+6)
                } else {
                    endMemory =
                        xWriteNodeRef(endMemory, pNodesPtr, conj.pastParticiple.masculineSingular.node);    // 22
                }
            }
            // TODO: reduce the space by having a specific storage for proper nouns
            else if (partOfSpeech_isNominal(currPartOfSpeech) || currPartOfSpeech == PartOfSpeech::ADJECTIVE) {
                auto itConj = nounConjugaison.find(currMeaning.first);
                assert(itConj != nounConjugaison.end());
                NounAdjConjugaison& conj = itConj->second;
                endMemory = xWriteNodeRef(endMemory, pNodesPtr, conj.masculineSingular.node);
                endMemory = xWriteNodeRef(endMemory, pNodesPtr, conj.masculinePlural.node);
                endMemory = xWriteNodeRef(endMemory, pNodesPtr, conj.feminineSingular.node);
                endMemory = xWriteNodeRef(endMemory, pNodesPtr, conj.femininePlural.node);
                endMemory = xWriteNodeRef(endMemory, pNodesPtr, conj.neutralSingular.node);
                endMemory = xWriteNodeRef(endMemory, pNodesPtr, conj.neutralPlural.node);
                if (pLangType == SemanticLanguageEnum::ENGLISH && currPartOfSpeech == PartOfSpeech::ADJECTIVE) {
                    endMemory = xWriteNodeRef(endMemory, pNodesPtr, conj.comparative.node);
                    endMemory = xWriteNodeRef(endMemory, pNodesPtr, conj.superlative.node);
                }
            }
        }
    }

    for (const auto& currMeaningPtr : pMeaningsPtr) {
        LingdbMultiMeaningsNode* multiNodeInfos = currMeaningPtr.first->getLemma()->getDatasIfItsAMultiMeaningNode();
        const LingdbMeaning* rootMeaning =
            multiNodeInfos == nullptr ? currMeaningPtr.first : multiNodeInfos->getRootMeaning();
        auto itMmeaningToConj = meaningToConj.find(rootMeaning);
        assert(itMmeaningToConj != meaningToConj.end());

        // write the synthesizer offset in the meaning struct of the dico database
        int* meaningPtr = pBeginMeaning.pint + currMeaningPtr.second;
        *meaningPtr |= itMmeaningToConj->second;
    }

    std::ofstream outfile(pFilename, std::ofstream::binary);
    outfile.write(reinterpret_cast<const char*>(&fFormalism), sizeof(fFormalism));

    // write the size of conjugaisons
    binarysaver::writePtr(outfile, endMemory.val - mem.val);

    // write the concepts to meanings ids
    binarymasks::Ptr begConceptToMeanings = endMemory;
    binarymasks::Ptr beginMeaningsFromConcepts;
    endMemory = xAddConceptsToMeanings(
        beginMeaningsFromConcepts, pConceptToMeanings, pConceptsOffsets, pMeaningsPtr, endMemory);

    // write the size of concepts to meanings ids
    binarysaver::writePtr(outfile, beginMeaningsFromConcepts.val - begConceptToMeanings.val);

    // write the size of meanings from concepts
    binarysaver::writePtr(outfile, endMemory.val - beginMeaningsFromConcepts.val);

    // Write all the memory
    outfile.write(reinterpret_cast<const char*>(mem.ptr), endMemory.val - mem.val);
    outfile.close();

    ::operator delete(mem.ptr);
}

binarymasks::Ptr BinaryDatabaseDicoSaver::xAddConceptsToMeanings(
    binarymasks::Ptr& pBeginMeaningsFromConcepts,
    const std::map<const LingdbConcept*, std::set<MeaningAndConfidence> >& pConceptToMeanings,
    const std::map<std::string, ConceptsBinMem>& pConceptsOffsets,
    const std::map<const LingdbMeaning*, int>& pMeaningsPtr,
    binarymasks::Ptr pEndMemory) const {
    binarymasks::Ptr begCptToMns = pEndMemory;
    std::size_t nbConcepts = pConceptsOffsets.size();
    for (std::size_t i = 0; i < nbConcepts + 1; ++i) {
        binarysaver::writeInt(pEndMemory.pchar, 0);
        pEndMemory.val += 4;
    }

    // write the meanings for each concepts
    pBeginMeaningsFromConcepts = pEndMemory;
    binarysaver::writeInt(pEndMemory.pchar, 0);
    pEndMemory.val += 4;
    for (const auto& currCptToMn : pConceptToMeanings) {
        const LingdbConcept* currCptPtr = currCptToMn.first;
        pEndMemory = xWriteMeanings(*currCptPtr,
                                    currCptToMn.second,
                                    pConceptsOffsets,
                                    pMeaningsPtr,
                                    begCptToMns,
                                    pBeginMeaningsFromConcepts,
                                    pEndMemory);
    }
    return pEndMemory;
}

binarymasks::Ptr BinaryDatabaseDicoSaver::xWriteMeanings(const LingdbConcept& pConcept,
                                                         const std::set<MeaningAndConfidence>& pMeaningsAndConf,
                                                         const std::map<std::string, ConceptsBinMem>& pConceptsOffsets,
                                                         const std::map<const LingdbMeaning*, int>& pMeaningsPtr,
                                                         binarymasks::Ptr pBegCptToMns,
                                                         binarymasks::Ptr pBeginMeaningsFromConcepts,
                                                         binarymasks::Ptr pEndMemory) const {
    auto itCptOff = pConceptsOffsets.find(pConcept.getName()->toStr());
    assert(itCptOff != pConceptsOffsets.end());
    assert(itCptOff->second.id <= pConceptsOffsets.size());
    binarymasks::Ptr prevPtr = pBegCptToMns;
    prevPtr.val += sizeof(int) * itCptOff->second.id;
    binarysaver::writeInThreeBytes(prevPtr.pchar,
                                   binarysaver::alignedDecToSave(pEndMemory.val - pBeginMeaningsFromConcepts.val));

    binarysaver::writeChar(pEndMemory.pchar++, static_cast<char>(pMeaningsAndConf.size()));
    binarysaver::writeChar(pEndMemory.pchar++, 0);
    binarysaver::writeChar(pEndMemory.pchar++, 0);
    binarysaver::writeChar(pEndMemory.pchar++, 0);

    for (const auto& currMeaning : pMeaningsAndConf) {
        auto itMnsPtr = pMeaningsPtr.find(currMeaning.meaning);
        assert(itMnsPtr != pMeaningsPtr.end());

        binarysaver::writeInThreeBytes(pEndMemory.pchar, itMnsPtr->second);
        pEndMemory.val += 3;
        binarysaver::writeChar(pEndMemory.pchar++, currMeaning.confidence);
    }
    return pEndMemory;
}

binarymasks::Ptr BinaryDatabaseDicoSaver::xWriteVerbConj(binarymasks::Ptr pEndMemory,
                                                         const std::map<LingdbDynamicTrieNode const*, int>& pNodesPtr,
                                                         const std::vector<WordLinkForConj>& pTense) const {
    for (std::size_t i = 0; i < pTense.size(); ++i) {
        pEndMemory = xWriteNodeRef(pEndMemory, pNodesPtr, pTense[i].node);
    }
    return pEndMemory;
}

binarymasks::Ptr BinaryDatabaseDicoSaver::xWriteNodeRef(binarymasks::Ptr pEndMemory,
                                                        const std::map<LingdbDynamicTrieNode const*, int>& pNodesPtr,
                                                        LingdbDynamicTrieNode const* pNode) const {
    if (pNode == nullptr) {
        binarysaver::writeInt(pEndMemory.pchar, 0);
    } else {
        std::map<LingdbDynamicTrieNode const*, int>::const_iterator itNode = pNodesPtr.find(pNode);
        if (itNode == pNodesPtr.end()) {
            binarysaver::writeInt(pEndMemory.pchar, 0);
        } else {
            binarysaver::writeInt(pEndMemory.pchar, itNode->second);
        }
    }
    pEndMemory.val += 4;
    return pEndMemory;
}

binarymasks::Ptr BinaryDatabaseDicoSaver::xAddMeanings(
    std::map<const LingdbMeaning*, int>& pMeaningsPtr,
    std::map<const LingdbConcept*, std::set<MeaningAndConfidence> >& pConceptToMeanings,
    const std::map<std::string, ConceptsBinMem>& pConceptsOffsets,
    const CompositePoolAllocator& pFPAlloc,
    const binarymasks::Ptr pBeginMemory,
    binarymasks::Ptr pEndMemory) const {
    std::map<LingdbMeaning*, std::list<binarymasks::Ptr> > meaningsPointersToWrite;

    // Be sure that there is no meaning at offset 0
    // (because offset 0 means no meaning)
    binarysaver::writeInt(pEndMemory.pchar, 0);
    pEndMemory.val += 4;

    LingdbMeaning* meaning = pFPAlloc.first<LingdbMeaning>();
    while (meaning != nullptr) {
        int meaningAlignedOffest = binarysaver::alignedDecToSave(pEndMemory.val - pBeginMemory.val);
        pMeaningsPtr[meaning] = meaningAlignedOffest;

        // we will save a pointer to the meaning of "synthesizer"
        binarysaver::writeInThreeBytes(pEndMemory.pchar, 0);
        pEndMemory.val += 3;

        // Grammatical type
        char partOfSpeechChar = static_cast<char>(meaning->getPartOfSpeech());
        assert((partOfSpeechChar & 0x80) == 0);    // not signed
        binarysaver::writeChar(pEndMemory.pchar++, partOfSpeechChar);

        // we will save a pointer lemme node
        binarysaver::writeInThreeBytes(pEndMemory.pchar, 0);
        pEndMemory.val += 3;

        LingdbDynamicTrieNode* lemmaNode = meaning->getLemma();

        // write number of gathering meanings
        std::list<LingdbDynamicTrieNode*> gatheringMeanings;
        const ForwardPtrList<LingdbDynamicTrieNode>* multiMeaningsNodes = lemmaNode->getMultiMeaningsNodes();
        while (multiMeaningsNodes != nullptr) {
            LingdbMeaning* rootMeaning = multiMeaningsNodes->elt->getDatasIfItsAMultiMeaningNode()->getRootMeaning();
            if (rootMeaning == meaning) {
                gatheringMeanings.push_back(multiMeaningsNodes->elt);
            }
            multiMeaningsNodes = multiMeaningsNodes->next;
        }
        unsigned char gatheringMeanings_size = static_cast<unsigned char>(gatheringMeanings.size());
        assert((gatheringMeanings_size & 0x80) == 0);    // not signed
        binarysaver::writeChar(pEndMemory.pchar++, gatheringMeanings_size);

        // write number of linked meanings
        const ForwardPtrList<LingdbNodeLinkedMeaning>* linkedMeanings = nullptr;
        {
            LingdbMultiMeaningsNode* gatherMeaningInfos = lemmaNode->getDatasIfItsAMultiMeaningNode();
            if (gatherMeaningInfos == nullptr) {
                binarysaver::writeChar(pEndMemory.pchar++, 0);
            } else {
                linkedMeanings = gatherMeaningInfos->getLinkedMeanings();
                unsigned char linkedMeanings_size = linkedMeanings->length();
                assert((linkedMeanings_size & 0x80) == 0);    // not signed
                binarysaver::writeChar(pEndMemory.pchar++, linkedMeanings_size);
            }
        }

        // write the number of concepts
        std::vector<const LingdbLinkToAConcept*> linkToConcepts;
        meaning->getLinkToConceptsWithoutNullRelation(linkToConcepts);
        unsigned char linkToConcepts_length = static_cast<unsigned char>(linkToConcepts.size());
        assert((linkToConcepts_length & 0x80) == 0);    // not signed
        binarysaver::writeChar(pEndMemory.pchar++, linkToConcepts_length);

        // write the context infos
        std::list<char> contextInfos;
        meaning->getAllContextInfos(contextInfos);
        assert(contextInfos.size() < 255);
        unsigned char contextInfos_length = static_cast<char>(contextInfos.size());
        assert((contextInfos_length & 0x80) == 0);    // not signed
        binarysaver::writeChar(pEndMemory.pchar++, contextInfos_length);
        for (std::list<char>::const_iterator itContInfo = contextInfos.begin(); itContInfo != contextInfos.end();
             ++itContInfo) {
            assert((*itContInfo & 0x80) == 0);    // not signed
            binarysaver::writeChar(pEndMemory.pchar++, *itContInfo);
        }
        pEndMemory = binarysaver::alignMemory(pEndMemory);

        // Write the associated concepts
        for (std::size_t iLkToCpt = 0; iLkToCpt < linkToConcepts.size(); ++iLkToCpt) {
            const LingdbConcept* concept = linkToConcepts[iLkToCpt]->getConcept();
            std::string conceptStr = concept->getName()->toStr();
            char relatedToConcept = linkToConcepts[iLkToCpt]->getRelatedToConcept();

            if (!LingdbConcept::conceptNameFinishWithAStar(conceptStr)) {
                pConceptToMeanings[concept].insert(MeaningAndConfidence(meaning, relatedToConcept));
            }
            std::map<std::string, ConceptsBinMem>::const_iterator itLink = pConceptsOffsets.find(conceptStr);
            assert(itLink != pConceptsOffsets.end());
            binarysaver::writeInThreeBytes(pEndMemory.pchar, itLink->second.alignedBegNode);
            pEndMemory.val += 3;
            binarysaver::writeChar(pEndMemory.pchar++, relatedToConcept);
        }

        // write the gathering meanings
        for (const LingdbDynamicTrieNode* multiMeaning : gatheringMeanings) {
            meaningsPointersToWrite[multiMeaning->getWordForms()->elt->getMeaning()].push_back(pEndMemory);
            binarysaver::writeInThreeBytes(pEndMemory.pchar, 0);
            pEndMemory.val += 3;
            binarysaver::writeChar(pEndMemory.pchar++, 0);
        }

        // write the linkedMeanings
        while (linkedMeanings != nullptr) {
            // write linked meaning
            meaningsPointersToWrite[linkedMeanings->elt->meaning].push_back(pEndMemory);
            binarysaver::writeInThreeBytes(pEndMemory.pchar, 0);
            pEndMemory.val += 3;
            // write the direction of the linked meaning
            binarysaver::writeChar(pEndMemory.pchar++, linkedMeanings->elt->direction);
            linkedMeanings = linkedMeanings->next;
        }

        meaning = pFPAlloc.next<LingdbMeaning>(meaning);
    }

    for (const auto& meaningPtrToWrite : meaningsPointersToWrite) {
        auto itMeaning = pMeaningsPtr.find(meaningPtrToWrite.first);
        assert(itMeaning != pMeaningsPtr.end());
        for (const binarymasks::Ptr binPtrToWrite : meaningPtrToWrite.second) {
            binarysaver::writeInThreeBytes(binPtrToWrite.pchar, itMeaning->second);
        }
    }
    return pEndMemory;
}

binarymasks::Ptr BinaryDatabaseDicoSaver::xAddSomeFlexions(std::map<std::string, unsigned char>& pFlexionsPtr,
                                                           const CompositePoolAllocator& pFPAlloc,
                                                           const binarymasks::Ptr pBeginMemory,
                                                           binarymasks::Ptr pEndMemory) const {
    LingdbFlexions* flexion = pFPAlloc.first<LingdbFlexions>();
    while (flexion != nullptr) {
        assert(flexion->getNbFlexions() > 0);
        std::string strFlexion(reinterpret_cast<const char*>(flexion->getMemory()), flexion->getNbFlexions());
        if (pFlexionsPtr.find(strFlexion) == pFlexionsPtr.end()) {
            int newOffset = binarysaver::alignedDecToSave(pEndMemory.val - pBeginMemory.val);
            // if it cannot store the offset in a char less one bit.
            // other flexions will be stored directly in the patricia trie.
            // the remaining bit will be used to say if the flexion is stored here or in the patricia trie.
            if (newOffset > 0x0000007F) {
                return pEndMemory;
            }

            pFlexionsPtr[strFlexion] = static_cast<unsigned char>(0x80 | static_cast<unsigned char>(newOffset));
            unsigned char nbFlexions = flexion->getNbFlexions();
            assert((nbFlexions & 0x80) == 0);    // not signed
            binarysaver::writeChar(pEndMemory.pchar++, nbFlexions);
            memcpy(pEndMemory.ptr, flexion->getMemory(), nbFlexions);
            pEndMemory.val += nbFlexions;
            pEndMemory = binarysaver::alignMemory(pEndMemory);
        }
        flexion = pFPAlloc.next<LingdbFlexions>(flexion);
    }
    return pEndMemory;
}

binarymasks::Ptr BinaryDatabaseDicoSaver::TreeCreationWorkState::printEndOfANode(LingdbDynamicTrieNode* pNode,
                                                                                 int pDecNode,
                                                                                 binarymasks::Ptr pEndMemory) {
    assert((pDecNode & 0xFF000000) == 0);
    nodesPtr[pNode] = pDecNode;

    std::list<std::pair<LingdbWordForms*, LingdbMeaning*> > wordFromsOrMeanings;
    pNode->getWordFormsAndMeanings(wordFromsOrMeanings);
    assert(!wordFromsOrMeanings.empty());
    for (const auto& wfOrMeaning : wordFromsOrMeanings) {
        LingdbWordForms* wordForm = wfOrMeaning.first;
        LingdbMeaning* currMeaning =
            (wfOrMeaning.second != nullptr || wordForm == nullptr) ? wfOrMeaning.second : wordForm->getMeaning();

        // write dec to meaning
        auto itMeaning = meaningsPtr.find(currMeaning);
        assert(itMeaning != meaningsPtr.end());
        binarysaver::writeInThreeBytes(pEndMemory.pchar, itMeaning->second);
        pEndMemory.val += 3;

        if (wordForm != nullptr)    // if, it's a wordfrom
        {
            // write if it's a wordForm = True
            binarysaver::writeChar(pEndMemory.pchar++, 1);

            // write the flexions
            if (wordForm->getFlexions() != nullptr) {
                std::string strFlexion(reinterpret_cast<const char*>(wordForm->getFlexions()->getMemory()),
                                       wordForm->getFlexions()->getNbFlexions());
                std::map<std::string, unsigned char>::const_iterator itFl = flexionsPtr.find(strFlexion);
                if (itFl == flexionsPtr.end()) {
                    unsigned char nbFlexions = wordForm->getFlexions()->getNbFlexions();
                    assert((nbFlexions & 0x80) == 0);    // not signed
                    binarysaver::writeChar(pEndMemory.pchar++, nbFlexions);
                    memcpy(pEndMemory.ptr, strFlexion.c_str(), strFlexion.size());
                    pEndMemory.val += strFlexion.size();
                } else {
                    // this char has his first byte equal to 1
                    // (to make difference between flexions writen inside the node)
                    binarysaver::writeChar(pEndMemory.pchar++, itFl->second);
                }
            } else {
                binarysaver::writeChar(pEndMemory.pchar++, 0);
            }
            pEndMemory = binarysaver::alignMemory(pEndMemory);
        } else {
            // write if it's a wordForm = False
            binarysaver::writeChar(pEndMemory.pchar++, 0);
        }
    }

    return pEndMemory;
}

void BinaryDatabaseDicoSaver::xWriteNbWithSpaces(std::ofstream& pOutFile, std::size_t pNb) const {
    std::size_t currNb = pNb;
    if (currNb > 999999) {
        std::size_t nbToPrint = currNb / 1000000;
        pOutFile << nbToPrint << " ";
        pOutFile << std::setfill('0') << std::setw(3);
        currNb -= nbToPrint * 1000000;
    }
    if (currNb > 999) {
        std::size_t nbToPrint = currNb / 1000;
        pOutFile << nbToPrint << " ";
        pOutFile << std::setfill('0') << std::setw(3);
        currNb -= nbToPrint * 1000;
    }
    pOutFile << currNb;
}

}    // End of namespace onsem
