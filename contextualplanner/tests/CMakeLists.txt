cmake_minimum_required(VERSION 2.8)

project(contextualplanner_tests)

include_directories(".")

if(NOT CONTEXTUALPLANNER_FOUND)
  find_package(contextualplanner)
endif()


add_executable(
  contextualplanner_tests

  src/test_chatbotplanner.cpp
  src/test_arithmeticevaluator.hpp
  src/test_arithmeticevaluator.cpp
)


target_link_libraries(contextualplanner_tests
  ${CONTEXTUALPLANNER_LIBRARIES}
)
