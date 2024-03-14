cmake_minimum_required(VERSION 2.8)


set(ONSEMCOMMON_HPPS
    include/onsem/common/exportsymbols/macro.hpp
    include/onsem/common/api.hpp
    include/onsem/common/linguisticmeaning_nomeaningid.hpp
    include/onsem/common/linguisticsubordinateid.hpp
    include/onsem/common/enum/chunklinktype.hpp
    include/onsem/common/enum/comparisonoperator.hpp
    include/onsem/common/enum/comparisontype.hpp
    include/onsem/common/enum/contextualannotation.hpp
    include/onsem/common/enum/grammaticaltype.hpp
    include/onsem/common/enum/infomationtype.hpp
    include/onsem/common/enum/linguisticcondition.hpp
    include/onsem/common/enum/semanticverbtense.hpp
    include/onsem/common/enum/interpretationsource.hpp
    include/onsem/common/enum/lingenumlinkedmeaningdirection.hpp
    include/onsem/common/enum/linguisticverbtense.hpp
    include/onsem/common/enum/listexpressiontype.hpp
    include/onsem/common/enum/partofspeech.hpp
    include/onsem/common/enum/relativeperson.hpp
    include/onsem/common/enum/relativepersonwithoutnumber.hpp
    include/onsem/common/enum/semanticgendertype.hpp
    include/onsem/common/enum/semanticnumbertype.hpp
    include/onsem/common/enum/semanticentitytype.hpp
    include/onsem/common/enum/semanticquantitytype.hpp
    include/onsem/common/enum/semanticlanguageenum.hpp
    include/onsem/common/enum/semanticrequesttype.hpp
    include/onsem/common/enum/semanticreferencetype.hpp
    include/onsem/common/enum/semanticrelativelocationtype.hpp
    include/onsem/common/enum/semanticrelativetimetype.hpp
    include/onsem/common/enum/semanticrelativedurationtype.hpp
    include/onsem/common/enum/semanticsubjectivequantity.hpp
    include/onsem/common/enum/timeweekdayenum.hpp
    include/onsem/common/enum/verbgoalenum.hpp
    include/onsem/common/enum/semanticsourceenum.hpp
    include/onsem/common/enum/treepatternconventionenum.hpp
    include/onsem/common/enum/wordcontextualinfos.hpp
    include/onsem/common/enum/wordtoverbrelation.hpp
    include/onsem/common/binary/binarymasks.hpp
    include/onsem/common/binary/binaryloader.hpp
    include/onsem/common/binary/binarysaver.hpp
    include/onsem/common/utility/container.hpp
    include/onsem/common/binary/enummapreader.hpp
    include/onsem/common/binary/enummapsaver.hpp
    include/onsem/common/binary/radixmapreaderwithkeyretrieving.hpp
    include/onsem/common/binary/radixmapsaverwithkeyretrieving.hpp
    include/onsem/common/binary/radixmapreader.hpp
    include/onsem/common/binary/radixmapsaver.hpp
    include/onsem/common/binary/simpleintmapping.hpp
    include/onsem/common/utility/detail/searchendingpoint.hpp
    include/onsem/common/utility/are_equals.hpp
    include/onsem/common/utility/enum_vector.hpp
    include/onsem/common/utility/enum_vector_initialized.hpp
    include/onsem/common/utility/getendofparenthesis.hpp
    include/onsem/common/utility/lexical_cast.hpp
    include/onsem/common/utility/optional.hpp
    include/onsem/common/utility/noresult.hpp
    include/onsem/common/utility/number.hpp
    include/onsem/common/utility/observable/connection.hpp
    include/onsem/common/utility/observable/observable.hpp
    include/onsem/common/utility/observable/observableunsafe.hpp
    include/onsem/common/utility/pointer_from.hpp
    include/onsem/common/utility/radix_map.hpp
    include/onsem/common/utility/radix_map_forward_declaration.hpp
    include/onsem/common/utility/radix_map_struct.hpp
    include/onsem/common/utility/random.hpp
    include/onsem/common/utility/sizeprinterinbytes.hpp
    include/onsem/common/utility/string.hpp
    include/onsem/common/utility/to_underlying.hpp
    include/onsem/common/utility/unique_propagate_const.hpp
    include/onsem/common/utility/unordered_set_of_refs.hpp
    include/onsem/common/utility/uppercasehandler.hpp
    include/onsem/common/utility/vector_map.hpp
    include/onsem/common/utility/vector_of_refs.hpp
    include/onsem/common/keytostreams.hpp
)

set(ONSEMCOMMON_SRCS
    src/utility/getendofparenthesis.cpp
    src/utility/random.cpp
    src/utility/string.cpp
    src/utility/uppercasehandler.cpp
    src/keytostreams.cpp
)

if(MSVC)
  # Force to always compile with W4
  if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
    string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
  endif()
elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  # Update if necessary
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long -pedantic")
endif()


include_directories(include)

add_library(onsemcommon SHARED
            ${ONSEMCOMMON_SRCS} ${ONSEMCOMMON_HPPS})
target_compile_features(onsemcommon PRIVATE cxx_std_17)

target_include_directories(onsemcommon PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)


# Create a proper installed package that can be included.
install(TARGETS onsemcommon
    EXPORT onsemcommonTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
install(DIRECTORY include/onsem DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT onsemcommonTargets
    FILE onsemcommonTargets.cmake
    NAMESPACE onsem::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/onsem
)
