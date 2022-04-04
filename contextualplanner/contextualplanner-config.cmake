get_filename_component(_contextualplanner_root "${CMAKE_CURRENT_LIST_FILE}" PATH)
get_filename_component(_contextualplanner_root "${_contextualplanner_root}" ABSOLUTE)


set(CONTEXTUALPLANNER_FOUND TRUE)

set(
  CONTEXTUALPLANNER_INCLUDE_DIRS
  ${_contextualplanner_root}/include
  CACHE INTERNAL "" FORCE
)

set(
  CONTEXTUALPLANNER_LIBRARIES
  "contextualplanner"
)



