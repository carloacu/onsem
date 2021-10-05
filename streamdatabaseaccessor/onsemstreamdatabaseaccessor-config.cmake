get_filename_component(_onsemstreamdatabaseaccessor_root "${CMAKE_CURRENT_LIST_FILE}" PATH)
get_filename_component(_onsemstreamdatabaseaccessor_root "${_onsemstreamdatabaseaccessor_root}" ABSOLUTE)


set(ONSEMSTREADATABASEACCESSOR_FOUND TRUE)

set(
  ONSEMSTREADATABASEACCESSOR_INCLUDE_DIRS
  ${_onsemstreamdatabaseaccessor_root}/include
  CACHE INTERNAL "" FORCE
)

set(
  ONSEMSTREADATABASEACCESSOR_LIBRARIES
  "onsemstreamdatabaseaccessor"
)



