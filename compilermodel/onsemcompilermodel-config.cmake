get_filename_component(_onsemcompilermodel_root "${CMAKE_CURRENT_LIST_FILE}" PATH)
get_filename_component(_onsemcompilermodel_root "${_onsemcompilermodel_root}" ABSOLUTE)


set(ONSEMCOMPILERMODEL_FOUND TRUE)

set(
  ONSEMCOMPILERMODEL_INCLUDE_DIRS
  ${_onsemcompilermodel_root}/include
  CACHE INTERNAL "" FORCE
)

set(
  ONSEMCOMPILERMODEL_LIBRARIES
  "onsemcompilermodel"
)


