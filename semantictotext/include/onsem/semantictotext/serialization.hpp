#ifndef ONSEM_SEMANTICTOTEXT_SERIALIZATION_HPP
#define ONSEM_SEMANTICTOTEXT_SERIALIZATION_HPP

#include <memory>
#include <boost/property_tree/ptree.hpp>
#include "api.hpp"

namespace onsem {
namespace linguistics {
struct LinguisticDatabase;
class TranslationDictionary;
}
struct SemanticMemory;
struct UniqueSemanticExpression;
struct SemanticExpression;
namespace serialization {

// Gzip file compression

ONSEMSEMANTICTOTEXT_API
void propertyTreeFromZippedFile(boost::property_tree::ptree& pTree, const std::string& pFilename);

ONSEMSEMANTICTOTEXT_API
void propertyTreeToZipedFile(const boost::property_tree::ptree& pTree,
                             std::string pFilename,
                             const std::string& pExtansion);

// Gzip compression

ONSEMSEMANTICTOTEXT_API
void propertyTreeFromCompressedString(boost::property_tree::ptree& pTree, const std::string& pCompressed);

ONSEMSEMANTICTOTEXT_API
std::string propertyTreeToCompressedString(const boost::property_tree::ptree& pTree);

// Semantic expression

ONSEMSEMANTICTOTEXT_API
UniqueSemanticExpression loadSemExp(const boost::property_tree::ptree& pTree);

ONSEMSEMANTICTOTEXT_API
void saveSemExp(boost::property_tree::ptree& pTree, const SemanticExpression& pSemExp);

// User memory

ONSEMSEMANTICTOTEXT_API
void loadSemMemory(const boost::property_tree::ptree& pTree,
                   SemanticMemory& pSemMemory,
                   const linguistics::LinguisticDatabase& pLingDb);

ONSEMSEMANTICTOTEXT_API
void saveSemMemory(boost::property_tree::ptree& pTree, const SemanticMemory& pSemMemory);

// Linguistic database

ONSEMSEMANTICTOTEXT_API
void loadLingDatabase(const boost::property_tree::ptree& pTree, linguistics::LinguisticDatabase& pLingDb);

ONSEMSEMANTICTOTEXT_API
void saveLingDatabase(boost::property_tree::ptree& pTree, const linguistics::LinguisticDatabase& pLingDb);

}    // End of namespace serialization
}    // End of namespace onsem

#endif    // ONSEM_SEMANTICTOTEXT_SERIALIZATION_HPP
