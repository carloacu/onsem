#ifndef ONSEM_TEXTTOSEMANTIC_TYPE_INFLECTION_VERBALINFLECTIONS_HPP
#define ONSEM_TEXTTOSEMANTIC_TYPE_INFLECTION_VERBALINFLECTIONS_HPP

#include <list>
#include <onsem/common/enum/linguisticverbtense.hpp>
#include <onsem/common/enum/relativeperson.hpp>
#include <onsem/common/enum/semanticgendertype.hpp>
#include <onsem/common/enum/semanticnumbertype.hpp>
#include <onsem/texttosemantic/dbtype/inflection/inflections.hpp>
#include "../../api.hpp"

namespace onsem {

struct ONSEM_TEXTTOSEMANTIC_API VerbalInflection {
    VerbalInflection() = default;
    VerbalInflection(const std::string& pInflectionalCode);
    bool operator==(const VerbalInflection& pOther) const;
    SemanticNumberType number() const { return relativePerson_toNumberType(person); }

    RelativePerson person{RelativePerson::UNKNOWN};
    LinguisticVerbTense tense{LinguisticVerbTense::INFINITIVE};
    SemanticGenderType gender{SemanticGenderType::UNKNOWN};
};

struct ONSEM_TEXTTOSEMANTIC_API VerbalInflections : public Inflections {
    VerbalInflections();
    VerbalInflections(const std::vector<std::string>& pInflectionalCodes);
    bool operator==(const VerbalInflections& pOther) const;

    static std::unique_ptr<VerbalInflections> get_inflections_infinitive();
    static std::unique_ptr<VerbalInflections> get_inflections_imperative();

    virtual VerbalInflections& getVerbalI() { return *this; }
    virtual const VerbalInflections& getVerbalI() const { return *this; }
    virtual VerbalInflections* getVerbalIPtr() { return this; }
    virtual const VerbalInflections* getVerbalIPtr() const { return this; }

    std::list<VerbalInflection> inflections;
};

inline std::ostream& operator<<(std::ostream& pOs, const VerbalInflection& pVerbInfl) {
    pOs << linguisticVerbTense_toChar(pVerbInfl.tense);
    relativePerson_toConcisePrintWithoutNumber(pOs, pVerbInfl.person);
    gender_toConcisePrint(pOs, pVerbInfl.gender);
    number_toConcisePrint(pOs, pVerbInfl.number());
    return pOs;
}

inline std::ostream& operator<<(std::ostream& pOs, const VerbalInflections& pVerbInfls) {
    bool firstLoop = true;
    for (const auto& currInfl : pVerbInfls.inflections) {
        if (firstLoop)
            firstLoop = false;
        else
            pOs << ",";
        pOs << currInfl;
    }
    return pOs;
}

}    // End of namespace onsem

#include "detail/verbalinflections.hxx"

#endif    // ONSEM_TEXTTOSEMANTIC_TYPE_INFLECTION_VERBALINFLECTIONS_HPP
