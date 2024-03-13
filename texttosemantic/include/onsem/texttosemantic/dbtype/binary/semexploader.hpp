#ifndef ONSEM_TEXTTOSEMANTIC_SAVER_SEMEXPLOADER_HPP
#define ONSEM_TEXTTOSEMANTIC_SAVER_SEMEXPLOADER_HPP

#include <memory>
#include <onsem/common/binary/binaryloader.hpp>
#include "../../api.hpp"

namespace onsem {
namespace linguistics {
struct LinguisticDatabase;
}
struct UniqueSemanticExpression;
struct GroundedExpression;

namespace semexploader {

ONSEM_TEXTTOSEMANTIC_API
std::unique_ptr<GroundedExpression> loadGrdExp(const unsigned char*& pPtr,
                                               const linguistics::LinguisticDatabase& pLingDb);

ONSEM_TEXTTOSEMANTIC_API
UniqueSemanticExpression loadSemExp(const unsigned char*& pPtr, const linguistics::LinguisticDatabase& pLingDb);

}    // End of namespace semexploader
}    // End of namespace onsem

#endif    // ONSEM_TEXTTOSEMANTIC_SAVER_SEMEXPLOADER_HPP
