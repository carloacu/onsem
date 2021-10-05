#include <onsem/semantictotext/serialization.hpp>
#include <zlib.h>
#include <sstream>
#include <iostream>
#include <boost/property_tree/info_parser.hpp>
#include <onsem/texttosemantic/dbtype/semanticexpression/semanticexpression.hpp>
#include "io/loadmodel.hpp"
#include "io/savemodel.hpp"

namespace onsem
{
namespace serialization
{

namespace
{

std::string _compress(const std::string& str,
                      int compressionlevel = Z_BEST_COMPRESSION)
{
    z_stream zs;                        // z_stream is zlib's control structure
    memset(&zs, 0, sizeof(zs));

    if (deflateInit(&zs, compressionlevel) != Z_OK)
        throw(std::runtime_error("deflateInit failed while compressing."));

    zs.next_in = (Bytef*)str.data();
    if (str.size() > static_cast<std::size_t>(std::numeric_limits<int>::max()))
      std::cerr << "Error size of string to compress is too big!" << std::endl;
    zs.avail_in = static_cast<unsigned int>(str.size());           // set the z_stream's input

    int ret;
    char outbuffer[32768];
    std::string outstring;

    // retrieve the compressed bytes blockwise
    do {
        zs.next_out = reinterpret_cast<Bytef*>(outbuffer);
        zs.avail_out = sizeof(outbuffer);

        ret = deflate(&zs, Z_FINISH);

        if (outstring.size() < zs.total_out) {
            // append the block to the output string
            outstring.append(outbuffer,
                             zs.total_out - outstring.size());
        }
    } while (ret == Z_OK);

    deflateEnd(&zs);

    if (ret != Z_STREAM_END) {          // an error occurred that was not EOF
        std::ostringstream oss;
        oss << "Exception during zlib compression: (" << ret << ") " << zs.msg;
        throw(std::runtime_error(oss.str()));
    }

    return outstring;
}


void _decompress(std::stringstream& pDecompressed,
                 const std::string& data)
{
  z_stream zs;                        // z_stream is zlib's control structure
  memset(&zs, 0, sizeof(zs));

  if (inflateInit(&zs) != Z_OK)
      throw(std::runtime_error("inflateInit failed while decompressing."));

  zs.next_in = (Bytef*)data.data();
  if (data.size() > static_cast<std::size_t>(std::numeric_limits<int>::max()))
    std::cerr << "Error size of string to decompress is too big!" << std::endl;
  zs.avail_in = static_cast<unsigned int>(data.size());

  int ret;
  char outbuffer[32768];
  std::string outstring;

  // get the decompressed bytes blockwise using repeated calls to inflate
  do {
      zs.next_out = reinterpret_cast<Bytef*>(outbuffer);
      zs.avail_out = sizeof(outbuffer);

      ret = inflate(&zs, 0);

      if (outstring.size() < zs.total_out) {
          outstring.append(outbuffer,
                           zs.total_out - outstring.size());
      }

  } while (ret == Z_OK);

  inflateEnd(&zs);

  if (ret != Z_STREAM_END) {          // an error occurred that was not EOF
      std::ostringstream oss;
      oss << "Exception during zlib decompression: (" << ret << ") "
          << zs.msg;
      throw(std::runtime_error(oss.str()));
  }
  pDecompressed << outstring;
}

}


// Gzip file compression

void propertyTreeFromZippedFile(boost::property_tree::ptree& pTree,
                                const std::string& pFilename)
{
  gzFile fi = gzopen(pFilename.c_str(),"rb");
  if (!fi)
    throw std::runtime_error(gzerror(fi, nullptr));
  char* endOfFile;

  static const int readSize = 1024 * 1024;
  char* buf = new char[readSize];
  std::stringstream ss;
  do
  {
    endOfFile = gzgets(fi, buf, readSize);
    if (endOfFile != nullptr)
      ss << buf;
  }
  while (endOfFile != nullptr);

  gzclose(fi);
  delete[] buf;
  boost::property_tree::read_info(ss, pTree);
}


void propertyTreeToZipedFile(const boost::property_tree::ptree& pTree,
                             std::string pFilename,
                             const std::string& pExtansion)
{
  std::size_t pos = pFilename.find_last_of('.');
  if (pos == std::string::npos ||
      pFilename.compare(pos, pExtansion.size(), pExtansion) != 0)
  {
    pFilename += pExtansion;
  }

  std::stringstream ss;
  boost::property_tree::info_parser::write_info(ss, pTree);

  gzFile fi = gzopen(pFilename.c_str(),"wb");
  std::string buf = ss.str();
  gzwrite(fi, buf.c_str(), static_cast<unsigned int>(buf.size()));
  gzclose(fi);
}



// Gzip compression

void propertyTreeFromCompressedString(boost::property_tree::ptree& pTree,
                                      const std::string& pCompressed)
{
  std::stringstream ss;
  _decompress(ss, pCompressed);
  boost::property_tree::read_info(ss, pTree);
}

std::string propertyTreeToCompressedString(const boost::property_tree::ptree& pTree)
{
  std::stringstream ss;
  boost::property_tree::info_parser::write_info(ss, pTree);
  return _compress(ss.str());
}



// Semantic expression

UniqueSemanticExpression loadSemExp(const boost::property_tree::ptree& pTree)
{
  return serializationprivate::loadSemExp(pTree);
}


void saveSemExp(boost::property_tree::ptree& pTree,
                const SemanticExpression& pSemExp)
{
  serializationprivate::saveSemExp(pTree, pSemExp);
}


// User memory

void loadSemMemory(const boost::property_tree::ptree& pTree,
                   SemanticMemory& pSemMemory,
                   const linguistics::LinguisticDatabase& pLingDb)
{
  serializationprivate::loadSemMemory(pTree, pSemMemory, pLingDb);
}

void saveSemMemory(boost::property_tree::ptree& pTree,
                   const SemanticMemory& pSemMemory)
{
  serializationprivate::saveSemMemory(pTree, pSemMemory);
}



// Linguistic database

void loadLingDatabase(const boost::property_tree::ptree& pTree,
                      linguistics::LinguisticDatabase& pLingDb)
{
  serializationprivate::loadLingDatabase(pTree, pLingDb);
}

void saveLingDatabase(boost::property_tree::ptree& pTree,
                      const linguistics::LinguisticDatabase& pLingDb)
{
  serializationprivate::saveLingDatabase(pTree, pLingDb);
}


} // End of namespace serialization
} // End of namespace onsem
