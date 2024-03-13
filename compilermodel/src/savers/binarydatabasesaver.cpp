#include <onsem/compilermodel/savers/binarydatabasesaver.hpp>
#include <cstring>
#include <onsem/compilermodel/lingdbstring.hpp>

namespace onsem {

const int BinaryDatabaseSaver::fFormalism = 3;

binarymasks::Ptr BinaryDatabaseSaver::xWriteString(binarymasks::Ptr pEndMemory, const LingdbString* pString) const {
    if (pString == nullptr) {
        *(pEndMemory.pchar++) = 0;
        return pEndMemory;
    }
    assert(pString->length() < 127);
    *(pEndMemory.pchar++) = pString->length();
    memcpy(pEndMemory.pchar, pString->toCStr(), pString->length());
    pEndMemory.pchar += pString->length();

    return pEndMemory;
}

unsigned char BinaryDatabaseSaver::xMaxNbOfLettersInTheNode(LingdbDynamicTrieNode* pNode) const {
    unsigned char res = 0;
    while (pNode->getNbChildren() == 1 && pNode->nbMeaningsAtThisLemme() == 0 && pNode->nbWordForms() == 0
           && res < 255) {
        pNode = pNode->getFirstChild();
        ++res;
    }
    return res;
}

}    // End of namespace onsem
