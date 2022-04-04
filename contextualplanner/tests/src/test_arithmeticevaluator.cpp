#include "test_arithmeticevaluator.hpp"
#include <iostream>
#include <assert.h>
#include <contextualplanner/arithmeticevaluator.hpp>

using namespace cp;

namespace
{
template <typename TYPE>
void assert_eq(const TYPE& pExpected,
               const TYPE& pValue)
{
  if (pExpected != pValue)
    assert(false);
}

}

void test_arithmeticEvaluator()
{ 
  assert_eq(2, evalute("1+1"));
  assert_eq(6, evalute("5+1"));
  assert_eq(16, evalute("15+1"));
  try {
    evalute("`15+1`");
    assert(false);
  } catch (...) {}
  assert_eq(14, evalute("`13+1`", 1));
  assert_eq<std::string>("14", evaluteToStr("`13+1`", 1));
  assert_eq<std::string>("_", evaluteToStr("`13+1`"));
  std::cout << "arithmetic evaluator is ok !!!" << std::endl;
}
