#ifndef SEMANTICGROUNDINGS_H
#define SEMANTICGROUNDINGS_H

#include <map>
#include <string>
#include <vector>
#include <memory>
#include <assert.h>
#include "../../api.hpp"

namespace onsem
{
struct SemanticGenericGrounding;
struct SemanticStatementGrounding;
struct SemanticAgentGrounding;
struct SemanticTimeGrounding;
struct SemanticDurationGrounding;
struct SemanticTextGrounding;
struct SemanticLanguageGrounding;
struct SemanticMetaGrounding;
struct SemanticNameGrounding;
struct SemanticRelativeLocationGrounding;
struct SemanticRelativeTimeGrounding;
struct SemanticRelativeDurationGrounding;
struct SemanticConceptualGrounding;
struct SemanticResourceGrounding;



#define SEMANTIC_GROUNDING_TYPE_TABLE                                  \
  SEMANTIC_GROUNGING_TYPE(AGENT, "agent")                              \
  SEMANTIC_GROUNGING_TYPE(GENERIC, "generic")                          \
  SEMANTIC_GROUNGING_TYPE(STATEMENT, "statement")                      \
  SEMANTIC_GROUNGING_TYPE(TIME, "time")                                \
  SEMANTIC_GROUNGING_TYPE(TEXT, "text")                                \
  SEMANTIC_GROUNGING_TYPE(DURATION, "duration")                        \
  SEMANTIC_GROUNGING_TYPE(LANGUAGE, "language")                        \
  SEMANTIC_GROUNGING_TYPE(RELATIVELOCATION, "relativeLocation")        \
  SEMANTIC_GROUNGING_TYPE(RELATIVETIME, "relativeTime")                \
  SEMANTIC_GROUNGING_TYPE(RELATIVEDURATION, "relativeDuration")        \
  SEMANTIC_GROUNGING_TYPE(RESOURCE, "resource")                        \
  SEMANTIC_GROUNGING_TYPE(META, "meta")                                \
  SEMANTIC_GROUNGING_TYPE(NAME, "name")                                \
  SEMANTIC_GROUNGING_TYPE(CONCEPTUAL, "conceptual")

#define SEMANTIC_GROUNGING_TYPE(a, b) a,
enum class SemanticGroudingType : char // TODO: rename in SemanticGrouNdingType
{
  SEMANTIC_GROUNDING_TYPE_TABLE
};
#undef SEMANTIC_GROUNGING_TYPE


#define SEMANTIC_GROUNGING_TYPE(a, b) b,
static const std::vector<std::string> _SemanticGroudingsType_toStr = {
  SEMANTIC_GROUNDING_TYPE_TABLE
};
#undef SEMANTIC_GROUNGING_TYPE

#define SEMANTIC_GROUNGING_TYPE(a, b) {b, SemanticGroudingType::a},
static std::map<std::string, SemanticGroudingType> _SemanticGroudingsType_fromStr = {
  SEMANTIC_GROUNDING_TYPE_TABLE
};
#undef SEMANTIC_GROUNGING_TYPE

#define SEMANTIC_GROUNGING_TYPE(a, b) SemanticGroudingType::a,
static const std::vector<SemanticGroudingType> semanticGroudingType_allValues = {
  SEMANTIC_GROUNDING_TYPE_TABLE
};
#undef SEMANTIC_GROUNGING_TYPE

#define SEMANTIC_GROUNGING_TYPE(a, b) 1 +
static const std::size_t semanticGroudingType_size =
  SEMANTIC_GROUNDING_TYPE_TABLE
0;
#undef SEMANTIC_GROUNGING_TYPE
#undef SEMANTIC_GROUNDING_TYPE_TABLE


static inline char semanticGroudingsType_toChar(SemanticGroudingType pGroundingType)
{
  return static_cast<char>(pGroundingType);
}

static inline SemanticGroudingType semanticGroudingsType_fromChar(unsigned char pGroundingType)
{
  return static_cast<SemanticGroudingType>(pGroundingType);
}

static inline std::string semanticGroudingsType_toStr(SemanticGroudingType pGroundingType)
{
  return _SemanticGroudingsType_toStr[semanticGroudingsType_toChar(pGroundingType)];
}

static inline SemanticGroudingType semanticGroudingsType_fromStr
(const std::string& pGroundingStr)
{
  auto it = _SemanticGroudingsType_fromStr.find(pGroundingStr);
  if (it != _SemanticGroudingsType_fromStr.end())
  {
    return it->second;
  }
  return SemanticGroudingType::GENERIC;
}

static inline bool semanticGroudingsType_fromStrIfExist
(SemanticGroudingType& pRes,
 const std::string& pGroundingStr)
{
  auto it = _SemanticGroudingsType_fromStr.find(pGroundingStr);
  if (it != _SemanticGroudingsType_fromStr.end())
  {
    pRes = it->second;
    return true;
  }
  return false;
}

static inline bool semanticGroudingsType_isRelativeType(SemanticGroudingType pRes)
{
  return pRes == SemanticGroudingType::RELATIVETIME ||
      pRes == SemanticGroudingType::RELATIVEDURATION ||
      pRes == SemanticGroudingType::RELATIVELOCATION;
}


struct ONSEM_TEXTTOSEMANTIC_API SemanticGrounding
{
public:
  virtual ~SemanticGrounding() {}

  static std::unique_ptr<SemanticGrounding> make(SemanticGroudingType pType);

  const SemanticGroudingType type;

  virtual const SemanticGenericGrounding& getGenericGrounding() const;
  virtual SemanticGenericGrounding& getGenericGrounding();
  virtual const SemanticGenericGrounding* getGenericGroundingPtr() const { return nullptr; }
  virtual SemanticGenericGrounding* getGenericGroundingPtr() { return nullptr; }

