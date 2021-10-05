get_filename_component(_onsemtester_root "${CMAKE_CURRENT_LIST_FILE}" PATH)
get_filename_component(_onsemtester_root "${_onsemtester_root}" ABSOLUTE)


set(ONSEMTESTER_FOUND TRUE)

set(
  ONSEMTESTER_INCLUDE_DIRS
  ${_onsemtester_root}/include
  CACHE INTERNAL "" FORCE
)

set(
  ONSEMTESTER_LIBRARIES
  "onsemtester"
)



