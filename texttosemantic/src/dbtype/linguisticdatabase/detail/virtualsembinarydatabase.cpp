#include <onsem/texttosemantic/dbtype/linguisticdatabase/detail/virtualsembinarydatabase.hpp>

namespace onsem {
const int VirtualSemBinaryDatabase::fFormalism = 3;

VirtualSemBinaryDatabase::VirtualSemBinaryDatabase()
    : fTotalSize(0)
    , fErrorMessage("NOT_LOADED") {}

std::size_t VirtualSemBinaryDatabase::getSize(std::string& pErrorStr, bool& pIsLoaded) const {
    pErrorStr = fErrorMessage;
    pIsLoaded = xIsLoaded();
    return fTotalSize;
}

}    // End of namespace onsem
