#ifndef ONSEM_TEXTTOSEMANTIC_TYPE_INFLECTION_NOMINALINFLECTIONS_HPP
#define ONSEM_TEXTTOSEMANTIC_TYPE_INFLECTION_NOMINALINFLECTIONS_HPP

#include <list>
#include <set>
#include <onsem/common/enum/semanticgendertype.hpp>
#include <onsem/common/enum/semanticnumbertype.hpp>
#include <onsem/texttosemantic/dbtype/inflection/inflections.hpp>
#include "../../api.hpp"

namespace onsem {

struct ONSEM_TEXTTOSEMANTIC_API NominalInflection {
    NominalInflection() = default;
    NominalInflection(SemanticGenderType pGender, SemanticNumberType pNumber);
    NominalInflection(const std::string& pInflectionalCode);
    bool operator==(const NominalInflection& pOther) const;
    bool operator<(const NominalInflection& pOther) const;

    SemanticGenderType gender{SemanticGenderType::UNKNOWN};
    SemanticNumberType number{SemanticNumberType::UNKNOWN};
};

struct ONSEM_TEXTTOSEMANTIC_API NominalInflections : public Inflections {
    NominalInflections();
    NominalInflections(const std::vector<std::string>& pInflectionalCodes);
    void set(const std::string& pInflectionalCodes);
    bool operator==(const NominalInflections& pOther) const;
    bool empty() const { return inflections.empty(); }
    static std::unique_ptr<NominalInflections> get_inflections_ns();
    static std::unique_ptr<NominalInflections> get_inflections_ms();
    static std::unique_ptr<NominalInflections> get_inflections_ns_np();
    static std::unique_ptr<NominalInflections> get_inflections_ms_mp_fs_fp();

    virtual NominalInflections& getNominalI() { return *this; }
    virtual const NominalInflections& getNominalI() const { return *this; }
    virtual NominalInflections* getNominalIPtr() { return this; }
    virtual const NominalInflections* getNominalIPtr() const { return this; }

    std::list<NominalInflection> inflections;
};

inline std::ostream& operator<<(std::ostream& pOs, const NominalInflection& pNomInfl) {
    gender_toConcisePrint(pOs, pNomInfl.gender);
    number_toConcisePrint(pOs, pNomInfl.number);
    return pOs;
}

inline std::ostream& operator<<(std::ostream& pOs, const NominalInflections& pNomInfls) {
    bool firstLoop = true;
    for (const auto& currInfl : pNomInfls.inflections) {
        if (firstLoop)
            firstLoop = false;
        else
            pOs << ",";
        pOs << currInfl;
    }
    return pOs;
}

}    // End of namespace onsem

#include "detail/nominalinflections.hxx"

#endif    // ONSEM_TEXTTOSEMANTIC_TYPE_INFLECTION_NOMINALINFLECTIONS_HPP
