#include <onsem/common/keytostreams.hpp>

namespace onsem {
namespace linguistics {

void KeyToStreams::clear() {
    mainDicToStream = nullptr;
    synthesizerToStream = nullptr;
    translationStreams.clear();
    conversionsStreams.clear();
}

void LinguisticDatabaseStreams::clear() {
    concepts = nullptr;
    languageToStreams.clear();
    dynamicContentStreams.clear();
}

void KeyToFStreams::addMainDicFile(SemanticLanguageEnum pLanguage, const std::string& pFilename) {
    linguisticDatabaseStreams.languageToStreams[pLanguage].mainDicToStream =
        &_addKeyToIfStreamFile(pLanguage, pFilename);
}

void KeyToFStreams::addSynthesizerFile(SemanticLanguageEnum pLanguage, const std::string& pFilename) {
    linguisticDatabaseStreams.languageToStreams[pLanguage].synthesizerToStream =
        &_addKeyToIfStreamFile(pLanguage, pFilename);
}

std::ifstream& KeyToFStreams::_addKeyToIfStreamFile(SemanticLanguageEnum pLanguage, const std::string& pFilename) {
    keyToIfStream.emplace_back(std::ifstream(pFilename, std::ifstream::binary));
    auto& fstream = keyToIfStream.back();
    if (!fstream.is_open())
        throw std::runtime_error("File not found: \"" + pFilename + "\"");
    return fstream;
}

void KeyToFStreams::addFile(SemanticLanguageEnum pInLanguage,
                            SemanticLanguageEnum pOutLanguage,
                            const std::string& pFilename) {
    keyToIfStream.emplace_back(std::ifstream(pFilename, std::ifstream::binary));
    auto& fstream = keyToIfStream.back();
    if (!fstream.is_open())
        throw std::runtime_error("File not found: \"" + pFilename + "\"");
    linguisticDatabaseStreams.languageToStreams[pInLanguage].translationStreams[pOutLanguage] = &fstream;
}

void KeyToFStreams::addConceptFile(const std::string& pFilename) {
    keyToIfStream.emplace_back(std::ifstream(pFilename, std::ifstream::binary));
    auto& fstream = keyToIfStream.back();
    if (!fstream.is_open())
        throw std::runtime_error("File not found: \"" + pFilename + "\"");
    linguisticDatabaseStreams.concepts = &fstream;
}

void KeyToFStreams::addDynamicContentFile(const std::string& pFilename) {
    keyToIfStream.emplace_back(std::ifstream(pFilename, std::ifstream::binary));
    auto& fstream = keyToIfStream.back();
    if (!fstream.is_open())
        throw std::runtime_error("File not found: \"" + pFilename + "\"");
    linguisticDatabaseStreams.dynamicContentStreams.emplace_back(&fstream);
}

void KeyToFStreams::addConversionFile(SemanticLanguageEnum pLanguage,
                                      const std::string& pLocalPath,
                                      const std::string& pFilename) {
    keyToIfStream.emplace_back(std::ifstream(pFilename, std::ifstream::binary));
    auto& fstream = keyToIfStream.back();
    if (!fstream.is_open())
        throw std::runtime_error("File not found: \"" + pFilename + "\"");
    linguisticDatabaseStreams.languageToStreams[pLanguage].conversionsStreams.emplace(pLocalPath, &fstream);
}

void KeyToFStreams::close() {
    for (auto& currElt : keyToIfStream)
        currElt.close();
    linguisticDatabaseStreams.clear();
}

}    // End of namespace linguistics
}    // End of namespace onsem
