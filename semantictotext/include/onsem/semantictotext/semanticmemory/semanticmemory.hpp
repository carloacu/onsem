#ifndef ONSEM_SEMANTICTOTEXT_SEMANTICMEMORY_SEMANTICMEMORY_HPP
#define ONSEM_SEMANTICTOTEXT_SEMANTICMEMORY_SEMANTICMEMORY_HPP

#include <onsem/common/utility/unique_propagate_const.hpp>
#include <onsem/common/enum/semanticlanguagetype.hpp>
#include <onsem/texttosemantic/dbtype/misc/parameterswithvalue.hpp>
#include <onsem/semantictotext/api.hpp>
#include "semanticmemorybloc.hpp"

namespace onsem
{
struct ExternalFallback;

struct ONSEMSEMANTICTOTEXT_API MemBlockAndExternalCallback
{
  virtual ~MemBlockAndExternalCallback() {}

  virtual mystd::unique_propagate_const<UniqueSemanticExpression> getAnswer
  (const IndexToSubNameToParameterValue& pParams,
   SemanticLanguageEnum pLanguageType) const = 0;

  virtual std::string idStr() const = 0;

  virtual std::list<UniqueSemanticExpression>& getSemExpThatCanBeAnswered() = 0;
  virtual std::list<UniqueSemanticExpression>& getTriggers() = 0;

  SemanticMemoryBlock memBlock{};
  SemanticMemoryBlock memBlockForTriggers{};
};


struct ONSEMSEMANTICTOTEXT_API ProativeSpecifications
{
  bool informTheUserHowToTeachMe = false;
  bool canLearnANewAxiomaticnAction = false;
};



/**
 * @brief A semantic memory combining two memories with different policy.
 *
 * It contains a semantic memory block for user-provided information, that could
 * complement information stored as in the "system memory". System memory would
 * always be considered as an undeniable truth that the user memory cannot contradict.
 */
struct ONSEMSEMANTICTOTEXT_API SemanticMemory
{
  SemanticMemory();

  SemanticMemory(SemanticMemory&& pOther) = delete;
  SemanticMemory& operator=(SemanticMemory&& pOther) = delete;

  SemanticMemory(const SemanticMemory&) = delete;
  SemanticMemory& operator=(const SemanticMemory&) = delete;

  // Modify the memory
  void addNewUserFocusedToSemExp(UniqueSemanticExpression pSemExp,
                                 const std::string& pUserId);
  const std::map<std::string, std::list<UniqueSemanticExpression>>&
  getNewUserFocusedToSemExps() const {return _newUserFocusedToSemExps; }

  void clearLocalInformationButNotTheSubBloc();
  void clear();
  void copySemExps(std::list<UniqueSemanticExpression>& pCopiedSemExps);
  void setCurrUserId(const std::string& pNewUserId);

  void extractSemExpsForASpecificUser(std::list<UniqueSemanticExpression>& pSemExps,
                                      const std::string& pUserId);

  void registerExternalFallback(std::unique_ptr<ExternalFallback> pExtFallback);

  const ExternalFallback* getExternalFallback() const { return  _externalFallback ? &*_externalFallback : nullptr; }

  void registerExternalInfosProvider(std::unique_ptr<MemBlockAndExternalCallback> pExtCallbackPtr,
                                     const linguistics::LinguisticDatabase& pLingDb);
  void unregisterExternalInfosProvider(const std::string& pIdStr);

  std::string getCurrUserId() const;


  // memory informations
  SemanticMemoryBlock memBloc;
  SemanticLanguageEnum defaultLanguage;
  std::list<mystd::unique_propagate_const<MemBlockAndExternalCallback>> callbackToSentencesCanBeAnswered;
  ProativeSpecifications proativeSpecifications;

private:
  // things to do when a specific user comes
  std::map<std::string, std::list<UniqueSemanticExpression>> _newUserFocusedToSemExps;
  std::string _currUserId;
  std::unique_ptr<ExternalFallback> _externalFallback;
};




struct ONSEMSEMANTICTOTEXT_API ExternalFallback
{
  virtual ~ExternalFallback();

  virtual void addFallback(UniqueSemanticExpression& pSemExp,
                           const std::string& pUserId,
                           const GroundedExpression& pOriginalGrdExp) const = 0;
};



} // End of namespace onsem


#endif // ONSEM_SEMANTICTOTEXT_SEMANTICMEMORY_SEMANTICMEMORY_HPP
