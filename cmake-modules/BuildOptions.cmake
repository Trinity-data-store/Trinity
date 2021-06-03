include(CMakeDependentOption)

# Components to build
option(BUILD_TESTS "Build with unittests" ON)

message(STATUS "----------------------------------------------------------")
message(STATUS "${PROJECT_NAME} version:                            ${PROJECT_VERSION}")
message(STATUS "Build configuration Summary")
message(STATUS "  Build unit tests:                       ${BUILD_TESTS}")
message(STATUS "----------------------------------------------------------")
