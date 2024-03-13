#include "grdexptooutputterinformation.hpp"
#include <onsem/texttosemantic/dbtype/linguisticdatabase/conceptset.hpp>
#include <onsem/texttosemantic/tool/semexpgetter.hpp>

namespace onsem {

bool grdExpToOutputInformation(OutputInformation& pOutputInformation, const GroundedExpression& pGrdExp) {
    auto& grounding = pGrdExp.grounding();
    if (grounding.type != SemanticGroundingType::STATEMENT)
        return false;
    bool res = false;
    bool checkForBackgroundChild = false;

    if (ConceptSet::haveAConcept(grounding.concepts, "verb_action_repeat")) {
        int nbOfRepetitions = SemExpGetter::getNumberOfRepetitions(pGrdExp.children);
        pOutputInformation.nbOfTimes.emplace(nbOfRepetitions + 1);
        res = true;
        checkForBackgroundChild = true;
    }

    if (ConceptSet::haveAConcept(grounding.concepts, "verb_action")) {
        int nbOfTimes = SemExpGetter::getNumberOfRepetitions(pGrdExp.children);
        if (nbOfTimes > 1) {
            pOutputInformation.nbOfTimes.emplace(nbOfTimes + 1);
            res = true;
        }
        checkForBackgroundChild = true;
    }

    if (checkForBackgroundChild) {
        auto itBackgroundChild = pGrdExp.children.find(GrammaticalType::IN_BACKGROUND);
        if (itBackgroundChild != pGrdExp.children.end()) {
            pOutputInformation.toDoInBackground.emplace(&*itBackgroundChild->second);
            res = true;
        }
    }

    return res;
}

bool hasGrdExpOutputInformations(const GroundedExpression& pGrdExp) {
    OutputInformation outputInformation;
    return grdExpToOutputInformation(outputInformation, pGrdExp);
}

}    // End of namespace onsem
