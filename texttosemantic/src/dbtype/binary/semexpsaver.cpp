#include <onsem/texttosemantic/dbtype/binary/semexpsaver.hpp>
#include <onsem/common/binary/binarysaver.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase.hpp>
#include <onsem/texttosemantic/dbtype/linguisticdatabase/staticconceptset.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpressions.hpp>
#include <onsem/texttosemantic/dbtype/semanticgroundings.hpp>

namespace onsem {
namespace semexpsaver {
namespace {

void _writeConcepts(binarymasks::Ptr& pPtr,
                    const std::map<std::string, char>& pConcepts,
                    const StaticConceptSet& pCptSet) {
    auto* beginPchar = pPtr.pchar++;

    auto* beginNb = pPtr.pchar++;
    bool writeCptIds = false;
    std::list<std::pair<std::string, char>> concepts;
    std::size_t nbOfCptIds = 0;
    for (const auto& currCpt : pConcepts) {
        int cptId = pCptSet.getConceptId(currCpt.first);
        if (cptId != StaticConceptSet::noConcept) {
            binarysaver::writeInThreeBytes(pPtr.pchar, binarysaver::alignedDecToSave(cptId));
            pPtr.val += 3;
            binarysaver::writeChar(pPtr.pchar++, currCpt.second);
            writeCptIds = true;
            ++nbOfCptIds;
        } else {
            concepts.emplace_back(currCpt);
        }
    }
    binarysaver::writeChar_6(beginPchar, writeCptIds);
    assert(concepts.size() < 128);
    unsigned char cptStrSize = binarysaver::sizet_to_uchar(concepts.size());
    binarysaver::writeChar_7(beginPchar, cptStrSize > 0);

    if (writeCptIds)
        binarysaver::writeChar(beginNb, nbOfCptIds);
    else
        --pPtr.pchar;

    if (cptStrSize > 0) {
        binarysaver::writeChar(pPtr.pchar++, cptStrSize);
        for (const auto& currCpt : concepts) {
            binarysaver::writeString(pPtr, currCpt.first);
            binarysaver::writeChar(pPtr.pchar++, currCpt.second);
        }
    }
}

void _writeGroundingMotherClass(binarymasks::Ptr& pPtr,
                                const SemanticGrounding& pGrd,
                                const StaticConceptSet& pCptSet) {
    binarysaver::writeChar_0To4(pPtr.pchar, semanticGroundingsType_toChar(pGrd.type));
    if (pGrd.type == SemanticGroundingType::AGENT) {
        ++pPtr.pchar;
        return;
    }
    binarysaver::writeChar_5(pPtr.pchar, pGrd.polarity);
    _writeConcepts(pPtr, pGrd.concepts, pCptSet);
}

void _writeWord(binarymasks::Ptr& pPtr, const SemanticWord& pWord, const linguistics::LinguisticDatabase& pLingDb) {
    const bool writeWord = !pWord.isEmpty();
    binarysaver::writeChar_0(pPtr.pchar, writeWord);
    if (writeWord) {
        auto& statDico = pLingDb.langToSpec[pWord.language].lingDico.statDb;
        auto statLingMeaning = statDico.getLingMeaning(pWord.lemma, pWord.partOfSpeech, true);
        const bool writeWordId = !statLingMeaning.isEmpty();
        binarysaver::writeChar_1(pPtr.pchar, writeWordId);
        binarysaver::writeChar_2To7(pPtr.pchar, semanticLanguageEnum_toChar(pWord.language));
        ++pPtr.pchar;
        if (writeWordId) {
            binarysaver::writeInThreeBytes(pPtr.pchar, binarysaver::alignedDecToSave(statLingMeaning.meaningId));
            pPtr.val += 3;
        } else {
            binarysaver::writeString(pPtr, pWord.lemma);
            binarysaver::writeChar(pPtr.pchar++, partOfSpeech_toChar(pWord.partOfSpeech));
        }
    }
}

void _writeCoreference(signed char* pPchar, const mystd::optional<Coreference>& pCorefOpt) {
    const bool coreferenceHasValue = pCorefOpt.has_value();
    binarysaver::writeChar_3(pPchar, coreferenceHasValue);
    if (coreferenceHasValue)
        binarysaver::writeChar_4To7(pPchar, coreferenceDirectionEnum_toChar(pCorefOpt->getDirection()));
}

void _writeGenders(binarymasks::Ptr& pPtr, const std::set<SemanticGenderType>& pGenders) {
    unsigned char nbOfGenders = binarysaver::sizet_to_uchar(pGenders.size());
    binarysaver::writeChar_1To2(pPtr.pchar, nbOfGenders);
    if (nbOfGenders > 0) {
        auto itGender = pGenders.begin();
        binarysaver::writeChar_4To5(pPtr.pchar, semanticGenderType_toChar(*itGender));
        ++itGender;
        if (itGender == pGenders.end()) {
            ++pPtr.pchar;
            return;
        }
        binarysaver::writeChar_6To7(pPtr.pchar, semanticGenderType_toChar(*itGender));
        ++pPtr.pchar;
        ++itGender;
        if (itGender == pGenders.end())
            return;
        binarysaver::writeChar_0To1(pPtr.pchar, semanticGenderType_toChar(*itGender));
        ++itGender;
        if (itGender == pGenders.end()) {
            ++pPtr.pchar;
            return;
        }
        binarysaver::writeChar_2To3(pPtr.pchar, semanticGenderType_toChar(*itGender));
        ++pPtr.pchar;
        assert(++itGender == pGenders.end());
    } else {
        ++pPtr.pchar;
    }
}

void _writeNameInfos(binarymasks::Ptr& pPtr, const NameInfos& pNameInfos) {
    _writeGenders(pPtr, pNameInfos.possibleGenders);
    unsigned char nbOfNames = binarysaver::sizet_to_uchar(pNameInfos.names.size());
    binarysaver::writeChar(pPtr.pchar++, nbOfNames);
    for (unsigned char nameId = 0; nameId < nbOfNames; ++nameId)
        binarysaver::writeString(pPtr, pNameInfos.names[nameId]);
}

void _writeCharOrInt(binarymasks::Ptr& pPtr, int pVal, bool pCharOrInt) {
    if (pCharOrInt)
        binarysaver::writeChar(pPtr.pchar++, static_cast<char>(pVal));
    else
        binarysaver::writeInt(pPtr.pint++, pVal);
}

bool _canSemanticFloatBeWrittenInAChar(const SemanticFloat& pSemanticFloat) {
    return pSemanticFloat.isPositive() && pSemanticFloat.isAnInteger()
        && binarysaver::intCanBeStoredInAChar(pSemanticFloat.value);
}

void _writeSemanticFloat(binarymasks::Ptr& pPtr, const SemanticFloat& pSemanticFloat, bool pNbCanBeWrittenInAChar) {
    _writeCharOrInt(pPtr, pSemanticFloat.value, pNbCanBeWrittenInAChar);
    if (!pNbCanBeWrittenInAChar) {
        binarysaver::writeInt(pPtr.pint++, pSemanticFloat.valueAfterTheDecimalPoint);
        binarysaver::writeChar_0(pPtr.pchar, pSemanticFloat.isPositive());
        binarysaver::writeChar_1To7(pPtr.pchar, pSemanticFloat.nbOfSignificantDigit);
        ++pPtr.pchar;
    }
}

void _writeAngle(binarymasks::Ptr& pPtr, const SemanticAngle& pAngle) {
    unsigned char nbOfangleInfos = binarysaver::sizet_to_uchar(pAngle.angleInfos.size());
    binarysaver::writeChar_0To6(pPtr.pchar, nbOfangleInfos);
    ++pPtr.pchar;
    for (const auto& currAngleInfo : pAngle.angleInfos) {
        const bool charOrInt = _canSemanticFloatBeWrittenInAChar(currAngleInfo.second);
        binarysaver::writeChar_0(pPtr.pchar, charOrInt);
        binarysaver::writeChar_1To7(pPtr.pchar, semanticAngleUnity_toChar(currAngleInfo.first));
        ++pPtr.pchar;
        _writeSemanticFloat(pPtr, currAngleInfo.second, charOrInt);
    }
}

void _writeLength(binarymasks::Ptr& pPtr, const SemanticLength& pLength) {
    unsigned char nbOfLengthInfos = binarysaver::sizet_to_uchar(pLength.lengthInfos.size());
    binarysaver::writeChar_0To6(pPtr.pchar, nbOfLengthInfos);
    ++pPtr.pchar;
    for (const auto& currLengthInfo : pLength.lengthInfos) {
        const bool charOrInt = _canSemanticFloatBeWrittenInAChar(currLengthInfo.second);
        binarysaver::writeChar_0(pPtr.pchar, charOrInt);
        binarysaver::writeChar_1To7(pPtr.pchar, semanticLengthUnity_toChar(currLengthInfo.first));
        ++pPtr.pchar;
        _writeSemanticFloat(pPtr, currLengthInfo.second, charOrInt);
    }
}

void _writeDuration(binarymasks::Ptr& pPtr, const SemanticDuration& pDuration) {
    binarysaver::writeChar_0(pPtr.pchar, pDuration.sign == Sign::POSITIVE);
    unsigned char nbOfTimeInfos = binarysaver::sizet_to_uchar(pDuration.timeInfos.size());
    binarysaver::writeChar_1To7(pPtr.pchar, nbOfTimeInfos);
    ++pPtr.pchar;
    for (const auto& currTimeInfo : pDuration.timeInfos) {
        const bool charOrInt = _canSemanticFloatBeWrittenInAChar(currTimeInfo.second);
        binarysaver::writeChar_0(pPtr.pchar, charOrInt);
        binarysaver::writeChar_1To7(pPtr.pchar, semanticTimeUnity_toChar(currTimeInfo.first));
        ++pPtr.pchar;
        _writeSemanticFloat(pPtr, currTimeInfo.second, charOrInt);
    }
}

void _writeGrounding(binarymasks::Ptr& pPtr,
                     const SemanticGrounding& pGrd,
                     const linguistics::LinguisticDatabase& pLingDb) {
    _writeGroundingMotherClass(pPtr, pGrd, pLingDb.conceptSet.statDb);
    switch (pGrd.type) {
        case SemanticGroundingType::GENERIC: {
            auto& genGrd = pGrd.getGenericGrounding();
            binarysaver::writeChar_0To2(pPtr.pchar, semanticReferenceType_tochar(genGrd.referenceType));
            _writeCoreference(pPtr.pchar++, genGrd.coreference);
            binarysaver::writeChar_0To5(pPtr.pchar, semanticEntityType_toChar(genGrd.entityType));
            const bool writeTheQuantity = (!genGrd.quantity.isUnknown() && !genGrd.quantity.isEqualToOne())
                                       || genGrd.quantity.subjectiveValue != SemanticSubjectiveQuantity::UNKNOWN
                                       || !genGrd.quantity.paramSpec.empty();
            binarysaver::writeChar_6(pPtr.pchar, writeTheQuantity);
            if (writeTheQuantity) {
                const bool nbCanBeWrittenInAChar = _canSemanticFloatBeWrittenInAChar(genGrd.quantity.nb);
                binarysaver::writeChar_7(pPtr.pchar, nbCanBeWrittenInAChar);
                ++pPtr.pchar;
                binarysaver::writeChar_0To3(pPtr.pchar, semanticQuantityType_toChar(genGrd.quantity.type));
                binarysaver::writeChar_4To7(pPtr.pchar,
                                            semanticSubjectiveQuantity_toChar(genGrd.quantity.subjectiveValue));
                ++pPtr.pchar;
                _writeSemanticFloat(pPtr, genGrd.quantity.nb, nbCanBeWrittenInAChar);
                binarysaver::writeString(pPtr, genGrd.quantity.paramSpec);
            } else {
                binarysaver::writeChar_7(pPtr.pchar, genGrd.quantity.isUnknown());
                ++pPtr.pchar;
            }
            _writeWord(pPtr, genGrd.word, pLingDb);
            _writeGenders(pPtr, genGrd.possibleGenders);
            return;
        }
        case SemanticGroundingType::STATEMENT: {
            auto& statGrd = pGrd.getStatementGrounding();
            _writeWord(pPtr, statGrd.word, pLingDb);
            const bool writeVerbGoalAndCoreferenceAndIsPassive =
                statGrd.verbGoal != VerbGoalEnum::NOTIFICATION || statGrd.coreference.has_value() || statGrd.isPassive;
            binarysaver::writeChar_1(pPtr.pchar, writeVerbGoalAndCoreferenceAndIsPassive);
            unsigned char nbOfRequests = binarysaver::sizet_to_uchar(statGrd.requests.types.size());
            binarysaver::writeChar_2To3(pPtr.pchar, nbOfRequests);
            binarysaver::writeChar_4To7(pPtr.pchar, semanticVerbTense_toChar(statGrd.verbTense));
            ++pPtr.pchar;
            if (writeVerbGoalAndCoreferenceAndIsPassive) {
                binarysaver::writeChar_0To2(pPtr.pchar, semVerbGoalEnum_toChar(statGrd.verbGoal));
                _writeCoreference(pPtr.pchar++, statGrd.coreference);
                binarysaver::writeChar_1(pPtr.pchar, statGrd.isPassive);
                if (statGrd.isPassive)
                    binarysaver::writeChar_2(pPtr.pchar, *statGrd.isPassive);
                pPtr.pchar++;
            }
            for (const auto& currRequest : statGrd.requests.types)
                binarysaver::writeChar(pPtr.pchar++, semanticRequestType_toChar(currRequest));
            return;
        }
        case SemanticGroundingType::AGENT: {
            auto& agentGrd = pGrd.getAgentGrounding();
            binarysaver::writeString(pPtr, agentGrd.userId);
            binarysaver::writeString(pPtr, agentGrd.userIdWithoutContext);
            const bool writeNameInfos = agentGrd.nameInfos.has_value();
            binarysaver::writeChar_0(pPtr.pchar, writeNameInfos);
            if (writeNameInfos)
                _writeNameInfos(pPtr, *agentGrd.nameInfos);
            else
                ++pPtr.pchar;
            return;
        }
        case SemanticGroundingType::ANGLE: {
            auto& angleGrd = pGrd.getAngleGrounding();
            _writeAngle(pPtr, angleGrd.angle);
            return;
        }
        case SemanticGroundingType::NAME: {
            auto& nameGrd = pGrd.getNameGrounding();
            _writeNameInfos(pPtr, nameGrd.nameInfos);
            return;
        }
        case SemanticGroundingType::TIME: {
            auto& timeGrd = pGrd.getTimeGrounding();
            const bool writeYear = timeGrd.date.year.has_value();
            const bool writeYearInAChar = writeYear && binarysaver::intCanBeStoredInAChar(*timeGrd.date.year);
            const bool writeMonth = timeGrd.date.month.has_value();
            const bool writeMonthInAChar = writeMonth && binarysaver::intCanBeStoredInAChar(*timeGrd.date.month);
            const bool writeDay = timeGrd.date.day.has_value();
            const bool writeDayInAChar = writeDay && binarysaver::intCanBeStoredInAChar(*timeGrd.date.day);
            binarysaver::writeChar_0(pPtr.pchar, writeYear);
            binarysaver::writeChar_1(pPtr.pchar, writeYearInAChar);
            binarysaver::writeChar_2(pPtr.pchar, writeMonth);
            binarysaver::writeChar_3(pPtr.pchar, writeMonthInAChar);
            binarysaver::writeChar_4(pPtr.pchar, writeDay);
            binarysaver::writeChar_5(pPtr.pchar, writeDayInAChar);
            ++pPtr.pchar;
            if (writeYear)
                _writeCharOrInt(pPtr, *timeGrd.date.year, writeYearInAChar);
            if (writeMonth)
                _writeCharOrInt(pPtr, *timeGrd.date.month, writeMonthInAChar);
            if (writeDay)
                _writeCharOrInt(pPtr, *timeGrd.date.day, writeDayInAChar);
            _writeDuration(pPtr, timeGrd.timeOfDay);
            _writeDuration(pPtr, timeGrd.length);
            _writeConcepts(pPtr, timeGrd.fromConcepts, pLingDb.conceptSet.statDb);
            return;
        }
        case SemanticGroundingType::LENGTH: {
            auto& lengthGrd = pGrd.getLengthGrounding();
            _writeLength(pPtr, lengthGrd.length);
            return;
        }
        case SemanticGroundingType::DURATION: {
            auto& durationGrd = pGrd.getDurationGrounding();
            _writeDuration(pPtr, durationGrd.duration);
            return;
        }
        case SemanticGroundingType::TEXT: {
            auto& textGrd = pGrd.getTextGrounding();
            binarysaver::writeString(pPtr, textGrd.text);
            binarysaver::writeChar_0To6(pPtr.pchar, semanticLanguageEnum_toChar(textGrd.forLanguage));
            binarysaver::writeChar_7(pPtr.pchar, textGrd.hasQuotationMark);
            ++pPtr.pchar;
            return;
        }
        case SemanticGroundingType::META: {
            auto& metaGrd = pGrd.getMetaGrounding();
            binarysaver::writeChar_0To6(pPtr.pchar, semanticGroundingsType_toChar(metaGrd.refToType));
            const bool paramIdCanBeWrittenInAChar = binarysaver::intCanBeStoredInAChar(metaGrd.paramId);
            binarysaver::writeChar_7(pPtr.pchar, paramIdCanBeWrittenInAChar);
            ++pPtr.pchar;
            _writeCharOrInt(pPtr, metaGrd.paramId, paramIdCanBeWrittenInAChar);
            binarysaver::writeString(pPtr, metaGrd.attibuteName);
            return;
        }
        case SemanticGroundingType::RESOURCE: {
            auto& resGrd = pGrd.getResourceGrounding();
            binarysaver::writeString(pPtr, resGrd.resource.label);
            binarysaver::writeChar(pPtr.pchar++, semanticLanguageEnum_toChar(resGrd.resource.language));
            binarysaver::writeString(pPtr, resGrd.resource.value);
            return;
        }
        case SemanticGroundingType::LANGUAGE: {
            auto& langGrd = pGrd.getLanguageGrounding();
            binarysaver::writeChar(pPtr.pchar++, semanticLanguageEnum_toChar(langGrd.language));
            return;
        }
        case SemanticGroundingType::CONCEPTUAL: return;
        case SemanticGroundingType::RELATIVETIME: {
            auto& relTimeGrd = pGrd.getRelTimeGrounding();
            binarysaver::writeChar(pPtr.pchar++, semanticRelativeTimeType_toChar(relTimeGrd.timeType));
            return;
        }
        case SemanticGroundingType::RELATIVEDURATION: {
            auto& relDurationGrd = pGrd.getRelDurationGrounding();
            binarysaver::writeChar(pPtr.pchar++, semanticRelativeDurationType_toChar(relDurationGrd.durationType));
            return;
        }
        case SemanticGroundingType::RELATIVELOCATION: {
            auto& relLocationGrd = pGrd.getRelLocationGrounding();
            binarysaver::writeChar(pPtr.pchar++, semanticRelativeLocationType_toChar(relLocationGrd.locationType));
            return;
        }
        case SemanticGroundingType::PERCENTAGE: {
            auto& percentageGrd = pGrd.getPercentageGrounding();
            const bool charOrInt = _canSemanticFloatBeWrittenInAChar(percentageGrd.value);
            binarysaver::writeChar_0(pPtr.pchar, charOrInt);
            ++pPtr.pchar;
            _writeSemanticFloat(pPtr, percentageGrd.value, charOrInt);
            return;
        }
        case SemanticGroundingType::UNITY: {
            auto& unityGrd = pGrd.getUnityGrounding();
            binarysaver::writeChar(pPtr.pchar++, typeOfUnity_toChar(unityGrd.typeOfUnity));
            binarysaver::writeChar(pPtr.pchar++, unityGrd.value);
            return;
        }
    }
    assert(false);
}

void _writeChildren(binarymasks::Ptr& pPtr,
                    const std::map<GrammaticalType, UniqueSemanticExpression>& pChildren,
                    const linguistics::LinguisticDatabase& pLingDb,
                    SemExpPtrOffsets* pSemExpPtrOffsetsPtr) {
    unsigned char nbOfChildren = binarysaver::sizet_to_uchar(pChildren.size());
    binarysaver::writeChar(pPtr.pchar++, nbOfChildren);
    for (const auto& currChild : pChildren) {
        binarysaver::writeChar(pPtr.pchar++, grammaticalType_toChar(currChild.first));
        writeSemExp(pPtr, *currChild.second, pLingDb, pSemExpPtrOffsetsPtr);
    }
}

void _writeSemExpOpt(binarymasks::Ptr& pPtr,
                     const mystd::unique_propagate_const<UniqueSemanticExpression>& pSemExpOpt,
                     const linguistics::LinguisticDatabase& pLingDb,
                     SemExpPtrOffsets* pSemExpPtrOffsetsPtr) {
    bool descInitialized = pSemExpOpt.has_value();
    binarysaver::writeBool(pPtr.pchar++, descInitialized);
    if (descInitialized)
        writeSemExp(pPtr, **pSemExpOpt, pLingDb, pSemExpPtrOffsetsPtr);
}

}

SemExpPtrOffsets::SemExpPtrOffsets(const unsigned char* pBeginPtr)
    : _beginPtr(pBeginPtr)
    , _grdExpToOffsetsPtr()
    , _semExpToOffsetsPtr() {}

void SemExpPtrOffsets::addSemExp(const SemanticExpression& pSemExp, unsigned char* pPtr) {
    if (pSemExp.type == SemanticExpressionType::GROUNDED)
        _grdExpToOffsetsPtr.emplace(pSemExp.getGrdExpPtr(), pPtr);
    else
        _semExpToOffsetsPtr.emplace(&pSemExp, pPtr);
}

void SemExpPtrOffsets::clearSemExps() {
    _semExpToOffsetsPtr.clear();
}

uint32_t SemExpPtrOffsets::grdExpToOffset(const GroundedExpression& pGrdExp, unsigned char* pPtr) const {
    auto it = _grdExpToOffsetsPtr.find(&pGrdExp);
    if (it != _grdExpToOffsetsPtr.end())
        return pPtr - it->second;
    assert(false);
    return 0u;
}

uint32_t SemExpPtrOffsets::semExpToOffset(const SemanticExpression& pSemExp, unsigned char* pPtr) const {
    auto it = _semExpToOffsetsPtr.find(&pSemExp);
    if (it != _semExpToOffsetsPtr.end())
        return pPtr - it->second;
    auto itGrdExp = _grdExpToOffsetsPtr.find(pSemExp.getGrdExpPtr());
    if (itGrdExp != _grdExpToOffsetsPtr.end())
        return pPtr - itGrdExp->second;
    assert(false);
    return 0u;
}

uint32_t SemExpPtrOffsets::grdExpToOffsetFromBegin(const GroundedExpression& pGrdExp) const {
    auto it = _grdExpToOffsetsPtr.find(&pGrdExp);
    if (it != _grdExpToOffsetsPtr.end())
        return it->second - _beginPtr;
    assert(false);
    return 0u;
}

void writeSemExp(binarymasks::Ptr& pPtr,
                 const SemanticExpression& pSemExp,
                 const linguistics::LinguisticDatabase& pLingDb,
                 SemExpPtrOffsets* pSemExpPtrOffsetsPtr) {
    if (pSemExpPtrOffsetsPtr != nullptr)
        pSemExpPtrOffsetsPtr->addSemExp(pSemExp, pPtr.puchar);
    binarysaver::writeChar(pPtr.pchar++, semanticExpressionType_toChar(pSemExp.type));
    switch (pSemExp.type) {
        case SemanticExpressionType::GROUNDED: {
            auto& grdExp = pSemExp.getGrdExp();
            auto& grd = grdExp.grounding();
            _writeGrounding(pPtr, grd, pLingDb);
            _writeChildren(pPtr, grdExp.children, pLingDb, pSemExpPtrOffsetsPtr);
            return;
        }
        case SemanticExpressionType::LIST: {
            auto& listExp = pSemExp.getListExp();
            binarysaver::writeChar(pPtr.pchar++, listExpressionType_toChar(listExp.listType));
            unsigned char nbOfElts = binarysaver::sizet_to_uchar(listExp.elts.size());
            binarysaver::writeChar(pPtr.pchar++, nbOfElts);
            for (const auto& currElt : listExp.elts)
                writeSemExp(pPtr, *currElt, pLingDb, pSemExpPtrOffsetsPtr);
            return;
        }
        case SemanticExpressionType::COMMAND: {
            auto& cmdExp = pSemExp.getCmdExp();
            writeSemExp(pPtr, *cmdExp.semExp, pLingDb, pSemExpPtrOffsetsPtr);
            _writeSemExpOpt(pPtr, cmdExp.description, pLingDb, pSemExpPtrOffsetsPtr);
            return;
        }
        case SemanticExpressionType::FEEDBACK: {
            auto& fdkExp = pSemExp.getFdkExp();
            writeSemExp(pPtr, *fdkExp.feedbackExp, pLingDb, pSemExpPtrOffsetsPtr);
            writeSemExp(pPtr, *fdkExp.concernedExp, pLingDb, pSemExpPtrOffsetsPtr);
            return;
        }
        case SemanticExpressionType::METADATA: {
            auto& metadataExp = pSemExp.getMetadataExp();
            binarysaver::writeChar(pPtr.pchar++, semanticSourceEnum_toChar(metadataExp.from));
            binarysaver::writeChar(pPtr.pchar++, contextualAnnotation_toChar(metadataExp.contextualAnnotation));
            binarysaver::writeChar(pPtr.pchar++, semanticLanguageEnum_toChar(metadataExp.fromLanguage));
            binarysaver::writeChar(pPtr.pchar++, metadataExp.confidence);
            binarysaver::writeString(pPtr, metadataExp.fromText);
            unsigned char nbOfReferences = binarysaver::sizet_to_uchar(metadataExp.references.size());
            binarysaver::writeChar(pPtr.pchar++, nbOfReferences);
            for (const auto& currReference : metadataExp.references)
                binarysaver::writeString(pPtr, currReference);
            _writeSemExpOpt(pPtr, metadataExp.source, pLingDb, pSemExpPtrOffsetsPtr);
            writeSemExp(pPtr, *metadataExp.semExp, pLingDb, pSemExpPtrOffsetsPtr);
            return;
        }
        case SemanticExpressionType::ANNOTATED: {
            auto& annExp = pSemExp.getAnnExp();
            binarysaver::writeBool(pPtr.pchar++, annExp.synthesizeAnnotations);
            _writeChildren(pPtr, annExp.annotations, pLingDb, pSemExpPtrOffsetsPtr);
            writeSemExp(pPtr, *annExp.semExp, pLingDb, pSemExpPtrOffsetsPtr);
            return;
        }
        case SemanticExpressionType::CONDITION: {
            auto& condExp = pSemExp.getCondExp();
            binarysaver::writeBool(pPtr.pchar++, condExp.isAlwaysActive);
            binarysaver::writeBool(pPtr.pchar++, condExp.conditionPointsToAffirmations);
            writeSemExp(pPtr, *condExp.conditionExp, pLingDb, pSemExpPtrOffsetsPtr);
            writeSemExp(pPtr, *condExp.thenExp, pLingDb, pSemExpPtrOffsetsPtr);
            _writeSemExpOpt(pPtr, condExp.elseExp, pLingDb, pSemExpPtrOffsetsPtr);
            return;
        }
        case SemanticExpressionType::COMPARISON: {
            auto& compExp = pSemExp.getCompExp();
            binarysaver::writeChar_0To3(pPtr.pchar, ComparisonOperator_toChar(compExp.op));
            binarysaver::writeChar_4To7(pPtr.pchar, semanticVerbTense_toChar(compExp.tense));
            ++pPtr.pchar;
            binarysaver::writeChar(pPtr.pchar++, semanticRequestType_toChar(compExp.request));
            _writeSemExpOpt(pPtr, compExp.whatIsComparedExp, pLingDb, pSemExpPtrOffsetsPtr);
            writeSemExp(pPtr, *compExp.leftOperandExp, pLingDb, pSemExpPtrOffsetsPtr);
            _writeSemExpOpt(pPtr, compExp.rightOperandExp, pLingDb, pSemExpPtrOffsetsPtr);
            return;
        }
        case SemanticExpressionType::SETOFFORMS: {
            auto& sofExp = pSemExp.getSetOfFormsExp();
            unsigned char nbOfPrioToForms = binarysaver::sizet_to_uchar(sofExp.prioToForms.size());
            binarysaver::writeChar(pPtr.pchar++, nbOfPrioToForms);
            for (const auto& currPTF : sofExp.prioToForms) {
                binarysaver::writeInt(pPtr.pint++, currPTF.first);
                unsigned char nbOfQuestionExpForm = binarysaver::sizet_to_uchar(currPTF.second.size());
                binarysaver::writeChar(pPtr.pchar++, nbOfQuestionExpForm);
                for (const auto& currQEF : currPTF.second) {
                    writeSemExp(pPtr, *currQEF->exp, pLingDb, pSemExpPtrOffsetsPtr);
                    binarysaver::writeBool(pPtr.pchar++, currQEF->isOriginalForm);
                }
            }
            return;
        }
        case SemanticExpressionType::FIXEDSYNTHESIS: {
            auto& fsExp = pSemExp.getFSynthExp();
            unsigned char nbOfLangToSynt = binarysaver::sizet_to_uchar(fsExp.langToSynthesis.size());
            binarysaver::writeChar(pPtr.pchar++, nbOfLangToSynt);
            for (const auto& currLangToSynt : fsExp.langToSynthesis) {
                binarysaver::writeChar(pPtr.pchar++, semanticLanguageEnum_toChar(currLangToSynt.first));
                binarysaver::writeString(pPtr, currLangToSynt.second);
            }
            writeSemExp(pPtr, fsExp.getSemExp(), pLingDb, pSemExpPtrOffsetsPtr);
            return;
        }
        case SemanticExpressionType::INTERPRETATION: {
            auto& intExp = pSemExp.getIntExp();
            binarysaver::writeChar(pPtr.pchar++, interpretationFrom_toChar(intExp.source));
            writeSemExp(pPtr, *intExp.interpretedExp, pLingDb, pSemExpPtrOffsetsPtr);
            writeSemExp(pPtr, *intExp.originalExp, pLingDb, pSemExpPtrOffsetsPtr);
            return;
        }
    }
    assert(false);
}

}    // End of namespace semexpsaver
}    // End of namespace onsem
