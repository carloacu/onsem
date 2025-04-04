project(typeallocatorandserialyzer)
cmake_minimum_required(VERSION 2.8)


# Making a lib
set(TYPEALLOCATORANDSERIALYZER_HPPS
  include/onsem/typeallocatorandserializer/typeallocatorandserializer.hpp
  include/onsem/typeallocatorandserializer/advanced/treememoryprettyprinter.hpp
  include/onsem/typeallocatorandserializer/advanced/leafpoolallocator.hpp
  include/onsem/typeallocatorandserializer/advanced/compositepoolallocator.hpp
  include/onsem/typeallocatorandserializer/advanced/componentpoolallocator.hpp
  include/onsem/typeallocatorandserializer/advanced/details/treememoryprettyprinter.hxx
  include/onsem/typeallocatorandserializer/advanced/details/leafpoolallocator.hxx
  include/onsem/typeallocatorandserializer/advanced/details/compositepoolallocator.hxx
  include/onsem/typeallocatorandserializer/details/typeallocatorandserializer.hxx
)

set(TYPEALLOCATORANDSERIALYZER_SRCS
  src/compositepoolallocator.cpp
  src/componentpoolallocator.cpp
)



include_directories(include)


if(NOT ONSEMCOMMON_FOUND)
  find_package(onsemcommon)
endif()


add_library(onsemtypeallocatorandserialyzer
  ${TYPEALLOCATORANDSERIALYZER_HPPS}
  ${TYPEALLOCATORANDSERIALYZER_SRCS}
)

# Add position independent code flag if building shared libs or if explicitly requested
if(BUILD_SHARED_LIBS OR POSITION_INDEPENDENT_CODE)
    set_target_properties(onsemtypeallocatorandserialyzer PROPERTIES
        POSITION_INDEPENDENT_CODE ON)
endif()

include_directories(
  ${ONSEMCOMMON_INCLUDE_DIRS}
)
target_link_libraries(onsemtypeallocatorandserialyzer
  ${ONSEMCOMMON_LIBRARIES}
)



