cmake_minimum_required(VERSION 3.20)
project(gitscribe-shell VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Output to bin directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Source files
set(SOURCES
    src/dllmain.cpp
    src/ClassFactory.cpp
    src/GitScribeOverlay.cpp
    src/GitRepository.cpp
    src/MenuContext.cpp
    src/ContextMenu.cpp
    src/AppLauncher.cpp
    src/PropertySheet.cpp
    src/PerformanceCache.cpp
)

# Header files
set(HEADERS
    src/ClassFactory.h
    src/GitScribeOverlay.h
    src/GitRepository.h
    src/MenuContext.h
    src/ContextMenu.h
    src/AppLauncher.h
    src/PropertySheet.h
    src/PerformanceCache.h
    src/resource.h
)

# Create the DLL
add_library(gitscribe-shell SHARED ${SOURCES} ${HEADERS})

# Link against gitscribe-core and Windows libraries
target_link_libraries(gitscribe-shell PRIVATE
    ${CMAKE_SOURCE_DIR}/../gitscribe-core/target/x86_64-pc-windows-msvc/release/gitscribe_core.dll.lib
    shlwapi.lib  # For PathFileExists
)

# Include gitscribe-core headers (we'll generate these)
target_include_directories(gitscribe-shell PRIVATE
    ${CMAKE_SOURCE_DIR}/../gitscribe-core/include
)

# Windows-specific settings
if(WIN32)
    # Embed resources
    target_sources(gitscribe-shell PRIVATE src/gitscribe-shell.rc)

    # Set DLL properties
    set_target_properties(gitscribe-shell PROPERTIES
        PREFIX ""
        OUTPUT_NAME "GitScribeShell"
        SUFFIX ".dll"
    )

    # Export DEF file for COM
    set_target_properties(gitscribe-shell PROPERTIES
        LINK_FLAGS "/DEF:${CMAKE_SOURCE_DIR}/src/gitscribe-shell.def"
    )
endif()

# Copy Rust DLL to output directory after build
add_custom_command(TARGET gitscribe-shell POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/../gitscribe-core/target/x86_64-pc-windows-msvc/release/gitscribe_core.dll"
        "${CMAKE_BINARY_DIR}/bin/$<CONFIG>/gitscribe_core.dll"
    COMMENT "Copying Rust DLL to output directory"
)

# Copy icon resources to output directory
add_custom_command(TARGET gitscribe-shell POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/resources"
        "${CMAKE_BINARY_DIR}/resources"
    COMMENT "Copying icon resources to output directory"
)

# Installation
install(TARGETS gitscribe-shell
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
)

install(FILES "${CMAKE_SOURCE_DIR}/../gitscribe-core/target/x86_64-pc-windows-msvc/release/gitscribe_core.dll"
    DESTINATION bin
)

# Test executable for debugging (commented out - not implemented yet)
# add_executable(test-overlay ../test-overlay.cpp)
# target_link_libraries(test-overlay PRIVATE ole32 shell32)
