#include <gtest/gtest.h>
#include <onsem/texttosemantic/dbtype/inflection/inflections.hpp>

using namespace onsem;
static const std::string nothingStr = "##NOTHING##";

namespace
{
void _checkInflections(InflectionType pType,
                       const std::string& pInflectionalCodes,
                       const std::string& pExpectedOut = nothingStr)
{
  auto infls = Inflections::create(pType, pInflectionalCodes);
  ASSERT_EQ(pType, infls->type);
  std::stringstream ss;
  infls->concisePrint(ss);
  if (pExpectedOut == nothingStr)
    ASSERT_EQ(pInflectionalCodes, ss.str());
  else
  {
    ASSERT_EQ(pExpectedOut, ss.str());
    _checkInflections(pType, pExpectedOut);
  }
}
}


TEST(SemanticBase, check_inflections)
{
  _checkInflections(InflectionType::ADJECTIVAL, "");
  _checkInflections(InflectionType::ADJECTIVAL, "m");
  _checkInflections(InflectionType::ADJECTIVAL, "s");
  _checkInflections(InflectionType::ADJECTIVAL, ",m");
  _checkInflections(InflectionType::ADJECTIVAL, "ms");
  _checkInflections(InflectionType::ADJECTIVAL, "sm", "ms");
  _checkInflections(InflectionType::ADJECTIVAL, "C");
  _checkInflections(InflectionType::ADJECTIVAL, "S");
  _checkInflections(InflectionType::ADJECTIVAL, "ms,fs");
  _checkInflections(InflectionType::ADJECTIVAL, "ms,f");
  _checkInflections(InflectionType::ADJECTIVAL, "ms,s");
  _checkInflections(InflectionType::ADJECTIVAL, "msC,s");
  ASSERT_ANY_THROW(_checkInflections(InflectionType::ADJECTIVAL, "e"));
  ASSERT_ANY_THROW(_checkInflections(InflectionType::ADJECTIVAL, "P"));
  ASSERT_ANY_THROW(_checkInflections(InflectionType::ADJECTIVAL, "ms, fs"));
  ASSERT_ANY_THROW(_checkInflections(InflectionType::ADJECTIVAL, "lol"));
  ASSERT_ANY_THROW(_checkInflections(InflectionType::ADJECTIVAL, "mss"));
  ASSERT_ANY_THROW(_checkInflections(InflectionType::ADJECTIVAL, "3ms"));
  ASSERT_ANY_THROW(_checkInflections(InflectionType::ADJECTIVAL, "Pms"));

  _checkInflections(InflectionType::NOMINAL, "");
  _checkInflections(InflectionType::NOMINAL, "m");
  _checkInflections(InflectionType::NOMINAL, "s");
  _checkInflections(InflectionType::NOMINAL, ",m");
  _checkInflections(InflectionType::NOMINAL, "ms");
  _checkInflections(InflectionType::NOMINAL, "ms,fs");
  _checkInflections(InflectionType::NOMINAL, "ms,f");
  _checkInflections(InflectionType::NOMINAL, "ms,s");
  _checkInflections(InflectionType::NOMINAL, "ms,mp,fs,fp,ns,np");
  _checkInflections(InflectionType::NOMINAL, "ms,mp,fs,fp");
  _checkInflections(InflectionType::NOMINAL, "ns,np");
  ASSERT_ANY_THROW(_checkInflections(InflectionType::NOMINAL, "e"));
  ASSERT_ANY_THROW(_checkInflections(InflectionType::NOMINAL, "P"));
  ASSERT_ANY_THROW(_checkInflections(InflectionType::NOMINAL, "C"));
  ASSERT_ANY_THROW(_checkInflections(InflectionType::NOMINAL, "S"));
  ASSERT_ANY_THROW(_checkInflections(InflectionType::NOMINAL, "ms, fs"));
  ASSERT_ANY_THROW(_checkInflections(InflectionType::NOMINAL, "msC,s"));
  ASSERT_ANY_THROW(_checkInflections(InflectionType::NOMINAL, "lol"));
  ASSERT_ANY_THROW(_checkInflections(InflectionType::NOMINAL, "mss"));
  ASSERT_ANY_THROW(_checkInflections(InflectionType::NOMINAL, "3ms"));
  ASSERT_ANY_THROW(_checkInflections(InflectionType::NOMINAL, "Pms"));

  _checkInflections(InflectionType::PRONOMINAL, "");
  _checkInflections(InflectionType::PRONOMINAL, "m");
  _checkInflections(InflectionType::PRONOMINAL, "s");
  _checkInflections(InflectionType::PRONOMINAL, ",m");
  _checkInflections(InflectionType::PRONOMINAL, "ms");
  _checkInflections(InflectionType::PRONOMINAL, "3ms");
  _checkInflections(InflectionType::PRONOMINAL, "3ms,m2s,pf1", "3ms,2ms,1fp");
  _checkInflections(InflectionType::PRONOMINAL, "ms,fs");
  _checkInflections(InflectionType::PRONOMINAL, "ms,f");
  _checkInflections(InflectionType::PRONOMINAL, "ms,s");
  _checkInflections(InflectionType::PRONOMINAL, "ms,mp,fs,fp,ns,np");
  _checkInflections(InflectionType::PRONOMINAL, "ms,mp,fs,fp");
  _checkInflections(InflectionType::PRONOMINAL, "ns,np");
  ASSERT_ANY_THROW(_checkInflections(InflectionType::PRONOMINAL, "e"));
  ASSERT_ANY_THROW(_checkInflections(InflectionType::PRONOMINAL, "P"));
  ASSERT_ANY_THROW(_checkInflections(InflectionType::PRONOMINAL, "C"));
  ASSERT_ANY_THROW(_checkInflections(InflectionType::PRONOMINAL, "S"));
  ASSERT_ANY_THROW(_checkInflections(InflectionType::PRONOMINAL, "ms, fs"));
  ASSERT_ANY_THROW(_checkInflections(InflectionType::PRONOMINAL, "msC,s"));
  ASSERT_ANY_THROW(_checkInflections(InflectionType::PRONOMINAL, "lol"));
  ASSERT_ANY_THROW(_checkInflections(InflectionType::PRONOMINAL, "mss"));
  ASSERT_ANY_THROW(_checkInflections(InflectionType::PRONOMINAL, "Pms"));


  _checkInflections(InflectionType::VERBAL, "");
  _checkInflections(InflectionType::VERBAL, "P1", "P1s");
  _checkInflections(InflectionType::VERBAL, "I1m", "I1ms");
  _checkInflections(InflectionType::VERBAL, "S1ms");
  _checkInflections(InflectionType::VERBAL, "T1p");
  _checkInflections(InflectionType::VERBAL, "Y2s,F2p");
  _checkInflections(InflectionType::VERBAL, "Cs", "C3s");
  _checkInflections(InflectionType::VERBAL, "Js1", "J1s");
  _checkInflections(InflectionType::VERBAL, "W");
  _checkInflections(InflectionType::VERBAL, "G1s,Gp2", "G1s,G2p");
  _checkInflections(InflectionType::VERBAL, "K");
  _checkInflections(InflectionType::VERBAL, "F1s,K3p");
  _checkInflections(InflectionType::VERBAL, "Pms", "P3ms");
  ASSERT_ANY_THROW(_checkInflections(InflectionType::VERBAL, "U"));
  ASSERT_ANY_THROW(_checkInflections(InflectionType::VERBAL, "A"));
  ASSERT_ANY_THROW(_checkInflections(InflectionType::VERBAL, "m"));
  ASSERT_ANY_THROW(_checkInflections(InflectionType::VERBAL, "s"));
  ASSERT_ANY_THROW(_checkInflections(InflectionType::VERBAL, "1Ps"));
  ASSERT_ANY_THROW(_checkInflections(InflectionType::VERBAL, ",P1s"));
  ASSERT_ANY_THROW(_checkInflections(InflectionType::VERBAL, "ms"));
  ASSERT_ANY_THROW(_checkInflections(InflectionType::VERBAL, "3ms"));
  ASSERT_ANY_THROW(_checkInflections(InflectionType::VERBAL, "3ms,m2s,pf1", "3ms,2ms,1fp"));
  ASSERT_ANY_THROW(_checkInflections(InflectionType::VERBAL, "ms,s"));
  ASSERT_ANY_THROW(_checkInflections(InflectionType::VERBAL, "P1s,np"));
  ASSERT_ANY_THROW(_checkInflections(InflectionType::VERBAL, "e"));
  ASSERT_ANY_THROW(_checkInflections(InflectionType::VERBAL, "ms, fs"));
  ASSERT_ANY_THROW(_checkInflections(InflectionType::VERBAL, "msC,s"));
  ASSERT_ANY_THROW(_checkInflections(InflectionType::VERBAL, "lol"));
  ASSERT_ANY_THROW(_checkInflections(InflectionType::VERBAL, "mss"));

  _checkInflections(InflectionType::EMPTY, "");
  _checkInflections(InflectionType::EMPTY, "ms", "");
  _checkInflections(InflectionType::EMPTY, "hello", "");
}
