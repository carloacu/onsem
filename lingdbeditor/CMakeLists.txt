project(lingdbeditor)
cmake_minimum_required(VERSION 2.8)


set(ONSEMLINGDBEDITOR_HPPS
  include/onsem/lingdbeditor/allingdbwordforms.hpp
  include/onsem/lingdbeditor/allingdbtypes.hpp
  include/onsem/lingdbeditor/allingdbmodifier.hpp
  include/onsem/lingdbeditor/allingdbmeaning.hpp
  include/onsem/lingdbeditor/allingdbflexions.hpp
  include/onsem/lingdbeditor/allingdbdynamictrienode.hpp
  include/onsem/lingdbeditor/allingdbmultimeaningnode.hpp
  include/onsem/lingdbeditor/linguisticintermediarydatabase.hpp
  include/onsem/lingdbeditor/allingdbstring.hpp
  include/onsem/lingdbeditor/meaningandconfidence.hpp
  include/onsem/lingdbeditor/loaders/alwlksdatabaseloader.hpp
  include/onsem/lingdbeditor/loaders/aldeladatabaseloader.hpp
  include/onsem/lingdbeditor/loaders/alanydatabaseloader.hpp
  include/onsem/lingdbeditor/details/allingdbwordforms.hxx
  include/onsem/lingdbeditor/details/allingdbmeaning.hxx
  include/onsem/lingdbeditor/details/allingdbdynamictrienode.hxx
  include/onsem/lingdbeditor/details/linguisticintermediarydatabase.hxx
  include/onsem/lingdbeditor/databaserules/allingdbquestionwords.hpp
  include/onsem/lingdbeditor/savers/albinarydatabaseconceptssaver.hpp
  include/onsem/lingdbeditor/savers/albinarydatabasedicosaver.hpp
  include/onsem/lingdbeditor/savers/details/albinarydatabasedicosaver.hxx
  include/onsem/lingdbeditor/savers/albinarytradsaver.hpp
  include/onsem/lingdbeditor/savers/albinarydatabasesaver.hpp
  include/onsem/lingdbeditor/savers/details/albinarydatabasesaver.hxx
  include/onsem/lingdbeditor/savers/traductionwriter.hpp
  include/onsem/lingdbeditor/allingdbmeaningtowords.hpp
  include/onsem/lingdbeditor/allingdbtree.hpp
  include/onsem/lingdbeditor/details/allingdbtree.hxx
)

set(ONSEMLINGDBEDITOR_SRCS
  src/concept/allingdblinktoaconcept.hpp
  src/concept/allingdbconcept.hpp
  src/allingdbanimationtag.hpp
  src/allingdbwordforms.cpp
  src/allingdbmodifier.cpp
  src/allingdbmeaning.cpp
  src/allingdbflexions.cpp
  src/allingdbdynamictrienode.cpp
  src/allingdbmultimeaningnode.cpp
  src/linguisticintermediarydatabase.cpp
  src/allingdbanimationtag.cpp
  src/allingdbstring.cpp
  src/meaningandconfidence.cpp
  src/savers/traductionwriter.cpp
  src/loaders/alxmldatabaseloader.hpp
  src/loaders/alxmldatabaseloader.cpp
  src/loaders/alwlksdatabaseloader.cpp
  src/loaders/aldeladatabaseloader.cpp
  src/loaders/algfsdatabaseloader.hpp
  src/loaders/algfsdatabaseloader.cpp
  src/loaders/alcptsdatabaseloader.hpp
  src/loaders/alcptsdatabaseloader.cpp
  src/loaders/alrladatabaseloader.hpp
  src/loaders/alrladatabaseloader.cpp
  src/loaders/alanydatabaseloader.cpp
  src/databaserules/allingdbquestionwords.cpp
  src/concept/allingdblinktoaconcept.cpp
  src/concept/allingdbconcept.cpp
  src/savers/albinarydatabaseconceptssaver.cpp
  src/savers/albinarydatabasedicosaver.cpp
  src/savers/albinarytradsaver.cpp
  src/savers/albinarydatabasesaver.cpp
  src/allingdbmeaningtowords.cpp
  src/allingdbtree.cpp
)


include_directories(include)


if(NOT ONSEMTYPEALLOCATORANDSERIALYZER_FOUND)
  find_package(onsemtypeallocatorandserialyzer)
endif()
if(NOT FOUND_ONSEMCOMMON)
  find_package(onsemcommon)
endif()


add_library(onsemlingdbeditor STATIC
  ${ONSEMLINGDBEDITOR_HPPS}
  ${ONSEMLINGDBEDITOR_SRCS}
)

include_directories(
  ${BOOST_INCLUDE_DIRS}
  ${ONSEMTYPEALLOCATORANDSERIALYZER_INCLUDE_DIRS}
  ${ONSEMCOMMON_INCLUDE_DIRS}
)

target_link_libraries(onsemlingdbeditor
  ${BOOST_LIBRARIES}
  ${ONSEMTYPEALLOCATORANDSERIALYZER_LIBRARIES}
  ${ONSEMCOMMON_LIBRARIES}
)




