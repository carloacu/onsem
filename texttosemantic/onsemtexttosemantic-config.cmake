get_filename_component(_onsemtexttosemantic_root "${CMAKE_CURRENT_LIST_FILE}" PATH)
get_filename_component(_onsemtexttosemantic_root "${_onsemtexttosemantic_root}" ABSOLUTE)


set(ONSEMTEXTTOSEMANTIC_FOUND TRUE)

set(
  ONSEMTEXTTOSEMANTIC_INCLUDE_DIRS
  ${_onsemtexttosemantic_root}/include
  CACHE INTERNAL "" FORCE
)

set(
  ONSEMTEXTTOSEMANTIC_LIBRARIES
  "onsemtexttosemantic"
)


