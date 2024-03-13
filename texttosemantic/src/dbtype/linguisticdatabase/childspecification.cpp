#include "childspecification.hpp"
#include <iostream>
#include <onsem/common/utility/getendofparenthesis.hpp>
#include <onsem/common/utility/string.hpp>

namespace onsem {
namespace linguistics {
namespace {
// trim from start (in place)
void _ltrim(std::string& s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) { return !std::isspace(ch); }));
}

void _strToConditionValue(std::shared_ptr<LinguisticConditionTree>& pConditionTree,
                          const std::string& pConditionStr,
                          const std::size_t pBegin,
                          const std::size_t pEnd) {
    if (pBegin >= pEnd)
        return;
    if (pConditionStr[pBegin] == '!') {
        auto res = std::make_shared<LinguisticConditionTreeOperand>(LinguisticConditionTreeOperandEnum::NOT);
        res->children.emplace_back();
        _strToConditionValue(res->children.back(), pConditionStr, pBegin + 1, pEnd);
        pConditionTree = std::move(res);
        return;
    }

    LinguisticCondition condition;
    for (std::size_t i = pBegin; i < pEnd; i++) {
        if (pConditionStr[i] == '(') {
            if (linguisticCondition_fromStr(condition, pConditionStr.substr(pBegin, i - pBegin))) {
                auto res = std::make_shared<LinguisticConditionTreeValue>(condition);
                std::size_t begOfParametersPos = i + 1;
                auto itEndOfParenthesis = getEndOfParenthesis(pConditionStr, i, pEnd);
                if (itEndOfParenthesis != std::string::npos) {
                    std::string allParametersStr =
                        pConditionStr.substr(begOfParametersPos, itEndOfParenthesis - begOfParametersPos);
                    mystd::split(res->parameters, allParametersStr, ",");
                    for (auto& currParam : res->parameters)
                        _ltrim(currParam);
                    pConditionTree = std::move(res);
                }
            }
            return;
        }
    }

    if (linguisticCondition_fromStr(condition, pConditionStr.substr(pBegin, pEnd - pBegin)))
        pConditionTree = std::make_shared<LinguisticConditionTreeValue>(condition);
}

void _strToConditionTree(std::shared_ptr<LinguisticConditionTree>& pConditionTree,
                         const std::string& pConditionStr,
                         const std::size_t pBegin,
                         const std::size_t pEnd);

void _completeConditionTreeWithStr(std::shared_ptr<LinguisticConditionTree>& pConditionTree,
                                   const std::string& pConditionStr,
                                   const std::size_t pBegin,
                                   const std::size_t pEnd) {
    if (pBegin >= pEnd)
        return;
    mystd::optional<LinguisticConditionTreeOperandEnum> operatorEnumOpt;
    switch (pConditionStr[pBegin]) {
        case '&': operatorEnumOpt = LinguisticConditionTreeOperandEnum::AND; break;
        case '|': operatorEnumOpt = LinguisticConditionTreeOperandEnum::OR; break;
    }
    if (operatorEnumOpt) {
        if (pConditionTree && pConditionTree->type == LinguisticConditionTreeType::OPERAND
            && pConditionTree->getOperand().operand == *operatorEnumOpt) {
            auto& operandStruct = pConditionTree->getOperand();
            operandStruct.children.emplace_back();
            _strToConditionTree(operandStruct.children.back(), pConditionStr, pBegin + 1, pEnd);
        } else {
            auto res = std::make_shared<LinguisticConditionTreeOperand>(*operatorEnumOpt);
            res->children.emplace_back(std::move(pConditionTree));
            res->children.emplace_back();
            _strToConditionTree(res->children.back(), pConditionStr, pBegin + 1, pEnd);
            pConditionTree = std::move(res);
        }
    }
}

void _strToConditionTree(std::shared_ptr<LinguisticConditionTree>& pConditionTree,
                         const std::string& pConditionStr,
                         const std::size_t pBegin,
                         const std::size_t pEnd) {
    if (pBegin >= pEnd)
        return;
    if (pConditionStr[pBegin] == '[') {
        const std::size_t endingFound = getEndOfParenthesis(pConditionStr, pBegin, pEnd);
        if (endingFound != std::string::npos) {
            _strToConditionTree(pConditionTree, pConditionStr, pBegin + 1, endingFound);
            _completeConditionTreeWithStr(pConditionTree, pConditionStr, endingFound + 1, pEnd);
            return;
        }
        assert(false);
        return;
    }

    auto itAndPos = mystd::findFirstOf(pConditionStr, "&|", pBegin + 1, pEnd);
    if (itAndPos != pEnd) {
        _strToConditionValue(pConditionTree, pConditionStr, pBegin, itAndPos);
        _completeConditionTreeWithStr(pConditionTree, pConditionStr, itAndPos, pEnd);
    } else {
        _strToConditionValue(pConditionTree, pConditionStr, pBegin, pEnd);
    }
}
}

const boost::property_tree::ptree& fillChildSpecs(ChildSpecification& pChildSpec,
                                                  const boost::property_tree::ptree& pChildTree,
                                                  SemanticLanguageEnum pLanguage) {
    auto& childAttributes = pChildTree.get_child("<xmlattr>");
    pChildSpec.chunkLinkType = chunkLinkType_fromStr(childAttributes.get<std::string>("chunkLinkType"));

    const std::string introWordLemma = childAttributes.get<std::string>("lemma", "");
    if (!introWordLemma.empty()) {
        const std::string posStr = childAttributes.get<std::string>("pos", "");
        const PartOfSpeech posEnum = posStr.empty() ? PartOfSpeech::UNKNOWN : partOfSpeech_fromStr(posStr);
        pChildSpec.introWord = SemanticWord(pLanguage, introWordLemma, posEnum);
    }

    const std::string conditionStr = childAttributes.get<std::string>("condition", "");
    if (!conditionStr.empty())
        _strToConditionTree(pChildSpec.conditionTree, conditionStr, 0, conditionStr.size());

    for (const auto& currCond : pChildTree) {
        if (currCond.first == "remove_verb_concept")
            pChildSpec.verbConceptsToRemove.emplace_back(currCond.second.get<std::string>("<xmlattr>.val"));
        if (currCond.first == "add_concept")
            pChildSpec.conceptsToAdd.emplace_back(currCond.second.get<std::string>("<xmlattr>.val"));
    }
    return childAttributes;
}

void fillChildSpecsFromId(ChildSpecification& pChildSpec,
                          const boost::property_tree::ptree& pChildTree,
                          SemanticLanguageEnum pLanguage,
                          int& pLastTemplatePos) {
    fillChildSpecs(pChildSpec, pChildTree, pLanguage);
    pChildSpec.templatePos = pLastTemplatePos;
    ++pLastTemplatePos;
}

void fillChildSpecsFromBookmark(ChildSpecification& pChildSpec,
                                const boost::property_tree::ptree& pChildTree,
                                SemanticLanguageEnum pLanguage,
                                const std::map<std::string, int>& pBookmarkToTemplatePos) {
    auto& childAttributes = fillChildSpecs(pChildSpec, pChildTree, pLanguage);
    const std::string bookmarkName = childAttributes.get<std::string>("bookmark", "");
    auto itBookmark = pBookmarkToTemplatePos.find(bookmarkName);
    if (itBookmark != pBookmarkToTemplatePos.end()) {
        pChildSpec.templatePos = itBookmark->second;
    } else {
        std::cerr << "Bookmark not found [" << bookmarkName << "]!" << std::endl;
        assert(false);
    }
}

}    // End of namespace linguistics
}    // End of namespace onsem
