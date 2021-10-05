project(chatbotplanner)
cmake_minimum_required(VERSION 2.8)


# Making a lib
set(CHATBOTPLANNER_HPPS
    include/onsem/chatbotplanner/api.hpp
    include/onsem/chatbotplanner/alias.hpp
    include/onsem/chatbotplanner/linguisticplanner.hpp
    include/onsem/chatbotplanner/arithmeticevaluator.hpp
)


set(CHATBOTPLANNER_SRCS
    src/linguisticplanner.cpp
    src/arithmeticevaluator.cpp
)


include_directories(include)


if(NOT ONSEMCOMMON_FOUND)
  find_package(onsemcommon)
endif()

add_library(onsemchatbotplanner SHARED
  ${CHATBOTPLANNER_HPPS}
  ${CHATBOTPLANNER_SRCS}
)
include_directories(
  ${ONSEMCOMMON_INCLUDE_DIRS}
)
target_link_libraries(onsemchatbotplanner
  ${ONSEMCOMMON_LIBRARIES}
)
install(TARGETS onsemchatbotplanner DESTINATION lib)
install(DIRECTORY include/ DESTINATION include)



