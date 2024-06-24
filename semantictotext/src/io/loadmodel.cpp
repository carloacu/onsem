#include "loadmodel.hpp"
#include <iostream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/info_parser.hpp>
#include <onsem/common/utility/lexical_cast.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpressions.hpp>
#include <onsem/texttosemantic/dbtype/semanticgroundings.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase.hpp>
#include <onsem/semantictotext/semanticmemory/links/expressionwithlinks.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemory.hpp>
#include <onsem/semantictotext/semanticmemory/links/sentencewithlinks.hpp>

namespace onsem {
namespace serializationprivate {
struct LoaderSemExpLinks {
    std::map<int, SemanticExpression*> linkToSemExpPtr{};
    std::map<int, GroundedExpression*> linkToGrdExpPtr{};

    SemanticExpression* getSemExp(mystd::optional<int> pIntLink) const {
        if (pIntLink) {
            auto itSemExp = linkToSemExpPtr.find(*pIntLink);
            assert(itSemExp != linkToSemExpPtr.end());
            return itSemExp->second;
        }
        return nullptr;
    }
    GroundedExpression* getGrdExp(int pIntLink) const {
        auto itGrdExp = linkToGrdExpPtr.find(pIntLink);
        assert(itGrdExp != linkToGrdExpPtr.end());
        return itGrdExp->second;
    }
};
std::unique_ptr<SemanticExpression> _loadSemExp(const boost::property_tree::ptree& pTree, LoaderSemExpLinks* pLinks);

static const std::string sourceEnumDefaultStr = semanticSourceEnum_toStr(SemanticSourceEnum::UNKNOWN);
static const std::string contextualAnnotationDefaultStr = contextualAnnotation_toStr(ContextualAnnotation::PROACTIVE);
static const std::string languageDefaultStr = semanticLanguageEnum_toStr(SemanticLanguageEnum::UNKNOWN);
static const std::string listExpTypeDefaultStr = listExpressionType_toStr(ListExpressionType::UNRELATED);
static const std::string semanticRefTypeDefaultStr = semanticReferenceType_toStr(SemanticReferenceType::UNDEFINED);
static const std::string agentTypeDefaultStr = semanticEntityType_toStr(SemanticEntityType::UNKNOWN);
static const std::string semQuantityTypeDefaultStr = semanticQuantityType_toStr(SemanticQuantityType::UNKNOWN);
static const std::string semanticSubjectiveQuantityDefaultStr =
    semanticSubjectiveQuantity_toStr(SemanticSubjectiveQuantity::UNKNOWN);
static const std::string partOfSpeechDefaultStr = partOfSpeech_toStr(PartOfSpeech::UNKNOWN);
static const std::string verbTenseDefaultStr = semanticVerbTense_toStr(SemanticVerbTense::UNKNOWN);
static const std::string verbTensePresentStr = semanticVerbTense_toStr(SemanticVerbTense::PRESENT);
static const std::string requestTypeDefaultStr = semanticRequestType_toStr(SemanticRequestType::NOTHING);
static const std::string verbGoalDefaultStr = semVerbGoalEnum_toStr(VerbGoalEnum::NOTIFICATION);
static const std::string genderDefaultStr = semanticGenderType_toStr(SemanticGenderType::UNKNOWN);
static const std::string numberDefaultStr = semanticNumberType_toStr(SemanticNumberType::UNKNOWN);
static const std::string relativePersonDefaultStr = relativePerson_toStr(RelativePerson::UNKNOWN);
static const std::string relativePersonWithoutNumberDefaultStr =
    relativePersonWithoutNumber_toStr(RelativePersonWithoutNumber::UNKNOWN);
static const std::string lingVerbTenseInfinitiveStr = linguisticVerbTense_toStr(LinguisticVerbTense::INFINITIVE);
static const std::string semExpTypeLabelStr = "type";
static const std::string serializationLinkStr = "serializationLink";
static const std::string fromTextIntroductionStr = "fromText_introduction";
static const std::string fromTextContentStr = "fromText_content";
static const std::string falseStr = "False";
static const std::string trueStr = "True";

bool _isASemExpLabel(const std::string& pLabel) {
    return pLabel == semExpTypeLabelStr || pLabel == serializationLinkStr ||
            pLabel == fromTextIntroductionStr || pLabel == fromTextContentStr;
}

template<typename T>
mystd::optional<T> _boostOptToMyStdOpt(boost::optional<T> pOpt) {
    mystd::optional<T> res;
    if (pOpt)
        res.emplace(*pOpt);
    return res;
}

mystd::unique_propagate_const<UniqueSemanticExpression> _loadOptionalSemExp(const boost::property_tree::ptree& pTree,
                                                                            const std::string& pPath,
                                                                            LoaderSemExpLinks* pLinks) {
    auto optTree = pTree.get_child_optional(pPath);
    if (optTree)
        return mystd::unique_propagate_const<UniqueSemanticExpression>(_loadSemExp(*optTree, pLinks));
    return mystd::unique_propagate_const<UniqueSemanticExpression>();
}

// this "if" is needed otherwise we have a crash on mac if we try to iterate on an empty tree
#define childLoop(TREE, ELT, LABEL)                    \
    auto optChildren = TREE.get_child_optional(LABEL); \
    if (optChildren)                                   \
        for (const auto& ELT : *optChildren)

void _loadConcepts(std::map<std::string, char>& pConcepts,
                   const boost::property_tree::ptree& pTree,
                   const std::string& pConceptsLabel = "concepts") {
    childLoop(pTree, currCpt, pConceptsLabel) pConcepts.emplace(currCpt.first.data(), currCpt.second.get_value<char>());
}

void _loadGrd(SemanticGrounding& pGrd, const boost::property_tree::ptree& pTree) {
    pGrd.polarity = pTree.get<bool>("polarity", true);
    _loadConcepts(pGrd.concepts, pTree);
}

void _loadNameInfos(NameInfos& pNameInfos, const boost::property_tree::ptree& pTree) {
    const boost::property_tree::ptree& namesChild = pTree.get_child("names");
    pNameInfos.names.reserve(10);
    if (!namesChild.empty())
        for (const auto& currName : namesChild)
            pNameInfos.names.emplace_back(currName.second.get_value<std::string>());
    childLoop(pTree, currGender, "possibleGenders")
        pNameInfos.possibleGenders.insert(semanticGenderType_fromStr(currGender.second.get_value<std::string>()));
}

std::unique_ptr<SemanticNameGrounding> _loadNameGrd(const boost::property_tree::ptree& pTree) {
    auto res = std::make_unique<SemanticNameGrounding>();
    _loadGrd(*res, pTree);
    _loadNameInfos(res->nameInfos, pTree);
    return res;
}

std::unique_ptr<SemanticPercentageGrounding> _loadPercentageGrd(const boost::property_tree::ptree& pTree) {
    auto res = std::make_unique<SemanticPercentageGrounding>();
    _loadGrd(*res, pTree);
    res->value.fromStr(pTree.get("value", ""));
    return res;
}

std::unique_ptr<SemanticAgentGrounding> _loadAgentGrd(const boost::property_tree::ptree& pTree) {
    auto userId = pTree.get("userId", SemanticAgentGrounding::userNotIdentified);
    auto userIdWithoutContext = pTree.get("userIdWithoutContext", userId);
    auto res = std::make_unique<SemanticAgentGrounding>(userId, userIdWithoutContext);
    res->concepts.clear();
    _loadGrd(*res, pTree);
    auto nameGrdTreeOpt = pTree.get_child_optional("nameInfos");
    if (nameGrdTreeOpt) {
        res->nameInfos.emplace();
        _loadNameInfos(*res->nameInfos, *nameGrdTreeOpt);
    }
    return res;
}

void _loadSemanticAngle(SemanticAngle& pSemanticAngle, const boost::property_tree::ptree& pTree) {
    if (!pTree.empty())
        for (const auto& currAngleInfo : pTree) {
            const std::string label = currAngleInfo.first.data();
            pSemanticAngle.angleInfos[semanticAngleUnity_fromAbreviation(label)].fromStr(
                currAngleInfo.second.get_value<std::string>());
        }
}

void _loadSemanticLength(SemanticLength& pSemanticLength, const boost::property_tree::ptree& pTree) {
    if (!pTree.empty())
        for (const auto& currLengthInfo : pTree) {
            const std::string label = currLengthInfo.first.data();
            pSemanticLength.lengthInfos[semanticLengthUnity_fromAbreviation(label)].fromStr(
                currLengthInfo.second.get_value<std::string>());
        }
}

void _loadSemanticDuration(SemanticDuration& pSemanticDuration, const boost::property_tree::ptree& pTree) {
    pSemanticDuration.sign = Sign::POSITIVE;
    if (!pTree.empty())
        for (const auto& currTimeInfo : pTree) {
            const std::string label = currTimeInfo.first.data();
            if (label == "sign")
                pSemanticDuration.sign = Sign::NEGATIVE;
            else
                pSemanticDuration.timeInfos[semanticTimeUnity_fromStr(label)].fromStr(
                    currTimeInfo.second.get_value<std::string>());
        }
}

void _loadSemanticDate(SemanticDate& pSemanticDate, const boost::property_tree::ptree& pTree) {
    pSemanticDate.year = _boostOptToMyStdOpt(pTree.get_optional<int>("year"));
    pSemanticDate.month = _boostOptToMyStdOpt(pTree.get_optional<int>("month"));
    pSemanticDate.day = _boostOptToMyStdOpt(pTree.get_optional<int>("day"));
}

std::unique_ptr<SemanticTimeGrounding> _loadTimeGrd(const boost::property_tree::ptree& pTree) {
    auto res = std::make_unique<SemanticTimeGrounding>();
    SemanticTimeGrounding& timeGrd = *res;
    _loadGrd(timeGrd, pTree);
    _loadSemanticDate(timeGrd.date, pTree.get_child("date"));
    _loadSemanticDuration(timeGrd.timeOfDay, pTree.get_child("timeOfDay"));
    _loadSemanticDuration(timeGrd.length, pTree.get_child("length"));
    _loadConcepts(timeGrd.fromConcepts, pTree, "fromConcepts");
    return res;
}

void _loadSemanticQuantity(SemanticQuantity& pSemanticQuantity, const boost::property_tree::ptree& pTree) {
    pSemanticQuantity.type = semanticQuantityType_fromStr(pTree.get("type", semQuantityTypeDefaultStr));
    pSemanticQuantity.nb.fromStr(pTree.get("nb", ""));
    pSemanticQuantity.paramSpec = pTree.get("paramSpec", "");
    pSemanticQuantity.subjectiveValue =
        semanticSubjectiveQuantity_fromStr(pTree.get("subjectiveValue", semanticSubjectiveQuantityDefaultStr));
}

void _loadSemanticWord(SemanticWord& pSemanticWord, const boost::property_tree::ptree& pTree) {
    pSemanticWord.language = semanticLanguageEnum_fromStr(pTree.get("language", languageDefaultStr));
    pSemanticWord.lemma = pTree.get("lemma", "");
    pSemanticWord.partOfSpeech = partOfSpeech_fromStr(pTree.get("partOfSpeech", partOfSpeechDefaultStr));
}

std::unique_ptr<SemanticGenericGrounding> _loadGenericGrd(const boost::property_tree::ptree& pTree) {
    auto res = std::make_unique<SemanticGenericGrounding>();
    _loadGrd(*res, pTree);
    res->referenceType = semanticReferenceType_fromStr(pTree.get("referenceType", semanticRefTypeDefaultStr));
    auto optCoreference = pTree.get_child_optional("coreference");
    if (optCoreference)
        res->coreference.emplace(coreferenceDirectionEnum_fromStr(optCoreference->get_value<std::string>()));
    res->entityType = semanticEntityType_fromStr(pTree.get("entityType", agentTypeDefaultStr));
    _loadSemanticQuantity(res->quantity, pTree.get_child("quantity", {}));
    _loadSemanticWord(res->word, pTree.get_child("word", {}));
    childLoop(pTree, currGender, "possibleGenders")
        res->possibleGenders.insert(semanticGenderType_fromStr(currGender.second.get_value<std::string>()));
    return res;
}

std::unique_ptr<SemanticStatementGrounding> _loadStatementGrd(const boost::property_tree::ptree& pTree) {
    auto res = std::make_unique<SemanticStatementGrounding>();
    _loadGrd(*res, pTree);
    childLoop(pTree, currReq, "requestTypes")
        res->requests.addWithoutCollisionCheck(semanticRequestType_fromStr(currReq.second.get_value<std::string>()));
    _loadSemanticWord(res->word, pTree.get_child("word", {}));
    res->verbTense = semanticVerbTense_fromStr(pTree.get("verbTense", verbTenseDefaultStr));
    res->verbGoal = semVerbGoalEnum_fromStr(pTree.get("verbGoal", verbGoalDefaultStr));
    auto optCoreference = pTree.get_child_optional("coreference");
    if (optCoreference)
        res->coreference.emplace(coreferenceDirectionEnum_fromStr(optCoreference->get_value<std::string>()));
    auto optIsPassive = pTree.get_child_optional("isPassive");
    if (optIsPassive)
        res->isPassive.emplace(optIsPassive);
    return res;
}

std::unique_ptr<SemanticTextGrounding> _loadTextGrd(const boost::property_tree::ptree& pTree) {
    auto res = std::make_unique<SemanticTextGrounding>(pTree.get<std::string>("text"));
    res->concepts.clear();
    _loadGrd(*res, pTree);
    res->forLanguage = semanticLanguageEnum_fromStr(pTree.get("forLanguage", languageDefaultStr));
    res->hasQuotationMark = pTree.get("hasQuotationMark", falseStr) == trueStr;
    return res;
}

std::unique_ptr<SemanticAngleGrounding> _loadAngleGrd(const boost::property_tree::ptree& pTree) {
    auto res = std::make_unique<SemanticAngleGrounding>();
    _loadGrd(*res, pTree);
    _loadSemanticAngle(res->angle, pTree.get_child("angle"));
    return res;
}

std::unique_ptr<SemanticLengthGrounding> _loadLengthGrd(const boost::property_tree::ptree& pTree) {
    auto res = std::make_unique<SemanticLengthGrounding>();
    _loadGrd(*res, pTree);
    _loadSemanticLength(res->length, pTree.get_child("length"));
    return res;
}

std::unique_ptr<SemanticDurationGrounding> _loadDurationGrd(const boost::property_tree::ptree& pTree) {
    auto res = std::make_unique<SemanticDurationGrounding>();
    _loadGrd(*res, pTree);
    _loadSemanticDuration(res->duration, pTree.get_child("duration"));
    return res;
}

std::unique_ptr<SemanticLanguageGrounding> _loadLanguageGrd(const boost::property_tree::ptree& pTree) {
    auto res = std::make_unique<SemanticLanguageGrounding>(
        semanticLanguageEnum_fromStr(pTree.get("language", languageDefaultStr)));
    _loadGrd(*res, pTree);
    return res;
}

std::unique_ptr<SemanticRelativeLocationGrounding> _loadRelLocationGrd(const boost::property_tree::ptree& pTree) {
    auto res = std::make_unique<SemanticRelativeLocationGrounding>(
        semanticRelativeLocationType_fromStr(pTree.get<std::string>("locationType")));
    _loadGrd(*res, pTree);
    return res;
}

std::unique_ptr<SemanticRelativeTimeGrounding> _loadRelTimeGrd(const boost::property_tree::ptree& pTree) {
    auto res = std::make_unique<SemanticRelativeTimeGrounding>(
        semanticRelativeTimeType_fromStr(pTree.get<std::string>("timeType")));
    _loadGrd(*res, pTree);
    return res;
}

std::unique_ptr<SemanticRelativeDurationGrounding> _loadRelDurationGrd(const boost::property_tree::ptree& pTree) {
    auto res = std::make_unique<SemanticRelativeDurationGrounding>(
        semanticRelativeDurationType_fromStr(pTree.get<std::string>("durationType")));
    _loadGrd(*res, pTree);
    return res;
}

std::unique_ptr<SemanticResourceGrounding> _loadResourceGrd(const boost::property_tree::ptree& pTree) {
    auto res = std::make_unique<SemanticResourceGrounding>(
        pTree.get<std::string>("label"),
        semanticLanguageEnum_fromStr(pTree.get("language", languageDefaultStr)),
        pTree.get<std::string>("value"));
    _loadGrd(*res, pTree);
    return res;
}

std::unique_ptr<SemanticMetaGrounding> _loadMetaGrd(const boost::property_tree::ptree& pTree) {
    auto res =
        std::make_unique<SemanticMetaGrounding>(semanticGroundingsType_fromStr(pTree.get<std::string>("refToType")),
                                                pTree.get<int>("paramId"),
                                                pTree.get<std::string>("attibuteName"),
                                                pTree.get<std::string>("attibuteValue"));
    _loadGrd(*res, pTree);
    return res;
}

std::unique_ptr<SemanticConceptualGrounding> _loadConceptualGrd(const boost::property_tree::ptree& pTree) {
    auto res = std::make_unique<SemanticConceptualGrounding>();
    _loadGrd(*res, pTree);
    return res;
}

std::unique_ptr<SemanticUnityGrounding> _loadUnityGrd(const boost::property_tree::ptree& pTree) {
    auto res = std::make_unique<SemanticUnityGrounding>(typeOfUnity_fromStr(pTree.get<std::string>("typeOfUnity")),
                                                        pTree.get<std::string>("value"));
    _loadGrd(*res, pTree);
    return res;
}

std::unique_ptr<SemanticGrounding> _loadGrounding(const boost::property_tree::ptree& pTree) {
    switch (semanticGroundingsType_fromStr(
        pTree.get<std::string>("type", semanticGroundingsType_toStr(SemanticGroundingType::GENERIC)))) {
        case SemanticGroundingType::AGENT: return _loadAgentGrd(pTree);
        case SemanticGroundingType::ANGLE: return _loadAngleGrd(pTree);
        case SemanticGroundingType::GENERIC: return _loadGenericGrd(pTree);
        case SemanticGroundingType::STATEMENT: return _loadStatementGrd(pTree);
        case SemanticGroundingType::TIME: return _loadTimeGrd(pTree);
        case SemanticGroundingType::TEXT: return _loadTextGrd(pTree);
        case SemanticGroundingType::DURATION: return _loadDurationGrd(pTree);
        case SemanticGroundingType::LANGUAGE: return _loadLanguageGrd(pTree);
        case SemanticGroundingType::RELATIVELOCATION: return _loadRelLocationGrd(pTree);
        case SemanticGroundingType::RELATIVETIME: return _loadRelTimeGrd(pTree);
        case SemanticGroundingType::RELATIVEDURATION: return _loadRelDurationGrd(pTree);
        case SemanticGroundingType::RESOURCE: return _loadResourceGrd(pTree);
        case SemanticGroundingType::LENGTH: return _loadLengthGrd(pTree);
        case SemanticGroundingType::META: return _loadMetaGrd(pTree);
        case SemanticGroundingType::NAME: return _loadNameGrd(pTree);
        case SemanticGroundingType::PERCENTAGE: return _loadPercentageGrd(pTree);
        case SemanticGroundingType::CONCEPTUAL: return _loadConceptualGrd(pTree);
        case SemanticGroundingType::UNITY: return _loadUnityGrd(pTree);
    }
    assert(false);
    return std::make_unique<SemanticConceptualGrounding>();
}

std::unique_ptr<GroundedExpression> _loadGrdExp(const boost::property_tree::ptree& pTree, LoaderSemExpLinks* pLinks) {
    auto res = std::make_unique<GroundedExpression>(_loadGrounding(pTree.get_child("grounding")));
    if (!pTree.empty())
        for (const auto& currChild : pTree)
            if (!_isASemExpLabel(currChild.first) && currChild.first != "grounding")
                res->children.emplace(grammaticalType_fromStr(currChild.first), _loadSemExp(currChild.second, pLinks));
    return res;
}

std::unique_ptr<ListExpression> _loadListExp(const boost::property_tree::ptree& pTree, LoaderSemExpLinks* pLinks) {
    auto res =
        std::make_unique<ListExpression>(listExpressionType_fromStr(pTree.get("listType", listExpTypeDefaultStr)));
    if (!pTree.empty())
        for (const auto& currElt : pTree)
            if (currElt.first == "e")
                res->elts.emplace_back(_loadSemExp(currElt.second, pLinks));
    return res;
}

std::unique_ptr<ConditionExpression> _loadConditionExp(const boost::property_tree::ptree& pTree,
                                                       LoaderSemExpLinks* pLinks) {
    auto res = std::make_unique<ConditionExpression>(pTree.get("isAlwaysActive", falseStr) == trueStr,
                                                     pTree.get("conditionPointsToAffirmations", falseStr) == trueStr,
                                                     _loadSemExp(pTree.get_child("conditionExp"), pLinks),
                                                     _loadSemExp(pTree.get_child("thenExp"), pLinks));
    res->elseExp = _loadOptionalSemExp(pTree, "elseExp", pLinks);
    return res;
}

std::unique_ptr<ComparisonExpression> _loadComparisonExp(const boost::property_tree::ptree& pTree,
                                                         LoaderSemExpLinks* pLinks) {
    auto res = std::make_unique<ComparisonExpression>(ComparisonOperator_fromStr(pTree.get<std::string>("op")),
                                                      _loadSemExp(pTree.get_child("leftOperandExp"), pLinks));
    res->tense = semanticVerbTense_fromStr(pTree.get("tense", verbTensePresentStr));
    res->request = semanticRequestType_fromStr(pTree.get("request", requestTypeDefaultStr));
    res->whatIsComparedExp = _loadOptionalSemExp(pTree, "whatIsComparedExp", pLinks);
    res->rightOperandExp = _loadOptionalSemExp(pTree, "rightOperandExp", pLinks);
    return res;
}

std::unique_ptr<InterpretationExpression> _loadInterpretationExp(const boost::property_tree::ptree& pTree,
                                                                 LoaderSemExpLinks* pLinks) {
    return std::make_unique<InterpretationExpression>(interpretationFrom_fromStr(pTree.get<std::string>("source")),
                                                      _loadSemExp(pTree.get_child("interpretedExp"), pLinks),
                                                      _loadSemExp(pTree.get_child("originalExp"), pLinks));
}

std::unique_ptr<FeedbackExpression> _loadFeedbackExp(const boost::property_tree::ptree& pTree,
                                                     LoaderSemExpLinks* pLinks) {
    return std::make_unique<FeedbackExpression>(_loadSemExp(pTree.get_child("feedbackExp"), pLinks),
                                                _loadSemExp(pTree.get_child("concernedExp"), pLinks));
}

std::unique_ptr<AnnotatedExpression> _loadAnnotatedExp(const boost::property_tree::ptree& pTree,
                                                       LoaderSemExpLinks* pLinks) {
    auto res = std::make_unique<AnnotatedExpression>(_loadSemExp(pTree.get_child("semExp"), pLinks));
    res->synthesizeAnnotations = pTree.get<bool>("synthesizeAnnotations", false);
    if (!pTree.empty())
        for (const auto& currChild : pTree)
            if (!_isASemExpLabel(currChild.first) && currChild.first != "semExp"
                && currChild.first != "synthesizeAnnotations")
                res->annotations.emplace(grammaticalType_fromStr(currChild.first),
                                         _loadSemExp(currChild.second, pLinks));
    return res;
}

std::unique_ptr<MetadataExpression> _loadMetadataExp(const boost::property_tree::ptree& pTree,
                                                     LoaderSemExpLinks* pLinks) {
    auto res = std::make_unique<MetadataExpression>(_loadSemExp(pTree.get_child("semExp"), pLinks));
    res->from = semanticSourceEnum_fromStr(pTree.get("from", sourceEnumDefaultStr));
    res->contextualAnnotation =
        contextualAnnotation_fromStr(pTree.get("contextualAnnotation", contextualAnnotationDefaultStr));
    res->fromLanguage = semanticLanguageEnum_fromStr(pTree.get("fromLanguage", languageDefaultStr));
    res->confidence = pTree.get<unsigned char>("confidence", 100u);
    { childLoop(pTree, currRef, "references") res->references.emplace_back(currRef.second.data()); }
    res->source = _loadOptionalSemExp(pTree, "source", pLinks);
    return res;
}

std::unique_ptr<SetOfFormsExpression> _loadSetOfFormsExp(const boost::property_tree::ptree& pTree,
                                                         LoaderSemExpLinks* pLinks) {
    auto res = std::make_unique<SetOfFormsExpression>();
    if (!pTree.empty())
        for (const auto& currElt : pTree) {
            if (!_isASemExpLabel(currElt.first)) {
                int prio = mystd::lexical_cast<int>(currElt.first);
                res->prioToForms[prio].emplace_back(
                    std::make_unique<QuestExpressionFrom>(_loadSemExp(currElt.second.get_child("exp"), pLinks),
                                                          currElt.second.get("isOriginalForm", falseStr) == trueStr));
            }
        }
    return res;
}

std::unique_ptr<CommandExpression> _loadCommandExp(const boost::property_tree::ptree& pTree,
                                                   LoaderSemExpLinks* pLinks) {
    auto res = std::make_unique<CommandExpression>(_loadSemExp(pTree.get_child("semExp"), pLinks));
    res->description = _loadOptionalSemExp(pTree, "description", pLinks);
    return res;
}

std::unique_ptr<FixedSynthesisExpression> _loadFSynthExp(const boost::property_tree::ptree& pTree,
                                                         LoaderSemExpLinks* pLinks) {
    auto res = std::make_unique<FixedSynthesisExpression>(_loadSemExp(pTree.get_child("semExp"), pLinks));
    auto langToSynthesisOptTree = pTree.get_child_optional("langToSynthesis");
    if (langToSynthesisOptTree)
        for (const auto& currLangToSynthTree : *langToSynthesisOptTree)
            res->langToSynthesis.emplace(semanticLanguageEnum_fromStr(currLangToSynthTree.first),
                                         currLangToSynthTree.second.get_value<std::string>());
    return res;
}

std::unique_ptr<SemanticExpression> _loadSemExp(const boost::property_tree::ptree& pTree, LoaderSemExpLinks* pLinks) {
    int serialoizationLink = 0;
    if (pLinks != nullptr) {
        serialoizationLink = pTree.get<int>(serializationLinkStr);
    }

    auto res = [&]() -> std::unique_ptr<SemanticExpression> {
        switch (semanticExpressionType_fromStr(pTree.get<std::string>(
            semExpTypeLabelStr, semanticExpressionType_toStr(SemanticExpressionType::GROUNDED)))) {
            case SemanticExpressionType::GROUNDED: {
                auto grdExpRes = _loadGrdExp(pTree, pLinks);
                if (pLinks != nullptr)
                    pLinks->linkToGrdExpPtr.emplace(serialoizationLink, &*grdExpRes);
                return grdExpRes;
            }
            case SemanticExpressionType::LIST: return _loadListExp(pTree, pLinks);
            case SemanticExpressionType::CONDITION: return _loadConditionExp(pTree, pLinks);
            case SemanticExpressionType::COMPARISON: return _loadComparisonExp(pTree, pLinks);
            case SemanticExpressionType::INTERPRETATION: return _loadInterpretationExp(pTree, pLinks);
            case SemanticExpressionType::FEEDBACK: return _loadFeedbackExp(pTree, pLinks);
            case SemanticExpressionType::ANNOTATED: return _loadAnnotatedExp(pTree, pLinks);
            case SemanticExpressionType::METADATA: return _loadMetadataExp(pTree, pLinks);
            case SemanticExpressionType::SETOFFORMS: return _loadSetOfFormsExp(pTree, pLinks);
            case SemanticExpressionType::COMMAND: return _loadCommandExp(pTree, pLinks);
            case SemanticExpressionType::FIXEDSYNTHESIS: return _loadFSynthExp(pTree, pLinks);
        }
        assert(false);
        return std::make_unique<GroundedExpression>();
    }();

    if (pLinks != nullptr)
        pLinks->linkToSemExpPtr.emplace(serialoizationLink, &*res);
    res->fromText.introduction = pTree.get(fromTextIntroductionStr, "");
    res->fromText.content = pTree.get(fromTextContentStr, "");
    return res;
}

UniqueSemanticExpression loadSemExp(const boost::property_tree::ptree& pTree) {
    return _loadSemExp(pTree.get_child("semExp"), nullptr);
}

void _loadContextAxiom(const boost::property_tree::ptree& pTree,
                       SentenceWithLinks& pContextAxiom,
                       const LoaderSemExpLinks& pLinks,
                       const linguistics::LinguisticDatabase& pLingDb) {
    const auto& triggerAxiomIdOpt = pTree.get_child_optional("triggerAxiomId");
    if (triggerAxiomIdOpt) {
        const auto& triggerAxiomId = *triggerAxiomIdOpt;
        pContextAxiom.triggerAxiomId.nbOfAxioms = triggerAxiomId.get("nbOfAxioms", 1);
        pContextAxiom.triggerAxiomId.idOfAxiom = triggerAxiomId.get("idOfAxiom", 0);
        pContextAxiom.triggerAxiomId.listExpType =
            listExpressionType_fromStr(triggerAxiomId.get("listExpType", listExpTypeDefaultStr));
    }
    pContextAxiom.semExpToDoIsAlwaysActive = pTree.get("semExpToDoIsAlwaysActive", falseStr) == trueStr;
    pContextAxiom.semExpToDo = pLinks.getSemExp(_boostOptToMyStdOpt(pTree.get_optional<int>("semExpToDo")));
    pContextAxiom.semExpToDoElse = pLinks.getSemExp(_boostOptToMyStdOpt(pTree.get_optional<int>("semExpToDoElse")));
    pContextAxiom.infCommandToDo = pLinks.getSemExp(_boostOptToMyStdOpt(pTree.get_optional<int>("infCommandToDo")));
    const boost::property_tree::ptree& memorySentencesChild = pTree.get_child("memorySentences");
    static const std::string andOrStr = "and_or";
    pContextAxiom.memorySentences.and_or = memorySentencesChild.get(andOrStr, falseStr) == trueStr;
    if (!memorySentencesChild.empty())
        for (const auto& currMemSent : memorySentencesChild) {
            if (currMemSent.first != andOrStr) {
                const boost::property_tree::ptree& semMemTree = currMemSent.second;
                std::map<GrammaticalType, const SemanticExpression*> annotations;
                childLoop(semMemTree, currAnnotationTree, "annotations")
                    annotations.emplace(grammaticalType_fromStr(currAnnotationTree.first),
                                        pLinks.getSemExp(currAnnotationTree.second.get_value<int>()));
                pContextAxiom.memorySentences.elts.emplace_back(
                    pContextAxiom,
                    *pLinks.getGrdExp(semMemTree.get<int>("grdExp")),
                    semMemTree.get("gatherAllTheLinks", falseStr) == trueStr,
                    annotations,
                    semMemTree.get("isATrigger", falseStr) == trueStr,
                    pLingDb,
                    semMemTree.get("isAConditionToSatisfy", falseStr) == trueStr,
                    semMemTree.get("isEnabled", falseStr) == trueStr,
                    semMemTree.get<intSemId>("id"));
            }
        }
}

std::unique_ptr<ExpressionWithLinks> _loadExpressionWithLinks(const boost::property_tree::ptree& pTree,
                                                              SemanticMemoryBlock& pParentMemoryBlock,
                                                              const linguistics::LinguisticDatabase& pLingDb) {
    LoaderSemExpLinks links;
    auto res =
        std::make_unique<ExpressionWithLinks>(pParentMemoryBlock, _loadSemExp(pTree.get_child("semExp"), &links));
    { childLoop(pTree, currLI, "linkedInfos") res->linkedInfos.emplace(currLI.first, currLI.second.data()); }
    childLoop(pTree, currContextAxiom, "contextAxioms") {
        const boost::property_tree::ptree& childTree = currContextAxiom.second;
        res->contextAxioms.emplace_back(informationType_fromStr(childTree.get("informationType", "information")), *res);
        SentenceWithLinks& newContextAxiom = res->contextAxioms.back();
        _loadContextAxiom(childTree, newContextAxiom, links, pLingDb);
    }
    res->outputToAnswerIfTriggerHasMatched = _loadOptionalSemExp(pTree, "outputToAnswerIfTriggerHasMatched", nullptr);
    return res;
}

void _loadMemoryBlock(const boost::property_tree::ptree& pTree,
                      SemanticMemoryBlock& pMemoryBlock,
                      const linguistics::LinguisticDatabase& pLingDb) {
    childLoop(pTree, currKnowledgeMemory, "expressionsMemories") {
        pMemoryBlock.addExpHandleInMemory(
            _loadExpressionWithLinks(currKnowledgeMemory.second, pMemoryBlock, pLingDb), pLingDb, nullptr);
    }
    pMemoryBlock.maxNbOfExpressionsInAMemoryBlock = pTree.get<std::size_t>("maxNbOfKnowledgesInAMemoryBloc");

    auto optFallsbacksBlock = pTree.get_child_optional("fallbacksBlock");
    if (optFallsbacksBlock) {
        pMemoryBlock.ensureFallbacksBlock();
        _loadMemoryBlock(*optFallsbacksBlock, *pMemoryBlock.getFallbacksBlockPtr(), pLingDb);
    }
}

void _loadProativeSpecifications(const boost::property_tree::ptree& pTree,
                                 ProativeSpecifications& pProativeSpecifications) {
    pProativeSpecifications.informTheUserHowToTeachMe = pTree.get<bool>("informTheUserHowToTeachMe");
}

void _loadSemMemory(const boost::property_tree::ptree& pTree,
                    SemanticMemory& pSemMemory,
                    const linguistics::LinguisticDatabase& pLingDb) {
    _loadMemoryBlock(pTree.get_child("memBloc"), pSemMemory.memBloc, pLingDb);
    pSemMemory.defaultLanguage = semanticLanguageEnum_fromStr(pTree.get("defaultLanguage", languageDefaultStr));
    _loadProativeSpecifications(pTree.get_child("proativeSpecifications"), pSemMemory.proativeSpecifications);
    childLoop(pTree, currNewUserFocusedToSemExps, "newUserFocusedToSemExps") {
        const std::string& userId = currNewUserFocusedToSemExps.first;
        for (const auto& currSemExp : currNewUserFocusedToSemExps.second) {
            LoaderSemExpLinks links;
            pSemMemory.addNewUserFocusedToSemExp(_loadSemExp(currSemExp.second, &links), userId);
        }
    }
    pSemMemory.setCurrUserId(pTree.get("currUserId", SemanticAgentGrounding::currentUser));
}

void loadSemMemory(const boost::property_tree::ptree& pTree,
                   SemanticMemory& pSemMemory,
                   const linguistics::LinguisticDatabase& pLingDb) {
    _loadSemMemory(pTree.get_child("semanticMemory"), pSemMemory, pLingDb);
}

void _loadConceptSet(const boost::property_tree::ptree& pTree, ConceptSet& pConceptSet) {
    {
        childLoop(pTree, currConcept, "localConcepts")
            pConceptSet.addConcept(currConcept.second.get_value<std::string>());
    }
    childLoop(pTree, currOppConcepts, "oppositeConcepts") for (const auto& currOppCpts : currOppConcepts.second)
        pConceptSet.notifyOppositeConcepts(currOppConcepts.first, currOppCpts.second.get_value<std::string>());
}

std::unique_ptr<SemanticWord> _loadOptSemanticWord(const boost::property_tree::ptree& pTree,
                                                   const std::string& pLabelStr) {
    std::unique_ptr<SemanticWord> res;
    auto optWordTree = pTree.get_child_optional(pLabelStr);
    if (optWordTree) {
        res = std::make_unique<SemanticWord>();
        _loadSemanticWord(*res, *optWordTree);
    }
    return res;
}

linguistics::LingWordsGroup _loadLingWordsGroup(const boost::property_tree::ptree& pTree) {
    linguistics::LingWordsGroup res(_loadOptSemanticWord(pTree, "linkedMeanings"));
    childLoop(pTree, currLinkedMeaning, "linkedMeanings") {
        const boost::property_tree::ptree& subTree = currLinkedMeaning.second;
        res.linkedMeanings.emplace_back(_loadOptSemanticWord(subTree, "word"),
                                        linkedMeaningDirection_fromStr(subTree.get<std::string>("direction")));
    }
    return res;
}

void _loadWordAssociatedInfos(const boost::property_tree::ptree& pTree, linguistics::WordAssociatedInfos& pInfos) {
    _loadConcepts(pInfos.concepts, pTree);
    {
        childLoop(pTree, currContextualInfo, "contextualInfos") pInfos.contextualInfos.insert(
            wordContextualInfos_fromStr(currContextualInfo.second.get_value<std::string>()));
    }
    auto optLinkedMeaning = pTree.get_child_optional("linkedMeanings");
    if (optLinkedMeaning)
        pInfos.linkedMeanings = _loadLingWordsGroup(*optLinkedMeaning);
    childLoop(pTree, currMetaMeaning, "metaMeanings")
        pInfos.metaMeanings.emplace_back(_loadLingWordsGroup(currMetaMeaning.second));
}

void _loadInflectedFormInfos(const boost::property_tree::ptree& pTree,
                             const SemanticWord& pWord,
                             linguistics::SpecificLinguisticDatabase& pSpecLingDb) {
    for (const auto& currElt : pTree) {
        const boost::property_tree::ptree& subTree = currElt.second;
        std::string inflectedFrom = subTree.get("inflectedFrom", pWord.lemma);
        std::unique_ptr<Inflections> inflections;
        auto optInflectionsChild = subTree.get_child_optional("inflections");
        if (optInflectionsChild) {
            const auto& inflectionsTree = optInflectionsChild->front();
            inflections = Inflections::create(inflectionType_fromStr(inflectionsTree.first),
                                              inflectionsTree.second.get_value<std::string>());
        }
        if (!inflections)
            inflections = std::make_unique<EmptyInflections>();
        char frequency = subTree.get<char>("frequency", 1u);
        pSpecLingDb.addInflectedWord(inflectedFrom, pWord, std::move(inflections), frequency);
    }
}

void _loadSpecificLingDatabase(const boost::property_tree::ptree& pTree,
                               linguistics::SpecificLinguisticDatabase& pSpecLingDb) {
    childLoop(pTree, currWordToIgram, "wordToSavedInfos") {
        const boost::property_tree::ptree& subTree = currWordToIgram.second;
        SemanticWord newWord;
        _loadSemanticWord(newWord, subTree.get_child("word"));
        linguistics::WordAssociatedInfos wordWithAssocInfos;
        _loadWordAssociatedInfos(subTree.get_child("infos"), wordWithAssocInfos);
        pSpecLingDb.addInfosToAWord(newWord, wordWithAssocInfos);
        _loadInflectedFormInfos(subTree.get_child("inflectedFormInfos"), newWord, pSpecLingDb);
    }
}

void _loadTranslationDictionary(const boost::property_tree::ptree& pTree,
                                linguistics::TranslationDictionary& pTransDic) {
    childLoop(pTree, currTrans, "translations") {
        const boost::property_tree::ptree& subTree = currTrans.second;
        SemanticWord inWord;
        _loadSemanticWord(inWord, subTree.get_child("inWord"));
        SemanticWord outWord;
        _loadSemanticWord(outWord, subTree.get_child("outWord"));
        pTransDic.addTranslation(inWord, outWord);
    }
}

void _loadLingDatabase(const boost::property_tree::ptree& pTree, linguistics::LinguisticDatabase& pLingDb) {
    _loadConceptSet(pTree.get_child("conceptSet"), pLingDb.conceptSet);
    childLoop(pTree, currLangToSpec, "langToSpec") _loadSpecificLingDatabase(
        currLangToSpec.second, pLingDb.langToSpec[semanticLanguageEnum_fromStr(currLangToSpec.first)]);
    _loadTranslationDictionary(pTree.get_child("transDict"), pLingDb.transDict);
}

void loadLingDatabase(const boost::property_tree::ptree& pTree, linguistics::LinguisticDatabase& pLingDb) {
    _loadLingDatabase(pTree.get_child("lingDb"), pLingDb);
}

}    // End of namespace serializationprivate
}    // End of namespace onsem
