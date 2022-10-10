#ifndef ONSEM_TEXTTOSEMANTIC_TOOL_SEMEXPGENERATOR_HPP
#define ONSEM_TEXTTOSEMANTIC_TOOL_SEMEXPGENERATOR_HPP

#include <memory>
#include <string>
#include <list>
#include <map>
#include <onsem/common/utility/optional.hpp>
#include <onsem/common/enum/semanticentitytype.hpp>
#include <onsem/texttosemantic/dbtype/misc/coreference.hpp>


namespace onsem
{
struct SemanticNameGrounding;
struct UniqueSemanticExpression;


namespace SemExpGenerator
{


std::unique_ptr<SemanticNameGrounding> makeNameGrd(const std::list<std::string>& pNames,
                                                   const std::map<std::string, char>* pConceptsPtr = nullptr);

UniqueSemanticExpression makeCoreferenceExpression(CoreferenceDirectionEnum pDirection,
                                                   const mystd::optional<SemanticEntityType>& pEntityType = mystd::optional<SemanticEntityType>());

UniqueSemanticExpression makeHumanCoreferenceBefore();

UniqueSemanticExpression emptyStatementSemExp();


UniqueSemanticExpression whatIs(UniqueSemanticExpression pSubjectSemExp);


UniqueSemanticExpression whatAbout(UniqueSemanticExpression pSubjectSemExp);


} // End of namespace SemExpGenerator

} // End of namespace onsem


#endif // ONSEM_TEXTTOSEMANTIC_TOOL_SEMEXPGENERATOR_HPP
