#include "listwordsbypos.hpp"
#include <fstream>
#include <sstream>
#include <onsem/common/enum/semanticlanguageenum.hpp>
#include <onsem/compilermodel/lingdbtree.hpp>
#include <onsem/compilermodel/linguisticintermediarydatabase.hpp>
#include <onsem/compilermodel/lingdbdynamictrienode.hpp>

namespace onsem {

namespace {
void _list_words_by_pos_for_a_langauge_and_write_in_file(const LingdbTree pLingDbTree, SemanticLanguageEnum pLanguage) {
    LinguisticIntermediaryDatabase lingDb;
    lingDb.setLanguage(semanticLanguageEnum_toLegacyStr(pLanguage));
    lingDb.load(pLingDbTree.getDynamicDatabasesFolder() + "/" + lingDb.getLanguage()->toStr() + "."
                + pLingDbTree.getExtDynDatabase());

    auto languageStr = semanticLanguageEnum_toStr(pLanguage);
    std::ofstream outFile("nouns_" + languageStr + ".txt");
    list_words_by_pos_for_a_langauge(
        pLingDbTree, pLanguage, [&](const std::string& pWord) { outFile << pWord << "\n"; });
    outFile.close();
}
}

void list_words_by_pos(const LingdbTree& pLingDbTree) {
    _list_words_by_pos_for_a_langauge_and_write_in_file(pLingDbTree, SemanticLanguageEnum::FRENCH);
    _list_words_by_pos_for_a_langauge_and_write_in_file(pLingDbTree, SemanticLanguageEnum::ENGLISH);
}

void list_words_by_pos_for_a_langauge(const LingdbTree pLingDbTree,
                                      SemanticLanguageEnum pLanguage,
                                      const std::function<void(const std::string&)>& pCallback) {
    LinguisticIntermediaryDatabase lingDb;
    lingDb.setLanguage(semanticLanguageEnum_toLegacyStr(pLanguage));
    lingDb.load(pLingDbTree.getDynamicDatabasesFolder() + "/" + lingDb.getLanguage()->toStr() + "."
                + pLingDbTree.getExtDynDatabase());

    LingdbMeaning* meaning = lingDb.getFPAlloc().first<LingdbMeaning>();
    while (meaning != nullptr) {
        PartOfSpeech partOfSpeech = meaning->getPartOfSpeech();
        if (partOfSpeech == PartOfSpeech::NOUN)
            pCallback(meaning->getLemma()->getWord(true));
        meaning = lingDb.getFPAlloc().next<LingdbMeaning>(meaning);
    }
}

}    // End of namespace onsem
