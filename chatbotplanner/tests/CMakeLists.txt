cmake_minimum_required(VERSION 2.8)

project(chatbotplanner_tests)

include_directories(".")

if(NOT ONSEMCHATBOTPLANNER_FOUND)
  find_package(onsemchatbotplanner)
endif()


add_executable(
  chatbotplanner_tests

  src/test_chatbotplanner.cpp
  src/test_arithmeticevaluator.hpp
  src/test_arithmeticevaluator.cpp
)


target_link_libraries(chatbotplanner_tests
  ${ONSEMCHATBOTPLANNER_LIBRARIES}
)
