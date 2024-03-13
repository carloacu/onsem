#ifndef DATABASEGENERATORPORECESSINGGUI_LISTWORDSBYPOS_LISTWORDSBYPOS_HPP
#define DATABASEGENERATORPORECESSINGGUI_LISTWORDSBYPOS_LISTWORDSBYPOS_HPP

#include <functional>
#include <string>
#include <onsem/common/enum/semanticlanguageenum.hpp>

namespace onsem {
class LingdbTree;

void list_words_by_pos(const LingdbTree& pLingDbTree);

void list_words_by_pos_for_a_langauge(const LingdbTree pLingDbTree,
                                      SemanticLanguageEnum pLanguage,
                                      const std::function<void(const std::string&)>& pCallback);

}    // End of namespace onsem

#endif    // DATABASEGENERATORPORECESSINGGUI_LISTWORDSBYPOS_LISTWORDSBYPOS_HPP
