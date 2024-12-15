#include <onsem/texttosemantic/dbtype/linguisticdatabase/conceptset.hpp>
#include <sstream>
#include <onsem/common/utility/lexical_cast.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticlanguagegrounding.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticgenericgrounding.hpp>
#include <onsem/texttosemantic/dbtype/linguisticmeaning.hpp>

namespace onsem {

ConceptSet::ConceptSet(std::istream& pIStream)
    : statDb(_getStatDbInstance(pIStream))
    , _localConcepts()
    , _oppositeConcepts() {}

const StaticConceptSet& ConceptSet::_getStatDbInstance(std::istream& pIStream) {
  static std::mutex _pathToStatConceptsMutex{};
  static std::unique_ptr<StaticConceptSet> _statConcepts{};

  std::lock_guard<std::mutex> lock(_pathToStatConceptsMutex);
    if (!_statConcepts)
        _statConcepts = std::make_unique<StaticConceptSet>(pIStream);
    return *_statConcepts;
}

void ConceptSet::addConcept(const std::string& pConceptName) {
    if (!statDb.hasConcept(pConceptName))
        _localConcepts.insert(pConceptName);
}

void ConceptSet::notifyOppositeConcepts(const std::string& pConceptName1, const std::string& pConceptName2) {
    _oppositeConcepts[pConceptName1].insert(pConceptName2);
    _oppositeConcepts[pConceptName2].insert(pConceptName1);
}

void ConceptSet::getOppositeConcepts(std::set<std::string>& pOppositeConcepts, const std::string& pConcept) const {
    statDb.getOppositeConcepts(pOppositeConcepts, pConcept);
    auto it = _oppositeConcepts.find(pConcept);
    if (it != _oppositeConcepts.end())
        pOppositeConcepts.insert(it->second.begin(), it->second.end());
}

bool ConceptSet::areOppositeConcepts(const std::string& pConcept1, const std::string& pConcept2) const {
    std::set<std::string> oppositeConcepts;
    getOppositeConcepts(oppositeConcepts, pConcept1);
    return oppositeConcepts.count(pConcept2) > 0;
}

bool ConceptSet::haveOppositeConcepts(const std::map<std::string, char>& pConcepts1,
                                      const std::map<std::string, char>& pConcepts2) const {
    for (const auto& currCpt1 : pConcepts1) {
        std::set<std::string> oppositeConcepts1;
        getOppositeConcepts(oppositeConcepts1, currCpt1.first);
        for (const auto& oppCpt1 : oppositeConcepts1)
            if (pConcepts2.count(oppCpt1) > 0)
                return true;
    }
    return false;
}

void ConceptSet::conceptToChildConcepts(std::vector<std::string>& pResConcepts, const std::string& pConceptName) const {
    statDb.conceptToChildConcepts(pResConcepts, pConceptName);
}

void ConceptSet::conceptToNearlyEqualConcepts(std::vector<std::string>& pResConcepts,
                                              const std::string& pConceptName) const {
    statDb.conceptsToNearlyEqualConcepts(pResConcepts, pConceptName);
}

void ConceptSet::conceptToParentConcepts(std::vector<std::string>& pResConcepts, const std::string& pConceptName) {
    for (std::size_t i = 0; i < pConceptName.size(); ++i)
        if (pConceptName[i] == '_')
            pResConcepts.emplace_back(pConceptName.substr(0, i));
}

bool ConceptSet::cpts1HaveAParentCptOf2(const std::map<std::string, char>& pConcepts1,
                                        const std::map<std::string, char>& pConcepts2) {
    for (const auto& currCpt2 : pConcepts2) {
        std::vector<std::string> parentConcepts;
        conceptToParentConcepts(parentConcepts, currCpt2.first);
        for (const auto& currParentCpt : parentConcepts)
            if (pConcepts1.count(currParentCpt) > 0)
                return true;
    }
    return false;
}

void ConceptSet::reset() {
    _localConcepts.clear();
}

std::size_t ConceptSet::getStaticConceptsSize(std::string& pErrorStr, bool& pIsLoaded) const {
    return statDb.getSize(pErrorStr, pIsLoaded);
}

mystd::optional<bool> ConceptSet::areConceptsCompatibles(const std::map<std::string, char>& pConcepts1,
                                                         const std::map<std::string, char>& pConcepts2) const {
    mystd::optional<bool> res;
    if (!_areConceptsCompatiblesOneSide(res, pConcepts1, pConcepts2))
        _areConceptsCompatiblesOneSide(res, pConcepts2, pConcepts1);
    return res;
}

bool ConceptSet::_areConceptsCompatiblesOneSide(mystd::optional<bool>& pRes,
                                                const std::map<std::string, char>& pConcepts1,
                                                const std::map<std::string, char>& pConcepts2) const {
    for (const auto& currCpt1 : pConcepts1) {
        bool cpt1IsAConceptAny = isAConceptAny(currCpt1.first);
        for (const auto& currCpt2 : pConcepts2) {
            if (statDb.areConceptsNearlyEqual(currCpt1.first, currCpt2.first)) {
                pRes.emplace(true);
                return true;
            }

            if (!cpt1IsAConceptAny) {
                if (currCpt1.first == currCpt2.first) {
                    pRes.emplace(true);
                    return true;
                }
                pRes.emplace(false);
            }
        }
    }
    return false;
}

bool ConceptSet::haveAConcept(const std::map<std::string, char>& pInputConcepts, const std::string& pConceptName) {
    return pInputConcepts.find(pConceptName) != pInputConcepts.end();
}

bool ConceptSet::haveAnyOfConcepts(const std::map<std::string, char>& pInputConcepts,
                                   const std::vector<std::string>& pConceptNames) {
    for (const auto& currConceptName : pConceptNames)
        if (pInputConcepts.find(currConceptName) != pInputConcepts.end())
            return true;
    return false;
}

std::pair<std::string, char> ConceptSet::getAnyOfConcepts(const std::map<std::string, char>& pInputConcepts,
                                                          const std::vector<std::string>& pConceptNames) {
    for (const auto& currConceptName : pConceptNames) {
        auto it = pInputConcepts.find(currConceptName);
        if (it != pInputConcepts.end())
            return *it;
    }
    return {};
}

bool ConceptSet::haveAConceptThatBeginWith(const std::map<std::string, char>& pInputConcepts,
                                           const std::string& pBeginOfConceptName) {
    for (const auto& currConcept : pInputConcepts)
        if (doesConceptBeginWith(currConcept.first, pBeginOfConceptName))
            return true;
    return false;
}

bool ConceptSet::haveAConceptOrAHyponym(const std::map<std::string, char>& pInputConcepts,
                                        const std::string& pConcept) {
    for (const auto& currConcept : pInputConcepts)
        if (doesConceptBeginWith(currConcept.first, pConcept)
            && (currConcept.first.size() <= pConcept.size() || currConcept.first[pConcept.size()] == '_'))
            return true;
    return false;
}

bool ConceptSet::haveAnotherConceptThan(const std::map<std::string, char>& pInputConcepts,
                                        const std::string& pConceptName) {
    for (const auto& currConcept : pInputConcepts)
        if (currConcept.first != pConceptName)
            return true;
    return false;
}

void ConceptSet::removeConceptsOrHyponyms(std::map<std::string, char>& pInputConcepts, const std::string& pConcept) {
    for (auto it = pInputConcepts.begin(); it != pInputConcepts.end();) {
        if (doesConceptBeginWith(it->first, pConcept)
            && (it->first.size() <= pConcept.size() || it->first[pConcept.size()] == '_'))
            it = pInputConcepts.erase(it);
        else
            ++it;
    }
}

bool ConceptSet::haveAConceptThatBeginWithAnyOf(const std::map<std::string, char>& pInputConcepts,
                                                const std::vector<std::string>& pBeginOfConcepts) {
    for (const auto& currInCpt : pInputConcepts)
        if (doesConceptBeginWithWithAnyOf(currInCpt.first, pBeginOfConcepts))
            return true;
    return false;
}

bool ConceptSet::doesConceptBeginWithWithAnyOf(const std::string& pConceptName,
                                               const std::vector<std::string>& pBeginOfConcepts) {
    for (const auto& currBeginOfCpt : pBeginOfConcepts)
        if (pConceptName.compare(0, currBeginOfCpt.size(), currBeginOfCpt) == 0)
            return true;
    return false;
}

void ConceptSet::extractConceptsThatBeginWith(std::map<std::string, char>& pExtractedConcepts,
                                              const std::map<std::string, char>& pInputConcepts,
                                              const std::string& pBeginOfConceptName) {
    // This method don't read directly the database
    for (const auto& currConcept : pInputConcepts)
        if (doesConceptBeginWith(currConcept.first, pBeginOfConceptName))
            pExtractedConcepts.emplace(currConcept.first, currConcept.second);
}

bool ConceptSet::extractUserId(std::string& pUserId, const std::map<std::string, char>& pInputConcepts) {
    static const std::string begOfUserIdConceptStr = "agent_userId_";
    static const std::size_t begOfUserIdConceptStrSize = begOfUserIdConceptStr.size();
    for (const auto& currCpt : pInputConcepts) {
        if (currCpt.first.size() > begOfUserIdConceptStrSize
            && doesConceptBeginWith(currCpt.first, begOfUserIdConceptStr)) {
            pUserId = currCpt.first.substr(begOfUserIdConceptStrSize, currCpt.first.size() - begOfUserIdConceptStrSize);
            return true;
        }
    }
    return false;
}

bool ConceptSet::haveAConceptIncompatibleWithSomethingCountable(const std::map<std::string, char>& pInputConcepts) {
    return haveAConceptThatBeginWith(pInputConcepts, "liquid_");
}

char ConceptSet::getConfidenceOfConcept(const std::map<std::string, char>& pInputConcepts,
                                        const std::string& pConceptName) {
    auto it = pInputConcepts.find(pConceptName);
    if (it != pInputConcepts.end())
        return it->second;
    return 0;
}

bool ConceptSet::isAConceptAny(const std::string& pConceptName) {
    return pConceptName.size() > 2 && pConceptName.compare(pConceptName.size() - 2, 2, "_*") == 0;
}

bool ConceptSet::haveAConceptNotAny(const std::map<std::string, char>& pInputConcepts) {
    for (const auto& currConcept : pInputConcepts)
        if (!isAConceptAny(currConcept.first))
            return true;
    return false;
}

void ConceptSet::getMotherConceptOfConceptAny(std::string& pConceptName) {
    if (pConceptName.size() > 2 && pConceptName.compare(pConceptName.size() - 2, 2, "_*") == 0)
        pConceptName = pConceptName.substr(0, pConceptName.size() - 2);
}

void ConceptSet::sortConceptsSortedByConfidence(std::map<char, std::map<std::string, char>>& pSortedConcepts,
                                                const std::map<std::string, char>& pConcepts) {
    for (const auto& currConcept : pConcepts) {
        pSortedConcepts[currConcept.second][currConcept.first] = currConcept.second;
    }
}

bool ConceptSet::haveAConceptParentOf(const std::map<std::string, char>& pConcepts1,
                                      const std::map<std::string, char>& pConcepts2) {
    for (const auto& currConcept : pConcepts1) {
        std::vector<std::string> parentConcepts;
        conceptToParentConcepts(parentConcepts, currConcept.first);
        for (const auto& currParentConcept : parentConcepts)
            if (haveAConcept(pConcepts2, currParentConcept))
                return true;
    }
    return false;
}

void ConceptSet::sortAndPrintConcepts(std::list<std::string>& pPrintedConcepts,
                                      const std::map<std::string, char>& pConcepts) {
    std::map<char, std::map<std::string, char>> sortedConcepts;
    ConceptSet::sortConceptsSortedByConfidence(sortedConcepts, pConcepts);
    for (const auto& currCptsForACoef : sortedConcepts) {
        for (const auto& currCpt : currCptsForACoef.second) {
            std::stringstream ss;
            ss << currCpt.first << ", " << static_cast<int>(currCpt.second);
            pPrintedConcepts.emplace_front(ss.str());
        }
    }
}

bool ConceptSet::verbSyntesisIsFollowedByAnInfinitiveThatMeansAnImperative(
    const std::map<std::string, char>& pConcepts) {
    return pConcepts.count("verb_action_say_ask") > 0;
}

RelativePerson ConceptSet::conceptsToRelativePerson(const std::map<std::string, char>& pConcepts) {
    for (const auto& currCpt : pConcepts) {
        if (currCpt.first == "tolink_1s")
            return RelativePerson::FIRST_SING;
        if (currCpt.first == "tolink_2s")
            return RelativePerson::SECOND_SING;
        if (currCpt.first == "tolink_3s")
            return RelativePerson::THIRD_SING;
        if (currCpt.first == "tolink_1p")
            return RelativePerson::FIRST_PLUR;
        if (currCpt.first == "tolink_2p")
            return RelativePerson::SECOND_PLUR;
        if (currCpt.first == "tolink_3p")
            return RelativePerson::THIRD_PLUR;
    }
    return RelativePerson::UNKNOWN;
}

bool ConceptSet::rankConceptToNumberStr(std::string& pNumberStr, const std::map<std::string, char>& pConcepts) {
    static const std::string begOfRankCpt = "rank_";
    static const std::size_t begOfRankCpt_size = begOfRankCpt.size();
    for (const auto& currCpt : pConcepts) {
        if (ConceptSet::doesConceptBeginWith(currCpt.first, begOfRankCpt)) {
            pNumberStr = currCpt.first.substr(begOfRankCpt_size, currCpt.first.size() - begOfRankCpt_size);
            try {
                mystd::lexical_cast<int>(pNumberStr);
                return true;
            } catch (...) {}
        }
    }
    return false;
}


const std::string& ConceptSet::getConceptVerbEquality()
{
  static const std::string conceptVerbEquality = "verb_equal_be";
  return conceptVerbEquality;
}


}    // End of namespace onsem
