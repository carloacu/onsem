#ifndef ONSEM_TEXTTOSEMANTIC_SAVER_SEMEXPSAVER_HPP
#define ONSEM_TEXTTOSEMANTIC_SAVER_SEMEXPSAVER_HPP

#include <map>
#include <onsem/common/binary/binarymasks.hpp>
#include "../../api.hpp"

namespace onsem
{
namespace linguistics
{
struct LinguisticDatabase;
}
struct GroundedExpression;
struct SemanticExpression;


namespace semexpsaver
{

class ONSEM_TEXTTOSEMANTIC_API SemExpPtrOffsets
{
public:
  SemExpPtrOffsets(const unsigned char* pBeginPtr);
  void addSemExp(const SemanticExpression& pSemExp,
                 unsigned char* pPtr);
  void clearSemExps();
  uint32_t grdExpToOffset(const GroundedExpression& pGrdExp,
                          unsigned char* pPtr) const;
  uint32_t semExpToOffset(const SemanticExpression& pSemExp,
                          unsigned char* pPtr) const;
  uint32_t grdExpToOffsetFromBegin(const GroundedExpression& pGrdExp) const;

private:
  const unsigned char* _beginPtr;
  std::map<const GroundedExpression*, unsigned char*> _grdExpToOffsetsPtr;
  std::map<const SemanticExpression*, unsigned char*> _semExpToOffsetsPtr;
};


ONSEM_TEXTTOSEMANTIC_API
void writeSemExp(binarymasks::Ptr& pPtr,
                 const SemanticExpression& pSemExp,
                 const linguistics::LinguisticDatabase& pLingDb,
                 SemExpPtrOffsets* pSemExpPtrOffsetsPtr);


} // End of namespace semexpsaver
} // End of namespace onsem


#endif // ONSEM_TEXTTOSEMANTIC_SAVER_SEMEXPSAVER_HPP
