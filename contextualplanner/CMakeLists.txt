project(contextualplanner)
cmake_minimum_required(VERSION 2.8)


# Making a lib
set(CHATBOTPLANNER_HPPS
    include/contextualplanner/exportsymbols/macro.hpp
    include/contextualplanner/api.hpp
    include/contextualplanner/alias.hpp
    include/contextualplanner/fact.hpp
    include/contextualplanner/goal.hpp
    include/contextualplanner/contextualplanner.hpp
    include/contextualplanner/arithmeticevaluator.hpp
    include/contextualplanner/observableunsafe.hpp
)


set(CHATBOTPLANNER_SRCS
    src/fact.cpp
    src/goal.cpp
    src/contextualplanner.cpp
    src/arithmeticevaluator.cpp
)


include_directories(include)


add_library(contextualplanner SHARED
  ${CHATBOTPLANNER_HPPS}
  ${CHATBOTPLANNER_SRCS}
)
target_compile_features(contextualplanner PRIVATE cxx_std_14)

install(TARGETS contextualplanner DESTINATION lib)
install(DIRECTORY include/ DESTINATION include)


add_subdirectory(tests)

