#include "verbcanbefollowedbydeinfr.hpp"
#include <QtXml>
#include <fstream>
#include <onsem/compilermodel/lingdbmeaning.hpp>
#include <onsem/compilermodel/lingdbdynamictrienode.hpp>
#include <onsem/texttosemantic/type/chunk.hpp>
#include <onsem/texttosemantic/type/chunklink.hpp>
#include <onsem/texttosemantic/type/syntacticgraph.hpp>

namespace onsem {

namespace {
const linguistics::Chunk* _getObjectChunk(const linguistics::Chunk& pVerbChunk) {
    for (const auto& currChild : pVerbChunk.children)
        if (currChild.type == linguistics::ChunkLinkType::DIRECTOBJECT)
            return &*currChild.chunk;
    return nullptr;
}
}

VerbCanBeFollowedByDeInFr::VerbCanBeFollowedByDeInFr(const CompositePoolAllocator& pAlloc)
    : _verbStats() {
    // init map of verb to auxiliaries stats
    LingdbMeaning* meaning = pAlloc.first<LingdbMeaning>();
    while (meaning != nullptr) {
        if (meaning->getPartOfSpeech() == PartOfSpeech::VERB)
            _verbStats[meaning->getLemma()->getWord()];
        meaning = pAlloc.next<LingdbMeaning>(meaning);
    }
}

VerbCanBeFollowedByDeInFr::VerbCanBeFollowedByDeInFr(const std::vector<VerbCanBeFollowedByDeInFr>& pOthers)
    : _verbStats() {
    for (const auto& currOther : pOthers)
        for (const auto& currVerbStat : currOther._verbStats)
            _verbStats[currVerbStat.first].add(currVerbStat.second);
}

void VerbCanBeFollowedByDeInFr::processText(const linguistics::SyntacticGraph& pSyntGraph, const std::string& pText) {
    xProcessChunkLinkList(pSyntGraph.firstChildren, pText);
}

void VerbCanBeFollowedByDeInFr::writeResults(const std::string& pResultFilename) const {
    std::cout << "Write results life: " << pResultFilename << std::endl;
    std::ofstream resultFile(pResultFilename.c_str());
    for (auto it = _verbStats.begin(); it != _verbStats.end(); ++it)
        resultFile << "#" << it->first << "\t#" << it->second.nbOccs << "_" << it->second.nbFollowedByDe << "_"
                   << std::endl;
    resultFile.close();
}

void VerbCanBeFollowedByDeInFr::writeXml(const std::string& pResultFilename, const std::string& pResultFilenameXml) {
    std::cout << "Write xml life: " << pResultFilenameXml << std::endl;
    std::ifstream resultFile(pResultFilename.c_str(), std::ifstream::in);
    if (!resultFile.is_open()) {
        std::cerr << "Error: Can't open " << pResultFilename << " file !" << std::endl;
        return;
    }

    QDomDocument resultXml;
    QDomElement rootXml = resultXml.createElement("linguisticdatabase");
    resultXml.appendChild(rootXml);
    std::string line;
    while (getline(resultFile, line)) {
        std::string lemma;
        std::vector<int> nbs(2);
        spitResultLine(lemma, nbs, line);
        int nbOccs = nbs[0];
        int nbFollowedByDe = nbs[1];
        if ((nbFollowedByDe * 100) > nbOccs && nbFollowedByDe > 10) {
            if (lemma.size() > 3 && lemma.compare(lemma.size() - 3, 3, "~se") != 0) {
                QDomElement newBeAux = resultXml.createElement("existingMeaning");
                newBeAux.setAttribute("lemme", QString::fromUtf8(lemma.c_str()));
                newBeAux.setAttribute("gram", QString::fromUtf8(partOfSpeech_toStr(PartOfSpeech::VERB).c_str()));
                QDomElement contextInfo = resultXml.createElement("contextInfo");
                contextInfo.setAttribute("val", "fr_verbFollowedByWordDe");
                newBeAux.appendChild(contextInfo);
                rootXml.appendChild(newBeAux);
            }
        }
    }
    resultFile.close();

    QFile file(QString::fromUtf8(pResultFilenameXml.c_str()));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        std::cerr << "Open the file for writing failed" << std::endl;
    } else {
        QString text = resultXml.toString();
        file.write(text.toUtf8());
        file.close();
    }
}

void VerbCanBeFollowedByDeInFr::xProcessChunkLinkList(const std::list<linguistics::ChunkLink>& pChunkLinks,
                                                      const std::string& pText) {
    for (const auto& currChkLk : pChunkLinks) {
        const linguistics::Chunk& chunk = *currChkLk.chunk;
        if (chunk.type == linguistics::ChunkType::VERB_CHUNK) {
            const std::string& lemma = chunk.head->inflWords.front().word.lemma;
            auto itVerbStats = _verbStats.find(lemma);
            if (itVerbStats == _verbStats.end()) {
                std::cerr << "verb: " << lemma << " is not in database (" << pText << ")" << std::endl;
            } else {
                ++itVerbStats->second.nbOccs;
                const linguistics::Chunk* objChunk = _getObjectChunk(chunk);
                if (objChunk != nullptr) {
                    const std::string& lemma = objChunk->tokRange.getItBegin()->inflWords.front().word.lemma;
                    if (lemma == "de" || lemma == "du")
                        ++itVerbStats->second.nbFollowedByDe;
                }
            }
        }
        xProcessChunkLinkList(chunk.children, pText);
    }
}

}    // End of namespace onsem
