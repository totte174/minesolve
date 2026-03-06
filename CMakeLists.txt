cmake_minimum_required(VERSION 3.15)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR
        "In-source builds are not supported. Run cmake from a separate directory:\n"
        "  cmake -B build && make -C build\n"
        "Or simply: make")
endif()

project(minesolve VERSION 0.2.0 LANGUAGES C)

# Default to Release if invoked without an explicit build type
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release RelWithDebInfo MinSizeRel)
endif()

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)  # Required: uses __builtin_popcountll and GNU statement-expression macros

if(NOT CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    message(WARNING "minesolve requires GCC or Clang for __builtin_popcountll and GNU statement-expression extensions.")
endif()

# Optional profiling support (off by default; -pg adds measurable overhead)
option(MINESOLVE_ENABLE_PROFILING "Build with gprof profiling instrumentation (-pg)" OFF)

set(MINESOLVE_COMPILE_OPTIONS -Wall -Wextra)
list(APPEND MINESOLVE_COMPILE_OPTIONS $<$<CONFIG:Release,RelWithDebInfo>:-march=native>)
if(MINESOLVE_ENABLE_PROFILING)
    list(APPEND MINESOLVE_COMPILE_OPTIONS -pg)
endif()

# ----------- Library -----------

set(LIB_SOURCES
    src/lib/board.c
    src/lib/solver.c
    src/lib/equations.c
    src/lib/permutations.c
    src/lib/probability.c
    src/lib/minesolve.c
)

add_library(minesolve_static STATIC ${LIB_SOURCES})
set_target_properties(minesolve_static PROPERTIES OUTPUT_NAME minesolve)
target_include_directories(minesolve_static PUBLIC include)
target_link_libraries(minesolve_static PUBLIC m)  # PUBLIC: static lib consumers must also link m
target_compile_options(minesolve_static PRIVATE ${MINESOLVE_COMPILE_OPTIONS})

add_library(minesolve_shared SHARED ${LIB_SOURCES})
set_target_properties(minesolve_shared PROPERTIES OUTPUT_NAME minesolve)
target_include_directories(minesolve_shared PUBLIC include)
target_link_libraries(minesolve_shared PRIVATE m)
target_compile_options(minesolve_shared PRIVATE ${MINESOLVE_COMPILE_OPTIONS})

# ----------- CLI -----------

add_executable(minesolve src/cli/main.c src/cli/commands.c)
target_include_directories(minesolve PRIVATE include src/cli)
target_link_libraries(minesolve PRIVATE minesolve_static)
target_compile_options(minesolve PRIVATE ${MINESOLVE_COMPILE_OPTIONS})
if(MINESOLVE_ENABLE_PROFILING)
    target_link_options(minesolve PRIVATE -pg)
endif()

# ----------- Install -----------

include(GNUInstallDirs)

install(TARGETS minesolve_static minesolve_shared minesolve
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(FILES include/minesolve.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
