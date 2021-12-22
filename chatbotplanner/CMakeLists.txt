project(chatbotplanner)
cmake_minimum_required(VERSION 2.8)


# Making a lib
set(CHATBOTPLANNER_HPPS
    include/onsem/chatbotplanner/exportsymbols/macro.hpp
    include/onsem/chatbotplanner/api.hpp
    include/onsem/chatbotplanner/alias.hpp
    include/onsem/chatbotplanner/fact.hpp
    include/onsem/chatbotplanner/chatbotplanner.hpp
    include/onsem/chatbotplanner/arithmeticevaluator.hpp
    include/onsem/chatbotplanner/observableunsafe.hpp
)


set(CHATBOTPLANNER_SRCS
    src/fact.cpp
    src/chatbotplanner.cpp
    src/arithmeticevaluator.cpp
)


include_directories(include)


add_library(onsemchatbotplanner SHARED
  ${CHATBOTPLANNER_HPPS}
  ${CHATBOTPLANNER_SRCS}
)
install(TARGETS onsemchatbotplanner DESTINATION lib)
install(DIRECTORY include/ DESTINATION include)


add_subdirectory(tests)

