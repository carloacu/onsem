#ifndef ONSEM_TEXTTOSEMANTIC_TYPE_ENUM_CHUNKTYPE_HPP
#define ONSEM_TEXTTOSEMANTIC_TYPE_ENUM_CHUNKTYPE_HPP


namespace onsem
{
namespace linguistics
{

enum class ChunkType
{
  SEPARATOR_CHUNK,
  INTERJECTION_CHUNK,
  NOMINAL_CHUNK,
  PREPOSITIONAL_CHUNK,
  AUX_CHUNK,
  VERB_CHUNK,
  INFINITVE_VERB_CHUNK,
  AND_CHUNK,
  OR_CHUNK,
  THEN_CHUNK
};


inline bool chunkTypeIsAList(ChunkType pChunkType)
{
  return pChunkType == ChunkType::AND_CHUNK || pChunkType == ChunkType::OR_CHUNK ||
      pChunkType == ChunkType::THEN_CHUNK;
}

inline bool chunkTypeIsVerbal(ChunkType pChunkType)
{
  return pChunkType == ChunkType::VERB_CHUNK || pChunkType == ChunkType::INFINITVE_VERB_CHUNK;
}


} // End of namespace linguistics
} // End of namespace onsem


#endif // ONSEM_TEXTTOSEMANTIC_TYPE_ENUM_CHUNKTYPE_HPP
