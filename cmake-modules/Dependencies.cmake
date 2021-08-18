include(ExternalProject)
include(GNUInstallDirs)

# External C/C++ Flags
set(EXTERNAL_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_${UPPERCASE_BUILD_TYPE}}")
set(EXTERNAL_C_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_${UPPERCASE_BUILD_TYPE}}")

# Prefer static to dynamic libraries
set(CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_STATIC_LIBRARY_SUFFIX} ${CMAKE_FIND_LIBRARY_SUFFIXES})

# Threads
find_package(Threads REQUIRED)

# Jemalloc
include(JemallocExternal)
set(HEAP_MANAGER_EP "jemalloc_ep")
set(HEAP_MANAGER_LIBRARY ${JEMALLOC_LIBRARY})

# Apache Thrift
include(ThriftExternal)

# If testing is enabled
if (BUILD_TESTS)
	# Catch2
	include(CatchExternal)
endif()
