#include <onsem/texttosemantic/dbtype/linguisticdatabase/statictranslationdictionary.hpp>
#include <onsem/common/binary/binaryloader.hpp>
#include <onsem/texttosemantic/dbtype/semanticgrounding/semanticgenericgrounding.hpp>
#include <onsem/texttosemantic/dbtype/inflectedword.hpp>

namespace onsem {

StaticTranslationDictionary::StaticTranslationDictionary(linguistics::LinguisticDatabaseStreams& pIStreams,
                                                         SemanticLanguageEnum pInLangEnum,
                                                         SemanticLanguageEnum pOutLangEnum)
    : MetaWordTreeDb()
    , fTransId(pInLangEnum, pOutLangEnum) {
    xLoad(pIStreams);
}

StaticTranslationDictionary::~StaticTranslationDictionary() {
    xUnload();
}

void StaticTranslationDictionary::xUnload() {
    if (_ptrPatriciaTrie != nullptr) {
        binaryloader::deallocMemZone(&_ptrPatriciaTrie);
        _ptrPatriciaTrie = nullptr;
    }
    fTotalSize = 0;
    fErrorMessage = "NOT_LOADED";
}

void StaticTranslationDictionary::xLoad(linguistics::LinguisticDatabaseStreams& pIStreams) {
    if (xIsLoaded())
        return;
    auto* streamPtr = pIStreams.languageToStreams[fTransId.inLangEnum].translationStreams[fTransId.outLangEnum];

    if (streamPtr == nullptr) {
        _loadedWithoutStream = true;
        return;
    }
    auto& stream = *streamPtr;
    DatabaseHeader header;
    stream.read(header.charValues, sizeof(DatabaseHeader));
    if (header.intValues[0] != fFormalism) {
        fErrorMessage = "BAD_FORMALISM";
        return;
    }
    fTotalSize = static_cast<std::size_t>(header.intValues[1]);

    if (!binaryloader::allocMemZone(&_ptrPatriciaTrie, stream, fTotalSize)) {
        xUnload();
        fErrorMessage = "BAD_ALLOC";
        return;
    }

    // Close database file
    fErrorMessage = "";
}

int StaticTranslationDictionary::getTranslation(const std::string& pLemma,
                                                const StaticLinguisticMeaning& pInMeaning) const {
    if (!xIsLoaded())
        return LinguisticMeaning_noMeaningId;
    assert(pInMeaning.language == fTransId.inLangEnum);

    const auto* node = xGetNode(pLemma, 0, pLemma.size(), false);
    if (node == nullptr) {
        return LinguisticMeaning_noMeaningId;
    }
    unsigned char nbMeanings = xNbMeanings(node);
    const int* transElt = xGetTranslationList(node);
    for (unsigned char i = 0; i < nbMeanings; ++i) {
        if (binaryloader::alignedDecToInt(*transElt) == pInMeaning.meaningId) {
            return binaryloader::alignedDecToInt(*(++transElt));
        }
        transElt += 2;
    }
    return LinguisticMeaning_noMeaningId;
}

const int* StaticTranslationDictionary::xGetTranslationList(const signed char* pNode) const {
    return xGetBeginOfEndingStruct(pNode);
}

}    // End of namespace onsem
