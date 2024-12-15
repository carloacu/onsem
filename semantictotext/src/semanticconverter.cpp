#include <onsem/semantictotext/semanticconverter.hpp>
#include <onsem/texttosemantic/linguisticanalyzer.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/treeconverter.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpressions.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticmetagrounding.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semantictextgrounding.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semantictimegrounding.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticstatementgrounding.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticgenericgrounding.hpp>
#include <onsem/texttosemantic/dbtype/sentiment/sentimentcontext.hpp>
#include <onsem/texttosemantic/dbtype/textprocessingcontext.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase.hpp>
#include <onsem/texttosemantic/tool/semexpgetter.hpp>
#include <onsem/texttosemantic/tool/semexpmodifier.hpp>
#include <onsem/texttosemantic/algorithmsetforalanguage.hpp>
#include <onsem/texttosemantic/languagedetector.hpp>
#include <onsem/semantictotext/semexpoperators.hpp>
#include <onsem/semantictotext/semanticmemory/links/expressionwithlinks.hpp>
#include <onsem/semantictotext/semanticmemory/semanticmemory.hpp>
#include <onsem/semantictotext/sentiment/sentimentdetector.hpp>
#include <onsem/semantictotext/type/naturallanguageexpression.hpp>
#include <onsem/semantictotext/tool/semexpcomparator.hpp>
#include "linguisticsynthesizer/linguisticsynthesizer.hpp"
#include "conversion/mandatoryformconverter.hpp"
#include "conversion/occurrencerankconverter.hpp"
#include "conversion/simplesentencesplitter.hpp"
#include "conversion/reasonofrefactor.hpp"
#include "interpretation/completewithcontext.hpp"
#include "linguisticsynthesizer/synthesizerresulttypes.hpp"
#include "utility/semexpcreator.hpp"

