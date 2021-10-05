get_filename_component(_onsemlingdbeditor_root "${CMAKE_CURRENT_LIST_FILE}" PATH)
get_filename_component(_onsemlingdbeditor_root "${_onsemlingdbeditor_root}" ABSOLUTE)


set(ONSEMLINGDBEDITOR_FOUND TRUE)

set(
  ONSEMLINGDBEDITOR_INCLUDE_DIRS
  ${_onsemlingdbeditor_root}/include
  CACHE INTERNAL "" FORCE
)

set(
  ONSEMLINGDBEDITOR_LIBRARIES
  "onsemlingdbeditor"
)


