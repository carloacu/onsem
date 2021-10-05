get_filename_component(_onsemtypeallocatorandserialyzer_root "${CMAKE_CURRENT_LIST_FILE}" PATH)
get_filename_component(_onsemtypeallocatorandserialyzer_root "${_onsemtypeallocatorandserialyzer_root}" ABSOLUTE)


set(ONSEMTYPEALLOCATORANDSERIALYZER_FOUND TRUE)

set(
  ONSEMTYPEALLOCATORANDSERIALYZER_INCLUDE_DIRS
  ${_onsemtypeallocatorandserialyzer_root}/include
  CACHE INTERNAL "" FORCE
)

set(
  ONSEMTYPEALLOCATORANDSERIALYZER_LIBRARIES
  "onsemtypeallocatorandserialyzer"
)


