get_filename_component(_onsemcommon_root "${CMAKE_CURRENT_LIST_FILE}" PATH)
get_filename_component(_onsemcommon_root "${_onsemcommon_root}" ABSOLUTE)


set(ONSEMCOMMON_FOUND TRUE)

set(
  ONSEMCOMMON_INCLUDE_DIRS
  ${_onsemcommon_root}/include
  CACHE INTERNAL "" FORCE
)

set(
  ONSEMCOMMON_LIBRARIES
  "onsemcommon"
)


