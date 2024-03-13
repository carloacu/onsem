#include "inflectionscheckervirtual.hpp"
#include <onsem/texttosemantic/dbtype/inflectedword.hpp>
#include <onsem/texttosemantic/dbtype/inflection/adjectivalinflections.hpp>
#include <onsem/texttosemantic/dbtype/inflection/nominalinflections.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/conceptset.hpp>

namespace onsem {
namespace linguistics {

InflectionsCheckerVirtual::InflectionsCheckerVirtual(const LinguisticDictionary& pLingDic)
    : _lingDic(pLingDic) {}

bool InflectionsCheckerVirtual::isNounAdjCompatibles(const InflectedWord& pNounInflWord,
                                                     const Inflections& pAdjInflections) const {
    const auto& nounInflections = pNounInflWord.inflections();
    if (nounInflections.type != InflectionType::NOMINAL || pAdjInflections.type != InflectionType::ADJECTIVAL)
        return true;
    const NominalInflections& nomInfls = nounInflections.getNominalI();
    const AdjectivalInflections& adjInfls = pAdjInflections.getAdjectivalI();
    if (nomInfls.inflections.empty() || adjInfls.inflections.empty())
        return true;
    for (const auto& currNomInfl : nomInfls.inflections)
        for (const auto& currAdjInfl : adjInfls.inflections)
            if (gendersAreWeaklyEqual(currNomInfl.gender, currAdjInfl.gender)
                && numbersAreWeaklyEqual(currNomInfl.number, currAdjInfl.number))
                return true;
    return false;
}

bool InflectionsCheckerVirtual::_areNounNounInflectionsWeaklyEqual(const NominalInflections& pNounInfl1,
                                                                   const NominalInflections& pNounInfl2,
                                                                   bool pCheckGenders) {
    if (pNounInfl1.empty() || pNounInfl2.empty())
        return true;
    for (const auto& currInfl1 : pNounInfl1.inflections)
        for (const auto& currInfl2 : pNounInfl2.inflections)
            if ((!pCheckGenders || gendersAreWeaklyEqual(currInfl1.gender, currInfl2.gender))
                && numbersAreWeaklyEqual(currInfl1.number, currInfl2.number))
                return true;
    return false;
}

}    // End of namespace linguistics
}    // End of namespace onsem
