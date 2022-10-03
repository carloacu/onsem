get_filename_component(_onsemdictionaryextractor_root "${CMAKE_CURRENT_LIST_FILE}" PATH)
get_filename_component(_onsemdictionaryextractor_root "${_onsemdictionaryextractor_root}" ABSOLUTE)


set(ONSEMDICTIONARYEXTRACTOR_FOUND TRUE)

set(
  ONSEMDICTIONARYEXTRACTOR_INCLUDE_DIRS
  ${_onsemdictionaryextractor_root}/include
  CACHE INTERNAL "" FORCE
)

set(
  ONSEMDICTIONARYEXTRACTOR_LIBRARIES
  "onsemdictionaryextractor"
)



