project(databasegeneratorprepocessinggui)
cmake_minimum_required(VERSION 2.8)


# Avoid compile failed between boost "signals" and Qt "signals"
add_definitions(-DQT_NO_KEYWORDS)


# Define Source files
set(${PROJECT_NAME}_SRC
  src/dbGeneratorpreprocessingwindow.hpp
  src/dbGeneratorpreprocessingwindow.cpp
  src/main.cpp

  src/firstnamesextractor/firstnamesextractor.hpp
  src/firstnamesextractor/detail/firstnamesextractor.cpp

  src/frgrammarbookextractor/frgrammarbookextractor.hpp
  src/frgrammarbookextractor/detail/frgrammarbookextractor.cpp
  src/frgrammarbookextractor/detail/helpers.hpp
  src/frgrammarbookextractor/detail/helpers.cpp

  src/aspiredh/aspiredhextractor.hpp
  src/aspiredh/aspiredhextractor.cpp

  src/textextractor/textextractor.hpp
  src/textextractor/detail/textextractor.cpp
  src/textextractor/detail/adjbeforenounextractor.hpp
  src/textextractor/detail/adjbeforenounextractor.cpp
  src/textextractor/detail/verbprepextractor.hpp
  src/textextractor/detail/verbprepextractor.cpp
  src/textextractor/detail/metaextractor.hpp
  src/textextractor/detail/metaextractor.cpp
  src/textextractor/detail/verbcanbefollowedbydeinfr.hpp
  src/textextractor/detail/verbcanbefollowedbydeinfr.cpp

  src/wikitionaryextractor/wikitionaryextractor.hpp
  src/wikitionaryextractor/detail/wikitionaryextractor.cpp
  src/wikitionaryextractor/detail/dataextractor.cpp
  src/wikitionaryextractor/detail/dataextractor.hpp
  src/wikitionaryextractor/detail/conceptsextractor.hpp
  src/wikitionaryextractor/detail/conceptsextractor.cpp
  src/wikitionaryextractor/detail/conceptsextractor.cpp
  src/wikitionaryextractor/detail/addcomposedwords.cpp
  src/wikitionaryextractor/detail/addcomposedwords.hpp
  src/wikitionaryextractor/detail/wikiutils.cpp
  src/wikitionaryextractor/detail/wikiutils.hpp
  src/wikitionaryextractor/detail/transitiveverbextractor.hpp
  src/wikitionaryextractor/detail/transitiveverbextractor.cpp
  src/wikitionaryextractor/detail/metawiki/wikikeywords.hpp
  src/wikitionaryextractor/detail/metawiki/patternrecognizer.hpp
  src/wikitionaryextractor/detail/metawiki/patternrecognizer.cpp
  src/wikitionaryextractor/detail/frwiki/frwikikeywords.cpp
  src/wikitionaryextractor/detail/frwiki/frwikikeywords.hpp
  src/wikitionaryextractor/detail/frwiki/frpatternrecognizer.cpp
  src/wikitionaryextractor/detail/frwiki/frpatternrecognizer.hpp
  src/wikitionaryextractor/detail/enwiki/enwikikeywords.cpp
  src/wikitionaryextractor/detail/enwiki/enwikikeywords.hpp
  src/wikitionaryextractor/detail/enwiki/enpatternrecognizer.cpp
  src/wikitionaryextractor/detail/enwiki/enpatternrecognizer.hpp

  src/webtranslator/webtranslator.hpp
  src/webtranslator/detail/webtranslator.cpp
  src/webtranslator/detail/webrequest.hpp
  src/webtranslator/detail/webrequest.cpp
  src/webtranslator/detail/webresults.hpp
  src/webtranslator/detail/webresults.cpp

  src/traductiongenerator/traductiongenerator.hpp
  src/traductiongenerator/detail/traductiongenerator.cpp
  )

# Define ui file
set(${PROJECT_NAME}_UI
  src/dbGeneratorpreprocessingwindow.ui
  )


# Define Moc Header files
set(${PROJECT_NAME}_MOC
  src/dbGeneratorpreprocessingwindow.hpp
  )


#qi_create_bin(${PROJECT_NAME} SRC
#${${PROJECT_NAME}_SRC}
#${${PROJECT_NAME}_UI_SRC}
#NO_INSTALL
#)


include_directories(${CMAKE_CURRENT_BINARY_DIR})

find_package(Qt5 COMPONENTS Core Widgets Xml REQUIRED)

# This is called *before* any call to target_link_libraries() is made.
# Here we need to define the qt macros, such as qt5_wrap_cpp,
# qt5_add_resources, qt5_wrap_ui
set(CMAKE_AUTOMOC ON)


qt5_wrap_ui(${PROJECT_NAME}_UI_SRC ${${PROJECT_NAME}_UI})


find_package(Threads REQUIRED)
if(NOT ONSEMCOMMON_FOUND)
  find_package(onsemcommon)
endif()
if(NOT ONSEMTEXTTOSEMANTIC_FOUND)
  find_package(onsemtexttosemantic)
endif()
if(NOT ONSEMTYPEALLOCATORANDSERIALYZER_FOUND)
  find_package(onsemtypeallocatorandserialyzer)
endif()
if(NOT ONSEMCOMPILERMODEL_FOUND)
  find_package(onsemcompilermodel)
endif()
if(NOT ONSEMSTREADATABASEACCESSOR_FOUND)
  find_package(onsemstreamdatabaseaccessor)
endif()
if(NOT ONSEMDICTIONARYEXTRACTOR_FOUND)
  find_package(onsemdictionaryextractor)
endif()


add_executable(${PROJECT_NAME}
  ${${PROJECT_NAME}_SRC}
  ${${PROJECT_NAME}_UI_SRC}
)

include_directories(
  ${BOOST_PROPERTY_TREE_WITH_DEPS_INCLUDE_DIRS}
  ${ONSEMCOMMON_INCLUDE_DIRS}
  ${ONSEMTEXTTOSEMANTIC_INCLUDE_DIRS}
  ${ONSEMTYPEALLOCATORANDSERIALYZER_INCLUDE_DIRS}
  ${ONSEMCOMPILERMODEL_INCLUDE_DIRS}
  ${ONSEMSTREADATABASEACCESSOR_INCLUDE_DIRS}
  ${ONSEMDICTIONARYEXTRACTOR_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}
  Qt5::Core Qt5::Widgets Qt5::Xml
  pthread
  ${ONSEMCOMMON_LIBRARIES}
  ${ONSEMTEXTTOSEMANTIC_LIBRARIES}
  ${ONSEMTYPEALLOCATORANDSERIALYZER_LIBRARIES}
  ${ONSEMCOMPILERMODEL_LIBRARIES}
  ${ONSEMSTREADATABASEACCESSOR_LIBRARIES}
  ${ONSEMDICTIONARYEXTRACTOR_LIBRARIES}
)


target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

