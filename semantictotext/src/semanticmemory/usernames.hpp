#ifndef ONSEM_SEMANTICTOTEXT_SRC_SEMANTICMEMORY_USERNAMES_HPP
#define ONSEM_SEMANTICTOTEXT_SRC_SEMANTICMEMORY_USERNAMES_HPP

#include <vector>
#include <string>

namespace onsem {

struct UserNames {
    std::string getName() const;
    bool operator<(const UserNames& pOther) const;
    bool operator==(const UserNames& pOther) const;
    bool operator!=(const UserNames& pOther) const { return !operator==(pOther); }

    std::vector<std::string> names{};
};

}    // End of namespace onsem

#endif    // ONSEM_SEMANTICTOTEXT_SRC_SEMANTICMEMORY_USERNAMES_HPP