namespace onsem {
namespace converter {

namespace {
const SemanticMemoryBlock _emptyMemBlock(0);


const SemanticExpression* _skipCustomRoot(const SemanticExpression& pSemExp, const SemanticExpression& pCustomRoot,
                                         const linguistics::LinguisticDatabase& pLingDb) {
    const GroundedExpression* customRootGrdExpPtr = pCustomRoot.getGrdExpPtr_SkipWrapperPtrs();
    if (customRootGrdExpPtr != nullptr && customRootGrdExpPtr->children.size() < 2) {
        const GroundedExpression* grdExpPtr = pSemExp.getGrdExpPtr_SkipWrapperPtrs();
        if (grdExpPtr != nullptr && grdExpPtr->children.size() == 1) {
             const SemanticMemoryBlock memBlock;
             if (SemExpComparator::groundingsAreEqual(customRootGrdExpPtr->grounding(), grdExpPtr->grounding(), memBlock, pLingDb)) {
                 auto& grdExpFirstChild = *grdExpPtr->children.begin();
                 if (customRootGrdExpPtr->children.size() == 0)
                     return &*grdExpFirstChild.second;

                 auto& customRootFirstChild = *customRootGrdExpPtr->children.begin();
                 if (customRootFirstChild.first == grdExpFirstChild.first)
                     return _skipCustomRoot(*customRootFirstChild.second, *grdExpFirstChild.second, pLingDb);
             }
        }
    }
    return &pSemExp;
}

void _completeWithContextInternallyToASemExp(SemanticExpression& pSemExp,
                                             const linguistics::LinguisticDatabase& pLingDb,
                                             const SemanticExpression* pAuthorSemExpPtr) {
    // complete with context between each sentence
    switch (pSemExp.type) {
        case SemanticExpressionType::GROUNDED: {
            GroundedExpression& grdExp = pSemExp.getGrdExp();
            auto itSubject = grdExp.children.find(GrammaticalType::SUBJECT);
            if (itSubject != grdExp.children.end()
                && SemExpGetter::doesSemExpCanBeCompletedWithContext(*itSubject->second))
                for (auto& currChild : grdExp.children)
                    if (currChild.first != GrammaticalType::SUBJECT)
                        for (auto& currElt : SemExpGetter::iterateOnListOfGrdExps(*currChild.second))
                            for (auto& currEltChild : currElt->children)
                                completeWithContext(currEltChild.second,
                                                    currEltChild.first,
                                                    *itSubject->second,
                                                    true,
                                                    pAuthorSemExpPtr,
                                                    _emptyMemBlock,
                                                    pLingDb);
            break;
        }
        case SemanticExpressionType::LIST: {
            ListExpression& listExp = pSemExp.getListExp();
            for (auto& currElt : listExp.elts)
                _completeWithContextInternallyToASemExp(*currElt, pLingDb, pAuthorSemExpPtr);

            auto itPrevElt = listExp.elts.begin();
            if (itPrevElt != listExp.elts.end()) {
                auto it = itPrevElt;
                ++it;
                while (it != listExp.elts.end()) {
                    completeWithContext(
                        *it, GrammaticalType::UNKNOWN, **itPrevElt, true, pAuthorSemExpPtr, _emptyMemBlock, pLingDb);
                    itPrevElt = it;
                    ++it;
                }
            }
            break;
        }
        case SemanticExpressionType::CONDITION: {
            ConditionExpression& condExp = pSemExp.getCondExp();
            _completeWithContextInternallyToASemExp(*condExp.conditionExp, pLingDb, pAuthorSemExpPtr);
            _completeWithContextInternallyToASemExp(*condExp.thenExp, pLingDb, pAuthorSemExpPtr);
            if (condExp.elseExp)
                _completeWithContextInternallyToASemExp(**condExp.elseExp, pLingDb, pAuthorSemExpPtr);
            break;
        }
        case SemanticExpressionType::FEEDBACK: {
            FeedbackExpression& fbkExp = pSemExp.getFdkExp();
            _completeWithContextInternallyToASemExp(*fbkExp.concernedExp, pLingDb, pAuthorSemExpPtr);
            break;
        }
        case SemanticExpressionType::INTERPRETATION: {
            InterpretationExpression& intExp = pSemExp.getIntExp();
            _completeWithContextInternallyToASemExp(*intExp.interpretedExp, pLingDb, pAuthorSemExpPtr);
            break;
        }
        case SemanticExpressionType::ANNOTATED: {
            AnnotatedExpression& annExp = pSemExp.getAnnExp();
            _completeWithContextInternallyToASemExp(*annExp.semExp, pLingDb, pAuthorSemExpPtr);
            break;
        }
        case SemanticExpressionType::METADATA: {
            MetadataExpression& metaExp = pSemExp.getMetadataExp();
            pAuthorSemExpPtr = metaExp.getAuthorSemExpPtr();
            _completeWithContextInternallyToASemExp(*metaExp.semExp, pLingDb, pAuthorSemExpPtr);
            break;
        }
        case SemanticExpressionType::FIXEDSYNTHESIS:
        case SemanticExpressionType::COMMAND:
        case SemanticExpressionType::COMPARISON:
        case SemanticExpressionType::SETOFFORMS: break;
    }
}

UniqueSemanticExpression _syntGraphToSemExp(
    const linguistics::SyntacticGraph& pSyntGraph,
    const TextProcessingContext& pLocutionContext,
    const SemanticTimeGrounding& pNowTimeGrd,
    bool pDoWeSplitQuestions,
    std::list<std::list<SemLineToPrint>>* pDebugOutput,
    std::unique_ptr<SemanticAgentGrounding> pAgentWeAreTalkingAbout = std::unique_ptr<SemanticAgentGrounding>()) {
    auto semExp =
        linguistics::convertToSemExp(pSyntGraph, pLocutionContext, pNowTimeGrd, std::move(pAgentWeAreTalkingAbout));
    splitter::splitInVerySimpleSentences(semExp, pDoWeSplitQuestions);
    semanticReasonOfRefactor::process(semExp);
    pSyntGraph.langConfig.lingDb.treeConverter.refactorSemExp(
        semExp, TREEPATTERN_INTEXT, TREEPATTERN_MIND, pSyntGraph.langConfig.getLanguageType(), pDebugOutput);
    if (pLocutionContext.linguisticAnalysisConfig.tryToResolveCoreferences)
        _completeWithContextInternallyToASemExp(*semExp, pSyntGraph.langConfig.lingDb, nullptr);
    return semExp;
}

void _naturalLanguageTextToSemanticWord(SemanticWord& pWord, const NaturalLanguageText& pNaturalLanguageText) {
    pWord.lemma = pNaturalLanguageText.text;
    if (pNaturalLanguageText.type == NaturalLanguageTypeOfText::VERB)
        pWord.partOfSpeech = PartOfSpeech::VERB;
    else
        pWord.partOfSpeech = PartOfSpeech::NOUN;
    pWord.language = pNaturalLanguageText.language;
}

}

void addDifferentForms(UniqueSemanticExpression& pSemExp,
                       const linguistics::LinguisticDatabase& pLingDb,
                       std::list<std::list<SemLineToPrint>>* pDebugOutput) {
    semanticOccurrenceRankConverter::process(pSemExp);
    // TODO: do a refactor inside splitPossibilitiesOfQuestion to not make difference between languages
    pLingDb.treeConverter.addDifferentForms(pSemExp, SemanticLanguageEnum::UNKNOWN, pDebugOutput);
}

void addBothDirectionForms(UniqueSemanticExpression& pSemExp,
                           const linguistics::LinguisticDatabase& pLingDb,
                           std::list<std::list<SemLineToPrint>>* pDebugOutput) {
    pLingDb.treeConverter.addBothDirectionForms(pSemExp, SemanticLanguageEnum::UNKNOWN, pDebugOutput);
}

void unsplitPossibilitiesOfQuestions(UniqueSemanticExpression& pSemExp) {
    switch (pSemExp->type) {
        case SemanticExpressionType::ANNOTATED: {
            auto& annExp = pSemExp->getAnnExp();
            unsplitPossibilitiesOfQuestions(annExp.semExp);
            break;
        }
        case SemanticExpressionType::COMMAND: {
            auto& cmdExp = pSemExp->getCmdExp();
            unsplitPossibilitiesOfQuestions(cmdExp.semExp);
            break;
        }
        case SemanticExpressionType::COMPARISON: {
            auto& compExp = pSemExp->getCompExp();
            unsplitPossibilitiesOfQuestions(compExp.leftOperandExp);
            if (compExp.rightOperandExp)
                unsplitPossibilitiesOfQuestions(*compExp.rightOperandExp);
            break;
        }
        case SemanticExpressionType::CONDITION: {
            auto& condExp = pSemExp->getCondExp();
            unsplitPossibilitiesOfQuestions(condExp.conditionExp);
            unsplitPossibilitiesOfQuestions(condExp.thenExp);
            if (condExp.elseExp)
                unsplitPossibilitiesOfQuestions(*condExp.elseExp);
            break;
        }
        case SemanticExpressionType::FEEDBACK: {
            auto& fdkExp = pSemExp->getFdkExp();
            unsplitPossibilitiesOfQuestions(fdkExp.concernedExp);
            unsplitPossibilitiesOfQuestions(fdkExp.feedbackExp);
            break;
        }
        case SemanticExpressionType::GROUNDED: {
            auto& grdExp = pSemExp->getGrdExp();
            for (auto& currChild : grdExp.children)
                unsplitPossibilitiesOfQuestions(currChild.second);
            break;
        }
        case SemanticExpressionType::INTERPRETATION: {
            auto& intExp = pSemExp->getIntExp();
            unsplitPossibilitiesOfQuestions(intExp.interpretedExp);
            break;
        }
        case SemanticExpressionType::LIST: {
            auto& listExp = pSemExp->getListExp();
            for (auto& currElt : listExp.elts)
                unsplitPossibilitiesOfQuestions(currElt);
            break;
        }
        case SemanticExpressionType::METADATA: {
            auto& metadataExp = pSemExp->getMetadataExp();
            unsplitPossibilitiesOfQuestions(metadataExp.semExp);
            if (metadataExp.source)
                unsplitPossibilitiesOfQuestions(*metadataExp.source);
            break;
        }
        case SemanticExpressionType::SETOFFORMS: {
            UniqueSemanticExpression* originalFromPtr = pSemExp->getSetOfFormsExp().getOriginalForm();
            if (originalFromPtr != nullptr) {
                pSemExp = std::move(*originalFromPtr);
                unsplitPossibilitiesOfQuestions(pSemExp);
            }
            break;
        }
        case SemanticExpressionType::FIXEDSYNTHESIS: break;
    }
}

UniqueSemanticExpression textToSemExp(const std::string& pText,
                                      const TextProcessingContext& pTextProcContext,
                                      const linguistics::LinguisticDatabase& pLingDb,
                                      bool pDoWeSplitQuestions,
                                      SemanticLanguageEnum* pExtractedLanguagePtr,
                                      std::unique_ptr<SemanticTimeGrounding>* pNowTimePtr,
                                      const std::list<std::string>* pReferencesPtr,
                                      std::unique_ptr<SemanticAgentGrounding> pAgentWeAreTalkingAbout,
                                      unsigned char* pConfidencePtr) {
    SemanticLanguageEnum language = [&] {
        if (pTextProcContext.langType == SemanticLanguageEnum::UNKNOWN)
            return linguistics::getLanguage(pText, pLingDb);
        return pTextProcContext.langType;
    }();
    if (pExtractedLanguagePtr != nullptr)
        *pExtractedLanguagePtr = language;

    auto nowTimeGrd = SemanticTimeGrounding::nowInstance();
    const SemanticTimeGrounding& nowTimeRef = [&] {
        if (pNowTimePtr != nullptr) {
            *pNowTimePtr = std::move(nowTimeGrd);
            return **pNowTimePtr;
        }
        return *nowTimeGrd;
    }();
    linguistics::SyntacticGraph syntGraph(pLingDb, language);
    linguistics::tokenizationAndSyntacticalAnalysis(syntGraph, pText, pTextProcContext.linguisticAnalysisConfig);
    if (pConfidencePtr != nullptr)
        *pConfidencePtr = syntGraph.parsingConfidence.toPercentage();
    auto resSemExp = _syntGraphToSemExp(
        syntGraph, pTextProcContext, nowTimeRef, pDoWeSplitQuestions, nullptr, std::move(pAgentWeAreTalkingAbout));

    if (pReferencesPtr != nullptr) {
        auto res = std::make_unique<MetadataExpression>(
            SemanticSourceEnum::UNKNOWN, UniqueSemanticExpression(), std::move(resSemExp));
        res->references = *pReferencesPtr;
        return res;
    }
    return resSemExp;
}

std::unique_ptr<MetadataExpression> wrapSemExpWithContextualInfos(UniqueSemanticExpression pSemExp,
                                                                  const std::string& pText,
                                                                  const TextProcessingContext& pLocutionContext,
                                                                  SemanticSourceEnum pFrom,
                                                                  SemanticLanguageEnum pLanguage,
                                                                  std::unique_ptr<SemanticTimeGrounding> pNowTimeGrd,
                                                                  unsigned char pConfidence,
                                                                  const std::list<std::string>* pReferencesPtr) {
    assert(pNowTimeGrd);
    auto source = MetadataExpression::constructSourceFromSourceEnum(
        std::make_unique<SemanticAgentGrounding>(pLocutionContext.author),
        std::make_unique<SemanticAgentGrounding>(pLocutionContext.receiver),
        pFrom,
        std::move(pNowTimeGrd));

    IndexToSubNameToParameterValue params;
    params[0].emplace("", std::make_unique<ReferenceOfSemanticExpressionContainer>(*pSemExp));
    static const std::set<SemanticExpressionType> expressionTypesToSkip{SemanticExpressionType::SETOFFORMS};
    source = source->clone(&params, true, &expressionTypesToSkip);
    if (SemExpGetter::doesSemExpContainsOnlyARequest(*pSemExp))
        SemExpModifier::replaceSayByAskToRobot(*source);

    auto res = std::make_unique<MetadataExpression>(pFrom, std::move(source), std::move(pSemExp));
    res->fromLanguage = pLanguage;
    res->fromText = pText;
    if (pReferencesPtr != nullptr)
        res->references = *pReferencesPtr;
    res->confidence = pConfidence;
    return res;
}

UniqueSemanticExpression textToContextualSemExp(const std::string& pText,
                                                const TextProcessingContext& pLocutionContext,
                                                SemanticSourceEnum pFrom,
                                                const linguistics::LinguisticDatabase& pLingDb,
                                                const std::list<std::string>* pReferencesPtr,
                                                std::unique_ptr<SemanticAgentGrounding> pAgentWeAreTalkingAbout) {
    SemanticLanguageEnum language = SemanticLanguageEnum::UNKNOWN;
    std::unique_ptr<SemanticTimeGrounding> nowTimeGrd;
    unsigned char confidence = 0;
    auto semExp = textToSemExp(pText,
                               pLocutionContext,
                               pLingDb,
                               false,
                               &language,
                               &nowTimeGrd,
                               nullptr,
                               std::move(pAgentWeAreTalkingAbout),
                               &confidence);
    return wrapSemExpWithContextualInfos(
        std::move(semExp), pText, pLocutionContext, pFrom, language, std::move(nowTimeGrd), confidence, pReferencesPtr);
}

UniqueSemanticExpression naturalLanguageExpressionToSemanticExpression(
    const NaturalLanguageExpression& pNaturalLanguageExpression,
    const linguistics::LinguisticDatabase& pLingDb,
    const std::vector<std::string>& pResourceLabels) {
    if (pNaturalLanguageExpression.word.type == NaturalLanguageTypeOfText::EXPRESSION) {
        auto textProc =
            TextProcessingContext::getTextProcessingContextFromRobot(pNaturalLanguageExpression.word.language);
        if (!pResourceLabels.empty())
            textProc.linguisticAnalysisConfig.cmdGrdExtractorPtr = std::make_shared<ResourceGroundingExtractor>(pResourceLabels);
        return textToSemExp(pNaturalLanguageExpression.word.text, textProc, pLingDb);
    }

    auto res = std::make_unique<GroundedExpression>([&]() -> std::unique_ptr<SemanticGrounding> {
        if (pNaturalLanguageExpression.word.type == NaturalLanguageTypeOfText::VERB) {
            auto statGrd = std::make_unique<SemanticStatementGrounding>();
            _naturalLanguageTextToSemanticWord(statGrd->word, pNaturalLanguageExpression.word);
            statGrd->verbTense = pNaturalLanguageExpression.verbTense;
            statGrd->verbGoal = pNaturalLanguageExpression.verbGoal;
            statGrd->polarity = pNaturalLanguageExpression.polarity;
            pLingDb.langToSpec[pNaturalLanguageExpression.word.language].lingDico.getConceptsFromWord(statGrd->concepts,
                                                                                                      statGrd->word);
            return statGrd;
        }
        if (pNaturalLanguageExpression.word.type == NaturalLanguageTypeOfText::AGENT) {
            return std::make_unique<SemanticAgentGrounding>(pNaturalLanguageExpression.word.text);
        }
        if (pNaturalLanguageExpression.word.type == NaturalLanguageTypeOfText::QUOTE) {
            return std::make_unique<SemanticTextGrounding>(pNaturalLanguageExpression.word.text);
        }
        auto genGrd = std::make_unique<SemanticGenericGrounding>();
        _naturalLanguageTextToSemanticWord(genGrd->word, pNaturalLanguageExpression.word);
        genGrd->quantity = pNaturalLanguageExpression.quantity;
        genGrd->referenceType = pNaturalLanguageExpression.reference;
        pLingDb.langToSpec[pNaturalLanguageExpression.word.language].lingDico.getConceptsFromWord(genGrd->concepts,
                                                                                                  genGrd->word);
        return genGrd;
    }());
    for (const auto& currChild : pNaturalLanguageExpression.children)
        res->children.emplace(currChild.first,
                              naturalLanguageExpressionToSemanticExpression(currChild.second, pLingDb));
    return res;
}

UniqueSemanticExpression agentIdWithNameToSemExp(const std::string& pAgentId, const std::vector<std::string>& pNames) {
    // fill verb
    auto res = std::make_unique<GroundedExpression>([]() {
        auto statementGrd = std::make_unique<SemanticStatementGrounding>();
        statementGrd->verbTense = SemanticVerbTense::PRESENT;
        statementGrd->concepts.emplace(ConceptSet::conceptVerbEquality, 4);
        return statementGrd;
    }());

    // fill subject
    res->children.emplace(GrammaticalType::SUBJECT,
                          std::make_unique<GroundedExpression>(std::make_unique<SemanticAgentGrounding>(pAgentId)));

    // fill object
    res->children.emplace(GrammaticalType::OBJECT,
                          std::make_unique<GroundedExpression>(std::make_unique<SemanticNameGrounding>(pNames)));

    return res;
}

UniqueSemanticExpression syntGraphToSemExp(const linguistics::SyntacticGraph& pSyntGraph,
                                           const TextProcessingContext& pLocutionContext,
                                           std::list<std::list<SemLineToPrint>>* pDebugOutput) {
    auto nowTimeGrd = SemanticTimeGrounding::nowInstance();
    return _syntGraphToSemExp(pSyntGraph, pLocutionContext, *nowTimeGrd, false, pDebugOutput);
}

void semExpToSentiments(std::list<std::unique_ptr<SentimentContext>>& pSentInfos,
                        const SemanticExpression& pSemExp,
                        const ConceptSet& pConceptSet) {
    const SemanticAgentGrounding* authorPtr = SemExpGetter::extractAuthor(pSemExp);
    if (authorPtr != nullptr)
        sentimentDetector::semExpToSentimentInfos(pSentInfos, pSemExp, *authorPtr, pConceptSet);
}

void semExpToText(std::string& pResStr,
                  UniqueSemanticExpression pSemExp,
                  const TextProcessingContext& pTextProcContext,
                  bool pOneLinePerSentence,
                  const SemanticMemoryBlock& pMemBlock,
                  const std::string& pCurrentUserId,
                  const linguistics::LinguisticDatabase& pLingDb,
                  std::list<std::list<SemLineToPrint>>* pDebugOutput) {
    std::list<std::unique_ptr<SynthesizerResult>> res;
    synthesize(res,
               std::move(pSemExp),
               pOneLinePerSentence,
               pMemBlock,
               pCurrentUserId,
               pTextProcContext,
               pLingDb,
               pDebugOutput);
    for (const auto& currElt : res) {
        switch (currElt->type) {
            case SynthesizerResultEnum::TEXT: {
                const auto& syntText = *dynamic_cast<const SynthesizerText*>(&*currElt);
                pResStr += syntText.text;
                break;
            }
            case SynthesizerResultEnum::TASK: {
                const auto& syntTask = *dynamic_cast<const SynthesizerTask*>(&*currElt);
                pResStr += syntTask.resource.value;
                break;
            }
        }
    }
}

void semExpToText(std::string& pResStr,
                  UniqueSemanticExpression pSemExp,
                  const TextProcessingContext& pTextProcContext,
                  bool pOneLinePerSentence,
                  const SemanticMemory& pSemanticMemory,
                  const linguistics::LinguisticDatabase& pLingDb,
                  std::list<std::list<SemLineToPrint>>* pDebugOutput) {
    semExpToText(pResStr,
                 std::move(pSemExp),
                 pTextProcContext,
                 pOneLinePerSentence,
                 pSemanticMemory.memBloc,
                 pSemanticMemory.getCurrUserId(),
                 pLingDb,
                 pDebugOutput);
}


UniqueSemanticExpression getImperativeAssociateFrom(UniqueSemanticExpression pUSemExp) {
    auto* grdExpPtr = pUSemExp->getGrdExpPtr_SkipWrapperPtrs();
    if (grdExpPtr != nullptr)
        return SemExpCreator::getImperativeAssociateForm(*grdExpPtr);
    return pUSemExp;
}


void infinitiveToRequestVariations(std::list<UniqueSemanticExpression>& pOuts,
                                   UniqueSemanticExpression pUSemExp) {
    const UniqueSemanticExpression* imperativeSemExpPtr = nullptr;
    {
        auto* grdExpPtr = pUSemExp->getGrdExpPtr_SkipWrapperPtrs();
        if (grdExpPtr != nullptr)
        {
            pOuts.emplace_back(SemExpCreator::getImperativeAssociateForm(*grdExpPtr));
            imperativeSemExpPtr = &pOuts.back();
        }
    }

    if (imperativeSemExpPtr != nullptr)
    {
        auto* grdExpPtr = (*imperativeSemExpPtr)->getGrdExpPtr_SkipWrapperPtrs();
        if (grdExpPtr != nullptr)
            pOuts.emplace_back(SemExpCreator::iWantThatYou(SemanticAgentGrounding::getCurrentUser(),
                                                           SemExpCreator::getIndicativeFromImperative(*grdExpPtr)));
    }

    {
        pOuts.emplace_back(SemExpCreator::infToDoYouWant(std::move(pUSemExp)));
    }
}

UniqueSemanticExpression getFutureIndicativeAssociatedForm(UniqueSemanticExpression pUSemExp) {
    auto* grdExpPtr = pUSemExp->getGrdExpPtr_SkipWrapperPtrs();
    if (grdExpPtr != nullptr)
        return SemExpCreator::getFutureIndicativeAssociatedForm(*grdExpPtr);

    auto* listExpPtr = pUSemExp->getListExpPtr_SkipWrapperPtrs();
    if (listExpPtr != nullptr) {
        auto& listExp = *listExpPtr;
        auto res = std::make_unique<ListExpression>(listExp.listType);
        for (auto& currElt : listExp.elts)
            res->elts.push_back(getFutureIndicativeAssociatedForm(std::move(currElt)));
        return res;
    }
    return UniqueSemanticExpression();
}

std::unique_ptr<UniqueSemanticExpression> imperativeToInfinitive(const SemanticExpression& pSemExp) {
    auto* grdExpPtr = pSemExp.getGrdExpPtr_SkipWrapperPtrs();
    if (grdExpPtr != nullptr) {
        auto* statGrdPtr = grdExpPtr->grounding().getStatementGroundingPtr();
        if (statGrdPtr != nullptr) {
            auto& statGrd = *statGrdPtr;
            if (statGrd.requests.has(SemanticRequestType::ACTION))
                return std::make_unique<UniqueSemanticExpression>(
                    SemExpCreator::getInfinitiveFromImperativeForm(*grdExpPtr));
        }
    }
    return {};
}

void imperativeToMandatory(UniqueSemanticExpression& pSemExp) {
    mandatoryFormConverter::process(pSemExp);
}

UniqueSemanticExpression constructTeachSemExp(UniqueSemanticExpression pInfitiveLabelSemExp,
                                              UniqueSemanticExpression pSemExpToDo) {
    auto res = std::make_unique<GroundedExpression>();
    res->children.emplace(GrammaticalType::PURPOSE, std::move(pInfitiveLabelSemExp));
    res->children.emplace(GrammaticalType::OBJECT, std::move(pSemExpToDo));
    return res;
}

void addOtherTriggerFormulations(std::list<UniqueSemanticExpression>& pRes, const SemanticExpression& pSemExp) {
    auto inf = imperativeToInfinitive(pSemExp);
    if (inf)
        pRes.emplace_back(std::move(*inf));
}


void createParameterSemanticexpressions(
    std::map<std::string, std::vector<UniqueSemanticExpression>>& pParameterLabelToQuestionsSemExps,
    const std::map<std::string, std::vector<std::string>>& pParameterLabelToQuestionsStrs,
    const linguistics::LinguisticDatabase& pLingDb,
    SemanticLanguageEnum pLanguage) {
    TextProcessingContext paramQuestionProcContext(
        SemanticAgentGrounding::getMe(), SemanticAgentGrounding::getCurrentUser(), pLanguage);
    paramQuestionProcContext.isTimeDependent = false;
    for (auto& currLabelToQuestions : pParameterLabelToQuestionsStrs) {
        for (auto& currQuestion : currLabelToQuestions.second) {
            auto paramSemExp = converter::textToSemExp(currQuestion, paramQuestionProcContext, pLingDb);
            pParameterLabelToQuestionsSemExps[currLabelToQuestions.first].emplace_back(std::move(paramSemExp));
        }
    }
}

std::unique_ptr<SemanticResourceGrounding> createResourceWithParameters(
    const std::string& pResourceLabel,
    const std::string& pResourceValue,
    const std::map<std::string, std::vector<std::string>>& pParameterLabelToQuestionsStrs,
    const SemanticExpression& pContextForParameters,
    const linguistics::LinguisticDatabase& pLingDb,
    SemanticLanguageEnum pLanguage) {
    std::map<std::string, std::vector<UniqueSemanticExpression>> parameterLabelToQuestionsSemExps;
    createParameterSemanticexpressions(
        parameterLabelToQuestionsSemExps, pParameterLabelToQuestionsStrs, pLingDb, pLanguage);
    return createResourceWithParametersFromSemExp(pResourceLabel, pResourceValue, parameterLabelToQuestionsSemExps,
                                                  pContextForParameters, pLingDb, pLanguage);
}

std::unique_ptr<SemanticResourceGrounding> createResourceWithParametersFromSemExp(
    const std::string& pResourceLabel,
    const std::string& pResourceValue,
    const std::map<std::string, std::vector<UniqueSemanticExpression>>& pParameterLabelToQuestionsSemExps,
    const SemanticExpression& pContextForParameters,
    const linguistics::LinguisticDatabase& pLingDb,
    SemanticLanguageEnum pLanguage) {
    auto res = std::make_unique<SemanticResourceGrounding>(pResourceLabel, pLanguage, pResourceValue);

    for (auto& currLabelToQuestions : pParameterLabelToQuestionsSemExps) {
        for (auto& currQuestionSemExp : currLabelToQuestions.second) {
            SemanticMemory semMemory;
            memoryOperation::inform(
                std::make_unique<MetadataExpression>(
                    SemanticSourceEnum::WRITTENTEXT, UniqueSemanticExpression(), pContextForParameters.clone()),
                semMemory,
                pLingDb);
            UniqueSemanticExpression questionMergedWithContext = currQuestionSemExp->clone();
            memoryOperation::mergeWithContext(questionMergedWithContext, semMemory, pLingDb);
            res->resource.parameterLabelsToQuestions[currLabelToQuestions.first].emplace_back(
                std::move(questionMergedWithContext));
        }
    }
    return res;
}

void extractParameters(std::map<std::string, std::vector<UniqueSemanticExpression>>& pParameters,
                       const std::map<std::string, std::vector<UniqueSemanticExpression>>& pParameterLabelsToQuestions,
                       UniqueSemanticExpression pInputSemExp,
                       const linguistics::LinguisticDatabase& pLingDb) {
    SemanticMemory semMemory;
    auto expWithLinks = memoryOperation::inform(std::move(pInputSemExp), semMemory, pLingDb);

    for (const auto& currParam : pParameterLabelsToQuestions) {
        for (const auto& currQuestion : currParam.second) {
            UniqueSemanticExpression questionSemExp = currQuestion->clone();
            std::vector<std::unique_ptr<GroundedExpression>> answers;
            memoryOperation::get(answers, std::move(questionSemExp), semMemory, pLingDb);

            // Little hack to still get the answer even if the child answer is not well positionned in the tree
            if (answers.empty()) {
                auto* questionRequestPtr = SemExpGetter::getRequestListFromSemExp(*currQuestion);
                if (questionRequestPtr != nullptr && !questionRequestPtr->empty()) {
                    auto requestType = questionRequestPtr->types.front();
                    if (requestType != SemanticRequestType::QUANTITY) {
                        GrammaticalType grammaticalChildrenType = semanticRequestType_toSemGram(requestType);
                        if (grammaticalChildrenType != GrammaticalType::UNKNOWN && expWithLinks) {
                            auto answerChildSemExpPtr = SemExpGetter::getChildFromSemExpRecursively(
                                *expWithLinks->semExp, grammaticalChildrenType);
                            if (answerChildSemExpPtr != nullptr) {

                                auto questionChildPtr = SemExpGetter::getChildFromSemExpRecursively(*currQuestion, grammaticalChildrenType);
                                if (questionChildPtr != nullptr)
                                    answerChildSemExpPtr = _skipCustomRoot(*answerChildSemExpPtr, *questionChildPtr, pLingDb);

                                if (answerChildSemExpPtr != nullptr) {
                                    std::list<const GroundedExpression*> grdExpPtrs;
                                    answerChildSemExpPtr->getGrdExpPtrs_SkipWrapperLists(grdExpPtrs);
                                    for (auto* currGrdExpPtr : grdExpPtrs)
                                        answers.emplace_back(currGrdExpPtr->clone());
                                }
                            }
                        }
                    }
                }
            }

            if (!answers.empty()) {
                auto& exitingParams = pParameters[currParam.first];
                for (auto& currAnswer : answers)
                    exitingParams.emplace_back(std::move(currAnswer));
                break;
            }
        }
    }
}



void correferenceToRobot(UniqueSemanticExpression& pSemExp,
                         const linguistics::LinguisticDatabase& pLingDb) {
    switch (pSemExp->type) {
    case SemanticExpressionType::ANNOTATED: {
        AnnotatedExpression& annExp = pSemExp->getAnnExp();
        correferenceToRobot(annExp.semExp, pLingDb);
        for (auto& currAnn : annExp.annotations)
            correferenceToRobot(currAnn.second, pLingDb);
        break;
    }
    case SemanticExpressionType::COMMAND: {
        CommandExpression& cmdExp = pSemExp->getCmdExp();
        correferenceToRobot(cmdExp.semExp, pLingDb);
        break;
    }
    case SemanticExpressionType::FEEDBACK: {
        FeedbackExpression& fdkExp = pSemExp->getFdkExp();
        correferenceToRobot(fdkExp.concernedExp, pLingDb);
        break;
    }
    case SemanticExpressionType::GROUNDED: {
        GroundedExpression& grdExp = pSemExp->getGrdExp();
        if (grdExp.grounding().type == SemanticGroundingType::GENERIC) {
            SemanticGenericGrounding& genGrd = grdExp->getGenericGrounding();
            if (SemExpGetter::isASpecificHuman(genGrd) &&
                SemExpGetter::isACoreferenceFromGenericGrounding(genGrd, CoreferenceDirectionEnum::BEFORE)) {
                grdExp.moveGrounding(SemanticAgentGrounding::getRobotAgentPtr());
            }
        }
        if (grdExp.grounding().type == SemanticGroundingType::META) {
            SemanticMetaGrounding& metaGrd = grdExp->getMetaGrounding();
            if (metaGrd.attibuteName == "self") {
                grdExp.moveGrounding(SemanticAgentGrounding::getRobotAgentPtr());
            }
        }

        for (auto& child : grdExp.children) {
            correferenceToRobot(child.second, pLingDb);
        }
        break;
    }
    case SemanticExpressionType::INTERPRETATION: {
        InterpretationExpression& intExp = pSemExp->getIntExp();
        correferenceToRobot(intExp.interpretedExp, pLingDb);
        if (intExp.source == InterpretationSource::STATEMENTCOREFERENCE)
            correferenceToRobot(intExp.originalExp, pLingDb);
        break;
    }
    case SemanticExpressionType::LIST: {
        ListExpression& listExp = pSemExp->getListExp();
        for (auto& elt : listExp.elts) {
            correferenceToRobot(elt, pLingDb);
        }
        break;
    }
    case SemanticExpressionType::METADATA: {
        MetadataExpression& metaExp = pSemExp->getMetadataExp();
        if (metaExp.source) {
            correferenceToRobot(*metaExp.source, pLingDb);
        }
        correferenceToRobot(metaExp.semExp, pLingDb);
        break;
    }
    case SemanticExpressionType::CONDITION: {
        ConditionExpression& condExp = pSemExp->getCondExp();
        correferenceToRobot(condExp.conditionExp, pLingDb);
        correferenceToRobot(condExp.thenExp, pLingDb);
        if (condExp.elseExp) {
            correferenceToRobot(*condExp.elseExp, pLingDb);
        }
        break;
    }
    case SemanticExpressionType::FIXEDSYNTHESIS: {
        FixedSynthesisExpression& fSynthExp = pSemExp->getFSynthExp();
        correferenceToRobot(fSynthExp.getUSemExp(), pLingDb);
        break;
    }
    case SemanticExpressionType::COMPARISON:
    case SemanticExpressionType::SETOFFORMS: break;
    }
}

}    // End of namespace converter
}    // End of namespace onsem
