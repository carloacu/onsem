#ifndef ONSEM_TEXTTOSEMANTIC_TYPES_MISC_PARAMETERWITHVALUE_HPP
#define ONSEM_TEXTTOSEMANTIC_TYPES_MISC_PARAMETERWITHVALUE_HPP

#include <map>
#include <string>
#include <memory>

namespace onsem
{
struct SemanticExpressionContainer;


// ex: for the method
//     fn method1(str s) with s = "chocolate"
//     it will be
//     0 -> "" -> "chocolate"
// ex: for the method
//     fn method2(int i, str s) with s = "chocolate"
//     it will be
//     1 -> "" -> "chocolate"
// ex: for the method
//     fn method3(int i, MyStrcut ms) with ms.toto = "yes"
//     it will be
//     1 -> "toto" -> "yes"
using IndexToSubNameToParameterValue = std::map<int, std::map<std::string, std::unique_ptr<SemanticExpressionContainer>>>;


} // End of namespace onsem


#endif // ONSEM_TEXTTOSEMANTIC_TYPES_MISC_PARAMETERWITHVALUE_HPP
