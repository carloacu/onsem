#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticmetagrounding.hpp>
#include <onsem/common/utility/lexical_cast.hpp>

namespace onsem {
const char SemanticMetaGrounding::firstCharOfStrOld = '\\';
const char SemanticMetaGrounding::firstCharOfStr = '[';
const int SemanticMetaGrounding::returnId = -1;
static const std::string beginOfParam = "\\p_";

bool SemanticMetaGrounding::operator==(const SemanticMetaGrounding& pOther) const {
    return this->isEqual(pOther);
}

bool SemanticMetaGrounding::isEqual(const SemanticMetaGrounding& pOther) const {
    return _isMotherClassEqual(pOther) && refToType == pOther.refToType && paramId == pOther.paramId
        && attibuteName == pOther.attibuteName && attibuteValue == pOther.attibuteValue;
}

bool SemanticMetaGrounding::groundingTypeFromStr(SemanticGroundingType& pRefToType, const std::string& pStr) {
    const std::string endOfAParam = "\\";

    if (isTheBeginOfAParamOld(pStr) && pStr.size() > endOfAParam.size()
        && pStr.compare(pStr.size() - endOfAParam.size(), endOfAParam.size(), endOfAParam) == 0) {
        std::size_t beginOfGrdType = beginOfParam.size();
        std::size_t endOfGrdType = pStr.find('=', beginOfGrdType + 1);
        if (endOfGrdType != std::string::npos) {
            return semanticGroundingsType_fromStrIfExist(pRefToType,
                                                         pStr.substr(beginOfGrdType, endOfGrdType - beginOfGrdType));
        }
    }
    return false;
}

bool SemanticMetaGrounding::isTheBeginOfAParamOld(const std::string& pStr) {
    return pStr.compare(0, beginOfParam.size(), beginOfParam) == 0;
}

bool SemanticMetaGrounding::parseParameterOld(int& pParamId,
                                              std::string& pLabel,
                                              std::string& pAttributeName,
                                              const std::string& pStr) {
    const std::string endOfAParam = "\\";

    if (isTheBeginOfAParamOld(pStr) && pStr.size() > endOfAParam.size()
            && pStr.compare(pStr.size() - endOfAParam.size(), endOfAParam.size(), endOfAParam) == 0) {
        std::size_t beginOfGrdType = beginOfParam.size();
        std::size_t endOfGrdType = pStr.find('=', beginOfGrdType + 1);
        if (endOfGrdType != std::string::npos) {
            std::size_t beginOfParamId = endOfGrdType + 1;
            auto endOfValue = pStr.size() - endOfAParam.size();
            auto endOfParamId = endOfValue;

            auto beginOfAttributeName = pStr.find('_', beginOfParamId);
            if (beginOfAttributeName != std::string::npos) {
                endOfParamId = beginOfAttributeName;
                ++beginOfAttributeName;
                if (beginOfAttributeName < pStr.size())
                    pAttributeName = pStr.substr(beginOfAttributeName, endOfValue - beginOfAttributeName);
            }

            if (beginOfParamId < pStr.size() && endOfParamId > beginOfParamId) {
                try {
                    pParamId = mystd::lexical_cast<int>(pStr.substr(beginOfParamId, endOfParamId - beginOfParamId));
                } catch (...) { return false; }
                pLabel = pStr.substr(beginOfGrdType, endOfGrdType - beginOfGrdType);
                return true;
            }
        }
    }
    return false;
}

bool SemanticMetaGrounding::parseParameter(std::string& pAttributeName,
                                           std::string& pAttributeValue,
                                           const std::string& pStr) {
    auto endPos = pStr.find(']', 1);
    if (endPos != std::string::npos) {
        pAttributeName = pStr.substr(1, endPos - 1);

        auto beginOfParenthesis = endPos + 1;
        if (beginOfParenthesis < pStr.size() &&
            pStr[beginOfParenthesis] == '(') {
            auto beginOfInsideParenthesis = beginOfParenthesis + 1;
            auto endParenthesisPos = pStr.find(')', beginOfInsideParenthesis);
            if (endParenthesisPos != std::string::npos) {
                pAttributeValue = pStr.substr(beginOfInsideParenthesis, endParenthesisPos - beginOfInsideParenthesis);
            }
        }
        return true;
    }
    return false;
}

std::unique_ptr<SemanticMetaGrounding> SemanticMetaGrounding::makeMetaGroundingFromStrOld(const std::string& pStr) {
    int paramId = 0;
    std::string label;
    std::string attributeName;
    if (parseParameterOld(paramId, label, attributeName, pStr)) {
        SemanticGroundingType refToType = SemanticGroundingType::GENERIC;
        if (semanticGroundingsType_fromStrIfExist(refToType, label))
            return std::make_unique<SemanticMetaGrounding>(refToType, paramId, attributeName);
    }
    return std::unique_ptr<SemanticMetaGrounding>();
}


std::unique_ptr<SemanticMetaGrounding> SemanticMetaGrounding::makeMetaGroundingFromStr(const std::string& pStr) {
    std::string attributeName;
    std::string attributeValue;
    if (parseParameter(attributeName, attributeValue, pStr)) {
        SemanticGroundingType refToType = SemanticGroundingType::META;
        return std::make_unique<SemanticMetaGrounding>(refToType, 0, attributeName, attributeValue);
    }
    return std::unique_ptr<SemanticMetaGrounding>();
}

}    // End of namespace onsem
