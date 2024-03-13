#include <onsem/semantictotext/semanticmemory/links/groundedexpwithlinkslist.hpp>

namespace onsem {

void GroundedExpWithLinksList::setEnabled(bool pEnabled) {
    for (GroundedExpWithLinks& currElt : elts)
        currElt.setEnabled(pEnabled);
}

void GroundedExpWithLinksList::clear() {
    auto itElt = elts.begin();
    while (itElt != elts.end()) {
        GroundedExpWithLinks& memSen = *itElt;
        memSen.setEnabled(false);
        itElt = elts.erase(itElt);
    }
}

std::string GroundedExpWithLinksList::getName(const std::string& pUserId) const {
    if (elts.size() == 1)
        return elts.front().getName(pUserId);
    if (and_or) {
        for (const auto& currElt : elts) {
            const std::string name = currElt.getName(pUserId);
            if (!name.empty())
                return name;
        }
    }
    return "";
}

bool GroundedExpWithLinksList::hasEquivalentUserIds(const std::string& pUserId) const {
    if (and_or)
        for (const auto& currElt : elts)
            if (currElt.hasEquivalentUserIds(pUserId))
                return true;
    return false;
}

intSemId GroundedExpWithLinksList::getIdOfFirstSentence() const {
    for (const auto& currElt : elts)
        return currElt.id;
    return 0;
}

}    // End of namespace onsem
