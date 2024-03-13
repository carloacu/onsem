#ifndef ONSEM_COMPILERMODEL_MEANINGANDONFIDENCE_HPP
#define ONSEM_COMPILERMODEL_MEANINGANDONFIDENCE_HPP

#include <onsem/compilermodel/lingdbmeaning.hpp>

namespace onsem {

struct MeaningAndConfidence {
    MeaningAndConfidence(LingdbMeaning* pMeaning, char pConfidence)
        : meaning(pMeaning)
        , confidence(pConfidence) {}

    MeaningAndConfidence(const MeaningAndConfidence& pObj)
        : meaning(pObj.meaning)
        , confidence(pObj.confidence) {}

    bool operator<(const MeaningAndConfidence& pOther) const;

    LingdbMeaning* meaning;
    char confidence;
};

}    // End of namespace onsem

#endif    // ONSEM_COMPILERMODEL_MEANINGANDONFIDENCE_HPP
