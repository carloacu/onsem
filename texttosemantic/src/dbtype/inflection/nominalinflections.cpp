#include <onsem/texttosemantic/dbtype/inflection/nominalinflections.hpp>
#include <onsem/common/utility/string.hpp>

namespace onsem {

std::unique_ptr<NominalInflections> NominalInflections::get_inflections_ns() {
    auto res = std::make_unique<NominalInflections>();
    res->inflections.emplace_back(SemanticGenderType::NEUTRAL, SemanticNumberType::SINGULAR);
    return res;
}

std::unique_ptr<NominalInflections> NominalInflections::get_inflections_ms() {
    auto res = std::make_unique<NominalInflections>();
    res->inflections.emplace_back(SemanticGenderType::MASCULINE, SemanticNumberType::SINGULAR);
    return res;
}

std::unique_ptr<NominalInflections> NominalInflections::get_inflections_ns_np() {
    auto res = std::make_unique<NominalInflections>();
    res->inflections.emplace_back(SemanticGenderType::NEUTRAL, SemanticNumberType::SINGULAR);
    res->inflections.emplace_back(SemanticGenderType::NEUTRAL, SemanticNumberType::PLURAL);
    return res;
}

std::unique_ptr<NominalInflections> NominalInflections::get_inflections_ms_mp_fs_fp() {
    auto res = std::make_unique<NominalInflections>();
    res->inflections.emplace_back(SemanticGenderType::MASCULINE, SemanticNumberType::SINGULAR);
    res->inflections.emplace_back(SemanticGenderType::MASCULINE, SemanticNumberType::PLURAL);
    res->inflections.emplace_back(SemanticGenderType::FEMININE, SemanticNumberType::SINGULAR);
    res->inflections.emplace_back(SemanticGenderType::FEMININE, SemanticNumberType::PLURAL);
    return res;
}

void NominalInflections::set(const std::string& pInflectionalCodes) {
    if (!pInflectionalCodes.empty()) {
        std::vector<std::string> vecOfInflectionalCodes;
        mystd::split(vecOfInflectionalCodes, pInflectionalCodes, ",");
        for (const auto& currInflCode : vecOfInflectionalCodes)
            inflections.emplace_back(currInflCode);
    }
}

}    // End of namespace onsem
