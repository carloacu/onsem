﻿#include "syntacticgraphtosemantic.hpp"
#include <onsem/common/linguisticsubordinateid.hpp>
#include <onsem/common/utility/uppercasehandler.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/conceptset.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpressions.hpp>
#include <onsem/texttosemantic/dbtype/semanticgroundings.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/conditionexpression.hpp>
#include <onsem/texttosemantic/tool/semexpmodifier.hpp>
#include <onsem/texttosemantic/tool/semexpgetter.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/staticlinguisticdictionary.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/semanticframedictionary.hpp>
#include <onsem/texttosemantic/tool/inflectionschecker.hpp>
#include <onsem/texttosemantic/type/syntacticgraph.hpp>
#include <onsem/texttosemantic/algorithmsetforalanguage.hpp>
#include <onsem/texttosemantic/tool/syntacticanalyzertokenshandler.hpp>
#include <onsem/texttosemantic/type/enumsconvertions.hpp>
#include "semtreehardcodedconverter.hpp"
#include "../tool/chunkshandler.hpp"
#include "../tool/semexpgenerator.hpp"

namespace onsem {
namespace linguistics {
static const std::vector<std::string> adjAdvShouldNotBeReplacedByAConcept = {"degree_",
                                                                             "health_",
                                                                             "manner_",
                                                                             "meaning_",
                                                                             "sentiment_"};

namespace {

const std::set<ChunkLinkType> _ifAndElseChildrenTypes{ChunkLinkType::IF, ChunkLinkType::ELSE};

ListExpressionType _chunkTypeToListExpType(ChunkType pChunkType) {
    if (pChunkType == ChunkType::AND_CHUNK)
        return ListExpressionType::AND;
    if (pChunkType == ChunkType::OR_CHUNK)
        return ListExpressionType::OR;
    if (pChunkType == ChunkType::THEN_CHUNK)
        return ListExpressionType::THEN;
    return ListExpressionType::THEN_REVERSED;
}

void _refactorToAOtherThanGrd(SemanticGrounding& pGrd) {
    auto* genGrdPtr = pGrd.getGenericGroundingPtr();
    if (genGrdPtr != nullptr) {
        genGrdPtr->coreference.emplace(CoreferenceDirectionEnum::BEFORE);
        if (genGrdPtr->referenceType != SemanticReferenceType::DEFINITE
            && genGrdPtr->quantity.type == SemanticQuantityType::NUMBER)
            genGrdPtr->quantity.clear();
    }
}

bool _verbChunkHasAnObjectWithAContxtualInfoAny(Chunk& pVerbChunk) {
    ChunkLink* doChunkLinkPtr = getDOChunkLink(pVerbChunk);
    if (doChunkLinkPtr != nullptr) {
        Chunk& doChunk = *doChunkLinkPtr->chunk;
        auto itFirstWord = doChunk.tokRange.getItBegin();
        if (itFirstWord != doChunk.tokRange.getItEnd()) {
            WordAssociatedInfos& firstWordInfos = itFirstWord->inflWords.front().infos;
            if (firstWordInfos.contextualInfos.count(WordContextualInfos::ANY) != 0) {
                firstWordInfos.concepts.emplace("number_0", 4);
                return true;
            }
        }
    }
    return false;
}

bool _extractNumberOfAChunk(SemanticFloat& pNumber, const Chunk& pChunk) {
    for (TokIt itTokBeforeHead = pChunk.tokRange.getItBegin(); itTokBeforeHead != pChunk.head;
         itTokBeforeHead = getNextToken(itTokBeforeHead, pChunk.head)) {
        const Token& detToken = *itTokBeforeHead;
        const InflectedWord& introInflWord = detToken.inflWords.front();
        if (introInflWord.word.partOfSpeech == PartOfSpeech::DETERMINER)
            return getNumberHoldByTheInflWord(pNumber, itTokBeforeHead, pChunk.head, "number_");
    }
    return false;
}

// TODO: use the one of SemExpCreator
UniqueSemanticExpression _whoSemExp() {
    return std::make_unique<GroundedExpression>([]() {
        auto anyHumanGrd = std::make_unique<SemanticGenericGrounding>();
        anyHumanGrd->entityType = SemanticEntityType::HUMAN;
        anyHumanGrd->concepts.emplace("agent", 4);
        return anyHumanGrd;
    }());
}

GroundedExpression& _getRootForNextdjective(GroundedExpression& pGrdExp) {
    for (auto itChild = pGrdExp.children.rbegin(); itChild != pGrdExp.children.rend(); ++itChild) {
        if (itChild->first == GrammaticalType::OTHER_THAN)
            continue;
        auto* childGrdExpPtr = itChild->second->getGrdExpPtr_SkipWrapperPtrs();
        if (childGrdExpPtr != nullptr) {
            auto* childGenGrdPtr = childGrdExpPtr->grounding().getGenericGroundingPtr();
            if (childGenGrdPtr != nullptr && childGenGrdPtr->word.partOfSpeech == PartOfSpeech::NOUN)
                return *childGrdExpPtr;
        }
    }
    return pGrdExp;
}

void _convertToGeneralitySentence(GroundedExpression& pGrdExpSentence, SemanticStatementGrounding& pStatementGrd) {
    pStatementGrd.concepts.clear();
    pStatementGrd.word.lemma.clear();
    pStatementGrd.concepts["verb_generality"] = 4;
    pGrdExpSentence.children.erase(GrammaticalType::SUBJECT);
}

void _completeWithFollowingProperNouns(std::list<std::string>& pNames, TokIt& pCurrTok, const TokIt& pEndTok) {
    while (true) {
        pCurrTok = getNextToken(pCurrTok, pEndTok);
        if (pCurrTok == pEndTok)
            break;
        const SemanticWord& currTokWord = pCurrTok->inflWords.front().word;
        if (currTokWord.partOfSpeech == PartOfSpeech::PROPER_NOUN) {
            pNames.emplace_back(currTokWord.lemma);
            continue;
        }
        break;
    }
}

void _tryToConvertToInfinitiveSubordinate(UniqueSemanticExpression& pSemExp) {
    auto* grdExpPtr = pSemExp->getGrdExpPtr();
    if (grdExpPtr != nullptr) {
        auto itSepcifier = grdExpPtr->children.find(GrammaticalType::SPECIFIER);
        if (itSepcifier != grdExpPtr->children.end()) {
            auto* specifierGrdExpPtr = itSepcifier->second->getGrdExpPtr();
            if (specifierGrdExpPtr != nullptr) {
                auto* specifierStatGrdPtr = specifierGrdExpPtr->grounding().getStatementGroundingPtr();
                if (specifierStatGrdPtr != nullptr && specifierStatGrdPtr->verbTense == SemanticVerbTense::UNKNOWN) {
                    auto itSpecSubject = specifierGrdExpPtr->children.find(GrammaticalType::SUBJECT);
                    if (itSpecSubject != specifierGrdExpPtr->children.end()
                        && SemExpGetter::isACoreference(*itSpecSubject->second, CoreferenceDirectionEnum::PARENT)) {
                        UniqueSemanticExpression res(std::move(itSepcifier->second));
                        grdExpPtr->children.erase(itSepcifier);
                        auto* newGrdPtr = res->getGrdExpPtr();
                        if (newGrdPtr != nullptr) {
                            newGrdPtr->children[GrammaticalType::SUBJECT] = std::move(pSemExp);
                            pSemExp = std::move(res);
                        } else {
                            assert(false);
                        }
                    }
                }
            }
        }
        return;
    }

    auto* listExpPtr = pSemExp->getListExpPtr();
    if (listExpPtr != nullptr)
        for (auto& currElt : listExpPtr->elts)
            _tryToConvertToInfinitiveSubordinate(currElt);
}

void _fillConditions(UniqueSemanticExpression& pSemExp,
                     std::map<GrammaticalType, std::unique_ptr<ConditionExpression>>& pChildTypeToCondition,
                     const Chunk& pFromChunk,
                     bool pWriteOriginalText) {
    auto it = pChildTypeToCondition.begin();
    while (it != pChildTypeToCondition.end()) {
        UniqueSemanticExpression& rootSemExpOfTheCondition = [&it, &pSemExp]() -> UniqueSemanticExpression& {
            if (it->first != GrammaticalType::UNKNOWN) {
                auto* grdExpPtr = pSemExp->getGrdExpPtr_SkipWrapperPtrs();
                auto itChildForCond = grdExpPtr->children.find(it->first);
                if (itChildForCond != grdExpPtr->children.end())
                    return itChildForCond->second;
            }
            return pSemExp;
        }();

        it->second->thenExp = std::move(rootSemExpOfTheCondition);
        if (pWriteOriginalText)
            it->second->thenExp->fromText = pFromChunk.getTokRangeWrappingChildren(&_ifAndElseChildrenTypes).toStr();
        rootSemExpOfTheCondition = std::move(it->second);
        it = pChildTypeToCondition.erase(it);
    }
}

void _tryToAddTheIntroductingWord(SemanticExpression& pSemExp, const ChunkLink& pChLink) {
    TokIt itTok = pChLink.tokRange.getItBegin();
    if (itTok != pChLink.tokRange.getItEnd()) {
        const auto& word = itTok->inflWords.front().word;
        if (word.partOfSpeech == PartOfSpeech::PREPOSITION) {
            SemExpModifier::addChildFromSemExp(
                pSemExp, GrammaticalType::INTRODUCTING_WORD, std::make_unique<GroundedExpression>([&word] {
                    auto res = std::make_unique<SemanticGenericGrounding>();
                    if (word.lemma == "au" && word.language == SemanticLanguageEnum::FRENCH)
                        res->word = SemanticWord(word.language, "à", PartOfSpeech::PREPOSITION);
                    else
                        res->word = word;
                    res->entityType = SemanticEntityType::MODIFIER;
                    return res;
                }()));
        }
    }
}

void _flattenSomeSpecifiers(SemanticExpression& pSemExp, GroundedExpression& pGrdExpSentence) {
    auto* grdExpPtr = pSemExp.getGrdExpPtr_SkipWrapperPtrs();
    if (grdExpPtr != nullptr) {
        auto& grdExp = *grdExpPtr;
        if (grdExp->getRelLocationGroundingPtr() != nullptr)
            return;
        for (auto itChild = grdExp.children.begin(); itChild != grdExp.children.end();) {
            if (itChild->first == GrammaticalType::SPECIFIER) {
                auto* specifierGrdExpPtr = itChild->second->getGrdExpPtr_SkipWrapperPtrs();
                if (specifierGrdExpPtr != nullptr) {
                    auto& specGrd = specifierGrdExpPtr->grounding();
                    if (specGrd.getAngleGroundingPtr() != nullptr) {
                        SemExpModifier::addChild(pGrdExpSentence,
                                                 GrammaticalType::LOCATION,
                                                 std::move(itChild->second),
                                                 ListExpressionType::UNRELATED);
                        itChild = grdExp.children.erase(itChild);
                        break;
                    }
                    if (specGrd.getLengthGroundingPtr() != nullptr) {
                        SemExpModifier::addChild(pGrdExpSentence,
                                                 GrammaticalType::LENGTH,
                                                 std::move(itChild->second),
                                                 ListExpressionType::UNRELATED);
                        itChild = grdExp.children.erase(itChild);
                        break;
                    }
                    if (specGrd.getPercentageGroundingPtr() != nullptr) {
                        SemExpModifier::addChild(pGrdExpSentence,
                                                 GrammaticalType::SPECIFIER,
                                                 std::move(itChild->second),
                                                 ListExpressionType::UNRELATED);
                        itChild = grdExp.children.erase(itChild);
                        break;
                    }
                }
            }
            ++itChild;
        }
    }
}

std::unique_ptr<GroundedExpression> _tryToConvertLanguageChunk(const std::map<std::string, char>& pConcepts) {
    static const std::string langLabel = "language_";
    for (const auto& currCpt : pConcepts) {
        const std::string& conceptName = currCpt.first;
        if (ConceptSet::doesConceptBeginWith(conceptName, langLabel)) {
            return std::make_unique<GroundedExpression>(
                std::make_unique<SemanticLanguageGrounding>(semanticLanguageTypeGroundingEnumFromStr(
                    conceptName.substr(langLabel.size(), conceptName.size() - langLabel.size()))));
        }
    }
    return std::unique_ptr<GroundedExpression>();
}

}

SyntacticGraphToSemantic::SyntacticGraphToSemantic(const AlgorithmSetForALanguage& pConfiguration)
    : fConfiguration(pConfiguration)
    , fSemFrameDict(pConfiguration.getSpecifcLingDb().getSemFrameDict())
    , fLingDico(pConfiguration.getSpecifcLingDb().lingDico) {}

SyntacticGraphToSemantic::~SyntacticGraphToSemantic() {}

UniqueSemanticExpression SyntacticGraphToSemantic::process(
    const SyntacticGraph& pSyntGraph,
    const TextProcessingContext& pTextProcContext,
    const SemanticTimeGrounding& pTimeGrd,
    std::unique_ptr<SemanticAgentGrounding> pAgentWeAreTalkingAbout) const {
    SemanticLanguageEnum language = pSyntGraph.langConfig.getLingDico().getLanguage();
    if (language == SemanticLanguageEnum::JAPANESE) {
        return xAddSemExpWithMainSentimentalWord(pSyntGraph.tokensTree);
    }

    ChunkLinkType chunkLinkTypeThatIntroduceAText = ChunkLinkType::SIMPLE;
    mystd::unique_propagate_const<UniqueSemanticExpression> res;
    ToGenRepGeneral general(res, pTextProcContext, pSyntGraph, pTimeGrd, std::move(pAgentWeAreTalkingAbout));
    for (auto itChild = pSyntGraph.firstChildren.begin(); itChild != pSyntGraph.firstChildren.end(); ++itChild) {
        ToGenRepContext context(*itChild);
        if (itChild->type == ChunkLinkType::NOTUNDERSTOOD) {
            auto semExp = xFillSemExp(general, context);
            if (semExp) {
                auto semExpWrapped = std::make_unique<GroundedExpression>();
                semExpWrapped->children.emplace(GrammaticalType::NOT_UNDERSTOOD, std::move(*semExp));
                xAddSemExpPtr(general.uSemExp, std::move(semExpWrapped));
            }
        } else {
            if (context.chunk.type != ChunkType::SEPARATOR_CHUNK
                && (context.chunk.type != ChunkType::PREPOSITIONAL_CHUNK
                    || context.chunk.getHeadPartOfSpeech() != PartOfSpeech::PREPOSITION)) {
                auto semExp = _processQuestionWithoutVerb(itChild, general, context.chunk, pSyntGraph);

                if (!semExp)
                    semExp = xFillSemExp(general, context);
                if (semExp) {
                    if (chunkLinkTypeThatIntroduceAText != ChunkLinkType::SIMPLE) {
                        auto grammTypeThatIntroduceATextOpt = chunkTypeToGrammaticalType(chunkLinkTypeThatIntroduceAText);
                        if (grammTypeThatIntroduceATextOpt)
                            SemExpModifier::addCoreferenceMotherSemExp(*semExp, *grammTypeThatIntroduceATextOpt);
                        else if (chunkLinkTypeThatIntroduceAText == ChunkLinkType::IF) {
                            auto newCondSemExp = std::make_unique<ConditionExpression>(false, false, std::move(*semExp),
                                                                                        SemExpGenerator::makeCoreferenceExpression(CoreferenceDirectionEnum::BEFORE));
                            *semExp = std::move(newCondSemExp);
                        }
                        chunkLinkTypeThatIntroduceAText = ChunkLinkType::SIMPLE;
                    }
                    xAddSemExpPtr(general.uSemExp, std::move(*semExp));
                }
            } else {
                auto semExp = xConvertSeparatorChunkToSemExp(general, context);
                if (semExp)
                    xAddSemExpIfNotEmpty(general.uSemExp, std::move(*semExp));
                else
                    chunkLinkTypeThatIntroduceAText = xConvertSeparatorChunkToChunkLinkType(context);
            }
        }
    }

    if (res)
        return std::move(*res);
    return UniqueSemanticExpression();
}

void SyntacticGraphToSemantic::xAddSemExpPtr(mystd::unique_propagate_const<UniqueSemanticExpression>& pUSemExpToFill,
                                             UniqueSemanticExpression pSemExp) const {
    xReplaceQuestWordByRequest(*pSemExp);
    if (pUSemExpToFill)
        SemExpModifier::addNewSemExp(*pUSemExpToFill, std::move(pSemExp));
    else
        pUSemExpToFill.emplace(std::move(pSemExp));
}

void SyntacticGraphToSemantic::xAddSemExpIfNotEmpty(
    mystd::unique_propagate_const<UniqueSemanticExpression>& pUSemExp,
    mystd::unique_propagate_const<UniqueSemanticExpression> pSemExp) const {
    if (pSemExp)
        xAddSemExpPtr(pUSemExp, std::move(*pSemExp));
}

void SyntacticGraphToSemantic::xReplaceQuestWordByRequest(SemanticExpression& pSemExp) const {
    if (pSemExp.type == SemanticExpressionType::GROUNDED) {
        GroundedExpression& grdExp = pSemExp.getGrdExp();
        if (grdExp->type == SemanticGroundingType::GENERIC && grdExp.children.empty()) {
            const SemanticGenericGrounding& genGrd = grdExp->getGenericGrounding();
            SemanticRequestType requType = fLingDico.aloneWordToRequest(genGrd.word);
            if (requType != SemanticRequestType::NOTHING) {
                auto possibleGenders = genGrd.possibleGenders;
                grdExp.moveGrounding([&requType] {
                    auto statGrd = std::make_unique<SemanticStatementGrounding>();
                    statGrd->requests.set(requType);
                    statGrd->coreference.emplace();
                    return statGrd;
                }());
                if (requType == SemanticRequestType::OBJECT && genGrd.word.partOfSpeech == PartOfSpeech::PRONOUN) {
                    if (possibleGenders.size() == 1) {
                        SemExpModifier::addChild(
                            grdExp, GrammaticalType::OBJECT, std::make_unique<GroundedExpression>([&] {
                                auto genGrd = std::make_unique<SemanticGenericGrounding>();
                                genGrd->possibleGenders = std::move(possibleGenders);
                                genGrd->referenceType = SemanticReferenceType::DEFINITE;
                                return genGrd;
                            }()));
                    }
                }
            }
        }
    }
}

UniqueSemanticExpression SyntacticGraphToSemantic::xAddSemExpWithMainSentimentalWord(
    const TokensTree& pTokensTree) const {
    const Token* mainToken = nullptr;
    if (!pTokensTree.tokens.empty()) {
        mainToken = &pTokensTree.beginToken().getToken();
        char mainConfidence = 0;
        for (ConstTokenIterator itTok = pTokensTree.beginToken(); !itTok.atEnd(); ++itTok) {
            const Token& currToken = itTok.getToken();
            for (const auto& currCpt : currToken.inflWords.front().infos.concepts) {
                char absConfidence = currCpt.second < 0 ? static_cast<char>(-currCpt.second) : currCpt.second;
                if (absConfidence > mainConfidence) {
                    mainToken = &currToken;
                    mainConfidence = absConfidence;
                }
            }
        }
    }

    if (mainToken != nullptr) {
        auto textGrounding = std::make_unique<SemanticTextGrounding>(mainToken->str);
        textGrounding->concepts = mainToken->inflWords.front().infos.concepts;
        return UniqueSemanticExpression(std::make_unique<GroundedExpression>(std::move(textGrounding)));
    }
    return UniqueSemanticExpression();
}

void SyntacticGraphToSemantic::xFillPossibleGenders(std::set<SemanticGenderType>& pPossibleGenders,
                                                    const InflectedWord& pInflWord,
                                                    SemanticNumberType pNumberOfInflWord) const {
    if (pInflWord.word.partOfSpeech == PartOfSpeech::NOUN) {
        std::set<SemanticGenderType> possibleGendersFromWord;
        fConfiguration.getSpecifcLingDb().synthDico.getNounGendersFromWord(
            possibleGendersFromWord, pInflWord.word, pNumberOfInflWord);
        if (possibleGendersFromWord.size() > 1)
            fConfiguration.getFlsChecker().initGenderSetFromIGram(pPossibleGenders, pInflWord);
    } else if (pInflWord.word.partOfSpeech == PartOfSpeech::UNKNOWN
               || pInflWord.word.partOfSpeech == PartOfSpeech::PRONOUN) {
        fConfiguration.getFlsChecker().initGenderSetFromIGram(pPossibleGenders, pInflWord);
    }
}

void SyntacticGraphToSemantic::xInitGenGroundingsFromToken(SemanticGenericGrounding& pGenGroundings,
                                                           TokIt pToken,
                                                           const TokIt& pItEndToken) const {
    const InflectedWord& inflWord = pToken->inflWords.front();
    PartOfSpeech headPartOfSpeech = inflWord.word.partOfSpeech;

    if (headPartOfSpeech != PartOfSpeech::VERB && headPartOfSpeech != PartOfSpeech::PREPOSITION
        && headPartOfSpeech != PartOfSpeech::ADVERB && headPartOfSpeech != PartOfSpeech::ADJECTIVE
        && headPartOfSpeech != PartOfSpeech::INTERJECTION) {
        SemanticNumberType numberOfInflWord = SemExpGetter::getNumberFromInflections(inflWord);
        if (fConfiguration.getLanguageType() == SemanticLanguageEnum::FRENCH)
            xFillPossibleGenders(pGenGroundings.possibleGenders, inflWord, numberOfInflWord);
        if (numberOfInflWord == SemanticNumberType::PLURAL)
            pGenGroundings.quantity.setPlural();
        else if (!InflectionsChecker::isuncountable(inflWord.infos))
            pGenGroundings.quantity.setNumber(1);
    }

    {
        SemanticFloat numberForToken;
        if (getNumberHoldByTheInflWord(numberForToken, pToken, pItEndToken, "number_")) {
            pGenGroundings.entityType = SemanticEntityType::NUMBER;
            pGenGroundings.quantity.setNumber(numberForToken);
            return;
        }
        if (getNumberHoldByTheInflWord(numberForToken, pToken, pItEndToken, "rank_")) {
            pGenGroundings.entityType = SemanticEntityType::MODIFIER;
            return;
        }
    }

    if (inflWord.word.partOfSpeech == PartOfSpeech::ADJECTIVE || inflWord.word.partOfSpeech == PartOfSpeech::ADVERB
        || inflWord.word.partOfSpeech == PartOfSpeech::INTERJECTION)
        pGenGroundings.entityType = SemanticEntityType::MODIFIER;
    else
        pGenGroundings.entityType = SemanticEntityType::THING;

    if (!inflWord.word.lemma.empty()) {
        pGenGroundings.word = inflWord.word;
        if (ConceptSet::haveAConceptOrAHyponym(pGenGroundings.concepts, "agent"))
            pGenGroundings.entityType = SemanticEntityType::HUMAN;
        else if (inflWord.word.partOfSpeech == PartOfSpeech::PROPER_NOUN)
            pGenGroundings.entityType = SemanticEntityType::AGENTORTHING;
    } else {
        pGenGroundings.word.lemma = pToken->str;
    }
    SemExpModifier::fillCoreference(pGenGroundings.coreference, inflWord.infos.concepts);
}

void SyntacticGraphToSemantic::xConvertVerbChunkToRootGrounding(SemanticStatementGrounding& pRootGrounding,
                                                                const Chunk& pChunk) const {
    const InflectedWord& headIgram = pChunk.head->inflWords.front();
    if (!headIgram.word.lemma.empty()) {
        pRootGrounding.word = headIgram.word;
        pRootGrounding.concepts = headIgram.infos.concepts;
    }
}

bool _shouldConceptsBeInsideAConceptualGrounding(bool pHasReferenceConcept,
                                                 const std::map<std::string, char>& pConcepts) {
    if (pHasReferenceConcept)
        return false;
    bool res = false;
    for (const auto& currCpt : pConcepts) {
        if (!ConceptSet::isAConceptAny(currCpt.first)
            && !ConceptSet::doesConceptBeginWithWithAnyOf(currCpt.first, adjAdvShouldNotBeReplacedByAConcept)) {
            if (currCpt.first != "accordance_agreement_yes" && currCpt.first != "engagement_engage"
                && currCpt.first != "alone" && currCpt.first != "quantity_everything")
                res = true;
            else
                return false;
        }
    }
    return res;
}

std::unique_ptr<GroundedExpression> _strToMetaOrResourceGrdOld(
    const std::string& pStr,
    const std::shared_ptr<ResourceGroundingExtractor>& pCmdGrdExtractorPtr) {
    if (SemanticMetaGrounding::isTheBeginOfAParamOld(pStr)) {
        std::unique_ptr<SemanticMetaGrounding> metaGrd;
        metaGrd = SemanticMetaGrounding::makeMetaGroundingFromStrOld(pStr);
        if (metaGrd)
            return std::make_unique<GroundedExpression>(std::move(metaGrd));
    } else if (pCmdGrdExtractorPtr) {
        const std::string& begOfTheResource = pCmdGrdExtractorPtr->extractBeginOfAResource(pStr);
        if (!begOfTheResource.empty()) {
            std::unique_ptr<SemanticResourceGrounding> resourceGrd;
            resourceGrd = ResourceGroundingExtractor::makeResourceGroundingFromStr(pStr, begOfTheResource);
            if (resourceGrd)
                return std::make_unique<GroundedExpression>(std::move(resourceGrd));
        }
    }
    return std::unique_ptr<GroundedExpression>();
}


std::unique_ptr<GroundedExpression> _strToMetaOrResourceGrd(
    const std::string& pStr) {
    std::unique_ptr<SemanticMetaGrounding> metaGrd;
    metaGrd = SemanticMetaGrounding::makeMetaGroundingFromStr(pStr);
    if (metaGrd)
        return std::make_unique<GroundedExpression>(std::move(metaGrd));
    return std::unique_ptr<GroundedExpression>();
}


void _fillRelativeCharEncodedFromTokenRange(LinguisticSubordinateId& pRes, const TokenRange& pTokRange) {
    for (TokIt itTok = pTokRange.getItBegin(); itTok != pTokRange.getItEnd();
         itTok = getNextToken(itTok, pTokRange.getItEnd())) {
        const InflectedWord& linkWorkIGram = itTok->inflWords.front();
        if (linkWorkIGram.word.partOfSpeech == PartOfSpeech::PREPOSITION
            || linkWorkIGram.word.partOfSpeech == PartOfSpeech::SUBORDINATING_CONJONCTION)
            fillRelativeCharEncodedFromInflWord(pRes, linkWorkIGram);
    }
}

bool _modifyTimeGrdAccordingToARelativeDay(SemanticTimeGrounding& pTimeGrd, const std::string& pConceptStr) {
    if (pConceptStr == "time_relativeDay_yesterday") {
        pTimeGrd.date.moveOfANumberOfDaysInPast(1);
        return true;
    }
    if (pConceptStr == "time_relativeDay_tomorrow") {
        pTimeGrd.date.moveOfANumberOfDaysInFuture(1);
        return true;
    }
    if (pConceptStr == "time_relativeDay_today") {
        return true;
    }
    if (pConceptStr == "time_relativeDay_dayAfterTomorrow") {
        pTimeGrd.date.moveOfANumberOfDaysInFuture(2);
        return true;
    }
    if (pConceptStr == "time_relativeDay_dayBeforeYesterday") {
        pTimeGrd.date.moveOfANumberOfDaysInPast(2);
        return true;
    }
    return false;
}

void _considerTimeModifiersBeforeHead(SemanticTimeGrounding& pTimeGrd, const Chunk& pChunk) {
    for (TokIt it = pChunk.tokRange.getItBegin(); it != pChunk.head; it = getNextToken(it, pChunk.head)) {
        const auto& wordConcepts = it->inflWords.front().infos.concepts;
        if (ConceptSet::haveAConceptThatBeginWith(wordConcepts, "time_")) {
            std::map<std::string, char> timeRelConcepts;
            ConceptSet::extractConceptsThatBeginWith(timeRelConcepts, wordConcepts, "time_relativeDay_");
            if (timeRelConcepts.size() == 1) {
                const std::string& relCptStr = timeRelConcepts.begin()->first;
                _modifyTimeGrdAccordingToARelativeDay(pTimeGrd, relCptStr);
            }
        }
    }
}

void _considerTimeModifiersAfterHead(SemanticTimeGrounding& pTimeGrd, const Chunk& pChunk) {
    auto itEnd = pChunk.tokRange.getItEnd();
    for (TokIt it = getNextToken(pChunk.head, itEnd); it != itEnd; it = getNextToken(it, itEnd)) {
        const auto& wordConcepts = it->inflWords.front().infos.concepts;
        std::map<std::string, char> timeConcepts;
        ConceptSet::extractConceptsThatBeginWith(timeConcepts, wordConcepts, "time_");
        if (timeConcepts.size() == 1) {
            pTimeGrd.fromConcepts.insert(timeConcepts.begin(), timeConcepts.end());
            std::map<std::string, char> dayRelConcepts;
            ConceptSet::extractConceptsThatBeginWith(dayRelConcepts, wordConcepts, "time_day_");
            if (dayRelConcepts.size() == 1) {
                const std::string& dayRelCptStr = dayRelConcepts.begin()->first;
                pTimeGrd.modifyTimeGrdAccordingToADayPart(dayRelCptStr);
            }
        }
    }
}

void SyntacticGraphToSemantic::xAddOwnertTolink(GroundedExpression& pGrdExpParent,
                                                const InflectedWord& pIntroInflWord,
                                                RelativePerson pRelativePerson,
                                                const ToGenRepContext& pContext,
                                                const ToGenRepGeneral& pGeneral) const {
    switch (pRelativePerson) {
        case RelativePerson::FIRST_SING:
        case RelativePerson::SECOND_SING:
        case RelativePerson::SECOND_PLUR: {
            auto detSemExp = xTranslateRelativePersToPerson(pRelativePerson, pIntroInflWord, pContext, pGeneral);
            if (detSemExp)
                SemExpModifier::addChild(pGrdExpParent, GrammaticalType::OWNER, std::move(detSemExp));
            break;
        }
        case RelativePerson::THIRD_SING: {
            auto subGenGrd = std::make_unique<SemanticGenericGrounding>();
            subGenGrd->referenceType = SemanticReferenceType::DEFINITE;
            subGenGrd->entityType = SemanticEntityType::HUMAN;
            subGenGrd->coreference.emplace();
            subGenGrd->quantity.setNumber(1);
            SemExpModifier::addChild(
                pGrdExpParent, GrammaticalType::OWNER, std::make_unique<GroundedExpression>(std::move(subGenGrd)));
            break;
        }
        case RelativePerson::FIRST_PLUR: {
            SemExpModifier::addChild(pGrdExpParent, GrammaticalType::OWNER, pGeneral.textProcContext.usSemExp->clone());
            break;
        }
        case RelativePerson::THIRD_PLUR: {
            auto subGenGrd = std::make_unique<SemanticGenericGrounding>();
            subGenGrd->referenceType = SemanticReferenceType::DEFINITE;
            subGenGrd->entityType = SemanticEntityType::HUMAN;
            subGenGrd->coreference.emplace();
            subGenGrd->quantity.setPlural();
            SemExpModifier::addChild(
                pGrdExpParent, GrammaticalType::OWNER, std::make_unique<GroundedExpression>(std::move(subGenGrd)));
            break;
        }
        case RelativePerson::UNKNOWN: break;
    }
}

void SyntacticGraphToSemantic::xConsiderDeterminerAtTheBeginningOfAGrounding(GroundedExpression& pRootGrdExp,
                                                                             const Chunk& pChunk,
                                                                             const ToGenRepContext& pContext,
                                                                             const ToGenRepGeneral& pGeneral) const {
    TokIt itTokBeforeHead = pChunk.tokRange.getItBegin();
    auto& resGrd = pRootGrdExp.grounding();
    bool knowReferenceForSure = false;
    while (itTokBeforeHead != pChunk.head) {
        const Token& detToken = *itTokBeforeHead;
        const InflectedWord& introInflWord = detToken.inflWords.front();
        if (introInflWord.word.partOfSpeech == PartOfSpeech::DETERMINER
            || introInflWord.word.partOfSpeech == PartOfSpeech::PARTITIVE)
            itTokBeforeHead = xAddDeterminerToAGrounding(
                pRootGrdExp, resGrd, knowReferenceForSure, itTokBeforeHead, pChunk.head, pContext, pGeneral);
        itTokBeforeHead = getNextToken(itTokBeforeHead, pChunk.head);
    }
}

TokIt SyntacticGraphToSemantic::xAddDeterminerToAGrounding(GroundedExpression& pRootGrdExp,
                                                           SemanticGrounding& pRootGrounding,
                                                           bool& pKnowReferenceForSure,
                                                           const TokIt& pItDetToken,
                                                           const TokIt& pItEndToken,
                                                           const ToGenRepContext& pContext,
                                                           const ToGenRepGeneral& pGeneral) const {
    TokIt res = pItDetToken;
    const InflectedWord& introInflWord = pItDetToken->inflWords.front();
    SemanticGenericGrounding* genGrdPtr = pRootGrounding.getGenericGroundingPtr();
    if (genGrdPtr != nullptr) {
        SemanticLanguageEnum language = fConfiguration.getLanguageType();
        SemanticGenericGrounding& genGrd = *genGrdPtr;

        bool canCheckReference = true;
        if (genGrd.entityType == SemanticEntityType::AGENTORTHING)
            genGrd.entityType = SemanticEntityType::THING;

        {
            mystd::optional<SemanticFloat> numberOpt;
            res = eatNumber(numberOpt, pItDetToken, pItEndToken, "number_");
            if (numberOpt) {
                if (genGrd.entityType != SemanticEntityType::NUMBER) {
                    if (!pKnowReferenceForSure)
                        genGrd.referenceType = SemanticReferenceType::INDEFINITE;
                    genGrd.quantity.setNumber(*numberOpt);
                } else {
                    if (*numberOpt != 1) {
                        canCheckReference = false;
                        if (*numberOpt > genGrd.quantity.nb)
                            genGrd.quantity.increaseNumber(*numberOpt);
                        else
                            genGrd.quantity.setNumber(*numberOpt * genGrd.quantity.nb);
                    }
                }
            }
        }

        if (language == SemanticLanguageEnum::FRENCH && introInflWord.word.partOfSpeech == PartOfSpeech::DETERMINER
            && genGrd.word.partOfSpeech == PartOfSpeech::PROPER_NOUN && genGrd.possibleGenders.empty()) {
            fConfiguration.getFlsChecker().initGenderSetFromIGram(genGrd.possibleGenders, introInflWord);
        }

        if (!introInflWord.word.lemma.empty()) {
            SemExpModifier::fillCoreference(genGrd.coreference, introInflWord.infos.concepts);
            if (canCheckReference
                && ConceptSet::haveAConceptThatBeginWith(introInflWord.infos.concepts, "reference_")) {
                if (ConceptSet::haveAConcept(introInflWord.infos.concepts, "reference_indefinite")) {
                    genGrd.referenceType = SemanticReferenceType::INDEFINITE;
                    pKnowReferenceForSure = true;
                } else if (ConceptSet::haveAConcept(introInflWord.infos.concepts, "reference_definite")) {
                    genGrd.referenceType = SemanticReferenceType::DEFINITE;
                    pKnowReferenceForSure = true;
                }
                if (ConceptSet::haveAConcept(introInflWord.infos.concepts, "reference_other")) {
                    auto corefGrounding = pRootGrdExp.cloneGrounding();
                    _refactorToAOtherThanGrd(*corefGrounding);
                    SemExpModifier::addChild(pRootGrdExp,
                                             GrammaticalType::OTHER_THAN,
                                             std::make_unique<GroundedExpression>(std::move(corefGrounding)));
                }
            }

            if (ConceptSet::haveAConceptThatBeginWith(introInflWord.infos.concepts, "tolink_")) {
                genGrd.referenceType = SemanticReferenceType::DEFINITE;
                pKnowReferenceForSure = true;
                RelativePerson relPers = ConceptSet::conceptsToRelativePerson(introInflWord.infos.concepts);
                xAddOwnertTolink(pRootGrdExp, introInflWord, relPers, pContext, pGeneral);
            } else if (ConceptSet::haveAConceptThatBeginWith(introInflWord.infos.concepts, "quantity_")) {
                if (ConceptSet::haveAConcept(introInflWord.infos.concepts, "quantity_anything")) {
                    genGrd.referenceType = SemanticReferenceType::INDEFINITE;
                    pKnowReferenceForSure = true;
                    genGrd.quantity.type = SemanticQuantityType::ANYTHING;
                } else if (ConceptSet::haveAConcept(introInflWord.infos.concepts, "quantity_everything")) {
                    genGrd.referenceType = SemanticReferenceType::INDEFINITE;
                    pKnowReferenceForSure = true;
                    genGrd.quantity.type = SemanticQuantityType::EVERYTHING;
                } else if (ConceptSet::haveAConcept(introInflWord.infos.concepts, "quantity_maxNumber")) {
                    genGrd.referenceType = SemanticReferenceType::DEFINITE;
                    pKnowReferenceForSure = true;
                    genGrd.quantity.type = SemanticQuantityType::MAXNUMBER;
                } else if (ConceptSet::haveAConcept(introInflWord.infos.concepts, "quantity_many")) {
                    genGrd.referenceType = SemanticReferenceType::INDEFINITE;
                    pKnowReferenceForSure = true;
                    genGrd.quantity.type = SemanticQuantityType::MOREOREQUALTHANNUMBER;
                    genGrd.quantity.nb = 2;
                    genGrd.quantity.subjectiveValue = SemanticSubjectiveQuantity::MANY;
                } else if (ConceptSet::haveAConcept(introInflWord.infos.concepts, "quantity_some")) {
                    genGrd.referenceType = SemanticReferenceType::INDEFINITE;
                    pKnowReferenceForSure = true;
                    genGrd.quantity.type = SemanticQuantityType::MOREOREQUALTHANNUMBER;
                    genGrd.quantity.nb = 2;
                    genGrd.quantity.subjectiveValue = SemanticSubjectiveQuantity::SOME;
                } else if (ConceptSet::haveAConcept(introInflWord.infos.concepts, "quantity_more")) {
                    genGrd.referenceType = SemanticReferenceType::INDEFINITE;
                    pKnowReferenceForSure = true;
                    genGrd.quantity.subjectiveValue = SemanticSubjectiveQuantity::MORE;
                } else if (ConceptSet::haveAConcept(introInflWord.infos.concepts, "quantity_less")) {
                    genGrd.referenceType = SemanticReferenceType::INDEFINITE;
                    pKnowReferenceForSure = true;
                    genGrd.quantity.subjectiveValue = SemanticSubjectiveQuantity::LESS;
                } else if (ConceptSet::haveAConcept(introInflWord.infos.concepts, "quantity_nothing")) {
                    genGrd.quantity.setNumber(0);
                } else {
                    pKnowReferenceForSure = true;
                }
            } else if (language == SemanticLanguageEnum::FRENCH) {
                if (introInflWord.word.partOfSpeech == PartOfSpeech::PARTITIVE && genGrd.quantity.isEqualToOne()) {
                    genGrd.quantity.clear();
                } else if (introInflWord.word.partOfSpeech == PartOfSpeech::DETERMINER
                           && introInflWord.infos.hasContextualInfo(WordContextualInfos::UNCOUNTABLE)) {
                    if (genGrd.quantity.type == SemanticQuantityType::NUMBER)
                        genGrd.quantity.clear();
                }
            }
        }
        {
            if (!pItDetToken->str.empty() && pItDetToken->str[0] == SemanticMetaGrounding::firstCharOfStrOld) {
                int paramId = 0;
                std::string label;
                std::string attributeName;
                if (SemanticMetaGrounding::parseParameterOld(paramId, label, attributeName, pItDetToken->str)
                    && label == "number") {
                    genGrd.quantity.setNumberToFill(paramId, attributeName);
                    if (!pKnowReferenceForSure)
                        genGrd.referenceType = SemanticReferenceType::INDEFINITE;
                }
            }
        }
    } else if (!introInflWord.word.lemma.empty()) {
        RelativePerson relPers = ConceptSet::conceptsToRelativePerson(introInflWord.infos.concepts);
        xAddOwnertTolink(pRootGrdExp, introInflWord, relPers, pContext, pGeneral);
    }
    return res;
}

mystd::unique_propagate_const<UniqueSemanticExpression> SyntacticGraphToSemantic::xConvertToTimeGrounding(
    ToGenRepContext& pContext,
    ToGenRepGeneral& pGeneral,
    const Chunk& pChunk,
    SemanticReferenceType pReferenceType,
    const std::map<std::string, char>& pConcepts) const {
    SemanticLanguageEnum languageType = fConfiguration.getLanguageType();
    std::map<std::string, char> timeRelConcepts;
    if (languageType != SemanticLanguageEnum::FRENCH || pReferenceType != SemanticReferenceType::DEFINITE) {
        ConceptSet::extractConceptsThatBeginWith(timeRelConcepts, pConcepts, "time_day_");
        if (timeRelConcepts.size() == 1) {
            const std::string& relCptStr = timeRelConcepts.begin()->first;
            auto timeGrounding = std::make_unique<SemanticTimeGrounding>();
            timeGrounding->fromConcepts = timeRelConcepts;
            timeGrounding->date = pGeneral.syntGraphTime.date;
            if (timeGrounding->modifyTimeGrdAccordingToADayPart(relCptStr)) {
                _considerTimeModifiersBeforeHead(*timeGrounding, pChunk);
                if ((relCptStr == "time_day_midnight" || relCptStr == "time_day_night")
                    && isAPastTense(pContext.holdingSentenceVerbTense)
                    && timeGrounding->isAfter(pGeneral.syntGraphTime))
                    timeGrounding->date.moveOfANumberOfDaysInPast(1);
                return mystd::unique_propagate_const<UniqueSemanticExpression>(
                    std::make_unique<GroundedExpression>(std::move(timeGrounding)));
            }
        }
    }

    if (pContext.grammTypeFromParent == GrammaticalType::TIME && ConceptSet::haveAConcept(pConcepts, "time_day")) {
        TokIt itTokBeforeHead = pChunk.tokRange.getItBegin();
        while (itTokBeforeHead != pChunk.head) {
            const InflectedWord& introInflWord = itTokBeforeHead->inflWords.front();
            PartOfSpeech introGram = introInflWord.word.partOfSpeech;
            if ((introGram == PartOfSpeech::DETERMINER || introGram == PartOfSpeech::PARTITIVE)
                && ConceptSet::haveAConcept(introInflWord.infos.concepts, "reference_definite")) {
                auto timeGrounding = std::make_unique<SemanticTimeGrounding>();
                timeGrounding->fromConcepts.emplace("time_day", 4);
                timeGrounding->date = pGeneral.syntGraphTime.date;
                timeGrounding->length.add(SemanticTimeUnity::DAY, 1);
                return mystd::unique_propagate_const<UniqueSemanticExpression>(
                    std::make_unique<GroundedExpression>(std::move(timeGrounding)));
            }
            itTokBeforeHead = getNextToken(itTokBeforeHead, pChunk.head);
        }
    }
    return mystd::unique_propagate_const<UniqueSemanticExpression>();
}

UniqueSemanticExpression SyntacticGraphToSemantic::xConvertNominalChunkToSemExp(ToGenRepContext& pContext,
                                                                                ToGenRepGeneral& pGeneral,
                                                                                const ChunkLink& pChunkLink,
                                                                                const Chunk& pChunk) const {
    if (pChunk.tokRange.isEmpty())
        return std::make_unique<GroundedExpression>();

    const auto& headConcepts = pChunk.head->inflWords.front().infos.concepts;
    bool headBeginWithTimeConcept = ConceptSet::haveAConceptThatBeginWith(headConcepts, "time_");
    if (headBeginWithTimeConcept) {
        if (pGeneral.textProcContext.isTimeDependent
            && (pContext.holdingSentenceVerbTense == SemanticVerbTense::UNKNOWN
                || isAPresentTense(pContext.holdingSentenceVerbTense))) {
            auto timeCpt = ConceptSet::getAnyOfConcepts(headConcepts, {"time_relative_now", "time_relative_rightAway"});
            if (!timeCpt.first.empty()) {
                auto timeGrounding = std::make_unique<SemanticTimeGrounding>(pGeneral.syntGraphTime);
                timeGrounding->fromConcepts.insert(timeCpt);
                return std::make_unique<GroundedExpression>(std::move(timeGrounding));
            }
        }

        if (pGeneral.textProcContext.isTimeDependent) {
            std::map<std::string, char> timeRelConcepts;
            ConceptSet::extractConceptsThatBeginWith(timeRelConcepts, headConcepts, "time_relativeDay_");
            if (timeRelConcepts.size() >= 1) {
                const std::string& relCptStr = timeRelConcepts.begin()->first;
                auto timeGrounding = std::make_unique<SemanticTimeGrounding>();
                timeGrounding->date = pGeneral.syntGraphTime.date;
                if (_modifyTimeGrdAccordingToARelativeDay(*timeGrounding, relCptStr)) {
                    timeGrounding->length.add(SemanticTimeUnity::DAY, 1);
                    timeGrounding->concepts = std::move(timeRelConcepts);
                    return std::make_unique<GroundedExpression>(std::move(timeGrounding));
                }
            }
        }

        std::map<std::string, char> weekDayConcepts;
        ConceptSet::extractConceptsThatBeginWith(weekDayConcepts, headConcepts, "time_weekday_");
        if (weekDayConcepts.size() == 1) {
            auto timeGrounding = std::make_unique<SemanticTimeGrounding>();
            timeGrounding->fromConcepts = weekDayConcepts;
            bool shouldBeInPast = isAPastTense(pContext.holdingSentenceVerbTense);
            timeGrounding->date.dayEqualToAWeekDayOfThisWeek(
                semanticTimeWeekdayEnum_fromStr(weekDayConcepts.begin()->first), shouldBeInPast);
            _considerTimeModifiersAfterHead(*timeGrounding, pChunk);
            return std::make_unique<GroundedExpression>(std::move(timeGrounding));
        }

        {
            std::map<std::string, char> hourRelConcepts;
            ConceptSet::extractConceptsThatBeginWith(hourRelConcepts, headConcepts, "time_hour_");
            if (hourRelConcepts.size() >= 1) {
                const std::string& relHourCptStr = hourRelConcepts.begin()->first;
                auto timeGrounding = std::make_unique<SemanticTimeGrounding>();

                SemanticFloat hourNumber;
                if (_extractNumberOfAChunk(hourNumber, pChunk)) {
                    if (relHourCptStr == "time_hour_am") {
                        timeGrounding->timeOfDay.add(SemanticTimeUnity::HOUR, hourNumber);
                        return std::make_unique<GroundedExpression>(std::move(timeGrounding));
                    }
                    if (relHourCptStr == "time_hour_pm") {
                        timeGrounding->timeOfDay.add(SemanticTimeUnity::HOUR, hourNumber + 12);
                        return std::make_unique<GroundedExpression>(std::move(timeGrounding));
                    }
                }
            }
        }
    }

    if (!pChunk.head->str.empty()) {
        if (pChunk.head->str[0] == SemanticMetaGrounding::firstCharOfStrOld) {
            const std::string& headStr = pChunk.head->str;
            auto res = _strToMetaOrResourceGrdOld(headStr, pGeneral.textProcContext.linguisticAnalysisConfig.cmdGrdExtractorPtr);
            if (res)
                return res;
        }
        if (pChunk.head->str[0] == SemanticMetaGrounding::firstCharOfStr) {
            const std::string& headStr = pChunk.head->str;
            auto res = _strToMetaOrResourceGrd(headStr);
            if (res)
                return res;
        }
    }

    SemanticLanguageEnum languageType = fConfiguration.getLanguageType();
    const InflectedWord& chunkHeadIGram = pChunk.head->inflWords.front();
    if (chunkHeadIGram.word.partOfSpeech == PartOfSpeech::UNKNOWN) {
        const std::string& headStr = pChunk.head->str;
        if (SemanticTextGrounding::isAQuotedText(headStr))
            return std::make_unique<GroundedExpression>(std::make_unique<SemanticTextGrounding>(
                SemanticTextGrounding::getTheUnquotedText(headStr), SemanticLanguageEnum::UNKNOWN, true));
    } else if (chunkHeadIGram.word.partOfSpeech == PartOfSpeech::PRONOUN
               || chunkHeadIGram.word.partOfSpeech == PartOfSpeech::PRONOUN_SUBJECT
               || chunkHeadIGram.word.partOfSpeech == PartOfSpeech::PRONOUN_COMPLEMENT) {
        RelativePerson pronounRelPerson = fConfiguration.getFlsChecker().pronounGetPerson(chunkHeadIGram);

        if (chunkHeadIGram.word.partOfSpeech == PartOfSpeech::PRONOUN && pronounRelPerson != RelativePerson::FIRST_SING
            && pronounRelPerson != RelativePerson::SECOND_SING
            && pContext.holdingSentenceVerbTense != SemanticVerbTense::UNKNOWN
            && ConceptSet::haveAConcept(chunkHeadIGram.infos.concepts, "reflexive")) {
            if (pContext.holdingGrdExpPtr != nullptr) {
                auto itSubject = pContext.holdingGrdExpPtr->children.find(GrammaticalType::SUBJECT);
                if (itSubject != pContext.holdingGrdExpPtr->children.end()
                    && !SemExpGetter::isACoreference(*itSubject->second, CoreferenceDirectionEnum::UNKNOWN))
                    return itSubject->second->clone();
            }
            return std::make_unique<GroundedExpression>(std::make_unique<SemanticConceptualGrounding>("reflexive"));
        }

        // it's a pronoun that refer to to a person
        const bool isAQuestionWord = pChunk.children.empty()
                                  && fLingDico.aloneWordToRequest(chunkHeadIGram.word) != SemanticRequestType::NOTHING;
        if (!isAQuestionWord && !ConceptSet::haveAConcept(chunkHeadIGram.infos.concepts, "reference_other")) {
            auto pronSemExp = xTranslateRelativePersToPerson(pronounRelPerson, chunkHeadIGram, pContext, pGeneral);
            if (pContext.grammTypeFromParent != GrammaticalType::RECEIVER
                && chunkHeadIGram.infos.hasContextualInfo(WordContextualInfos::POSSESSIVE)) {
                auto corefGrdExp = std::make_unique<GroundedExpression>([] {
                    auto corefGenGrd = std::make_unique<SemanticGenericGrounding>();
                    corefGenGrd->referenceType = SemanticReferenceType::DEFINITE;
                    corefGenGrd->coreference.emplace();
                    return corefGenGrd;
                }());
                corefGrdExp->children.emplace(GrammaticalType::OWNER, std::move(pronSemExp));
                return corefGrdExp;
            }
            return pronSemExp;
        }
    } else if (chunkHeadIGram.word.partOfSpeech == PartOfSpeech::DETERMINER) {
        RelativePerson relPers = ConceptSet::conceptsToRelativePerson(chunkHeadIGram.infos.concepts);
        if (relPers != RelativePerson::UNKNOWN)
            return xTranslateRelativePersToPerson(relPers, chunkHeadIGram, pContext, pGeneral);
    } else if ((chunkHeadIGram.word.partOfSpeech == PartOfSpeech::ADJECTIVE
                || chunkHeadIGram.word.partOfSpeech == PartOfSpeech::ADVERB)
               && pChunk.tokRange.getItBegin()->getPartOfSpeech() != PartOfSpeech::DETERMINER
               && _shouldConceptsBeInsideAConceptualGrounding(
                   ConceptSet::haveAConceptThatBeginWith(chunkHeadIGram.infos.concepts, "reference_"),
                   chunkHeadIGram.infos.concepts)) {
        auto res = std::make_unique<GroundedExpression>(
            std::make_unique<SemanticConceptualGrounding>(chunkHeadIGram.infos.concepts));
        xAddModifiers(res, pContext, pChunk, true);
        if (headBeginWithTimeConcept
            && (res->children.count(GrammaticalType::SPECIFIER) > 0
                || (languageType == SemanticLanguageEnum::ENGLISH
                    && ConceptSet::haveAConcept(chunkHeadIGram.infos.concepts, "fromRecentContext")))) {
            auto timeGrdOpt = xConvertToTimeGrounding(
                pContext, pGeneral, pChunk, SemanticReferenceType::UNDEFINED, chunkHeadIGram.infos.concepts);
            if (timeGrdOpt)
                return std::move(*timeGrdOpt);
        }
        return res;
    } else if (chunkHeadIGram.word.partOfSpeech == PartOfSpeech::PROPER_NOUN
               && !doesChunkHaveDeterminerBeforeHead(pChunk)
               && !ConceptSet::haveAConceptThatBeginWith(headConcepts, "country_")) {
        if (chunkHeadIGram.word.lemma.size() > 1 && chunkHeadIGram.word.lemma[0] == '@') {
            std::string userId;
            if (ConceptSet::extractUserId(userId, chunkHeadIGram.infos.concepts))
                return std::make_unique<GroundedExpression>(std::make_unique<SemanticAgentGrounding>(userId));
        }
        std::list<std::string> names(1, chunkHeadIGram.word.lemma);
        if (languageType == SemanticLanguageEnum::FRENCH) {
            TokIt currTok = pChunk.head;
            _completeWithFollowingProperNouns(names, currTok, pChunk.tokRange.getItEnd());
        } else {
            TokIt currTok = pChunk.head;
            while (currTok != pChunk.tokRange.getItBegin()) {
                currTok = getPrevToken(currTok, pChunk.tokRange.getItBegin(), pChunk.head);
                const SemanticWord& currTokWord = currTok->inflWords.front().word;
                if (currTokWord.partOfSpeech == PartOfSpeech::PROPER_NOUN) {
                    names.emplace_front(currTokWord.lemma);
                    continue;
                }
                break;
            }
        }

        TokIt itTokBeforeHead = pChunk.tokRange.getItBegin();
        const InflectedWord& headInflWord = itTokBeforeHead->inflWords.front();
        bool hasASpecificUserId = pGeneral.agentWeAreTalkingAbout && itTokBeforeHead->tokenPos.isAtBegin();
        auto res = [&] {
            if (hasASpecificUserId) {
                const std::string userIdWithoutContext = SemanticAgentGrounding::namesToUserId(names);
                auto agentGrd = std::make_unique<SemanticAgentGrounding>(
                    pGeneral.agentWeAreTalkingAbout->userId, userIdWithoutContext, names);
                fConfiguration.getFlsChecker().initGenderSetFromIGram(agentGrd->nameInfos->possibleGenders,
                                                                      headInflWord);
                return std::make_unique<GroundedExpression>(std::move(agentGrd));
            } else {
                auto nameGrd = SemExpGenerator::makeNameGrd(names, &headConcepts);
                fConfiguration.getFlsChecker().initGenderSetFromIGram(nameGrd->nameInfos.possibleGenders, headInflWord);
                return std::make_unique<GroundedExpression>(std::move(nameGrd));
            }
        }();
        bool lookAtTokenAfterTheHead = languageType != SemanticLanguageEnum::FRENCH;
        xConsiderDeterminerAtTheBeginningOfAGrounding(*res, pChunk, pContext, pGeneral);
        xAddModifiers(res, pContext, pChunk, lookAtTokenAfterTheHead);

        if (hasASpecificUserId) {
            auto agentWeAreTalkingAbout = std::move(pGeneral.agentWeAreTalkingAbout);
            pGeneral.agentWeAreTalkingAbout.reset();
            return std::make_unique<InterpretationExpression>(
                InterpretationSource::FIRSTAGENTOFTEXT,
                std::make_unique<GroundedExpression>(std::move(agentWeAreTalkingAbout)),
                std::move(res));
        }
        return res;
    } else if (chunkHeadIGram.word.partOfSpeech == PartOfSpeech::ADJECTIVE
               || chunkHeadIGram.word.partOfSpeech == PartOfSpeech::NOUN) {
        auto langaugeGrdExp = _tryToConvertLanguageChunk(chunkHeadIGram.infos.concepts);
        if (langaugeGrdExp) {
            xConsiderDeterminerAtTheBeginningOfAGrounding(*langaugeGrdExp, pChunk, pContext, pGeneral);
            return langaugeGrdExp;
        }
    }

    auto res = std::make_unique<GroundedExpression>(std::make_unique<SemanticGenericGrounding>());
    SemanticGenericGrounding& genGrounding = res->grounding().getGenericGrounding();
    PartOfSpeech headPartOfSpeech = chunkHeadIGram.word.partOfSpeech;
    SemExpModifier::fillSemanticConcepts(genGrounding.concepts, headConcepts);
    if (!pChunk.head->str.empty() && pChunk.head->str[0] == SemanticMetaGrounding::firstCharOfStrOld) {
        int paramId = 0;
        std::string label;
        std::string attributeName;
        if (SemanticMetaGrounding::parseParameterOld(paramId, label, attributeName, pChunk.head->str)
            && label == "number") {
            genGrounding.quantity.setNumberToFill(paramId, attributeName);
            genGrounding.entityType = SemanticEntityType::NUMBER;
            return res;
        }
    }
    xInitGenGroundingsFromToken(genGrounding, pChunk.head, pChunk.tokRange.getItEnd());

    const InflectedWord& headInflWord = pChunk.head->inflWords.front();
    if (headInflWord.word.partOfSpeech != PartOfSpeech::PROPER_NOUN) {
        if (languageType == SemanticLanguageEnum::ENGLISH && headInflWord.word.partOfSpeech == PartOfSpeech::NOUN) {
            auto synthgraphRequest = pContext.getSynthGraphRequest();
            auto gramChildOfTheRequest = semanticRequestType_toSemGram(synthgraphRequest);
            auto holdingSentenceRequest = pContext.holdingSentenceRequests.firstOrNothing();
            auto synthGramOfTheChild = semanticRequestType_toSemGram(holdingSentenceRequest);
            auto childGrammTypeOpt = chunkTypeToGrammaticalType(pChunkLink.type);
            if (genGrounding.entityType != SemanticEntityType::NUMBER && pChunkLink.type != ChunkLinkType::IN_CASE_OF
                && pChunkLink.type != ChunkLinkType::OWNER && pChunkLink.type != ChunkLinkType::SPECIFICATION
                && (pChunkLink.type != ChunkLinkType::DIRECTOBJECT
                    || holdingSentenceRequest != SemanticRequestType::QUANTITY)
                && (!childGrammTypeOpt
                    || ((gramChildOfTheRequest == GrammaticalType::UNKNOWN
                         || *childGrammTypeOpt != gramChildOfTheRequest)
                        && (synthGramOfTheChild == GrammaticalType::UNKNOWN
                            || *childGrammTypeOpt != synthGramOfTheChild)))
                && !doesHaveAPos(pChunk.tokRange, PartOfSpeech::PROPER_NOUN)) {
                if (genGrounding.quantity.isPlural())
                    genGrounding.referenceType = SemanticReferenceType::INDEFINITE;
                else if ((haveChild(pChunk, ChunkLinkType::OWNER) || haveChild(pChunk, ChunkLinkType::TIME)
                          || (pContext.holdingGrdExpPtr != nullptr
                              && !ConceptSet::haveAConcept(pContext.holdingGrdExpPtr->grounding().concepts,
                                                           "verb_action_say")
                              && !ConceptSet::haveAConceptThatBeginWithAnyOf(
                                  headInflWord.infos.concepts,
                                  {"accordance_", "engagement_", "gender_", "location_", "mentalState_", "number_"}))))
                    genGrounding.referenceType = SemanticReferenceType::DEFINITE;
            }
        }

        for (TokIt itTok = pChunkLink.tokRange.getItBegin(); itTok != pChunkLink.tokRange.getItEnd();
             itTok = getNextToken(itTok, pChunkLink.tokRange.getItEnd())) {
            const InflectedWord& linkWorkIGram = itTok->inflWords.front();
            if (linkWorkIGram.word.partOfSpeech == PartOfSpeech::PREPOSITION) {
                if (ConceptSet::haveAConcept(linkWorkIGram.infos.concepts, "reference_definite"))
                    genGrounding.referenceType = SemanticReferenceType::DEFINITE;
                else if (languageType == SemanticLanguageEnum::FRENCH
                         && linkWorkIGram.infos.hasContextualInfo(WordContextualInfos::UNCOUNTABLE)) {
                    if (genGrounding.quantity.type == SemanticQuantityType::NUMBER)
                        genGrounding.quantity.clear();
                }
            }
        }
    }

    switch (headPartOfSpeech) {
        case PartOfSpeech::ADJECTIVE:
        case PartOfSpeech::ADVERB: {
            genGrounding.entityType = SemanticEntityType::MODIFIER;
            if (SemExpModifier::fillQuantityAndReferenceFromConcepts(
                    genGrounding.quantity, genGrounding.referenceType, chunkHeadIGram.infos.concepts)) {
                genGrounding.word.clear();
                genGrounding.entityType = SemanticEntityType::UNKNOWN;
            }
            if (SemExpModifier::fillReferenceFromConcepts(genGrounding.referenceType, chunkHeadIGram.infos.concepts)) {
                genGrounding.entityType = SemanticEntityType::UNKNOWN;
            }
            break;
        }
        case PartOfSpeech::PREPOSITION: {
            genGrounding.entityType = SemanticEntityType::UNKNOWN;
            break;
        }
        case PartOfSpeech::VERB: {
            auto statementGr = std::make_unique<SemanticStatementGrounding>();
            statementGr->concepts = genGrounding.concepts;
            statementGr->polarity = genGrounding.polarity;
            statementGr->word = genGrounding.word;
            bool isVerbAtImperative = false;
            if (pContext.holdingVerbChunkPtr != nullptr
                && ConceptSet::verbSyntesisIsFollowedByAnInfinitiveThatMeansAnImperative(
                    pContext.holdingVerbChunkPtr->getHeadConcepts())) {
                auto* indObjChkLinkPtr =
                    getChunkLinkWithAuxSkip(*pContext.holdingVerbChunkPtr, ChunkLinkType::INDIRECTOBJECT);
                if (indObjChkLinkPtr != nullptr) {
                    {
                        ToGenRepContext subContext(pContext, *indObjChkLinkPtr, *indObjChkLinkPtr->chunk);
                        subContext.grammTypeFromParent = GrammaticalType::SUBJECT;
                        xAddNewGrammInfo(*res, pGeneral, subContext);
                    }
                    statementGr->requests.set(SemanticRequestType::ACTION);
                    statementGr->verbTense = SemanticVerbTense::PUNCTUALPRESENT;
                    isVerbAtImperative = true;
                }
            }
            if (!isVerbAtImperative)
                statementGr->verbTense = SemanticVerbTense::UNKNOWN;
            res->moveGrounding(std::move(statementGr));
            xAddModifiers(res, pContext, pChunk, true);
            return res;
        }
        case PartOfSpeech::DETERMINER:
        case PartOfSpeech::PARTITIVE: {
            SemExpModifier::fillReferenceFromConcepts(genGrounding.referenceType, chunkHeadIGram.infos.concepts);
            genGrounding.entityType = SemanticEntityType::UNKNOWN;
            genGrounding.coreference.emplace(CoreferenceDirectionEnum::AFTER);
            genGrounding.quantity.clear();
            break;
        }
        case PartOfSpeech::PROPER_NOUN: {
            auto wordInLowerCase = genGrounding.word.lemma;
            if (lowerCaseText(wordInLowerCase)) {
                auto lowerCaseStaticLingMeaning =
                    fLingDico.statDb.getLingMeaning(wordInLowerCase, PartOfSpeech::NOUN, false);

                if (lowerCaseStaticLingMeaning.meaningId != LinguisticMeaning_noMeaningId) {
                    auto lowerCaseLingMeaning = LinguisticMeaning(lowerCaseStaticLingMeaning);
                    linguistics::InflectedWord lowerCaseInflectedWord;
                    fLingDico.getInfoGram(lowerCaseInflectedWord, lowerCaseLingMeaning);
                    genGrounding.concepts.insert(lowerCaseInflectedWord.infos.concepts.begin(),
                                                 lowerCaseInflectedWord.infos.concepts.end());
                }
            }
            break;
        }
        default: break;
    }

    bool askWhatIs = false;
    TokIt itTokBeforeHead = pChunk.tokRange.getItBegin();
    bool knowReferenceForSure = false;
    while (itTokBeforeHead != pChunk.head) {
        const Token& detToken = *itTokBeforeHead;
        const InflectedWord& introInflWord = detToken.inflWords.front();
        PartOfSpeech introGram = introInflWord.word.partOfSpeech;

        switch (introGram) {
            case PartOfSpeech::DETERMINER:
            case PartOfSpeech::PARTITIVE: {
                itTokBeforeHead = xAddDeterminerToAGrounding(
                    *res, genGrounding, knowReferenceForSure, itTokBeforeHead, pChunk.head, pContext, pGeneral);
                break;
            }
            case PartOfSpeech::PREPOSITION: {
                if (introInflWord.infos.hasContextualInfo(WordContextualInfos::UNCOUNTABLE)
                    && genGrounding.quantity.type == SemanticQuantityType::NUMBER)
                    genGrounding.quantity.clear();
                break;
            }
            case PartOfSpeech::ADVERB: {
                if (introInflWord.infos.hasContextualInfo(WordContextualInfos::NEGATION))
                    genGrounding.polarity = !genGrounding.polarity;
                break;
            }
            case PartOfSpeech::ADJECTIVE: {
                if (ConceptSet::haveAConcept(introInflWord.infos.concepts, "quantity_nothing")) {
                    genGrounding.referenceType = SemanticReferenceType::INDEFINITE;
                    genGrounding.quantity.setNumber(0);
                }
                break;
            }
            case PartOfSpeech::PROPER_NOUN: {
                if (genGrounding.referenceType == SemanticReferenceType::INDEFINITE) {
                    auto newGrdExp = std::make_unique<GroundedExpression>(std::make_unique<SemanticGenericGrounding>());
                    SemanticGenericGrounding& genGrounding = newGrdExp->grounding().getGenericGrounding();
                    xInitGenGroundingsFromToken(genGrounding, itTokBeforeHead, pChunk.head);
                    SemExpModifier::addChild(*res, GrammaticalType::SPECIFIER, std::move(newGrdExp));
                } else {
                    std::list<std::string> names(1, introInflWord.word.lemma);
                    _completeWithFollowingProperNouns(names, itTokBeforeHead, pChunk.head);
                    SemExpModifier::addChild(*res,
                                             GrammaticalType::SPECIFIER,
                                             std::make_unique<GroundedExpression>(SemExpGenerator::makeNameGrd(names)));
                }
                break;
            }
            case PartOfSpeech::PRONOUN: {
                auto* qWordPtr = fLingDico.statDb.wordToQuestionWord(introInflWord.word, false, false);
                if (qWordPtr != nullptr && !qWordPtr->followedByRequestedWord
                    && qWordPtr->request == SemanticRequestType::OBJECT)
                    askWhatIs = true;
                break;
            }
            default: break;
        }
        itTokBeforeHead = getNextToken(itTokBeforeHead, pChunk.head);
    }

    if (headBeginWithTimeConcept
        && (genGrounding.coreference || genGrounding.referenceType == SemanticReferenceType::UNDEFINED
            || pChunk.children.empty() || res->children.count(GrammaticalType::SPECIFIER) > 0)) {
        auto timeGrdOpt =
            xConvertToTimeGrounding(pContext, pGeneral, pChunk, genGrounding.referenceType, genGrounding.concepts);
        if (timeGrdOpt)
            return std::move(*timeGrdOpt);
    }

    xAddModifiers(res, pContext, pChunk, true);
    if (askWhatIs)
        return SemExpGenerator::whatIs(std::move(res));
    return res;
}

void SyntacticGraphToSemantic::xAddModifiers(std::unique_ptr<GroundedExpression>& pGrdExpPtr,
                                             ToGenRepContext& pContext,
                                             const Chunk& pChunk,
                                             bool pLookAtTokenAfterTheHead) const {
    // add the modifiers
    for (TokIt it = pChunk.tokRange.getItBegin(); it != pChunk.head; it = getNextToken(it, pChunk.head)) {
        const InflectedWord& currIGram = it->inflWords.front();
        if ((currIGram.word.partOfSpeech == PartOfSpeech::ADVERB
             || currIGram.word.partOfSpeech == PartOfSpeech::ADJECTIVE
             || currIGram.word.partOfSpeech == PartOfSpeech::NOUN
             || currIGram.word.partOfSpeech == PartOfSpeech::UNKNOWN)
            && !currIGram.infos.hasContextualInfo(WordContextualInfos::NEGATION)
            && currIGram.infos.concepts.count("quantity_nothing") == 0) {
            if (currIGram.infos.concepts.count("reference_other") > 0) {
                auto corefGrounding = pGrdExpPtr->cloneGrounding();
                _refactorToAOtherThanGrd(*corefGrounding);
                SemExpModifier::addChild(*pGrdExpPtr,
                                         GrammaticalType::OTHER_THAN,
                                         std::make_unique<GroundedExpression>(std::move(corefGrounding)));
            } else {
                if (currIGram.word.partOfSpeech == PartOfSpeech::ADVERB) {
                    auto* qWordPtr = fLingDico.statDb.wordToQuestionWord(currIGram.word, false, false);
                    if (qWordPtr != nullptr) {
                        auto statGrd = std::make_unique<SemanticStatementGrounding>();
                        statGrd->verbTense = SemanticVerbTense::PRESENT;
                        statGrd->requests.set(qWordPtr->request);
                        statGrd->coreference.emplace();
                        auto newGrdExp = std::make_unique<GroundedExpression>(std::move(statGrd));
                        auto childGramType = GrammaticalType::OBJECT;
                        if (pGrdExpPtr->grounding().type == SemanticGroundingType::UNITY)
                            childGramType = GrammaticalType::UNITY;
                        newGrdExp->children.emplace(childGramType, std::move(pGrdExpPtr));
                        pGrdExpPtr = std::move(newGrdExp);
                        continue;
                    }
                }
                SemExpModifier::addChild(
                    *pGrdExpPtr,
                    GrammaticalType::SPECIFIER,
                    std::make_unique<GroundedExpression>([&]() -> std::unique_ptr<SemanticGrounding> {
                        // TODO: redo it but think of "droit" "droite" problem
                        // if (_shouldConceptsBeInsideAConcpetualGrounding(currIGram.infos.concepts))
                        //  return std::make_unique<SemanticConceptualGrounding>(currIGram.infos.concepts);
                        auto genGrounding = std::make_unique<SemanticGenericGrounding>();
                        SemExpModifier::fillSemanticConcepts(genGrounding->concepts, currIGram.infos.concepts);
                        xInitGenGroundingsFromToken(*genGrounding, it, pChunk.head);
                        return genGrounding;
                    }()));
            }
        }
    }

    if (pLookAtTokenAfterTheHead) {
        bool firstIteration = true;
        TokIt itEnd = pChunk.tokRange.getItEnd();
        for (TokIt it = getNextToken(pChunk.head, itEnd, SkipPartOfWord::YES); it != pChunk.tokRange.getItEnd();
             it = getNextToken(it, itEnd, SkipPartOfWord::YES)) {
            const InflectedWord& currTokInflWord = it->inflWords.front();
            if (currTokInflWord.word.partOfSpeech == PartOfSpeech::PROPER_NOUN) {
                std::list<std::string> names(1, currTokInflWord.word.lemma);
                _completeWithFollowingProperNouns(names, it, pChunk.tokRange.getItEnd());
                auto properNounGrdExp = std::make_unique<GroundedExpression>(SemExpGenerator::makeNameGrd(names));
                if (firstIteration) {
                    SemExpModifier::addChild(*pGrdExpPtr, GrammaticalType::SUB_CONCEPT, std::move(properNounGrdExp));
                } else {
                    SemExpModifier::addEmptyIntroductingWord(*properNounGrdExp);
                    SemExpModifier::addChild(*pGrdExpPtr, GrammaticalType::SPECIFIER, std::move(properNounGrdExp));
                }
            } else {
                xAddModifiersOfATokenAfterVerb(*pGrdExpPtr, pContext, it, itEnd);
            }
            firstIteration = false;
        }
    }
}

void SyntacticGraphToSemantic::xAddModifiersOfATokenAfterVerb(GroundedExpression& pGrdExp,
                                                              ToGenRepContext& pContext,
                                                              TokIt pItToken,
                                                              const TokIt& pItEndToken) const {
    const InflectedWord& firstTokIGram = pItToken->inflWords.front();
    if (firstTokIGram.word.partOfSpeech == PartOfSpeech::ADVERB
        || firstTokIGram.word.partOfSpeech == PartOfSpeech::ADJECTIVE) {
        if (!firstTokIGram.infos.hasContextualInfo(WordContextualInfos::NEGATION)
            && !(fLingDico.getLanguage() == SemanticLanguageEnum::FRENCH && pContext.holdingVerbChunkPtr != nullptr
                 && !pContext.holdingVerbChunkPtr->positive && firstTokIGram.word.lemma == "plus")) {
            auto genGrounding = std::make_unique<SemanticGenericGrounding>();
            SemExpModifier::fillSemanticConcepts(genGrounding->concepts, firstTokIGram.infos.concepts);
            xInitGenGroundingsFromToken(*genGrounding, pItToken, pItEndToken);
            GrammaticalType childGrammType = GrammaticalType::SPECIFIER;
            if (pGrdExp->type == SemanticGroundingType::STATEMENT) {
                if (ConceptSet::haveAConceptOrAHyponym(genGrounding->concepts, "manner"))
                    childGrammType = GrammaticalType::MANNER;
                else if (ConceptSet::haveAConceptOrAHyponym(genGrounding->concepts, "location"))
                    childGrammType = GrammaticalType::LOCATION;
            }
            const bool hasReferenceConcept =
                SemExpModifier::fillReferenceFromConcepts(genGrounding->referenceType, firstTokIGram.infos.concepts);
            auto& rootGrdExp = [&]() -> GroundedExpression& {
                if (pContext.grammTypeFromParent == GrammaticalType::SPECIFIER && pContext.holdingGrdExpPtr != nullptr
                    && firstTokIGram.word.partOfSpeech == PartOfSpeech::ADJECTIVE) {
                    SemanticReferenceType grdExpReference = SemExpGetter::getReferenceTypeFromGrd(*pGrdExp);
                    if (grdExpReference == SemanticReferenceType::UNDEFINED) {
                        SemanticNumberType grdExpNumber = SemExpGetter::getNumberFromGrd(*pGrdExp);
                        if (!InflectionsChecker::isAdjCompatibleWithNumberType(firstTokIGram.inflections(),
                                                                               grdExpNumber))
                            return *pContext.holdingGrdExpPtr;
                    }
                }
                return pGrdExp;
            }();
            auto& rootGrdExpForAdj = _getRootForNextdjective(rootGrdExp);
            if (_shouldConceptsBeInsideAConceptualGrounding(hasReferenceConcept, genGrounding->concepts)) {
                SemExpModifier::addChild(rootGrdExpForAdj,
                                         childGrammType,
                                         std::make_unique<GroundedExpression>(
                                             std::make_unique<SemanticConceptualGrounding>(genGrounding->concepts)));
            } else {
                SemExpModifier::addChild(
                    rootGrdExpForAdj, childGrammType, std::make_unique<GroundedExpression>(std::move(genGrounding)));
            }
        }
    } else if (firstTokIGram.word.partOfSpeech != PartOfSpeech::PRONOUN
               && firstTokIGram.word.partOfSpeech != PartOfSpeech::PRONOUN_COMPLEMENT) {
        auto genGrounding = std::make_unique<SemanticGenericGrounding>();
        SemExpModifier::fillSemanticConcepts(genGrounding->concepts, firstTokIGram.infos.concepts);
        xInitGenGroundingsFromToken(*genGrounding, pItToken, pItEndToken);
        auto newSpecSGrdExp = std::make_unique<GroundedExpression>(std::move(genGrounding));
        SemExpModifier::addEmptyIntroductingWord(*newSpecSGrdExp);
        SemExpModifier::addChild(pGrdExp, GrammaticalType::SPECIFIER, std::move(newSpecSGrdExp));
    }
}

mystd::unique_propagate_const<UniqueSemanticExpression> SyntacticGraphToSemantic::xFillSemExp(
    ToGenRepGeneral& pGeneral,
    ToGenRepContext& pContext) const {
    // if we already encounter this chunk we re-take the previous context
    if (!pContext.chunk.hasOnlyOneReference) {
        auto it = pGeneral.prevChunks.find(&pContext.chunk);
        if (it != pGeneral.prevChunks.end())
            return SemExpGetter::getASimplifiedVersion(*it->second);
    }

    LinguisticSubordinateId linguisticSubordinateId;
    if (!chunkTypeIsAList(pContext.chunk.type)) {
        linguisticSubordinateId.chunkLinkType = pContext.chLink.type;
        _fillRelativeCharEncodedFromTokenRange(linguisticSubordinateId, pContext.chLink.tokRange);
        if (linguisticSubordinateId.relativeSubodinate == noRelativeSubordinate)
            _fillRelativeCharEncodedFromTokenRange(linguisticSubordinateId, pContext.chunk.tokRange);
    }

    mystd::unique_propagate_const<UniqueSemanticExpression> res;
    switch (pContext.chunk.type) {
        case ChunkType::INTERJECTION_CHUNK: {
            auto uSemExp = xConvertInterjectionChunk(pGeneral, pContext);
            if (pGeneral.textProcContext.writeOriginalText)
                uSemExp->fromText = pContext.chunk.getTokRangeWrappingChildren().toStr();
            res = mystd::unique_propagate_const<UniqueSemanticExpression>(std::move(uSemExp));
            break;
        }
        case ChunkType::NOMINAL_CHUNK:
        case ChunkType::PREPOSITIONAL_CHUNK: {
            auto uSemExp = xConvertNominalChunk(pGeneral, pContext);
            if (pGeneral.textProcContext.writeOriginalText)
                uSemExp->fromText = pContext.chunk.getTokRangeWrappingChildren().toStr();
            res = mystd::unique_propagate_const<UniqueSemanticExpression>(std::move(uSemExp));
            break;
        }
        case ChunkType::VERB_CHUNK:
        case ChunkType::INFINITVE_VERB_CHUNK: {
            auto sentence = xInitNewSentence(pGeneral, pContext, pContext.chunk);
            GroundedExpression& sentenceGrdExp = *sentence;
            std::map<GrammaticalType, std::unique_ptr<ConditionExpression>> childTypeToCondition;
            xFillNewSentence(sentenceGrdExp, childTypeToCondition, pGeneral, pContext);
            if (pContext.isAtRoot) {
                GrammaticalType subjectGrammType = GrammaticalType::SUBJECT;
                if (pContext.chunk.isPassive)
                    subjectGrammType = GrammaticalType::OBJECT;
                SemanticStatementGrounding* statGrdPtr = sentenceGrdExp->getStatementGroundingPtr();
                if (statGrdPtr != nullptr && sentenceGrdExp.children.count(subjectGrammType) == 0) {
                    SemanticStatementGrounding& statGrd = *statGrdPtr;
                    if (statGrd.requests.empty()) {
                        if (statGrd.verbTense != SemanticVerbTense::UNKNOWN)
                            sentenceGrdExp.children.emplace(
                                subjectGrammType,
                                SemExpGenerator::makeCoreferenceExpression(CoreferenceDirectionEnum::BEFORE));
                    }
                }
            }
            xAfterCreationModificationsOnSentence(sentenceGrdExp, pContext, pGeneral);
            SemanticLanguageEnum languageType = fConfiguration.getLanguageType();

            std::unique_ptr<SemanticExpression> unsRes;
            if (languageType == SemanticLanguageEnum::ENGLISH) {
                forEachAuxiliaryChunks(pContext.chunk, [&](const Chunk& pAuxChunk) {
                    const InflectedWord& inflAux = pAuxChunk.head->inflWords.front();
                    const auto& auxConcepts = inflAux.infos.concepts;
                    if (!semTreeHardCodedConverter::manageEnglishAuxiliaries(sentenceGrdExp, auxConcepts)
                        && inflAux.word.lemma == "may")
                        SemExpModifier::fillVerbGoal(sentenceGrdExp, VerbGoalEnum::POSSIBILITY);
                });
                unsRes = semTreeHardCodedConverter::convertEnglishSentenceToASemExp(std::move(sentence));
                semTreeHardCodedConverter::refactorEnglishSentencesWithAGoal(unsRes, childTypeToCondition);
            } else if (languageType == SemanticLanguageEnum::FRENCH) {
                unsRes = std::move(sentence);
                semTreeHardCodedConverter::refactorFrenchSentencesWithAGoal(
                    unsRes, childTypeToCondition, pGeneral.textProcContext, pContext.chunk.isPassive);
            }

            if (unsRes) {
                UniqueSemanticExpression resSemExp(std::move(unsRes));
                _refactorThEPurposeOfPatterns(resSemExp, pGeneral, pContext);
                xFillSentenceSubordonates(
                    resSemExp, childTypeToCondition, GrammaticalType::UNKNOWN, pGeneral, pContext);
                _fillConditions(resSemExp, childTypeToCondition, pContext.chunk, pGeneral.textProcContext.writeOriginalText);
                res = mystd::unique_propagate_const<UniqueSemanticExpression>(std::move(resSemExp));
            }
            break;
        }
        case ChunkType::AND_CHUNK:
        case ChunkType::OR_CHUNK:
        case ChunkType::THEN_CHUNK:
        case ChunkType::THEN_REVERSED_CHUNK: {
            auto uSemExp = xConvertListChunk(pGeneral, pContext);
            if (pGeneral.textProcContext.writeOriginalText)
                uSemExp->fromText = pContext.chunk.getTokRangeWrappingChildren().toStr();
            res = mystd::unique_propagate_const<UniqueSemanticExpression>(std::move(uSemExp));
            break;
        }
        case ChunkType::TEACH_CHUNK: {
            auto semExpWrapped = std::make_unique<GroundedExpression>();
            for (const ChunkLink& currChkLk : pContext.chunk.children) {
                ToGenRepContext subContext(pContext, currChkLk, *currChkLk.chunk);
                subContext.holdingGrdExpPtr = &*semExpWrapped;

                auto gramTypeOptional = chunkTypeToGrammaticalType(currChkLk.type);
                subContext.grammTypeFromParent = gramTypeOptional ? *gramTypeOptional : GrammaticalType::SUBORDINATE;
                xAddNewGrammInfo(*semExpWrapped, pGeneral, subContext);
            }
            if (pGeneral.textProcContext.writeOriginalText)
                semExpWrapped->fromText = pContext.chunk.getTokRangeWrappingChildren().toStr();
            res = mystd::unique_propagate_const<UniqueSemanticExpression>(std::move(semExpWrapped));
        }
        case ChunkType::SEPARATOR_CHUNK:
        case ChunkType::AUX_CHUNK: break;
    }

    // same the context, if the chunk has many fathers
    if (!res)
        return mystd::unique_propagate_const<UniqueSemanticExpression>();

    if (!pContext.chunk.hasOnlyOneReference) {
        pGeneral.prevChunks.emplace(&pContext.chunk, res->getSharedPtr());
    }

    if (linguisticSubordinateId.relativeSubodinate != noRelativeSubordinate) {
        auto grdExp = [&] {
            switch (linguisticSubordinateId.chunkLinkType) {
                case ChunkLinkType::LOCATION:
                    return std::make_unique<GroundedExpression>(std::make_unique<SemanticRelativeLocationGrounding>(
                        semanticRelativeLocationType_fromChar(linguisticSubordinateId.relativeSubodinate)));
                case ChunkLinkType::TIME:
                    return std::make_unique<GroundedExpression>(std::make_unique<SemanticRelativeTimeGrounding>(
                        semanticRelativeTimeType_fromChar(linguisticSubordinateId.relativeSubodinate)));
                case ChunkLinkType::DURATION:
                    return std::make_unique<GroundedExpression>(std::make_unique<SemanticRelativeDurationGrounding>(
                        semanticRelativeDurationType_fromChar(linguisticSubordinateId.relativeSubodinate)));
                default: return std::unique_ptr<GroundedExpression>();
            }
        }();
        if (grdExp) {
            grdExp->children.emplace(GrammaticalType::SPECIFIER, std::move(*res));
            return mystd::unique_propagate_const<UniqueSemanticExpression>(std::move(grdExp));
        }
    } else if (pContext.chunk.introductingWordToSaveForSynthesis) {
        SemExpModifier::addChildFromSemExp(
            **res, GrammaticalType::INTRODUCTING_WORD, std::make_unique<GroundedExpression>([&pContext] {
                auto genGrd = std::make_unique<SemanticGenericGrounding>();
                if (*pContext.chunk.introductingWordToSaveForSynthesis != nullptr)
                    genGrd->word = **pContext.chunk.introductingWordToSaveForSynthesis;
                return genGrd;
            }()));
    }
    return res;
}

void SyntacticGraphToSemantic::_refactorThEPurposeOfPatterns(UniqueSemanticExpression& pUSemExp,
                                                             ToGenRepGeneral& pGeneral,
                                                             const ToGenRepContext& pContext) const {
    auto* grdExpPtr = pUSemExp->getGrdExpPtr_SkipWrapperPtrs();
    if (grdExpPtr != nullptr) {
        auto* statGrdPtr = grdExpPtr->grounding().getStatementGroundingPtr();
        if (statGrdPtr != nullptr && statGrdPtr->verbTense == SemanticVerbTense::UNKNOWN) {
            auto itPurposeOfChild = linguistics::getChunkLink(pContext.chunk, linguistics::ChunkLinkType::PURPOSE_OF);
            if (itPurposeOfChild != pContext.chunk.children.end()) {
                ToGenRepContext subContext(pContext, *itPurposeOfChild, *itPurposeOfChild->chunk);
                auto purposeOfSemExpOpt = xFillSemExp(pGeneral, subContext);
                if (purposeOfSemExpOpt) {
                    auto* purposeGrdExpPtr = (*purposeOfSemExpOpt)->getGrdExpPtr_SkipWrapperPtrs();
                    if (purposeGrdExpPtr != nullptr) {
                        SemExpModifier::addChild(*purposeGrdExpPtr, GrammaticalType::PURPOSE, std::move(pUSemExp));
                        pUSemExp = std::move(*purposeOfSemExpOpt);
                        return;
                    }

                    auto res = std::make_unique<AnnotatedExpression>(std::move(*purposeOfSemExpOpt));
                    res->synthesizeAnnotations = true;
                    res->annotations.emplace(GrammaticalType::PURPOSE, std::move(pUSemExp));
                    pUSemExp = std::move(res);
                    return;
                }
            }
        }
    }
}

mystd::unique_propagate_const<UniqueSemanticExpression> SyntacticGraphToSemantic::xConvertSeparatorChunkToSemExp(
    ToGenRepGeneral& pGeneral,
    ToGenRepContext& pContext) const {
    if (ConceptSet::haveAConceptThatBeginWith(pContext.chunk.head->inflWords.front().infos.concepts,
                                              "accordance_agreement_"))
        return xConvertNominalChunkToSemExp(pContext, pGeneral, pContext.chLink, pContext.chunk);
    return mystd::unique_propagate_const<UniqueSemanticExpression>();
}

ChunkLinkType SyntacticGraphToSemantic::xConvertSeparatorChunkToChunkLinkType(
    SyntacticGraphToSemantic::ToGenRepContext& pContext) const {
    const auto& word = pContext.chunk.head->inflWords.front().word;
    auto chLkTypeOpt = fSemFrameDict.introductionWordToChunkLinkType(word);
    if (chLkTypeOpt)
        return *chLkTypeOpt;
    return ChunkLinkType::SIMPLE;
}

UniqueSemanticExpression SyntacticGraphToSemantic::xConvertListChunk(ToGenRepGeneral& pGeneral,
                                                                     ToGenRepContext& pContext) const {
    mystd::unique_propagate_const<UniqueSemanticExpression> interjectionSemExp;
    auto newListExp = std::make_unique<ListExpression>(_chunkTypeToListExpType(pContext.chunk.type));
    bool hasACondition = false;
    SemanticExpression* semExpBeforeListSeparator = nullptr;
    SemanticExpression* semExpAfterListSeparator = nullptr;
    for (auto it = pContext.chunk.children.begin(); it != pContext.chunk.children.end(); ++it) {
        Chunk& currChunk = *it->chunk;
        if (it->type == ChunkLinkType::SIMPLE)    // list elt
        {
            ToGenRepContext subContext(pContext, pContext.chLink, currChunk);
            auto newElt = xFillSemExp(pGeneral, subContext);
            if (newElt
                && (pContext.chunk.children.size() > 1
                    || !SemExpGetter::isACoreference(**newElt, CoreferenceDirectionEnum::UNKNOWN))) {
                SemanticRequestType requestofHoldingChunkLink = xSubBordinateChkLkToRequest(*it);
                if (requestofHoldingChunkLink != SemanticRequestType::NOTHING) {
                    GroundedExpression* newGrdExpPtr = (*newElt)->getGrdExpPtr();
                    if (newGrdExpPtr != nullptr)
                        SemExpModifier::addRequest(*newGrdExpPtr, requestofHoldingChunkLink);
                }

                newListExp->elts.emplace_back(std::move(*newElt));
                if (checkOrder(pContext.chunk, currChunk))
                    semExpAfterListSeparator = &*newListExp->elts.back();
                else
                    semExpBeforeListSeparator = &*newListExp->elts.back();
            }
        } else if (it->type == ChunkLinkType::IF) {
            hasACondition = true;
        } else if (it->type == ChunkLinkType::INTERJECTION) {
            ToGenRepContext subContext(pContext, pContext.chLink, currChunk);
            interjectionSemExp = xFillSemExp(pGeneral, subContext);
        }
    }

    if (semExpBeforeListSeparator != nullptr || semExpAfterListSeparator != nullptr) {
        if (semExpBeforeListSeparator == nullptr) {
            mystd::optional<SemanticEntityType> entityType;
            if (semExpAfterListSeparator != nullptr)
                entityType = SemExpGetter::getEntity(*semExpAfterListSeparator);
            newListExp->elts.emplace_front(
                SemExpGenerator::makeCoreferenceExpression(CoreferenceDirectionEnum::BEFORE, entityType));
        }
        if (semExpAfterListSeparator == nullptr) {
            mystd::optional<SemanticEntityType> entityType;
            if (semExpBeforeListSeparator != nullptr)
                entityType = SemExpGetter::getEntity(*semExpBeforeListSeparator);
            newListExp->elts.emplace_back(
                SemExpGenerator::makeCoreferenceExpression(CoreferenceDirectionEnum::AFTER, entityType));
        }
    }

    UniqueSemanticExpression resSemExp(std::move(newListExp));
    if (hasACondition)
        xTryToFillCondition(resSemExp, pGeneral, pContext);
    if (interjectionSemExp)
        resSemExp = std::make_unique<FeedbackExpression>(std::move(*interjectionSemExp), std::move(resSemExp));
    if (!pContext.chunk.requests.empty()) {
        auto newRes = std::make_unique<GroundedExpression>([&]() {
            auto statRes = std::make_unique<SemanticStatementGrounding>();
            statRes->requests = pContext.chunk.requests;
            return statRes;
        }());
        newRes->children.emplace(GrammaticalType::UNKNOWN, std::move(resSemExp));
        resSemExp = std::move(newRes);
    }
    return resSemExp;
}

SemanticVerbTense SyntacticGraphToSemantic::xChunkToVerbTimeTense(VerbGoalEnum& pVerbGoal,
                                                                  const Chunk& pVerbChunk,
                                                                  ChunkLinkType pChunkLinkType) const {
    SemanticLanguageEnum langType = fConfiguration.getLanguageType();
    if (langType == SemanticLanguageEnum::FRENCH)
        return xChunkToVerbTimeTenseForFrench(pVerbGoal, pVerbChunk, pChunkLinkType);
    return xChunkToVerbTimeTenseForEnglish(pVerbGoal, pVerbChunk, pChunkLinkType);
}

SemanticVerbTense SyntacticGraphToSemantic::xChunkToVerbTimeTenseForFrench(VerbGoalEnum& pVerbGoal,
                                                                           const Chunk& pVerbChunk,
                                                                           ChunkLinkType pChunkLinkType) const {
    SemanticVerbTense res = SemanticVerbTense::UNKNOWN;
    bool isRootAVerb =
        pChunkLinkType != ChunkLinkType::SUBJECT_OF;    // kind of a hack to extract this info // TODO: improve that
    const Chunk* auxChunk = getAuxiliaryChunk(pVerbChunk);
    const Chunk* verbChunkPtr = nullptr;

    if (auxChunk != nullptr && pVerbChunk.isPassive) {
        const auto& auxInflWord = auxChunk->head->inflWords.front();
        fConfiguration.getFlsChecker().verbTenseAndGoalFromInflections(
            res, pVerbGoal, auxInflWord.inflections(), isRootAVerb);
        verbChunkPtr = auxChunk;
        auxChunk = getAuxiliaryChunk(*verbChunkPtr);
    } else {
        const auto& inflWord = pVerbChunk.head->inflWords.front();
        fConfiguration.getFlsChecker().verbTenseAndGoalFromInflections(
            res, pVerbGoal, inflWord.inflections(), isRootAVerb);
        verbChunkPtr = &pVerbChunk;
    }

    if (auxChunk != nullptr && (res == SemanticVerbTense::PAST || res == SemanticVerbTense::PRESENT)) {
        const auto& auxWord = auxChunk->head->inflWords.front().word;
        if (auxWord == fLingDico.getHaveAux().word || auxWord == fLingDico.getBeAux().word)
            return SemanticVerbTense::PUNCTUALPAST;
    }

    if (res == SemanticVerbTense::PRESENT) {
        const auto& inflWord = verbChunkPtr->head->inflWords.front();
        if (ConceptSet::haveAConceptThatBeginWith(inflWord.infos.concepts, "verb_action")) {
            if (pChunkLinkType == ChunkLinkType::SUBJECT_OF && inflWord.word.lemma == "dire")
                return SemanticVerbTense::PUNCTUALPAST;
            return SemanticVerbTense::PUNCTUALPRESENT;
        }
    }
    return res;
}

SemanticVerbTense SyntacticGraphToSemantic::xChunkToVerbTimeTenseForEnglish(VerbGoalEnum& pVerbGoal,
                                                                            const Chunk& pVerbChunk,
                                                                            ChunkLinkType pChunkLinkType) const {
    if (xVerbAuxiliaryIndicateFutureForEnglish(pVerbChunk))
        return SemanticVerbTense::FUTURE;

    const InflectedWord& vbIGram = pVerbChunk.head->inflWords.front();
    SemanticVerbTense res = SemanticVerbTense::UNKNOWN;
    bool isRootAVerb =
        pChunkLinkType != ChunkLinkType::SUBJECT_OF;    // kind of a hack to extract this info // TODO: improve that
    fConfiguration.getFlsChecker().verbTenseAndGoalFromInflections(res, pVerbGoal, vbIGram.inflections(), isRootAVerb);

    const Chunk* auxChunk = getAuxiliaryChunk(pVerbChunk);
    if (auxChunk != nullptr) {
        const SemanticWord& auxWord = auxChunk->head->inflWords.front().word;
        bool isBeAux = auxWord == fLingDico.getBeAux().word;
        VerbGoalEnum tmp;
        SemanticVerbTense newRes = xChunkToVerbTimeTense(tmp, *auxChunk, pChunkLinkType);
        if (res != SemanticVerbTense::PAST && newRes == SemanticVerbTense::PUNCTUALPAST) {
            if ((auxWord.lemma == "can" || auxWord.lemma == "do") && auxWord.partOfSpeech == PartOfSpeech::AUX)
                return SemanticVerbTense::PUNCTUALPAST;
            if (isBeAux) {
                if (vbIGram.infos.hasContextualInfo(WordContextualInfos::BEISTHEAUXILIARY))
                    return SemanticVerbTense::PUNCTUALPAST;
                else
                    return SemanticVerbTense::PAST;
            }
        }
        if (isBeAux) {
            if (newRes == SemanticVerbTense::PRESENT && !isAPastTense(res))
                return SemanticVerbTense::PUNCTUALPRESENT;
            return newRes;
        }
    }
    if (res == SemanticVerbTense::PAST)
        return SemanticVerbTense::PUNCTUALPAST;
    else if (res == SemanticVerbTense::PRESENT && pVerbChunk.requests.has(SemanticRequestType::ACTION))
        return SemanticVerbTense::PUNCTUALPRESENT;
    return res;
}

bool SyntacticGraphToSemantic::xVerbAuxiliaryIndicateFutureForEnglish(const Chunk& pVerbChunk) {
    const Chunk* aux = getAuxiliaryChunk(pVerbChunk);
    if (aux != nullptr) {
        return aux->head->str == "will" || aux->head->str == "ll";
    }
    return false;
}

void _addARequest(SemanticRequests& requestTypes, SemanticRequestType pNewRequest, bool pIsPassive) {
    if (pIsPassive)
        SemExpModifier::modifyRequestIfAtPassiveForm(pNewRequest);
    requestTypes.add(pNewRequest);
}

std::unique_ptr<GroundedExpression> SyntacticGraphToSemantic::xInitNewSentence(ToGenRepGeneral& pGeneral,
                                                                               ToGenRepContext& pContext,
                                                                               Chunk& pVerbChunk) const {
    if (ConceptSet::haveAConcept(pVerbChunk.head->inflWords.front().infos.concepts, "verb_equal_be")) {
        pContext.holdingVerbIsBe = true;
        ChunkLink* subject = getSubjectChunkLink(pVerbChunk);
        if (subject != nullptr)
            subject->chunk->hasOnlyOneReference = false;
        ChunkLink* objectChkLk = getDOChunkLink(pVerbChunk);
        if (objectChkLk != nullptr)
            objectChkLk->chunk->hasOnlyOneReference = false;
    }

    std::unique_ptr<GroundedExpression> res;
    {
        auto rootGrounding = std::make_unique<SemanticStatementGrounding>();
        rootGrounding->verbTense = xChunkToVerbTimeTense(rootGrounding->verbGoal, pVerbChunk, pContext.chLink.type);
        pContext.holdingSentenceVerbTense = rootGrounding->verbTense;
        pContext.holdingVerbChunkPtr = &pVerbChunk;
        xConvertVerbChunkToRootGrounding(*rootGrounding, pVerbChunk);
        // invert confidence signe => inverse the pointed meaning
        if (!pVerbChunk.positive && (pContext.holdingVerbIsBe || !_verbChunkHasAnObjectWithAContxtualInfoAny(pVerbChunk)))
            rootGrounding->polarity = !rootGrounding->polarity;
        if (pVerbChunk.isPassive)
            rootGrounding->isPassive.emplace(true);
        if (pContext.requestToSet != SemanticRequestType::NOTHING)
            _addARequest(rootGrounding->requests, pContext.requestToSet, pVerbChunk.isPassive);
        res = std::make_unique<GroundedExpression>(std::move(rootGrounding));
    }
    xFillVerbChunk(res, pGeneral, pContext, pVerbChunk);
    if (pGeneral.textProcContext.writeOriginalText)
        res->fromText = pVerbChunk.getTokRangeWrappingChildren().toStr();
    return res;
}

void _setRequestFromChildren(SemanticRequests& pRequests, const Chunk& pVerbChunk) {
    for (const auto& currChild : pVerbChunk.children) {
        if (currChild.type == ChunkLinkType::AUXILIARY) {
            _setRequestFromChildren(pRequests, *currChild.chunk);
        } else if (currChild.type != ChunkLinkType::SUBJECT && currChild.type != ChunkLinkType::DIRECTOBJECT
                   && currChild.type != ChunkLinkType::INDIRECTOBJECT && checkOrder(*currChild.chunk, pVerbChunk)) {
            auto requestOpt = chunkTypeToRequestType(currChild.type);
            if (requestOpt)
                pRequests.set(*requestOpt);
        }
    }
}

void SyntacticGraphToSemantic::xFillVerbChunk(std::unique_ptr<GroundedExpression>& pGrdExp,
                                              ToGenRepGeneral& pGeneral,
                                              ToGenRepContext& pContext,
                                              Chunk& pVerbChunk) const {
    xAddModifiers(pGrdExp, pContext, pVerbChunk, true);
    for (auto& currSubVerbChunkLink : pVerbChunk.children) {
        if (currSubVerbChunkLink.type == ChunkLinkType::AUXILIARY)
            xAddModifiers(pGrdExp, pContext, *currSubVerbChunkLink.chunk, true);
    }

    // if it has a child with type "simple"
    auto itChildSimple = getChunkLink(pVerbChunk, ChunkLinkType::SIMPLE);
    if (itChildSimple != pVerbChunk.children.end()) {
        const Chunk& subSimpleChunk = *itChildSimple->chunk;
        const TokIt& subSimpleChunkItEnd = subSimpleChunk.tokRange.getItEnd();
        for (TokIt it = getTheNextestToken(
                 subSimpleChunk.tokRange.getItBegin(), subSimpleChunk.tokRange.getItEnd(), SkipPartOfWord::YES);
             it != subSimpleChunkItEnd;
             it = getNextToken(it, subSimpleChunk.tokRange.getItEnd(), SkipPartOfWord::YES))
            xAddModifiersOfATokenAfterVerb(*pGrdExp, pContext, it, subSimpleChunkItEnd);
    }

    if (!pVerbChunk.requests.empty()) {
        auto requests = pVerbChunk.requests;
        if (pVerbChunk.requests.has(SemanticRequestType::OBJECT))
            _setRequestFromChildren(requests, pVerbChunk);
        if (pVerbChunk.isPassive)
            for (auto& currRequest : requests.types)
                SemExpModifier::modifyRequestIfAtPassiveForm(currRequest);
        SemExpModifier::setRequests(*pGrdExp, requests);
        if (pVerbChunk.requests.has(SemanticRequestType::ACTION)) {
            const InflectedWord& vbChkHeadIGram = pVerbChunk.head->inflWords.front();
            auto impSubj = xTranslateRelativePersToPerson(
                fConfiguration.getFlsChecker().imperativeVerbToRelativePerson(vbChkHeadIGram),
                vbChkHeadIGram,
                pContext,
                pGeneral);
            if (impSubj) {
                SemExpModifier::addChild(*pGrdExp, GrammaticalType::SUBJECT, std::move(impSubj));
            }
        }
        if (pContext.holdingSentenceRequests.empty())
            pContext.holdingSentenceRequests = std::move(requests);
    }
}

SemanticRequestType SyntacticGraphToSemantic::xSubBordinateChkLkToRequest(const ChunkLink& pSubBordinateChkLk) const {
    if (!pSubBordinateChkLk.tokRange.isEmpty()) {
        const auto& inflWordSub = pSubBordinateChkLk.tokRange.getItBegin()->inflWords.front();
        return fLingDico.statDb.wordToSubordinateRequest(inflWordSub.word);
    }
    return SemanticRequestType::NOTHING;
}

const TextProcessingContext& SyntacticGraphToSemantic::xGetTextProcContext(const ToGenRepContext& pContext,
                                                                           const ToGenRepGeneral& pGeneral) const {
    if (pContext.localTextProcContextPtr)
        return *pContext.localTextProcContextPtr;
    return pGeneral.textProcContext;
}

std::unique_ptr<SemanticExpression> SyntacticGraphToSemantic::xTranslateRelativePersToPerson(
    RelativePerson pRelPers,
    const InflectedWord& pIGram,
    const ToGenRepContext& pContext,
    const ToGenRepGeneral& pGeneral) const {
    std::string userId = SemanticAgentGrounding::getAnyUser();
    SemanticEntityType agentType = SemanticEntityType::THING;
    SemanticReferenceType referenceType = SemanticReferenceType::INDEFINITE;
    SemanticQuantity quantity;
    bool hasToBeCompletedFromContext = false;
    switch (pRelPers) {
        case RelativePerson::FIRST_SING: {
            return std::make_unique<GroundedExpression>(
                std::make_unique<SemanticAgentGrounding>(xGetTextProcContext(pContext, pGeneral).author));
        }
        case RelativePerson::SECOND_SING:
        case RelativePerson::SECOND_PLUR: {
            referenceType = SemanticReferenceType::DEFINITE;
            agentType = SemanticEntityType::HUMAN;
            userId = xGetTextProcContext(pContext, pGeneral).receiver.userId;
            break;
        }
        case RelativePerson::FIRST_PLUR: {
            return pGeneral.textProcContext.usSemExp->clone();
        }
        case RelativePerson::THIRD_SING:
        case RelativePerson::THIRD_PLUR: {
            if (ConceptSet::haveAConcept(pIGram.infos.concepts, "reflexive")) {
                referenceType = SemanticReferenceType::DEFINITE;
                agentType = SemanticEntityType::HUMAN;
                userId = xGetTextProcContext(pContext, pGeneral).receiver.userId;
                break;
            }

            if (pRelPers == RelativePerson::THIRD_SING)
                quantity.setNumber(1);
            bool cannotBeCompletedFromContext =
                (pContext.holdingSentenceRequests.has(SemanticRequestType::SUBJECT)
                 && pContext.grammTypeFromParent == GrammaticalType::SUBJECT)
                || pIGram.infos.hasContextualInfo(WordContextualInfos::CANNOTBEACOREFRENCE);
            bool quantityFilled =
                SemExpModifier::fillQuantityAndReferenceFromConcepts(quantity, referenceType, pIGram.infos.concepts);
            if (!cannotBeCompletedFromContext && !quantityFilled
                && ConceptSet::haveAConcept(pIGram.infos.concepts, "fromRecentContext"))
                hasToBeCompletedFromContext = true;
            if (ConceptSet::haveAConcept(pIGram.infos.concepts, "reference_indefinite")) {
                referenceType = SemanticReferenceType::INDEFINITE;
                if (!quantityFilled)
                    quantity.clear();
            } else if (!quantityFilled) {
                referenceType = SemanticReferenceType::DEFINITE;
                if (!cannotBeCompletedFromContext)
                    hasToBeCompletedFromContext = true;
            }

            if (ConceptSet::haveAConceptOrAHyponym(pIGram.infos.concepts, "agent")) {
                agentType = SemanticEntityType::HUMAN;
            } else {
                if (pIGram.infos.hasContextualInfo(WordContextualInfos::REFTOAPERSON))
                    agentType = SemanticEntityType::HUMAN;
                else
                    agentType = SemanticEntityType::THING;
            }
            break;
        }
        default: return std::unique_ptr<SemanticExpression>();
    }

    if (referenceType == SemanticReferenceType::DEFINITE && userId != SemanticAgentGrounding::getAnyUser()) {
        return std::make_unique<GroundedExpression>(std::make_unique<SemanticAgentGrounding>(userId));
    }

    auto pronGenGrd = std::make_unique<SemanticGenericGrounding>(referenceType, agentType);
    SemExpModifier::fillConceptsForPronouns(pronGenGrd->concepts, pIGram.infos.concepts);
    if (agentType == SemanticEntityType::HUMAN && quantity.type == SemanticQuantityType::EVERYTHING)
        pronGenGrd->concepts.emplace("agent_*", 4);
    if (hasToBeCompletedFromContext) {
        if (pIGram.infos.hasContextualInfo(WordContextualInfos::REFTOASENTENCE))
            pronGenGrd->entityType = SemanticEntityType::SENTENCE;
        pronGenGrd->coreference.emplace();
    }
    fConfiguration.getFlsChecker().initGenderSetFromIGram(pronGenGrd->possibleGenders, pIGram);
    if (quantity.type == SemanticQuantityType::UNKNOWN
        && SemExpGetter::getNumberFromInflections(pIGram) == SemanticNumberType::PLURAL)
        pronGenGrd->quantity.setPlural();
    else
        pronGenGrd->quantity = quantity;
    return std::make_unique<GroundedExpression>(std::move(pronGenGrd));
}

UniqueSemanticExpression SyntacticGraphToSemantic::xConvertInterjectionChunk(ToGenRepGeneral& pGeneral,
                                                                             ToGenRepContext& pContext) const {
    std::list<std::unique_ptr<GroundedExpression>> interjectionGrdExps;

    auto chunkItEnd = pContext.chunk.tokRange.getItEnd();
    for (TokIt currIt = pContext.chunk.tokRange.getItBegin(); currIt != chunkItEnd;
         currIt = getNextToken(currIt, pContext.chunk.tokRange.getItEnd(), SkipPartOfWord::YES)) {
        auto genGroundings = std::make_unique<SemanticGenericGrounding>();
        genGroundings->concepts = currIt->inflWords.front().infos.concepts;
        xInitGenGroundingsFromToken(*genGroundings, currIt, chunkItEnd);
        interjectionGrdExps.emplace_back(std::make_unique<GroundedExpression>(std::move(genGroundings)));
    }

    if (!interjectionGrdExps.empty()) {
        std::unique_ptr<GroundedExpression> root = std::move(interjectionGrdExps.front());
        interjectionGrdExps.pop_front();
        while (!interjectionGrdExps.empty()) {
            auto newRoot = std::move(interjectionGrdExps.front());
            interjectionGrdExps.pop_front();
            newRoot->children.emplace(GrammaticalType::SPECIFIER, std::move(root));
            root = std::move(newRoot);
        }
        const InflectedWord& contextHeadIGram = pContext.chunk.head->inflWords.front();
        UniqueSemanticExpression interjectionGrdExp = std::move(root);
        xIterateOnChildrenOfNominalChunk(interjectionGrdExp, pGeneral, pContext, contextHeadIGram);
        return interjectionGrdExp;
    }
    return UniqueSemanticExpression();
}

UniqueSemanticExpression SyntacticGraphToSemantic::xConvertNominalChunk(ToGenRepGeneral& pGeneral,
                                                                        ToGenRepContext& pContext) const {
    {
        auto optRes = xFillTimeStruct(pContext);
        if (optRes)
            return optRes;
        auto optResGrd = xFillHourTimeStruct(pContext.chunk);
        if (optResGrd)
            return std::make_unique<GroundedExpression>(std::move(optResGrd));
    }

    {
        auto optRes = xFillLengthStruct(pContext);
        if (!optRes)
            optRes = xFillLocationStruct(pContext);
        if (!optRes)
            optRes = xFillPercentageStruct(pContext);
        if (!optRes)
            optRes = xFillDurationStruct(pContext);
        if (optRes) {
            xAddModifiers(optRes, pContext, pContext.chunk, true);
            return optRes;
        }
    }

    auto semExp = xConvertNominalChunkToSemExp(pContext, pGeneral, pContext.chLink, pContext.chunk);
    const InflectedWord& contextHeadIGram = pContext.chunk.head->inflWords.front();
    xIterateOnChildrenOfNominalChunk(semExp, pGeneral, pContext, contextHeadIGram);

    /**
     * It's because we can have:
     *        |
     *       de
     *        |
     *       AND
     *      /   \
     *    elt1  elt2
     */
    GroundedExpression* grdExpPtr = semExp->getGrdExpPtr_SkipWrapperPtrs();
    if (grdExpPtr != nullptr && grdExpPtr->children.size() == 1
        && (contextHeadIGram.word.partOfSpeech == PartOfSpeech::PARTITIVE
            || contextHeadIGram.word.partOfSpeech == PartOfSpeech::DETERMINER)) {
        auto res = std::move(grdExpPtr->children.begin()->second);
        grdExpPtr->children.clear();
        const auto& detGrd = grdExpPtr->grounding();
        if (SemExpGetter::isAHumanFromGrd(detGrd)) {
            SemExpModifier::addChildFromSemExp(*res, GrammaticalType::OWNER, std::move(semExp));
        } else {
            SemanticReferenceType detReferenceType = SemExpGetter::getReferenceTypeFromGrd(detGrd);
            if (detReferenceType != SemanticReferenceType::UNDEFINED)
                SemExpModifier::setReferenceTypeOfSemExp(*res, detReferenceType);
        }
        return res;
    }

    xTryToFillCondition(semExp, pGeneral, pContext);
    return semExp;
}

void SyntacticGraphToSemantic::xAddSubjectOf(GroundedExpression& pNewGrdExp,
                                             ToGenRepGeneral& pGeneral,
                                             ToGenRepContext& pContext,
                                             ListExpressionType pListExpType) const {
    if (chunkTypeIsAList(pContext.chunk.type)) {
        pListExpType = _chunkTypeToListExpType(pContext.chunk.type);
        for (auto& currChild : pContext.chunk.children) {
            ToGenRepContext subContext(pContext, currChild, *currChild.chunk);
            xAddSubjectOf(pNewGrdExp, pGeneral, subContext, pListExpType);
        }
        return;
    }

    std::map<GrammaticalType, UniqueSemanticExpression> additionalChildren;
    GrammaticalType gramTypeOfSubject = GrammaticalType::SUBJECT;
    if (pContext.chunk.isPassive && !haveADO(pContext.chunk) && !haveASubordonateClause(pContext.chunk))
        gramTypeOfSubject = GrammaticalType::OBJECT;
    additionalChildren.emplace(gramTypeOfSubject,
                               SemExpGenerator::makeCoreferenceExpression(CoreferenceDirectionEnum::PARENT));
    pContext.grammTypeFromParent = GrammaticalType::SPECIFIER;
    xAddNewGrammInfo(pNewGrdExp, pGeneral, pContext, SemanticRequestType::NOTHING, pListExpType, &additionalChildren);
}

void SyntacticGraphToSemantic::xIterateOnChildrenOfNominalChunk(UniqueSemanticExpression& pNewSemExp,
                                                                ToGenRepGeneral& pGeneral,
                                                                const ToGenRepContext& pContext,
                                                                const InflectedWord& pContextHeadIGram) const {
    auto newGrdExpPtr = pNewSemExp->getGrdExpPtr_SkipWrapperPtrs();
    if (newGrdExpPtr == nullptr) {
        auto* listExpPtr = pNewSemExp->getListExpPtr();
        if (listExpPtr != nullptr)
            for (auto& currElt : listExpPtr->elts)
                xIterateOnChildrenOfNominalChunk(currElt, pGeneral, pContext, pContextHeadIGram);
        return;
    }
    GroundedExpression& newGrdExp = *newGrdExpPtr;
    for (const auto& currChild : pContext.chunk.children) {
        ToGenRepContext subContext(pContext, currChild, *currChild.chunk);
        subContext.posFromParent = pContextHeadIGram.word.partOfSpeech;
        if (chunkTypeIsAList(currChild.chunk->type) && subContext.posFromParent == PartOfSpeech::DETERMINER) {
            subContext.grammTypeFromParent = GrammaticalType::SPECIFIER;
            xAddNewGrammInfo(newGrdExp, pGeneral, subContext);
        } else if (currChild.type == ChunkLinkType::COMPLEMENT
                   || (currChild.type == ChunkLinkType::SIMPLE
                       && subContext.posFromParent != PartOfSpeech::DETERMINER)) {
            const Token& headToken = *currChild.chunk->head;
            PartOfSpeech childHeadPartOfSpeech = headToken.inflWords.front().word.partOfSpeech;
            if (childHeadPartOfSpeech == PartOfSpeech::DETERMINER || childHeadPartOfSpeech == PartOfSpeech::PARTITIVE) {
                bool knowReferenceForSure = false;
                auto& newGrd = newGrdExp.grounding();
                xAddDeterminerToAGrounding(newGrdExp,
                                           newGrd,
                                           knowReferenceForSure,
                                           currChild.chunk->head,
                                           currChild.chunk->tokRange.getItEnd(),
                                           pContext,
                                           pGeneral);
            } else {
                auto childExp = xFillSemExp(pGeneral, subContext);
                if (childExp) {
                    if (SemExpGetter::isDefiniteModifier(**childExp)
                        && SemExpGetter::getReference(newGrdExp) == SemanticReferenceType::UNDEFINED)
                        SemExpModifier::setReference(newGrdExp, SemanticReferenceType::DEFINITE);
                    SemExpModifier::addChild(newGrdExp,
                                             GrammaticalType::SPECIFIER,
                                             std::move(*childExp),
                                             ListExpressionType::UNRELATED,
                                             true);
                }
            }
        } else if (currChild.type == ChunkLinkType::SPECIFICATION) {
            // if the specification is hold in the root meaning, we skip the specification
            if (!currChild.chunk->head->linkedTokens.empty()
                && currChild.chunk->head->linkedTokens.front() == pContext.chunk.head) {
                xIterateOnChildrenOfNominalChunk(
                    pNewSemExp, pGeneral, subContext, subContext.chunk.head->inflWords.front());
            } else {
                subContext.grammTypeFromParent = GrammaticalType::SPECIFIER;
                xAddNewGrammInfo(newGrdExp, pGeneral, subContext);
            }
        } else if (currChild.type == ChunkLinkType::OBJECT_OF) {
            subContext.grammTypeFromParent = GrammaticalType::SPECIFIER;
            xAddNewGrammInfo(newGrdExp, pGeneral, subContext);
        } else if (currChild.type == ChunkLinkType::SUBJECT_OF) {
            xAddSubjectOf(newGrdExp, pGeneral, subContext, ListExpressionType::UNRELATED);
        } else if (currChild.type == ChunkLinkType::SUBORDINATE_CLAUSE) {
            SemanticRequestType subordinateRequest = [this, &currChild, &subContext] {
                SemanticRequestType res = xSubBordinateChkLkToRequest(currChild);
                if (res == SemanticRequestType::NOTHING) {
                    if (!haveASubject(subContext.chunk))
                        return SemanticRequestType::SUBJECT;
                    return SemanticRequestType::OBJECT;
                }
                return res;
            }();
            subContext.grammTypeFromParent = GrammaticalType::OBJECT;
            xAddNewGrammInfo(newGrdExp, pGeneral, subContext, subordinateRequest);
        } else {
            auto optGrammType = chunkTypeToGrammaticalType(currChild.type);
            if (optGrammType) {
                subContext.grammTypeFromParent = *optGrammType;
                xAddNewGrammInfo(newGrdExp, pGeneral, subContext);
            }
        }
    }

    for (const auto& currChild : pContext.chunk.children) {
        if (currChild.type == ChunkLinkType::INTERJECTION) {
            ToGenRepContext subContext(pContext, currChild, *currChild.chunk);
            auto childExp = xFillSemExp(pGeneral, subContext);
            if (childExp)
                pNewSemExp = std::make_unique<FeedbackExpression>(std::move(*childExp), std::move(pNewSemExp));
        } else if (currChild.type == ChunkLinkType::SPECIFICATION) {
            SemanticLanguageEnum language = fConfiguration.getLanguageType();
            if (language == SemanticLanguageEnum::FRENCH) {
                auto subNewGrdExpPtr = pNewSemExp->getGrdExpPtr_SkipWrapperPtrs();
                if (subNewGrdExpPtr != nullptr && subNewGrdExpPtr->children.size() == 1
                    && SemExpGetter::isNothing(*subNewGrdExpPtr)) {
                    auto itSpecifier = subNewGrdExpPtr->children.find(GrammaticalType::SPECIFIER);
                    if (itSpecifier != subNewGrdExpPtr->children.end()) {
                        pNewSemExp = std::move(itSpecifier->second);
                        SemExpModifier::setNumberFromSemExp(pNewSemExp, 0);
                    }
                }
            }
        }
    }
}

void SyntacticGraphToSemantic::xAfterCreationModificationsOnSentence(GroundedExpression& pGrdExpSentence,
                                                                     const ToGenRepContext& pContext,
                                                                     const ToGenRepGeneral& pGeneral) const {
    SemanticLanguageEnum language = fConfiguration.getLanguageType();

    SemanticStatementGrounding* statementGrdPtr = pGrdExpSentence->getStatementGroundingPtr();
    if (statementGrdPtr == nullptr)
        return;
    auto& statementGrd = *statementGrdPtr;

    // the case "il y a <blabla>"
    if (language == SemanticLanguageEnum::FRENCH) {
        if (haveChild(pContext.chunk, ChunkLinkType::REFLEXIVE)) {
            Chunk* subjectChunkPtr = getSubjectChunk(pContext.chunk);
            if (subjectChunkPtr != nullptr && subjectChunkPtr->head->inflWords.front().word.lemma == "il") {
                if (statementGrd.concepts.count("verb_have") > 0)
                    _convertToGeneralitySentence(pGrdExpSentence, statementGrd);
                else if (statementGrd.word.lemma == "passer~se")
                    pGrdExpSentence.children.erase(GrammaticalType::SUBJECT);
                else if (statementGrd.word.lemma == "falloir")
                    pGrdExpSentence.children[GrammaticalType::SUBJECT] = pGeneral.textProcContext.usSemExp->clone();
            }
        }

        if (statementGrd.requests.has(SemanticRequestType::SUBJECT)) {
            auto itSubject = pGrdExpSentence.children.find(GrammaticalType::SUBJECT);
            if (itSubject != pGrdExpSentence.children.end()
                && SemExpGetter::semExpIsAnEmptyStatementGrd(*itSubject->second)) {
                auto itObject = pGrdExpSentence.children.find(GrammaticalType::OBJECT);
                if (itObject != pGrdExpSentence.children.end()) {
                    itSubject->second.swap(itObject->second);
                    statementGrd.requests.erase(SemanticRequestType::SUBJECT);
                    statementGrd.requests.add(SemanticRequestType::OBJECT);
                }
            }
        }
    }

    // the case of the be verb
    if (pContext.holdingVerbIsBe) {
        if (language == SemanticLanguageEnum::ENGLISH) {
            Chunk* subjectChunkPtr = getSubjectChunk(pContext.chunk);
            if (subjectChunkPtr != nullptr && subjectChunkPtr->head->inflWords.front().word.lemma == "there")
                _convertToGeneralitySentence(pGrdExpSentence, statementGrd);
        }

        auto itSubject = pGrdExpSentence.children.find(GrammaticalType::SUBJECT);
        if (itSubject != pGrdExpSentence.children.end()) {
            auto* subjectGrdPtr = itSubject->second->getGrdExpPtr_SkipWrapperPtrs();
            if (subjectGrdPtr != nullptr) {
                auto* subjectGenGrdPtr = subjectGrdPtr->grounding().getGenericGroundingPtr();
                if (subjectGenGrdPtr != nullptr && subjectGenGrdPtr->coreference
                    && subjectGenGrdPtr->entityType == SemanticEntityType::SENTENCE) {
                    auto itObject = pGrdExpSentence.children.find(GrammaticalType::OBJECT);
                    if (itObject != pGrdExpSentence.children.end()) {
                        SemanticEntityType objEntity = SemExpGetter::getEntity(*itObject->second);
                        if (objEntity != SemanticEntityType::UNKNOWN && objEntity != SemanticEntityType::AGENTORTHING)
                            subjectGenGrdPtr->entityType = objEntity;
                    }
                }
            }
        }
    }
}

void SyntacticGraphToSemantic::xFillNewSentence(
    GroundedExpression& pGrdExpSentence,
    std::map<GrammaticalType, std::unique_ptr<ConditionExpression>>& pChildTypeToCondition,
    ToGenRepGeneral& pGeneral,
    const ToGenRepContext& pContext) const {
    const Token& headToken = *pContext.chunk.head;
    const auto& headConcepts = headToken.inflWords.front().infos.concepts;
    bool askRequest = ConceptSet::haveAConcept(headConcepts, "verb_action_say_ask");
    bool sayRequest = ConceptSet::haveAConcept(headConcepts, "verb_action_say");
    SemanticLanguageEnum language = fConfiguration.getLanguageType();

    bool isPassive = false;
    if (pContext.grammTypeFromParent != GrammaticalType::SPECIFIER) {
        if (pContext.chunk.type == ChunkType::AUX_CHUNK)
            isPassive = pContext.holdingVerbChunkPtr != nullptr && pContext.holdingVerbChunkPtr->isPassive;
        else
            isPassive = pContext.chunk.isPassive;
    }

    for (const ChunkLink& currChkLk : pContext.chunk.children) {
        ToGenRepContext subContext(pContext, currChkLk, *currChkLk.chunk);
        subContext.holdingGrdExpPtr = &pGrdExpSentence;

        switch (currChkLk.type) {
            case ChunkLinkType::SUBJECT: {
                if (language == SemanticLanguageEnum::FRENCH
                    && pContext.chunk.requests.has(SemanticRequestType::SUBJECT)
                    && (subContext.chunk.head->inflWords.front().infos.hasContextualInfo(
                            WordContextualInfos::REFTOASENTENCE)
                        || subContext.chunk.head->inflWords.front().infos.hasContextualInfo(
                            WordContextualInfos::REFTOAPERSON)
                        || pGrdExpSentence.children.count(GrammaticalType::SUBJECT) > 0))
                    break;

                if (isPassive)
                    subContext.grammTypeFromParent = GrammaticalType::OBJECT;
                else
                    subContext.grammTypeFromParent = GrammaticalType::SUBJECT;
                xAddNewGrammInfo(pGrdExpSentence, pGeneral, subContext);
                break;
            }
            case ChunkLinkType::DIRECTOBJECT: {
                GrammaticalType childGramType = GrammaticalType::OBJECT;
                if (isPassive)
                    childGramType = GrammaticalType::SUBJECT;
                if (currChkLk.chunk->type == ChunkType::INFINITVE_VERB_CHUNK) {
                    auto sentence = xInitNewSentence(pGeneral, subContext, *currChkLk.chunk);
                    std::map<GrammaticalType, std::unique_ptr<ConditionExpression>> childTypeToCondition;
                    xFillNewSentence(*sentence, childTypeToCondition, pGeneral, subContext);
                    UniqueSemanticExpression semExp(std::move(sentence));
                    _fillConditions(semExp, childTypeToCondition, *currChkLk.chunk, pGeneral.textProcContext.writeOriginalText);
                    xFillSentenceSubordonates(semExp, pChildTypeToCondition, childGramType, pGeneral, subContext);

                    // add the request of the subordinate
                    SemanticRequestType reqOfSubordinate = xSubBordinateChkLkToRequest(currChkLk);
                    if (reqOfSubordinate != SemanticRequestType::NOTHING
                        && reqOfSubordinate != SemanticRequestType::SUBJECT)
                        SemExpModifier::addRequest(*semExp, reqOfSubordinate);

                    SemExpModifier::addChild(pGrdExpSentence, childGramType, std::move(semExp));
                } else {
                    subContext.grammTypeFromParent = childGramType;
                    xAddNewGrammInfo(pGrdExpSentence, pGeneral, subContext);
                }
                break;
            }
            case ChunkLinkType::SUBORDINATE: {
                subContext.grammTypeFromParent = GrammaticalType::SUBORDINATE;
                xAddNewGrammInfo(pGrdExpSentence, pGeneral, subContext);
                break;
            }
            case ChunkLinkType::REASONOF: {
                if ((askRequest || sayRequest)
                    && pGrdExpSentence.children.find(GrammaticalType::OBJECT) == pGrdExpSentence.children.end()) {
                    subContext.grammTypeFromParent = GrammaticalType::OBJECT;
                    xAddNewGrammInfo(pGrdExpSentence, pGeneral, subContext, SemanticRequestType::CAUSE);
                } else {
                    subContext.grammTypeFromParent = GrammaticalType::REASONOF;
                    xAddNewGrammInfo(pGrdExpSentence, pGeneral, subContext);
                }
                break;
            }
            case ChunkLinkType::INDIRECTOBJECT: {
                if (pGrdExpSentence.children.find(GrammaticalType::RECEIVER) == pGrdExpSentence.children.end()) {
                    subContext.grammTypeFromParent = GrammaticalType::RECEIVER;
                    xAddNewGrammInfo(pGrdExpSentence, pGeneral, subContext);
                }
                break;
            }
            case ChunkLinkType::ACCORDINGTO:
            case ChunkLinkType::AGAINST:
            case ChunkLinkType::BETWEEN:
            case ChunkLinkType::CAUSE:
            case ChunkLinkType::DESPITE_CONTRAINT:
            case ChunkLinkType::LENGTH:
            case ChunkLinkType::DURATION:
            case ChunkLinkType::STARTING_POINT:
            case ChunkLinkType::IN_BACKGROUND:
            case ChunkLinkType::IN_CASE_OF:
            case ChunkLinkType::LANGUAGE:
            case ChunkLinkType::LOCATION:
            case ChunkLinkType::MANNER:
            case ChunkLinkType::MITIGATION:
            case ChunkLinkType::OCCURRENCE_RANK:
            case ChunkLinkType::PURPOSE:
            case ChunkLinkType::RECEIVER:
            case ChunkLinkType::REPETITION:
            case ChunkLinkType::SIMILARITY:
            case ChunkLinkType::SPECIFICATION:
            case ChunkLinkType::THANKS_TO:
            case ChunkLinkType::TIME:
            case ChunkLinkType::TOPIC:
            case ChunkLinkType::UNITY:
            case ChunkLinkType::WAY:
            case ChunkLinkType::WITH:
            case ChunkLinkType::WITHOUT: {
                mystd::optional<GrammaticalType> grammaticalTypeOpt = chunkTypeToGrammaticalType(currChkLk.type);
                if (grammaticalTypeOpt) {
                    subContext.grammTypeFromParent = *grammaticalTypeOpt;
                    if (subContext.grammTypeFromParent == GrammaticalType::DURATION
                        && pContext.chunk.requests.has(SemanticRequestType::DURATION)
                        && subContext.chunk.children.empty()
                        && ConceptSet::haveAConcept(subContext.chunk.head->inflWords.front().infos.concepts, "time"))
                        break;
                    xAddNewGrammInfo(pGrdExpSentence, pGeneral, subContext);
                }
                break;
            }
            case ChunkLinkType::INTERVAL: {
                auto optRes = xFillInterval(subContext);
                if (optRes)
                    pGrdExpSentence.children.emplace(GrammaticalType::INTERVAL, std::move(optRes));
                break;
            }
            case ChunkLinkType::AUXILIARY:
            case ChunkLinkType::SIMPLE: {
                xFillNewSentence(pGrdExpSentence, pChildTypeToCondition, pGeneral, subContext);
                break;
            }
            case ChunkLinkType::QUESTIONWORD: {
                bool refToAPerson = false;
                if ((refToAPerson = subContext.chunk.head->inflWords.front().infos.hasContextualInfo(
                         WordContextualInfos::REFTOAPERSON))
                    || subContext.chunk.head->inflWords.front().infos.hasContextualInfo(
                        WordContextualInfos::REFTOASENTENCE)) {
                    auto* statGrdPtr = pGrdExpSentence->getStatementGroundingPtr();
                    if (statGrdPtr != nullptr && !statGrdPtr->requests.empty()) {
                        auto request = statGrdPtr->requests.firstOrNothing();
                        if (request == SemanticRequestType::SUBJECT || request == SemanticRequestType::OBJECT) {
                            auto whoGrammType = semanticRequestType_toSemGram(request);
                            SemExpModifier::addChild(
                                pGrdExpSentence,
                                whoGrammType,
                                refToAPerson ? _whoSemExp() : SemExpGenerator::emptyStatementSemExp());
                        }
                    }
                }
                break;
            }
            case ChunkLinkType::COMPARATOR_DIFFERENT:
            case ChunkLinkType::COMPARATOR_EQUAL:
            case ChunkLinkType::COMPARATOR_MORE:
            case ChunkLinkType::COMPARATOR_LESS:
            case ChunkLinkType::RIGHTOPCOMPARISON:
            case ChunkLinkType::SUBJECT_OF:
            case ChunkLinkType::OBJECT_OF:
            case ChunkLinkType::PURPOSE_OF:
            case ChunkLinkType::REFLEXIVE:
            case ChunkLinkType::SUBORDINATE_CLAUSE:
            case ChunkLinkType::INTERJECTION:
            case ChunkLinkType::OWNER:
            case ChunkLinkType::IF:
            case ChunkLinkType::ELSE:
            case ChunkLinkType::COMPLEMENT:
            case ChunkLinkType::NOTUNDERSTOOD:
            case ChunkLinkType::TODO:
            case ChunkLinkType::IGNORE: break;
        }
    }
}

void SyntacticGraphToSemantic::xFillSentenceSubordonates(
    UniqueSemanticExpression& pSemExpSentence,
    std::map<GrammaticalType, std::unique_ptr<ConditionExpression>>& pChildTypeToCondition,
    GrammaticalType pCurrGramChild,
    ToGenRepGeneral& pGeneral,
    const ToGenRepContext& pContext) const {
    // generate comparisons
    for (const auto& currChild : pContext.chunk.children) {
        if (chunkLinkType_isAComparator(currChild.type)) {
            ToGenRepContext compSubContext(pContext, currChild, *currChild.chunk);
            auto whatIsComparedExp = xFillSemExp(pGeneral, compSubContext);
            if (whatIsComparedExp) {
                if (pSemExpSentence->type == SemanticExpressionType::GROUNDED) {
                    GroundedExpression& grdExp = pSemExpSentence->getGrdExp();
                    auto itSubject = grdExp.children.find(GrammaticalType::SUBJECT);
                    if (itSubject != grdExp.children.end()) {
                        auto compExp = std::make_unique<ComparisonExpression>(
                            chunkLinkType_toCompPolarity(currChild.type), std::move(itSubject->second));
                        grdExp.children.erase(itSubject);    // TODDO: also deal with other children of grdExp

                        {
                            VerbGoalEnum pVerbGoal = VerbGoalEnum::NOTIFICATION;
                            compExp->tense = xChunkToVerbTimeTense(pVerbGoal, pContext.chunk, pContext.chLink.type);
                            compExp->request = pContext.chunk.requests.firstOrNothing();
                        }
                        auto itRightOp = getChunkLink(pContext.chunk, ChunkLinkType::RIGHTOPCOMPARISON);
                        if (itRightOp != pContext.chunk.children.end()) {
                            ToGenRepContext rOpSubContext(pContext, currChild, *itRightOp->chunk);
                            compExp->rightOperandExp = xFillSemExp(pGeneral, rOpSubContext);
                        }
                        if (compExp->op != ComparisonOperator::EQUAL && compExp->op != ComparisonOperator::DIFFERENT)
                            compExp->whatIsComparedExp = std::move(whatIsComparedExp);
                        pSemExpSentence = std::move(compExp);
                    }
                }
            }
        }
    }

    ChunkLink const* conditionChunkLink = nullptr;
    for (const auto& currChild : pContext.chunk.children) {
        if (currChild.type == ChunkLinkType::SUBORDINATE_CLAUSE || currChild.type == ChunkLinkType::IF) {
            ToGenRepContext subContext(pContext, currChild, *currChild.chunk);
            xFillSubordonateClause(pSemExpSentence, conditionChunkLink, pGeneral, subContext, currChild, pContext);
        }
    }
    if (conditionChunkLink != nullptr)
        xFillCondition(pChildTypeToCondition, pCurrGramChild, pGeneral, *conditionChunkLink, pContext);

    for (const auto& currChild : pContext.chunk.children) {
        if (currChild.type == ChunkLinkType::INTERJECTION) {
            ToGenRepContext subContext(pContext, currChild, *currChild.chunk);
            auto intjExp = xFillSemExp(pGeneral, subContext);
            if (intjExp) {
                pSemExpSentence = std::make_unique<FeedbackExpression>(std::move(*intjExp), std::move(pSemExpSentence));
            }
        }
    }
}

void SyntacticGraphToSemantic::xTryToFillCondition(UniqueSemanticExpression& pSemExpSentence,
                                                   ToGenRepGeneral& pGeneral,
                                                   const ToGenRepContext& pContext) const {
    auto itIfChkLink = getChunkLink(pContext.chunk, ChunkLinkType::IF);
    if (itIfChkLink != pContext.chunk.children.end()) {
        std::map<GrammaticalType, std::unique_ptr<ConditionExpression>> childTypeToCondition;
        xFillCondition(childTypeToCondition, GrammaticalType::UNKNOWN, pGeneral, *itIfChkLink, pContext);
        _fillConditions(pSemExpSentence, childTypeToCondition, pContext.chunk, pGeneral.textProcContext.writeOriginalText);
    }
}

void SyntacticGraphToSemantic::xFillCondition(
    std::map<GrammaticalType, std::unique_ptr<ConditionExpression>>& pChildTypeToCondition,
    GrammaticalType pCurrGramChild,
    ToGenRepGeneral& pGeneral,
    const ChunkLink& pSubClauseChunkLink,
    const ToGenRepContext& pContext) const {
    bool isAlwaysActive = true;
    if (!pSubClauseChunkLink.tokRange.isEmpty()) {
        for (TokIt itTokRange = pSubClauseChunkLink.tokRange.getItBegin();
             itTokRange != pSubClauseChunkLink.tokRange.getItEnd();
             itTokRange = getNextToken(itTokRange, pSubClauseChunkLink.tokRange.getItEnd())) {
            const InflectedWord& itBegiIGram = itTokRange->inflWords.front();
            if (partOfSpeech_isAWord(itBegiIGram.word.partOfSpeech)
                && itBegiIGram.infos.hasContextualInfo(WordContextualInfos::CONDITION)
                && fLingDico.statDb.wordToSubordinateRequest(itBegiIGram.word) == SemanticRequestType::YESORNO) {
                isAlwaysActive = false;
                break;
            }
        }
    }

    ToGenRepContext subContext(pContext, pSubClauseChunkLink, *pSubClauseChunkLink.chunk);
    auto conLinkToExp = xFillSemExp(pGeneral, subContext);
    if (conLinkToExp) {
        auto condExp = std::make_unique<ConditionExpression>(
            isAlwaysActive, false, std::move(*conLinkToExp), UniqueSemanticExpression());
        auto itElse = getChunkLink(pContext.chunk, ChunkLinkType::ELSE);
        if (itElse != pContext.chunk.children.end()) {
            ToGenRepContext elseSubContext(pContext, *itElse, *itElse->chunk);
            condExp->elseExp = xFillSemExp(pGeneral, elseSubContext);
        }
        pChildTypeToCondition.emplace(pCurrGramChild, std::move(condExp));
    }
}

void SyntacticGraphToSemantic::xFillSubordonateClause(UniqueSemanticExpression& pSemExpSentence,
                                                      ChunkLink const*& pConditionChunkLink,
                                                      ToGenRepGeneral& pGeneral,
                                                      ToGenRepContext& pSubContext,
                                                      const ChunkLink& pSubClauseChunkLink,
                                                      const ToGenRepContext& pContext) const {
    const Token& headToken = *pContext.chunk.head;
    const auto& headConcepts = headToken.inflWords.front().infos.concepts;
    bool askRequest = ConceptSet::haveAConcept(headConcepts, "verb_action_say_ask");
    bool sayRequest = ConceptSet::haveAConcept(headConcepts, "verb_action_say");

    if (pSubClauseChunkLink.type == ChunkLinkType::IF
        && ((!askRequest && !sayRequest) || conditionIsTheFirstChildAndThereIsManyChildren(pContext.chunk)
            || haveADO(pContext.chunk) || haveASubordonateClause(pContext.chunk))) {
        pConditionChunkLink = &pSubClauseChunkLink;
        return;
    }

    GroundedExpression* grdExpPtr = pSemExpSentence->getGrdExpPtr_SkipWrapperPtrs();
    if (grdExpPtr == nullptr)
        return;
    GroundedExpression& grdExp = *grdExpPtr;

    SemanticRequestType requestOfQuestion = xSubBordinateChkLkToRequest(pSubClauseChunkLink);
    if (requestOfQuestion == SemanticRequestType::SUBJECT && haveASubject(*pSubClauseChunkLink.chunk))
        requestOfQuestion = SemanticRequestType::OBJECT;
    pSubContext.holdingSentenceRequests.set(requestOfQuestion);
    if (sayRequest && pSubClauseChunkLink.tokRange.isEmpty()
        && !pSubClauseChunkLink.chunk->requests.has(SemanticRequestType::ACTION)) {
        std::string subjectUserId = SemExpGetter::getUserIdOfSubject(grdExp);
        if (subjectUserId == SemanticAgentGrounding::getUserNotIdentified())
            subjectUserId = pGeneral.textProcContext.receiver.userId;
        const std::string& newReceiverUserId = subjectUserId == SemanticAgentGrounding::getMe()
                                                 ? SemanticAgentGrounding::getCurrentUser()
                                                 : SemanticAgentGrounding::getMe();
        pSubContext.localTextProcContextPtr =
            std::make_shared<TextProcessingContext>(TextProcessingContext(subjectUserId,
                                                                          newReceiverUserId,
                                                                          pGeneral.textProcContext.langType,
                                                                          pGeneral.textProcContext.usSemExp->clone()));
    }
    auto childExpObject = xFillSemExp(pGeneral, pSubContext);
    if (childExpObject) {
        if (!pSubClauseChunkLink.tokRange.isEmpty()
            && ((fConfiguration.getLanguageType() == SemanticLanguageEnum::FRENCH
                 && pSubClauseChunkLink.tokRange.getItBegin()->inflWords.front().word.lemma == "qui")
                || (fConfiguration.getLanguageType() == SemanticLanguageEnum::ENGLISH
                    && pSubClauseChunkLink.tokRange.getItBegin()->inflWords.front().word.lemma == "who"))) {
            GrammaticalType subGrammType = semanticRequestType_toSemGram(requestOfQuestion);
            SemExpModifier::addChildFromSemExp(**childExpObject, subGrammType, _whoSemExp());
        }
        std::list<GroundedExpression*> childGrdExpPtrs;
        (*childExpObject)->getGrdExpPtrs_SkipWrapperLists(childGrdExpPtrs);
        if (!childGrdExpPtrs.empty()) {
            GroundedExpression* firstChildGrdExpPtr = childGrdExpPtrs.front();
            if (firstChildGrdExpPtr != nullptr) {
                GroundedExpression& firstChildGrdExp = *firstChildGrdExpPtr;
                if (requestOfQuestion == SemanticRequestType::CAUSE
                    && ConceptSet::haveAConcept(headConcepts, "verb_equal_be"))
                    requestOfQuestion = SemanticRequestType::NOTHING;
                else if (requestOfQuestion != SemanticRequestType::NOTHING)
                    SemExpModifier::addRequest(firstChildGrdExp, requestOfQuestion);

                // if the sub sentence ask about an object &&
                // if the sub sentence has already an object that is link to a time concept
                //  => convert this child to a time child
                if (requestOfQuestion == SemanticRequestType::OBJECT) {
                    auto itObject = getChunkLink(pSubContext.chunk, ChunkLinkType::DIRECTOBJECT);
                    if (itObject != pSubContext.chunk.children.end() && !isAPrepositionnalChunkLink(*itObject)
                        && ConceptSet::haveAConceptOrAHyponym(itObject->chunk->head->inflWords.front().infos.concepts,
                                                              "time")) {
                        auto itObjOfGrdExp = firstChildGrdExp.children.find(GrammaticalType::OBJECT);
                        if (itObjOfGrdExp != firstChildGrdExp.children.end()) {
                            SemExpModifier::addChild(
                                firstChildGrdExp, GrammaticalType::TIME, std::move(itObjOfGrdExp->second));
                            firstChildGrdExp.children.erase(itObjOfGrdExp);
                        }
                    }
                }
            }
        }

        SemExpModifier::addChild(grdExp, GrammaticalType::OBJECT, std::move(*childExpObject));
    }
}

void SyntacticGraphToSemantic::xAddNewGrammInfo(
    GroundedExpression& pGrdExpSentence,
    ToGenRepGeneral& pGeneral,
    ToGenRepContext& pContext,
    SemanticRequestType pSubRequest,
    ListExpressionType pListExpType,
    std::map<GrammaticalType, UniqueSemanticExpression>* pAdditionalChildren) const {
    if (pSubRequest != SemanticRequestType::NOTHING)
        pContext.requestToSet = pSubRequest;
    pContext.holdingGrdExpPtr = &pGrdExpSentence;
    auto childExpObjectPtr = xFillSemExp(pGeneral, pContext);
    if (childExpObjectPtr) {
        UniqueSemanticExpression& childExpObject = *childExpObjectPtr;

        if (pSubRequest != SemanticRequestType::NOTHING) {
            SemExpModifier::applyVerbTenseModifOfSemExp(*childExpObject, [](SemanticVerbTense& pVerbTense) {
                if (pVerbTense == SemanticVerbTense::UNKNOWN)
                    pVerbTense = SemanticVerbTense::PRESENT;
            });
        }

        // skip the children of a relativeLocation node
        GroundedExpression* grdExpToLink = &pGrdExpSentence;
        if (pGrdExpSentence->type == SemanticGroundingType::RELATIVELOCATION
            && pContext.grammTypeFromParent != GrammaticalType::SPECIFIER) {
            auto itEquality = pGrdExpSentence.children.find(GrammaticalType::SPECIFIER);
            if (itEquality != pGrdExpSentence.children.end()
                && itEquality->second->type == SemanticExpressionType::GROUNDED) {
                grdExpToLink = &itEquality->second->getGrdExp();
            }
        }

        switch (pContext.grammTypeFromParent) {
            case GrammaticalType::OBJECT: {
                if (fConfiguration.getLanguageType() == SemanticLanguageEnum::ENGLISH)
                    _tryToConvertToInfinitiveSubordinate(childExpObject);
                _flattenSomeSpecifiers(*childExpObject, pGrdExpSentence);
                break;
            }
            case GrammaticalType::SUBORDINATE: _tryToAddTheIntroductingWord(*childExpObject, pContext.chLink); break;
            case GrammaticalType::LOCATION: _flattenSomeSpecifiers(*childExpObject, pGrdExpSentence); break;
            default: break;
        }

        if (pAdditionalChildren != nullptr)
            for (auto& currChild : *pAdditionalChildren)
                SemExpModifier::addChildFromSemExp(*childExpObject, currChild.first, std::move(currChild.second));

        SemExpModifier::addChild(
            *grdExpToLink, pContext.grammTypeFromParent, std::move(childExpObject), pListExpType, true);
    }
}

SyntacticGraphToSemantic::ToGenRepContext::ToGenRepContext(ToGenRepContext&& pOther)
    : chLink(pOther.chLink)
    , chunk(pOther.chunk)
    , holdingSentenceRequests(std::move(pOther.holdingSentenceRequests))
    , holdingSentenceVerbTense(std::move(pOther.holdingSentenceVerbTense))
    , grammTypeFromParent(std::move(pOther.grammTypeFromParent))
    , posFromParent(std::move(pOther.posFromParent))
    , localTextProcContextPtr(std::move(pOther.localTextProcContextPtr))
    , requestToSet(std::move(pOther.requestToSet)) {}

}    // End of namespace linguistics
}    // End of namespace onsem
