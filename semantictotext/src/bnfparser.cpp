#include <iostream>
#include <assert.h>
#include <fstream>
#include <list>
#include <vector>
#include <map>
#include <memory>
#include <sstream>

namespace onsem
{

namespace {

bool _readRuleName(
    std::string &pRuleName,
    std::size_t &pPos,
    const std::string &pLine) {
  while (pPos < pLine.size()) {
    if (pLine[pPos] == ' ') {
      ++pPos;
      continue;
    }
    if (pLine[pPos] == '<') {
      ++pPos;
      auto endPos = pLine.find('>', pPos);
      if (endPos != std::string::npos && endPos - 1 > pPos) {
        pRuleName = pLine.substr(pPos, endPos - 1);
        pPos = endPos + 1;
        return true;
      }
    }

    break;
  }
  return false;
}

enum class RegexTextType {
  RULE,
  TEXT
};


enum class RegexChildrenType {
  OPTIONAL,
  MANDATORY,
  CONCATENATE
};


struct Regex {
  Regex *parent = nullptr;
  RegexTextType textType = RegexTextType::TEXT;
  std::string text;
  RegexChildrenType childrenType = RegexChildrenType::CONCATENATE;
  std::list<Regex> values{};
  std::size_t beginPos = 0;
};


Regex *_skipConcatenateRegexes(Regex *pRegex) {
  while (pRegex != nullptr &&
         pRegex->childrenType == RegexChildrenType::CONCATENATE)
    pRegex = pRegex->parent;
  return pRegex;
}


Regex *_addChildRegex(Regex *pRegex,
                      std::size_t pPos,
                      RegexChildrenType pRegexType = RegexChildrenType::CONCATENATE) {
  auto *parent = pRegex;
  pRegex->values.emplace_back();
  pRegex = &pRegex->values.back();
  pRegex->parent = parent;
  pRegex->childrenType = pRegexType;
  pRegex->beginPos = pPos;

  if (pRegexType != RegexChildrenType::CONCATENATE)
    return _addChildRegex(pRegex, pPos);
  return pRegex;
}


Regex *_flushText(Regex *pRegex,
                  std::size_t pPos,
                  std::size_t &pBeginPos,
                  const std::string &pLine) {
  auto text = pLine.substr(pBeginPos, pPos - pBeginPos);
  if (text.empty())
    return pRegex;
  pRegex = _addChildRegex(pRegex, pPos);
  pRegex->text = text;
  pBeginPos = pPos;
  return pRegex;
}


Regex *_closeRegex(Regex *pRegex,
                   std::size_t pPos,
                   std::size_t &pBeginPos,
                   const std::string &pLine,
                   RegexChildrenType pRegexType) {
  pRegex = _flushText(pRegex, pPos, pBeginPos, pLine);
  if (pRegex == nullptr)
    return pRegex;
  pRegex = _skipConcatenateRegexes(pRegex);
  if (pRegex == nullptr)
    return pRegex;
  assert(pRegex->childrenType == pRegexType);
  if (pRegex->childrenType != pRegexType)
    return pRegex;
  pRegex = pRegex->parent;
  pBeginPos = pPos + 1;
  return pRegex;
}


void _regexToFlatenStr(
    std::vector<std::string> &pResults,
    const std::string &pPrefix,
    const Regex *pRegex) {
  if (pRegex->childrenType == RegexChildrenType::CONCATENATE) {
    std::string newPrefix = pPrefix + pRegex->text;
    std::vector<std::string> prefixes(1, newPrefix);
    for (const auto &currVal : pRegex->values) {
      std::vector<std::string> newPrefixes;
      for (const auto &currPrefix : prefixes)
        _regexToFlatenStr(newPrefixes, currPrefix, &currVal);
      prefixes = newPrefixes;
    }
    pResults.insert(pResults.end(), prefixes.begin(), prefixes.end());
  } else {
    std::string newPrefix = pPrefix + pRegex->text;
    if (pRegex->childrenType == RegexChildrenType::OPTIONAL)
      pResults.emplace_back(newPrefix);
    for (const auto &currVal : pRegex->values) {
      std::vector<std::string> newPrefixes;
      _regexToFlatenStr(newPrefixes, newPrefix, &currVal);
      pResults.insert(pResults.end(), newPrefixes.begin(), newPrefixes.end());
    }
  }
}

void _replaceRuleByTheirValues(Regex &pRegex,
                               std::map<std::string, Regex> &ruleToRegex) {
  if (pRegex.textType == RegexTextType::RULE) {
    auto itRule = ruleToRegex.find(pRegex.text);
    if (itRule != ruleToRegex.end()) {
      _replaceRuleByTheirValues(itRule->second, ruleToRegex);
      pRegex.values.push_back(itRule->second);
      pRegex.textType = RegexTextType::TEXT;
      pRegex.text = "";
    } else {
      std::cerr << "Rule not delcared: " << pRegex.text << std::endl;
      assert(false);
    }
  } else {
    for (auto &currChild : pRegex.values)
      _replaceRuleByTheirValues(currChild, ruleToRegex);
  }
}


}


std::vector<std::string> flattenBnfRegex(const std::string &pText) {
  if (pText.empty())
    return {};

  std::size_t pos = 0;
  auto beginRegex = pos;
  Regex rootRegex;
  rootRegex.childrenType = RegexChildrenType::MANDATORY;
  auto *currRegex = _addChildRegex(&rootRegex, pos);

  while (pos < pText.size()) {
    switch (pText[pos]) {
    case '<': {
      currRegex = _flushText(currRegex, pos, beginRegex, pText);
      auto beginRuleName = pos + 1;
      auto endRuleNamePos = pText.find('>', beginRuleName);
      if (endRuleNamePos != std::string::npos) {
        auto ruleName = pText.substr(beginRuleName, endRuleNamePos - beginRuleName);
        auto *ruleRegex = _addChildRegex(currRegex, pos);
        ruleRegex->textType = RegexTextType::RULE;
        ruleRegex->text = ruleName;
        beginRegex = endRuleNamePos + 1;
      }
      break;
    }
    case '(': {
      currRegex = _flushText(currRegex, pos, beginRegex, pText);
      currRegex = _addChildRegex(currRegex, pos, RegexChildrenType::MANDATORY);
      beginRegex = pos + 1;
      break;
    }
    case ')': {
      currRegex = _closeRegex(currRegex, pos, beginRegex, pText,
                              RegexChildrenType::MANDATORY);
      if (currRegex == nullptr) {
        pos = pText.size();
        break;
      }
      break;
    }
    case '[': {
      currRegex = _flushText(currRegex, pos, beginRegex, pText);
      currRegex = _addChildRegex(currRegex, pos, RegexChildrenType::OPTIONAL);
      beginRegex = pos + 1;
      break;
    }
    case ']': {
      currRegex = _closeRegex(currRegex, pos, beginRegex, pText,
                              RegexChildrenType::OPTIONAL);
      if (currRegex == nullptr) {
        pos = pText.size();
        break;
      }
      break;
    }
    case '|': {
      currRegex = _flushText(currRegex, pos, beginRegex, pText);
      currRegex = _skipConcatenateRegexes(currRegex);
      if (currRegex == nullptr) {
        pos = pText.size();
        break;
      }

      beginRegex = pos + 1;
      break;
    }
    }

    ++pos;
  }

  currRegex = _flushText(currRegex, pText.size(), beginRegex, pText);

//  if (currRegex != &rootRegex)
//    return {1, pText};

  // print the different forms of the current rule
  std::vector<std::string> results;
  _regexToFlatenStr(results, "", &rootRegex);
  return results;
}

} // End of namespace onsem
