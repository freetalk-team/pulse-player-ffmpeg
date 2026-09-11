
macro (SET_TARGET NAME)
    set (TARGET ${NAME})
endmacro ()

macro (ADD_SOURCES)
    target_sources(${TARGET} PRIVATE ${ARGN})
endmacro ()

macro (ADD_SOURCES_IF COND)
    if (${COND})
        target_sources(${TARGET} PRIVATE ${ARGN})
    endif ()
endmacro ()

macro (ADD_SOURCES_IF_NOT COND)
    if (NOT ${COND})
        target_sources(${TARGET} PRIVATE ${ARGN})
    endif ()
endmacro ()

macro(ADD_NASM_DEFINITION name)
    list(APPEND NASM_DEFINES "${name}=${${name}}")
endmacro()

macro(ADD_NASM_DEFINITIONS)
    foreach(name ${ARGV})
        list(APPEND NASM_DEFINES "${name}=${${name}}")
    endforeach()
endmacro()

