#ifndef ONSEM_TEXTTOSEMANTIC_DETAIL_VIRTUALSEMBINARYDATABASE_HPP
#define ONSEM_TEXTTOSEMANTIC_DETAIL_VIRTUALSEMBINARYDATABASE_HPP

#include <string>
#include "../../../api.hpp"

namespace onsem {

class ONSEM_TEXTTOSEMANTIC_API VirtualSemBinaryDatabase {
public:
    virtual ~VirtualSemBinaryDatabase() {}

    std::size_t getSize(std::string& pErrorStr, bool& pIsLoaded) const;

protected:
    std::size_t fTotalSize;
    std::string fErrorMessage;
    /// Formalism that the database has to have to be compatible with the code.
    static const int fFormalism;

    VirtualSemBinaryDatabase();

    virtual bool xIsLoaded() const = 0;
};

}    // End of namespace onsem

#endif    // ONSEM_TEXTTOSEMANTIC_DETAIL_VIRTUALSEMBINARYDATABASE_HPP
