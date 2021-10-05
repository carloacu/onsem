get_filename_component(_onsemsemanticdebugger_root "${CMAKE_CURRENT_LIST_FILE}" PATH)
get_filename_component(_onsemsemanticdebugger_root "${_onsemsemanticdebugger_root}" ABSOLUTE)


set(ONSEMSEMANTOCDEBUGGER_FOUND TRUE)

set(
  ONSEMSEMANTOCDEBUGGER_INCLUDE_DIRS
  ${_onsemsemanticdebugger_root}/include
  CACHE INTERNAL "" FORCE
)

set(
  ONSEMSEMANTOCDEBUGGER_LIBRARIES
  "onsemsemanticdebugger"
)