  virtual const SemanticStatementGrounding& getStatementGrounding() const;
  virtual SemanticStatementGrounding& getStatementGrounding();
  virtual const SemanticStatementGrounding* getStatementGroundingPtr() const { return nullptr; }
  virtual SemanticStatementGrounding* getStatementGroundingPtr() { return nullptr; }

  virtual const SemanticAgentGrounding& getAgentGrounding() const;
  virtual SemanticAgentGrounding& getAgentGrounding();
  virtual const SemanticAgentGrounding* getAgentGroundingPtr() const { return nullptr; }
  virtual SemanticAgentGrounding* getAgentGroundingPtr() { return nullptr; }

  virtual const SemanticTimeGrounding& getTimeGrounding() const;
  virtual SemanticTimeGrounding& getTimeGrounding();
  virtual const SemanticTimeGrounding* getTimeGroundingPtr() const { return nullptr; }
  virtual SemanticTimeGrounding* getTimeGroundingPtr() { return nullptr; }

  virtual const SemanticDurationGrounding& getDurationGrounding() const;
  virtual SemanticDurationGrounding& getDurationGrounding();
  virtual const SemanticDurationGrounding* getDurationGroundingPtr() const { return nullptr; }
  virtual SemanticDurationGrounding* getDurationGroundingPtr() { return nullptr; }

  virtual const SemanticTextGrounding& getTextGrounding() const;
  virtual SemanticTextGrounding& getTextGrounding();
  virtual const SemanticTextGrounding* getTextGroundingPtr() const { return nullptr; }
  virtual SemanticTextGrounding* getTextGroundingPtr() { return nullptr; }

  virtual const SemanticLanguageGrounding& getLanguageGrounding() const;
  virtual SemanticLanguageGrounding& getLanguageGrounding();
  virtual const SemanticLanguageGrounding* getLanguageGroundingPtr() const { return nullptr; }
  virtual SemanticLanguageGrounding* getLanguageGroundingPtr() { return nullptr; }

  virtual const SemanticRelativeLocationGrounding& getRelLocationGrounding() const;
  virtual SemanticRelativeLocationGrounding& getRelLocationGrounding();
  virtual const SemanticRelativeLocationGrounding* getRelLocationGroundingPtr() const { return nullptr; }
  virtual SemanticRelativeLocationGrounding* getRelLocationGroundingPtr() { return nullptr; }

  virtual const SemanticRelativeTimeGrounding& getRelTimeGrounding() const;
  virtual SemanticRelativeTimeGrounding& getRelTimeGrounding();
  virtual const SemanticRelativeTimeGrounding* getRelTimeGroundingPtr() const { return nullptr; }
  virtual SemanticRelativeTimeGrounding* getRelTimeGroundingPtr() { return nullptr; }

  virtual const SemanticRelativeDurationGrounding& getRelDurationGrounding() const;
  virtual SemanticRelativeDurationGrounding& getRelDurationGrounding();
  virtual const SemanticRelativeDurationGrounding* getRelDurationGroundingPtr() const { return nullptr; }
  virtual SemanticRelativeDurationGrounding* getRelDurationGroundingPtr() { return nullptr; }

  virtual const SemanticResourceGrounding& getResourceGrounding() const;
  virtual SemanticResourceGrounding& getResourceGrounding();
  virtual const SemanticResourceGrounding* getResourceGroundingPtr() const { return nullptr; }
  virtual SemanticResourceGrounding* getResourceGroundingPtr() { return nullptr; }

  virtual const SemanticMetaGrounding& getMetaGrounding() const;
  virtual SemanticMetaGrounding& getMetaGrounding();
  virtual const SemanticMetaGrounding* getMetaGroundingPtr() const { return nullptr; }
  virtual SemanticMetaGrounding* getMetaGroundingPtr() { return nullptr; }

  virtual const SemanticNameGrounding& getNameGrounding() const;
  virtual SemanticNameGrounding& getNameGrounding();
  virtual const SemanticNameGrounding* getNameGroundingPtr() const { return nullptr; }
  virtual SemanticNameGrounding* getNameGroundingPtr() { return nullptr; }

  virtual const SemanticConceptualGrounding& getConceptualGrounding() const;
  virtual SemanticConceptualGrounding& getConceptualGrounding();
  virtual const SemanticConceptualGrounding* getConceptualGroundingPtr() const { return nullptr; }
  virtual SemanticConceptualGrounding* getConceptualGroundingPtr() { return nullptr; }

  bool operator==(const SemanticGrounding& pOther) const;

  bool isEmpty() const;

  bool polarity; // false: sure of the contrary, true: sure of it

  /// The concepts carried by the semantic grounding. The concept name serves as the key, and the
  /// value is a weight of "how much" the concept is involved in the grounding.
  /// Small values (~5) are recommended for the weight currently.
  std::map<std::string, char> concepts;

protected:
  SemanticGrounding(SemanticGroudingType pType)
    : type(pType),
      polarity(true),
      concepts()
  {
  }
  SemanticGrounding(SemanticGroudingType pType,
                    const std::map<std::string, char>& pConcepts)
    : type(pType),
      polarity(true),
      concepts(pConcepts)
  {
  }
  SemanticGrounding(const SemanticGrounding& pOther)
    : type(pOther.type),
      polarity(pOther.polarity),
      concepts(pOther.concepts)
  {
  }

  bool _isMotherClassEqual(const SemanticGrounding& pOther) const;
};


} // End of namespace onsem


#endif // SEMANTICGROUNDINGS_H
