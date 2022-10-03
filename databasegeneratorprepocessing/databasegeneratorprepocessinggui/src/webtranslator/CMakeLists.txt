project(webtranslator)
cmake_minimum_required(VERSION 2.8)

set(CMAKE_AUTOMOC OFF)


if(MSVC)
  # Force to always compile with W4
  if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
    string(REGEX REPLACE "/W[0-4]" "/W3" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3")
  endif()
elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  # Update if necessary
  set(C_WARNFLAGS
    #-Werror
    -Wall
    -Wextra
    -Wconversion
    -Wpointer-arith
    -Wcast-align
    -fno-strict-aliasing
    -Wno-long-long
  )
  foreach(arg ${C_WARNFLAGS})
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${arg}")
  endforeach(arg ${C_WARNFLAGS})

  set(CXX_WARNFLAGS
    ${C_WARNFLAGS}
    -pedantic
    #-pedantic-errors
    -Weffc++
    -Wold-style-cast
  )
  foreach(arg ${CXX_WARNFLAGS})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${arg}")
  endforeach(arg ${CXX_WARNFLAGS})
endif()

# Making a lib
set(WEBTRANSLATOR_SRCS

)


include_directories(".")
add_executable(webtranslator ${WEBTRANSLATOR_SRCS})


