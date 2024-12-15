#include <onsem/texttosemantic/dbtype/resourcegroundingextractor.hpp>

namespace onsem {

PairOfLabelAndBeginOfResource::PairOfLabelAndBeginOfResource(const std::string& pLabel)
    : label(pLabel)
    , begOfResourceStr("\\" + pLabel + "=")
    , begOfResourceStrSize(begOfResourceStr.size()) {}

ResourceGroundingExtractor::ResourceGroundingExtractor(const std::string& pLabelStr)
    : _labels() {
    _labels.emplace_back(pLabelStr);
}

ResourceGroundingExtractor::ResourceGroundingExtractor(const std::vector<std::string>& pLabelsStrs)
    : _labels() {
    for (const auto& currLabelStr : pLabelsStrs)
        _labels.emplace_back(currLabelStr);
}

const std::string& ResourceGroundingExtractor::extractBeginOfAResource(const std::string& pStr) const {
    for (const PairOfLabelAndBeginOfResource& currLabel : _labels)
        if (pStr.compare(0, currLabel.begOfResourceStrSize, currLabel.begOfResourceStr) == 0)
            return currLabel.label;
    static const std::string emptyString;
    return emptyString;
}

bool ResourceGroundingExtractor::isBeginOfAResource(const std::string& pStr) const {
    return !extractBeginOfAResource(pStr).empty();
}

SemanticLanguageEnum _readLanguageEnumStr(std::size_t& pBeginValuePos,
                                          std::size_t pBeginOfLanguagePos,
                                          const std::string& pStr) {
    SemanticLanguageEnum res = SemanticLanguageEnum::UNKNOWN;
    std::size_t endPos = pStr.find('#', pBeginOfLanguagePos);
    if (endPos != std::string::npos
        && semanticLanguageEnum_fromStrIfExist(res, pStr.substr(pBeginOfLanguagePos, endPos - pBeginOfLanguagePos)))
        pBeginValuePos = endPos + 1;
    return res;
}

std::unique_ptr<SemanticResourceGrounding> ResourceGroundingExtractor::makeResourceGroundingFromStr(
    const std::string& pStr,
    const std::string& pLabelStr) {
    const std::string endOfAParam = "\\";

    if (pStr.size() > endOfAParam.size()
        && pStr.compare(pStr.size() - endOfAParam.size(), endOfAParam.size(), endOfAParam) == 0) {
        std::size_t begOfResourceStrSize = pLabelStr.size() + 2;
        auto endOfCommand = pStr.size() - endOfAParam.size();
        if (begOfResourceStrSize < pStr.size() && endOfCommand > begOfResourceStrSize) {
            std::size_t beginOfValue = begOfResourceStrSize;
            SemanticLanguageEnum language = SemanticLanguageEnum::UNKNOWN;
            if (pStr[beginOfValue] == '#') {
                std::size_t beginOfLanguage = beginOfValue + 1;
                language = _readLanguageEnumStr(beginOfValue, beginOfLanguage, pStr);
            }
            return std::make_unique<SemanticResourceGrounding>(
                pLabelStr, language, pStr.substr(beginOfValue, endOfCommand - beginOfValue));
        }
    }
    return std::unique_ptr<SemanticResourceGrounding>();
}

}    // End of namespace onsem
