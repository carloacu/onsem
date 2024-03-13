#include <onsem/compilermodel/lingdbflexions.hpp>
#include <onsem/typeallocatorandserializer/typeallocatorandserializer.hpp>
#include <onsem/compilermodel/lingdbwordforms.hpp>
#include <onsem/compilermodel/lingdbmeaning.hpp>
#include <onsem/compilermodel/lingdbdynamictrienode.hpp>

namespace onsem {

void LingdbFlexions::xDeallocate(CompositePoolAllocator& pFPAlloc) {
    pFPAlloc.deallocate<unsigned char>(fFlexions, fNbFlexions);
    pFPAlloc.deallocate<LingdbFlexions>(this);
}

LingdbFlexions* LingdbFlexions::xClone(CompositePoolAllocator& pFPAlloc) const {
    LingdbFlexions* newFlexions = pFPAlloc.allocate<LingdbFlexions>(1);
    newFlexions->fNbFlexions = fNbFlexions;
    newFlexions->fFlexions = pFPAlloc.allocate<unsigned char>(newFlexions->fNbFlexions);
    memcpy(newFlexions->fFlexions, fFlexions, fNbFlexions);
    return newFlexions;
}

LingdbFlexions::LingdbFlexions()
    : fNbFlexions(0)
    , fFlexions(nullptr) {}

void LingdbFlexions::xInit(CompositePoolAllocator& pFPAlloc,
                           PartOfSpeech pGram,
                           const std::vector<std::string>& pFlexions,
                           LingdbWordForms* pWFForErrorLog) {
    fNbFlexions = static_cast<unsigned char>(pFlexions.size());
    fFlexions = pFPAlloc.allocate<unsigned char>(fNbFlexions);
    for (std::size_t i = 0; i < pFlexions.size(); ++i) {
        fFlexions[i] = xFlexionStringToChar(pGram, pFlexions[i], pWFForErrorLog);
    }
}

void LingdbFlexions::xAddNewFlexions(CompositePoolAllocator& pFPAlloc,
                                     PartOfSpeech pGram,
                                     const std::vector<std::string>& pFlexions,
                                     LingdbWordForms* pWFForErrorLog) {
    unsigned char newNbFlexions = static_cast<unsigned char>(fNbFlexions + pFlexions.size());
    unsigned char* newFlexions = pFPAlloc.allocate<unsigned char>(newNbFlexions);
    memcpy(newFlexions, fFlexions, fNbFlexions);
    unsigned char idFls = fNbFlexions;
    for (std::size_t i = 0; i < pFlexions.size(); ++i) {
        char newChar = xFlexionStringToChar(pGram, pFlexions[i], pWFForErrorLog);
        bool alreadyExist = false;
        for (std::size_t j = 0; j < idFls; ++j) {
            if (newFlexions[j] == newChar) {
                alreadyExist = true;
            }
        }
        if (alreadyExist == false) {
            newFlexions[idFls++] = newChar;
        }
    }
    if (idFls == fNbFlexions) {
        pFPAlloc.deallocate<unsigned char>(newFlexions, newNbFlexions);
        return;
    }
    pFPAlloc.deallocate<unsigned char>(fFlexions, fNbFlexions);
    fNbFlexions = idFls;
    if (idFls < newNbFlexions) {
        fFlexions = pFPAlloc.allocate<unsigned char>(fNbFlexions);
        memcpy(fFlexions, newFlexions, idFls);
        pFPAlloc.deallocate<unsigned char>(newFlexions, newNbFlexions);
    } else {
        fFlexions = newFlexions;
    }
}

unsigned char LingdbFlexions::getNbFlexions() const {
    return fNbFlexions;
}

const unsigned char* LingdbFlexions::getMemory() const {
    return fFlexions;
}

void LingdbFlexions::xGetPointers(std::vector<const void*>& pRes, void* pVar) {
    pRes.emplace_back(&reinterpret_cast<LingdbFlexions*>(pVar)->fFlexions);
}

char LingdbFlexions::xFlexionStringToChar(PartOfSpeech pGram,
                                          const std::string& pFlexion,
                                          LingdbWordForms* pWFForErrorLog) const {
    switch (pGram) {
        case PartOfSpeech::PROPER_NOUN:
        case PartOfSpeech::NOUN:
        case PartOfSpeech::ADJECTIVE:
        case PartOfSpeech::DETERMINER:
        case PartOfSpeech::PARTITIVE:
        case PartOfSpeech::PREPOSITION: {
            return xFlexionStringToCharForNoun(pFlexion);
        }
        case PartOfSpeech::VERB:
        case PartOfSpeech::AUX: {
            return xFlexionStringToCharForVerb(pFlexion);
        }
        case PartOfSpeech::PRONOUN:
        case PartOfSpeech::PRONOUN_COMPLEMENT:
        case PartOfSpeech::PRONOUN_SUBJECT: {
            return xFlexionStringToCharForPronoun(pFlexion);
        }
        case PartOfSpeech::ADVERB: return 0;
        default: {
        }
    }

    std::cerr << partOfSpeech_toStr(pGram) << " doesn't allow any flexion, but found: " << pFlexion
              << " for the word: " << pWFForErrorLog->getMeaning()->getLemma()->getWord() << std::endl;
    return 0;
}

char LingdbFlexions::xFlexionStringToCharForNoun(const std::string& pFlexion) const {
    std::size_t i = 0;
    char res = 0;
    if (pFlexion[i] == 'm') {
        res |= 0x01;
        ++i;
    } else if (pFlexion[i] == 'f') {
        res |= 0x02;
        ++i;
    } else if (pFlexion[i] == 'n') {
        res |= 0x03;
        ++i;
    } else if (pFlexion[i] == 'C')    // comparative
    {
        res |= 0x10;
        ++i;
    } else if (pFlexion[i] == 'S')    // superlative
    {
        res |= 0x20;
        ++i;
    }

    if (pFlexion.size() > i) {
        if (pFlexion[i] == 's') {
            res |= 0x04;
            ++i;
        } else if (pFlexion[i] == 'p') {
            res |= 0x08;
            ++i;
        }

        if (pFlexion.size() > i) {
            std::cerr << "bad noun flexion: " << pFlexion << std::endl;
        }
    }
    return res;
}

char LingdbFlexions::xFlexionStringToCharForPronoun(const std::string& pFlexion) const {
    std::size_t i = 0;
    char res = 0;
    if (pFlexion[i] == '1') {
        res |= 0x01;
        ++i;
    } else if (pFlexion[i] == '2') {
        res |= 0x02;
        ++i;
    } else if (pFlexion[i] == '3') {
        res |= 0x03;
        ++i;
    }

    if (pFlexion[i] == 'm') {
        res |= 0x04;
        ++i;
    } else if (pFlexion[i] == 'f') {
        res |= 0x08;
        ++i;
    } else if (pFlexion[i] == 'n') {
        res |= 0x0C;
        ++i;
    }

    if (pFlexion.size() > i) {
        if (pFlexion[i] == 's') {
            res |= 0x10;
            ++i;
        } else if (pFlexion[i] == 'p') {
            res |= 0x20;
            ++i;
        }
        if (pFlexion.size() > i) {
            std::cerr << "bad pronoun flexion: " << pFlexion << std::endl;
        }
    }

    return res;
}

void LingdbFlexions::fillVerbConjugaison(VerbConjugaison& pVerbConjugaison,
                                         const LingdbDynamicTrieNode* pTriNode,
                                         char pFrequency) const {
    for (unsigned char i = 0; i < fNbFlexions; ++i) {
        if ((fFlexions[i] & 0x0F) == 0x00)    // present
        {
            xFillVerbTense(pVerbConjugaison.present, fFlexions[i], pTriNode, pFrequency);
        } else if ((fFlexions[i] & 0x0F) == 0x01)    // imperfect
        {
            xFillVerbTense(pVerbConjugaison.imperfect, fFlexions[i], pTriNode, pFrequency);
        } else if ((fFlexions[i] & 0x0F) == 0x02)    // present subjunctive
        {
            xFillVerbTense(pVerbConjugaison.presentSubjunctive, fFlexions[i], pTriNode, pFrequency);
        } else if ((fFlexions[i] & 0x0F) == 0x04)    // imperative
        {
            xFillImperativeVerbTense(pVerbConjugaison.imperative, fFlexions[i], pTriNode, pFrequency);
        } else if ((fFlexions[i] & 0x0F) == 0x05)    // conditional
        {
            xFillVerbTense(pVerbConjugaison.conditional, fFlexions[i], pTriNode, pFrequency);
        } else if ((fFlexions[i] & 0x0F) == 0x07)    // infinitive
        {
            pVerbConjugaison.infinitive.newNodeCandidate(pTriNode, pFrequency);
        } else if ((fFlexions[i] & 0x0F) == 0x08)    // present participle
        {
            pVerbConjugaison.presentParticiple.newNodeCandidate(pTriNode, pFrequency);
        } else if ((fFlexions[i] & 0x0F) == 0x09)    // past participle
        {
            xFillVerbNumberAndGenderConjugaison(pVerbConjugaison.pastParticiple, fFlexions[i], pTriNode, pFrequency);
        } else if ((fFlexions[i] & 0x0F) == 0x0A)    // future
        {
            xFillVerbTense(pVerbConjugaison.future, fFlexions[i], pTriNode, pFrequency);
        }
    }
}

void LingdbFlexions::xFillVerbNumberAndGenderConjugaison(
    VerbNumberAndGenderConjugaison& pVerbNumberAndGenderConjugaison,
    char pCurrFlexion,
    const LingdbDynamicTrieNode* pTriNode,
    char pFrequency) const {
    if ((pCurrFlexion & 0xC0) == 0x80)    // p
    {
        if ((pCurrFlexion & 0x30) == 0x20)    // f
        {
            pVerbNumberAndGenderConjugaison.femininePlural.newNodeCandidate(pTriNode, pFrequency);
        } else {
            pVerbNumberAndGenderConjugaison.masculinePlural.newNodeCandidate(pTriNode, pFrequency);
        }
    } else    // s
    {
        if ((pCurrFlexion & 0x30) == 0x20)    // f
        {
            pVerbNumberAndGenderConjugaison.feminineSingular.newNodeCandidate(pTriNode, pFrequency);
        } else {
            pVerbNumberAndGenderConjugaison.masculineSingular.newNodeCandidate(pTriNode, pFrequency);
        }
    }
}

void LingdbFlexions::fillNounConjugaison(NounAdjConjugaison& pNounConjugaison,
                                         const LingdbDynamicTrieNode* pTriNode,
                                         char pFrequency) const {
    for (unsigned char i = 0; i < fNbFlexions; ++i) {
        if (fFlexions[i] == 0x10)    // C
        {
            pNounConjugaison.comparative.newNodeCandidate(pTriNode, pFrequency);
            continue;
        } else if (fFlexions[i] == 0x20)    // S
        {
            pNounConjugaison.superlative.newNodeCandidate(pTriNode, pFrequency);
            continue;
        }

        if ((fFlexions[i] & 0x0C) == 0x08)    // p
        {
            if ((fFlexions[i] & 0x03) == 0x01)    // m
            {
                pNounConjugaison.masculinePlural.newNodeCandidate(pTriNode, pFrequency);
            } else if ((fFlexions[i] & 0x03) == 0x02)    // f
            {
                pNounConjugaison.femininePlural.newNodeCandidate(pTriNode, pFrequency);
            } else {
                pNounConjugaison.neutralPlural.newNodeCandidate(pTriNode, pFrequency);
            }
        } else    // s
        {
            if ((fFlexions[i] & 0x03) == 0x01)    // m
            {
                pNounConjugaison.masculineSingular.newNodeCandidate(pTriNode, pFrequency);
            } else if ((fFlexions[i] & 0x03) == 0x02)    // f
            {
                pNounConjugaison.feminineSingular.newNodeCandidate(pTriNode, pFrequency);
            } else {
                pNounConjugaison.neutralSingular.newNodeCandidate(pTriNode, pFrequency);
            }
        }
    }
}

void LingdbFlexions::xFillImperativeVerbTense(std::vector<WordLinkForConj>& pVerbTense,
                                              char pCurrFlexion,
                                              const LingdbDynamicTrieNode* pTriNode,
                                              char pFrequency) const {
    int index;
    if ((pCurrFlexion & 0xC0) == 0x80)    // plural
    {
        if ((pCurrFlexion & 0x30) == 0x10)    // 1e pers
        {
            index = 1;
        } else {
            index = 2;
        }
    } else {
        index = 0;
    }
    pVerbTense[index].newNodeCandidate(pTriNode, pFrequency);
}

void LingdbFlexions::xFillVerbTense(std::vector<WordLinkForConj>& pVerbTense,
                                    char pCurrFlexion,
                                    const LingdbDynamicTrieNode* pTriNode,
                                    char pFrequency) const {
    int index;
    if ((pCurrFlexion & 0x30) == 0x10)    // 1e pers
    {
        index = 0;
    } else if ((pCurrFlexion & 0x30) == 0x20)    // 2e pers
    {
        index = 1;
    } else if ((pCurrFlexion & 0x30) == 0x30)    // 3e pers
    {
        index = 2;
    } else {
        return;
    }

    if ((pCurrFlexion & 0xC0) == 0x80)    // plural
    {
        index += 3;
    } else if ((pCurrFlexion & 0xC0) != 0x40)    // not singular
    {
        return;
    }
    pVerbTense[index].newNodeCandidate(pTriNode, pFrequency);
}

bool LingdbFlexions::replaceInfinitiveByImperative() const {
    for (unsigned char i = 0; i < fNbFlexions; ++i) {
        if ((fFlexions[i] & 0x0F) == 0x07)    // at infinitive
        {
            fFlexions[i] &= 0xF0;    // clear tense value
            fFlexions[i] |= 0x04;    // put imperative tense
            return true;
        }
    }
    return false;
}

char LingdbFlexions::xFlexionStringToCharForVerb(const std::string& pFlexion) const {
    unsigned char res = 0;
    if (pFlexion[0] == 'P')    // present
    {
        res |= 0x00;
    } else if (pFlexion[0] == 'I')    // imperfect
    {
        res |= 0x01;
    } else if (pFlexion[0] == 'S')    // present subjunctive
    {
        res |= 0x02;
    } else if (pFlexion[0] == 'T')    // imperfect subjunctive
    {
        res |= 0x03;
    } else if (pFlexion[0] == 'Y')    // present imperative
    {
        res |= 0x04;
    } else if (pFlexion[0] == 'C')    // present conditional
    {
        res |= 0x05;
    } else if (pFlexion[0] == 'J')    // past
    {
        res |= 0x06;
    } else if (pFlexion[0] == 'W')    // infinitive
    {
        res |= 0x07;
    } else if (pFlexion[0] == 'G')    // present participle
    {
        res |= 0x08;
    } else if (pFlexion[0] == 'K')    // past participle
    {
        res |= 0x09;
    } else if (pFlexion[0] == 'F')    // future
    {
        res |= 0x0A;
    } else {
        std::cerr << "bad verb flexion: " << pFlexion << std::endl;
    }

    if (pFlexion.size() > 1) {
        if (pFlexion[1] == 'm' || pFlexion[1] == '1') {
            res |= 0x10;
        } else if (pFlexion[1] == 'f' || pFlexion[1] == '2') {
            res |= 0x20;
        } else if (pFlexion[1] == 'n' || pFlexion[1] == '3') {
            res |= 0x30;
        } else {
            std::cerr << "bad verb flexion: " << pFlexion << std::endl;
        }

        if (pFlexion.size() > 2) {
            if (pFlexion[2] == 's') {
                res |= 0x40;
            } else if (pFlexion[2] == 'p') {
                res |= 0x80;
            } else {
                std::cerr << "bad verb flexion: " << pFlexion << std::endl;
            }
        }
    }

    return res;
}

}    // End of namespace onsem
