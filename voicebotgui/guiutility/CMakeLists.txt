project(qisem_guiutility)
cmake_minimum_required(VERSION 2.8)



# Making a lib
set(QISEM_GUIUTILITY_H
    qisem/guiutility/lineedithistoricwrapper.hpp
)

set(QISEM_GUIUTILITY_SRCS
    src/lineedithistoricwrapper.cpp
)


option(
  USE_QIBUILD
  "Compile using qibuild."
  ON)

include_directories(".")

if (USE_QIBUILD)

find_package(qibuild)
qi_sanitize_compile_flags(HIDDEN_SYMBOLS)
qi_create_lib(qisem_guiutility
              SRC
              ${QISEM_GUIUTILITY_SRCS} ${QISEM_GUIUTILITY_H}
              STATIC)
qi_use_lib(qisem_guiutility
           QT5_CORE
           QT5_WIDGETS)
qi_stage_lib(qisem_guiutility)

qi_install_header(${QISEM_GUIUTILITY_H} KEEP_RELATIVE_PATHS)

else()

find_package(Qt5 COMPONENTS Core Widgets REQUIRED)

add_library(qisem_guiutility STATIC
            ${QISEM_GUIUTILITY_SRCS} ${QISEM_GUIUTILITY_H})

target_link_libraries(qisem_guiutility
                      Qt5::Core Qt5::Widgets)


endif()
