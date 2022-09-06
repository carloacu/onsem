get_filename_component(_onsemguiutility_root "${CMAKE_CURRENT_LIST_FILE}" PATH)
get_filename_component(_onsemguiutility_root "${_onsemguiutility_root}" ABSOLUTE)


set(ONSEMGUIUTILITY_FOUND TRUE)

set(
  ONSEMGUIUTILITY_INCLUDE_DIRS
  ${_onsemguiutility_root}/include
  CACHE INTERNAL "" FORCE
)

set(
  ONSEMGUIUTILITY_LIBRARIES
  "onsemguiutility"
)



