#include <gtest/gtest.h>
#include <iostream>
#include <onsem/texttosemantic/dbtype/semanticquantity.hpp>
#include "../semanticreasonergtests.hpp"

using namespace onsem;

std::string _loadWriteOfNumber(const std::string& pNumber)
{
  SemanticFloat nb;
  nb.fromStr(pNumber);
  return nb.toStr();
}

bool _checkLoadWriteOfNumber(const std::string& pNumber)
{
  auto nbAfterParsing = _loadWriteOfNumber(pNumber);
  if (nbAfterParsing == pNumber)
    return true;
  std::cout << "nbAfterParsing = \"" << nbAfterParsing << "\" | pNumber = \"" << pNumber << "\"" << std::endl;
  return false;
}


std::string _add(const std::string& pNb1, const std::string& pNb2)
{
  SemanticFloat nb1;
  nb1.fromStr(pNb1);
  SemanticFloat nb2;
  nb2.fromStr(pNb2);
  auto res2 = (nb1 + nb2).toStr();
  nb1.add(nb2);
  auto res = nb1.toStr();
  EXPECT_EQ(res, res2);
  return res;
}

std::string _subsctract(const std::string& pNb1, const std::string& pNb2)
{
  SemanticFloat nb1;
  nb1.fromStr(pNb1);
  SemanticFloat nb2;
  nb2.fromStr(pNb2);
  auto res2 = (nb1 - nb2).toStr();
  nb1.substract(nb2);
  auto res = nb1.toStr();
  EXPECT_EQ(res, res2);
  return res;
}


TEST_F(SemanticReasonerGTests, test_semanticfloat)
{
  EXPECT_TRUE(_checkLoadWriteOfNumber("3"));
  EXPECT_TRUE(_checkLoadWriteOfNumber("-1"));
  EXPECT_TRUE(_checkLoadWriteOfNumber("-0.1"));
  EXPECT_TRUE(_checkLoadWriteOfNumber("-3.4"));
  EXPECT_TRUE(_checkLoadWriteOfNumber("3.08"));

  EXPECT_EQ("0", _loadWriteOfNumber(""));
  EXPECT_EQ("0", _loadWriteOfNumber("a"));
  EXPECT_EQ("0", _loadWriteOfNumber("3.-4"));

  EXPECT_EQ("4", _add("3", "1"));
  EXPECT_EQ("2", _add("3", "-1"));
  EXPECT_EQ("3.1", _add("3", "0.1"));
  EXPECT_EQ("3.4", _add("3.3", "0.1"));
  EXPECT_EQ("3.12", _add("3.02", "0.1"));
  EXPECT_EQ("2.9", _add("3", "-0.1"));

  EXPECT_EQ("7", _subsctract("19", "12"));
}
