#include <onsem/texttosemantic/type/enumsconvertions.hpp>


namespace onsem
{
namespace linguistics
{

mystd::optional<ChunkLinkType> grammaticalTypeToChunkType(GrammaticalType pGramType)
{
  switch (pGramType)
  {
  case GrammaticalType::SUBJECT:
    return ChunkLinkType::SUBJECT;
  case GrammaticalType::RECEIVER:
    return ChunkLinkType::INDIRECTOBJECT;
  case GrammaticalType::OBJECT:
    return ChunkLinkType::DIRECTOBJECT;
  case GrammaticalType::OWNER:
    return ChunkLinkType::OWNER;
  case GrammaticalType::SPECIFIER:
    return ChunkLinkType::SPECIFICATION;
  case GrammaticalType::TIME:
    return ChunkLinkType::TIME;
  case GrammaticalType::LENGTH:
    return ChunkLinkType::LENGTH;
  case GrammaticalType::DURATION:
    return ChunkLinkType::DURATION;
  case GrammaticalType::MANNER:
    return ChunkLinkType::MANNER;
  case GrammaticalType::LOCATION:
    return ChunkLinkType::LOCATION;
  case GrammaticalType::LANGUAGE:
    return ChunkLinkType::LANGUAGE;
  case GrammaticalType::PURPOSE:
    return ChunkLinkType::PURPOSE;
  case GrammaticalType::ACCORDING_TO:
    return ChunkLinkType::ACCORDINGTO;
  case GrammaticalType::AGAINST:
    return ChunkLinkType::AGAINST;
  case GrammaticalType::BETWEEN:
    return ChunkLinkType::BETWEEN;
  case GrammaticalType::STARTING_POINT:
    return ChunkLinkType::STARTING_POINT;
  case GrammaticalType::IN_BACKGROUND:
    return ChunkLinkType::IN_BACKGROUND;
  case GrammaticalType::IN_CASE_OF:
    return ChunkLinkType::IN_CASE_OF;
  case GrammaticalType::CAUSE:
    return ChunkLinkType::CAUSE;
  case GrammaticalType::MITIGATION:
    return ChunkLinkType::MITIGATION;
  case GrammaticalType::OCCURRENCE_RANK:
    return ChunkLinkType::OCCURRENCE_RANK;
  case GrammaticalType::SIMILARITY:
    return ChunkLinkType::SIMILARITY;
  case GrammaticalType::INTERVAL:
    return ChunkLinkType::INTERVAL;
  case GrammaticalType::REPETITION:
    return ChunkLinkType::REPETITION;
  case GrammaticalType::DESPITE_CONTRAINT:
    return ChunkLinkType::DESPITE_CONTRAINT;
  case GrammaticalType::REASONOF:
    return ChunkLinkType::REASONOF;
  case GrammaticalType::THANKS_TO:
    return ChunkLinkType::THANKS_TO;
  case GrammaticalType::TODO:
    return ChunkLinkType::TODO;
  case GrammaticalType::TOPIC:
    return ChunkLinkType::TOPIC;
  case GrammaticalType::UNITY:
    return ChunkLinkType::UNITY;
  case GrammaticalType::WAY:
    return ChunkLinkType::WAY;
  case GrammaticalType::WITH:
    return ChunkLinkType::WITH;
  case GrammaticalType::WITHOUT:
    return ChunkLinkType::WITHOUT;
  case GrammaticalType::SUBORDINATE:
    return ChunkLinkType::SUBORDINATE;
  case GrammaticalType::NOT_UNDERSTOOD:
    return ChunkLinkType::NOTUNDERSTOOD;

  case GrammaticalType::SUB_CONCEPT:
  case GrammaticalType::OTHER_THAN:
  case GrammaticalType::INTRODUCTING_WORD:
  case GrammaticalType::UNKNOWN:
    return mystd::optional<ChunkLinkType>();
  }
  return mystd::optional<ChunkLinkType>();
}


mystd::optional<GrammaticalType> chunkTypeToGrammaticalType
(ChunkLinkType pChunkLinkType)
{
  switch (pChunkLinkType)
  {
  case ChunkLinkType::SUBJECT:
    return GrammaticalType::SUBJECT;
  case ChunkLinkType::INDIRECTOBJECT:
    return GrammaticalType::RECEIVER;
  case ChunkLinkType::DIRECTOBJECT:
  case ChunkLinkType::SUBORDINATE_CLAUSE:
    return GrammaticalType::OBJECT;
  case ChunkLinkType::OWNER:
    return GrammaticalType::OWNER;
  case ChunkLinkType::OBJECT_OF:
  case ChunkLinkType::SPECIFICATION:
    return GrammaticalType::SPECIFIER;
  case ChunkLinkType::TIME:
    return GrammaticalType::TIME;
  case ChunkLinkType::LENGTH:
    return GrammaticalType::LENGTH;
  case ChunkLinkType::DURATION:
    return GrammaticalType::DURATION;
  case ChunkLinkType::MANNER:
    return GrammaticalType::MANNER;
  case ChunkLinkType::LOCATION:
    return GrammaticalType::LOCATION;
  case ChunkLinkType::LANGUAGE:
    return GrammaticalType::LANGUAGE;
  case ChunkLinkType::PURPOSE:
    return GrammaticalType::PURPOSE;
  case ChunkLinkType::ACCORDINGTO:
    return GrammaticalType::ACCORDING_TO;
  case ChunkLinkType::AGAINST:
    return GrammaticalType::AGAINST;
  case ChunkLinkType::BETWEEN:
    return GrammaticalType::BETWEEN;
  case ChunkLinkType::STARTING_POINT:
    return GrammaticalType::STARTING_POINT;
  case ChunkLinkType::IN_BACKGROUND:
    return GrammaticalType::IN_BACKGROUND;
  case ChunkLinkType::IN_CASE_OF:
    return GrammaticalType::IN_CASE_OF;
  case ChunkLinkType::CAUSE:
    return GrammaticalType::CAUSE;
  case ChunkLinkType::MITIGATION:
    return GrammaticalType::MITIGATION;
  case ChunkLinkType::OCCURRENCE_RANK:
    return GrammaticalType::OCCURRENCE_RANK;
  case ChunkLinkType::SIMILARITY:
    return GrammaticalType::SIMILARITY;
  case ChunkLinkType::RECEIVER:
    return GrammaticalType::RECEIVER;
  case ChunkLinkType::INTERVAL:
    return GrammaticalType::INTERVAL;
  case ChunkLinkType::REPETITION:
    return GrammaticalType::REPETITION;
  case ChunkLinkType::DESPITE_CONTRAINT:
    return GrammaticalType::DESPITE_CONTRAINT;
  case ChunkLinkType::REASONOF:
    return GrammaticalType::REASONOF;
  case ChunkLinkType::THANKS_TO:
    return GrammaticalType::THANKS_TO;
  case ChunkLinkType::TODO:
    return GrammaticalType::TODO;
  case ChunkLinkType::TOPIC:
    return GrammaticalType::TOPIC;
  case ChunkLinkType::UNITY:
    return GrammaticalType::UNITY;
  case ChunkLinkType::WAY:
    return GrammaticalType::WAY;
  case ChunkLinkType::WITH:
    return GrammaticalType::WITH;
  case ChunkLinkType::WITHOUT:
    return GrammaticalType::WITHOUT;
  case ChunkLinkType::SUBORDINATE:
    return GrammaticalType::SUBORDINATE;
  case ChunkLinkType::NOTUNDERSTOOD:
    return GrammaticalType::NOT_UNDERSTOOD;

  case ChunkLinkType::COMPARATOR_DIFFERENT:
  case ChunkLinkType::COMPARATOR_EQUAL:
  case ChunkLinkType::COMPARATOR_MORE:
  case ChunkLinkType::COMPARATOR_LESS:
  case ChunkLinkType::RIGHTOPCOMPARISON:
  case ChunkLinkType::SUBJECT_OF:
  case ChunkLinkType::PURPOSE_OF:
  case ChunkLinkType::QUESTIONWORD:
  case ChunkLinkType::AUXILIARY:
  case ChunkLinkType::REFLEXIVE:
  case ChunkLinkType::INTERJECTION:
  case ChunkLinkType::IF:
  case ChunkLinkType::ELSE:
  case ChunkLinkType::COMPLEMENT:
  case ChunkLinkType::SIMPLE:
  case ChunkLinkType::IGNORE:
    return mystd::optional<GrammaticalType>();
  }
  return mystd::optional<GrammaticalType>();
}


mystd::optional<SemanticRequestType> chunkTypeToRequestType(ChunkLinkType pChunkLinkType)
{
  auto gramTypeOpt = chunkTypeToGrammaticalType(pChunkLinkType);
  if (gramTypeOpt)
    return semanticRequestType_fromSemGram(*gramTypeOpt);
  return mystd::optional<SemanticRequestType>();
}

mystd::optional<ChunkLinkType> requestTypeToChunkType(SemanticRequestType pRequestType)
{
  GrammaticalType gramType = semanticRequestType_toSemGram(pRequestType);
  return grammaticalTypeToChunkType(gramType);
}


} // End of namespace linguistics
} // End of namespace onsem
