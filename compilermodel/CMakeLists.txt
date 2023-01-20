project(compilermodel)
cmake_minimum_required(VERSION 2.8)


set(ONSEMCOMPILERMODEL_HPPS
  include/onsem/compilermodel/lingdbwordforms.hpp
  include/onsem/compilermodel/lingdbtypes.hpp
  include/onsem/compilermodel/lingdbmodifier.hpp
  include/onsem/compilermodel/lingdbmeaningtowords.hpp
  include/onsem/compilermodel/lingdbmeaning.hpp
  include/onsem/compilermodel/lingdbflexions.hpp
  include/onsem/compilermodel/lingdbdynamictrienode.hpp
  include/onsem/compilermodel/lingdbmultimeaningnode.hpp
  include/onsem/compilermodel/linguisticintermediarydatabase.hpp
  include/onsem/compilermodel/lingdbstring.hpp
  include/onsem/compilermodel/meaningandconfidence.hpp
  include/onsem/compilermodel/loaders/wlksdatabaseloader.hpp
  include/onsem/compilermodel/loaders/deladatabaseloader.hpp
  include/onsem/compilermodel/loaders/anydatabaseloader.hpp
  include/onsem/compilermodel/details/lingdbwordforms.hxx
  include/onsem/compilermodel/details/lingdbmeaning.hxx
  include/onsem/compilermodel/details/lingdbdynamictrienode.hxx
  include/onsem/compilermodel/details/linguisticintermediarydatabase.hxx
  include/onsem/compilermodel/databaserules/lingdbquestionwords.hpp
  include/onsem/compilermodel/savers/binarydatabaseconceptssaver.hpp
  include/onsem/compilermodel/savers/binarydatabasedicosaver.hpp
  include/onsem/compilermodel/savers/details/binarydatabasedicosaver.hxx
  include/onsem/compilermodel/savers/binarytradsaver.hpp
  include/onsem/compilermodel/savers/binarydatabasesaver.hpp
  include/onsem/compilermodel/savers/details/binarydatabasesaver.hxx
  include/onsem/compilermodel/savers/traductionwriter.hpp
  include/onsem/compilermodel/lingdbtree.hpp
  include/onsem/compilermodel/details/lingdbtree.hxx
)

set(ONSEMCOMPILERMODEL_SRCS
  src/concept/lingdblinktoaconcept.hpp
  src/concept/lingdbconcept.hpp
  src/lingdbanimationtag.hpp
  src/lingdbwordforms.cpp
  src/lingdbmodifier.cpp
  src/lingdbmeaningtowords.cpp
  src/lingdbmeaning.cpp
  src/lingdbflexions.cpp
  src/lingdbdynamictrienode.cpp
  src/lingdbmultimeaningnode.cpp
  src/linguisticintermediarydatabase.cpp
  src/lingdbanimationtag.cpp
  src/lingdbstring.cpp
  src/meaningandconfidence.cpp
  src/savers/traductionwriter.cpp
  src/loaders/xmldatabaseloader.hpp
  src/loaders/xmldatabaseloader.cpp
  src/loaders/wlksdatabaseloader.cpp
  src/loaders/deladatabaseloader.cpp
  src/loaders/gfsdatabaseloader.hpp
  src/loaders/gfsdatabaseloader.cpp
  src/loaders/cptsdatabaseloader.hpp
  src/loaders/cptsdatabaseloader.cpp
  src/loaders/rladatabaseloader.hpp
  src/loaders/rladatabaseloader.cpp
  src/loaders/anydatabaseloader.cpp
  src/databaserules/lingdbquestionwords.cpp
  src/concept/lingdblinktoaconcept.cpp
  src/concept/lingdbconcept.cpp
  src/savers/binarydatabaseconceptssaver.cpp
  src/savers/binarydatabasedicosaver.cpp
  src/savers/binarytradsaver.cpp
  src/savers/binarydatabasesaver.cpp
  src/lingdbtree.cpp
)


include_directories(include)


if(NOT ONSEMTYPEALLOCATORANDSERIALYZER_FOUND)
  find_package(onsemtypeallocatorandserialyzer)
endif()
if(NOT ONSEMCOMMON_FOUND)
  find_package(onsemcommon)
endif()

add_library(onsemcompilermodel STATIC
  ${ONSEMCOMPILERMODEL_HPPS}
  ${ONSEMCOMPILERMODEL_SRCS}
)

include_directories(
  ${BOOST_INCLUDE_DIRS}
  ${ONSEMTYPEALLOCATORANDSERIALYZER_INCLUDE_DIRS}
  ${ONSEMCOMMON_INCLUDE_DIRS}
)

target_link_libraries(onsemcompilermodel
  ${ONSEMTYPEALLOCATORANDSERIALYZER_LIBRARIES}
  ${ONSEMCOMMON_LIBRARIES}
)
target_compile_features(onsemcompilermodel PRIVATE cxx_std_17)



