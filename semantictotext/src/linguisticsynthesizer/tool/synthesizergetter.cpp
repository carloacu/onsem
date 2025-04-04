#include "synthesizergetter.hpp"
#include <onsem/texttosemantic/dbtype/semanticgroundings.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/listexpression.hpp>
#include <onsem/texttosemantic/tool/semexpgetter.hpp>
#include <onsem/texttosemantic/type/enumsconvertions.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemoryblock.hpp>
#include "../grounding/linguisticsynthesizergrounding.hpp"
#include "../synthesizertypes.hpp"

namespace onsem {
namespace synthGetter {

void fillLingMeaningFromConcepts(LinguisticMeaning& pLingMeaning,
                                 const std::map<std::string, char>& pConcepts,
                                 const linguistics::SynthesizerDictionary& pOutSynth) {
    char currentLingMeaning = 0;
    for (const auto& currCpt : pConcepts) {
        if (currentLingMeaning < currCpt.second) {
            pLingMeaning = pOutSynth.conceptToMeaning(currCpt.first);
            if (!pLingMeaning.isEmpty())
                currentLingMeaning = currCpt.second;
        }
    }
}

SemanticGenderType getGender(SemanticGenderType pContextGender,
                             const std::set<SemanticGenderType>& pMeaningPossGenders,
                             const std::set<SemanticGenderType>& pGenGrdPossGenders) {
    if (pMeaningPossGenders.find(pContextGender) != pMeaningPossGenders.end())
        return pContextGender;
    if (!pGenGrdPossGenders.empty())
        return *pGenGrdPossGenders.begin();
    if (pMeaningPossGenders.size() == 1)
        return *pMeaningPossGenders.begin();
    return SemanticGenderType::UNKNOWN;
}

bool getInflWordFromWordAndConcepts(linguistics::InflectedWord& pIGram,
                                    const SemanticWord& pWord,
                                    const std::map<std::string, char>& pConcepts,
                                    const linguistics::LinguisticDatabase& pLingDb,
                                    SemanticLanguageEnum pLanguage) {
    LinguisticMeaning lingMeaning;
    SemExpGetter::wordToAMeaning(lingMeaning, pWord, pLanguage, pLingDb);
    if (lingMeaning.isEmpty() && pWord.partOfSpeech != PartOfSpeech::PROPER_NOUN) {
        const auto& synthDico = pLingDb.langToSpec[pLanguage].synthDico;
        fillLingMeaningFromConcepts(lingMeaning, pConcepts, synthDico);

        if (lingMeaning.isEmpty())
            SemExpGetter::wordToAMeaningInLanguage(lingMeaning, pWord, pLanguage, pLingDb);
    }

    SemanticLanguageEnum meaningLanguage = SemanticLanguageEnum::UNKNOWN;
    if (lingMeaning.getLanguageIfNotEmpty(meaningLanguage)) {
        const auto& lingDico = pLingDb.langToSpec[meaningLanguage].lingDico;
        lingDico.getInfoGram(pIGram, lingMeaning);
        return true;
    }

    pIGram.word = pWord;
    return false;
}

SemanticGenderType getGenderFromGrounding(const SemanticGrounding& pGrounding,
                                          const SynthesizerConfiguration& pConf,
                                          const SynthesizerCurrentContext& pContext,
                                          const Linguisticsynthesizergrounding& pGrdSynth) {
    const SemanticGenericGrounding* genGrd = pGrounding.getGenericGroundingPtr();
    if (genGrd != nullptr) {
        linguistics::InflectedWord outInfoGram;
        SynthesizerWordContext wordContext;
        pGrdSynth.modifyContextForAGrounding(
            wordContext, outInfoGram, pConf, *genGrd, pContext.contextType, pContext.verbTense);
        return wordContext.gender;
    }

    const SemanticAgentGrounding* agentGrd = pGrounding.getAgentGroundingPtr();
    if (agentGrd != nullptr) {
        if (agentGrd->isSpecificUser())
            return pConf.memBlock.getGender(agentGrd->userId);
        return SemanticGenderType::UNKNOWN;
    }

    const SemanticNameGrounding* nameGrdPtr = pGrounding.getNameGroundingPtr();
    if (nameGrdPtr != nullptr)
        return SemExpGetter::possibleGendersToGender(nameGrdPtr->nameInfos.possibleGenders);

    const SemanticUnityGrounding* unityGrdPtr = pGrounding.getUnityGroundingPtr();
    if (unityGrdPtr != nullptr) {
        const auto& synthDico = pConf.lingDb.langToSpec[pConf.textProcessingContext.langType].synthDico;
        const auto& meaning = synthDico.conceptToMeaning(unityGrdPtr->getValueConcept());
        if (!meaning.isEmpty()) {
            std::string word;
            SemanticNumberType number = SemanticNumberType::SINGULAR;
            SemanticGenderType gender = SemanticGenderType::UNKNOWN;
            synthDico.getNounForm(word, meaning, gender, number);
            return gender;
        }
    }
    return SemanticGenderType::UNKNOWN;
}

SemanticGenderType getGenderFromSemExp(const SemanticExpression& pSemExp,
                                       const SynthesizerConfiguration& pConf,
                                       const SynthesizerCurrentContext& pContext,
                                       const Linguisticsynthesizergrounding& pGrdSynth) {
    const GroundedExpression* grdExpPtr = pSemExp.getGrdExpPtr_SkipWrapperPtrs();
    if (grdExpPtr != nullptr) {
        const GroundedExpression& grdExp = *grdExpPtr;
        SemanticGenderType res = getGenderFromGrounding(*grdExp, pConf, pContext, pGrdSynth);
        if (res == SemanticGenderType::UNKNOWN) {
            auto itSpecifier = grdExp.children.find(GrammaticalType::SPECIFIER);
            if (itSpecifier != grdExp.children.end())
                return getGenderFromSemExp(*itSpecifier->second, pConf, pContext, pGrdSynth);
        }
        return res;
    }

    const ListExpression* listExp = pSemExp.getListExpPtr();
    if (listExp != nullptr) {
        SemanticGenderType res = SemanticGenderType::UNKNOWN;
        for (const auto& currElt : listExp->elts) {
            SemanticGenderType subGender = getGenderFromSemExp(*currElt, pConf, pContext, pGrdSynth);
            if (subGender == SemanticGenderType::MASCULINE) {
                return SemanticGenderType::MASCULINE;
            }
            res = subGender;
        }
        return res;
    }
    return SemanticGenderType::UNKNOWN;
}

bool doesOutFinishedWithAS(const std::list<WordToSynthesize>& pOut) {
    if (!pOut.empty()) {
        const auto& inflections = pOut.back().inflections;
        if (!inflections.empty()) {
            const auto& str = inflections.front().str;
            return !str.empty() && str.back() == 's';
        }
    }
    return false;
}

}    // End of namespace synthGetter
}    // End of namespace onsem
