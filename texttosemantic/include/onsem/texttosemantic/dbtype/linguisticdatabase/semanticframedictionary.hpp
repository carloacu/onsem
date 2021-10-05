#ifndef ONSEM_TEXTTOSEMANTIC_SRC_TOOL_SEMANTICFRAMEDICTIONARY_HPP
#define ONSEM_TEXTTOSEMANTIC_SRC_TOOL_SEMANTICFRAMEDICTIONARY_HPP

#include <list>
#include <map>
#include <onsem/common/enum/chunklinktype.hpp>
#include <onsem/common/enum/semanticgendertype.hpp>
#include <onsem/common/enum/semanticnumbertype.hpp>
#include <onsem/texttosemantic/dbtype/semanticword.hpp>
#include <onsem/texttosemantic/dbtype/inflectedword.hpp>
#include <onsem/texttosemantic/dbtype/linguistic/lingtypetoken.hpp>


namespace onsem
{
struct SemanticGrounding;
struct GroundedExpression;
struct WordToSynthesize;
namespace linguistics
{
struct ChildSpecification;
struct ChildSpecificationsContainer;


class ONSEM_TEXTTOSEMANTIC_API SemanticFrameDictionary
{
public:
  SemanticFrameDictionary();
  ~SemanticFrameDictionary();

  int& getLastTemplatePos();
  std::map<std::string, int>& getBookmarkToTemplatePos();
  void addTemplateNameToChildSpecifications(const std::string& pTemplateName,
                                            ChildSpecification&& pChildSpec);
  void addWordToChildSpecifications(const SemanticWord& pWord,
                                    ChildSpecification&& pChildSpec);
  void addWordToTemplate(const SemanticWord& pWord,
                         const std::string& pTemplateName);
  void addAChildSpecificationsByDefault(ChildSpecification&& pChildSepcs);
  void addAChildSpecificationsWithoutVerbByDefault(ChildSpecification&& pChildSepcs);

  mystd::optional<ChunkLinkType> getChunkLinkFromContext(InflectedWord* pInflectedWordPtr,
                                                         bool& pWillBeAbleToSynthesizeIt,
                                                         const InflectedWord* pPrepInflWordPtr,
                                                         const ConstTokenIterator* pNextToken) const;

  bool getIntroWord(mystd::optional<SemanticWord>& pIntroWord,
                    ChunkLinkType pChunkLinkType,
                    const std::map<std::string, char>& pVerbConcepts,
                    const InflectedWord& pInflectedWord,
                    const GroundedExpression* pObjectGrdExpPtr,
                    const std::list<WordToSynthesize>* pOut,
                    SemanticNumberType pNumber,
                    SemanticGenderType pGender,
                    const InflectedWord& pInflWord) const;
  bool getIntroWordWithoutVerb(mystd::optional<SemanticWord>& pIntroWord,
                               ChunkLinkType pChunkLinkType,
                               const GroundedExpression* pObjectGrdExpPtr,
                               const std::list<WordToSynthesize>* pOut,
                               SemanticNumberType pNumber,
                               SemanticGenderType pGender,
                               const linguistics::InflectedWord& pInflWord) const;
  bool getIntroWordWithoutConditions(mystd::optional<SemanticWord>& pIntroWord,
                                     ChunkLinkType pChunkLinkType) const;

  bool doesIntroductionWordHasChunkLinkType(const SemanticWord& pIntroductionWord,
                                            ChunkLinkType pChunkLinkType) const;
  mystd::optional<ChunkLinkType> introductionWordToChunkLinkType(const SemanticWord& pIntroductionWord) const;


private:
  int _lastTemplatePos;
  std::map<std::string, int> _bookmarkToTemplatePos;

  std::unique_ptr<ChildSpecificationsContainer> _childSpecificationsByDefault;
  std::unique_ptr<ChildSpecificationsContainer> _childSpecificationsWithoutVerbByDefault;
  std::map<std::string, std::shared_ptr<ChildSpecificationsContainer>> _templateNameToChildSpecifications;
  std::map<SemanticWord, std::shared_ptr<ChildSpecificationsContainer>> _wordToChildSpecifications;
};

} // End of namespace linguistics
} // End of namespace onsem


#endif // ONSEM_TEXTTOSEMANTIC_SRC_TOOL_SEMANTICFRAMEDICTIONARY_HPP
