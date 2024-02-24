project(semanticdebugger)
cmake_minimum_required(VERSION 2.8)

# Making a lib
set(ONSEMSEMANTCDEBUGGER_HPPS
  include/onsem/semanticdebugger/printer/semanticprinter.hpp
  include/onsem/semanticdebugger/printer/semexplinestostr.hpp
  include/onsem/semanticdebugger/api.hpp
  include/onsem/semanticdebugger/aretextsequivalent.hpp
  include/onsem/semanticdebugger/diagnosisprinter.hpp
  include/onsem/semanticdebugger/dotsaver.hpp
  include/onsem/semanticdebugger/loaddbpediatxtmemory.hpp
  include/onsem/semanticdebugger/syntacticgraphresult.hpp
  include/onsem/semanticdebugger/semanticdebug.hpp
  include/onsem/semanticdebugger/textananlyzedebugger.hpp
  include/onsem/semanticdebugger/timechecker.hpp
)

set(ONSEMSEMANTCDEBUGGER_SRCS
  src/printer/semanticprinter.cpp
  src/printer/semexplinestostr.cpp
  src/printer/sentimentspecprinter.hpp
  src/printer/sentimentspecprinter.cpp
  src/aretextsequivalent.cpp
  src/diagnosisprinter.cpp
  src/dotsaver.cpp
  src/loaddbpediatxtmemory.cpp
  src/semanticdebug.cpp
  src/textananlyzedebugger.cpp
  src/timechecker.cpp
)


include_directories(include)

find_package(Boost REQUIRED)

if(NOT ONSEMCOMMON_FOUND)
  find_package(onsemcommon)
endif()
if(NOT ONSEMTEXTTOSEMANTIC_FOUND)
  find_package(onsemtexttosemantic)
endif()
if(NOT ONSEMSEMANTICTOTEXT_FOUND)
  find_package(onsemsemantictotext)
endif()

add_library(onsemsemanticdebugger SHARED
  ${ONSEMSEMANTCDEBUGGER_HPPS}
  ${ONSEMSEMANTCDEBUGGER_SRCS}
)

include_directories(
  ${ONSEMCOMMON_INCLUDE_DIRS}
  ${ONSEMTEXTTOSEMANTIC_INCLUDE_DIRS}
  ${ONSEMSEMANTICTOTEXT_INCLUDE_DIRS}
)

target_link_libraries(onsemsemanticdebugger
  z
  ${ONSEMCOMMON_LIBRARIES}
  ${ONSEMTEXTTOSEMANTIC_LIBRARIES}
  ${ONSEMSEMANTICTOTEXT_LIBRARIES}
)

target_compile_features(onsemsemanticdebugger PRIVATE cxx_std_17)

install(TARGETS onsemsemanticdebugger DESTINATION lib)
install(DIRECTORY include/ DESTINATION include)

