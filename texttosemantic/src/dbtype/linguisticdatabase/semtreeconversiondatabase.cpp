#include <onsem/texttosemantic/dbtype/linguisticdatabase/detail/semtreeconversiondatabase.hpp>
#include <iostream>
#include <boost/property_tree/xml_parser.hpp>
#include <onsem/common/utility/lexical_cast.hpp>
#include <onsem/common/utility/string.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticlanguagegrounding.hpp>

namespace onsem {

namespace {

void _extractConfidenceAtEndOfStr(std::string& pCptWithConfidence, std::list<char>& pConfidences) {
    std::size_t lastComma = pCptWithConfidence.find_last_of(',');
    if (lastComma != std::string::npos && lastComma + 1 < pCptWithConfidence.size()) {
        std::size_t beginNbConfidence = lastComma + 1;
        try {
            char newConfidence = static_cast<char>(mystd::lexical_cast<int>(
                pCptWithConfidence.substr(beginNbConfidence, pCptWithConfidence.size() - beginNbConfidence)));
            pConfidences.emplace_back(newConfidence);
            pCptWithConfidence = pCptWithConfidence.substr(0, lastComma);
        } catch (...) {}
    }
}

}

// this "if" is needed otherwise we have a crash on mac if we try to iterate on an empty tree
#define childLoop(TREE, ELT, LABEL)                    \
    auto optChildren = TREE.get_child_optional(LABEL); \
    if (optChildren)                                   \
        for (const auto& ELT : *optChildren)

SemExpTreeConversionDatabase::SemExpTreeConversionDatabase(linguistics::LinguisticDatabaseStreams& pIStreams)
    : VirtualSemBinaryDatabase()
    , isLoaded(false)
    , fConversions()
    , _semanticFormsBothDirections()
    , _semanticForms()
    , fTreesOfSemUniquePattern()
    , fCurrFilename()
    , fCurrConversionNb() {
    xLoad(pIStreams);
}

const std::string& SemExpTreeConversionDatabase::getFormalism()
{
  static const std::string formalism = "1";
  return formalism;
}

void SemExpTreeConversionDatabase::xLoad(linguistics::LinguisticDatabaseStreams& pIStreams) {
    if (isLoaded)
        return;
    isLoaded = true;

    for (auto& currLangToStreams : pIStreams.languageToStreams) {
        SemanticLanguageEnum langEnum = currLangToStreams.first;
        for (auto& currStreams : currLangToStreams.second.conversionsStreams)
            xLoadConvFile(langEnum, *currStreams.second, currStreams.first);
    }
}

void SemExpTreeConversionDatabase::xLoadConvFile(SemanticLanguageEnum pLangEnum,
                                                 std::istream& pIStream,
                                                 const std::string& pLocalPath) {
    fCurrFilename = pLocalPath;

    boost::property_tree::ptree tree;
    boost::property_tree::read_xml(pIStream, tree);

    fCurrConversionNb = 0;
    std::map<TreePatternEnumPair, std::list<std::pair<std::list<std::string>, ConversionRule>>> cptsToConvs;
    std::list<std::pair<std::list<std::string>, ConversionRule>> questionsPossTreesBothDirectionList;
    std::list<std::pair<std::list<std::string>, ConversionRule>> questionsPossList;
    std::list<std::pair<std::list<std::string>, UniqueInformationRule>> semanticUniquePatterns;

    childLoop(tree, currSubTree, "semantictreeconversions") {
        const std::string beaconName = currSubTree.first;
        const boost::property_tree::ptree& childTree = currSubTree.second;

        if (beaconName == "conversion") {
            ++fCurrConversionNb;
            PatternWorkStruct patternWkStruct;
            if (!childTree.empty()) {
                for (const auto& currElt : childTree) {
                    if (currElt.first != "tree")
                        throw std::runtime_error(
                            "Only a \"tree\" beacon can be directly inside a \"conversion\" beacon. "
                            "Here we found a \""
                            + currElt.first + "\" beacon.");
                    xAddConversionTreePattern(patternWkStruct, currElt.second);
                }
            }
            xGetGroupsOfConversions(cptsToConvs, patternWkStruct);
        } else if (beaconName == "addPossibleTrees") {
            ++fCurrConversionNb;
            xLoadAddPossibleTreesBeacon(childTree, questionsPossTreesBothDirectionList, questionsPossList);
        } else if (beaconName == "shallBeUnique") {
            ++fCurrConversionNb;
            xLoadUniquePatterns(childTree, semanticUniquePatterns);
        } else if (beaconName == "<xmlattr>") {
            if (childTree.get<std::string>("formalism") != getFormalism()) {
                fErrorMessage = "BAD_FORMALISM";
                return;
            }
        }
    }

    for (auto itCptToCv = cptsToConvs.begin(); itCptToCv != cptsToConvs.end(); ++itCptToCv) {
        xFillTreeOfConvs(fConversions[pLangEnum][itCptToCv->first], itCptToCv->second);
        fTotalSize += 100;
    }
    xFillTreeOfConvs(_semanticFormsBothDirections, questionsPossTreesBothDirectionList);
    xFillTreeOfConvs(_semanticForms, questionsPossList);
    xFillTreeOfConvs(fTreesOfSemUniquePattern, semanticUniquePatterns);
    fErrorMessage = "";
}

void SemExpTreeConversionDatabase::xLoadUniquePatterns(
    const boost::property_tree::ptree& pTree,
    std::list<std::pair<std::list<std::string>, UniqueInformationRule>>& pSemanticUniquePatterns) const {
    UniqueInformationRule semanticUniquePattern;

    childLoop(pTree, currAttribute, "<xmlattr>") {
        const std::string attrName = currAttribute.first;
        if (attrName == "childThatIsUnique")
            semanticUniquePattern.childThatIsUnique =
                grammaticalType_fromStr(currAttribute.second.get_value<std::string>());
    }
    assert(semanticUniquePattern.childThatIsUnique != GrammaticalType::UNKNOWN);

    RootNodeWithRelatedCpts treePattern;
    xAddTreePattern(treePattern, pTree);
    semanticUniquePattern.treePattern = treePattern.rootNode;
    pSemanticUniquePatterns.emplace_back(treePattern.concepts, std::move(semanticUniquePattern));
}

void SemExpTreeConversionDatabase::xLoadAddPossibleTreesBeacon(
    const boost::property_tree::ptree& pTree,
    std::list<std::pair<std::list<std::string>, ConversionRule>>& pQuestionsPossTreesBothDirectionList,
    std::list<std::pair<std::list<std::string>, ConversionRule>>& pQuestionsPossTreesList) const {
    RootNodeWithRelatedCpts fromTreePattern;
    RootNodeWithRelatedCpts toTreePattern;
    bool bothDirections = false;

    childLoop(pTree, currAttribute, "<xmlattr>") {
        const std::string attrName = currAttribute.first;
        if (attrName == "bothDirections")
            bothDirections = currAttribute.second.get_value<std::string>() == "true";
        else
            xWriteErrorMsg("\"" + attrName + "\" is not a valid addPossibleTrees attibute");
    }

    // from beacon
    {
        const boost::property_tree::ptree& fromTree = pTree.get_child("from");
        xAddTreePattern(fromTreePattern, fromTree);
    }

    // to beacon
    int addStrength = 0;
    {
        const boost::property_tree::ptree& toTree = pTree.get_child("to");
        auto strength = toTree.get_child_optional("<xmlattr>.addStrength");
        if (strength) {
            try {
                addStrength = mystd::lexical_cast<int>(strength->get_value<std::string>());
            } catch (...) {
                addStrength = 0;
                xWriteErrorMsg("Cannot convert strength \"" + strength->get_value<std::string>()
                               + "\" to an int value");
            }
        }
        xAddTreePattern(toTreePattern, toTree);
    }

    pQuestionsPossTreesList.emplace_back(
        fromTreePattern.concepts,
        ConversionRule(
            fCurrFilename, fCurrConversionNb, addStrength, fromTreePattern.rootNode, toTreePattern.rootNode));
    if (bothDirections) {
        pQuestionsPossTreesList.emplace_back(
            fromTreePattern.concepts,
            ConversionRule(
                fCurrFilename, fCurrConversionNb, addStrength, toTreePattern.rootNode, fromTreePattern.rootNode));

        pQuestionsPossTreesBothDirectionList.emplace_back(
            fromTreePattern.concepts,
            ConversionRule(
                fCurrFilename, fCurrConversionNb, addStrength, fromTreePattern.rootNode, toTreePattern.rootNode));
        pQuestionsPossTreesBothDirectionList.emplace_back(
            toTreePattern.concepts,
            ConversionRule(
                fCurrFilename, fCurrConversionNb, addStrength, toTreePattern.rootNode, fromTreePattern.rootNode));
    }
}

template<typename RULE>
void SemExpTreeConversionDatabase::xFillTreeOfConvs(
    ConceptTreeOfRules<RULE>& pTreeOfConvs,
    std::list<std::pair<std::list<std::string>, RULE>>& pCptToCvs) const {
    // get frequences for each concepts
    std::map<std::string, int> cptToFreqs;
    for (auto it = pCptToCvs.begin(); it != pCptToCvs.end();) {
        if (it->first.empty()) {
            pTreeOfConvs.rules.push_back(it->second);
            it = pCptToCvs.erase(it);
            continue;
        } else {
            for (const auto& currCpt : it->first)
                ++cptToFreqs[currCpt];
        }
        ++it;
    }

    // get more frequent elt
    std::map<std::string, int>::iterator itMoreFrequent = cptToFreqs.end();
    int nbFreqForTheMoreFreq = 0;
    for (auto it = cptToFreqs.begin(); it != cptToFreqs.end(); ++it) {
        if (it->second > nbFreqForTheMoreFreq) {
            itMoreFrequent = it;
            nbFreqForTheMoreFreq = it->second;
        }
    }

    if (itMoreFrequent != cptToFreqs.end()) {
        const std::string& moreFrequentCpt = itMoreFrequent->first;

        // get list of elts that hold the more frequent concept
        std::list<std::pair<std::list<std::string>, RULE>> moreFreqCptToCvs;
        for (auto itCptToCvs = pCptToCvs.begin(); itCptToCvs != pCptToCvs.end();) {
            auto itMoreFreqCpt = std::find(itCptToCvs->first.begin(), itCptToCvs->first.end(), moreFrequentCpt);
            if (itMoreFreqCpt != itCptToCvs->first.end()) {
                itCptToCvs->first.erase(itMoreFreqCpt);
                moreFreqCptToCvs.splice(moreFreqCptToCvs.begin(), pCptToCvs, itCptToCvs++);
                continue;
            }
            ++itCptToCvs;
        }

        // add a child for the more frequent concept
        xFillTreeOfConvs(pTreeOfConvs.children[moreFrequentCpt], moreFreqCptToCvs);

        // fill the other children of the same node
        if (!pCptToCvs.empty()) {
            xFillTreeOfConvs(pTreeOfConvs, pCptToCvs);
        }
    }
}

void SemExpTreeConversionDatabase::xGetGroupsOfConversions(
    std::map<TreePatternEnumPair, std::list<std::pair<std::list<std::string>, ConversionRule>>>& pRes,
    PatternWorkStruct& pPatternWkStruct) const {
    // iterate through "from step" conversions
    for (std::map<TreePatternConventionEnum, std::list<RootNodeWithRelatedCpts>>::iterator itFrom =
             pPatternWkStruct.begin();
         itFrom != pPatternWkStruct.end();
         ++itFrom) {
        if (itFrom->first != TREEPATTERN_OUTTEXT) {
            // iterate through "to step" conversions
            for (std::map<TreePatternConventionEnum, std::list<RootNodeWithRelatedCpts>>::iterator itTo =
                     pPatternWkStruct.begin();
                 itTo != pPatternWkStruct.end();
                 ++itTo) {
                // from and to should be different AND
                // from cannot be "inText"
                if (itFrom->first != itTo->first && itTo->first != TREEPATTERN_INTEXT) {
                    // iterate through "from" conversion trees
                    for (std::list<RootNodeWithRelatedCpts>::iterator itPatternFrom = itFrom->second.begin();
                         itPatternFrom != itFrom->second.end();
                         ++itPatternFrom) {
                        // iterate through "to" conversion trees
                        for (std::list<RootNodeWithRelatedCpts>::iterator itPatternTo = itTo->second.begin();
                             itPatternTo != itTo->second.end();
                             ++itPatternTo) {
                            if (&*itPatternFrom->rootNode != &*itPatternTo->rootNode) {
                                pRes[TreePatternEnumPair(itFrom->first, itTo->first)].emplace_back(
                                    itPatternFrom->concepts,
                                    ConversionRule(fCurrFilename,
                                                   fCurrConversionNb,
                                                   10,
                                                   itPatternFrom->rootNode,
                                                   itPatternTo->rootNode));
                            }
                        }
                    }
                }
            }
        }
    }
}

void SemExpTreeConversionDatabase::xAddConversionTreePattern(PatternWorkStruct& pPatternWkStruct,
                                                             const boost::property_tree::ptree& pTree) const {
    std::list<TreePatternConventionEnum> convEnums;

    childLoop(pTree, currAttribute, "<xmlattr>") {
        TreePatternConventionEnum convEnumVal = treePatternConventionEnum_fromStr(currAttribute.first);
        if (convEnumVal != TREEPATTERN_ENUM_TABLE_ENDFORNOCOMPILWARNING)
            convEnums.emplace_back(convEnumVal);
    }

    RootNodeWithRelatedCpts newTreePattern;
    xAddTreePattern(newTreePattern, pTree);

    for (auto it = convEnums.begin(); it != convEnums.end(); ++it)
        pPatternWkStruct[*it].push_back(newTreePattern);
}

void SemExpTreeConversionDatabase::xAddTreePattern(RootNodeWithRelatedCpts& pPatternToFill,
                                                   const boost::property_tree::ptree& pTree) const {
    pPatternToFill.rootNode = std::make_shared<SemExpTreePatternNode>();
    SemExpTreePatternNode* currNode = &*pPatternToFill.rootNode;
    for (const auto& currElt : pTree)
        if (currElt.first != "<xmlattr>")
            xAddSemExp(pPatternToFill, currNode, currElt.first, currElt.second, nullptr);
}

void SemExpTreeConversionDatabase::xAddSemExp(RootNodeWithRelatedCpts& pRootNodeWithRelatedCpts,
                                              SemExpTreePatternNode*& pCurrNode,
                                              const std::string& pBeaconName,
                                              const boost::property_tree::ptree& pTree,
                                              SemExpTreePatternNode* pFatherNode) const {
    if (pBeaconName == "root" && pFatherNode == nullptr) {
        xFillNode(pRootNodeWithRelatedCpts, *pCurrNode, pTree);
    } else {
        if (pBeaconName == "root" || pFatherNode == nullptr) {
            throw std::runtime_error("Semantic Expressions tree converter: root beacon badly placed");
        }
        GrammaticalType childGramType = grammaticalType_fromStr(pBeaconName);
        if (childGramType != GrammaticalType::UNKNOWN) {
            pFatherNode->children[childGramType] = SemExpTreePatternNode();
            pCurrNode = &pFatherNode->children[childGramType];
            xFillNode(pRootNodeWithRelatedCpts, *pCurrNode, pTree);
        } else if (pBeaconName.size() > 3 && pBeaconName.compare(0, 3, "not") == 0) {
            childGramType = grammaticalType_fromStr(pBeaconName.substr(3, pBeaconName.size() - 3));
            if (childGramType != GrammaticalType::UNKNOWN) {
                pFatherNode->notChildren.insert(childGramType);
            }
        }
    }

    SemExpTreePatternNode* fatherNode = pCurrNode;
    for (const auto& currElt : pTree)
        if (currElt.first != "<xmlattr>")
            xAddSemExp(pRootNodeWithRelatedCpts, pCurrNode, currElt.first, currElt.second, fatherNode);
}

void SemExpTreeConversionDatabase::xFillNode(RootNodeWithRelatedCpts& pRootNodeWithRelatedCpts,
                                             SemExpTreePatternNode& pCurrNode,
                                             const boost::property_tree::ptree& pTree) const {
    childLoop(pTree, currAttribute, "<xmlattr>") {
        const std::string attrName = currAttribute.first;
        std::string valStr = currAttribute.second.get_value<std::string>();
        if (attrName == "grounding") {
            pCurrNode.groundingType = semanticGroundingsType_fromStr(valStr);
        } else if (attrName == "id") {
            pCurrNode.id = valStr;
        } else if (attrName == "concept") {
            std::list<char> confidencesSpecs;
            _extractConfidenceAtEndOfStr(valStr, confidencesSpecs);
            std::vector<std::string> concepts;
            mystd::split(concepts, valStr, "|");
            for (const auto& currCpt : concepts) {
                pCurrNode.concepts[currCpt] = confidencesSpecs;
                pRootNodeWithRelatedCpts.concepts.emplace_back(currCpt);
            }
        } else if (attrName == "notConcept") {
            std::vector<std::string> concepts;
            mystd::split(concepts, valStr, "|");
            for (const auto& currCpt : concepts)
                pCurrNode.notConcepts.insert(currCpt);
        } else if (attrName == "beginOfConcept") {
            pCurrNode.beginOfConcepts.insert(valStr);
        } else if (attrName == "conceptOrHyponym") {
            std::vector<std::string> concepts;
            mystd::split(concepts, valStr, "|");
            for (const auto& currCpt : concepts)
                pCurrNode.conceptsOrHyponyms.insert(currCpt);
        } else if (attrName == "notConceptOrHyponym") {
            std::vector<std::string> concepts;
            mystd::split(concepts, valStr, "|");
            for (const auto& currCpt : concepts)
                pCurrNode.notConceptsOrHyponyms.insert(currCpt);
        } else if (attrName == "nb") {
            int value = 0;
            try {
                value = mystd::lexical_cast<int>(currAttribute.second.get_value<std::string>());
            } catch (const std::exception& e) {
                std::cerr << "Failed to convert a string to an int: " << e.what() << std::endl;
            }
            pCurrNode.nb.emplace(value);
        } else if (attrName == "request") {
            if (!pCurrNode.groundingType || pCurrNode.groundingType != SemanticGroundingType::STATEMENT) {
                xWriteErrorMsg("\"notRequest\" can only be set inside a statement grounding");
            }

            SemanticRequestType requestType = semanticRequestType_fromStr(valStr);
            if (requestType != SemanticRequestType::NOTHING) {
                pCurrNode.requests.add(requestType);
            } else {
                xWriteErrorMsg("\"" + valStr + "\" is not a valid request.");
            }
        } else if (attrName == "notRequest") {
            if (!pCurrNode.groundingType || pCurrNode.groundingType != SemanticGroundingType::STATEMENT) {
                xWriteErrorMsg("\"notRequest\" can only be set inside a statement grounding");
            }
            SemanticRequestType requestType = semanticRequestType_fromStr(valStr);
            if (requestType != SemanticRequestType::NOTHING) {
                pCurrNode.notRequests.add(requestType);
            } else {
                xWriteErrorMsg("\"" + valStr + "\" is not a valid request.");
            }
        } else if (attrName == "time") {
            if (!pCurrNode.groundingType || pCurrNode.groundingType != SemanticGroundingType::STATEMENT) {
                xWriteErrorMsg("\"time\" can only be set inside a statement grounding");
            }

            pCurrNode.time = semanticVerbTense_fromStr(valStr);
        } else if (attrName == "type") {
            if (!pCurrNode.groundingType || pCurrNode.groundingType != SemanticGroundingType::GENERIC) {
                xWriteErrorMsg("\"type\" can only be set inside a generic grounding");
            }

            pCurrNode.type = semanticEntityType_fromStr(valStr);
        } else if (attrName == "notType") {
            if (!pCurrNode.groundingType || pCurrNode.groundingType != SemanticGroundingType::GENERIC) {
                xWriteErrorMsg("\"notType\" can only be set inside a generic grounding");
            }

            SemanticEntityType typeEnum = semanticEntityType_fromStr(valStr);
            if (typeEnum != SemanticEntityType::UNKNOWN) {
                pCurrNode.notTypes.insert(typeEnum);
            } else {
                xWriteErrorMsg("\"" + valStr + "\" is not a valid type.");
            }
        } else if (attrName == "reference") {
            if (!pCurrNode.groundingType
                || (pCurrNode.groundingType != SemanticGroundingType::GENERIC
                    && pCurrNode.groundingType != SemanticGroundingType::AGENT)) {
                xWriteErrorMsg("\"reference\" can only be set inside a generic or an agent grounding");
            }
            pCurrNode.reference = semanticReferenceType_fromStr(valStr);
        } else if (attrName == "word") {
            if (!pCurrNode.groundingType
                || (pCurrNode.groundingType != SemanticGroundingType::GENERIC
                    && pCurrNode.groundingType != SemanticGroundingType::STATEMENT)) {
                xWriteErrorMsg("\"word\" can only be set inside a generic or a statement grounding");
            }

            try {
                std::vector<std::string> strs;
                mystd::split(strs, valStr, "|");
                std::size_t strs_size = strs.size();
                if (strs_size == 1) {
                    pCurrNode.word = SemanticWord();
                } else if (strs_size == 3) {
                    pCurrNode.word =
                        SemanticWord(semanticLanguageEnum_fromStr(strs[2]), strs[0], partOfSpeech_fromStr(strs[1]));
                } else {
                    xWriteErrorMsg("Wrong \"word\" attribute value: " + valStr);
                }
            } catch (const std::exception& e) {
                xWriteErrorMsg(std::string("Error detected in \"word\" attribute value: ") + e.what());
            }
        } else if (attrName == "removeWord") {
            if (!pCurrNode.groundingType
                || (pCurrNode.groundingType != SemanticGroundingType::GENERIC
                    && pCurrNode.groundingType != SemanticGroundingType::STATEMENT)) {
                xWriteErrorMsg("\"removeWord\" can only be set inside a generic or a statement grounding");
            }
            pCurrNode.removeWord.emplace(valStr == "true");
        } else if (attrName == "polarity") {
            if (!pCurrNode.groundingType || pCurrNode.groundingType != SemanticGroundingType::STATEMENT) {
                xWriteErrorMsg("\"polarity\" can only be set inside a statement grounding");
            }
            pCurrNode.polarity.emplace(valStr == "true");
        } else if (attrName == "timeType") {
            if (!pCurrNode.groundingType || (pCurrNode.groundingType != SemanticGroundingType::RELATIVETIME)) {
                xWriteErrorMsg("\"timeType\" can only be set inside a relative time grounding");
            }

            pCurrNode.timeType = semanticRelativeTimeType_fromStr(valStr);
        } else if (attrName == "hasToBeCompletedFromContext") {
            if (!pCurrNode.groundingType || (pCurrNode.groundingType != SemanticGroundingType::GENERIC)) {
                xWriteErrorMsg("\"hasToBeCompletedFromContext\" can only be set inside a generic grounding");
            }
            pCurrNode.hasToBeCompletedFromContext.emplace(valStr == "true");
        } else {
            xWriteErrorMsg("\"" + attrName + "\" is not a valid attibute");
        }
    }
}

void SemExpTreeConversionDatabase::xWriteErrorMsg(const std::string& pMsg) const {
    std::cerr << "Error in TreeConvertor:\nIn file: " << fCurrFilename << " (" << fCurrConversionNb
              << ")\nMessage: " << pMsg << std::endl;
}

}    // End of namespace onsem
