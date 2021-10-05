get_filename_component(_onsem_root "${CMAKE_CURRENT_LIST_FILE}" PATH)
get_filename_component(_onsem_root "${_onsem_root}" ABSOLUTE)

include(${_onsem_root}/common/onsemcommon-config.cmake)
include(${_onsem_root}/typeallocatorandserializer/onsemtypeallocatorandserialyzer-config.cmake)
include(${_onsem_root}/lingdbeditor/onsemlingdbeditor-config.cmake)

include(${_onsem_root}/chatbotplanner/onsemchatbotplanner-config.cmake)
include(${_onsem_root}/texttosemantic/onsemtexttosemantic-config.cmake)
include(${_onsem_root}/semantictotext/onsemsemantictotext-config.cmake)
include(${_onsem_root}/streamdatabaseaccessor/onsemstreamdatabaseaccessor-config.cmake)
include(${_onsem_root}/semanticdebugger/onsemsemanticdebugger-config.cmake)
include(${_onsem_root}/tests/onsemtester/onsemtester-config.cmake)

