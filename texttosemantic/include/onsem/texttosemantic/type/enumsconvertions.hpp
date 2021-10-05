#ifndef ONSEM_TEXTTOSEMANTIC_TYPE_ENUMSCONVERTIONS_HPP
#define ONSEM_TEXTTOSEMANTIC_TYPE_ENUMSCONVERTIONS_HPP

#include "../api.hpp"
#include <onsem/common/enum/grammaticaltype.hpp>
#include <onsem/common/enum/semanticrequesttype.hpp>
#include <onsem/common/enum/chunklinktype.hpp>
#include <onsem/common/utility/optional.hpp>

namespace onsem
{
namespace linguistics
{

ONSEM_TEXTTOSEMANTIC_API
mystd::optional<ChunkLinkType> grammaticalTypeToChunkType(GrammaticalType pGramType);

ONSEM_TEXTTOSEMANTIC_API
mystd::optional<GrammaticalType> chunkTypeToGrammaticalType(ChunkLinkType pChunkLinkType);

ONSEM_TEXTTOSEMANTIC_API
mystd::optional<SemanticRequestType> chunkTypeToRequestType(ChunkLinkType pChunkLinkType);

ONSEM_TEXTTOSEMANTIC_API
mystd::optional<ChunkLinkType> requestTypeToChunkType(SemanticRequestType pRequestType);

} // End of namespace linguistics
} // End of namespace onsem

#endif // ONSEM_TEXTTOSEMANTIC_TYPE_ENUMSCONVERTIONS_HPP
