# Vendored dependencies. Each is skipped when the consuming super-project
# already provides the same target, so embedding tsn-gen via
# add_subdirectory/FetchContent never collides with a host project's copy.

if(NOT TARGET ryml::ryml)
    if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/rapidyaml/CMakeLists.txt)
        message(FATAL_ERROR
            "rapidyaml submodule missing. Run:\n"
            "  git submodule update --init --recursive external/rapidyaml")
    endif()
    add_subdirectory(rapidyaml)
endif()

if(ENABLE_PARSER_TESTS AND NOT TARGET GTest::gtest_main)
    if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/googletest/CMakeLists.txt)
        message(FATAL_ERROR
            "googletest submodule missing. Run:\n"
            "  git submodule update --init external/googletest")
    endif()
    # Test-only dependency: keep it out of `cmake --install`.
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    add_subdirectory(googletest)
endif()
