get_filename_component(_onsemsemantictotext_root "${CMAKE_CURRENT_LIST_FILE}" PATH)
get_filename_component(_onsemsemantictotext_root "${_onsemsemantictotext_root}" ABSOLUTE)


set(ONSEMSEMANTICTOTEXT_FOUND TRUE)

set(
  ONSEMSEMANTICTOTEXT_INCLUDE_DIRS
  ${_onsemsemantictotext_root}/include
  CACHE INTERNAL "" FORCE
)

set(
  ONSEMSEMANTICTOTEXT_LIBRARIES
  "onsemsemantictotext"
)



