get_filename_component(_onsemchatbotplanner_root "${CMAKE_CURRENT_LIST_FILE}" PATH)
get_filename_component(_onsemchatbotplanner_root "${_onsemchatbotplanner_root}" ABSOLUTE)


set(ONSEMCHATBOTPLANNER_FOUND TRUE)

set(
  ONSEMCHATBOTPLANNER_INCLUDE_DIRS
  ${_onsemchatbotplanner_root}/include
  CACHE INTERNAL "" FORCE
)

set(
  ONSEMCHATBOTPLANNER_LIBRARIES
  "onsemchatbotplanner"
)



